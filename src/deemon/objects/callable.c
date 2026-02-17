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
#ifndef GUARD_DEEMON_OBJECTS_CALLABLE_C
#define GUARD_DEEMON_OBJECTS_CALLABLE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/callable.h>           /*  */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/format.h>             /* DeeFormat_PRINT, DeeFormat_Printf, PRFuSIZ */
#include <deemon/none-operator.h>      /* DeeNone_Operator* */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_*, Dee_DecrefNokill, Dee_Decref_unlikely, Dee_Incref, Dee_Movprefv, Dee_OBJECT_HEAD, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, Dee_visit_t, OBJECT_HEAD_INIT */
#include <deemon/objmethod.h>          /*  */
#include <deemon/seq.h>                /* DeeRefVector_NewReadonly */
#include <deemon/serial.h>             /* DeeSerial, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/type.h>               /* DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, Dee_TYPE_CONSTRUCTOR_INIT_VAR, METHOD_FCONSTCALL, TF_NONE, TP_F*, TYPE_*, type_* */

#include "../runtime/strings.h"

#include <stddef.h> /* NULL, size_t */

/* `Callable from deemon'
 *
 * Base class for callable wrapper types, such as ObjMethod, CMethod,
 * InstanceMethod or just a plain old function. There is no particular
 * reason why this exists, other than to allow user-code to query
 * for a type for that particular set of objects by simply writing
 * `x is Callable from deemon' */

DECL_BEGIN

INTDEF DeeTypeObject FunctionComposition_Type;

#define composition_serialize tuple_serialize
INTDEF WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL tuple_serialize(DeeTupleObject *__restrict self, DeeSerial *__restrict writer);
#define composition_fini tuple_fini
INTDEF NONNULL((1)) void DCALL tuple_fini(DeeTupleObject *__restrict self);
#define composition_visit tuple_visit
INTERN NONNULL((1, 2)) void DCALL tuple_visit(DeeTupleObject *__restrict self, Dee_visit_t proc, void *arg);

#if defined(CONFIG_NO_CACHES) || defined(CONFIG_NO_TUPLE_CACHES) || defined(CONFIG_EXPERIMENTAL_MMAP_DEC)
#define composition_tp_free_PTR NULL
#else /* CONFIG_NO_CACHES || CONFIG_NO_TUPLE_CACHES || CONFIG_EXPERIMENTAL_MMAP_DEC */
INTDEF NONNULL((1)) void DCALL tuple_tp_free(void *__restrict ob);
#define composition_tp_free_PTR &tuple_tp_free
#endif /* !CONFIG_NO_CACHES && !CONFIG_NO_TUPLE_CACHES && !CONFIG_EXPERIMENTAL_MMAP_DEC */

#define composition_hash tuple_hash
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL tuple_hash(DeeTupleObject *__restrict self);

PRIVATE struct {
	Dee_OBJECT_HEAD
	size_t     t_size;
/*	DeeObject *t_elem[0];*/
} identity_composition = {
	OBJECT_HEAD_INIT(&FunctionComposition_Type),
	/* .t_size = */ 0
};

/* Create a composite function taking a singular
 * argument and returning the result of:
 * >> argv[0](argv[1](argv[2](...(argv[argc-2](argv[argc-1](IN)))))
 *
 * This function is used to implement `Callable.compose()' */
PUBLIC WUNUSED ATTR_INS(2, 1) DREF DeeObject *DCALL
DeeFunctionComposition_Of(size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DREF DeeObject **dst;
	size_t i, total;
	total = argc;
	for (i = 0; i < argc; ++i) {
		DeeObject *cb = argv[i];
		if (DeeObject_InstanceOfExact(cb, &FunctionComposition_Type)) {
			total -= 1;
			total += ((DeeTupleObject *)cb)->t_size;
		}
	}
	switch (total) {
	case 0:
		Dee_Incref(&identity_composition);
		return Dee_AsObject(&identity_composition);
	case 1: {
		for (i = 0;; ++i) {
			DeeObject *cb;
			ASSERT(i < argc);
			cb = argv[i];
			if (DeeObject_InstanceOfExact(cb, &FunctionComposition_Type)) {
				if (((DeeTupleObject *)cb)->t_size == 0)
					continue;
				ASSERT(((DeeTupleObject *)cb)->t_size == 1);
				cb = ((DeeTupleObject *)cb)->t_elem[0];
			}
			Dee_Incref(cb);
			return cb;
		}
	}	break;
	default: break;
	}
	ASSERT(argc >= 1);
	ASSERT(total >= 2);
	result = DeeTuple_NewUninitialized(total);
	if unlikely(!result)
		goto done;
	ASSERT(!DeeObject_IsShared(result));
	Dee_DecrefNokill(&DeeTuple_Type);
	Dee_Incref(&FunctionComposition_Type);
	result->ob_type = &FunctionComposition_Type;
	for (i = 0, dst = result->t_elem; i < argc; ++i) {
		DeeObject *cb = argv[i];
		if (DeeObject_InstanceOfExact(cb, &FunctionComposition_Type)) {
			DeeTupleObject *icom = (DeeTupleObject *)cb;
			dst = Dee_Movprefv(dst, icom->t_elem, icom->t_size);
		} else {
			Dee_Incref(cb);
			*dst++ = cb;
		}
	}
	ASSERT(dst == (result->t_elem + result->t_size));
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
composition_init(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = DeeFunctionComposition_Of(argc, argv);
	if likely(result && !DeeObject_InstanceOfExact(result, &FunctionComposition_Type)) {
		DREF DeeTupleObject *composition;
		composition = DeeTuple_NewUninitialized(1);
		if unlikely(!composition)
			goto err_r;
		composition->t_elem[0] = result; /* Inherit reference */
		result = Dee_AsObject(composition);
	}
	return result;
err_r:
	Dee_Decref_unlikely(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
composition_compare_eq(DeeTupleObject *self, DeeTupleObject *other) {
	size_t i;
	if (DeeObject_AssertTypeExact(other, &FunctionComposition_Type))
		goto err;
	if (self->t_size != other->t_size)
		return Dee_COMPARE_NE;
	for (i = 0; i < self->t_size; ++i) {
		int temp = DeeObject_TryCompareEq(self->t_elem[i], other->t_elem[i]);
		if (!Dee_COMPARE_ISEQ(temp))
			return temp;
	}
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
composition_call(DeeTupleObject *self, size_t argc, DeeObject *const *argv) {
	size_t i;
	DREF DeeObject *result;
	DeeArg_Unpack1(err, argc, argv, "Composition.operator ()", &result);
	Dee_Incref(result);
	i = self->t_size;
	while (i) {
		DREF DeeObject *new_result;
		--i;
		new_result = DeeObject_Call(self->t_elem[i], 1, &result);
		Dee_Decref_unlikely(result);
		result = new_result;
		if unlikely(!result)
			break;
	}
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
composition_print(DeeTupleObject *self, Dee_formatprinter_t printer, void *arg) {
	if (self->t_size == 0)
		return DeeFormat_PRINT(printer, arg, "Identity function");
	return DeeFormat_Printf(printer, arg, "Composition of %" PRFuSIZ " functions", self->t_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
composition_printrepr(DeeTupleObject *self, Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result;
	switch (self->t_size) {
	case 0:
		return DeeFormat_PRINT(printer, arg, "Callable.identity");
	case 1:
		return DeeFormat_Printf(printer, arg, "Callable.Compose(%r)", self->t_elem[0]);
	case 2:
		if (DeeObject_Implements(self->t_elem[0], &DeeCallable_Type)) {
			return DeeFormat_Printf(printer, arg, "%r.compose(%r)",
			                        self->t_elem[0], self->t_elem[1]);
		}
		return DeeFormat_Printf(printer, arg, "Callable.compose(%r, %r)",
		                        self->t_elem[0], self->t_elem[1]);
	default: break;
	}
	result = DeeFormat_Printf(printer, arg, "Callable.compose(%r", self->t_elem[0]);
	if unlikely(result < 0)
		return result;
	for (i = 1; i < self->t_size; ++i) {
		temp = DeeFormat_Printf(printer, arg, ", %r", self->t_elem[i]);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	temp = DeeFormat_PRINT(printer, arg, ")");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
composition_get_callbacks(DeeTupleObject *__restrict self) {
	return DeeRefVector_NewReadonly(Dee_AsObject(self), self->t_size, self->t_elem);
}

PRIVATE struct type_cmp composition_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&composition_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&composition_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE struct type_getset tpconst composition_getsets[] = {
	TYPE_GETTER_AB("__callbacks__", &composition_get_callbacks,
	               "->?S?DCallable\n"
	               "The callbacks that are part of this composition (in reverse order of "
	               /**/ "execution, but that when @this is invoked, ${__callbacks__.last} "
	               /**/ "is invoked first). When ?#__callbacks__ is empty, then @this composition "
	               /**/ "is the identity composition (i.e. ?Ert:FunctionComposition_identity) "),
	TYPE_GETSET_END
};

INTERN DeeTypeObject FunctionComposition_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FunctionComposition",
	/* .tp_doc      = */ DOC("Used to implement ?Acompose?DCallable"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &composition_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &composition_serialize,
			/* tp_free:        */ composition_tp_free_PTR
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&composition_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&composition_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&composition_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&composition_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &composition_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ composition_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject *const *))&composition_call,
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
callable_compose2(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *callbacks[2];
	DeeArg_Unpack1(err, argc, argv, "compose", &callbacks[1]);
	callbacks[0] = self;
	return DeeFunctionComposition_Of(2, callbacks);
err:
	return NULL;
}


PRIVATE struct type_method tpconst callable_methods[] = {
	TYPE_METHOD_F("compose", &callable_compose2, METHOD_FCONSTCALL,
	              "(before:?.)->?.\n"
	              "Return a composed single-argument callable that invokes @before on a given "
	              /**/ "argument, then invokes @this with the return value of @before:"
	              "${"
	              /**/ "function intToString(x: int): string { ... }\n"
	              /**/ "function stringToBytes(x: string): Bytes { ... }\n"
	              /**/ "local intToStringToBytes = x -\\> stringToBytes(intToString(x)); /* Same as this... */\n"
	              /**/ "local intToStringToBytes = stringToBytes.compose(intToString);\n"
	              /**/ "print intToStringToBytes(10);         /* \"10\".bytes() */\n"
	              /**/ "print stringToBytes(intToString(10)); /* \"10\".bytes() */"
	              "}\n"
	              "Implemented similarly to:\n"
	              "${"
	              /**/ "function compose(before: Callable): Callable {\n"
	              /**/ "	return x -\\> this(before(x));\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD_END
};

PRIVATE DEFINE_CMETHOD(callable_compose_obj, &DeeFunctionComposition_Of, METHOD_FCONSTCALL);

PRIVATE struct type_member tpconst callable_class_members[] = {
	TYPE_MEMBER_CONST_DOC("compose", &callable_compose_obj,
	                      "(functions!:?.)->?.\n"
	                      "Same as ?#{i:compose}, but can be used to compose an arbitrary "
	                      /**/ "number of key-like mapper functions into a single function. "
	                      /**/ "The returned function will invoke elements from @functions in reverse order:"
	                      "${"
	                      /**/ "function intToString(x: int): string { ... }\n"
	                      /**/ "function stringToBytes(x: string): Bytes { ... }\n"
	                      /**/ "local intToStringToBytes = x -\\> stringToBytes(intToString(x)); /* Same as this... */\n"
	                      /**/ "local intToStringToBytes = Callable.compose(stringToBytes, intToString);\n"
	                      /**/ "print intToStringToBytes(10);         /* \"10\".bytes() */\n"
	                      /**/ "print stringToBytes(intToString(10)); /* \"10\".bytes() */"
	                      "}\n"
	                      "Implemented similarly to:\n"
	                      "${"
	                      /**/ "static function compose(functions: {Callable...}): Callable {\n"
	                      /**/ "	local functionsInOrder = Sequence.frozen(functions).reversed();\n"
	                      /**/ "	if (#functionsInOrder == 1)\n"
	                      /**/ "		return functionsInOrder.first;\n"
	                      /**/ "	return x -\\> {\n"
	                      /**/ "		for (local fun: functionsInOrder)\n"
	                      /**/ "			x = fun(x);\n"
	                      /**/ "		return x;\n"
	                      /**/ "	};\n"
	                      /**/ "}"
	                      "}"),
	TYPE_MEMBER_CONST_DOC("Composition", &FunctionComposition_Type,
	                      "Type used for function compositions of 2 or more ?{.}s"),
	TYPE_MEMBER_CONST_DOC("identity", &identity_composition,
	                      "Identity function (same as ${Callable.compose()})"),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeCallable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Callable),
	/* .tp_doc      = */ DOC("Base class for callable types such as ?DFunction or ?DInstanceMethod, "
	                         /**/ "as well as any implementation-specific, wrapper object type"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorSerialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&object_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ callable_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ callable_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CALLABLE_C */
