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
#ifndef GUARD_DEX_CTYPES_CORE_C
#define GUARD_DEX_CTYPES_CORE_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC, Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>             /* DeeArg_Unpack*, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>            /* DeeBool_Type, Dee_False, Dee_True, return_bool */
#include <deemon/bytes.h>           /* DeeBytes* */
#include <deemon/callable.h>        /* DeeCallable_Type */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/float.h>           /* DeeFloat_Type */
#include <deemon/format.h>          /* PRFuSIZ */
#include <deemon/gc.h>              /* DeeGCObject_CALLOC, DeeGCObject_FREE, DeeGC_TRACK */
#include <deemon/int.h>             /* DeeInt_* */
#include <deemon/mro.h>             /* DeeObject_TGenericIterAttr, Dee_attrhint, Dee_attriter, Dee_attriter_initempty, Dee_attriterchain_builder, Dee_attriterchain_builder_* */
#include <deemon/none.h>            /* DeeNone_Check, DeeNone_Type, Dee_None */
#include <deemon/object.h>          /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, DREF, DeeBuffer, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BUFFER_FWRITABLE, Dee_Decref*, Dee_Incref, Dee_IncrefIfNotZero, Dee_TYPE, Dee_hash_t, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/seq.h>             /* DeeRefVector_NewReadonly */
#include <deemon/string.h>          /* DeeString*, Dee_UNICODE_PRINTER_INIT, Dee_UNICODE_PRINTER_PRINT, Dee_unicode_printer*, STRING_ERROR_FREPLAC */
#include <deemon/system-features.h> /* bcmp, bzero, memcmp, memcpy */
#include <deemon/type.h>            /* DeeObject_*, DeeTypeType_GetOperatorById, DeeType_*, Dee_BUFFER_TYPE_FNORMAL, Dee_Visit, Dee_Visitv, Dee_operator_t, Dee_opinfo, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/hash.h>       /* Dee_HashPtr */
#include <deemon/util/lock.h>       /* Dee_ATOMIC_RWLOCK_INIT */

#include <hybrid/overflow.h>      /* OVERFLOW_UADD, OVERFLOW_USUB */
#include <hybrid/sequence/list.h> /* LIST_ISBOUND, LIST_REMOVE */
#include <hybrid/typecore.h>      /* __BYTE_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* int32_t, int64_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#define Q3 "??" "?"
#define OPNAME(opname) "operator " opname

DOC_DEF(ispointer_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GStructuredType is a ?GPointerType");
DOC_DEF(islvalue_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GStructuredType is an ?GLValueType");
DOC_DEF(isarray_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GStructuredType is an ?GArrayType");
DOC_DEF(isfunction_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GStructuredType is a ?GFunctionType");
DOC_DEF(isstruct_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GStructuredType is a ?GStructType");
DOC_DEF(struct_tobytes_doc,
        "->?DBytes\n"
        "(data:?DBytes,offset=!0)->?DBytes\n"
        "#tValueError{The given ${#data - offset} is less than ?#sizeof}"
        "#tBufferError{The given @data is not writable}"
        "Retrieve the underlying bytes of @this struct object instance");


/* Interpret `self' as a pointer and store the result in `*result'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPointer(DeeObject *self,
                    DeeSTypeObject *pointer_base,
                    union pointer *__restrict result) {
	int error;
	DREF DeePointerTypeObject *pointer_type;
	error = DeeObject_TryAsPointer(self, pointer_base, result);
	if (error <= 0)
		return error;
	/* Emit a type-assertion error. */
	pointer_type = DeeSType_Pointer(pointer_base);
	if (pointer_type) {
		DeeObject_TypeAssertFailed(self, DeePointerType_AsType(pointer_type));
		Dee_Decref(DeePointerType_AsType(pointer_type));
	}
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsPPointer(DeeObject *self,
                     DeeSTypeObject *pointer_base_base,
                     void ***p_result) {
	int error;
	DREF DeePointerTypeObject *pointer_base;
	pointer_base = DeeSType_Pointer(pointer_base_base);
	if unlikely(!pointer_base)
		goto err;
	error = DeeObject_AsPointer(self, DeePointerType_AsSType(pointer_base),
	                            (union pointer *)p_result);
	Dee_Decref(DeePointerType_AsType(pointer_base));
	return error;
err:
	return -1;
}


/* Same as `DeeObject_AsPointer()', but only ~try~ to interpret it. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsPointer(DeeObject *self,
                       DeeSTypeObject *pointer_base,
                       union pointer *__restrict result) {
	if (DeeNone_Check(self)) {
		/* none is the NULL pointer. */
null_pointer:
		result->ptr = NULL;
		return 0;
	}

	if (DeePointer_Check(self)) {
		DeeSTypeObject *base;
		base = DeeType_AsPointerType(Dee_TYPE(self))->pt_orig;
		/* A pointer of the same type, or a void-pointer. */
		if (base == pointer_base ||
		    base == &DeeCVoid_Type ||
		    pointer_base == &DeeCVoid_Type) {
			result->ptr = ((struct pointer_object *)self)->p_ptr.ptr;
			return 0;
		}
		goto nope;
	}

	/* int(0) also counts as a NULL-pointer. */
	if (DeeInt_Check(self)) {
		unsigned int val;
		if (DeeInt_TryAsUInt(self, &val) && val == 0)
			goto null_pointer;
		goto nope;
	}

	/* Special handling for strings (which can be cast to `char *') */
	if (DeeString_Check(self)) {
		if (pointer_base == &DeeCVoid_Type) {
			result->pcvoid = DeeString_STR(self);
			return 0;
		}
		if (pointer_base == &DeeCChar_Type) {
			result->pcvoid = DeeString_AsUtf8(self);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		if (pointer_base == &DeeCWChar_Type) {
			result->pcvoid = DeeString_AsWide(self);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		if (pointer_base == &DeeCChar16_Type) {
			result->pcvoid = DeeString_AsUtf16(self, STRING_ERROR_FREPLAC);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		if (pointer_base == &DeeCChar32_Type) {
			result->pcvoid = DeeString_AsUtf32(self);
			if unlikely(!result->ptr)
				goto err;
			return 0;
		}
		goto nope;
	}

	/* Taking the pointer of a Bytes object yield the buffer's base */
	if (DeeBytes_Check(self)) {
		if (pointer_base == &DeeCVoid_Type ||
		    pointer_base == &DeeCChar_Type ||
		    pointer_base == &DeeCInt8_Type ||
		    pointer_base == &DeeCUInt8_Type) {
			result->ptr = DeeBytes_DATA(self);
			return 0;
		}
		goto nope;
	}

	/* Special handling for lvalue->pointer */
	if (DeeLValue_Check(self)) {
		DeeSTypeObject *lv_base;
		lv_base = DeeType_AsLValueType(Dee_TYPE(self))->lt_orig;
		if (DeePointerType_Check(lv_base)) {
			DeeSTypeObject *base;
			base = DeeSType_AsPointerType(lv_base)->pt_orig;
			/* A pointer of the same type, or a void-pointer. */
			if (base == pointer_base ||
			    base == &DeeCVoid_Type ||
			    pointer_base == &DeeCVoid_Type) {
				/* LValue -> Pointer (must deref) */
				CTYPES_FAULTPROTECT(result->ptr = *(void **)((struct lvalue_object *)self)->l_ptr.ptr,
				                    goto err);
				return 0;
			}
		}
		goto nope;
	}

	/* Conversion failed. */
nope:
	return 1;
err:
	return -1;
}

/* S.a. `DeeObject_TryAsGenericPointer()'
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_AsGenericPointer(DeeObject *self,
                           DeeSTypeObject **__restrict p_pointer_base,
                           union pointer *__restrict result) {
	int error;
	error = DeeObject_TryAsGenericPointer(self, p_pointer_base, result);
	if (error <= 0)
		return error;
	return DeeObject_TypeAssertFailed(self, DeePointerType_AsType(&DeePointer_Type));
}

/* Similar to `DeeObject_TryAsPointer()', but fills in `*p_pointer_base' with the
 * pointer-base type. For use with type-generic functions (such as the `atomic_*' api)
 * @return:  1: The conversion failed.
 * @return:  0: Successfully converted `self' to a pointer.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_TryAsGenericPointer(DeeObject *self,
                              DeeSTypeObject **__restrict p_pointer_base,
                              union pointer *__restrict result) {
	if (DeePointer_Check(self)) {
		DeeSTypeObject *base;
		base = DeeType_AsPointerType(Dee_TYPE(self))->pt_orig;
		result->ptr = ((struct pointer_object *)self)->p_ptr.ptr;
		*p_pointer_base = base;
		return 0;
	}

	if (DeeBytes_Check(self)) {
		result->ptr = DeeBytes_DATA(self);
		*p_pointer_base = &DeeCUInt8_Type;
		return 0;
	}

	/* Special handling for lvalue->pointer */
	if (DeeLValue_Check(self)) {
		DeeSTypeObject *lv_base;
		lv_base = DeeType_AsLValueType(Dee_TYPE(self))->lt_orig;
		if (DeePointerType_Check(lv_base)) {
			*p_pointer_base = DeeSType_AsPointerType(lv_base)->pt_orig;
			/* LValue -> Pointer (must deref) */
			CTYPES_FAULTPROTECT(result->ptr = *(void **)((struct lvalue_object *)self)->l_ptr.ptr,
			                    goto err);
			return 0;
		}
		goto nope;
	}

	/* Conversion failed. */
nope:
	return 1;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err:
	return -1;
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
stype_init(DeeSTypeObject *__restrict self) {
	/* Clear all new fields added by structured types. */
	bzero((&self->st_base) + 1,
	      sizeof(DeeSTypeObject) -
	      COMPILER_OFFSETAFTER(DeeSTypeObject, st_base));
	return (*DeeType_Type.tp_init.tp_alloc.tp_ctor)(Dee_AsObject(&self->st_base));
}

PRIVATE NONNULL((1)) void DCALL
stype_fini(DeeSTypeObject *__restrict self) {
	/* Free array/function type caches. */
	Dee_Free(self->st_array.sa_list);
#ifndef CONFIG_NO_CFUNCTION
	if (self->st_ffitype != &ffi_type_pointer)
		Dee_Free(self->st_ffitype);
	Dee_Free(self->st_cfunction.sf_list);
#endif /* !CONFIG_NO_CFUNCTION */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_dofunc(DeeSTypeObject *self, size_t argc,
             DeeObject *const *argv, ctypes_cc_t cc_flags) {
	size_t i;
	ctypes_cc_t cc;
	DeeSTypeObject **argv_types;
	cc = (ctypes_cc_t)((unsigned int)CC_DEFAULT |
	                   (unsigned int)cc_flags);
	if (argc && DeeString_Check(argv[0])) {
		--argc;
		++argv;
		cc = cc_lookup(DeeString_STR(argv[-1]));
		if unlikely(cc == CC_INVALID)
			goto err;
		cc = (ctypes_cc_t)((unsigned int)cc |
		                   (unsigned int)cc_flags);
	}
	argv_types = (DeeSTypeObject **)Dee_Mallocc(argc, sizeof(DeeSTypeObject *));
	if unlikely(!argv_types)
		goto err;

	/* Translate argument types. */
	for (i = 0; i < argc; ++i) {
		argv_types[i] = DeeSType_Get(argv[i]);
		if unlikely(!argv_types[i])
			goto err_argv;
	}

	/* Lookup the associated C-function type while inheriting the argument vector. */
	return DeeCFunctionType_AsObject(DeeSType_CFunction(self, cc, argc, argv_types, true));
err_argv:
	Dee_Free(argv_types);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_func(DeeSTypeObject *self, size_t argc, DeeObject *const *argv) {
	return stype_dofunc(self, argc, argv, (ctypes_cc_t)0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_vfunc(DeeSTypeObject *self, size_t argc, DeeObject *const *argv) {
	return stype_dofunc(self, argc, argv, CC_FVARARGS);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_frombytes(DeeSTypeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t type_size, data_size;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("frombytes", params: "
	DeeBytesObject *data;
	size_t offset = 0;
", docStringPrefix: "stype");]]]*/
#define stype_frombytes_params "data:?DBytes,offset=!0"
	struct {
		DeeBytesObject *data;
		size_t offset;
	} args;
	args.offset = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "frombytes", &args, &args.data, "o", _DeeArg_AsObject, &args.offset, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.data, &DeeBytes_Type))
		goto err;
	type_size = DeeSType_Sizeof(self);
	data_size = DeeBytes_SIZE(args.data);
	if unlikely(OVERFLOW_UADD(data_size, args.offset, &data_size))
		goto err_bad_size;
	if unlikely(type_size > data_size)
		goto err_bad_size;
	result = DeeType_AllocInstance(DeeSType_AsType(self));
	if unlikely(!result)
		goto err;
	memcpy(DeeObject_DATA(result), DeeBytes_DATA(args.data) + args.offset, type_size);
	DeeObject_Init((DeeStructObject *)result, DeeSType_AsType(self));
	return result;
err_bad_size:
	data_size = DeeBytes_SIZE(args.data);
	if (OVERFLOW_USUB(data_size, args.offset, &data_size))
		data_size = 0;
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid bytes size: structued type `%r' has an "
	                "instance size of `%" PRFuSIZ "', but the given "
	                "`%" PRFuSIZ "'-large buffer at offset `%" PRFuSIZ "' "
	                "provides at most `%" PRFuSIZ " bytes'",
	                self, type_size, DeeBytes_SIZE(args.data),
	                args.offset, data_size);
err:
	return NULL;
}

PRIVATE struct type_method tpconst stype_methods[] = {
	TYPE_METHOD("func", &stype_func,
	            "(types!:?DType)->?GFunctionType\n"
	            "(cc:?Dstring,types!:?DType)->?GFunctionType\n"
	            "#tValueError{The given @cc is unknown, or not supported by the host}"
	            "#pcc{The name of the calling convention}"
	            "Construct a new function prototype, using @types as argument, @this "
	            /**/ "as return type, and @cc as calling convention\n"
	            "Note that unlike ?#{op:call}, certain types from the ?Mdeemon core "
	            /**/ "are also accepted as argument types, such as ?Dbool inplace of ?Gbool"),
	TYPE_METHOD("vfunc", &stype_vfunc,
	            "(types!:?DType)->?GFunctionType\n"
	            "(cc:?Dstring,types!:?DType)->?GFunctionType\n"
	            "#tValueError{The given @cc is unknown, or not supported by the host}"
	            "#pcc{The name of the calling convention}"
	            "Same as ?#func, but enable support for varargs"),
	TYPE_METHOD("frombytes", &stype_frombytes,
	            "(" stype_frombytes_params ")->?.\n"
	            "#tValueError{The given ${#data - offset} is less than ?#sizeof}"
	            "Construct an instance of @this structured type from @data"),
	//TYPE_METHOD("is_pointer", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_lvalue", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_structured", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_struct", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_array", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_foreign_function", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_sizeof(DeeSTypeObject *__restrict self) {
	size_t result = DeeSType_Sizeof(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_alignof(DeeSTypeObject *__restrict self) {
	size_t result = DeeSType_Alignof(self);
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset tpconst stype_getsets[] = {
	TYPE_GETTER_AB("ptr", &DeeSType_Pointer,
	               "->?GPointerType\n"
	               "Returns the pointer type associated with @this ?GStructuredType"),
	TYPE_GETTER_AB("lvalue", &DeeSType_LValue,
	               "->?GLValueType\n"
	               "Returns the l-value type associated with @this ?GStructuredType"),
	TYPE_GETTER_AB_F("sizeof", &stype_sizeof, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Returns the size of @this ?GStructuredType in bytes"),
	TYPE_GETTER_AB_F("alignof", &stype_alignof, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Returns the alignment of @this ?GStructuredType in bytes"),
	TYPE_GETTER_AB("pointer", &DeeSType_Pointer,
	               "->?GPointerType\n"
	               "Alias for ?#ptr"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst stype_members[] = {
	TYPE_MEMBER_CONST_DOC("ispointer", Dee_False, DOC_GET(ispointer_doc)),
	TYPE_MEMBER_CONST_DOC("islvalue", Dee_False, DOC_GET(islvalue_doc)),
	TYPE_MEMBER_CONST_DOC("isarray", Dee_False, DOC_GET(isarray_doc)),
	TYPE_MEMBER_CONST_DOC("isfunction", Dee_False, DOC_GET(isfunction_doc)),
	TYPE_MEMBER_CONST_DOC("isstruct", Dee_False, DOC_GET(isstruct_doc)),
	TYPE_MEMBER_END
};



PRIVATE struct type_seq stype_seq = {
	/* .tp_iter          = */ NULL,
	/* .tp_sizeob        = */ NULL,
	/* .tp_contains      = */ NULL,
	/* .tp_getitem       = */ NULL,
	/* .tp_delitem       = */ NULL,
	/* .tp_setitem       = */ NULL,
	/* .tp_getrange      = */ NULL,
	/* .tp_delrange      = */ NULL,
	/* .tp_setrange      = */ NULL,
	/* .tp_foreach       = */ NULL,
	/* .tp_foreach_pair  = */ NULL,
	/* .tp_bounditem     = */ NULL,
	/* .tp_hasitem       = */ NULL,
	/* .tp_size          = */ NULL,
	/* .tp_size_fast     = */ NULL,
	/* .tp_getitem_index = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&DeeSType_Array,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stype_call(DeeSTypeObject *self, size_t argc, DeeObject *const *argv) {
	size_t i;

	/* Create a new instance, or create a new function-type. */
	if (!argc)
		goto create_inst;
	for (i = 0; i < argc; ++i) {
		if (!DeeSType_Check(argv[i]))
			goto create_inst;
	}

	/* Special case: `xxx(void)' constructs a function prototype with no arguments. */
	if (argc == 1 && argv[0] == DeeSType_AsObject(&DeeCVoid_Type))
		argc = 0;

	/* Use the default calling convention for constructing this function type. */
	return DeeCFunctionType_AsObject(DeeSType_CFunction(self, CC_DEFAULT, argc,
	                                                    (DeeSTypeObject **)argv,
	                                                    false));
create_inst:
	/* Construct a new instance. */
	return DeeObject_New(DeeSType_AsType(self), argc, argv);
}


INTDEF struct type_operator tpconst stype_operator_decls[(OPERATOR_STYPE_MAX - OPERATOR_STYPE_MIN) + 1];

PRIVATE DeeTypeObject *tpconst stype_mro[] = {
	&DeeType_Type,
	&DeeObject_Type,
	&DeeCallable_Type, /* Types can be called to invoke their constructor, so have them implement deemon.Callable. */
	NULL,
};

INTERN DeeTypeObject DeeSType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "StructuredType",
	/* .tp_doc      = */ DOC("[]->\n"
	                         "Construct new array types using @this type as item type\n"
	                         "\n"

	                         "call()->?GStructured\n"
	                         "call(args!)->?GStructured\n"
	                         "call(types!:?GStructuredType)->?GFunction\n"
	                         "Construct a new function type using this type as return type, "
	                         /**/ "or construct a new instance of @this ?GStructuredType"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeSTypeObject,
			/* tp_ctor:        */ &stype_init,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stype_fini,
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
	/* .tp_seq           = */ &stype_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ stype_methods,
	/* .tp_getsets       = */ stype_getsets,
	/* .tp_members       = */ stype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&stype_call,
	/* .tp_callable      = */ NULL,
	/* .tp_mro           = */ stype_mro,
	/* .tp_operators     = */ stype_operator_decls,
	/* .tp_operators_size= */ COMPILER_LENOF(stype_operator_decls)
};


PRIVATE NONNULL((1)) void DCALL
ptype_fini(DeePointerTypeObject *__restrict self) {
	DeeSTypeObject *orig = self->pt_orig;
	/* Delete the weak-link to the original type. */
	DeeSType_CacheLockWrite(orig);
	if (orig->st_pointer == self)
		orig->st_pointer = NULL;
	DeeSType_CacheLockEndWrite(orig);
	Dee_Decref(DeeSType_AsType(orig));
}

PRIVATE NONNULL((1, 2)) void DCALL
ptype_visit(DeePointerTypeObject *__restrict self, Dee_visit_t proc, void *arg) {
	ASSERTF(DeeObject_Check(DeeSType_AsType(self->pt_orig)),
	        "Missing base type for %p:%s",
	        self,
	        self->pt_base.st_base.tp_name);
	Dee_Visit(DeeSType_AsType(self->pt_orig));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ptype_repr(DeePointerTypeObject *__restrict self) {
	return DeeString_Newf("%r.ptr", self->pt_orig);
}

PRIVATE struct type_member tpconst ptype_members[] = {
	TYPE_MEMBER_CONST_DOC("ispointer", Dee_True, DOC_GET(ispointer_doc)),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT, offsetof(DeePointerTypeObject, pt_orig), "->?GStructuredType"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeePointerType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "PointerType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeePointerTypeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ptype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ptype_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ptype_visit,
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
	/* .tp_members       = */ ptype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


/* Assert that we can re-use some operators from `pointer_type'. */
STATIC_ASSERT(offsetof(DeePointerTypeObject, pt_orig) ==
              offsetof(DeeLValueTypeObject, lt_orig));


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ltype_sizeof(DeeLValueTypeObject *__restrict self) {
	size_t result = DeeSType_Sizeof(self->lt_orig);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ltype_repr(DeeLValueTypeObject *__restrict self) {
	return DeeString_Newf("%r.lvalue", self->lt_orig);
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_bytes_not_writable)(DeeBytesObject *__restrict UNUSED(bytes_ob)) {
	return DeeError_Throwf(&DeeError_BufferError,
	                       "The Bytes object is not writable");
}

PRIVATE WUNUSED NONNULL((1)) DREF struct lvalue_object *DCALL
ltype_frombytes(DeeLValueTypeObject *self, size_t argc, DeeObject *const *argv) {
	DREF struct lvalue_object *result;
	size_t type_size, data_size;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("frombytes", params: "
	DeeBytesObject *data;
	size_t offset = 0;
", docStringPrefix: "ltype");]]]*/
#define ltype_frombytes_params "data:?DBytes,offset=!0"
	struct {
		DeeBytesObject *data;
		size_t offset;
	} args;
	args.offset = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "frombytes", &args, &args.data, "o", _DeeArg_AsObject, &args.offset, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.data, &DeeBytes_Type))
		goto err;
	if (!DeeBytes_IsWritable(args.data))
		goto err_not_writable;
	type_size = DeeSType_Sizeof(self->lt_orig);
	data_size = DeeBytes_SIZE(args.data);
	if unlikely(OVERFLOW_UADD(data_size, args.offset, &data_size))
		goto err_bad_size;
	if unlikely(type_size > data_size)
		goto err_bad_size;
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto err;
	result->l_ptr.ptr = DeeBytes_DATA(args.data) + args.offset;
	DeeObject_Init(result, DeeLValueType_AsType(self));
	return result;
err_bad_size:
	data_size = DeeBytes_SIZE(args.data);
	if (OVERFLOW_USUB(data_size, args.offset, &data_size))
		data_size = 0;
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid bytes size: lvalue type `%r' has an "
	                "instance size of `%" PRFuSIZ "', but the given "
	                "`%" PRFuSIZ "'-large buffer at offset `%" PRFuSIZ "' "
	                "provides at most `%" PRFuSIZ " bytes'",
	                self, type_size, DeeBytes_SIZE(args.data),
	                args.offset, data_size);
err:
	return NULL;
err_not_writable:
	err_bytes_not_writable(args.data);
	goto err;
}

PRIVATE struct type_method tpconst ltype_methods[] = {
	TYPE_METHOD("frombytes", &ltype_frombytes,
	            "(" ltype_frombytes_params ")->?.\n"
	            "#tValueError{The given ${#data - offset} is less than ?#sizeof}"
	            "#tBufferError{The given @data is not writable (an lvalue view cannot be created)}"
	            "Construct an instance of @this structured type from @data"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst ltype_getsets[] = {
	TYPE_GETTER_F("sizeof", &ltype_sizeof, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns the size of the base of @this ?GLValueType in bytes"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ltype_members[] = {
	TYPE_MEMBER_CONST_DOC("islvalue", Dee_True, DOC_GET(islvalue_doc)),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT, offsetof(DeeLValueTypeObject, lt_orig), "->?GStructuredType"),
	TYPE_MEMBER_END
};

PRIVATE NONNULL((1)) void DCALL
ltype_fini(DeeLValueTypeObject *__restrict self) {
	DeeSTypeObject *orig = self->lt_orig;
	/* Delete the weak-link to the original type. */
	DeeSType_CacheLockWrite(orig);
	if (orig->st_lvalue == self)
		orig->st_lvalue = NULL;
	DeeSType_CacheLockEndWrite(orig);
	Dee_Decref(DeeSType_AsType(orig));
}


INTERN DeeTypeObject DeeLValueType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "LValueType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeLValueTypeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ltype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ltype_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ptype_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ltype_methods,
	/* .tp_getsets       = */ ltype_getsets,
	/* .tp_members       = */ ltype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
make_structured_name(DeeSTypeObject *__restrict self, char class_ch) {
	/* TODO: Special handling for function-pointers. */
	return DeeString_Newf("%s%c", self->st_base.tp_name, class_ch);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeePointerTypeObject *DCALL
pointertype_new(DeeSTypeObject *__restrict self) {
	DREF DeePointerTypeObject *result;
	DREF DeeStringObject *name;
	result = DeeGCObject_CALLOC(DeePointerTypeObject);
	if unlikely(!result)
		goto err;

	/* Create the name of the resulting type. */
	name = (DREF DeeStringObject *)make_structured_name(self, '*');
	if unlikely(!name)
		goto err_r;

	/* Store a reference to the pointed-to type. */
	Dee_Incref(DeeSType_AsType(self));
	Dee_Incref(DeePointerType_AsType(&DeePointer_Type));

	/* Initialize fields. */
	result->pt_orig                 = self; /* Inherit reference. */
	result->pt_base.st_sizeof       = sizeof(void *);
	result->pt_base.st_align        = CONFIG_CTYPES_ALIGNOF_POINTER;
	result->pt_base.st_base.tp_name = DeeString_STR(name); /* Inherit reference. */
	result->pt_base.st_base.tp_init.tp_alloc.tp_instance_size = sizeof(struct pointer_object);
	result->pt_base.st_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FNAMEOBJECT | TP_FHEAP | TP_FMOVEANY;
	result->pt_base.st_base.tp_base  = DeePointerType_AsType(&DeePointer_Type); /* Inherit reference. */
	result->pt_size                  = DeeSType_Sizeof(self);
#ifndef CONFIG_NO_CFUNCTION
	result->pt_base.st_ffitype = &ffi_type_pointer;
#endif /* !CONFIG_NO_CFUNCTION */

	/* Setup use of the proper math operators. */
	if (result->pt_size == 0 || result->pt_size == 1) {
		result->pt_base.st_math = &pointer_math1;
		result->pt_base.st_seq  = &pointer_seq1;
	} else {
		result->pt_base.st_math = &pointer_mathn;
		result->pt_base.st_seq  = &pointer_seqn;
	}

	/* Finish the pointer type. */
	DeeObject_Init(DeePointerType_AsType(result), &DeePointerType_Type);
	return DeeType_AsPointerType(DeeGC_TRACK(DeeTypeObject, DeePointerType_AsType(result)));
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeLValueTypeObject *DCALL
lvaluetype_new(DeeSTypeObject *__restrict self) {
	DREF DeeLValueTypeObject *result;
	DREF DeeStringObject *name;
	result = DeeGCObject_CALLOC(DeeLValueTypeObject);
	if unlikely(!result)
		goto err;

	/* Create the name of the resulting type. */
	name = (DREF DeeStringObject *)make_structured_name(self, '&');
	if unlikely(!name)
		goto err_r;

	/* Store a reference to the pointed-to type. */
	Dee_Incref(DeeSType_AsType(self));
	Dee_Incref(DeeLValueType_AsType(&DeeLValue_Type));

	/* Initialize fields. */
	result->lt_orig                 = self; /* Inherit reference. */
	result->lt_base.st_sizeof       = sizeof(void *);
	result->lt_base.st_align        = CONFIG_CTYPES_ALIGNOF_LVALUE;
	result->lt_base.st_base.tp_name = DeeString_STR(name); /* Inherit reference. */
	result->lt_base.st_base.tp_init.tp_alloc.tp_instance_size = sizeof(struct lvalue_object);
	result->lt_base.st_base.tp_flags = (TP_FTRUNCATE | TP_FVARIABLE | TP_FINHERITCTOR |
	                                    TP_FNAMEOBJECT | TP_FHEAP | TP_FMOVEANY);
	result->lt_base.st_base.tp_base = DeeLValueType_AsType(&DeeLValue_Type); /* Inherit reference. */
#ifndef CONFIG_NO_CFUNCTION
	result->lt_base.st_ffitype = &ffi_type_pointer;
#endif /* !CONFIG_NO_CFUNCTION */

	/* Initialize the lvalue-pointer of the resulting l-value type to point
	 * to itself, thus preventing weird types like l-value-to-l-value. */
	result->lt_base.st_lvalue = result;

	/* Finish the lvalue type. */
	DeeObject_Init(DeeLValueType_AsType(result), &DeeLValueType_Type);
	return DeeType_AsLValueType(DeeGC_TRACK(DeeTypeObject, DeeLValueType_AsType(result)));
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeePointerTypeObject *DCALL
DeeSType_Pointer(DeeSTypeObject *__restrict self) {
	DREF DeePointerTypeObject *result;
	ASSERT_OBJECT_TYPE(DeeSType_AsType(self), &DeeSType_Type);
	DeeSType_CacheLockRead(self);
	result = self->st_pointer;
	if (result && !Dee_IncrefIfNotZero(DeePointerType_AsType(result)))
		result = NULL;
	DeeSType_CacheLockEndRead(self);
	if (!result) {
		/* Lazily construct missing types. */
		result = pointertype_new(self);
		if likely(result) {
			DeeSType_CacheLockWrite(self);
			/* Check if the type was created due to race conditions. */
			if unlikely(self->st_pointer &&
			            Dee_IncrefIfNotZero(DeePointerType_AsType(self->st_pointer))) {
				DREF DeePointerTypeObject *new_result;
				new_result = self->st_pointer;
				DeeSType_CacheLockEndWrite(self);
				Dee_DecrefDokill(DeePointerType_AsType(result));
				return new_result;
			}
			self->st_pointer = result; /* Weakly referenced. */
			DeeSType_CacheLockEndWrite(self);
		}
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeLValueTypeObject *DCALL
DeeSType_LValue(DeeSTypeObject *__restrict self) {
	DREF DeeLValueTypeObject *result;
	ASSERT_OBJECT_TYPE(DeeSType_AsType(self), &DeeSType_Type);
	DeeSType_CacheLockRead(self);
	result = self->st_lvalue;
	if (result && !Dee_IncrefIfNotZero(DeeLValueType_AsType(result)))
		result = NULL;
	DeeSType_CacheLockEndRead(self);
	if (!result) {
		/* Lazily construct missing types. */
		result = lvaluetype_new(self);
		if likely(result) {
			DeeSType_CacheLockWrite(self);
			/* Check if the type was created due to race conditions. */
			if unlikely(self->st_lvalue &&
			            Dee_IncrefIfNotZero(DeeLValueType_AsType(self->st_lvalue))) {
				DREF DeeLValueTypeObject *new_result;
				new_result = self->st_lvalue;
				DeeSType_CacheLockEndWrite(self);
				Dee_DecrefDokill(DeeLValueType_AsType(result));
				return new_result;
			}
			self->st_lvalue = result; /* Weakly referenced. */
			DeeSType_CacheLockEndWrite(self);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeSTypeObject *DCALL
DeeSType_GetAlias(DeeTypeObject *__restrict self) {
	/* Map some builtin types to their structured counterparts. */
	if (DeeNone_Check(self) || self == &DeeNone_Type)
		return &DeeCVoid_Type;
	if (self == &DeeBool_Type)
		return &DeeCBool_Type;
	if (self == &DeeInt_Type)
		return &DeeCInt_Type;
	if (self == &DeeFloat_Type)
		return &DeeCDouble_Type;
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DeeSTypeObject *DCALL
DeeSType_Get(DeeObject *__restrict self) {
	DeeSTypeObject *result;
	/* Quick check: is it a structured type. */
	if (DeeSType_Check(self))
		return DeeType_AsSType((DeeTypeObject *)self);

	/* Map some builtin types to their structured counterparts. */
	result = DeeSType_GetAlias((DeeTypeObject *)self);
	if (result)
		return result;

	/* Throw a type-assertion failure error. */
	DeeObject_TypeAssertFailed(self, &DeeSType_Type);
	return NULL;
}

/* Same as `DeeSType_Get()', but also able to handle the
 * case where "self" is an *instance*, rather a some type. */
INTERN WUNUSED NONNULL((1)) DeeSTypeObject *DCALL
DeeSType_GetTypeOf(DeeObject *__restrict self) {
	DeeSTypeObject *result;
	/* Quick check: is it a structured type? */
	if (DeeSType_Check(self))
		return DeeType_AsSType((DeeTypeObject *)self);

	/* Quick check: is it a structured instance? */
	if (DeeStruct_Check(self))
		return DeeType_AsSType(Dee_TYPE(self));

	/* Map some builtin types to their structured counterparts. */
	result = DeeSType_GetAlias((DeeTypeObject *)self);
	if (result)
		return result;
	result = DeeSType_GetAlias(Dee_TYPE(self));
	if (result)
		return result;

	/* Throw a type-assertion failure error. */
	DeeObject_TypeAssertFailed(self, &DeeSType_Type);
	return NULL;
}


INTERN ATTR_COLD int DCALL
err_unimplemented_operator(DeeSTypeObject *__restrict tp, Dee_operator_t operator_name) {
	struct Dee_opinfo const *info;
	info = DeeTypeType_GetOperatorById(Dee_TYPE(DeeSType_AsType(tp)), operator_name);
	ASSERT_OBJECT(DeeSType_AsType(tp));
	ASSERT(DeeType_Check(DeeSType_AsType(tp)));
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Operator `%k." OPNAME("%s") "' is not implemented",
	                       tp, info ? info->oi_sname : Q3);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
struct_ctor(DeeObject *__restrict self) {
	DeeSTypeObject *tp_self = DeeType_AsSType(Dee_TYPE(self));
	while (!DeeSType_Check(tp_self))
		tp_self = DeeSType_Base(tp_self);

	/* ZERO-initialize by default. */
	bzero(DeeStruct_Data(self), DeeSType_Sizeof(tp_self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_copy(DeeObject *__restrict self,
            DeeObject *__restrict other) {
	DeeSTypeObject *tp_self = DeeType_AsSType(Dee_TYPE(self));
	while (!DeeSType_Check(tp_self))
		tp_self = DeeSType_Base(tp_self);

	/* Copy memory. */
	memcpy(DeeStruct_Data(self),
	       DeeStruct_Data(other),
	       DeeSType_Sizeof(tp_self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
struct_init(DeeObject *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeSTypeObject *tp_self;
	tp_self = DeeType_AsSType(Dee_TYPE(self));
	do {
		if (tp_self->st_init)
			return (*tp_self->st_init)(tp_self, DeeStruct_Data(self), argc, argv);
	} while (DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_INIT));
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_INIT);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_assign(DeeObject *self, DeeObject *some_object) {
	return DeeStruct_Assign(DeeType_AsSType(Dee_TYPE(self)),
	                        DeeStruct_Data(self), some_object);
}

#define DEFINE_UNARY_STRUCT_OPERATOR(Treturn, struct_xxx, DeeStruct_Xxx) \
	PRIVATE WUNUSED NONNULL((1)) Treturn DCALL                           \
	struct_xxx(DeeObject *__restrict self) {                             \
		return DeeStruct_Xxx(DeeType_AsSType(Dee_TYPE(self)),            \
		                     DeeStruct_Data(self));                      \
	}
#define DEFINE_BINARY_STRUCT_OPERATOR(Treturn, struct_xxx, DeeStruct_Xxx) \
	PRIVATE WUNUSED NONNULL((1, 2)) Treturn DCALL                         \
	struct_xxx(DeeObject *self, DeeObject *other) {                       \
		return DeeStruct_Xxx(DeeType_AsSType(Dee_TYPE(self)),             \
		                     DeeStruct_Data(self), other);                \
	}
#define DEFINE_TRINARY_STRUCT_OPERATOR(Treturn, struct_xxx, DeeStruct_Xxx) \
	PRIVATE WUNUSED NONNULL((1, 2, 3)) Treturn DCALL                       \
	struct_xxx(DeeObject *self, DeeObject *a, DeeObject *b) {              \
		return DeeStruct_Xxx(DeeType_AsSType(Dee_TYPE(self)),              \
		                     DeeStruct_Data(self), a, b);                  \
	}
#define DEFINE_UNARY_INPLACE_STRUCT_OPERATOR(Treturn, struct_xxx, DeeStruct_Xxx) \
	PRIVATE WUNUSED NONNULL((1)) Treturn DCALL                                   \
	struct_xxx(DeeObject **__restrict p_self) {                                  \
		DeeObject *self = *p_self;                                               \
		return DeeStruct_Xxx(DeeType_AsSType(Dee_TYPE(self)),                    \
		                     DeeStruct_Data(self));                              \
	}
#define DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(Treturn, struct_xxx, DeeStruct_Xxx) \
	PRIVATE WUNUSED NONNULL((1, 2)) Treturn DCALL                                 \
	struct_xxx(DeeObject **__restrict p_self, DeeObject *other) {                 \
		DeeObject *self = *p_self;                                                \
		return DeeStruct_Xxx(DeeType_AsSType(Dee_TYPE(self)),                     \
		                     DeeStruct_Data(self), other);                        \
	}
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_str, DeeStruct_Str)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_repr, DeeStruct_Repr)
DEFINE_UNARY_STRUCT_OPERATOR(int, struct_bool, DeeStruct_Bool)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
	return DeeStruct_Call(DeeType_AsSType(Dee_TYPE(self)),
	                      DeeStruct_Data(self), argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_int32(DeeObject *__restrict self, int32_t *__restrict result) {
	return DeeStruct_Int32(DeeType_AsSType(Dee_TYPE(self)),
	                       DeeStruct_Data(self), result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_int64(DeeObject *__restrict self, int64_t *__restrict result) {
	return DeeStruct_Int64(DeeType_AsSType(Dee_TYPE(self)),
	                       DeeStruct_Data(self), result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_double(DeeObject *__restrict self, double *__restrict result) {
	return DeeStruct_Double(DeeType_AsSType(Dee_TYPE(self)),
	                        DeeStruct_Data(self), result);
}

DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_int, DeeStruct_Int)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_inv, DeeStruct_Inv)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_pos, DeeStruct_Pos)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_neg, DeeStruct_Neg)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_add, DeeStruct_Add)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_sub, DeeStruct_Sub)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_mul, DeeStruct_Mul)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_div, DeeStruct_Div)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_mod, DeeStruct_Mod)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_shl, DeeStruct_Shl)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_shr, DeeStruct_Shr)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_and, DeeStruct_And)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_or, DeeStruct_Or)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_xor, DeeStruct_Xor)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_pow, DeeStruct_Pow)
DEFINE_UNARY_INPLACE_STRUCT_OPERATOR(int, struct_inc, DeeStruct_Inc)
DEFINE_UNARY_INPLACE_STRUCT_OPERATOR(int, struct_dec, DeeStruct_Dec)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_add, DeeStruct_InplaceAdd)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_sub, DeeStruct_InplaceSub)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_mul, DeeStruct_InplaceMul)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_div, DeeStruct_InplaceDiv)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_mod, DeeStruct_InplaceMod)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_shl, DeeStruct_InplaceShl)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_shr, DeeStruct_InplaceShr)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_and, DeeStruct_InplaceAnd)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_or, DeeStruct_InplaceOr)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_xor, DeeStruct_InplaceXor)
DEFINE_BINARY_INPLACE_STRUCT_OPERATOR(int, struct_inplace_pow, DeeStruct_InplacePow)
DEFINE_UNARY_STRUCT_OPERATOR(Dee_hash_t, struct_hash, DeeStruct_Hash)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_eq, DeeStruct_Eq)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_ne, DeeStruct_Ne)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_lo, DeeStruct_Lo)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_le, DeeStruct_Le)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_gr, DeeStruct_Gr)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_ge, DeeStruct_Ge)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_iter, DeeStruct_Iter)
DEFINE_UNARY_STRUCT_OPERATOR(DREF DeeObject *, struct_size, DeeStruct_GetSize)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_contains, DeeStruct_Contains)
DEFINE_BINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_getitem, DeeStruct_GetItem)
DEFINE_BINARY_STRUCT_OPERATOR(int, struct_delitem, DeeStruct_DelItem)
DEFINE_TRINARY_STRUCT_OPERATOR(int, struct_setitem, DeeStruct_SetItem)
DEFINE_TRINARY_STRUCT_OPERATOR(DREF DeeObject *, struct_getrange, DeeStruct_GetRange)
DEFINE_TRINARY_STRUCT_OPERATOR(int, struct_delrange, DeeStruct_DelRange)

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
struct_setrange(DeeObject *self, DeeObject *begin,
                DeeObject *end, DeeObject *value) {
	return DeeStruct_SetRange(DeeType_AsSType(Dee_TYPE(self)),
	                          DeeStruct_Data(self), begin, end, value);
}

/* If attribute isn't defined, must use `DeeObject_TGenericGetAttr()' and friends! */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
struct_getattr(DeeObject *self, DeeObject *attr) {
	DREF DeeObject *result;
	result = DeeStruct_GetAttr(DeeType_AsSType(Dee_TYPE(self)),
	                           DeeStruct_Data(self), attr);
	if (result == ITER_DONE)
		result = DeeObject_GenericGetAttr(self, attr);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_delattr(DeeObject *self, DeeObject *attr) {
	int result;
	result = DeeStruct_DelAttr(DeeType_AsSType(Dee_TYPE(self)),
	                           DeeStruct_Data(self), attr);
	if (result == -2)
		result = DeeObject_GenericDelAttr(self, attr);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
struct_setattr(DeeObject *self, DeeObject *attr, DeeObject *value) {
	int result;
	result = DeeStruct_SetAttr(DeeType_AsSType(Dee_TYPE(self)),
	                           DeeStruct_Data(self), attr, value);
	if (result == -2)
		result = DeeObject_GenericSetAttr(self, attr, value);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
struct_iterattr(DeeTypeObject *tp_self, DeeObject *UNUSED(self),
                struct Dee_attriter *iterbuf, size_t bufsize,
                struct Dee_attrhint const *__restrict hint) {
	size_t req;
	struct Dee_attriterchain_builder builder;
	Dee_attriterchain_builder_init(&builder, iterbuf, bufsize);
	req = DeeStruct_IterAttr(DeeType_AsSType(tp_self),
	                         Dee_attriterchain_builder_getiterbuf(&builder),
	                         Dee_attriterchain_builder_getbufsize(&builder),
	                         hint);
	if unlikely(req == (size_t)-1)
		goto err_builder;
	Dee_attriterchain_builder_consume(&builder, req);
	req = DeeObject_TGenericIterAttr(tp_self,
	                                 Dee_attriterchain_builder_getiterbuf(&builder),
	                                 Dee_attriterchain_builder_getbufsize(&builder),
	                                 hint);
	if unlikely(req == (size_t)-1)
		goto err_builder;
	Dee_attriterchain_builder_consume(&builder, req);
	return Dee_attriterchain_builder_pack(&builder);
err_builder:
	Dee_attriterchain_builder_fini(&builder);
	return (size_t)-1;
}

#undef DEFINE_BINARY_INPLACE_STRUCT_OPERATOR
#undef DEFINE_UNARY_INPLACE_STRUCT_OPERATOR
#undef DEFINE_TRINARY_STRUCT_OPERATOR
#undef DEFINE_BINARY_STRUCT_OPERATOR
#undef DEFINE_UNARY_STRUCT_OPERATOR

PRIVATE struct type_math struct_math = {
	/* .tp_int32       = */ &struct_int32,
	/* .tp_int64       = */ &struct_int64,
	/* .tp_double      = */ &struct_double,
	/* .tp_int         = */ &struct_int,
	/* .tp_inv         = */ &struct_inv,
	/* .tp_pos         = */ &struct_pos,
	/* .tp_neg         = */ &struct_neg,
	/* .tp_add         = */ &struct_add,
	/* .tp_sub         = */ &struct_sub,
	/* .tp_mul         = */ &struct_mul,
	/* .tp_div         = */ &struct_div,
	/* .tp_mod         = */ &struct_mod,
	/* .tp_shl         = */ &struct_shl,
	/* .tp_shr         = */ &struct_shr,
	/* .tp_and         = */ &struct_and,
	/* .tp_or          = */ &struct_or,
	/* .tp_xor         = */ &struct_xor,
	/* .tp_pow         = */ &struct_pow,
	/* .tp_inc         = */ &struct_inc,
	/* .tp_dec         = */ &struct_dec,
	/* .tp_inplace_add = */ &struct_inplace_add,
	/* .tp_inplace_sub = */ &struct_inplace_sub,
	/* .tp_inplace_mul = */ &struct_inplace_mul,
	/* .tp_inplace_div = */ &struct_inplace_div,
	/* .tp_inplace_mod = */ &struct_inplace_mod,
	/* .tp_inplace_shl = */ &struct_inplace_shl,
	/* .tp_inplace_shr = */ &struct_inplace_shr,
	/* .tp_inplace_and = */ &struct_inplace_and,
	/* .tp_inplace_or  = */ &struct_inplace_or,
	/* .tp_inplace_xor = */ &struct_inplace_xor,
	/* .tp_inplace_pow = */ &struct_inplace_pow
};

PRIVATE struct type_cmp struct_cmp = {
	/* .tp_hash          = */ &struct_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ &struct_eq,
	/* .tp_ne            = */ &struct_ne,
	/* .tp_lo            = */ &struct_lo,
	/* .tp_le            = */ &struct_le,
	/* .tp_gr            = */ &struct_gr,
	/* .tp_ge            = */ &struct_ge
};

PRIVATE struct type_seq struct_seq = {
	/* .tp_iter     = */ &struct_iter,
	/* .tp_sizeob   = */ &struct_size,
	/* .tp_contains = */ &struct_contains,
	/* .tp_getitem  = */ &struct_getitem,
	/* .tp_delitem  = */ &struct_delitem,
	/* .tp_setitem  = */ &struct_setitem,
	/* .tp_getrange = */ &struct_getrange,
	/* .tp_delrange = */ &struct_delrange,
	/* .tp_setrange = */ &struct_setrange
};

PRIVATE struct type_attr struct_attr = {
	/* .tp_getattr  = */ &struct_getattr,
	/* .tp_delattr  = */ &struct_delattr,
	/* .tp_setattr  = */ &struct_setattr,
	/* .tp_iterattr = */ &struct_iterattr,
	/* TODO: Also define all of the string-based accessors! */
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
struct_getbuf(DeeObject *__restrict self,
              DeeBuffer *__restrict info,
              unsigned int UNUSED(flags)) {
	info->bb_base = DeeStruct_Data(self);
	info->bb_size = DeeStruct_Size(self);
	return 0;
}

PRIVATE struct type_buffer struct_buffer = {
	/* .tp_getbuf       = */ &struct_getbuf,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_sizeof(DeeObject *__restrict self) {
	size_t result = DeeStruct_Size(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_alignof(DeeObject *__restrict self) {
	size_t result = DeeStruct_Align(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_ref(DeeObject *__restrict self) {
	/* Construct a new pointer directed at the data of this structured object. */
	return DeePointer_NewFor(DeeType_AsSType(Dee_TYPE(self)),
	                         DeeStruct_Data(self));
}



PRIVATE struct type_getset tpconst struct_getsets[] = {
	TYPE_GETTER_AB_F("sizeof", &struct_sizeof, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Returns the size of @this ?GStructured object"),
	TYPE_GETTER_AB_F("alignof", &struct_alignof, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Returns the alignment of @this ?GStructured object"),
	TYPE_GETTER_AB("ref", &struct_ref,
	               "->?GPointer\n"
	               "Returns a pointer to @this ?GStructured object"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeBytesObject *DCALL
struct_tobytes(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeSTypeObject *stype = DeeType_AsSType(Dee_TYPE(self));
	size_t type_size = DeeSType_Sizeof(stype);
	size_t data_size;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tobytes", params: "
	DeeBytesObject *data = NULL;
	size_t offset = 0;
", docStringPrefix: "struct");]]]*/
#define struct_tobytes_params "data?:?DBytes,offset=!0"
	struct {
		DeeBytesObject *data;
		size_t offset;
	} args;
	args.data = NULL;
	args.offset = 0;
	if (DeeArg_UnpackStruct(argc, argv, "|o" UNPuSIZ ":tobytes", &args))
		goto err;
/*[[[end]]]*/

	/* When no explicit buffer was given, return a writable view for the data of `self' */
	if (args.data == NULL) {
		return (DREF DeeBytesObject *)DeeBytes_NewView(self, DeeObject_DATA(self),
		                                               type_size, Dee_BUFFER_FWRITABLE);
	}

	/* Otherwise, copy bytes of `self' into the provided bytes-buffer at the specified offset. */
	if (DeeObject_AssertTypeExact(args.data, &DeeBytes_Type))
		goto err;
	if unlikely(!DeeBytes_IsWritable(args.data))
		goto err_not_writable;
	data_size = DeeBytes_SIZE(args.data);
	if unlikely(OVERFLOW_USUB(data_size, args.offset, &data_size))
		goto err_bad_size;
	if unlikely(data_size < type_size)
		goto err_bad_size;
	memcpy(DeeBytes_DATA(args.data) + args.offset,
	       DeeObject_DATA(self), type_size);

	return_reference_(args.data);
err_bad_size:
	data_size = DeeBytes_SIZE(args.data);
	if (OVERFLOW_USUB(data_size, args.offset, &data_size))
		data_size = 0;
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid bytes size: structured type `%r' has an "
	                "instance size of `%" PRFuSIZ "', but the given "
	                "`%" PRFuSIZ "'-large buffer at offset `%" PRFuSIZ "' "
	                "provides at most `%" PRFuSIZ " bytes'",
	                self, type_size, DeeBytes_SIZE(args.data),
	                args.offset, data_size);
err:
	return NULL;
err_not_writable:
	err_bytes_not_writable(args.data);
	goto err;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_ref_func(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__ref__");
	return struct_ref(self);
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_method tpconst struct_methods[] = {
	TYPE_METHOD("tobytes", &struct_tobytes, struct_tobytes_doc),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Methods for backwards-compatibility with deemon 100+ */
	TYPE_METHOD("__ref__", &struct_ref_func,
	            "->?GPointer\n"
	            "Deprecated alias for ?#ref"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};

/* This type needs to implement _all_ operators
 * to forward them to their structured counterparts! */
INTERN DeeSTypeObject DeeStructured_Type = {
	/* .st_base = */ {
		OBJECT_HEAD_INIT(&DeeSType_Type),
		/* .tp_name     = */ "Structured",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeObject_Type,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
				/* TODO: Use a custom allocator that takes alignment into account. */
				/* T:              */ DeeObject,
				/* tp_ctor:        */ &struct_ctor,
				/* tp_copy_ctor:   */ &struct_copy,
				/* tp_deep_ctor:   */ &struct_copy,
				/* tp_any_ctor:    */ &struct_init,
				/* tp_any_ctor_kw: */ NULL,
				/* tp_serialize:   */ NULL /* TODO */
			),
			/* .tp_dtor        = */ NULL,
			/* .tp_assign      = */ &struct_assign,
			/* .tp_move_assign = */ &struct_assign
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ &struct_str,
			/* .tp_repr = */ &struct_repr,
			/* .tp_bool = */ &struct_bool
		},
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ &struct_math,
		/* .tp_cmp           = */ &struct_cmp,
		/* .tp_seq           = */ &struct_seq,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ &struct_attr,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ &struct_buffer,
#ifndef CONFIG_NO_DEEMON_100_COMPAT
		/* .tp_methods       = */ struct_methods,
#else /* !CONFIG_NO_DEEMON_100_COMPAT */
		/* .tp_methods       = */ NULL,
#endif /* CONFIG_NO_DEEMON_100_COMPAT */
		/* .tp_getsets       = */ struct_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL,
		/* .tp_method_hints  = */ NULL,
		/* .tp_call          = */ &struct_call,
	},
#ifndef CONFIG_NO_THREADS
	/* .st_cachelock = */ Dee_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .st_pointer  = */ &DeePointer_Type,
	/* .st_lvalue   = */ &DeeLValue_Type,
	/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
	/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
	/* .st_ffitype  = */ &ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
	/* .st_sizeof   = */ 0,
	/* .st_align    = */ 0,
	/* .st_init     = */ NULL,
	/* .st_assign   = */ NULL,
	/* .st_cast     = */ {
		/* .st_str  = */ NULL,
		/* .st_repr = */ NULL,
		/* .st_bool = */ NULL
	},
	/* .st_call     = */ NULL,
	/* .st_math     = */ NULL,
	/* .st_cmp      = */ NULL,
	/* .st_seq      = */ NULL,
	/* .st_attr     = */ NULL
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Ref(DeeObject *__restrict self) {
	if (DeeObject_AssertType(self, DeeSType_AsType(&DeeStructured_Type)))
		goto err;
	if (DeeLValue_Check(self)) {
		/* Special case: Must reference the data originally designated through the l-value. */
		struct lvalue_object *me = (struct lvalue_object *)self;
		DeeLValueTypeObject *tp = DeeType_AsLValueType(Dee_TYPE(me));
		return DeePointer_NewFor(tp->lt_orig, me->l_ptr.ptr);
	}

	/* Lookup the required pointer type. */
	return DeePointer_NewFor(DeeType_AsSType(Dee_TYPE(self)), DeeStruct_Data(self));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Deref(DeeObject *__restrict self) {
	if (DeePointer_Check(self)) {
		/* Regular pointer. */
		struct pointer_object *me = (struct pointer_object *)self;
		DeePointerTypeObject *tp = DeeType_AsPointerType(Dee_TYPE(me));
		return DeeLValue_NewFor(tp->pt_orig, me->p_ptr.ptr);
	}
	if (DeeLValue_Check(self)) {
		/* Check for LValue-to-pointer */
		struct lvalue_object *me = (struct lvalue_object *)self;
		DeeLValueTypeObject *tp = DeeType_AsLValueType(Dee_TYPE(me));
		DeePointerTypeObject *tp_base = DeeSType_AsPointerType(tp->lt_orig);
		if (DeePointerType_Check(tp_base)) {
			void *resaddr, **p_resaddr = (void **)me->l_ptr.ptr;
			/* Dereference this pointer. */
			CTYPES_FAULTPROTECT(resaddr = *p_resaddr, return NULL);
			return DeeLValue_NewFor(tp_base->pt_orig, resaddr);
		}
	}

	/* Throw an error. */
	DeeObject_TypeAssertFailed(self, DeePointerType_AsType(&DeePointer_Type));
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_New(DeePointerTypeObject *pointer_type,
               void *pointer_value) {
#ifdef __OPTIMIZE_SIZE__
	Dee_Incref(DeePointerType_AsType(pointer_type));
	return DeePointer_NewInherited(pointer_type, pointer_value);
#else /* __OPTIMIZE_SIZE__ */
	struct pointer_object *result;
	ASSERT(DeePointerType_Check(pointer_type));

	/* Allocate a new pointer object. */
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto done;

	/* Initialize the new pointer object. */
	DeeObject_Init(result, DeePointerType_AsType(pointer_type));
	result->p_ptr.ptr = pointer_value;
done:
	return Dee_AsObject(result);
#endif /* !__OPTIMIZE_SIZE__ */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_NewInherited(/*inherit(always)*/ DREF DeePointerTypeObject *pointer_type,
                        void *pointer_value) {
	struct pointer_object *result;
	ASSERT(DeePointerType_Check(pointer_type));

	/* Allocate a new pointer object. */
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err;

	/* Initialize the new pointer object. */
	DeeObject_InitInherited(result, DeePointerType_AsType(pointer_type));
	result->p_ptr.ptr = pointer_value;
	return Dee_AsObject(result);
err:
	Dee_Decref(DeePointerType_AsType(pointer_type));
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeePointer_NewFor(DeeSTypeObject *pointer_base_type,
                  void *pointer_value) {
	DREF DeePointerTypeObject *ptr_type;
	ptr_type = DeeSType_Pointer(pointer_base_type);
	if unlikely(!ptr_type)
		goto err;
	return DeePointer_NewInherited(ptr_type, pointer_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeLValue_NewInherited(/*inherit(always)*/ DREF DeeLValueTypeObject *lvalue_type,
                       void *pointer_value) {
	struct lvalue_object *result;
	ASSERT(DeeLValueType_Check(lvalue_type));

	/* Allocate a new lvalue object. */
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto err;

	/* Initialize the new lvalue object. */
	DeeObject_InitInherited(result, DeeLValueType_AsType(lvalue_type));
	result->l_ptr.ptr = pointer_value;
	return Dee_AsObject(result);
err:
	Dee_Decref(DeeLValueType_AsType(lvalue_type));
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeLValue_NewFor(DeeSTypeObject *lvalue_base_type, void *pointer_value) {
	DREF DeeLValueTypeObject *lval_type;
	lval_type = DeeSType_LValue(lvalue_base_type);
	if unlikely(!lval_type)
		goto err;
	return DeeLValue_NewInherited(lval_type, pointer_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeLValue_NewForInherited(/*inherit(always)*/ DREF DeeSTypeObject *lvalue_base_type, void *pointer_value) {
	DREF DeeObject *result = DeeLValue_NewFor(lvalue_base_type, pointer_value);
	Dee_Decref(DeeSType_AsType(lvalue_base_type));
	return result;
}






INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_Assign(DeeSTypeObject *tp_self,
                 void *self, DeeObject *value) {
	if (tp_self->st_assign ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_ASSIGN))
		return (*tp_self->st_assign)(tp_self, self, value);
	if (DeeObject_InstanceOf(value, DeeSType_AsType(tp_self))) {
		/* Copy-assign. */
		byte_t *dst = (byte_t *)self;
		byte_t *src = (byte_t *)DeeStruct_Data(value);
		size_t size = DeeSType_Sizeof(tp_self);
		CTYPES_FAULTPROTECT(memcpy(dst, src, size), return -1);
		return 0;
	}
	if (DeeNone_Check(value)) {
		/* Clear memory. */
		byte_t *dst = (byte_t *)self;
		size_t size = DeeSType_Sizeof(tp_self);
		CTYPES_FAULTPROTECT(bzero(dst, size), return -1);
		return 0;
	}
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_ASSIGN);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeStruct_Str(DeeSTypeObject *tp_self, void *self) {
	if likely(tp_self->st_cast.st_str ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_STR))
		return (*tp_self->st_cast.st_str)(tp_self, self);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_STR);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeStruct_Repr(DeeSTypeObject *tp_self, void *self) {
	if likely(tp_self->st_cast.st_repr ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_REPR))
		return (*tp_self->st_cast.st_repr)(tp_self, self);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_REPR);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeStruct_Bool(DeeSTypeObject *tp_self, void *self) {
	if likely(tp_self->st_cast.st_bool ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_BOOL))
		return (*tp_self->st_cast.st_bool)(tp_self, self);
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_BOOL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeStruct_Call(DeeSTypeObject *tp_self,
               void *self, size_t argc,
               DeeObject *const *argv) {
	if likely(tp_self->st_call ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_CALL))
		return (*tp_self->st_call)(tp_self, self, argc, argv);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_CALL);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_Int32(DeeSTypeObject *tp_self, void *self, int32_t *result) {
	if likely((tp_self->st_math && tp_self->st_math->st_int32) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_INT))
		return (*tp_self->st_math->st_int32)(tp_self, self, result);
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_INT);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_Int64(DeeSTypeObject *tp_self, void *self, int64_t *result) {
	if likely((tp_self->st_math && tp_self->st_math->st_int64) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_INT))
		return (*tp_self->st_math->st_int64)(tp_self, self, result);
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_INT);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_Double(DeeSTypeObject *tp_self, void *self, double *result) {
	if likely((tp_self->st_math && tp_self->st_math->st_double) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_INT))
		return (*tp_self->st_math->st_double)(tp_self, self, result);
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_INT);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeStruct_Int(DeeSTypeObject *tp_self, void *self) {
	if likely((tp_self->st_math && tp_self->st_math->st_int) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_INT))
		return (*tp_self->st_math->st_int)(tp_self, self);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_INT);
	return NULL;
}

#define DEFINE_UNARY_MATH_OPERATOR(Treturn, error_result, DeeStruct_Xxx, st_xxx, STYPE_OPERATOR_XXX) \
	INTERN WUNUSED NONNULL((1)) Treturn DCALL                                                        \
	DeeStruct_Xxx(DeeSTypeObject *tp_self, void *self) {                                             \
		if likely((tp_self->st_math && tp_self->st_math->st_xxx) ||                                  \
		          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_XXX))             \
			return (*tp_self->st_math->st_xxx)(tp_self, self);                                       \
		err_unimplemented_operator(tp_self, STYPE_OPERATOR_XXX);                                     \
		return error_result;                                                                         \
	}
#define DEFINE_BINARY_MATH_OPERATOR(Treturn, error_result, DeeStruct_Xxx, st_xxx, STYPE_OPERATOR_XXX) \
	INTERN WUNUSED NONNULL((1, 3)) Treturn DCALL                                                      \
	DeeStruct_Xxx(DeeSTypeObject *tp_self, void *self, DeeObject *other) {                            \
		if likely((tp_self->st_math && tp_self->st_math->st_xxx) ||                                   \
		          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_XXX))              \
			return (*tp_self->st_math->st_xxx)(tp_self, self, other);                                 \
		err_unimplemented_operator(tp_self, STYPE_OPERATOR_XXX);                                      \
		return error_result;                                                                          \
	}
DEFINE_UNARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Inv, st_inv, STYPE_OPERATOR_INV)
DEFINE_UNARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Pos, st_pos, STYPE_OPERATOR_POS)
DEFINE_UNARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Neg, st_neg, STYPE_OPERATOR_NEG)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Add, st_add, STYPE_OPERATOR_ADD)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Sub, st_sub, STYPE_OPERATOR_SUB)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Mul, st_mul, STYPE_OPERATOR_MUL)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Div, st_div, STYPE_OPERATOR_DIV)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Mod, st_mod, STYPE_OPERATOR_MOD)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Shl, st_shl, STYPE_OPERATOR_SHL)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Shr, st_shr, STYPE_OPERATOR_SHR)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_And, st_and, STYPE_OPERATOR_AND)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Or, st_or, STYPE_OPERATOR_OR)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Xor, st_xor, STYPE_OPERATOR_XOR)
DEFINE_BINARY_MATH_OPERATOR(DREF DeeObject *, NULL, DeeStruct_Pow, st_pow, STYPE_OPERATOR_POW)
DEFINE_UNARY_MATH_OPERATOR(int, -1, DeeStruct_Inc, st_inc, STYPE_OPERATOR_INC)
DEFINE_UNARY_MATH_OPERATOR(int, -1, DeeStruct_Dec, st_dec, STYPE_OPERATOR_DEC)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceAdd, st_inplace_add, STYPE_OPERATOR_INPLACE_ADD)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceSub, st_inplace_sub, STYPE_OPERATOR_INPLACE_SUB)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceMul, st_inplace_mul, STYPE_OPERATOR_INPLACE_MUL)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceDiv, st_inplace_div, STYPE_OPERATOR_INPLACE_DIV)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceMod, st_inplace_mod, STYPE_OPERATOR_INPLACE_MOD)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceShl, st_inplace_shl, STYPE_OPERATOR_INPLACE_SHL)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceShr, st_inplace_shr, STYPE_OPERATOR_INPLACE_SHR)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceAnd, st_inplace_and, STYPE_OPERATOR_INPLACE_AND)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceOr, st_inplace_or, STYPE_OPERATOR_INPLACE_OR)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplaceXor, st_inplace_xor, STYPE_OPERATOR_INPLACE_XOR)
DEFINE_BINARY_MATH_OPERATOR(int, -1, DeeStruct_InplacePow, st_inplace_pow, STYPE_OPERATOR_INPLACE_POW)
#undef DEFINE_BINARY_MATH_OPERATOR
#undef DEFINE_UNARY_MATH_OPERATOR

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeStruct_Hash(DeeSTypeObject *tp_self, void *self) {
	return Dee_HashPtr(self, DeeSType_Sizeof(tp_self));
}


INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Eq(DeeSTypeObject *tp_self, void *self, DeeObject *some_object) {
	if ((tp_self->st_cmp && tp_self->st_cmp->st_eq) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_EQ))
		return (*tp_self->st_cmp->st_eq)(tp_self, self, some_object);

	/* Compare object data. */
	if (tp_self == DeeType_AsSType(Dee_TYPE(some_object)))
		return_bool(bcmp(self, DeeStruct_Data(some_object), DeeSType_Sizeof(tp_self)) == 0);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_EQ);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Ne(DeeSTypeObject *tp_self, void *self, DeeObject *some_object) {
	if ((tp_self->st_cmp && tp_self->st_cmp->st_ne) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_NE))
		return (*tp_self->st_cmp->st_ne)(tp_self, self, some_object);

	/* Compare object data. */
	if (tp_self == DeeType_AsSType(Dee_TYPE(some_object)))
		return_bool(bcmp(self, DeeStruct_Data(some_object), DeeSType_Sizeof(tp_self)) != 0);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_NE);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Lo(DeeSTypeObject *tp_self, void *self, DeeObject *some_object) {
	if ((tp_self->st_cmp && tp_self->st_cmp->st_lo) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_LO))
		return (*tp_self->st_cmp->st_lo)(tp_self, self, some_object);

	/* Compare object data. */
	if (tp_self == DeeType_AsSType(Dee_TYPE(some_object)))
		return_bool(memcmp(self, DeeStruct_Data(some_object), DeeSType_Sizeof(tp_self)) < 0);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_LO);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Le(DeeSTypeObject *tp_self, void *self, DeeObject *some_object) {
	if ((tp_self->st_cmp && tp_self->st_cmp->st_le) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_LE))
		return (*tp_self->st_cmp->st_le)(tp_self, self, some_object);

	/* Compare object data. */
	if (tp_self == DeeType_AsSType(Dee_TYPE(some_object)))
		return_bool(memcmp(self, DeeStruct_Data(some_object), DeeSType_Sizeof(tp_self)) <= 0);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_LE);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Gr(DeeSTypeObject *tp_self, void *self, DeeObject *some_object) {
	if ((tp_self->st_cmp && tp_self->st_cmp->st_gr) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_GR))
		return (*tp_self->st_cmp->st_gr)(tp_self, self, some_object);

	/* Compare object data. */
	if (tp_self == DeeType_AsSType(Dee_TYPE(some_object)))
		return_bool(memcmp(self, DeeStruct_Data(some_object), DeeSType_Sizeof(tp_self)) > 0);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_GR);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Ge(DeeSTypeObject *tp_self, void *self, DeeObject *some_object) {
	if ((tp_self->st_cmp && tp_self->st_cmp->st_ge) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_GE))
		return (*tp_self->st_cmp->st_ge)(tp_self, self, some_object);

	/* Compare object data. */
	if (tp_self == DeeType_AsSType(Dee_TYPE(some_object)))
		return_bool(memcmp(self, DeeStruct_Data(some_object), DeeSType_Sizeof(tp_self)) >= 0);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_GE);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeStruct_Iter(DeeSTypeObject *tp_self, void *self) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_iter_self) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_ITER))
		return (*tp_self->st_seq->st_iter_self)(tp_self, self);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_ITER);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeStruct_GetSize(DeeSTypeObject *tp_self, void *self) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_size) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_SIZE))
		return (*tp_self->st_seq->st_size)(tp_self, self);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_SIZE);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_Contains(DeeSTypeObject *tp_self,
                   void *self, DeeObject *some_object) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_contains) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_CONTAINS))
		return (*tp_self->st_seq->st_contains)(tp_self, self, some_object);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_CONTAINS);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_GetItem(DeeSTypeObject *tp_self, void *self, DeeObject *index) {
	DREF DeeObject *result, *new_result;
	if likely((tp_self->st_seq && tp_self->st_seq->st_get) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_GETITEM))
		return (*tp_self->st_seq->st_get)(tp_self, self, index);

	/* Fallback: Implement getitem as `ind(add)' --> `foo[2]' same as `*(foo + 2)' */
	result = DeeStruct_Add(tp_self, self, index);
	if unlikely(!result)
		goto err;
	new_result = DeeObject_Deref(result);
	Dee_Decref(result);
	return new_result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_DelItem(DeeSTypeObject *tp_self, void *self, DeeObject *index) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_del) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_DELITEM))
		return (*tp_self->st_seq->st_del)(tp_self, self, index);

	/* Fallback: Do a setitem operation with `none' */
	return DeeStruct_SetItem(tp_self, self, index, Dee_None);
}

INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL
DeeStruct_SetItem(DeeSTypeObject *tp_self, void *self,
                  DeeObject *index, DeeObject *value) {
	DREF DeeObject *temp, *temp2;
	int result;
	if likely((tp_self->st_seq && tp_self->st_seq->st_set) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_SETITEM))
		return (*tp_self->st_seq->st_set)(tp_self, self, index, value);

	/* Fallback: Implement setitem as
	 * `ind(add) := value' --> `foo[2] = value'
	 *              same as `*(foo + 2) := value' */
	temp = DeeStruct_Add(tp_self, self, index);
	if unlikely(!temp)
		goto err;
	temp2 = DeeObject_Deref(temp);
	Dee_Decref(temp);
	if unlikely(!temp2)
		goto err;
	result = DeeObject_Assign(temp2, value);
	Dee_Decref(temp2);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3, 4)) DREF DeeObject *DCALL
DeeStruct_GetRange(DeeSTypeObject *tp_self, void *self,
                   DeeObject *begin, DeeObject *end) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_range_get) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_GETRANGE))
		return (*tp_self->st_seq->st_range_get)(tp_self, self, begin, end);
	err_unimplemented_operator(tp_self, STYPE_OPERATOR_GETRANGE);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL
DeeStruct_DelRange(DeeSTypeObject *tp_self, void *self,
                   DeeObject *begin, DeeObject *end) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_range_del) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_DELRANGE))
		return (*tp_self->st_seq->st_range_del)(tp_self, self, begin, end);
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_DELRANGE);
}

INTERN WUNUSED NONNULL((1, 3, 4, 5)) int DCALL
DeeStruct_SetRange(DeeSTypeObject *tp_self, void *self,
                   DeeObject *begin, DeeObject *end, DeeObject *value) {
	if likely((tp_self->st_seq && tp_self->st_seq->st_range_set) ||
	          DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_SETRANGE))
		return (*tp_self->st_seq->st_range_set)(tp_self, self, begin, end, value);
	return err_unimplemented_operator(tp_self, STYPE_OPERATOR_SETRANGE);
}

/* Get a struct attribute (excluding generic attributes)
 * @return: * :        Attribute value
 * @return: ITER_DONE: No such attribute
 * @return: NULL:      Error */
INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeStruct_GetAttr(DeeSTypeObject *tp_self, void *self, DeeObject *name) {
	if ((tp_self->st_attr && tp_self->st_attr->st_getattr) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_GETATTR))
		return (*tp_self->st_attr->st_getattr)(tp_self, self, name);
	return ITER_DONE;
}

/* Delete a struct attribute (excluding generic attributes)
 * @return: 0 : Success
 * @return: -1: Error
 * @return: -2: No such attribute */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeStruct_DelAttr(DeeSTypeObject *tp_self, void *self, DeeObject *name) {
	if ((tp_self->st_attr && tp_self->st_attr->st_delattr) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_DELATTR))
		return (*tp_self->st_attr->st_delattr)(tp_self, self, name);
	return -2;
}

/* Set a struct attribute (excluding generic attributes)
 * @return: 0 : Success
 * @return: -1: Error
 * @return: -2: No such attribute */
INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL
DeeStruct_SetAttr(DeeSTypeObject *tp_self, void *self,
                  DeeObject *name, DeeObject *value) {
	if ((tp_self->st_attr && tp_self->st_attr->st_setattr) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_SETATTR))
		return (*tp_self->st_attr->st_setattr)(tp_self, self, name, value);
	return -2;
}

/* Enumerate struct attributes (excluding generic attributes) */
INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
DeeStruct_IterAttr(DeeSTypeObject *__restrict tp_self,
                   struct Dee_attriter *iterbuf, size_t bufsize,
                   struct Dee_attrhint const *__restrict hint) {
	if ((tp_self->st_attr && tp_self->st_attr->st_iterattr) ||
	    DeeType_InheritOperator(DeeSType_AsType(tp_self), STYPE_OPERATOR_ENUMATTR))
		return (*tp_self->st_attr->st_iterattr)(tp_self, iterbuf, bufsize, hint);
	return Dee_attriter_initempty(iterbuf, bufsize);
}





PRIVATE NONNULL((1)) void DCALL
atype_fini(DeeArrayTypeObject *__restrict self) {
	DeeSTypeObject *orig = self->at_orig;
	/* Delete the weak-link to the original type. */
	DeeSType_CacheLockWrite(orig);
	ASSERT(LIST_ISBOUND(self, at_chain));
	LIST_REMOVE(self, at_chain);
	DeeSType_CacheLockEndWrite(orig);
	Dee_Decref(DeeSType_AsType(orig));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
atype_repr(DeeArrayTypeObject *__restrict self) {
	return DeeString_Newf("%r[%" PRFuSIZ "]", self->at_orig, self->at_count);
}

PRIVATE struct type_member tpconst atype_members[] = {
	TYPE_MEMBER_CONST_DOC("isarray", Dee_True, DOC_GET(isarray_doc)),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT, offsetof(DeeArrayTypeObject, at_orig),
	                      "->?GStructuredType"),
	TYPE_MEMBER_FIELD("size", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DeeArrayTypeObject, at_count)),
	TYPE_MEMBER_END
};

/* Assert that we can re-use some operators from `pointer_type'. */
STATIC_ASSERT(offsetof(DeeArrayTypeObject, at_orig) ==
              offsetof(DeeLValueTypeObject, lt_orig));

INTERN DeeTypeObject DeeArrayType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "ArrayType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeArrayTypeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&atype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&atype_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ptype_visit,
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
	/* .tp_members       = */ atype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


#ifdef CONFIG_NO_CFUNCTION
/* Throw a NotImplemented error explaining that cfunctions have been disabled. */
INTERN ATTR_COLD void DCALL err_no_cfunction(void) {
	DeeError_Throwf(&DeeError_NotImplemented,
	                "ctypes was built without C-functions being enabled");
}
#endif /* CONFIG_NO_CFUNCTION */


#ifndef CONFIG_NO_CFUNCTION
PRIVATE NONNULL((1)) void DCALL
ftype_fini(DeeCFunctionTypeObject *__restrict self) {
	DeeSTypeObject *orig = self->ft_orig;
	/* Delete the weak-link to the original type. */
	DeeSType_CacheLockWrite(orig);
	ASSERT(LIST_ISBOUND(self, ft_chain));
	LIST_REMOVE(self, ft_chain);
	DeeSType_CacheLockEndWrite(orig);

	Dee_Decref(DeeSType_AsType(orig));
	Dee_Decrefv((DeeObject **)self->ft_argv, self->ft_argc);
	Dee_Free(self->ft_argv);
	Dee_Free(self->ft_ffi_arg_type_v);
}

PRIVATE NONNULL((1, 2)) void DCALL
ftype_visit(DeeCFunctionTypeObject *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(DeeSType_AsType(self->ft_orig));
	Dee_Visitv((DeeObject **)self->ft_argv, self->ft_argc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ftype_repr(DeeCFunctionTypeObject *__restrict self) {
	size_t i;
	ctypes_cc_t cc;
	struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
	bool is_first_arg = true;
	if unlikely(Dee_unicode_printer_printobjectrepr(&printer, DeeSType_AsObject(self->ft_orig)) < 0)
		goto err;
	if unlikely((self->ft_cc & CC_FVARARGS ? Dee_UNICODE_PRINTER_PRINT(&printer, ".vfunc(")
	                                       : Dee_UNICODE_PRINTER_PRINT(&printer, ".func(")) < 0)
		goto err;
	cc = (ctypes_cc_t)((unsigned int)self->ft_cc & ~CC_FVARARGS);
	if (cc != CC_DEFAULT) {
		char const *cc_name = cc_getname(cc);
		if (cc_name) {
			if unlikely(Dee_unicode_printer_printf(&printer, "%q", cc_name) < 0)
				goto err;
			is_first_arg = false;
		}
	}
	for (i = 0; i < self->ft_argc; ++i) {
		if (!is_first_arg) {
			if unlikely(Dee_UNICODE_PRINTER_PRINT(&printer, ", ") < 0)
				goto err;
		}
		is_first_arg = false;
		if unlikely(Dee_unicode_printer_printobjectrepr(&printer, DeeSType_AsObject(self->ft_argv[i])) < 0)
			goto err;
	}
	if unlikely(Dee_unicode_printer_putascii(&printer, ')'))
		goto err;
	return Dee_unicode_printer_pack(&printer);
err:
	Dee_unicode_printer_fini(&printer);
	return NULL;
}


/* Assert that we can re-use some operators from `pointer_type'. */
STATIC_ASSERT(offsetof(DeeArrayTypeObject, at_orig) ==
              offsetof(DeeLValueTypeObject, lt_orig));
#endif /* !CONFIG_NO_CFUNCTION */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ftype_args(DeeCFunctionTypeObject *__restrict self) {
#ifdef CONFIG_NO_CFUNCTION
	(void)self;
	err_no_cfunction();
	return NULL;
#else /* CONFIG_NO_CFUNCTION */
	/* Use a read-only shared-reference vector to grant the user access to arguments. */
	return DeeRefVector_NewReadonly((DeeObject *)DeeCFunctionType_AsType(self),
	                                self->ft_argc, (DeeObject **)self->ft_argv);
#endif /* !CONFIG_NO_CFUNCTION */
}

PRIVATE struct type_getset tpconst ftype_getsets[] = {
	TYPE_GETTER_AB("args", &ftype_args,
	               "->?S?GStructuredType\n"
	               "Returns an immutable sequence describing "
	               /**/ "the argument types used by this function"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ftype_members[] = {
	TYPE_MEMBER_CONST_DOC("isfunction", Dee_True, DOC_GET(isfunction_doc)),
#ifndef CONFIG_NO_CFUNCTION
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT, offsetof(DeeCFunctionTypeObject, ft_orig),
	                      "->?GStructuredType"),
#endif /* !CONFIG_NO_CFUNCTION */
	TYPE_MEMBER_END
};



#ifdef CONFIG_NO_CFUNCTION
#define CFUNCTION_OPERATOR(x) NULL
#else /* CONFIG_NO_CFUNCTION */
#define CFUNCTION_OPERATOR(x) x
#endif /* !CONFIG_NO_CFUNCTION */

INTERN DeeTypeObject DeeCFunctionType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "FunctionType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeCFunctionTypeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ CFUNCTION_OPERATOR((void (DCALL *)(DeeObject *__restrict))&ftype_fini),
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ CFUNCTION_OPERATOR((DeeObject *(DCALL *)(DeeObject *__restrict))&ftype_repr),
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ CFUNCTION_OPERATOR((void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ftype_visit),
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
	/* .tp_getsets       = */ ftype_getsets,
	/* .tp_members       = */ ftype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

#undef CFUNCTION_OPERATOR

DECL_END


#endif /* !GUARD_DEX_CTYPES_CORE_C */
