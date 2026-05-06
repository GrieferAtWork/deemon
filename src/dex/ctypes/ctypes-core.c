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
#ifndef GUARD_DEX_CTYPES_CTYPES_CORE_C
#define GUARD_DEX_CTYPES_CTYPES_CORE_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>            /* DeeSlab_*, Dee_*alloc*, Dee_CollectMemory, Dee_Free, Dee_Freea, Dee_ReleaseSystemMemory, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>              /* DeeArg_Unpack*, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>             /* Dee_False, Dee_True */
#include <deemon/bytes.h>            /* DeeBytes* */
#include <deemon/callable.h>         /* DeeCallable_Type */
#include <deemon/error-rt.h>         /* DeeRT_ATTRIBUTE_ACCESS_DEL, DeeRT_ATTRIBUTE_ACCESS_SET, DeeRT_ErrIndexOutOfBounds, DeeRT_ErrRestrictedAttrCStr */
#include <deemon/error.h>            /* DeeError_*, Dee_ERROR_PRINT_DOHANDLE */
#include <deemon/format.h>           /* DeeFormat_Printf, PRFuSIZ, PRFxPTR */
#include <deemon/gc.h>               /* DeeGCObject_CALLOC, DeeGCObject_Callocc, DeeGCObject_FREE, DeeGCObject_Free, DeeGCObject_TryReallocc, DeeGC_TRACK, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/int.h>              /* DeeInt_NewPtrdiff, DeeInt_NewSize */
#include <deemon/method-hints.h>     /* DeeObject_InvokeMethodHint */
#include <deemon/mro.h>              /* Dee_attrdesc, Dee_attrhint, Dee_attriter, Dee_attrspec */
#include <deemon/none.h>             /* DeeNone_Check, return_none */
#include <deemon/numeric.h>          /* DeeNumeric_Type */
#include <deemon/object.h>           /* ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DeeObject_*, Dee_BUFFER_FREADONLY, Dee_Decref*, Dee_Incref, Dee_IncrefIfNotZero, Dee_Movrefv, Dee_XDecref, Dee_XIncref, return_reference_ */
#include <deemon/seq.h>              /* DeeRefVector_NewReadonly, DeeSeq_Type */
#include <deemon/serial.h>           /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>           /* DeeString*, Dee_string_object, Dee_unicode_printer*, STRING_ERROR_FREPLAC, WSTR_LENGTH */
#include <deemon/system-features.h>  /* DeeSystem_DEFINE_strcmp, bcmp, bzero, memcpy, strlen */
#include <deemon/type.h>             /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_*, Dee_Visit, Dee_XVisit, Dee_visit_t, METHOD_FCONSTCALL, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/types.h>            /* DREF, DeeObject, DeeTypeObject, DeeType_Implements, Dee_AsObject, Dee_TYPE, Dee_formatprinter_t, Dee_funptr_t, Dee_hash_t, Dee_ssize_t, OBJECT_HEAD_INIT, _Dee_HashSelectC */
#include <deemon/util/hash.h>        /* Dee_HashPointer, Dee_HashPtr */
#include <deemon/util/lock.h>        /* Dee_atomic_rwlock_cinit */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_FOREACH, Dee_SLAB_CHUNKSIZE_MIN */

#include <hybrid/align.h>           /* CEIL_ALIGN, IS_ALIGNED, IS_POWER_OF_TWO */
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/overflow.h>        /* OVERFLOW_* */
#include <hybrid/sequence/list.h>   /* LIST_* */
#include <hybrid/typecore.h>        /* __BYTE_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* intN_t, uintN_t, uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("ind");
print define_Dee_HashStr("ptr");
]]]*/
#define Dee_HashStr__ind _Dee_HashSelectC(0x36f5c430, 0x18549bb43b297a6f)
#define Dee_HashStr__ptr _Dee_HashSelectC(0x4170660c, 0x9d2f54779ea875bf)
/*[[[end]]]*/

#define isind(attr)                                                                            \
	((DeeString_HASH(attr) == Dee_HashStr__ind || DeeString_HASH(attr) == (Dee_hash_t) - 1) && \
	 DeeString_EQUALS_ASCII(attr, "ind"))
#define isind_string_hash(attr, hash) \
	((hash) == Dee_HashStr__ind && strcmp(attr, "ind") == 0)
#define isind_string_len_hash(attr, attrlen, hash) \
	((hash) == Dee_HashStr__ind && attrlen == 3 && strcmp(attr, "ind") == 0)

#define isptr(attr)                                                                            \
	((DeeString_HASH(attr) == Dee_HashStr__ptr || DeeString_HASH(attr) == (Dee_hash_t) - 1) && \
	 DeeString_EQUALS_ASCII(attr, "ptr"))
#define isptr_string_hash(attr, hash) \
	((hash) == Dee_HashStr__ptr && strcmp(attr, "ptr") == 0)
#define isptr_string_len_hash(attr, attrlen, hash) \
	((hash) == Dee_HashStr__ptr && attrlen == 3 && strcmp(attr, "ptr") == 0)



/* Optimize "tp_instance_size" into slab allocators (if possible) */
PRIVATE NONNULL((1)) void DCALL
DeeType_OptimizeAlloc(DeeTypeObject *__restrict self) {
#ifdef Dee_SLAB_CHUNKSIZE_MIN
	size_t instance_size = self->tp_init.tp_alloc.tp_instance_size;
#define CHECK_ALLOCATOR(N, _)                                 \
	if (instance_size == N) {                                 \
		self->tp_init.tp_alloc.tp_alloc = &DeeSlab_Malloc##N; \
		self->tp_init.tp_alloc.tp_free  = &DeeSlab_Free##N;   \
	} else
	Dee_SLAB_CHUNKSIZE_FOREACH(CHECK_ALLOCATOR, ~)
#undef CHECK_ALLOCATOR
	{}
#endif /* Dee_SLAB_CHUNKSIZE_MIN */
	(void)self;
}



/************************************************************************/
/* C TYPE                                                               */
/************************************************************************/

DOC_DEF(ispointer_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GCType is a ?GPointerType");
DOC_DEF(islvalue_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GCType is an ?GLValueType");
DOC_DEF(isarray_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GCType is an ?GArrayType");
DOC_DEF(isfunction_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GCType is a ?GFunctionType");
DOC_DEF(isstruct_doc,
        "->?Dbool\n"
        "Returns ?t if @this ?GCType is a ?GStructType");
DOC_DEF(tobytes_doc,
        "->?DBytes\n"
        "(data:?DBytes,offset=!0)->?DBytes\n"
        "#tValueError{The given ${#data - offset} is less than ?#sizeof}"
        "#tBufferError{The given @data is not writable}"
        "Retrieve the underlying bytes of @this struct object instance");

PRIVATE ATTR_COLD NONNULL((1)) int
(DCALL err_bytes_not_writable)(DeeBytesObject *__restrict bytes_ob) {
	(void)bytes_ob;
	return DeeError_Throwf(&DeeError_BufferError,
	                       "The Bytes object is not writable");
}




PRIVATE NONNULL((1)) void DCALL
ctype_fini(CType *__restrict self) {
	ASSERTF(self->ct_pointer == NULL, "Should have kept us alive");
	ASSERTF(self->ct_lvalue == NULL, "Should have kept us alive");
	ASSERTF(self->ct_arrays.sa_size == 0, "Should have kept us alive");
#ifndef CONFIG_NO_CFUNCTION
	ASSERTF(self->ct_functions.sf_size == 0, "Should have kept us alive");
#endif /* !CONFIG_NO_CFUNCTION */

	/* Free array/function type caches. */
	Dee_Free(self->ct_arrays.sa_list);
#ifndef CONFIG_NO_CFUNCTION
	if (self->ct_ffitype != &ffi_type_pointer)
		Dee_Free(self->ct_ffitype);
	Dee_Free(self->ct_functions.sf_list);
#endif /* !CONFIG_NO_CFUNCTION */

	/* Prevent "type_fini" from freeing these... */
	self->ct_base.tp_callable = NULL;
	self->ct_base.tp_iterator = NULL;
	self->ct_base.tp_math     = NULL;
	self->ct_base.tp_cmp      = NULL;
	self->ct_base.tp_seq      = NULL;
	self->ct_base.tp_with     = NULL;
	self->ct_base.tp_attr     = NULL;
	self->ct_base.tp_mro      = NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF CFunctionType *DCALL
ctype_dofunc(CType *return_type, size_t argc,
             DeeObject *const *argv, ctypes_cc_t cc_flags) {
	size_t i;
	ctypes_cc_t cc;
	CType **argv_types;
	DREF CFunctionType *result;
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
	argv_types = (CType **)Dee_Mallocac(argc, sizeof(CType *));
	if unlikely(!argv_types)
		goto err;

	/* Translate argument types. */
	for (i = 0; i < argc; ++i) {
		argv_types[i] = CType_Of(argv[i]);
		if unlikely(!argv_types[i])
			goto err_argv_types;
	}

	/* Lookup the associated C-function type while inheriting the argument vector. */
	result = CFunctionType_Of(return_type, cc, argc, argv_types);
	Dee_Freea(argv_types);
	return result;
err_argv_types:
	Dee_Freea(argv_types);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF CFunctionType *DCALL
ctype_func(CType *self, size_t argc, DeeObject *const *argv) {
	return ctype_dofunc(self, argc, argv, (ctypes_cc_t)0);
}

PRIVATE WUNUSED NONNULL((1)) DREF CFunctionType *DCALL
ctype_vfunc(CType *self, size_t argc, DeeObject *const *argv) {
	return ctype_dofunc(self, argc, argv, CC_FVARARGS);
}

PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL
ctype_frombytes(CType *self, size_t argc, DeeObject *const *argv) {
	DREF CObject *result;
	size_t type_size, data_size;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("frombytes", params: "
	DeeBytesObject *data;
	size_t offset = 0;
", docStringPrefix: "ctype");]]]*/
#define ctype_frombytes_params "data:?DBytes,offset=!0"
	struct {
		DeeBytesObject *data;
		size_t offset;
	} args;
	args.offset = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "frombytes", &args, &args.data, "o", _DeeArg_AsObject, &args.offset, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.data, &DeeBytes_Type))
		goto err;
	type_size = CType_Sizeof(self);
	data_size = DeeBytes_SIZE(args.data);
	if unlikely(OVERFLOW_UADD(data_size, args.offset, &data_size))
		goto err_bad_size;
	if unlikely(type_size > data_size)
		goto err_bad_size;
	ASSERTF(!CType_IsCFunctionType(self),
	        "Function types should have sizeof=SIZE_MAX, which "
	        "should have been caught by 'type_size > data_size' "
	        "combined with the fact that a single Bytes object "
	        "can't possibly have a size that spans all of memory. I "
	        "mean for starters: what'd be the address of this string?");
	result = CType_AllocInstance(self);
	if unlikely(!result)
		goto err;
	CObject_Init(result, self);
	return (*CType_Operators(self)->co_initobject)(result,
	                                               DeeBytes_DATA(args.data) +
	                                               args.offset);
err_bad_size:
	data_size = DeeBytes_SIZE(args.data);
	if (OVERFLOW_USUB(data_size, args.offset, &data_size))
		data_size = 0;
	DeeError_Throwf(&DeeError_ValueError,
	                //"Invalid bytes size: structued type `%r' has an "
	                "instance size of `%" PRFuSIZ "', but the given "
	                "`%" PRFuSIZ "'-large buffer at offset `%" PRFuSIZ "' "
	                "provides at most `%" PRFuSIZ " bytes'",
	                self, type_size, DeeBytes_SIZE(args.data),
	                args.offset, data_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
CType_PrintCRepr_impl(CType *__restrict self,
                      Dee_formatprinter_t printer, void *arg) {
	return CType_PrintCRepr(self, printer, arg, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ctype_typename(CType *self, size_t argc, DeeObject *const *argv) {
	struct Dee_unicode_printer printer;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("typename", params: "
	char const *varname = \"\";
", docStringPrefix: "ctype");]]]*/
#define ctype_typename_params "varname=!P{}"
	struct {
		char const *varname;
	} args;
	args.varname = "";
	if (DeeArg_UnpackStruct(argc, argv, "|s:typename", &args))
		goto err;
/*[[[end]]]*/
	Dee_unicode_printer_init(&printer);
	if (CType_PrintCRepr(self, &Dee_unicode_printer_print, &printer, args.varname) < 0)
		goto err_printer;
	return Dee_unicode_printer_pack(&printer);
err_printer:
	Dee_unicode_printer_fini(&printer);
err:
	return NULL;
}


PRIVATE struct type_method tpconst ctype_methods[] = {
	TYPE_METHOD_F("func", &ctype_func, METHOD_FCONSTCALL,
	              "(types!:?DType)->?GFunctionType\n"
	              "(cc:?Dstring,types!:?DType)->?GFunctionType\n"
	              "#tValueError{The given @cc is unknown, or not supported by the host}"
	              "#pcc{The name of the calling convention}"
	              "Construct a new function prototype, using @types as argument, @this "
	              /**/ "as return type, and @cc as calling convention\n"
	              "Note that unlike ?#{op:call}, certain types from the ?Mdeemon core "
	              /**/ "are also accepted as argument types, such as ?Dbool inplace of ?Gbool"),
	TYPE_METHOD_F("vfunc", &ctype_vfunc, METHOD_FCONSTCALL,
	              "(types!:?DType)->?GFunctionType\n"
	              "(cc:?Dstring,types!:?DType)->?GFunctionType\n"
	              "#tValueError{The given @cc is unknown, or not supported by the host}"
	              "#pcc{The name of the calling convention}"
	              "Same as ?#func, but enable support for varargs"),
	TYPE_METHOD("frombytes", &ctype_frombytes,
	            "(" ctype_frombytes_params ")->?.\n"
	            "#tValueError{The given ${#data - offset} is less than ?#sizeof}"
	            "Construct an instance of @this structured type from @data"),
	TYPE_METHOD_F("typename", &ctype_typename, METHOD_FCONSTCALL,
	              "(" ctype_typename_params ")->?Dstring\n"
	              "Same as ?#{op:str}, but allows you to specify the name of a "
	              /**/ "variable that should appear to be typed according to @this"),

	//TYPE_METHOD("is_pointer", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_lvalue", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_structured", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_struct", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_array", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	//TYPE_METHOD("is_foreign_function", &type_is_return_false, "->?Dbool\nDeprecated (always returns ?f)"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst ctype_getsets[] = {
	TYPE_GETTER_AB_F("ptr", &CPointerType_Of, METHOD_FCONSTCALL,
	                 "->?GPointerType\n"
	                 "Returns the pointer type for @this ?GCType"),
	TYPE_GETTER_AB_F("lvalue", &CLValueType_Of, METHOD_FCONSTCALL,
	                 "->?GLValueType\n"
	                 "Returns the l-value type for @this ?GCType"),

	TYPE_GETTER_AB_F("pointer", &CPointerType_Of, METHOD_FCONSTCALL,
	                 "->?GPointerType\n"
	                 "Deprecated alias for ?#ptr"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ctype_members[] = {
	TYPE_MEMBER_FIELD_DOC("sizeof", STRUCT_CONST | STRUCT_SIZE_T, offsetof(CType, ct_sizeof),
	                      "Returns the size of @this ?GCType in bytes"),
	TYPE_MEMBER_FIELD_DOC("alignof", STRUCT_CONST | STRUCT_SIZE_T, offsetof(CType, ct_alignof),
	                      "Returns the alignment of @this ?GCType in bytes"),
	TYPE_MEMBER_CONST_DOC("isarray", Dee_False, DOC_GET(isarray_doc)),
	TYPE_MEMBER_CONST_DOC("isstruct", Dee_False, DOC_GET(isstruct_doc)),
	TYPE_MEMBER_CONST_DOC("isfunction", Dee_False, DOC_GET(isfunction_doc)),
	TYPE_MEMBER_CONST_DOC("ispointer", Dee_False, DOC_GET(ispointer_doc)),
	TYPE_MEMBER_CONST_DOC("islvalue", Dee_False, DOC_GET(islvalue_doc)),
	TYPE_MEMBER_END
};



PRIVATE struct type_seq ctype_seq = {
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
	/* .tp_getitem_index = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&CArrayType_Of,
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ctype_call(CType *self, size_t argc, DeeObject *const *argv) {
	size_t i;
	DREF CFunctionType *result;

	/* Create a new instance, or create a new function-type. */
	if (!argc)
		goto create_inst;
	for (i = 0; i < argc; ++i) {
		if (!Object_IsCType(argv[i]))
			goto create_inst;
	}

	/* Special case: `xxx(void)' constructs a function prototype with no arguments. */
	if (argc == 1 && Object_AsCType(argv[0]) == &CVoid_Type)
		argc = 0;

	/* Use the default calling convention for constructing this function type. */
	result = CFunctionType_Of(self, CC_DEFAULT, argc, (CType *const *)argv);
	return Dee_AsObject(CFunctionType_AsType(result));
create_inst:
	/* Construct a new instance. */
	return DeeObject_New(CType_AsType(self), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ctype_call_kw(CType *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	if unlikely (!kw)
		return ctype_call(self, argc, argv);
	return DeeObject_NewKw(CType_AsType(self), argc, argv, kw);
}

PRIVATE struct type_callable ctype_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&ctype_call_kw,
};


PRIVATE DeeTypeObject *tpconst ctype_mro[] = {
	&DeeType_Type,
	/* Types can be called to invoke their constructor,
	 * so have them implement deemon.Callable. */
	&DeeCallable_Type,
	&DeeObject_Type,
	NULL,
};

INTERN DeeTypeObject CType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "CType",
	/* .tp_doc      = */ DOC("[]->\n"
	                         "Construct new array types using @this type as item type\n"
	                         "\n"

	                         "str->\n"
	                         "Returns the C-representation of this type (s.a. ?#typename)\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the deemon-representation of this type\n"
	                         "\n"

	                         "call()->?GStructured\n"
	                         "call(args!)->?GStructured\n"
	                         "call(types!:?GCType)->?GFunction\n"
	                         "Construct a new function type using this type as return type, "
	                         /**/ "or construct a new instance of @this ?GCType"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CType,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* XXX: Support me (probably impossible since lazily allocated
			                            *      singletons (which is what CType objects are) can't be
			                            *      serialized, since their address isn't fixed) */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ctype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&CType_PrintCRepr_impl,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&CType_PrintDRepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ctype_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ctype_methods,
	/* .tp_getsets       = */ ctype_getsets,
	/* .tp_members       = */ ctype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&ctype_call,
	/* .tp_callable      = */ &ctype_callable,
	/* .tp_mro           = */ ctype_mro
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
cobject_ctor(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	bzero(CObject_Data(self), CType_Sizeof(tp_self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_copy(CObject *__restrict self, CObject *__restrict other) {
	CType *tp_self = Dee_TYPE(self);
	memcpy(CObject_Data(self), CObject_Data(other), CType_Sizeof(tp_self));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cobject_init_kw(CObject *__restrict self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_initwith)(tp_self, CObject_Data(self), argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_assign(CObject *self, DeeObject *value) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_assign)(tp_self, CObject_Data(self), value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_serialize(CObject *__restrict self,
                  DeeSerial *__restrict writer,
                  Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(CObject, field))
	CObject *out = DeeSerial_Addr2Mem(writer, addr, CObject);
	size_t instance_size = CType_Sizeof(Dee_TYPE(self));
	memcpy(CObject_Data(out), CObject_Data(self), instance_size);
	return 0;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cobject_bool(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_bool)(tp_self, CObject_Data(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cobject_print(CObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_printcrepr)(tp_self, CObject_Data(self), printer, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cobject_printrepr(CObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_printdrepr)(tp_self, CObject_Data(self), printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
cobject_hash(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	return Dee_HashPtr(CObject_Data(self), CType_Sizeof(tp_self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_compare(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_compare)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE struct type_cmp cobject_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&cobject_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&cobject_compare,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cobject_compare,
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_int32(CObject *__restrict self, int32_t *__restrict p_result) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_int32)(tp_self, CObject_Data(self), p_result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_int64(CObject *__restrict self, int64_t *__restrict p_result) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_int64)(tp_self, CObject_Data(self), p_result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cobject_double(CObject *__restrict self, double *__restrict p_result) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_double)(tp_self, CObject_Data(self), p_result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cobject_int(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_int)(tp_self, CObject_Data(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cobject_inv(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_inv)(tp_self, CObject_Data(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cobject_pos(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_pos)(tp_self, CObject_Data(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cobject_neg(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_neg)(tp_self, CObject_Data(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_add(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_add)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_sub(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_sub)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_mul(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_mul)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_div(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_div)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_mod(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_mod)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_shl(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_shl)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_shr(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_shr)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_and(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_and)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_or(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_or)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_xor(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_xor)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cobject_pow(CObject *lhs, DeeObject *rhs) {
	CType *tp_self = Dee_TYPE(lhs);
	struct ctype_operators const *ops = CType_Operators(tp_self);
	return (*ops->co_pow)(tp_self, CObject_Data(lhs), rhs);
}

PRIVATE struct type_math cobject_math = {
	/* .tp_int32  = */ (int(DCALL *)(DeeObject *__restrict, int32_t *__restrict))&cobject_int32,
	/* .tp_int64  = */ (int(DCALL *)(DeeObject *__restrict, int64_t *__restrict))&cobject_int64,
	/* .tp_double = */ (int(DCALL *)(DeeObject *__restrict, double *__restrict))&cobject_double,
	/* .tp_int    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_int,
	/* .tp_inv    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_inv,
	/* .tp_pos    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_pos,
	/* .tp_neg    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_neg,
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_add,
	/* .tp_sub    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_sub,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_mul,
	/* .tp_div    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_div,
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_mod,
	/* .tp_shl    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_shl,
	/* .tp_shr    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_shr,
	/* .tp_and    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_and,
	/* .tp_or     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_or,
	/* .tp_xor    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_xor,
	/* .tp_pow    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_pow,
	/* Notice how all the inline operators are missing -- that's intentional! */
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cobject_native_sizeof(CObject *__restrict self) {
	CType *tp_self = Dee_TYPE(self);
	return DeeInt_NewSize(offsetof(CObject, co_data) +
	                      CType_Sizeof(tp_self));
}

PRIVATE struct type_getset tpconst cobject_getsets[] = {
	TYPE_GETTER_AB("__sizeof__", &cobject_native_sizeof,
	               "->?Dint\n"
	               "Returns the heap-size of a raw R-Value instance such as this"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeBytesObject *DCALL
cobject_tobytes(CObject *self, size_t argc, DeeObject *const *argv) {
	CType *tp_self = Dee_TYPE(self);
	size_t type_size = CType_Sizeof(tp_self);
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
	DeeArg_UnpackStruct0Or1XOr2X(err, argc, argv, "tobytes", &args, &args.data, "o", _DeeArg_AsObject, &args.offset, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/

	/* When no explicit buffer was given, return a writable view for the data of `self' */
	if (args.data == NULL) {
		return (DREF DeeBytesObject *)DeeBytes_NewView(Dee_AsObject(self), CObject_Data(self),
		                                               type_size, Dee_BUFFER_FREADONLY);
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
	       CObject_Data(self), type_size);

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

PRIVATE struct type_method tpconst cobject_methods[] = {
	TYPE_METHOD("tobytes", &cobject_tobytes, DOC_GET(tobytes_doc)),
	TYPE_METHOD_END
};

INTERN CType AbstractCObject_Type = {
	/* .ct_base = */ {
		OBJECT_HEAD_INIT(&CType_Type),
		/* .tp_name     = */ "CObject",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeObject_Type,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ CVoid,
				/* tp_ctor:        */ &cobject_ctor,
				/* tp_copy_ctor:   */ &cobject_copy,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &cobject_init_kw,
				/* tp_serialize:   */ &cobject_serialize
			),
			/* .tp_dtor        = */ NULL,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
			/* TODO: Custom "tp_new" to satisfy extended type alignments */
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
		},
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ &cobject_math,
		/* .tp_cmp           = */ &cobject_cmp,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ cobject_methods,
		/* .tp_getsets       = */ cobject_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL,
		/* .tp_method_hints  = */ NULL,
		/* .tp_call          = */ NULL,
		/* .tp_callable      = */ NULL,
		/* .tp_mro           = */ NULL
	},
	CTYPE_INIT_COMMON_EX(
		/* ct_sizeof:    */ 0,
		/* ct_alignof:   */ 1,
		/* ct_operators: */ &cvoid_operators,
		/* ct_ffitype:   */ &ffi_type_void,
		/* ct_pointer:   */ &AbstractCPointer_Type,
		/* ct_lvalue:    */ &AbstractCLValue_Type
	)
};









/************************************************************************/
/* ARRAY TYPE                                                           */
/************************************************************************/

/* Return a pointer to the `index' element (no bounds checking done) */
INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CArray_PlusOffset(CArray *__restrict self, ptrdiff_t index) {
	DREF CPointerType *item_pointer_type;
	CArrayType *tp_self = Dee_TYPE(self);
	union pointer res_cvalue;
	index *= CArrayType_Stride(tp_self);
	res_cvalue.ptr = CArray_Items(self);
	res_cvalue.sint += index;
	item_pointer_type = CPointerType_Of(CArrayType_PointedToType(tp_self));
	if unlikely(!item_pointer_type)
		goto err;
	Dee_Incref(self);
	return CPointer_NewExInherited(item_pointer_type,
	                               res_cvalue.ptr,
	                               Dee_AsObject(self));
err:
	return NULL;
}

/* Return an LValue to the `index' element (no bounds checking done) */
INTERN WUNUSED NONNULL((1)) DREF CLValue *DCALL
CArray_GetItem(CArray *__restrict self, ptrdiff_t index) {
	DREF CLValue *result;
	DREF CLValueType *item_lvalue_type;
	CArrayType *tp_self = Dee_TYPE(self);
	union pointer res_cvalue;
	index *= CArrayType_Stride(tp_self);
	res_cvalue.ptr = CArray_Items(self);
	res_cvalue.sint += index;
	item_lvalue_type = CLValueType_Of(CArrayType_PointedToType(tp_self));
	if unlikely(!item_lvalue_type)
		goto err;
	result = CLValue_Alloc();
	if unlikely(!result)
		goto err_item_lvalue_type;
	result->cl_value = res_cvalue;
	Dee_Incref(self);
	result->cl_owner = Dee_AsObject(self);
	CLValue_InitInherited(result, item_lvalue_type);
	return result;
err_item_lvalue_type:
	Dee_Decref_unlikely(CLValueType_AsType(item_lvalue_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF CPointer *DCALL
carray_add(CArray *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue;
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue))
		goto err;
	return CArray_PlusOffset(lhs, rhs_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
carray_sub(CArray *lhs, DeeObject *rhs) {
	CArrayType *tp_self = Dee_TYPE(lhs);
	ptrdiff_t rhs_cvalue_int;
	union pointer rhs_cvalue;
	CType *rhs_pointer_base;
	int status = DeeObject_TryAsGenericPointer(rhs, &rhs_pointer_base, &rhs_cvalue);
	if (status <= 0) {
		ptrdiff_t result;
		union pointer lhs_cvalue;
		lhs_cvalue.ptr = CArray_Items(lhs);
		if unlikely(status < 0)
			goto err;
		if (CArrayType_PointedToType(tp_self) != rhs_pointer_base) {
			DREF CPointerType *lhs_pointer_type;
			DREF CPointerType *rhs_pointer_type;
			lhs_pointer_type = CPointerType_Of(CArrayType_PointedToType(tp_self));
			if unlikely(!lhs_pointer_type)
				goto err;
			rhs_pointer_type = CPointerType_Of(rhs_pointer_base);
			if unlikely(!rhs_pointer_type) {
				Dee_Decref_unlikely(CPointerType_AsType(lhs_pointer_type));
				goto err;
			}
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot subtract incompatible pointer types: %k - %k",
			                lhs_pointer_type, rhs_pointer_type);
			Dee_Decref_unlikely(CPointerType_AsType(rhs_pointer_type));
			Dee_Decref_unlikely(CPointerType_AsType(lhs_pointer_type));
			goto err;
		}
		result = lhs_cvalue.uint - rhs_cvalue.uint;
		if (CArrayType_Stride(tp_self))
			result /= CArrayType_Stride(tp_self);
		return DeeInt_NewPtrdiff(result);
	}
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue_int))
		goto err;
	return Dee_AsObject(CArray_PlusOffset(lhs, -rhs_cvalue_int));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
carray_size(CArray *__restrict self) {
	CArrayType *tp_self = Dee_TYPE(self);
	return CArrayType_Count(tp_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF CLValue *DCALL
carray_getitem_index(CArray *__restrict self, size_t index) {
	CArrayType *tp_self = Dee_TYPE(self);
	if unlikely(index >= CArrayType_Count(tp_self))
		goto err_index_oob;
	return CArray_GetItem(self, (ptrdiff_t)index);
err_index_oob:
	DeeRT_ErrIndexOutOfBounds(self, index, CArrayType_Count(tp_self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
carray_delitem_index(CArray *__restrict self, size_t index) {
	CArrayType *tp_self = Dee_TYPE(self);
	if unlikely(index >= CArrayType_Count(tp_self))
		goto err_index_oob;
	index *= CArrayType_Stride(tp_self);
	bzero((byte_t *)CArray_Items(self) + index, CArrayType_SizeofPointedToType(tp_self));
	return 0;
err_index_oob:
	return DeeRT_ErrIndexOutOfBounds(self, index, CArrayType_Count(tp_self));
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
carray_setitem_index(CArray *__restrict self, size_t index, DeeObject *value) {
	CArrayType *tp_self = Dee_TYPE(self);
	CType *item_type = CArrayType_PointedToType(tp_self);
	struct ctype_operators const *item_ops = CType_Operators(item_type);
	if unlikely(index >= CArrayType_Count(tp_self))
		goto err_index_oob;
	index *= CArrayType_Stride(tp_self);
	return (*item_ops->co_assign)(item_type,
	                              (byte_t *)CArray_Items(self) + index,
	                              value);
err_index_oob:
	return DeeRT_ErrIndexOutOfBounds(self, index, CArrayType_Count(tp_self));
}

PRIVATE struct type_math carray_math = {
	/* .tp_int32  = */ (int(DCALL *)(DeeObject *__restrict, int32_t *__restrict))&cobject_int32,
	/* .tp_int64  = */ (int(DCALL *)(DeeObject *__restrict, int64_t *__restrict))&cobject_int64,
	/* .tp_double = */ (int(DCALL *)(DeeObject *__restrict, double *__restrict))&cobject_double,
	/* .tp_int    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_int,
	/* .tp_inv    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_inv, /* Unsupported */
	/* .tp_pos    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_pos, /* Unsupported */
	/* .tp_neg    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_neg, /* Unsupported */
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&carray_add,
	/* .tp_sub    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&carray_sub,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_mul, /* Unsupported */
	/* .tp_div    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_div, /* Unsupported */
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_mod, /* Unsupported */
	/* .tp_shl    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_shl, /* Unsupported */
	/* .tp_shr    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_shr, /* Unsupported */
	/* .tp_and    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_and, /* Unsupported */
	/* .tp_or     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_or,  /* Unsupported */
	/* .tp_xor    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_xor, /* Unsupported */
	/* .tp_pow    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_pow, /* Unsupported */
};

PRIVATE struct type_seq carray_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&carray_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&carray_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&carray_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&CArray_GetItem,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&carray_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&carray_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))NULL, /* TODO: &carray_hasitem_index */
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))NULL, /* TODO: &carray_hasitem_index */
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,  /* TODO: &carray_getrange_index */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,              /* TODO: &carray_delrange_index */
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))NULL, /* TODO: &carray_setrange_index */
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))NULL,               /* TODO: &carray_getrange_index_n */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))NULL,                           /* TODO: &carray_delrange_index_n */
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))NULL,              /* TODO: &carray_setrange_index_n */
};


PRIVATE DeeTypeObject *tpconst carray_subclass_mro[] = {
	CArrayType_AsType(&AbstractCArray_Type),
#define carray_mro (carray_subclass_mro + 1)
	&DeeSeq_Type,
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	NULL,
};

PRIVATE NONNULL((1)) void DCALL
carraytype_fini(CArrayType *__restrict self) {
	DREF CType *item_type = self->cat_item;

	/* Delete the weak-link to the original type. */
	CType_CacheLockWrite(item_type);
	ASSERT(LIST_ISBOUND(self, cat_chain));
	LIST_REMOVE(self, cat_chain);
	CType_CacheLockEndWrite(item_type);
	Dee_Decref(CType_AsType(item_type));

	/* Prevent "type_fini" from trying to free this */
	ASSERT(self->cat_base.ct_base.tp_mro == carray_subclass_mro);
	self->cat_base.ct_base.tp_mro = NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF CLValueType *DCALL
carraytype_itemtype(CArrayType *__restrict self) {
	return CLValueType_Of(CArrayType_PointedToType(self));
}

PRIVATE struct type_member tpconst carraytype_members[] = {
	TYPE_MEMBER_CONST_DOC("isarray", Dee_True, DOC_GET(isarray_doc)),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT_AB, offsetof(CArrayType, cat_item), "->?GCType"),
	TYPE_MEMBER_FIELD("size", STRUCT_CONST | STRUCT_SIZE_T, offsetof(CArrayType, cat_count)),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst carraytype_getsets[] = {
	TYPE_GETTER_AB("ItemType", &carraytype_itemtype, "->?GLValueType"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject CArrayType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "ArrayType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &CType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CArrayType,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&carraytype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
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
	/* .tp_getsets       = */ carraytype_getsets,
	/* .tp_members       = */ carraytype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DOC_DEF(carray_doc,
        "()\n"
        "Zero-initialize all array-elements\n"
        "\n"
        "(items:?S?O)\n"
        "Initialize array-elements from @items");

INTERN CArrayType AbstractCArray_Type = {
	/* .cat_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CArrayType_Type),
			/* .tp_name     = */ "Array",
			/* .tp_doc      = */ DOC_GET(carray_doc),
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CType_AsType(&AbstractCObject_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CVoid,
					/* tp_ctor:        */ &cobject_ctor,
					/* tp_copy_ctor:   */ &cobject_copy,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ &cobject_init_kw,
					/* tp_serialize:   */ &cobject_serialize
				),
				/* .tp_dtor        = */ NULL,
				/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&cobject_assign,
				/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&cobject_assign
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
			},
			/* .tp_visit         = */ NULL,
			/* .tp_gc            = */ NULL,
			/* .tp_math          = */ &carray_math,
			/* .tp_cmp           = */ &cobject_cmp,
			/* .tp_seq           = */ &carray_seq,
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
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ NULL,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ carray_mro
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ 0,
			/* ct_alignof:   */ 1,
			/* ct_operators: */ &carray_operators,
			/* ct_ffitype:   */ &ffi_type_void
		)
	},
	/* .cat_item   = */ &AbstractCObject_Type,
	/* .cat_chain  = */ LIST_ENTRY_UNBOUND_INITIALIZER,
	/* .cat_count  = */ 0,
	/* .cat_stride = */ 0,
};


PRIVATE WUNUSED NONNULL((1)) DREF CArrayType *DCALL
CArrayType_New(CType *__restrict item_type, size_t item_count) {
	DREF CArrayType *result;
	result = DeeGCObject_CALLOC(CArrayType);
	if unlikely(!result)
		goto err;

	/* Allowing "(size_t)-1" as size would lead to problems with error reporting */
	if unlikely(item_count == (size_t)-1)
		goto too_large;

	/* Calculate C sizeof */
	if (OVERFLOW_UMUL(item_count, CType_Sizeof(item_type),
	                  &result->cat_base.ct_sizeof)) {
		/* Overflow: Array is too large. */
too_large:
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "Array type `%r[%" PRFuSIZ "]` is too large",
		                item_type, item_count);
		goto err_r;
	}

	/* Initialize fields. */

	result->cat_base.ct_base.tp_init.tp_alloc.tp_instance_size = result->cat_base.ct_sizeof;
	result->cat_base.ct_base.tp_init.tp_alloc.tp_instance_size += offsetof(CObject, co_data);
	DeeType_OptimizeAlloc(&result->cat_base.ct_base);
	result->cat_base.ct_base.tp_name  = "Array";
	result->cat_base.ct_base.tp_doc   = DOC_GET(carray_doc);
	result->cat_base.ct_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FHEAP | TP_FMOVEANY;
	Dee_Incref(CArrayType_AsType(&AbstractCArray_Type));
	result->cat_base.ct_base.tp_base  = CArrayType_AsType(&AbstractCArray_Type);
	result->cat_base.ct_base.tp_cmp   = &cobject_cmp;
	result->cat_base.ct_base.tp_math  = &carray_math;
	result->cat_base.ct_base.tp_seq   = &carray_seq;
	result->cat_base.ct_base.tp_mro   = carray_subclass_mro;
	Dee_atomic_rwlock_cinit(&result->cat_base.ct_cachelock);
	result->cat_base.ct_alignof   = item_type->ct_alignof; /* Re-use item alignment. */
	result->cat_base.ct_operators = &carray_operators;

	/* Technically, if "item_type" is a CStructType, we'd need to implement attribute access
	 * on the array as accessing the struct members of the 0'th array element. Reason is that
	 * in regular C, arrays behavior the same as pointers, and deemon's ctypes library allows
	 * pointers-to-structs to directly access struct members without having to call ".ind"
	 * first. However, that would ruin the fact that ctypes arrays are already implementing
	 * deemon's native Sequence-interface (which heavily relies on helper functions and such),
	 * which would completely conflict with struct members.
	 *
	 * As such, in this one edge-case, we (intentionally)
	 * don't implement array-to-pointer decay! */

	Dee_Incref(CType_AsType(item_type));
	result->cat_item   = item_type; /* Inherit reference. */
	result->cat_count  = item_count;
	result->cat_stride = CType_Sizeof(item_type); /* XXX: Shouldn't this be aligned? */

	/* Finalize the array type. */
	DeeObject_InitStatic(CArrayType_AsType(result), &CArrayType_Type);
	return Type_AsCArrayType(DeeGC_TRACK(DeeTypeObject, CArrayType_AsType(result)));
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


PRIVATE NONNULL((1)) bool DCALL
ctype_array_rehash(CType *__restrict self, Dee_hash_t new_mask) {
	struct carray_type_list *new_map;
again:
	new_map = (struct carray_type_list *)Dee_TryCallocc(new_mask + 1,
	                                                    sizeof(struct carray_type_list));
	if unlikely(!new_map) {
		/* Try again with a 1-element mask. */
		if (!self->ct_arrays.sa_list && new_mask != 0) {
			new_mask = 1;
			goto again;
		}
		return false;
	}

	/* Do the re-hash. */
	if (self->ct_arrays.sa_size) {
		struct carray_type_list *biter, *bend;
		ASSERT(self->ct_arrays.sa_list);
		bend = (biter = self->ct_arrays.sa_list) + (self->ct_arrays.sa_mask + 1);
		for (; biter < bend; ++biter) {
			CArrayType *iter, *next;
			iter = LIST_FIRST(biter);
			while (iter) {
				struct carray_type_list *dst;
				next = LIST_NEXT(iter, cat_chain);
				dst  = &new_map[iter->cat_count & new_mask];
				/* Insert the entry into the new hash-map. */
				LIST_INSERT_HEAD(dst, iter, cat_chain);
				iter = next;
			}
		}
	}

	/* Delete the old map and install the new. */
	Dee_Free(self->ct_arrays.sa_list);
	self->ct_arrays.sa_mask = new_mask;
	self->ct_arrays.sa_list = new_map;
	return true;
}

/* Construct a new array-type `item_type[item_count]' */
INTERN WUNUSED NONNULL((1)) DREF CArrayType *DCALL
CArrayType_Of(CType *__restrict item_type, size_t item_count) {
	DREF CArrayType *result;
	DREF struct carray_type_list *bucket;
	ASSERT_OBJECT_TYPE(CType_AsType(item_type), &CType_Type);
	CType_CacheLockRead(item_type);
	ASSERT(!item_type->ct_arrays.sa_size ||
	       item_type->ct_arrays.sa_mask);
	if (item_type->ct_arrays.sa_size) {
		result = LIST_FIRST(&item_type->ct_arrays.sa_list[item_count & item_type->ct_arrays.sa_mask]);
		while (result && result->cat_count != item_count)
			result = LIST_NEXT(result, cat_chain);

		/* Check if we can re-use an existing type. */
		if (result && Dee_IncrefIfNotZero(CArrayType_AsType(result))) {
			CType_CacheLockEndRead(item_type);
			return result;
		}
	}
	CType_CacheLockEndRead(item_type);

	/* Construct a new array type. */
	result = CArrayType_New(item_type, item_count);
	if unlikely(!result)
		goto done;

	/* Add the new type to the cache. */
register_type:
	CType_CacheLockWrite(item_type);
	ASSERT(!item_type->ct_arrays.sa_size ||
	       item_type->ct_arrays.sa_mask);
	if (item_type->ct_arrays.sa_size) {
		DREF CArrayType *new_result;
		new_result = LIST_FIRST(&item_type->ct_arrays.sa_list[item_count & item_type->ct_arrays.sa_mask]);
		while (new_result && new_result->cat_count != item_count)
			new_result = LIST_NEXT(new_result, cat_chain);

		/* Check if we can re-use an existing type. */
		if (new_result && Dee_IncrefIfNotZero(CArrayType_AsType(new_result))) {
			CType_CacheLockEndRead(item_type);
			Dee_Decref(CArrayType_AsType(result));
			return new_result;
		}
	}

	/* Rehash when there are a lot of items. */
	if (item_type->ct_arrays.sa_size >= item_type->ct_arrays.sa_mask &&
	    (!ctype_array_rehash(item_type, item_type->ct_arrays.sa_mask
	                               ? (item_type->ct_arrays.sa_mask << 1) | 1
	                               : 16 - 1) &&
	     !item_type->ct_arrays.sa_mask)) {

		/* No space at all! */
		CType_CacheLockEndWrite(item_type);

		/* Collect enough memory for a 1-item hash map. */
		if (Dee_CollectMemory(sizeof(CArrayType *)))
			goto register_type;

		/* Failed to allocate the initial hash-map. */
		Dee_Decref(CArrayType_AsType(result));
		result = NULL;
		goto done;
	}

	/* Insert the new array type into the hash-map. */
	bucket = &item_type->ct_arrays.sa_list[item_count & item_type->ct_arrays.sa_mask];
	LIST_INSERT_HEAD(bucket, result, cat_chain); /* Weak reference. */
	CType_CacheLockEndWrite(item_type);
done:
	return result;
}





/************************************************************************/
/* STRUCT TYPE                                                          */
/************************************************************************/

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_no_such_struct_field(CStructType *__restrict type,
                         char const *field_name,
                         size_t field_name_length) {
	(void)type;
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "No such struct field: %$s",
	                       field_name_length, field_name);
}

PRIVATE NONNULL((1)) void DCALL
cstructtype_fini(CStructType *__restrict self) {
	size_t i;
	for (i = 0; i < self->cst_size; ++i) {
		struct cstruct_field *field = &self->cst_fvec[i];
		if (field->csf_name) {
			Dee_Decref(field->csf_name);
			Dee_Decref(CLValueType_AsType(field->csf_lvtype));
		}
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cstructtype_offsetof(CStructType *self, size_t argc, DeeObject *const *argv) {
	struct cstruct_field *field;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("offsetof", params: "
	DeeStringObject *field
", docStringPrefix: "cstructtype");]]]*/
#define cstructtype_offsetof_params "field:?Dstring"
	struct {
		DeeStringObject *field;
	} args;
	DeeArg_Unpack1(err, argc, argv, "offsetof", &args.field);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.field, &DeeString_Type))
		goto err;
	field = CStructType_FieldByName(self, Dee_AsObject(args.field));
	if unlikely(!field)
		goto err_no_such_field;
	return DeeInt_NewPtrdiff(cstruct_field_getoffset(field));
err_no_such_field:
	err_no_such_struct_field(self, DeeString_STR(args.field), DeeString_SIZE(args.field));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cstructtype_offsetafter(CStructType *self, size_t argc, DeeObject *const *argv) {
	struct cstruct_field *field;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("offsetafter", params: "
	DeeStringObject *field
", docStringPrefix: "cstructtype");]]]*/
#define cstructtype_offsetafter_params "field:?Dstring"
	struct {
		DeeStringObject *field;
	} args;
	DeeArg_Unpack1(err, argc, argv, "offsetafter", &args.field);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.field, &DeeString_Type))
		goto err;
	field = CStructType_FieldByName(self, Dee_AsObject(args.field));
	if unlikely(!field)
		goto err_no_such_field;
	return DeeInt_NewPtrdiff(cstruct_field_getoffset(field) +
	                         CType_Sizeof(cstruct_field_gettype(field)));
err_no_such_field:
	err_no_such_struct_field(self, DeeString_STR(args.field), DeeString_SIZE(args.field));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF CType *DCALL
cstructtype_typeof_field(CStructType *self, size_t argc, DeeObject *const *argv) {
	DREF CType *result;
	struct cstruct_field *field;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("typeof_field", params: "
	DeeStringObject *field
", docStringPrefix: "cstructtype");]]]*/
#define cstructtype_typeof_field_params "field:?Dstring"
	struct {
		DeeStringObject *field;
	} args;
	DeeArg_Unpack1(err, argc, argv, "typeof_field", &args.field);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.field, &DeeString_Type))
		goto err;
	field = CStructType_FieldByName(self, Dee_AsObject(args.field));
	if unlikely(!field)
		goto err_no_such_field;
	result = cstruct_field_gettype(field);
	Dee_Incref(CType_AsType(result));
	return result;
err_no_such_field:
	err_no_such_struct_field(self, DeeString_STR(args.field), DeeString_SIZE(args.field));
err:
	return NULL;
}

PRIVATE struct type_method tpconst cstructtype_methods[] = {
	TYPE_METHOD_F("offsetof", &cstructtype_offsetof, METHOD_FNOREFESCAPE,
	              "(" cstructtype_offsetof_params ")->?Dint\n"
	              "#tAttributeError{No field with the name @field exists}"
	              "Returns the offset of a given @field"),
	TYPE_METHOD_F("offsetafter", &cstructtype_offsetafter, METHOD_FNOREFESCAPE,
	              "(" cstructtype_offsetafter_params ")->?Dint\n"
	              "#tAttributeError{No field with the name @field exists}"
	              "Returns the offset after a given @field"),
	/* TODO: containerof(pointer p, string field) -> lvalue
	 *       Where type(p) === this.typeof(field).pointer,
	 *       and type(return) == this.lvalue */
	TYPE_METHOD_F("typeof_field", &cstructtype_typeof_field, METHOD_FNOREFESCAPE,
	              "(field:?Dstring)->?GCType\n"
	              "#tAttributeError{No field with the name @field exists}"
	              "Returns the typing of given @field"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst cstructtype_getsets[] = {
	/* TODO: "fields->?M?Dstring?GStructField", where "StructField" is
	 *       a custom proxy type to inspect attributes of struct fields */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cstructtype_members[] = {
	TYPE_MEMBER_CONST_DOC("isstruct", Dee_True, DOC_GET(isstruct_doc)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF CStructType *DCALL
cstructtype_class_of(DeeTypeObject *__restrict UNUSED(tp_self),
                     size_t argc, DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("of", params: """
	fields:?S?X2?T3?Dint?Dstring?DType?T2?Dint?GStructType,
	size_t alignment = 1,
", docStringPrefix: "cstructtype_class", defineKwList: true);]]]*/
	static DEFINE_KWLIST(of_kwlist, { KEX("fields", 0x4de26e52, 0x85564d5853cce154), KEX("alignment", 0xe24a3d8d, 0x3923d5964b5f4177), KEND });
#define cstructtype_class_of_params "fields:?S?X2?T3?Dint?Dstring?DType?T2?Dint?GStructType,alignment=!1"
	struct {
		DeeObject *fields;
		size_t alignment;
	} args;
	args.alignment = 1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, of_kwlist, "o|" UNPuSIZ ":of", &args))
		goto err;
/*[[[end]]]*/
	/* Extended constructor to allow custom struct types:
	 * - allowing you to manually specify offsets of fields
	 * - allowing you to set a custom alignment
	 * Can just use `struct cstruct_builder' to implement this! */
	return CStructType_OfExtended(args.fields, args.alignment);
err:
	return NULL;
}

PRIVATE struct type_method tpconst cstructtype_class_methods[] = {
	TYPE_KWMETHOD_F("of", &cstructtype_class_of, METHOD_FNOREFESCAPE,
	                "(" cstructtype_class_of_params ")->?.\n"
	                "Construct a custom struct type containing the "
	                /**/ "specified fields at the specified offsets:\n"
	                "${"
	                /**/ "local WeirdStruct = struct.of({\n"
	                /**/ "	{ 0, \"x\", le32 },\n"
	                /**/ "	{ 1, \"y\", le32 },\n"
	                /**/ "});\n"
	                /**/ "local instance = WeirdStruct();\n"
	                /**/ "instance.x = 0x11223344;\n"
	                /**/ "/* HINT: With `be32', this'd be `0x22334400' */\n"
	                /**/ "assert instance.y == 0x112233;"
	                "}"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED DREF CStructType *DCALL
cstructtype_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	unsigned int flags;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("StructType", params: """
	fields:?S?X2?T2?Dstring?DType?GStructType,
	bool union = false,
	bool packed = false,
	size_t alignment = 1,
", docStringPrefix: "cstructtype", defineKwList: true);]]]*/
	static DEFINE_KWLIST(StructType_kwlist, { KEX("fields", 0x4de26e52, 0x85564d5853cce154), KEX("union", 0x23b88b9b, 0x3b416e7d690babb2), KEX("packed", 0xfb4a9ef3, 0x3cfc3e48b37361b5), KEX("alignment", 0xe24a3d8d, 0x3923d5964b5f4177), KEND });
#define cstructtype_StructType_params "fields:?S?X2?T2?Dstring?DType?GStructType,union=!f,packed=!f,alignment=!1"
	struct {
		DeeObject *fields;
		bool union_;
		bool packed_;
		size_t alignment;
	} args;
	args.union_ = false;
	args.packed_ = false;
	args.alignment = 1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, StructType_kwlist, "o|bb" UNPuSIZ ":StructType", &args))
		goto err;
/*[[[end]]]*/
	flags = CSTRUCTTYPE_F_NORMAL;
	if (args.union_)
		flags |= CSTRUCTTYPE_F_UNION;
	if (args.packed_)
		flags |= CSTRUCTTYPE_F_PACKED;
	return CStructType_Of(args.fields, flags, args.alignment);
err:
	return NULL;
}

INTERN DeeTypeObject CStructType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "StructType",
	/* .tp_doc      = */ DOC("(" cstructtype_StructType_params ")\n"
	                         "#palignment{Minimum alignment of returned struct (if non-empty)}"
	                         "Construct a new struct type with the given fields. The given sequence "
	                         /**/ "may also contain instances of other ?. types, which will then be "
	                         /**/ "inlined into the returned struct in a way that mirrors anonymous "
	                         /**/ "struct/union types in regular C"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &CType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &cstructtype_init_kw,
			/* tp_serialize:   */ NULL,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cstructtype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
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
	/* .tp_methods       = */ cstructtype_methods,
	/* .tp_getsets       = */ cstructtype_getsets,
	/* .tp_members       = */ cstructtype_members,
	/* .tp_class_methods = */ cstructtype_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED NONNULL((2)) DREF CLValue *DCALL
CStruct_GetAttrField_p(void *self, struct cstruct_field const *field, DeeObject *owner) {
	DREF CLValue *result = CLValue_Alloc();
	if unlikely(!result)
		goto err;
	result->cl_value.ptr = self;
	result->cl_value.sint += field->csf_offset;
	Dee_XIncref(owner);
	result->cl_owner = owner;
	CLValue_Init(result, field->csf_lvtype);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((2)) int DCALL
CStruct_DelAttrField_p(void *self, struct cstruct_field const *field) {
	union pointer cvalue;
	cvalue.ptr = self;
	cvalue.sint += field->csf_offset;
	CTYPES_FAULTPROTECT({
		bzero(cvalue.ptr, CType_Sizeof(cstruct_field_gettype(field)));
	}, return -1);
	return 0;
}

INTERN WUNUSED NONNULL((2, 3)) int DCALL
CStruct_SetAttrField_p(void *self, struct cstruct_field const *field, DeeObject *value) {
	union pointer cvalue;
	CType *field_type = cstruct_field_gettype(field);
	struct ctype_operators const *ops = CType_Operators(field_type);
	cvalue.ptr = self;
	cvalue.sint += field->csf_offset;
	return (*ops->co_assign)(field_type, cvalue.ptr, value);
}


INTERN WUNUSED NONNULL((1, 3)) DREF CLValue *DCALL
CStruct_GetAttr_p(CStructType *__restrict tp_self, void *self,
                  /*String*/ DeeObject *attr, DeeObject *owner) {
	struct cstruct_field *field;
	field = CStructType_FieldByName(tp_self, attr);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_GetAttrField_p(self, field, owner);
err_no_such_field:
	err_no_such_struct_field(tp_self, DeeString_STR(attr), DeeString_SIZE(attr));
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF CLValue *DCALL
CStruct_GetAttrStringHash_p(CStructType *__restrict tp_self, void *self,
                            char const *attr, Dee_hash_t hash, DeeObject *owner) {
	struct cstruct_field *field;
	field = CStructType_FieldByNameStringHash(tp_self, attr, hash);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_GetAttrField_p(self, field, owner);
err_no_such_field:
	err_no_such_struct_field(tp_self, attr, strlen(attr));
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF CLValue *DCALL
CStruct_GetAttrStringLenHash_p(CStructType *__restrict tp_self, void *self,
                               char const *attr, size_t attrlen,
                               Dee_hash_t hash, DeeObject *owner) {
	struct cstruct_field *field;
	field = CStructType_FieldByNameStringLenHash(tp_self, attr, attrlen, hash);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_GetAttrField_p(self, field, owner);
err_no_such_field:
	err_no_such_struct_field(tp_self, attr, attrlen);
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
CStruct_DelAttr_p(CStructType *__restrict tp_self, void *self, /*String*/ DeeObject *attr) {
	struct cstruct_field *field;
	field = CStructType_FieldByName(tp_self, attr);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_DelAttrField_p(self, field);
err_no_such_field:
	err_no_such_struct_field(tp_self, DeeString_STR(attr), DeeString_SIZE(attr));
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
CStruct_DelAttrStringHash_p(CStructType *__restrict tp_self, void *self,
                            char const *attr, Dee_hash_t hash) {
	struct cstruct_field *field;
	field = CStructType_FieldByNameStringHash(tp_self, attr, hash);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_DelAttrField_p(self, field);
err_no_such_field:
	err_no_such_struct_field(tp_self, attr, strlen(attr));
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
CStruct_DelAttrStringLenHash_p(CStructType *__restrict tp_self, void *self,
                               char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct cstruct_field *field;
	field = CStructType_FieldByNameStringLenHash(tp_self, attr, attrlen, hash);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_DelAttrField_p(self, field);
err_no_such_field:
	err_no_such_struct_field(tp_self, attr, attrlen);
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 3, 4)) int DCALL
CStruct_SetAttr_p(CStructType *__restrict tp_self, void *self,
                  /*String*/ DeeObject *attr, DeeObject *value) {
	struct cstruct_field *field;
	field = CStructType_FieldByName(tp_self, attr);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_SetAttrField_p(self, field, value);
err_no_such_field:
	err_no_such_struct_field(tp_self, DeeString_STR(attr), DeeString_SIZE(attr));
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 3, 5)) int DCALL
CStruct_SetAttrStringHash_p(CStructType *__restrict tp_self, void *self,
                            char const *attr, Dee_hash_t hash, DeeObject *value) {
	struct cstruct_field *field;
	field = CStructType_FieldByNameStringHash(tp_self, attr, hash);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_SetAttrField_p(self, field, value);
err_no_such_field:
	err_no_such_struct_field(tp_self, attr, strlen(attr));
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 3, 6)) int DCALL
CStruct_SetAttrStringLenHash_p(CStructType *__restrict tp_self, void *self,
                               char const *attr, size_t attrlen,
                               Dee_hash_t hash, DeeObject *value) {
	struct cstruct_field *field;
	field = CStructType_FieldByNameStringLenHash(tp_self, attr, attrlen, hash);
	if unlikely(!field)
		goto err_no_such_field;
	return CStruct_SetAttrField_p(self, field, value);
err_no_such_field:
	err_no_such_struct_field(tp_self, attr, attrlen);
/*err:*/
	return -1;
}



INTERN WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
CStruct_GetAttr(CStruct *self, /*String*/ DeeObject *attr) {
	return CStruct_GetAttr_p(Dee_TYPE(self), CStruct_Data(self), attr, Dee_AsObject(self));
}

INTERN WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
CStruct_GetAttrStringHash(CStruct *self, char const *attr, Dee_hash_t hash) {
	return CStruct_GetAttrStringHash_p(Dee_TYPE(self), CStruct_Data(self), attr, hash, Dee_AsObject(self));
}

INTERN WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
CStruct_GetAttrStringLenHash(CStruct *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	return CStruct_GetAttrStringLenHash_p(Dee_TYPE(self), CStruct_Data(self), attr, attrlen, hash, Dee_AsObject(self));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
CStruct_DelAttr(CStruct *self, /*String*/ DeeObject *attr) {
	return CStruct_DelAttr_p(Dee_TYPE(self), CStruct_Data(self), attr);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
CStruct_DelAttrStringHash(CStruct *self, char const *attr, Dee_hash_t hash) {
	return CStruct_DelAttrStringHash_p(Dee_TYPE(self), CStruct_Data(self), attr, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
CStruct_DelAttrStringLenHash(CStruct *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	return CStruct_DelAttrStringLenHash_p(Dee_TYPE(self), CStruct_Data(self), attr, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
CStruct_SetAttr(CStruct *self, /*String*/ DeeObject *attr, DeeObject *value) {
	return CStruct_SetAttr_p(Dee_TYPE(self), CStruct_Data(self), attr, value);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
CStruct_SetAttrStringHash(CStruct *self, char const *attr, Dee_hash_t hash, DeeObject *value) {
	return CStruct_SetAttrStringHash_p(Dee_TYPE(self), CStruct_Data(self), attr, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
CStruct_SetAttrStringLenHash(CStruct *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value) {
	return CStruct_SetAttrStringLenHash_p(Dee_TYPE(self), CStruct_Data(self), attr, attrlen, hash, value);
}


PRIVATE struct type_attr cstruct_attr = {
	/* .tp_getattr                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&CStruct_GetAttr,
	/* .tp_delattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))&CStruct_DelAttr,
	/* .tp_setattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&CStruct_SetAttr,
	/* .tp_iterattr                  = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))NULL, /*TODO: &cstruct_iterattr,*/
	/* .tp_findattr                  = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))NULL, /*TODO: &cstruct_findattr,*/
	/* .tp_hasattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstruct_hasattr,*/
	/* .tp_boundattr                 = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstruct_boundattr,*/
	/* .tp_callattr                  = */ NULL,
	/* .tp_callattr_kw               = */ NULL,
	/* .tp_vcallattrf                = */ NULL,
	/* .tp_getattr_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&CStruct_GetAttrStringHash,
	/* .tp_delattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&CStruct_DelAttrStringHash,
	/* .tp_setattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&CStruct_SetAttrStringHash,
	/* .tp_hasattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstruct_hasattr_string_hash, */
	/* .tp_boundattr_string_hash     = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstruct_boundattr_string_hash, */
	/* .tp_callattr_string_hash      = */ NULL,
	/* .tp_callattr_string_hash_kw   = */ NULL,
	/* .tp_vcallattr_string_hashf    = */ NULL,
	/* .tp_getattr_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&CStruct_GetAttrStringLenHash,
	/* .tp_delattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&CStruct_DelAttrStringLenHash,
	/* .tp_setattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&CStruct_SetAttrStringLenHash,
	/* .tp_hasattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstruct_hasattr_string_len_hash, */
	/* .tp_boundattr_string_len_hash = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstruct_boundattr_string_len_hash, */
};

DOC_DEF(cstruct_doc,
        "()\n"
        "Construct a zero-initialize instance of this struct\n"
        "\n"
        "(initializers!!)\n"
        "Initialize struct fields using keyword arguments @initializers "
        /**/ "(uninitialized fields zero-initialized):\n"
        "${"
        /**/ "struct Point { .x = int, .y = int };\n"
        /**/ "Point val;\n"
        /**/ "val.x = 10;\n"
        /**/ "val.y = 20;\n"
        /**/ "assert val == Point(x: 10, y: 20);\n"
        /**/ ""
        "}\n"
        "\n"
        "(initializers:?S?O)\n"
        "Initialize struct fields in order of declaration, using values read from @initializers "
        /**/ "This method of initialization matches how standard C would require a struct initializer "
        /**/ "be to used (uninitialized fields zero-initialized):\n"
        "${"
        /**/ "struct Point { .x = int, .y = int };\n"
        /**/ "Point val = { 10, 20 };\n"
        /**/ "assert val == Point(x: 10, y: 20);"
        "}\n"
        "\n"
        "(initializers:?M?Dstring?O)\n"
        "Initialize struct fields using named field initializers "
        /**/ "(uninitialized fields zero-initialized):\n"
        "${"
        /**/ "struct Point { .x = int, .y = int };\n"
        /**/ "Point val = { .x = 10, .y = 20 };\n"
        /**/ "assert val == Point(x: 10, y: 20);"
        "}");

INTERN struct empty_cstruct_type_object AbstractCStruct_Type = {
	/* .cst_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CStructType_Type),
			/* .tp_name     = */ "Struct",
			/* .tp_doc      = */ DOC_GET(cstruct_doc),
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CType_AsType(&AbstractCObject_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CVoid,
					/* tp_ctor:        */ &cobject_ctor,
					/* tp_copy_ctor:   */ &cobject_copy,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ &cobject_init_kw,
					/* tp_serialize:   */ &cobject_serialize
				),
				/* .tp_dtor        = */ NULL,
				/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&cobject_assign,
				/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&cobject_assign
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
			},
			/* .tp_visit         = */ NULL,
			/* .tp_gc            = */ NULL,
			/* .tp_math          = */ &cobject_math,
			/* .tp_cmp           = */ &cobject_cmp,
			/* .tp_seq           = */ NULL,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ &cstruct_attr,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ NULL,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ NULL,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ NULL
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ 0,
			/* ct_alignof:   */ 1,
			/* ct_operators: */ &cstruct_operators,
			/* ct_ffitype:   */ &ffi_type_void
		)
	},
	/* .cst_first = */ NULL,
	/* .cst_fmsk  = */ 0,
	/* .cst_size  = */ 1,
	/* .cst_fvec  = */ {
		/* [0] = */ {
			/* .csf_name   = */ NULL,
			/* .csf_hash   = */ 0,
			/* .csf_offset = */ 0,
			/* .csf_next   = */ NULL,
			/* .csf_lvtype = */ NULL
		}
	}
};


INTERN WUNUSED NONNULL((1, 2)) struct cstruct_field *DCALL
CStructType_FieldByName(CStructType const *self, /*string*/DeeObject *name) {
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	return CStructType_FieldByNameStringLenHash(self,
	                                            DeeString_STR(name),
	                                            DeeString_SIZE(name),
	                                            DeeString_Hash(name));
}

INTERN WUNUSED NONNULL((1, 2)) struct cstruct_field *DCALL
CStructType_FieldByNameStringHash(CStructType const *self,
                                  char const *name, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct cstruct_field *field;
		DeeStringObject *field_name;
		field      = STRUCT_TYPE_HASHIT(self, i);
		field_name = field->csf_name;
		if unlikely(!field_name)
			break;
		if (field->csf_hash != hash)
			continue;
		if (strcmp(DeeString_STR(field_name), name) == 0)
			return field;
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) struct cstruct_field *DCALL
CStructType_FieldByNameStringLenHash(CStructType const *self, char const *name,
                                     size_t namelen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct cstruct_field *field;
		DeeStringObject *field_name;
		field      = STRUCT_TYPE_HASHIT(self, i);
		field_name = field->csf_name;
		if unlikely(!field_name)
			break;
		if (field->csf_hash != hash)
			continue;
		if (DeeString_SIZE(field_name) != namelen)
			continue;
		if (bcmp(DeeString_STR(field_name), name, namelen * sizeof(char)) == 0)
			return field;
	}
	return NULL;
}


struct cstruct_builder {
	CStructType           *csb_result;    /* [0..1] Struct built thus far ("cst_size" is the allocated size) */
	size_t                 csb__cst_size; /* Used size (after packing, this becomes `csb_result->cst_size') */
	size_t                 csb_msk_used;  /* # of named fields used */
	struct cstruct_field **csb_p_last;    /* [0..0][0..1][valid_if(csb_result)] Pointer to next-field of last-added field */
};

#define cstruct_builder_init(self) \
	(void)((self)->csb_result = NULL, (self)->csb__cst_size = 0, (self)->csb_msk_used = 0)
PRIVATE NONNULL((1)) void DCALL
cstruct_builder_fini(struct cstruct_builder *__restrict self) {
	size_t i;
	CStructType *result = self->csb_result;
	if (!result)
		return;
	for (i = 0; i < self->csb__cst_size; ++i) {
		struct cstruct_field *field = &result->cst_fvec[i];
		if (field->csf_name) {
			Dee_Decref(CLValueType_AsType(field->csf_lvtype));
			Dee_Decref(field->csf_name);
		}
	}
	DeeGCObject_Free(result);
}

PRIVATE NONNULL((1, 2, 3)) struct cstruct_field *DCALL
CStructType_AddField(CStructType *self,
                     /*inherit(on_success)*/ DREF struct Dee_string_object *name,
                     /*inherit(on_success)*/ DREF CLValueType *lvtype,
                     ptrdiff_t offset) {
	struct cstruct_field *field;
	Dee_hash_t i, perturb, hash = DeeString_Hash(name);
	ASSERT(!DeeString_IsEmpty(name));
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		DeeStringObject *field_name;
		field      = STRUCT_TYPE_HASHIT(self, i);
		field_name = field->csf_name;
		if unlikely(!field_name)
			break;
		if (field->csf_hash != hash)
			continue;
		if (DeeString_EqualsSTR(field_name, name))
			return NULL; /* Field with this name already exists */
	}
	field->csf_name   = name;
	field->csf_hash   = hash;
	field->csf_offset = offset;
	field->csf_next   = NULL;
	field->csf_lvtype = lvtype;
	return field;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cstruct_builder_rehash(struct cstruct_builder *__restrict self,
                       Dee_hash_t mask, size_t size) {
	CStructType *old_result = self->csb_result;
	CStructType *new_result;
	ASSERT(IS_POWER_OF_TWO(mask + 1));
	ASSERT(mask < size);
	new_result = (CStructType *)DeeGCObject_Callocc(offsetof(CStructType, cst_fvec),
	                                                size, sizeof(struct cstruct_field));
	if unlikely(!new_result)
		goto err;
	new_result->cst_fmsk = mask;
	new_result->cst_size = size;
	if (old_result) {
		size_t i;
		struct cstruct_field *src_iter;
		struct cstruct_field **p_dst_iter;

		/* Transfer base fields. */
		new_result->cst_base.ct_sizeof  = old_result->cst_base.ct_sizeof;
		new_result->cst_base.ct_alignof = old_result->cst_base.ct_alignof;

		/* Transfer struct fields... */
		for (i = 0; i <= old_result->cst_fmsk; ++i) {
			struct cstruct_field *old_field = &old_result->cst_fvec[i];
			if (old_field->csf_name) {
				struct cstruct_field *new_field;
				new_field = CStructType_AddField(new_result,
				                                 old_field->csf_name,
				                                 old_field->csf_lvtype,
				                                 old_field->csf_offset);
				ASSERTF(new_field,
				        "If field %r is duplicate, then how was it "
				        /**/ "ever added to the original CStructType?",
				        old_field->csf_name);
				ASSERT(new_field->csf_next == NULL);
			}
		}

		/* Migrate anonymous fields... */
		for (; i < self->csb__cst_size; ++i) {
			struct cstruct_field *old_field = &old_result->cst_fvec[i];
			struct cstruct_field *new_field = &new_result->cst_fvec[i - old_result->cst_fmsk + mask];
			ASSERT(old_field->csf_name);
			ASSERT(DeeString_IsEmpty(old_field->csf_name));
			*new_field = *old_field;
			new_field->csf_next = NULL;
		}

		/* Rebuild ordered field chain */
		p_dst_iter = &new_result->cst_first;
		for (src_iter = old_result->cst_first; src_iter; src_iter = src_iter->csf_next) {
			struct cstruct_field *new_field;
			ASSERT(src_iter->csf_name);
			if (DeeString_IsEmpty(src_iter->csf_name)) {
				/* Anonymous field */
				size_t old_field_index = src_iter - old_result->cst_fvec;
				size_t new_field_index = old_field_index - old_result->cst_fmsk + mask;
				new_field = &new_result->cst_fvec[new_field_index];
			} else {
				new_field = CStructType_FieldByName(new_result, Dee_AsObject(src_iter->csf_name));
			}
			ASSERT(new_field);
			ASSERT(new_field->csf_name == src_iter->csf_name);
			ASSERT(new_field->csf_hash == src_iter->csf_hash);
			ASSERT(new_field->csf_offset == src_iter->csf_offset);
			ASSERT(new_field->csf_lvtype == src_iter->csf_lvtype);
			ASSERT(new_field->csf_next == NULL);
			*p_dst_iter = (struct cstruct_field *)new_field;
			p_dst_iter = &new_field->csf_next;
		}
		self->csb_p_last = p_dst_iter;
		self->csb__cst_size -= old_result->cst_fmsk;
		self->csb__cst_size += mask;

		/* Free the old struct type buffer. */
		DeeGCObject_Free(old_result);
	} else {
		ASSERT(self->csb__cst_size == 0);
		ASSERT(self->csb_msk_used == 0);
		self->csb_p_last = &new_result->cst_first;
		self->csb__cst_size = mask + 1;
	}
	self->csb_result = new_result;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF CStructType *DCALL
cstruct_builder_pack(struct cstruct_builder *__restrict self) {
	DREF CStructType *result = self->csb_result;
	if unlikely(!result) {
		/* Special case: empty struct */
		Dee_Incref(CStructType_AsType(&AbstractCStruct_Type));
		return (DREF CStructType *)&AbstractCStruct_Type;
	}
	ASSERT(self->csb__cst_size <= result->cst_size);
	if (self->csb__cst_size < result->cst_size) {
		result = (CStructType *)DeeGCObject_TryReallocc(result,
		                                                offsetof(CStructType, cst_fvec),
		                                                self->csb__cst_size,
		                                                sizeof(struct cstruct_field));
		if unlikely(!result)
			result = self->csb_result;
		result->cst_size = self->csb__cst_size;
	}

	result->cst_base.ct_base.tp_init.tp_alloc.tp_instance_size = result->cst_base.ct_sizeof;
	result->cst_base.ct_base.tp_init.tp_alloc.tp_instance_size += offsetof(CObject, co_data);
	DeeType_OptimizeAlloc(&result->cst_base.ct_base);
	result->cst_base.ct_base.tp_name  = "Struct";
	result->cst_base.ct_base.tp_doc   = DOC_GET(cstruct_doc);
	result->cst_base.ct_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FHEAP | TP_FMOVEANY;
	Dee_Incref(CStructType_AsType(&AbstractCStruct_Type));
	result->cst_base.ct_base.tp_base  = CStructType_AsType(&AbstractCStruct_Type);
	result->cst_base.ct_base.tp_cmp   = &cobject_cmp;
	result->cst_base.ct_base.tp_math  = &cobject_math;
	result->cst_base.ct_base.tp_attr  = &cstruct_attr;
	Dee_atomic_rwlock_cinit(&result->cst_base.ct_cachelock);
	result->cst_base.ct_operators = &cstruct_operators;
	DeeObject_InitStatic(CStructType_AsType(result), &CStructType_Type);
	return Type_AsCStructType(DeeGC_TRACK(DeeTypeObject, CStructType_AsType(result)));
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 5)) int DCALL
cstruct_builder_addfield(struct cstruct_builder *__restrict self,
                         /*inherit(always)*/ DREF struct Dee_string_object *name,
                         /*inherit(always)*/ DREF CLValueType *lvtype,
                         ptrdiff_t offset, ptrdiff_t *p_end_offset,
                         bool part_of_chain) {
	struct cstruct_field *new_field;
	CStructType *result;
	ptrdiff_t end_offset;
	size_t min_align;
	if (!self->csb_result) {
		if unlikely(cstruct_builder_rehash(self, 7, 8))
			goto err;
	} else if (self->csb_msk_used >= self->csb_result->cst_fmsk) {
		size_t new_mask = (self->csb_result->cst_fmsk << 1) | 1;
		size_t new_size = new_mask + (self->csb__cst_size - self->csb_result->cst_fmsk);
		if unlikely(cstruct_builder_rehash(self, new_mask, new_size))
			goto err;
	}
	result = self->csb_result;
	new_field = CStructType_AddField(result, name, lvtype, offset);
	if unlikely(!new_field)
		goto err_duplicate_field;
	if (part_of_chain) {
		*self->csb_p_last = new_field;
		self->csb_p_last = &new_field->csf_next;
	}
	++self->csb_msk_used;

	/* Update struct end-offset if necessary */
	end_offset = offset + CType_Sizeof(CLValueType_PointedToType(lvtype));
	if (end_offset > (ptrdiff_t)result->cst_base.ct_sizeof)
		result->cst_base.ct_sizeof = (size_t)end_offset;
	*p_end_offset = end_offset;
	min_align = CType_Alignof(CLValueType_PointedToType(lvtype));
	if (IS_ALIGNED(offset, min_align)) {
		if (result->cst_base.ct_alignof < min_align)
			result->cst_base.ct_alignof = min_align;
	}
	return 0;
err_duplicate_field:
	DeeError_Throwf(&DeeError_TypeError, "Duplicate field %r in struct", name);
err:
	Dee_Decref_unlikely(name);
	Dee_Decref_unlikely(CLValueType_AsType(lvtype));
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
cstruct_builder_addfield_anon(struct cstruct_builder *__restrict self,
                              /*inherit(always)*/ DREF CLValueType *lvtype,
                              ptrdiff_t offset, ptrdiff_t *p_end_offset) {
	struct cstruct_field *new_field;
	CStructType *result;
	ptrdiff_t end_offset;
	size_t min_align;
	if (!self->csb_result) {
		if unlikely(cstruct_builder_rehash(self, 7, 9))
			goto err;
	} else if (self->csb__cst_size >= self->csb_result->cst_size) {
		size_t new_mask = self->csb_result->cst_fmsk;
		size_t new_size = self->csb__cst_size + 3;
		if unlikely(cstruct_builder_rehash(self, new_mask, new_size))
			goto err;
	}
	result = self->csb_result;
	ASSERT(self->csb__cst_size < result->cst_size);
	new_field = &result->cst_fvec[self->csb__cst_size];

	/* Fill in field. */
	new_field->csf_name   = (DREF DeeStringObject *)DeeString_NewEmpty();
//	new_field->csf_hash   = 0; /* Doesn't matter */
	new_field->csf_offset = offset;
	ASSERT(new_field->csf_next == NULL);
	new_field->csf_lvtype = lvtype; /* Inherit reference */

	/* Append field to chain */
	*self->csb_p_last = new_field;
	self->csb_p_last = &new_field->csf_next;
	++self->csb__cst_size;

	/* Update struct end-offset if necessary */
	end_offset = offset + CType_Sizeof(CLValueType_PointedToType(lvtype));
	if (end_offset > (ptrdiff_t)result->cst_base.ct_sizeof)
		result->cst_base.ct_sizeof = (size_t)end_offset;
	*p_end_offset = end_offset;
	min_align = CType_Alignof(CLValueType_PointedToType(lvtype));
	if (IS_ALIGNED(offset, min_align)) {
		if (result->cst_base.ct_alignof < min_align)
			result->cst_base.ct_alignof = min_align;
	}
	return 0;
err:
	Dee_Decref_unlikely(CLValueType_AsType(lvtype));
	return -1;
}



struct cstruct_of_data {
	struct cstruct_builder csod_builder; /* Struct builder */
	unsigned int           csod_flags;   /* Set of `CSTRUCTTYPE_F_*' */
	ptrdiff_t              csod_offset;  /* Offset of next field to add */
};

PRIVATE NONNULL((1, 2)) void DCALL
cstruct_of_data_maybe_align(struct cstruct_of_data *__restrict self, CType *align_for) {
	if (!(self->csod_flags & CSTRUCTTYPE_F_PACKED)) {
		size_t alignment = CType_Alignof(align_for);
		self->csod_offset = CEIL_ALIGN(self->csod_offset, alignment);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstruct_builder_addfield_from_struct(struct cstruct_builder *__restrict self,
                                     CStructType *__restrict in,
                                     ptrdiff_t base_offset) {
	Dee_hash_t i;

	/* Inline all struct fields */
	for (i = 0; i <= in->cst_fmsk; ++i) {
		struct cstruct_field *field = &in->cst_fvec[i];
		if (field->csf_name) {
			ptrdiff_t field_offset = base_offset + field->csf_offset;
			ptrdiff_t ignored_offset_after;
			Dee_Incref(field->csf_name);
			Dee_Incref(CLValueType_AsType(field->csf_lvtype));
			if (cstruct_builder_addfield(self, field->csf_name,
			                             field->csf_lvtype, field_offset,
			                             &ignored_offset_after, false))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
cstruct_of_cb(void *arg, DeeObject *elem) {
	struct cstruct_of_data *data = (struct cstruct_of_data *)arg;
	ptrdiff_t end_offset;
	DREF DeeObject *name_and_type[2];
	/* Given "elem" must be one of:
	 * - CStructType
	 * - (string, CType) */
	if (Object_IsCStructType(elem)) {
		/* Add struct as an anonymous field, and inline all fields */
		CStructType *in = Object_AsCStructType(elem);
		DREF CLValueType *in_lvalue = CLValueType_Of(CStructType_AsCType(in));
		if unlikely(!in_lvalue)
			goto err;
		cstruct_of_data_maybe_align(data, CStructType_AsCType(in));
		if (cstruct_builder_addfield_anon(&data->csod_builder, in_lvalue /* inherited */,
		                                  data->csod_offset, &end_offset))
			goto err;

		/* Inline all struct fields */
		if (cstruct_builder_addfield_from_struct(&data->csod_builder, in, data->csod_offset))
			goto err;
	} else {
		CType *field_type;
		DREF CLValueType *field_type_lvalue;

		/* Given "elem" must be a pair `{ fieldName: string, fieldType: CType }' */
		if (DeeObject_InvokeMethodHint(seq_unpack, elem, 2, name_and_type))
			goto err;
		if (DeeObject_AssertType(name_and_type[0], &DeeString_Type))
			goto err_name_and_type;
		field_type = CType_Of(name_and_type[1]);
		if unlikely(!field_type)
			goto err_name_and_type;

		/* Use "field_type" */
		field_type_lvalue = CLValueType_Of(field_type);
		if unlikely(!field_type_lvalue)
			goto err_name_and_type;
		Dee_Decref(name_and_type[1]);
		name_and_type[1] = Dee_AsObject(CLValueType_AsType(field_type_lvalue));

		/* Add field. */
		cstruct_of_data_maybe_align(data, field_type);
		if (cstruct_builder_addfield(&data->csod_builder,
		                             (DREF DeeStringObject *)name_and_type[0], /* Inherited */
		                             Object_AsCLValueType(name_and_type[1]),   /* Inherited */
		                             data->csod_offset, &end_offset, true))
			goto err;
	}

	/* If not a union, use end-offset of field as offset for next field. */
	if (!(data->csod_flags & CSTRUCTTYPE_F_UNION))
		data->csod_offset = end_offset;
	return 0;
err_name_and_type:
	Dee_Decref(name_and_type[1]);
	Dee_Decref(name_and_type[0]);
err:
	return -1;
}


/* Construct a new struct-type from `initializer', which
 * should be `{((string, CType) | CStructType)...}'
 * @param: flags: Set of `CSTRUCTTYPE_F_*' */
INTERN WUNUSED NONNULL((1)) DREF CStructType *DCALL
CStructType_Of(DeeObject *__restrict initializer,
               unsigned int flags, size_t min_alignment) {
	struct cstruct_of_data data;
	cstruct_builder_init(&data.csod_builder);
	data.csod_flags  = flags;
	data.csod_offset = 0;
	if unlikely(DeeObject_Foreach(initializer, &cstruct_of_cb, &data) < 0)
		goto err_builder;
	if (data.csod_builder.csb_result) {
		/* Force 1-byte alignment when building in packed-mode */
		if (data.csod_flags & CSTRUCTTYPE_F_PACKED)
			data.csod_builder.csb_result->cst_base.ct_alignof = 1;
		/* Apply caller-given minimum alignment constraint */
		if (data.csod_builder.csb_result->cst_base.ct_alignof < min_alignment)
			data.csod_builder.csb_result->cst_base.ct_alignof = min_alignment;
	}
	return cstruct_builder_pack(&data.csod_builder);
err_builder:
	cstruct_builder_fini(&data.csod_builder);
/*err:*/
	return NULL;
}


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
cstruct_ofex_cb(void *arg, DeeObject *elem) {
	struct cstruct_builder *builder = (struct cstruct_builder *)arg;
	ptrdiff_t field_offset, end_offset;
	DREF DeeObject *args[3];
	size_t arg_count;
	/* Given "elem" must be one of:
	 * - (int, CStructType)
	 * - (int, string, CType) */
	arg_count = DeeObject_InvokeMethodHint(seq_unpack_ex, elem, 2, 3, args);
	if unlikely(arg_count == (size_t)-1)
		goto err;
	ASSERT(arg_count == 2 || arg_count == 3);
	if (DeeObject_AsPtrdiff(args[0], &field_offset)) {
		if (arg_count == 3)
			goto err_args_3;
		goto err_args_2;
	}
	if (arg_count == 2) {
		/* Add struct as an anonymous field, and inline all fields */
		CStructType *in;
		DREF CLValueType *in_lvalue;
		if (!Object_IsCStructType(args[1])) {
			DeeObject_TypeAssertFailed(args[1], &CStructType_Type);
			goto err_args_2;
		}
		in = Object_AsCStructType(args[1]);
		in_lvalue = CLValueType_Of(CStructType_AsCType(in));
		if unlikely(!in_lvalue)
			goto err_args_2;
		if (cstruct_builder_addfield_anon(builder, in_lvalue /* inherited */,
		                                  field_offset, &end_offset))
			goto err_args_2;
		/* Inline all struct fields */
		if (cstruct_builder_addfield_from_struct(builder, in, field_offset))
			goto err_args_2;
		Dee_Decref(args[1]);
	} else {
		CType *field_type;
		DREF CLValueType *field_type_lvalue;
		if (DeeObject_AssertType(args[1], &DeeString_Type))
			goto err_args_3;
		field_type = CType_Of(args[2]);
		if unlikely(!field_type)
			goto err_args_3;

		/* Use "field_type" */
		field_type_lvalue = CLValueType_Of(field_type);
		if unlikely(!field_type_lvalue)
			goto err_args_3;
		Dee_Decref(args[2]);
		args[2] = Dee_AsObject(CLValueType_AsType(field_type_lvalue));

		/* Add field. */
		if (cstruct_builder_addfield(builder,
		                             (DREF DeeStringObject *)args[1], /* Inherited */
		                             Object_AsCLValueType(args[2]),   /* Inherited */
		                             field_offset, &end_offset, true))
			goto err_args_1;
	}
	Dee_Decref(args[0]);
	return 0;
err_args_3:
	Dee_Decref(args[2]);
err_args_2:
	Dee_Decref(args[1]);
err_args_1:
	Dee_Decref(args[0]);
err:
	return -1;
}

/* Construct a new struct-type from `initializer', which
 * should be `{((int, string, CType) | (int, CStructType))...}'
 * @param: flags: Set of `CSTRUCTTYPE_F_*' */
INTERN WUNUSED NONNULL((1)) DREF CStructType *DCALL
CStructType_OfExtended(DeeObject *__restrict initializer,
                       size_t min_alignment) {
	struct cstruct_builder builder;
	cstruct_builder_init(&builder);
	if unlikely(DeeObject_Foreach(initializer, &cstruct_ofex_cb, &builder) < 0)
		goto err_builder;
	/* Force 1-byte alignment when building in packed-mode */
	if (builder.csb_result &&
	    builder.csb_result->cst_base.ct_alignof < min_alignment)
		builder.csb_result->cst_base.ct_alignof = min_alignment;
	return cstruct_builder_pack(&builder);
err_builder:
	cstruct_builder_fini(&builder);
/*err:*/
	return NULL;
}








/************************************************************************/
/* FUNCTION TYPE                                                        */
/************************************************************************/

#ifdef CONFIG_NO_CFUNCTION
/* Throw a NotImplemented error explaining that cfunctions have been disabled. */
INTERN ATTR_COLD int DCALL err_no_cfunction(void) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "ctypes was built without C-functions being enabled");
}

#define PTR_cfunctiontype_fini NULL
#else /* CONFIG_NO_CFUNCTION */
#define PTR_cfunctiontype_fini &cfunctiontype_fini
PRIVATE NONNULL((1)) void DCALL
cfunctiontype_fini(CFunctionType *__restrict self) {
	CType *return_type = self->cft_return;

	/* Delete the weak-link to the original type. */
	CType_CacheLockWrite(return_type);
	ASSERT(LIST_ISBOUND(self, cft_chain));
	LIST_REMOVE(self, cft_chain);
	CType_CacheLockEndWrite(return_type);

	Dee_Decref(CType_AsType(return_type));
	Dee_Decrefv((DeeObject **)self->cft_argv, self->cft_argc);
	Dee_Free((void *)self->cft_ffi_arg_type_v);
}
#endif /* !CONFIG_NO_CFUNCTION */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cfunctiontype_args(CFunctionType *__restrict self) {
#ifdef CONFIG_NO_CFUNCTION
	(void)self;
	err_no_cfunction();
	return NULL;
#else /* CONFIG_NO_CFUNCTION */
	return DeeRefVector_NewReadonly(CFunctionType_AsType(self),
	                                self->cft_argc,
	                                (DeeObject **)self->cft_argv);
#endif /* !CONFIG_NO_CFUNCTION */
}

PRIVATE struct type_getset tpconst cfunctiontype_getsets[] = {
	TYPE_GETTER_AB_F("args", &cfunctiontype_args, METHOD_FCONSTCALL,
	                 "->?S?GCType\n"
	                 "Returns an immutable sequence describing "
	                 /**/ "the argument types used by this function"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cfunctiontype_members[] = {
	TYPE_MEMBER_CONST_DOC("isfunction", Dee_True, DOC_GET(isfunction_doc)),
#ifndef CONFIG_NO_CFUNCTION
	TYPE_MEMBER_FIELD_DOC("returntype", STRUCT_OBJECT_AB, offsetof(CFunctionType, cft_return),
	                      "->?GCType"),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT_AB, offsetof(CFunctionType, cft_return),
	                      "->?GCType\n"
	                      "Deprecated alias for ?#returntype"),
#else /* !CONFIG_NO_CFUNCTION */
	TYPE_MEMBER_CONST("returntype", &CVoid_Type),
#endif /* CONFIG_NO_CFUNCTION */
	TYPE_MEMBER_END
};

INTERN DeeTypeObject CFunctionType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "FunctionType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &CType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))PTR_cfunctiontype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
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
	/* .tp_getsets       = */ cfunctiontype_getsets,
	/* .tp_members       = */ cfunctiontype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject *tpconst cfunction_subclass_mro[] = {
	CFunctionType_AsType(&AbstractCFunction_Type),
#define cfunction_mro (cfunction_subclass_mro + 1)
	&DeeCallable_Type,
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	NULL,
};

#ifndef CONFIG_NO_CFUNCTION
union argument {
	int i;
	float f;
	double d;
	long double ld;
	unsigned int u;
	int8_t s8;
	uint8_t u8;
	int16_t s16;
	uint16_t u16;
	int32_t s32;
	uint32_t u32;
	int64_t s64;
	uint64_t u64;
	void *p;
	union pointer ptr;
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dummy_call(CFunctionType *__restrict tp_self, Dee_funptr_t self,
           size_t argc, DeeObject *const *argv) {
	(void)tp_self;
	(void)self;
	(void)argc;
	(void)argv;
	return_none;
}

#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES

/*POC:
import * from ctypes;
import rt;

local dCore = ShLib.ofmodule(import("deemon"));

local DeeObject = void;
local Dee_foreach_t = ssize_t.func("stdcall", void.ptr, DeeObject.ptr);
local DeeObject_Foreach = (ssize_t(DeeObject.ptr, Dee_foreach_t.ptr, void.ptr).ptr)
	dCore["DeeObject_Foreach@12"];


local cb = Dee_foreach_t(callback: (cookie, object_addr) -> {
	print "HERE:", repr cookie, repr object_addr;
	return 0;
});

local items = { 10, 20, 30 };
local result = DeeObject_Foreach(rt.ctypes_addrof(items), cb, void.ptr(42));
print "result:", result;
*/
PRIVATE void
cfunction_closure_proc(ffi_cif *cif,
                       union argument *return_storage,
                       union argument **argv,
                       CFunction *self) {
	int status;
	DREF DeeObject *result;
	CFunctionType *tp_self = Dee_TYPE(self);
	CType *return_type;
	struct ctype_operators const *return_type_ops;
	size_t i, argc = CFunction_ArgumentCount(tp_self);
	DREF DeeObject **deemon_argv;
	(void)cif;
	deemon_argv = (DREF DeeObject **)Dee_Mallocac(argc, sizeof(DREF DeeObject *));
	if unlikely(!deemon_argv)
		goto err;

	/* Wrap arguments as appropriate instances of C-objects. */
	for (i = 0; i < argc; ++i) {
		union argument *arg = argv[i];
		CType *arg_type = CFunction_ArgumentType(tp_self, i);
		DREF CObject *deemon_arg = CType_AllocInstance(arg_type);
		if unlikely(!deemon_arg)
			goto err_deemon_argv_i;
		ASSERTF(!CType_IsCFunctionType(arg_type),
		        "Should have been prevented by the check in CFunctionType_New");
		CObject_Init(deemon_arg, arg_type);
		deemon_arg = (*CType_Operators(arg_type)->co_initobject)(deemon_arg, arg);
		deemon_argv[i] = Dee_AsObject(deemon_arg); /* Inherit reference */
	}

	/* Invoke callback */
	result = DeeObject_Call(self->cf_cb, argc, deemon_argv);

	/* Cleanup arguments... */
	Dee_Decrefv(deemon_argv, argc);
	Dee_Freea(deemon_argv);

	/* Check for error */
	if unlikely(!result)
		goto err;

	/* Transform "result" into the appropriate return value */
	return_type = CFunction_ReturnType(tp_self);
	return_type_ops = CType_Operators(return_type);
	status = (*return_type_ops->co_initfrom)(return_type, return_storage, result);
	Dee_Decref(result);

	/* Check for errors during return-value conversion. */
	if unlikely(status)
		goto err;
	return;
err_deemon_argv_i:
	Dee_Decrefv(deemon_argv, i);
/*err_deemon_argv:*/
	Dee_Freea(deemon_argv);
err:
	DeeError_Print("Unhandled error in FFI closure",
	               Dee_ERROR_PRINT_DOHANDLE);
	bzero(return_storage, tp_self->cft_ffi_return_type->size);
}

/* Construct a new C-function that invokes "cb" when called. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cfunction_init_impl(CFunction *__restrict self, DeeObject *cb) {
	CFunctionType *prototype = Dee_TYPE(self);
	ffi_status error;
	if ((unsigned int)prototype->cft_cc & CC_FVARARGS) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot construct closure for varargs function type: %k",
		                prototype);
		goto err;
	}

again_alloc__cf_write:
	self->cf_write = (ffi_closure *)ffi_closure_alloc(sizeof(ffi_closure),
	                                                  &self->cf_func.cff_vptr);
	if unlikely(!self->cf_write) {
		if (Dee_ReleaseSystemMemory())
			goto again_alloc__cf_write;
		goto err;
	}

	/* Prep another CIF object */
#ifndef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI
	error = ffi_prep_cif(&self->cf_cif,
	                     (ffi_abi)((unsigned int)prototype->cft_cc &
	                               (unsigned int)CC_MTYPE),
	                     (unsigned int)prototype->cft_argc,
	                     prototype->cft_ffi_return_type,
	                     (ffi_type **)prototype->cft_ffi_arg_type_v);
	if (error != FFI_OK) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Failed to create function closure (ffi_prep_cif: %d)",
		                (int)error);
		goto err_write;
	}
#define LOCAL_used_cif (&self->cf_cif)
#else /* !CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI */
#define LOCAL_used_cif (&prototype->cft_ffi_cif)
#endif /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES_REUSE_CFI */

#if 1
	error = ffi_prep_closure_loc(self->cf_write, LOCAL_used_cif,
	                             (void (*)(ffi_cif *, void *, void **, void *))&cfunction_closure_proc,
	                             self, self->cf_func.cff_vptr);
#else
	error = ffi_prep_closure(self->cf_write, LOCAL_used_cif,
	                         (void (*)(ffi_cif *, void *, void **, void *))&cfunction_closure_proc,
	                         self);
#endif
	if (error != FFI_OK) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Failed to create function closure (ffi_prep_closure: %d)",
		                (int)error);
		goto err_write;
	}
	Dee_Incref(cb);
	self->cf_cb = cb;
	return 0;
#undef LOCAL_used_cif
err_write:
	ffi_closure_free(self->cf_write);
err:
	return -1;
}

/*[[[deemon (print_DEFINE_KWLIST from rt.gen.unpack)({ "callback" });]]]*/
#ifndef DEFINED_kwlist__callback
#define DEFINED_kwlist__callback
PRIVATE DEFINE_KWLIST(kwlist__callback, { KEX("callback", 0x3b9dd39e, 0x1e7dd8df6e98f4c6), KEND });
#endif /* !DEFINED_kwlist__callback */
/*[[[end]]]*/


#define cfunction_init_kw_PTR &cfunction_init_kw
PRIVATE WUNUSED NONNULL((1)) int DCALL
cfunction_init_kw(CFunction *__restrict self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *callback;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__callback,
	                          "o:Function", &callback))
		goto err;
	return cfunction_init_impl(self, callback);
err:
	return -1;
}

#define cfunction_fini_PTR &cfunction_fini
PRIVATE NONNULL((1)) void DCALL
cfunction_fini(CFunction *__restrict self) {
	ffi_closure_free(self->cf_write);
	Dee_Decref(self->cf_cb);
}

#define cfunction_visit_PTR &cfunction_visit
PRIVATE NONNULL((1, 2)) void DCALL
cfunction_visit(CFunction *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->cf_cb);
}

#define cfunction_print_PTR &cfunction_print
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cfunction_print(CFunction *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<closure %k: %k>" PRFxPTR,
	                        Dee_TYPE(self), self->cf_cb);
}

#define cfunction_printrepr_PTR &cfunction_printrepr
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cfunction_printrepr(CFunction *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r(callback: %r)",
	                        Dee_TYPE(self), self->cf_cb);
}

#define cfunction_call_PTR &cfunction_call_op
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cfunction_call_op(CFunction *__restrict self, size_t argc, DeeObject *const *argv) {
	CFunctionType *tp_self = Dee_TYPE(self);
	return (*tp_self->cft_call)(tp_self, CFunction_Func(self), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF CPointer *DCALL
cfunction_getptr(CFunction *__restrict self) {
	DREF CPointerType *result_type;
	DREF CPointer *result = CPointer_Alloc();
	if unlikely(!result)
		goto err;
	result_type = CPointerType_Of(CFunctionType_AsCType(Dee_TYPE(self)));
	if unlikely(!result_type)
		goto err_r;
	Dee_Incref(self);
	result->cp_owner = Dee_AsObject(self);
	result->cp_value.pfunc = CFunction_Func(self);
	CPointer_InitInherited(result, result_type);
	return result;
err_r:
	CPointer_Free(result);
err:
	return NULL;
}

#define cfunction_getsets cfunction_getsets
PRIVATE struct type_getset tpconst cfunction_getsets[] = {
	TYPE_GETTER_AB_F("ptr", &cfunction_getptr, METHOD_FNOREFESCAPE,
	                 "->?GPointer\n"
	                 "Returns a function pointer for this c-function"),
	TYPE_GETSET_END
};

#define cfunction_doc_PTR cfunction_doc
DOC_DEF(cfunction_doc,
        "(callback:?DCallable)\n"
        "Construct a new C closure that calls forward to @callback");

#endif /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
#endif /* !CONFIG_NO_CFUNCTION */

#ifndef cfunction_fini_PTR
#define cfunction_fini_PTR NULL
#endif /* !cfunction_fini_PTR */
#ifndef cfunction_visit_PTR
#define cfunction_visit_PTR NULL
#endif /* !cfunction_visit_PTR */
#ifndef cfunction_getsets
#define cfunction_getsets NULL
#endif /* !cfunction_getsets */
#ifndef cfunction_init_kw_PTR
#define cfunction_init_kw_PTR NULL
#endif /* !cfunction_init_kw_PTR */
#ifndef cfunction_print_PTR
#define cfunction_print_PTR NULL
#endif /* !cfunction_print_PTR */
#ifndef cfunction_printrepr_PTR
#define cfunction_printrepr_PTR NULL
#endif /* !cfunction_printrepr_PTR */
#ifndef cfunction_call_PTR
#define cfunction_call_PTR NULL
#endif /* !cfunction_call_PTR */
#ifndef cfunction_doc_PTR
#define cfunction_doc_PTR NULL
#endif /* !cfunction_doc_PTR */


INTERN struct empty_cfunction_type_object AbstractCFunction_Type = {
	/* .cst_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CFunctionType_Type),
			/* .tp_name     = */ "Function",
			/* .tp_doc      = */ cfunction_doc_PTR,
#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
#else /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY | TP_FVARIABLE,
#endif /* !CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CType_AsType(&AbstractCObject_Type),
			/* .tp_init = */ {
#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CFunction,
					/* tp_ctor:        */ NULL,
					/* tp_copy_ctor:   */ NULL,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ cfunction_init_kw_PTR,
					/* tp_serialize:   */ NULL
				),
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define PTR_cfunction_tp_alloc DeeSlab_GetMalloc(sizeof(CFunction), (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(CFunction))
#define PTR_cfunction_tp_free  DeeSlab_GetFree(sizeof(CFunction), NULL)
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#define PTR_cfunction_tp_alloc (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(CFunction)
#define PTR_cfunction_tp_free  NULL
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#else /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
				Dee_TYPE_CONSTRUCTOR_INIT_VAR(
					/* tp_ctor:        */ NULL,
					/* tp_copy_ctor:   */ NULL,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ cfunction_init_kw_PTR,
					/* tp_serialize:   */ NULL,
					/* tp_free:        */ NULL
				),
#endif /* !CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
				/* .tp_dtor        = */ (void (DCALL *)(DeeObject *))cfunction_fini_PTR,
				/* .tp_assign      = */ NULL,
				/* .tp_move_assign = */ NULL
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ NULL,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))cfunction_print_PTR,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))cfunction_printrepr_PTR,
			},
			/* .tp_visit         = */ (void (DCALL *)(DeeObject *, Dee_visit_t, void *))cfunction_visit_PTR,
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
			/* .tp_getsets       = */ cfunction_getsets,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))cfunction_call_PTR,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ cfunction_mro
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ (size_t)-1,
			/* ct_alignof:   */ 1,
			/* ct_operators: */ &cvoid_operators,
			/* ct_ffitype:   */ &ffi_type_pointer
		)
	},
#ifndef CONFIG_NO_CFUNCTION
	/* .cft_call            = */ &dummy_call,
	/* .cft_return          = */ &CVoid_Type,
	/* .cft_chain           = */ LIST_ENTRY_UNBOUND_INITIALIZER,
	/* .cft_hash            = */ 0,
	/* .cft_argc            = */ 0,
	/* .cft_cc              = */ CC_DEFAULT,
	/* .cft_ffi_return_type = */ &ffi_type_void,
	/* .cft_ffi_arg_type_v  = */ NULL,
	/* .cft_ffi_cif         = */ {},
	/* .cft_wsize           = */ 0,
	/* .cft_woff_argmem     = */ 0,
	/* .cft_woff_argptr     = */ 0,
#endif /* !CONFIG_NO_CFUNCTION */
};



#ifndef CONFIG_NO_CFUNCTION
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
CFunction_CallFunptrVarargs(CFunctionType *__restrict tp_self, Dee_funptr_t self,
                            size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
CFunction_CallFunptr(CFunctionType *__restrict tp_self, Dee_funptr_t self,
                     size_t argc, DeeObject *const *argv);


#ifndef __INTELLISENSE__
#include "ctypes-cfunction-call.c.inl"
#define VARARGS
#include "ctypes-cfunction-call.c.inl"
#endif /* !__INTELLISENSE__ */


PRIVATE WUNUSED NONNULL((1)) DREF CFunctionType *DCALL
CFunctionType_New(CType *__restrict return_type,
                  ctypes_cc_t calling_convention,
                  size_t argc, CType *const *__restrict argv,
                  Dee_hash_t function_hash) {
	size_t i;
	DREF CFunctionType *result;
	ffi_status error;
	if (CType_IsCFunctionType(return_type)) {
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot construct function type returning another "
		                "function %k (did you mean to return a function pointer?)",
		                return_type);
		goto err;
	}

	result = (DREF CFunctionType *)DeeGCObject_Callocc(offsetof(CFunctionType, cft_argv),
	                                                   argc, sizeof(DREF CType *));
	if unlikely(!result)
		goto err;

	/* Collect all the type descriptors used by libffi. */
	result->cft_ffi_return_type = CType_GetFFIType(return_type);
	if unlikely(!result->cft_ffi_return_type)
		goto err_r;
	result->cft_ffi_arg_type_v = (ffi_type **)Dee_Mallocc(argc, sizeof(ffi_type *));
	if unlikely(!result->cft_ffi_arg_type_v)
		goto err_r;
	for (i = 0; i < argc; ++i) {
		ffi_type *tp;
		if (CType_IsCFunctionType(argv[i])) {
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot construct function type with argument %" PRFuSIZ " being "
			                "another function %k (did you mean to take a function pointer instead?)",
			                i, argv[i]);
			goto err;
		}
		tp = CType_GetFFIType(argv[i]);
		if unlikely(!tp)
			goto err_r_ffi_typev;
		((ffi_type **)result->cft_ffi_arg_type_v)[i] = tp;
	}

	/* Initialize the FFI closure controller. */
	DBG_ALIGNMENT_DISABLE();
	error = ffi_prep_cif(&result->cft_ffi_cif,
	                     (ffi_abi)((unsigned int)calling_convention &
	                               (unsigned int)CC_MTYPE),
	                     (unsigned int)argc,
	                     result->cft_ffi_return_type,
	                     (ffi_type **)result->cft_ffi_arg_type_v);
	DBG_ALIGNMENT_ENABLE();
	if (error != FFI_OK) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Failed to create function CIF (%d)",
		                (int)error);
		goto err_r_ffi_typev;
	}

	/* Calculate the wbuffer cache sizes */
	result->cft_wsize = result->cft_ffi_return_type->size;
	if (result->cft_wsize < sizeof(ffi_arg))
		result->cft_wsize = sizeof(ffi_arg);
	result->cft_woff_argmem = result->cft_wsize;
	if (ctypes_cc_isvarargs(calling_convention)) {
		result->cft_woff_variadic_argmem = result->cft_wsize + (sizeof(union argument) * argc);
	} else {
		result->cft_wsize += (sizeof(union argument) * argc);
		result->cft_woff_argptr = result->cft_wsize;
		result->cft_wsize += (sizeof(void *) * argc);
	}
	if (result->cft_wsize == 0)
		result->cft_wsize = 1;
	if (argc == 0) {
		result->cft_woff_argmem = 0;
		result->cft_woff_argptr = 0;
	}

	/* Initialize fields. */
	Dee_Incref(CType_AsType(return_type));
	result->cft_return = return_type; /* Inherit reference. */
	result->cft_base.ct_base.tp_name  = "Function";
#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
	result->cft_base.ct_base.tp_flags = TP_FINHERITCTOR | TP_FHEAP;
	result->cft_base.ct_base.tp_init.tp_alloc.tp_alloc = PTR_cfunction_tp_alloc;
	result->cft_base.ct_base.tp_init.tp_alloc.tp_free  = PTR_cfunction_tp_free;
#else /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
	result->cft_base.ct_base.tp_flags = TP_FHEAP | TP_FVARIABLE;
#endif /* !CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
	Dee_Incref(CFunctionType_AsType(&AbstractCFunction_Type));
	result->cft_base.ct_base.tp_base  = CFunctionType_AsType(&AbstractCFunction_Type); /* Inherit reference. */
	result->cft_base.ct_base.tp_mro   = cfunction_subclass_mro;
	result->cft_base.ct_sizeof        = (size_t)-1; /* Really high value to cause error in `ctype_frombytes()' */
	result->cft_hash                  = function_hash;
	result->cft_argc                  = argc;
	result->cft_cc                    = calling_convention;
	Dee_Movrefv((DeeObject **)result->cft_argv, (DeeObject **)argv, argc);
	Dee_atomic_rwlock_cinit(&result->cft_base.ct_cachelock);
	result->cft_base.ct_operators = &cvoid_operators;

	/* Assign the appropriate call operator. */
	result->cft_call = ctypes_cc_isvarargs(calling_convention)
	                   ? &CFunction_CallFunptrVarargs
	                   : &CFunction_CallFunptr;

	/* Finalize the cfunction type. */
	DeeObject_InitStatic(CFunctionType_AsType(result), &CFunctionType_Type);
	return Type_AsCFunctionType(DeeGC_TRACK(DeeTypeObject, CFunctionType_AsType(result)));
err_r_ffi_typev:
	Dee_Free((void *)result->cft_ffi_arg_type_v);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
stype_cfunction_rehash(CType *__restrict self,
                       Dee_hash_t new_mask) {
	struct cfunction_type_list *new_map, *dst;
	struct cfunction_type_list *biter, *bend;
	CFunctionType *iter, *next;
again:
	new_map = (struct cfunction_type_list *)Dee_TryCallocc(new_mask + 1,
	                                                       sizeof(struct cfunction_type_list));
	if unlikely(!new_map) {
		/* Try again with a 1-element mask. */
		if (!self->ct_functions.sf_list && new_mask != 0) {
			new_mask = 1;
			goto again;
		}
		return false;
	}

	/* Do the re-hash. */
	if (self->ct_functions.sf_size) {
		ASSERT(self->ct_functions.sf_list);
		bend = (biter = self->ct_functions.sf_list) + (self->ct_functions.sf_mask + 1);
		for (; biter < bend; ++biter) {
			iter = LIST_FIRST(biter);
			while (iter) {
				next = LIST_NEXT(iter, cft_chain);
				dst  = &new_map[iter->cft_hash & new_mask];
				/* Insert the entry into the new hash-map. */
				LIST_INSERT_HEAD(dst, iter, cft_chain);
				iter = next;
			}
		}
	}

	/* Delete the old map and install the new. */
	Dee_Free(self->ct_functions.sf_list);
	self->ct_functions.sf_mask = new_mask;
	self->ct_functions.sf_list = new_map;
	return true;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) Dee_hash_t DCALL
cfunction_hashof(CType const *return_type, ctypes_cc_t calling_convention,
                 size_t argc, CType /*const*/ *const *argv) {
	size_t i;
	Dee_hash_t result;
	result = Dee_HashPointer(return_type) ^ (Dee_hash_t)calling_convention ^ (Dee_hash_t)argc;
	for (i = 0; i < argc; ++i)
		result ^= Dee_HashPointer(argv[i]);
	return result;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
cfunction_equals(CFunctionType const *self, CType const *return_type,
                 ctypes_cc_t calling_convention, size_t argc,
                 CType /*const*/ *const *argv) {
	size_t i;
	if (self->cft_return != return_type)
		goto nope;
	if (self->cft_argc != argc)
		goto nope;
	if (self->cft_cc != calling_convention)
		goto nope;
	for (i = 0; i < argc; ++i) {
		if (self->cft_argv[i] != argv[i])
			goto nope;
	}
	return true;
nope:
	return false;
}
#endif /* !CONFIG_NO_CFUNCTION */


/* Construct a C-Function type for the given arguments */
INTERN WUNUSED NONNULL((1)) DREF CFunctionType *DCALL
CFunctionType_Of(CType *__restrict return_type,
                 ctypes_cc_t calling_convention,
                 size_t argc, CType *const *argv) {
#ifdef CONFIG_NO_CFUNCTION
	(void)return_type;
	(void)calling_convention;
	(void)argc;
	(void)argv;
	err_no_cfunction();
	return NULL;
#else /* CONFIG_NO_CFUNCTION */
	Dee_hash_t hash;
	DREF CFunctionType *result, *new_result;
	DREF struct cfunction_type_list *bucket;
	ASSERT_OBJECT_TYPE(CType_AsType(return_type), &CType_Type);
	CType_CacheLockRead(return_type);
	ASSERT(!return_type->ct_functions.sf_size ||
	       return_type->ct_functions.sf_mask);
	hash = cfunction_hashof(return_type, calling_convention, argc, argv);
	if (return_type->ct_functions.sf_size) {
		result = LIST_FIRST(&return_type->ct_functions.sf_list[hash & return_type->ct_functions.sf_mask]);
		while (result &&
		       (result->cft_hash != hash ||
		        !cfunction_equals(result, return_type, calling_convention, argc, argv)))
			result = LIST_NEXT(result, cft_chain);
		/* Check if we can re-use an existing type. */
		if (result && Dee_IncrefIfNotZero(CFunctionType_AsType(result))) {
			CType_CacheLockEndRead(return_type);
			return result;
		}
	}
	CType_CacheLockEndRead(return_type);

	/* Construct a new cfunction type. */
	result = CFunctionType_New(return_type, calling_convention, argc, argv, hash);
	if unlikely(!result)
		goto done;

	/* Add the new type to the cache. */
register_type:
	CType_CacheLockWrite(return_type);
	ASSERT(!return_type->ct_functions.sf_size ||
	       return_type->ct_functions.sf_mask);
	if (return_type->ct_functions.sf_size) {
		new_result = LIST_FIRST(&return_type->ct_functions.sf_list[hash & return_type->ct_functions.sf_mask]);
		while (new_result &&
		       (new_result->cft_hash != hash ||
		        !cfunction_equals(new_result, return_type, calling_convention, argc, argv)))
			new_result = LIST_NEXT(new_result, cft_chain);

		/* Check if we can re-use an existing type. */
		if (new_result && Dee_IncrefIfNotZero(CFunctionType_AsType(new_result))) {
			CType_CacheLockEndRead(return_type);
			Dee_Decref(CFunctionType_AsType(result));
			return new_result;
		}
	}

	/* Rehash when there are a lot of items. */
	if (return_type->ct_functions.sf_size >= return_type->ct_functions.sf_mask &&
	    (!stype_cfunction_rehash(return_type, return_type->ct_functions.sf_mask ? (return_type->ct_functions.sf_mask << 1) | 1 : 16 - 1) &&
	     !return_type->ct_functions.sf_mask)) {

		/* No space at all! */
		CType_CacheLockEndWrite(return_type);

		/* Collect enough memory for a 1-item hash map. */
		if (Dee_CollectMemory(sizeof(CFunctionType *)))
			goto register_type;

		/* Failed to allocate the initial hash-map. */
		Dee_Decref(CFunctionType_AsType(result));
		result = NULL;
		goto done;
	}

	/* Insert the new cfunction type into the hash-map. */
	bucket = &return_type->ct_functions.sf_list[hash & return_type->ct_functions.sf_mask];
	LIST_INSERT_HEAD(bucket, result, cft_chain); /* Weak reference. */
	CType_CacheLockEndWrite(return_type);
done:
	return result;
#endif /* !CONFIG_NO_CFUNCTION */
}









/************************************************************************/
/* POINTER TYPE                                                         */
/************************************************************************/

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define PTR_pointer_tp_alloc DeeSlab_GetMalloc(sizeof(CPointer), (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(CPointer))
#define PTR_pointer_tp_free  DeeSlab_GetFree(sizeof(CPointer), NULL)
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#define PTR_pointer_tp_alloc (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(CPointer)
#define PTR_pointer_tp_free  NULL
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

PRIVATE NONNULL((1)) void DCALL
cpointertype_fini(CPointerType *__restrict self) {
	CType *orig = self->cpt_orig;

	/* Delete the weak-link to the original type. */
	CType_CacheLockWrite(orig);
	if (orig->ct_pointer == self)
		orig->ct_pointer = NULL;
	CType_CacheLockEndWrite(orig);
	Dee_Decref(CType_AsType(orig));
}

PRIVATE struct type_member tpconst cpointertype_members[] = {
	TYPE_MEMBER_CONST_DOC("ispointer", Dee_True, DOC_GET(ispointer_doc)),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT_AB, offsetof(CPointerType, cpt_orig), "->?GCType"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject CPointerType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "PointerType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &CType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CPointerType,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cpointertype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
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
	/* .tp_members       = */ cpointertype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTERN WUNUSED NONNULL((1)) DREF CLValue *DCALL
CPointer_Ind(CPointer *__restrict self) {
	CType *base_type;
	DREF CLValueType *lvalue_type;
	CPointerType *tp_self;
	DREF CLValue *result = CLValue_Alloc();
	if unlikely(!result)
		goto err;
	tp_self     = Dee_TYPE(self);
	base_type   = CPointerType_PointedToType(tp_self);
	lvalue_type = CLValueType_Of(base_type);
	if unlikely(!lvalue_type)
		goto err_r;
	result->cl_value = self->cp_value;
	Dee_XIncref(self->cp_owner);
	result->cl_owner = self->cp_owner;
	CLValue_InitInherited(result, lvalue_type);
	return result;
err_r:
	CLValue_Free(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
cpointer_fini(CPointer *__restrict self) {
	Dee_XDecref(self->cp_owner);
}

PRIVATE NONNULL((1, 2)) void DCALL
cpointer_visit(CPointer *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(self->cp_owner);
}



/* Return a new pointer offset by `index' */
INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CPointer_PlusOffset(CPointer *__restrict self, ptrdiff_t index) {
	CPointerType *tp_self = Dee_TYPE(self);
	union pointer res_cvalue;
	index *= CPointerType_SizeofPointedToType(tp_self);
	res_cvalue = self->cp_value;
	res_cvalue.sint += index;
	Dee_Incref(CPointerType_AsType(tp_self));
	return CPointer_NewExInherited(tp_self, res_cvalue.ptr, self->cp_owner);
}

/* Return an L-Value after `index' as an offset to "self"  */
INTERN WUNUSED NONNULL((1)) DREF CLValue *DCALL
CPointer_GetItem(CPointer *__restrict self, ptrdiff_t index) {
	DREF CLValue *result;
	DREF CLValueType *lvalue_type;
	CPointerType *tp_self = Dee_TYPE(self);
	union pointer res_cvalue;
	index *= CPointerType_SizeofPointedToType(tp_self);
	res_cvalue = self->cp_value;
	res_cvalue.sint += index;
	lvalue_type = CLValueType_Of(CPointerType_PointedToType(tp_self));
	if unlikely(!lvalue_type)
		goto err;
	result = CLValue_Alloc();
	if unlikely(!result)
		goto err_lvalue_type;
	result->cl_value = res_cvalue;
	Dee_XIncref(self->cp_owner);
	result->cl_owner = self->cp_owner;
	CLValue_InitInherited(result, lvalue_type);
	return result;
err_lvalue_type:
	Dee_Decref_unlikely(CLValueType_AsType(lvalue_type));
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF CPointer *DCALL
cpointer_add(CPointer *lhs, DeeObject *rhs) {
	ptrdiff_t offset;
	if (DeeObject_AsPtrdiff(rhs, &offset))
		goto err;
	return CPointer_PlusOffset(lhs, offset);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cpointer_sub(CPointer *lhs, DeeObject *rhs) {
	CPointerType *tp_self = Dee_TYPE(lhs);
	ptrdiff_t rhs_cvalue_int;
	union pointer rhs_cvalue;
	CType *rhs_pointer_base;
	int status = DeeObject_TryAsGenericPointer(rhs, &rhs_pointer_base, &rhs_cvalue);
	if (status <= 0) {
		ptrdiff_t result;
		if unlikely(status < 0)
			goto err;
		if (CPointerType_PointedToType(tp_self) != rhs_pointer_base) {
			DREF CPointerType *rhs_pointer_type;
			rhs_pointer_type = CPointerType_Of(rhs_pointer_base);
			if unlikely(!rhs_pointer_type)
				goto err;
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot subtract incompatible pointer types: %k - %k",
			                tp_self, rhs_pointer_type);
			Dee_Decref_unlikely(CPointerType_AsType(rhs_pointer_type));
			goto err;
		}
		result = lhs->cp_value.uint - rhs_cvalue.uint;
		if (CPointerType_SizeofPointedToType(tp_self))
			result /= CPointerType_SizeofPointedToType(tp_self);
		return DeeInt_NewPtrdiff(result);
	}
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue_int))
		goto err;
	return Dee_AsObject(CPointer_PlusOffset(lhs, -rhs_cvalue_int));
err:
	return NULL;
}

PRIVATE struct type_math cpointer_math = {
	/* .tp_int32  = */ (int(DCALL *)(DeeObject *__restrict, int32_t *__restrict))&cobject_int32,
	/* .tp_int64  = */ (int(DCALL *)(DeeObject *__restrict, int64_t *__restrict))&cobject_int64,
	/* .tp_double = */ (int(DCALL *)(DeeObject *__restrict, double *__restrict))&cobject_double,
	/* .tp_int    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_int, /* TODO: Pointers shouldn't be directly convertible to integers (there should have to be an explicit cast to "ctypes.uintptr_t" or similar first) */
	/* .tp_inv    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_inv, /* Unsupported */
	/* .tp_pos    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_pos, /* Unsupported */
	/* .tp_neg    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cobject_neg, /* Unsupported */
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cpointer_add,
	/* .tp_sub    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cpointer_sub,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_mul, /* Unsupported */
	/* .tp_div    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_div, /* Unsupported */
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_mod, /* Unsupported */
	/* .tp_shl    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_shl, /* Unsupported */
	/* .tp_shr    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_shr, /* Unsupported */
	/* .tp_and    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_and, /* Unsupported */
	/* .tp_or     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_or,  /* Unsupported */
	/* .tp_xor    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_xor, /* Unsupported */
	/* .tp_pow    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cobject_pow, /* Unsupported */
};

#define cpointer_getitem_index CPointer_GetItem
PRIVATE WUNUSED NONNULL((1)) int DCALL
cpointer_delitem_index(CPointer *__restrict self, size_t index) {
	union pointer cvalue = self->cp_value;
	size_t item_size = CPointerType_SizeofPointedToType(Dee_TYPE(self));
	index *= item_size;
	cvalue.sint += index;
	CTYPES_FAULTPROTECT({
		bzero(cvalue.ptr, item_size);
	}, return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
cpointer_setitem_index(CPointer *self, size_t index, DeeObject *value) {
	union pointer cvalue = self->cp_value;
	CPointerType *tp_self = Dee_TYPE(self);
	CType *base_type = CPointerType_PointedToType(tp_self);
	struct ctype_operators const *ops = CType_Operators(base_type);
	index *= CPointerType_SizeofPointedToType(tp_self);
	cvalue.sint += index;
	return (*ops->co_assign)(base_type, cvalue.ptr, value);
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
cpointer_getitem(CPointer *self, DeeObject *index) {
	ptrdiff_t index_value;
	if (DeeObject_AsPtrdiff(index, &index_value))
		goto err;
	return cpointer_getitem_index(self, index_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cpointer_delitem(CPointer *self, DeeObject *index) {
	ptrdiff_t index_value;
	if (DeeObject_AsPtrdiff(index, &index_value))
		goto err;
	return cpointer_delitem_index(self, index_value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
cpointer_setitem(CPointer *self, DeeObject *index, DeeObject *value) {
	ptrdiff_t index_value;
	if (DeeObject_AsPtrdiff(index, &index_value))
		goto err;
	return cpointer_setitem_index(self, index_value, value);
err:
	return -1;
}


PRIVATE struct type_seq cpointer_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cpointer_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cpointer_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&cpointer_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cpointer_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&cpointer_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&cpointer_setitem_index,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,  /* TODO: &cpointer_getrange_index (return lvalue-to-array) */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,              /* TODO: &cpointer_delrange_index */
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))NULL, /* TODO: &cpointer_setrange_index */
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))NULL,               /* TODO: &cpointer_getrange_index_n (return lvalue-to-array) */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))NULL,                           /* TODO: &cpointer_delrange_index_n */
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))NULL,              /* TODO: &cpointer_setrange_index_n */
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
cpointer_ctor(CPointer *__restrict self) {
	self->cp_value.ptr = NULL;
	self->cp_owner     = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cpointer_copy(CPointer *__restrict self, CPointer *__restrict other) {
	self->cp_value.ptr = other->cp_value.ptr;
	self->cp_owner     = other->cp_owner;
	Dee_XIncref(self->cp_owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cpointer_serialize(CPointer *__restrict self,
                   DeeSerial *__restrict writer,
                   Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(CPointer, field))
	CPointer *out = DeeSerial_Addr2Mem(writer, addr, CPointer);
	out->cp_value = self->cp_value;
	return DeeSerial_XPutObject(writer, ADDROF(cp_owner), self->cp_owner);
#undef ADDROF
}


/*[[[deemon (print_DEFINE_KWLIST from rt.gen.unpack)({ "value" });]]]*/
#ifndef DEFINED_kwlist__value
#define DEFINED_kwlist__value
PRIVATE DEFINE_KWLIST(kwlist__value, { KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEND });
#endif /* !DEFINED_kwlist__value */
/*[[[end]]]*/

#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
#define cpointer_doc_params "value:?X8?N?GPointer?GFunction?Dstring?DBytes?GLValue?GArray?Dint"
#else /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
#define cpointer_doc_params "value:?X7?N?GPointer?Dstring?DBytes?GLValue?GArray?Dint"
#endif /* !CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
DOC_DEF(cpointer_doc,
        "(" cpointer_doc_params ")\n"
        "Re-interpret @value as a pointer of @this typing.\n"
        "#L-"
        "{When @value is an L-Value, it must be an L-Value-to-pointer, and the referenced pointer is cast"
        "|When @value is ?Dstring, ?Abase?GPointerType must be one of "
        /**/ "?Gchar/?Gwchar_t/?Gchar16_t/?Gchar32_t/?Gvoid/?Gint8_t/?Guint8_t, "
        /**/ "or an ?GArrayType of one of those, whose length does not exceed the "
        /**/ "number of words in the relevant encoding of the string @value, plus 1 "
        /**/ "(e.g. ${char[4].ptr(\"foo\")} is OK and returns a sequence similar to "
        /**/ "${\"foo\0\".ordinals}, but ${char[5].ptr(\"foo\")} would throw a :ValueError)"
        "|When @value is ?DBytes, said bytes must be ${>=} the ?Gsizeof ?Abase?GPointerType"
        "}");

PRIVATE WUNUSED NONNULL((1)) int DCALL
cpointer_init_kw(CPointer *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *value;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          "o:Pointer", &value))
		goto err;
	if (DeeNone_Check(value)) {
		self->cp_owner     = NULL;
		self->cp_value.ptr = NULL;
	} else if (Object_IsCPointer(value)) {
		CPointer *value_ob = Object_AsCPointer(value);
		self->cp_owner = value_ob->cp_owner;
		self->cp_value = value_ob->cp_value;
		Dee_XIncref(self->cp_owner);
#ifdef CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES
	} else if (Object_IsCFunction(value)) {
		CFunction *value_ob = Object_AsCFunction(value);
		self->cp_value.pfunc = CFunction_Func(value_ob);
		Dee_Incref(value_ob);
		self->cp_owner = Dee_AsObject(value_ob);
#endif /* CONFIG_HAVE_CTYPES_FUNCTION_CLOSURES */
	} else if (DeeString_Check(value)) {
		CType *pointer_base = CPointerType_PointedToType(Dee_TYPE(self));
		if (pointer_base == &CChar_Type) {
			self->cp_value.pcvoid = DeeString_AsUtf8(value);
		} else if (pointer_base == &CWChar_Type) {
			self->cp_value.pcvoid = DeeString_AsWide(value);
		} else if (pointer_base == &CChar16_Type) {
			self->cp_value.pcvoid = DeeString_AsUtf16(value, STRING_ERROR_FREPLAC);
		} else if (pointer_base == &CChar32_Type) {
			self->cp_value.pcvoid = DeeString_AsUtf32(value);
		} else if (pointer_base == &CVoid_Type || pointer_base == &CInt8_Type || pointer_base == &CUInt8_Type) {
			self->cp_value.pcvoid = DeeString_AsBytes(value, false);
		} else if (CType_IsCArrayType(pointer_base)) {
			CArrayType *array_type = CType_AsCArrayType(pointer_base);
			CType *array_base = CArrayType_PointedToType(array_type);
			size_t string_size;
			if (array_base == &CChar_Type) {
				self->cp_value.pcvoid = DeeString_AsUtf8(value);
			} else if (array_base == &CWChar_Type) {
				self->cp_value.pcvoid = DeeString_AsWide(value);
			} else if (array_base == &CChar16_Type) {
				self->cp_value.pcvoid = DeeString_AsUtf16(value, STRING_ERROR_FREPLAC);
			} else if (array_base == &CChar32_Type) {
				self->cp_value.pcvoid = DeeString_AsUtf32(value);
			} else if (array_base == &CInt8_Type || array_base == &CUInt8_Type) {
				self->cp_value.pcvoid = DeeString_AsBytes(value, false);
			} else {
				goto err_cannot_cast_string;
			}
			if unlikely(!self->cp_value.pcvoid)
				goto err;
			string_size = WSTR_LENGTH(self->cp_value.pcvoid) + 1;
			if unlikely(string_size < CArrayType_Count(array_type)) {
				DeeError_Throwf(&DeeError_ValueError,
				                "String is too short to cast to array-pointer type %k (could "
				                "only be cast to pointer-to-arrays up to %" PRFuSIZ " elements)",
				                Dee_TYPE(self), string_size);
				goto err;
			}
			goto set_value_as_owner;
		} else {
err_cannot_cast_string:
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot cast string to pointer-type %k: only "
			                "pointer-to-char/wchar_t/char16_t/char32_t/void/int8_t/uint8_t "
			                "are accepted here",
			                Dee_TYPE(self));
			goto err;
		}
		if unlikely(!self->cp_value.pcvoid)
			goto err;
set_value_as_owner:
		Dee_Incref(value);
		self->cp_owner = value;
	} else if (DeeBytes_Check(value)) {
		CType *pointer_base = CPointerType_PointedToType(Dee_TYPE(self));
		if (DeeBytes_SIZE(value) < CType_Sizeof(pointer_base)) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot cast bytes with length %" PRFuSIZ " to pointer-type %k: "
			                "this pointer-type requires at least %" PRFuSIZ " of space",
			                DeeBytes_SIZE(value), Dee_TYPE(self), CType_Sizeof(pointer_base));
			goto err;
		}
		self->cp_value.ptr = DeeBytes_DATA(value);
		self->cp_owner = ((DeeBytesObject *)value)->b_orig;
		if (self->cp_owner == NULL)
			self->cp_owner = value;
		Dee_Incref(self->cp_owner);
	} else if (Object_IsCLValue(value)) {
		/* Special handling for lvalue->pointer */
		CLValue *value_ob = Object_AsCLValue(value);
		CType *lv_base = CLValueType_LogicalPointedToType(Dee_TYPE(value_ob));
		if (!CType_IsCPointerType(lv_base)) {
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot cast lvalue type %k to %k. Only lvalue-to-pointer can be cast here",
			                Dee_TYPE(value_ob), Dee_TYPE(self));
			goto err;
		}

		/* LValue -> Pointer (must deref) */
		self->cp_owner = NULL; /* Owner is unknown, since "value_ob->cl_value" is the owner of
		                        * the pointer, but not of whatever the pointer may point *at*. */
		CTYPES_FAULTPROTECT(self->cp_value.ptr = *(void **)CLValue_GetLogicalValue(value_ob), goto err);
	} else if (Object_IsCArray(value)) {
		/* Special handling for array->pointer
		 *
		 * Note that we don't assert that the array (as a whole) is larger than a
		 * single pointer dereference. -- C doesn't complain either if you do that,
		 * so it's up to the user to be smart about this. */
		Dee_Incref(value);
		self->cp_owner     = value;
		self->cp_value.ptr = CArray_Items(Object_AsCArray(value));
	} else {
		/* Fallback: convert object to integer */
		self->cp_owner = NULL;
		return DeeObject_AsUIntptr(value, &self->cp_value.uint);
	}
	return 0;
err:
	return -1;
}



#define cpointer_getind CPointer_Ind

PRIVATE WUNUSED NONNULL((1)) int DCALL
cpointer_delind(CPointer *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return cpointer_delitem_index(self, 0);
#else /* __OPTIMIZE_SIZE__ */
	union pointer cvalue = self->cp_value;
	size_t item_size = CPointerType_SizeofPointedToType(Dee_TYPE(self));
	CTYPES_FAULTPROTECT({
		bzero(cvalue.ptr, item_size);
	}, return -1);
	return 0;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cpointer_setind(CPointer *self, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return cpointer_setitem_index(self, 0, value);
#else /* __OPTIMIZE_SIZE__ */
	CPointerType *tp_self = Dee_TYPE(self);
	CType *base_type = CPointerType_PointedToType(tp_self);
	struct ctype_operators const *ops = CType_Operators(base_type);
	return (*ops->co_assign)(base_type, self->cp_value.ptr, value);
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE struct type_getset tpconst cpointer_getsets[] = {
	TYPE_GETSET_AB_F("ind",
	                 &cpointer_getind,
	                 &cpointer_delind,
	                 &cpointer_setind,
	                 METHOD_FNOREFESCAPE,
	                 "->?GLValue\n"
	                 "Accesses the indirection of @this pointer as an L-Value"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cpointer_members[] = {
	TYPE_MEMBER_FIELD_DOC("__owner__", STRUCT_OBJECT, offsetof(CPointer, cp_owner),
	                      "The owner of the piece of memory being pointed-to (if known)"),
	TYPE_MEMBER_END
};

INTERN CPointerType AbstractCPointer_Type = {
	/* .cpt_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CPointerType_Type),
			/* .tp_name     = */ "Pointer",
			/* .tp_doc      = */ DOC_GET(cpointer_doc),
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CType_AsType(&AbstractCObject_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CPointer,
					/* tp_ctor:        */ &cpointer_ctor,
					/* tp_copy_ctor:   */ &cpointer_copy,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ &cpointer_init_kw,
					/* tp_serialize:   */ &cpointer_serialize
				),
				/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cpointer_fini,
				/* .tp_assign      = */ NULL,
				/* .tp_move_assign = */ NULL
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
			},
			/* .tp_visit         = */ (void (DCALL *)(DeeObject *, Dee_visit_t, void *))&cpointer_visit,
			/* .tp_gc            = */ NULL,
			/* .tp_math          = */ &cpointer_math,
			/* .tp_cmp           = */ &cobject_cmp,
			/* .tp_seq           = */ &cpointer_seq,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ cpointer_getsets,
			/* .tp_members       = */ cpointer_members,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ NULL,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ NULL
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ CTYPES_sizeof_pointer,
			/* ct_alignof:   */ CTYPES_alignof_pointer,
			/* ct_operators: */ &cpointer_operators,
			/* ct_ffitype:   */ &ffi_type_pointer
		)
	},
	/* .cpt_orig = */ &AbstractCObject_Type,
	/* .cpt_size = */ 0,
};


/* Type-specific operators */

#ifndef CONFIG_NO_CFUNCTION
/************************************************************************/
/* POINTER TO FUNCTION                                                  */
/************************************************************************/

PRIVATE DeeTypeObject *tpconst cfuncpointer_subclass_mro[] = {
	CPointerType_AsType(&AbstractCPointer_Type),
	&DeeCallable_Type,
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	NULL,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cfuncpointer_call(CPointer *__restrict self, size_t argc, DeeObject *const *argv) {
	CPointerType *tp_self = Dee_TYPE(self);
	CType *base_type = CPointerType_PointedToType(tp_self);
	CFunctionType *function_type;
	ASSERT(CType_IsCFunctionType(base_type));
	function_type = CType_AsCFunctionType(base_type);
	return (*function_type->cft_call)(function_type, self->cp_value.pfunc, argc, argv);
}
#endif /* !CONFIG_NO_CFUNCTION */


/************************************************************************/
/* POINTER TO STRUCT                                                    */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
cstructpointer_getattr(CPointer *self, DeeObject *attr) {
	CStructType *struct_type;
	if (isind(attr))
		return cpointer_getind(self);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_GetAttr_p(struct_type, self->cp_value.ptr, attr, self->cp_owner);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
cstructpointer_getattr_string_hash(CPointer *self, char const *attr, Dee_hash_t hash) {
	CStructType *struct_type;
	if (isind_string_hash(attr, hash))
		return cpointer_getind(self);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_GetAttrStringHash_p(struct_type, self->cp_value.ptr, attr, hash, self->cp_owner);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
cstructpointer_getattr_string_len_hash(CPointer *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	CStructType *struct_type;
	if (isind_string_len_hash(attr, attrlen, hash))
		return cpointer_getind(self);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_GetAttrStringLenHash_p(struct_type, self->cp_value.ptr, attr, attrlen, hash, self->cp_owner);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructpointer_delattr(CPointer *self, DeeObject *attr) {
	CStructType *struct_type;
	if (isind(attr))
		return cpointer_delind(self);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_DelAttr_p(struct_type, self->cp_value.ptr, attr);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructpointer_delattr_string_hash(CPointer *self, char const *attr, Dee_hash_t hash) {
	CStructType *struct_type;
	if (isind_string_hash(attr, hash))
		return cpointer_delind(self);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_DelAttrStringHash_p(struct_type, self->cp_value.ptr, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructpointer_delattr_string_len_hash(CPointer *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	CStructType *struct_type;
	if (isind_string_len_hash(attr, attrlen, hash))
		return cpointer_delind(self);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_DelAttrStringLenHash_p(struct_type, self->cp_value.ptr, attr, attrlen, hash);
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
cstructpointer_setattr(CPointer *self, DeeObject *attr, DeeObject *value) {
	CStructType *struct_type;
	if (isind(attr))
		return cpointer_setind(self, value);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_SetAttr_p(struct_type, self->cp_value.ptr, attr, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
cstructpointer_setattr_string_hash(CPointer *self, char const *attr, Dee_hash_t hash, DeeObject *value) {
	CStructType *struct_type;
	if (isind_string_hash(attr, hash))
		return cpointer_setind(self, value);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_SetAttrStringHash_p(struct_type, self->cp_value.ptr, attr, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
cstructpointer_setattr_string_len_hash(CPointer *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value) {
	CStructType *struct_type;
	if (isind_string_len_hash(attr, attrlen, hash))
		return cpointer_setind(self, value);
	ASSERT(CType_IsCStructType(CPointerType_PointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CPointerType_PointedToType(Dee_TYPE(self)));
	return CStruct_SetAttrStringLenHash_p(struct_type, self->cp_value.ptr, attr, attrlen, hash, value);
}

PRIVATE struct type_attr cstructpointer_attr = {
	/* .tp_getattr                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cstructpointer_getattr,
	/* .tp_delattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))&cstructpointer_delattr,
	/* .tp_setattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&cstructpointer_setattr,
	/* .tp_iterattr                  = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))NULL, /*TODO: &cstruct_iterattr,*/
	/* .tp_findattr                  = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))NULL, /*TODO: &cstruct_findattr,*/
	/* .tp_hasattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstructpointer_hasattr,*/
	/* .tp_boundattr                 = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstructpointer_boundattr,*/
	/* .tp_callattr                  = */ NULL,
	/* .tp_callattr_kw               = */ NULL,
	/* .tp_vcallattrf                = */ NULL,
	/* .tp_getattr_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cstructpointer_getattr_string_hash,
	/* .tp_delattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&cstructpointer_delattr_string_hash,
	/* .tp_setattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&cstructpointer_setattr_string_hash,
	/* .tp_hasattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstructpointer_hasattr_string_hash, */
	/* .tp_boundattr_string_hash     = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstructpointer_boundattr_string_hash, */
	/* .tp_callattr_string_hash      = */ NULL,
	/* .tp_callattr_string_hash_kw   = */ NULL,
	/* .tp_vcallattr_string_hashf    = */ NULL,
	/* .tp_getattr_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cstructpointer_getattr_string_len_hash,
	/* .tp_delattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cstructpointer_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&cstructpointer_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstructpointer_hasattr_string_len_hash, */
	/* .tp_boundattr_string_len_hash = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstructpointer_boundattr_string_len_hash, */
};



PRIVATE WUNUSED NONNULL((1)) DREF CPointerType *DCALL
CPointerType_New(CType *__restrict self) {
	DREF CPointerType *result;
	result = DeeGCObject_CALLOC(CPointerType);
	if unlikely(!result)
		goto err;

	/* Initialize fields. */
	result->cpt_base.ct_sizeof       = CTYPES_sizeof_pointer;
	result->cpt_base.ct_alignof      = CTYPES_alignof_pointer;
	result->cpt_base.ct_base.tp_name = "Pointer";
	result->cpt_base.ct_base.tp_doc  = DOC_GET(cpointer_doc);
	result->cpt_base.ct_base.tp_init.tp_alloc.tp_alloc = PTR_pointer_tp_alloc;
	result->cpt_base.ct_base.tp_init.tp_alloc.tp_free  = PTR_pointer_tp_free;
	result->cpt_base.ct_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FHEAP | TP_FMOVEANY;
	Dee_Incref(CPointerType_AsType(&AbstractCPointer_Type));
	result->cpt_base.ct_base.tp_base = CPointerType_AsType(&AbstractCPointer_Type);
	result->cpt_base.ct_base.tp_cmp  = &cobject_cmp;
	result->cpt_base.ct_base.tp_math = &cpointer_math;
	result->cpt_base.ct_base.tp_seq  = &cpointer_seq;
	Dee_Incref(CType_AsType(self));
	result->cpt_orig = self; /* Inherit reference. */
	result->cpt_size = CType_Sizeof(self);
#ifndef CONFIG_NO_CFUNCTION
	result->cpt_base.ct_ffitype = &ffi_type_pointer;
#endif /* !CONFIG_NO_CFUNCTION */
	result->cpt_base.ct_operators = &cpointer_operators;

	/* Special type traits based on pointer base-class */
#ifndef CONFIG_NO_CFUNCTION
	if (CType_IsCFunctionType(self)) {
		/* Pointer-to-function is callable */
		result->cpt_base.ct_base.tp_call = (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&cfuncpointer_call;
		result->cpt_base.ct_base.tp_mro  = cfuncpointer_subclass_mro;
	} else
#endif /* !CONFIG_NO_CFUNCTION */
	{
		if (CType_IsCStructType(self)) {
			/* Pointer-to-struct allows for access to struct members */
			result->cpt_base.ct_base.tp_attr = &cstructpointer_attr;
		}
	}

	/* Finish the pointer type. */
	DeeObject_InitStatic(CPointerType_AsType(result), &CPointerType_Type);
	return Type_AsCPointerType(DeeGC_TRACK(DeeTypeObject, CPointerType_AsType(result)));
/*
err_r:
	DeeGCObject_FREE(result);
*/
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF CPointerType *DCALL
CPointerType_Of(CType *__restrict self) {
	DREF CPointerType *result;
	ASSERT_OBJECT_TYPE(CType_AsType(self), &CType_Type);
	CType_CacheLockRead(self);
	result = self->ct_pointer;
	if (result && !Dee_IncrefIfNotZero(CPointerType_AsType(result)))
		result = NULL;
	CType_CacheLockEndRead(self);
	if (!result) {
		/* Lazily construct missing types. */
		result = CPointerType_New(self);
		if likely(result) {
			CType_CacheLockWrite(self);
			/* Check if the type was created due to race conditions. */
			if unlikely(self->ct_pointer &&
			            Dee_IncrefIfNotZero(CPointerType_AsType(self->ct_pointer))) {
				DREF CPointerType *new_result;
				new_result = self->ct_pointer;
				CType_CacheLockEndWrite(self);
				Dee_DecrefDokill(CPointerType_AsType(result));
				return new_result;
			}
			self->ct_pointer = result; /* Weakly referenced. */
			CType_CacheLockEndWrite(self);
		}
	}
	return result;
}







/************************************************************************/
/* LVALUE TYPE                                                          */
/************************************************************************/

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define PTR_lvalue_tp_alloc DeeSlab_GetMalloc(sizeof(CLValue), (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(CLValue))
#define PTR_lvalue_tp_free  DeeSlab_GetFree(sizeof(CLValue), NULL)
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#define PTR_lvalue_tp_alloc (void *(DCALL *)(void))(void *)(uintptr_t)sizeof(CLValue)
#define PTR_lvalue_tp_free  NULL
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

PRIVATE NONNULL((1)) void DCALL
clvaluetype_fini(CLValueType *__restrict self) {
	CType *orig = self->clt_orig;

	/* Delete the weak-link to the original type. */
	CType_CacheLockWrite(orig);
	if (orig->ct_lvalue == self)
		orig->ct_lvalue = NULL;
	CType_CacheLockEndWrite(orig);
	Dee_Decref(CType_AsType(orig));
	Dee_Decref(CType_AsType(self->clt_logicalorig));
}

PRIVATE struct type_member tpconst clvaluetype_members[] = {
	TYPE_MEMBER_CONST_DOC("islvalue", Dee_True, DOC_GET(islvalue_doc)),
	TYPE_MEMBER_FIELD_DOC("base", STRUCT_OBJECT_AB, offsetof(CLValueType, clt_orig), "->?GCType"),
	TYPE_MEMBER_FIELD_DOC("logical_base", STRUCT_OBJECT_AB, offsetof(CLValueType, clt_orig), "->?GCType"),
	TYPE_MEMBER_FIELD_DOC("logical_ind", STRUCT_CONST | STRUCT_SIZE_T, offsetof(CLValueType, clt_logicalind),
	                      "Number of extra indirections necessary to reach ?#logical_base "
	                      /**/ "from ?#base (when ?#base is another ?GLValueType)"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject CLValueType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "LValueType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &CType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CLValueType,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clvaluetype_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
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
	/* .tp_members       = */ clvaluetype_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

STATIC_ASSERT(offsetof(CLValue, cl_owner) == offsetof(CPointer, cp_owner));
#define clvalue_fini      cpointer_fini
#define clvalue_visit     cpointer_visit
#define clvalue_copy      cpointer_copy
#define clvalue_serialize cpointer_serialize

/*[[[deemon (print_DEFINE_KWLIST from rt.gen.unpack)({ "value" });]]]*/
#ifndef DEFINED_kwlist__value
#define DEFINED_kwlist__value
PRIVATE DEFINE_KWLIST(kwlist__value, { KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEND });
#endif /* !DEFINED_kwlist__value */
/*[[[end]]]*/

DOC_DEF(clvalue_doc,
        "(value:?X5?GLValue?GStruct?GArray?Dstring?DBytes)\n"
        "Force-cast another l-value or struct into @this lvalue's type\n"
        "Note that ?Dstring is only accepted for lvalue-to-array-of-"
        /**/ "?Gchar/?Gwchar_t/?Gchar16_t/?Gchar32_t/?Gint8_t/?Guint8_t");

PRIVATE WUNUSED NONNULL((1)) int DCALL
clvalue_init_kw(CLValue *__restrict self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *value;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          "o:LValue", &value))
		goto err;
	if (Object_IsCLValue(value)) {
		/* Force-cast lvalue */
		CLValue *val = Object_AsCLValue(value);
		CLValueType *dst_type = Dee_TYPE(self);
		CLValueType *src_type = Dee_TYPE(val);
		if (dst_type->clt_logicalind > src_type->clt_logicalind) {
			/* Not allowed: target has greater indirection level than source */
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot cast lvalue type %k (with indirection level %" PRFuSIZ ") "
			                "to lvalue type %k (with a greater indirection lavel %" PRFuSIZ ")",
			                src_type, src_type->clt_logicalind,
			                dst_type, dst_type->clt_logicalind);
			goto err;
		} else {
			union pointer cvalue;
			cvalue = val->cl_value;
			if (dst_type->clt_logicalind < src_type->clt_logicalind) {
				/* Unwind (some) indirections... */
				size_t ind_delta = src_type->clt_logicalind -
				                   dst_type->clt_logicalind;
				CTYPES_FAULTPROTECT({
					do {
						cvalue.ptr = *cvalue.pptr;
					} while (--ind_delta);
				}, goto err);
			}
			self->cl_value = cvalue;
		}

		self->cl_owner = val->cl_owner;
		Dee_XIncref(self->cl_owner);
	} else if (Object_IsCStruct(value) || Object_IsCArray(value)) {
		self->cl_value.ptr = CStruct_Data(Object_AsCStruct(value));
set_value_as_owner:
		Dee_Incref(value);
		self->cl_owner = value;
	} else if (DeeString_Check(value)) {
		/* Allow casting strings to certain types of l-values:
		 * - (char[N].lvalue)string     // when "(#string.encode("utf-8") + 1) >= N"
		 * - (wchar_t[N].lvalue)string  // when "(#string.encode("wide") + 1) >= N"
		 * - (char16_t[N].lvalue)string // when "(#string.encode("utf-16") + 1) >= N"
		 * - (char32_t[N].lvalue)string // when "(#string.encode("utf-32") + 1) >= N"
		 * - (int8_t[N].lvalue)string   // when "(DeeString_SIZE(string) + 1) >= N"
		 * - (uint8_t[N].lvalue)string  // when "(DeeString_SIZE(string) + 1) >= N" */
		CType *lv_base = CLValueType_PointedToType(Dee_TYPE(self));
		CArrayType *lv_base_array;
		CType *item_type;
		void const *string_repr;
		size_t string_size;
		if (!CType_IsCArrayType(lv_base)) {
err_bad_value_type_for_string:
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot cast string to lvalue type %k: only "
			                "lvalue-to-array-of-char/wchar_t/char16_t/char32_t/int8_t/uint8_t "
			                "cant be used for this purpose",
			                Dee_TYPE(self));
			goto err;
		}
		lv_base_array = CType_AsCArrayType(lv_base);
		item_type = CArrayType_PointedToType(lv_base_array);
		if (item_type == &CChar_Type) {
			string_repr = DeeString_AsUtf8(value);
		} else if (item_type == &CWChar_Type) {
			string_repr = DeeString_AsWide(value);
		} else if (item_type == &CChar16_Type) {
			string_repr = DeeString_AsUtf16(value, STRING_ERROR_FREPLAC);
		} else if (item_type == &CChar32_Type) {
			string_repr = DeeString_AsUtf32(value);
		} else if (item_type == &CInt8_Type || item_type == &CUInt8_Type) {
			string_repr = DeeString_STR(value);
		} else {
			goto err_bad_value_type_for_string;
		}
		if unlikely(!string_repr)
			goto err;
		string_size = WSTR_LENGTH(string_repr) + 1;
		if unlikely(string_size < CArrayType_Count(lv_base_array)) {
			DeeError_Throwf(&DeeError_ValueError,
			                "String is too short to cast to lvalue type %k (could only "
			                "be cast to lvalue-to-arrays up to %" PRFuSIZ " elements)",
			                Dee_TYPE(self), string_size);
			goto err;
		}
		self->cl_value.ptr = (void *)string_repr;
		goto set_value_as_owner;
	} else if (DeeBytes_Check(value)) {
		size_t bytes_size, lvalue_size;
		/* Allow casting all types of (writable) bytes to l-values */
		if unlikely(!DeeBytes_IsWritable(value)) {
			err_bytes_not_writable((DeeBytesObject *)value);
			goto err;
		}
		bytes_size  = DeeBytes_SIZE(value);
		lvalue_size = CType_Sizeof(CLValueType_PointedToType(Dee_TYPE(self)));
		if unlikely(bytes_size < lvalue_size) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Bytes too small (%" PRFuSIZ " supplied) to cast to %k "
			                /**/ "(which requires at least %" PRFuSIZ " bytes)",
			                bytes_size, Dee_TYPE(self), lvalue_size);
			goto err;
		}
		self->cl_value.ptr = (void *)DeeBytes_DATA(value);
		self->cl_owner = ((DeeBytesObject *)value)->b_orig;
		if (self->cl_owner == NULL)
			self->cl_owner = value;
		Dee_Incref(self->cl_owner);
	} else {
		goto err_bad_value_type;
	}
	return 0;
err_bad_value_type:
	DeeObject_TypeAssertFailed3(value,
	                            CLValueType_AsType(&AbstractCLValue_Type),
	                            CStructType_AsType(&AbstractCStruct_Type),
	                            CArrayType_AsType(&AbstractCArray_Type));
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
clvalue_assign(CLValue *self, DeeObject *value) {
	union pointer cvalue;
	CType *base_type = CLValueType_LogicalPointedToType(Dee_TYPE(self));
	struct ctype_operators const *ops = CType_Operators(base_type);
	CTYPES_FAULTPROTECT(cvalue.ptr = CLValue_GetLogicalValue(self), return -1);
	return (*ops->co_assign)(base_type, cvalue.ptr, value);
}

INTERN WUNUSED NONNULL((1)) DREF CPointer *DCALL
CLValue_Ptr(CLValue *__restrict self) {
	DREF CPointer *result;
	CLValueType *tp_self;
	CType *base_type;
	DREF CPointerType *pointer_type;
	result = CPointer_Alloc();
	if unlikely(!result)
		goto err;
	/* Always return a pointer to the *logical* base:
	 * >> struct A { .x = int };
	 * >> struct B { .x = int.lvalue };
	 * >> local a = A(x: 10);
	 * >> local b = B(x: a.x);
	 * >> assert b.x.ptr == a.x.ptr; // "ptr" of lvalue-to-lvalue must return pointer to logical base! */
#if 1
	CTYPES_FAULTPROTECT({
		result->cp_value.ptr = CLValue_GetLogicalValue(self);
	}, goto err_r);
	tp_self      = Dee_TYPE(self);
	base_type    = CLValueType_LogicalPointedToType(tp_self);
	pointer_type = CPointerType_Of(base_type);
	if unlikely(!pointer_type)
		goto err_r;
#else
	tp_self      = Dee_TYPE(self);
	base_type    = CLValueType_PointedToType(tp_self);
	pointer_type = CPointerType_Of(base_type);
	if unlikely(!pointer_type)
		goto err_r;
	result->cp_value = self->cl_value;
#endif
	Dee_XIncref(self->cl_owner);
	result->cp_owner = self->cl_owner;
	CPointer_InitInherited(result, pointer_type);
	return result;
err_r:
	CPointer_Free(result);
err:
	return NULL;
}

/* Special hook to override behavior of "tp_new_copy" for LValue types:
 * This causes the "copy" operator (when applied to L-Values) to return
 * a copy of the referenced value in the form of an R-Value, rather than
 * return a copy of the L-Value itself. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF CObject *DCALL
clvalue_new_copy(CLValueType *tp_self, CLValue *self) {
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	DREF CObject *result;
	if unlikely(CType_IsCFunctionType(base_type))
		goto err_cannot_copy_function;
	result = CObject_Alloc(base_type);
	if unlikely(!result)
		goto err;
	CObject_Init(result, base_type);
	CTYPES_FAULTPROTECT({
		void *value = CLValueType_LogicalPtr(tp_self, CLValue_GetValue(self));
		result = (*CType_Operators(base_type)->co_initobject)(result, value);
	}, {
		CObject_Free(base_type, result);
		goto err;
	});
	return result;
err_cannot_copy_function:
	DeeError_Throwf(&DeeError_TypeError,
	                "Cannot copy lvalue-to-function %k",
	                tp_self);
err:
	return NULL;
}

#define clvalue_getptr CLValue_Ptr
PRIVATE struct type_getset tpconst clvalue_getsets[] = {
	TYPE_GETTER_AB_F("ptr", &clvalue_getptr, METHOD_FNOREFESCAPE,
	                 "->?GPointer\n"
	                 "Returns a pointer to the value represented by @this L-Value"),
	TYPE_GETTER_AB_F("ref", &clvalue_getptr, METHOD_FNOREFESCAPE,
	                 "->?GPointer\n"
	                 "Deprecated alias for ?#ptr"),
	TYPE_GETSET_END
};


#ifdef CONFIG_NO_DEEMON_100_COMPAT
#define clvalue_methods NULL
#else /* CONFIG_NO_DEEMON_100_COMPAT */
PRIVATE WUNUSED NONNULL((1)) DREF CPointer *DCALL
clvalue_legacy_ref_func(CLValue *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__ref__");
	return CLValue_Ptr(self);
err:
	return NULL;
}
PRIVATE struct type_method tpconst clvalue_methods[] = {
	TYPE_METHOD("__ref__", &clvalue_legacy_ref_func,
	            "->?GPointer\n"
	            "Deprecated alias for ?#ref"),
	TYPE_METHOD_END
};
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

STATIC_ASSERT(offsetof(CLValue, cl_owner) == offsetof(CPointer, cp_owner));
#define clvalue_members cpointer_members


INTERN CLValueType AbstractCLValue_Type = {
	/* .clt_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CLValueType_Type),
			/* .tp_name     = */ "LValue",
			/* .tp_doc      = */ DOC_GET(clvalue_doc),
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CType_AsType(&AbstractCObject_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CLValue,
					/* tp_ctor:        */ NULL, /* Not raw-constructible */
					/* tp_copy_ctor:   */ &clvalue_copy,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ &clvalue_init_kw, /* For casting... */
					/* tp_serialize:   */ &clvalue_serialize
				),
				/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clvalue_fini,
				/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&clvalue_assign,
				/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&clvalue_assign,
				/* .tp_new         = */ NULL,
				/* .tp_new_kw      = */ NULL,
				/* .tp_new_copy    = */ (DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))&clvalue_new_copy,
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
			},
			/* .tp_visit         = */ (void (DCALL *)(DeeObject *, Dee_visit_t, void *))&clvalue_visit,
			/* .tp_gc            = */ NULL,
			/* .tp_math          = */ &cobject_math,
			/* .tp_cmp           = */ &cobject_cmp,
			/* .tp_seq           = */ NULL,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ clvalue_methods,
			/* .tp_getsets       = */ clvalue_getsets,
			/* .tp_members       = */ clvalue_members,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ NULL,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ NULL
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ CTYPES_sizeof_lvalue,
			/* ct_alignof:   */ CTYPES_alignof_lvalue,
			/* ct_operators: */ &clvalue_operators,
			/* ct_ffitype:   */ &ffi_type_pointer
		)
	},
	/* .clt_orig = */ &AbstractCObject_Type
};



/* Type-specific operators */

#ifndef CONFIG_NO_CFUNCTION
/************************************************************************/
/* LVALUE OF FUNCTION                                                   */
/************************************************************************/
PRIVATE DeeTypeObject *tpconst cfunclvalue_subclass_mro[] = {
	CLValueType_AsType(&AbstractCLValue_Type),
	&DeeCallable_Type,
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	NULL,
};

STATIC_ASSERT(offsetof(CLValue, cl_value) == offsetof(CPointer, cp_value));
STATIC_ASSERT(offsetof(CLValueType, clt_orig) == offsetof(CPointerType, cpt_orig));
#define cfunclvalue_call_0 cfuncpointer_call

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cfunclvalue_call(CLValue *__restrict self, size_t argc, DeeObject *const *argv) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	CFunctionType *function_type;
	union pointer cvalue;
	ASSERT(CType_IsCFunctionType(base_type));
	function_type = CType_AsCFunctionType(base_type);
	CTYPES_FAULTPROTECT(cvalue.ptr = CLValue_GetLogicalValue(self), return NULL);
	return (*function_type->cft_call)(function_type, cvalue.pfunc, argc, argv);
}

#endif /* !CONFIG_NO_CFUNCTION */


/************************************************************************/
/* LVALUE OF STRUCT                                                     */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cstructlvalue_getattr(CLValue *self, DeeObject *attr) {
	void *cvalue;
	CStructType *struct_type;
	if (isptr(attr))
		return Dee_AsObject(clvalue_getptr(self));
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return NULL);
	return Dee_AsObject(CStruct_GetAttr_p(struct_type, cvalue, attr, self->cl_owner));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cstructlvalue_getattr_string_hash(CLValue *self, char const *attr, Dee_hash_t hash) {
	void *cvalue;
	CStructType *struct_type;
	if (isptr_string_hash(attr, hash))
		return Dee_AsObject(clvalue_getptr(self));
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return NULL);
	return Dee_AsObject(CStruct_GetAttrStringHash_p(struct_type, cvalue, attr, hash, self->cl_owner));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cstructlvalue_getattr_string_len_hash(CLValue *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr_string_len_hash(attr, attrlen, hash))
		return Dee_AsObject(clvalue_getptr(self));
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return NULL);
	return Dee_AsObject(CStruct_GetAttrStringLenHash_p(struct_type, cvalue, attr, attrlen, hash, self->cl_owner));
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructlvalue_delattr(CLValue *self, DeeObject *attr) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr(attr))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_DEL);
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return -1);
	return CStruct_DelAttr_p(struct_type, cvalue, attr);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructlvalue_delattr_string_hash(CLValue *self, char const *attr, Dee_hash_t hash) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr_string_hash(attr, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_DEL);
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return -1);
	return CStruct_DelAttrStringHash_p(struct_type, cvalue, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructlvalue_delattr_string_len_hash(CLValue *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr_string_len_hash(attr, attrlen, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_DEL);
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return -1);
	return CStruct_DelAttrStringLenHash_p(struct_type, cvalue, attr, attrlen, hash);
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
cstructlvalue_setattr(CLValue *self, DeeObject *attr, DeeObject *value) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr(attr))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_SET);
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return -1);
	return CStruct_SetAttr_p(struct_type, cvalue, attr, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
cstructlvalue_setattr_string_hash(CLValue *self, char const *attr, Dee_hash_t hash, DeeObject *value) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr_string_hash(attr, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_SET);
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return -1);
	return CStruct_SetAttrStringHash_p(struct_type, cvalue, attr, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
cstructlvalue_setattr_string_len_hash(CLValue *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value) {
	CStructType *struct_type;
	void *cvalue;
	if (isptr_string_len_hash(attr, attrlen, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_SET);
	ASSERT(CType_IsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self))));
	struct_type = CType_AsCStructType(CLValueType_LogicalPointedToType(Dee_TYPE(self)));
	CTYPES_FAULTPROTECT(cvalue = CLValue_GetLogicalValue(self), return -1);
	return CStruct_SetAttrStringLenHash_p(struct_type, cvalue, attr, attrlen, hash, value);
}

PRIVATE struct type_attr cstructlvalue_attr = {
	/* .tp_getattr                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cstructlvalue_getattr,
	/* .tp_delattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))&cstructlvalue_delattr,
	/* .tp_setattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&cstructlvalue_setattr,
	/* .tp_iterattr                  = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))NULL, /*TODO: &cstruct_iterattr,*/
	/* .tp_findattr                  = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))NULL, /*TODO: &cstruct_findattr,*/
	/* .tp_hasattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstructlvalue_hasattr,*/
	/* .tp_boundattr                 = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstructlvalue_boundattr,*/
	/* .tp_callattr                  = */ NULL,
	/* .tp_callattr_kw               = */ NULL,
	/* .tp_vcallattrf                = */ NULL,
	/* .tp_getattr_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cstructlvalue_getattr_string_hash,
	/* .tp_delattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&cstructlvalue_delattr_string_hash,
	/* .tp_setattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&cstructlvalue_setattr_string_hash,
	/* .tp_hasattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstructlvalue_hasattr_string_hash, */
	/* .tp_boundattr_string_hash     = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstructlvalue_boundattr_string_hash, */
	/* .tp_callattr_string_hash      = */ NULL,
	/* .tp_callattr_string_hash_kw   = */ NULL,
	/* .tp_vcallattr_string_hashf    = */ NULL,
	/* .tp_getattr_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cstructlvalue_getattr_string_len_hash,
	/* .tp_delattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cstructlvalue_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&cstructlvalue_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstructlvalue_hasattr_string_len_hash, */
	/* .tp_boundattr_string_len_hash = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstructlvalue_boundattr_string_len_hash, */
};


/************************************************************************/
/* LVALUE OF POINTER TO STRUCT                                          */
/************************************************************************/

#define cstructpointerlvalue_getstruct(self)                                                                                          \
	(ASSERT(CType_IsCPointerType(CLValueType_LogicalPointedToType(Dee_TYPE(self)))),                                                  \
	 ASSERT(CType_IsCStructType(CPointerType_PointedToType(CType_AsCPointerType(CLValueType_LogicalPointedToType(Dee_TYPE(self)))))), \
	 CType_AsCStructType(CPointerType_PointedToType(CType_AsCPointerType(CLValueType_LogicalPointedToType(Dee_TYPE(self))))))

PRIVATE WUNUSED NONNULL((1)) DREF CLValue *DCALL
cpointerlvalue_getind(CLValue *__restrict self);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cstructpointerlvalue_getattr(CLValue *self, DeeObject *attr) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr(attr))
		return Dee_AsObject(clvalue_getptr(self));
	if (isind(attr))
		return Dee_AsObject(cpointerlvalue_getind(self));
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return NULL);
	return Dee_AsObject(CStruct_GetAttr_p(struct_type, cvalue.ptr, attr, NULL));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cstructpointerlvalue_getattr_string_hash(CLValue *self, char const *attr, Dee_hash_t hash) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr_string_hash(attr, hash))
		return Dee_AsObject(clvalue_getptr(self));
	if (isind_string_hash(attr, hash))
		return Dee_AsObject(cpointerlvalue_getind(self));
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return NULL);
	return Dee_AsObject(CStruct_GetAttrStringHash_p(struct_type, cvalue.ptr, attr, hash, NULL));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cstructpointerlvalue_getattr_string_len_hash(CLValue *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr_string_len_hash(attr, attrlen, hash))
		return Dee_AsObject(clvalue_getptr(self));
	if (isind_string_len_hash(attr, attrlen, hash))
		return Dee_AsObject(cpointerlvalue_getind(self));
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return NULL);
	return Dee_AsObject(CStruct_GetAttrStringLenHash_p(struct_type, cvalue.ptr, attr, attrlen, hash, NULL));
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructpointerlvalue_delattr(CLValue *self, DeeObject *attr) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr(attr))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_DEL);
	if (isind(attr))
		return DeeRT_ErrRestrictedAttrCStr(self, "ind", DeeRT_ATTRIBUTE_ACCESS_DEL);
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return CStruct_DelAttr_p(struct_type, cvalue.ptr, attr);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructpointerlvalue_delattr_string_hash(CLValue *self, char const *attr, Dee_hash_t hash) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr_string_hash(attr, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_DEL);
	if (isind_string_hash(attr, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ind", DeeRT_ATTRIBUTE_ACCESS_DEL);
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return CStruct_DelAttrStringHash_p(struct_type, cvalue.ptr, attr, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cstructpointerlvalue_delattr_string_len_hash(CLValue *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr_string_len_hash(attr, attrlen, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_DEL);
	if (isind_string_len_hash(attr, attrlen, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ind", DeeRT_ATTRIBUTE_ACCESS_DEL);
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return CStruct_DelAttrStringLenHash_p(struct_type, cvalue.ptr, attr, attrlen, hash);
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
cstructpointerlvalue_setattr(CLValue *self, DeeObject *attr, DeeObject *value) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr(attr))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_SET);
	if (isind(attr))
		return DeeRT_ErrRestrictedAttrCStr(self, "ind", DeeRT_ATTRIBUTE_ACCESS_SET);
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return CStruct_SetAttr_p(struct_type, cvalue.ptr, attr, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
cstructpointerlvalue_setattr_string_hash(CLValue *self, char const *attr, Dee_hash_t hash, DeeObject *value) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr_string_hash(attr, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_SET);
	if (isind_string_hash(attr, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ind", DeeRT_ATTRIBUTE_ACCESS_SET);
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return CStruct_SetAttrStringHash_p(struct_type, cvalue.ptr, attr, hash, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
cstructpointerlvalue_setattr_string_len_hash(CLValue *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value) {
	CStructType *struct_type;
	union pointer cvalue;
	if (isptr_string_len_hash(attr, attrlen, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ptr", DeeRT_ATTRIBUTE_ACCESS_SET);
	if (isind_string_len_hash(attr, attrlen, hash))
		return DeeRT_ErrRestrictedAttrCStr(self, "ind", DeeRT_ATTRIBUTE_ACCESS_SET);
	struct_type = cstructpointerlvalue_getstruct(self);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return CStruct_SetAttrStringLenHash_p(struct_type, cvalue.ptr, attr, attrlen, hash, value);
}

PRIVATE struct type_attr cstructpointerlvalue_attr = {
	/* .tp_getattr                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cstructpointerlvalue_getattr,
	/* .tp_delattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))&cstructpointerlvalue_delattr,
	/* .tp_setattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&cstructpointerlvalue_setattr,
	/* .tp_iterattr                  = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))NULL, /*TODO: &cstruct_iterattr,*/
	/* .tp_findattr                  = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))NULL, /*TODO: &cstruct_findattr,*/
	/* .tp_hasattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstructpointerlvalue_hasattr,*/
	/* .tp_boundattr                 = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL, /*TODO: &cstructpointerlvalue_boundattr,*/
	/* .tp_callattr                  = */ NULL,
	/* .tp_callattr_kw               = */ NULL,
	/* .tp_vcallattrf                = */ NULL,
	/* .tp_getattr_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cstructpointerlvalue_getattr_string_hash,
	/* .tp_delattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&cstructpointerlvalue_delattr_string_hash,
	/* .tp_setattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&cstructpointerlvalue_setattr_string_hash,
	/* .tp_hasattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstructpointerlvalue_hasattr_string_hash, */
	/* .tp_boundattr_string_hash     = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL, /*TODO: &cstructpointerlvalue_boundattr_string_hash, */
	/* .tp_callattr_string_hash      = */ NULL,
	/* .tp_callattr_string_hash_kw   = */ NULL,
	/* .tp_vcallattr_string_hashf    = */ NULL,
	/* .tp_getattr_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cstructpointerlvalue_getattr_string_len_hash,
	/* .tp_delattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cstructpointerlvalue_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&cstructpointerlvalue_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstructpointerlvalue_hasattr_string_len_hash, */
	/* .tp_boundattr_string_len_hash = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))NULL, /*TODO: &cstructpointerlvalue_boundattr_string_len_hash, */
};




/************************************************************************/
/* LVALUE OF ARRAY                                                      */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
carraylvalue_size(CLValue *__restrict self) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	CArrayType *array_type = CType_AsCArrayType(base_type);
	ASSERT(CType_IsCArrayType(base_type));
	return CArrayType_Count(array_type);
}

PRIVATE WUNUSED NONNULL((1)) DREF CLValue *DCALL
carraylvalue_getitem_index_fast(CLValue *__restrict self, size_t index) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	CArrayType *array_type = CType_AsCArrayType(base_type);
	CType *array_item_type;
	DREF CLValue *result;
	DREF CLValueType *item_lvalue_type;
	union pointer res_cvalue;
	ASSERT(CType_IsCArrayType(base_type));
	array_item_type = CArrayType_PointedToType(array_type);
	index *= CType_Sizeof(array_item_type);
	CTYPES_FAULTPROTECT(res_cvalue.ptr = CLValue_GetLogicalValue(self), goto err);
	res_cvalue.sint += index;
	item_lvalue_type = CLValueType_Of(array_item_type);
	if unlikely(!item_lvalue_type)
		goto err;
	result = CLValue_Alloc();
	if unlikely(!result)
		goto err_item_lvalue_type;
	result->cl_value = res_cvalue;
	Dee_XIncref(self->cl_owner);
	result->cl_owner = self->cl_owner;
	CLValue_InitInherited(result, item_lvalue_type);
	return result;
err_item_lvalue_type:
	Dee_Decref_unlikely(CLValueType_AsType(item_lvalue_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF CLValue *DCALL
carraylvalue_getitem_index(CLValue *__restrict self, size_t index) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	CArrayType *array_type = CType_AsCArrayType(base_type);
	if unlikely(index >= CArrayType_Count(array_type))
		goto err_index_oob;
	return carraylvalue_getitem_index_fast(self, index);
err_index_oob:
	DeeRT_ErrIndexOutOfBounds(self, index, CArrayType_Count(array_type));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
carraylvalue_delitem_index(CLValue *__restrict self, size_t index) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	CArrayType *array_type = CType_AsCArrayType(base_type);
	CType *array_item_type;
	union pointer res_cvalue;
	ASSERT(CType_IsCArrayType(base_type));
	array_item_type = CArrayType_PointedToType(array_type);
	index *= CType_Sizeof(array_item_type);
	CTYPES_FAULTPROTECT({
		res_cvalue.ptr = CLValue_GetLogicalValue(self);
		res_cvalue.sint += index;
		bzero(res_cvalue.ptr, CType_Sizeof(array_item_type));
	}, return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
carraylvalue_setitem_index(CLValue *self, size_t index, DeeObject *value) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *base_type = CLValueType_LogicalPointedToType(tp_self);
	CArrayType *array_type = CType_AsCArrayType(base_type);
	CType *array_item_type;
	struct ctype_operators const *array_item_type_operators;
	union pointer res_cvalue;
	ASSERT(CType_IsCArrayType(base_type));
	array_item_type = CArrayType_PointedToType(array_type);
	array_item_type_operators = CType_Operators(array_item_type);
	index *= CType_Sizeof(array_item_type);
	CTYPES_FAULTPROTECT(res_cvalue.ptr = CLValue_GetLogicalValue(self), return -1);
	res_cvalue.sint += index;
	return (*array_item_type_operators->co_assign)(array_item_type, res_cvalue.ptr, value);
}

PRIVATE DeeTypeObject *tpconst carraylvalue_subclass_mro[] = {
	CLValueType_AsType(&AbstractCLValue_Type),
	&DeeSeq_Type,
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	NULL,
};

PRIVATE struct type_seq carraylvalue_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&carraylvalue_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&carraylvalue_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&carraylvalue_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&carraylvalue_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&carraylvalue_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&carraylvalue_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))NULL, /* TODO: &carraylvalue_hasitem_index */
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))NULL, /* TODO: &carraylvalue_hasitem_index */
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,  /* TODO: &carraylvalue_getrange_index */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,              /* TODO: &carraylvalue_delrange_index */
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))NULL, /* TODO: &carraylvalue_setrange_index */
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))NULL,               /* TODO: &carraylvalue_getrange_index_n */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))NULL,                           /* TODO: &carraylvalue_delrange_index_n */
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))NULL,              /* TODO: &carraylvalue_setrange_index_n */
};


#ifndef CONFIG_NO_CFUNCTION
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cfuncpointerlvalue_call(CLValue *__restrict self, size_t argc, DeeObject *const *argv) {
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base_type = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type;
	CType *pointer_base_type;
	CFunctionType *function_type;
	union pointer cvalue;
	ASSERT(CType_IsCPointerType(lvalue_base_type));
	pointer_type = CType_AsCPointerType(lvalue_base_type);
	pointer_base_type = CPointerType_PointedToType(pointer_type);
	ASSERT(CType_IsCFunctionType(pointer_base_type));
	function_type = CType_AsCFunctionType(pointer_base_type);
	CTYPES_FAULTPROTECT(cvalue.pfunc = *(Dee_funptr_t *)CLValue_GetLogicalValue(self), return NULL);
	return (*function_type->cft_call)(function_type, cvalue.pfunc, argc, argv);
}
#endif /* !CONFIG_NO_CFUNCTION */


/* Return an l-value to an offset to the indirection of the pointer referenced by "self" */
PRIVATE WUNUSED NONNULL((1)) DREF CLValue *DCALL
cpointerlvalue_getitem_index(CLValue *__restrict self, ptrdiff_t index) {
	union pointer cvalue;
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type = (ASSERT(CType_IsCPointerType(lvalue_base)), CType_AsCPointerType(lvalue_base));
	CType *pointer_base = CPointerType_PointedToType(pointer_type);
	DREF CLValue *result;
	DREF CLValueType *pointer_base_lvalue;
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), goto err);
	result = CLValue_Alloc();
	if unlikely(!result)
		goto err;
	index *= CType_Sizeof(pointer_base);
	pointer_base_lvalue = CLValueType_Of(pointer_base);
	if unlikely(!pointer_base_lvalue)
		goto err_r;
	cvalue.sint += index;
	result->cl_value = cvalue;

	/* No owner -- this is an l-value of a free-standing pointer (which
	 * we just happened to have an L-value to), however the owner of this
	 * data would be whatever structure that pointer points into, which
	 * is something we don't know. */
	result->cl_owner = NULL;
	CLValue_InitInherited(result, pointer_base_lvalue);
	return result;
err_r:
	CLValue_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cpointerlvalue_delitem_index(CLValue *__restrict self, ptrdiff_t index) {
	union pointer cvalue;
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type = (ASSERT(CType_IsCPointerType(lvalue_base)), CType_AsCPointerType(lvalue_base));
	CType *pointer_base = CPointerType_PointedToType(pointer_type);
	size_t base_size = CType_Sizeof(pointer_base);
	index *= base_size;
	CTYPES_FAULTPROTECT({
		cvalue.ptr = *(void **)CLValue_GetLogicalValue(self);
		cvalue.sint += index;
		bzero(cvalue.ptr, base_size);
	}, return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
cpointerlvalue_setitem_index(CLValue *self, ptrdiff_t index, DeeObject *value) {
	union pointer cvalue;
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type = (ASSERT(CType_IsCPointerType(lvalue_base)), CType_AsCPointerType(lvalue_base));
	CType *pointer_base = CPointerType_PointedToType(pointer_type);
	struct ctype_operators const *pointer_base_operators = CType_Operators(pointer_base);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	index *= CType_Sizeof(pointer_base);
	cvalue.sint += index;
	return (*pointer_base_operators->co_assign)(pointer_base, cvalue.ptr, value);
}



/* Return an l-value to an offset to the indirection of the pointer referenced by "self" */
PRIVATE WUNUSED NONNULL((1, 2)) DREF CLValue *DCALL
cpointerlvalue_getitem(CLValue *__restrict self, DeeObject *index) {
	ptrdiff_t index_value;
	if (DeeObject_AsPtrdiff(index, &index_value))
		goto err;
	return cpointerlvalue_getitem_index(self, index_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cpointerlvalue_delitem(CLValue *__restrict self, DeeObject *index) {
	ptrdiff_t index_value;
	if (DeeObject_AsPtrdiff(index, &index_value))
		goto err;
	return cpointerlvalue_delitem_index(self, index_value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
cpointerlvalue_setitem(CLValue *self, DeeObject *index, DeeObject *value) {
	ptrdiff_t index_value;
	if (DeeObject_AsPtrdiff(index, &index_value))
		goto err;
	return cpointerlvalue_setitem_index(self, index_value, value);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF CLValue *DCALL
cpointerlvalue_getind(CLValue *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return cpointerlvalue_getitem_index(self, 0);
#else /* __OPTIMIZE_SIZE__ */
	union pointer cvalue;
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type = (ASSERT(CType_IsCPointerType(lvalue_base)), CType_AsCPointerType(lvalue_base));
	CType *pointer_base = CPointerType_PointedToType(pointer_type);
	DREF CLValue *result;
	DREF CLValueType *pointer_base_lvalue;
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), goto err);
	result = CLValue_Alloc();
	if unlikely(!result)
		goto err;
	pointer_base_lvalue = CLValueType_Of(pointer_base);
	if unlikely(!pointer_base_lvalue)
		goto err_r;
	result->cl_value = cvalue;

	/* No owner -- this is an l-value of a free-standing pointer (which
	 * we just happened to have an L-value to), however the owner of this
	 * data would be whatever structure that pointer points into, which
	 * is something we don't know. */
	result->cl_owner = NULL;
	CLValue_InitInherited(result, pointer_base_lvalue);
	return result;
err_r:
	CLValue_Free(result);
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cpointerlvalue_delind(CLValue *__restrict self) {
#ifdef __OPTIMIZE_SIZE__
	return cpointerlvalue_delitem_index(self, 0);
#else /* __OPTIMIZE_SIZE__ */
	union pointer cvalue;
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type = (ASSERT(CType_IsCPointerType(lvalue_base)), CType_AsCPointerType(lvalue_base));
	CType *pointer_base = CPointerType_PointedToType(pointer_type);
	size_t base_size = CType_Sizeof(pointer_base);
	CTYPES_FAULTPROTECT({
		cvalue.ptr = *(void **)CLValue_GetLogicalValue(self);
		bzero(cvalue.ptr, base_size);
	}, return -1);
	return 0;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cpointerlvalue_setind(CLValue *self, DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return cpointerlvalue_setitem_index(self, 0, value);
#else /* __OPTIMIZE_SIZE__ */
	union pointer cvalue;
	CLValueType *tp_self = Dee_TYPE(self);
	CType *lvalue_base = CLValueType_LogicalPointedToType(tp_self);
	CPointerType *pointer_type = (ASSERT(CType_IsCPointerType(lvalue_base)), CType_AsCPointerType(lvalue_base));
	CType *pointer_base = CPointerType_PointedToType(pointer_type);
	struct ctype_operators const *pointer_base_operators = CType_Operators(pointer_base);
	CTYPES_FAULTPROTECT(cvalue.ptr = *(void **)CLValue_GetLogicalValue(self), return -1);
	return (*pointer_base_operators->co_assign)(pointer_base, cvalue.ptr, value);
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE struct type_getset tpconst cpointerlvalue_getsets[] = {
	TYPE_GETSET_AB_F("ind",
	                 &cpointerlvalue_getind,
	                 &cpointerlvalue_delind,
	                 &cpointerlvalue_setind,
	                 METHOD_FNOREFESCAPE,
	                 "->?GLValue\n"
	                 "Accesses the indirection of the pointer referenced by this l-value"),
	TYPE_GETSET_END
};

PRIVATE struct type_seq cpointerlvalue_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cpointerlvalue_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cpointerlvalue_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&cpointerlvalue_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cpointerlvalue_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&cpointerlvalue_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&cpointerlvalue_setitem_index,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,  /* TODO: &cpointerlvalue_getrange_index (return lvalue-to-array) */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))NULL,              /* TODO: &cpointerlvalue_delrange_index */
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))NULL, /* TODO: &cpointerlvalue_setrange_index */
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))NULL,               /* TODO: &cpointerlvalue_getrange_index_n (return lvalue-to-array) */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))NULL,                           /* TODO: &cpointerlvalue_delrange_index_n */
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))NULL,              /* TODO: &cpointerlvalue_setrange_index_n */
};

PRIVATE DeeTypeObject *tpconst cnumericlvalue_subclass_mro[] = {
	CLValueType_AsType(&AbstractCLValue_Type),
	&DeeNumeric_Type,
	CType_AsType(&AbstractCObject_Type),
	&DeeObject_Type,
	NULL,
};

PRIVATE WUNUSED NONNULL((1)) void *DFCALL
clvalue_getlogicalptr_0(CLValueType const *tp_self, void *ptr) {
	(void)tp_self;
	return ptr;
}

PRIVATE WUNUSED NONNULL((1)) void *DFCALL
clvalue_getlogicalptr_1(CLValueType const *tp_self, void *ptr) {
	(void)tp_self;
	return *(void **)ptr;
}

PRIVATE WUNUSED NONNULL((1)) void *DFCALL
clvalue_getlogicalptr_n(CLValueType const *tp_self, void *ptr) {
	size_t n = tp_self->clt_logicalind;
	do {
		ptr = *(void **)ptr;
	} while (--n);
	return ptr;
}


PRIVATE WUNUSED NONNULL((1)) DREF CLValueType *DCALL
CLValueType_New(CType *__restrict self) {
	DREF CLValueType *result;
	size_t extra_indirections;
	result = DeeGCObject_CALLOC(CLValueType);
	if unlikely(!result)
		goto err;

	/* Initialize fields. */
	result->clt_base.ct_sizeof       = CTYPES_sizeof_lvalue;
	result->clt_base.ct_alignof      = CTYPES_alignof_lvalue;
	result->clt_base.ct_base.tp_name = "LValue";
	result->clt_base.ct_base.tp_doc  = DOC_GET(clvalue_doc);
	result->clt_base.ct_base.tp_init.tp_alloc.tp_alloc = PTR_lvalue_tp_alloc;
	result->clt_base.ct_base.tp_init.tp_alloc.tp_free  = PTR_lvalue_tp_free;
	result->clt_base.ct_base.tp_init.tp_new_copy = (DREF DeeObject *(DCALL *)(DeeTypeObject *, DeeObject *))&clvalue_new_copy;
	result->clt_base.ct_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FHEAP | TP_FMOVEANY;
	Dee_Incref(CLValueType_AsType(&AbstractCLValue_Type));
	result->clt_base.ct_base.tp_base = CLValueType_AsType(&AbstractCLValue_Type);
	result->clt_base.ct_base.tp_math = &cobject_math;
	result->clt_base.ct_base.tp_cmp  = &cobject_cmp;
	Dee_Incref(CType_AsType(self));
	result->clt_orig = self; /* Inherit reference. */
#ifndef CONFIG_NO_CFUNCTION
	result->clt_base.ct_ffitype = &ffi_type_pointer;
#endif /* !CONFIG_NO_CFUNCTION */
	result->clt_base.ct_operators = &clvalue_operators;

	/* Determine the "logical" lvalue base type (and how many extra indirections to get there) */
	for (extra_indirections = 0; CType_IsCLValueType(self);
	     self = CLValueType_PointedToType(CType_AsCLValueType(self)))
		++extra_indirections;
	Dee_Incref(CType_AsType(self));
	result->clt_logicalorig = self;
	result->clt_logicalind  = extra_indirections;
	if (extra_indirections == 0) {
		result->clt_getlogicalptr = &clvalue_getlogicalptr_0;
	} else if (extra_indirections == 1) {
		result->clt_getlogicalptr = &clvalue_getlogicalptr_1;
	} else {
		result->clt_getlogicalptr = &clvalue_getlogicalptr_n;
	}

	/* Special type traits based on lvalue base-class */
#ifndef CONFIG_NO_CFUNCTION
	if (CType_IsCFunctionType(self)) {
		/* LValue-to-function is callable */
		result->clt_base.ct_base.tp_call = extra_indirections == 0
		                                   ? (DREF DeeObject * (DCALL *)(DeeObject *, size_t, DeeObject *const *))&cfunclvalue_call_0
		                                   : (DREF DeeObject * (DCALL *)(DeeObject *, size_t, DeeObject *const *))&cfunclvalue_call;
		result->clt_base.ct_base.tp_mro  = cfunclvalue_subclass_mro;
	} else
#endif /* !CONFIG_NO_CFUNCTION */
	{
		if (CType_IsCStructType(self)) {
			/* LValue-to-struct allows for access to struct members */
			result->clt_base.ct_base.tp_attr = &cstructlvalue_attr;
		} else if (CType_IsCArrayType(self)) {
			/* LValue-to-array allows for access to array elements
			 *
			 * NOTE: At this point, we slightly differ from regular C:
			 *       Since arrays in C act like pointers, and since we
			 *       allow 1 level of pointer indirection to happen
			 *       automatically when accessing struct members, we'd
			 *       technically need expose attribute access to the
			 *       array's first element, if the array's element
			 *       types are CStructType.
			 * However, that would completely conflict with deemon's
			 * native sequence API, which array types already expose.
			 * Plus: I don't really see any advantage, since you can
			 *       just access one of the array's elements and get
			 *       an L-Value that *can* be used to access struct
			 *       members. */
			result->clt_base.ct_base.tp_seq = &carraylvalue_seq;
			result->clt_base.ct_base.tp_mro = carraylvalue_subclass_mro;
		} else if (CType_IsCPointerType(self)) {
			CPointerType *ptr = CType_AsCPointerType(self);
			CType *ptr_base   = CPointerType_PointedToType(ptr);
#ifndef CONFIG_NO_CFUNCTION
			if (CType_IsCFunctionType(ptr_base)) {
				/* LValue-to-pointer-to-function allows that function to be called, too
				 * This is especially important when the function is part of a struct,
				 * in which case accessing a struct member would always return an l-value
				 * to a pointer, to a function (which has to be callable by itself) */
				result->clt_base.ct_base.tp_call = (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&cfuncpointerlvalue_call;
				result->clt_base.ct_base.tp_mro  = cfunclvalue_subclass_mro;
			} else
#endif /* !CONFIG_NO_CFUNCTION */
			{
				if (CType_IsCStructType(ptr_base)) {
					/* LValue-to-pointer-to-struct can also directly access struct fields */
					result->clt_base.ct_base.tp_attr = &cstructpointerlvalue_attr;
				}
			}
			/* LValue-to-pointer allows for array-style indexing */
			result->clt_base.ct_base.tp_seq     = &cpointerlvalue_seq;
			result->clt_base.ct_base.tp_getsets = cpointerlvalue_getsets;
		} else if (CType_IsCLValueType(self)) {
			/* TODO: LValue-of-lvalue sounds weird, but is easily possible
			 *       when accessing a struct member that is itself another
			 *       l-value! */
		} else if (DeeType_Implements(CType_AsType(self), &DeeNumeric_Type)) {
			/* Retain (and expose) helper methods from "Numeric" if the referenced object is a number */
			result->clt_base.ct_base.tp_mro = cnumericlvalue_subclass_mro;
		}
	}

	/* Finish the lvalue type. */
	DeeObject_InitStatic(CLValueType_AsType(result), &CLValueType_Type);
	return Type_AsCLValueType(DeeGC_TRACK(DeeTypeObject, CLValueType_AsType(result)));
/*
err_r:
	DeeGCObject_FREE(result);*/
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF CLValueType *DCALL
CLValueType_Of(CType *__restrict self) {
	DREF CLValueType *result;
	ASSERT_OBJECT_TYPE(CType_AsType(self), &CType_Type);
	CType_CacheLockRead(self);
	result = self->ct_lvalue;
	if (result && !Dee_IncrefIfNotZero(CLValueType_AsType(result)))
		result = NULL;
	CType_CacheLockEndRead(self);
	if (!result) {
		/* Lazily construct missing types. */
		result = CLValueType_New(self);
		if likely(result) {
			CType_CacheLockWrite(self);
			/* Check if the type was created due to race conditions. */
			if unlikely(self->ct_lvalue &&
			            Dee_IncrefIfNotZero(CLValueType_AsType(self->ct_lvalue))) {
				DREF CLValueType *new_result;
				new_result = self->ct_lvalue;
				CType_CacheLockEndWrite(self);
				Dee_DecrefDokill(CLValueType_AsType(result));
				return new_result;
			}
			self->ct_lvalue = result; /* Weakly referenced. */
			CType_CacheLockEndWrite(self);
		}
	}
	return result;
}

DECL_END

/* Define built-in types */
#ifndef __INTELLISENSE__
#define DEFINE_CVoid_Type
#include "ctypes-builtin.c.inl"

#define DEFINE_CChar_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CWChar_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CChar16_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CChar32_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBool_Type
#include "ctypes-builtin.c.inl"

#define DEFINE_CInt8_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CInt16_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CInt32_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CInt64_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CInt128_Type
#include "ctypes-builtin.c.inl"

#define DEFINE_CUInt8_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUInt16_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUInt32_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUInt64_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUInt128_Type
#include "ctypes-builtin.c.inl"

#define DEFINE_CBSwapInt16_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapInt32_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapInt64_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapInt128_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapUInt16_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapUInt32_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapUInt64_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CBSwapUInt128_Type
#include "ctypes-builtin.c.inl"

#define DEFINE_CFloat_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CDouble_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CLDouble_Type
#include "ctypes-builtin.c.inl"

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
#define DEFINE_CSChar_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUChar_Type
#include "ctypes-builtin.c.inl"
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
#define DEFINE_CShort_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUShort_Type
#include "ctypes-builtin.c.inl"
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
#define DEFINE_CInt_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CUInt_Type
#include "ctypes-builtin.c.inl"
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
#define DEFINE_CLong_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CULong_Type
#include "ctypes-builtin.c.inl"
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
#define DEFINE_CLLong_Type
#include "ctypes-builtin.c.inl"
#define DEFINE_CULLong_Type
#include "ctypes-builtin.c.inl"
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

#ifdef CTYPES_DEFINE_STATIC_POINTER_TYPES
INTERN CPointerType CVoidPtr_Type = {
	/* .cpt_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CPointerType_Type),
			/* .tp_name     = */ "Pointer",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CPointerType_AsType(&AbstractCPointer_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CPointer,
					/* tp_ctor:        */ &cpointer_ctor,
					/* tp_copy_ctor:   */ &cpointer_copy,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ &cpointer_init_kw,
					/* tp_serialize:   */ &cpointer_serialize
				),
				/* .tp_dtor        = */ NULL,
				/* .tp_assign      = */ NULL,
				/* .tp_move_assign = */ NULL
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
			},
			/* .tp_visit         = */ (void (DCALL *)(DeeObject *, Dee_visit_t, void *))&cpointer_visit,
			/* .tp_gc            = */ NULL,
			/* .tp_math          = */ &cpointer_math,
			/* .tp_cmp           = */ &cobject_cmp,
			/* .tp_seq           = */ &cpointer_seq,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ cpointer_getsets,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ NULL,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ NULL
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ CTYPES_sizeof_pointer,
			/* ct_alignof:   */ CTYPES_alignof_pointer,
			/* ct_operators: */ &cpointer_operators,
			/* ct_ffitype:   */ &ffi_type_pointer
		)
	},
	/* .cpt_orig = */ &CVoid_Type,
	/* .cpt_size = */ 0,
};

INTERN CPointerType CCharPtr_Type = {
	/* .cpt_base = */ {
		/* .ct_base = */ {
			OBJECT_HEAD_INIT(&CPointerType_Type),
			/* .tp_name     = */ "Pointer",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ CPointerType_AsType(&AbstractCPointer_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
					/* T:              */ CPointer,
					/* tp_ctor:        */ &cpointer_ctor,
					/* tp_copy_ctor:   */ &cpointer_copy,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ &cpointer_init_kw,
					/* tp_serialize:   */ &cpointer_serialize
				),
				/* .tp_dtor        = */ NULL,
				/* .tp_assign      = */ NULL,
				/* .tp_move_assign = */ NULL
			},
			/* .tp_cast = */ {
				/* .tp_str       = */ NULL,
				/* .tp_repr      = */ NULL,
				/* .tp_bool      = */ (int (DCALL *)(DeeObject *))&cobject_bool,
				/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_print,
				/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&cobject_printrepr,
			},
			/* .tp_visit         = */ (void (DCALL *)(DeeObject *, Dee_visit_t, void *))&cpointer_visit,
			/* .tp_gc            = */ NULL,
			/* .tp_math          = */ &cpointer_math,
			/* .tp_cmp           = */ &cobject_cmp,
			/* .tp_seq           = */ &cpointer_seq,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ cpointer_getsets,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call          = */ NULL,
			/* .tp_callable      = */ NULL,
			/* .tp_mro           = */ NULL
		},
		CTYPE_INIT_COMMON(
			/* ct_sizeof:    */ CTYPES_sizeof_pointer,
			/* ct_alignof:   */ CTYPES_alignof_pointer,
			/* ct_operators: */ &cpointer_operators,
			/* ct_ffitype:   */ &ffi_type_pointer
		)
	},
	/* .cpt_orig = */ &CChar_Type,
	/* .cpt_size = */ CTYPES_sizeof_char,
};
#endif /* CTYPES_DEFINE_STATIC_POINTER_TYPES */

DECL_END

#endif /* !GUARD_DEX_CTYPES_CTYPES_CORE_C */
