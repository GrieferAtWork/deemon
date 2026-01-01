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
#ifndef GUARD_DEX_CTYPES_CFUNCTION_C
#define GUARD_DEX_CTYPES_CFUNCTION_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/callable.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/util/lock.h>

#include <hybrid/debug-alignment.h>
#include <hybrid/minmax.h>
#include <hybrid/sequence/list.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* intN_t, uintN_t */

DECL_BEGIN


#ifdef CONFIG_NO_CFUNCTION
INTERN WUNUSED NONNULL((1)) ctypes_cc_t DCALL
cc_trylookup(char const *__restrict UNUSED(name)) {
	return CC_INVALID;
}

INTERN WUNUSED char const *DCALL
cc_getname(ctypes_cc_t UNUSED(cc)) {
	return NULL;
}
#else /* CONFIG_NO_CFUNCTION */

struct cc_entry {
	char        name[12];
	ctypes_cc_t cc;
};

#ifndef CONFIG_HAVE_SYSTEM_FFI
#ifdef X86_WIN32
#define CONFIG_HAVE_FFI_SYSV
#define CONFIG_HAVE_FFI_STDCALL
#define CONFIG_HAVE_FFI_THISCALL
#define CONFIG_HAVE_FFI_FASTCALL
#define CONFIG_HAVE_FFI_MS_CDECL
#define CONFIG_HAVE_FFI_PASCAL
#define CONFIG_HAVE_FFI_REGISTER
#elif defined(X86_WIN64)
#define CONFIG_HAVE_FFI_WIN64
#else /* ... */
#define CONFIG_HAVE_FFI_SYSV
#define CONFIG_HAVE_FFI_UNIX64
#define CONFIG_HAVE_FFI_THISCALL
#define CONFIG_HAVE_FFI_FASTCALL
#define CONFIG_HAVE_FFI_STDCALL
#define CONFIG_HAVE_FFI_PASCAL
#define CONFIG_HAVE_FFI_REGISTER
#endif /* !... */
#endif /* !CONFIG_HAVE_SYSTEM_FFI */

PRIVATE struct cc_entry const cc_db[] = {
#ifdef CONFIG_HAVE_FFI_SYSV
	{ "sysv", FFI_SYSV },
#endif /* CONFIG_HAVE_FFI_SYSV */
#ifdef CONFIG_HAVE_FFI_STDCALL
	{ "stdcall", FFI_STDCALL },
#endif /* CONFIG_HAVE_FFI_STDCALL */
#ifdef CONFIG_HAVE_FFI_THISCALL
	{ "thiscall", FFI_THISCALL },
#endif /* CONFIG_HAVE_FFI_THISCALL */
#ifdef CONFIG_HAVE_FFI_FASTCALL
	{ "fastcall", FFI_FASTCALL },
#endif /* CONFIG_HAVE_FFI_FASTCALL */
#ifdef CONFIG_HAVE_FFI_MS_CDECL
	{ "ms_cdecl", FFI_MS_CDECL },
#endif /* CONFIG_HAVE_FFI_MS_CDECL */
#ifdef CONFIG_HAVE_FFI_PASCAL
	{ "pascal", FFI_PASCAL },
#endif /* CONFIG_HAVE_FFI_PASCAL */
#ifdef CONFIG_HAVE_FFI_REGISTER
	{ "register", FFI_REGISTER },
#endif /* CONFIG_HAVE_FFI_REGISTER */
#ifdef CONFIG_HAVE_FFI_WIN64
	{ "win64", FFI_WIN64 },
#endif /* CONFIG_HAVE_FFI_WIN64 */
#ifdef CONFIG_HAVE_FFI_UNIX64
	{ "unix64", FFI_UNIX64 },
#endif /* CONFIG_HAVE_FFI_UNIX64 */
#ifdef CONFIG_HAVE_FFI_GNUW64
	{ "gnuw64", FFI_GNUW64 },
#endif /* CONFIG_HAVE_FFI_GNUW64 */
#ifdef CONFIG_HAVE_FFI_EFI64
	{ "efi64", FFI_EFI64 },
#endif /* CONFIG_HAVE_FFI_EFI64 */
	{ "", FFI_DEFAULT_ABI },
	{ "default", FFI_DEFAULT_ABI },
};

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */


INTERN WUNUSED NONNULL((1)) ctypes_cc_t DCALL
cc_trylookup(char const *__restrict name) {
	size_t i;
	/* Search for a calling convention matching the given name. */
	for (i = 0; i < COMPILER_LENOF(cc_db); ++i) {
		if (strcmp(cc_db[i].name, name) == 0)
			return cc_db[i].cc;
	}
	return CC_INVALID;
}

INTERN WUNUSED char const *DCALL
cc_getname(ctypes_cc_t cc) {
	size_t i;
	/* Search for the database entry for the given CC. */
	for (i = 0; i < COMPILER_LENOF(cc_db); ++i) {
		if (cc_db[i].cc == cc)
			return cc_db[i].name;
	}
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) ffi_type *DCALL
stype_ffitype(DeeSTypeObject *__restrict self) {
	if (self->st_ffitype)
		return self->st_ffitype;
	/* TODO: Lazily allocate struct descriptors. */

	DeeError_NOTIMPLEMENTED();
	return NULL;
}

#endif /* !CONFIG_NO_CFUNCTION */

INTERN WUNUSED NONNULL((1)) ctypes_cc_t DCALL
cc_lookup(char const *__restrict name) {
#ifdef CONFIG_NO_CFUNCTION
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized calling convention %q",
	                name);
	return CC_INVALID;
#else /* CONFIG_NO_CFUNCTION */
	ctypes_cc_t result = cc_trylookup(name);
	if (result == CC_INVALID) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Unrecognized calling convention %q",
		                name);
	}
	return result;
#endif /* !CONFIG_NO_CFUNCTION */
}


PRIVATE DeeTypeObject *tpconst function_mro[] = {
	DeeSType_AsType(&DeeStructured_Type),
	&DeeCallable_Type,
	&DeeObject_Type,
	NULL
};


INTERN DeeCFunctionTypeObject DeeCFunction_Type = {
	/* .ft_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT(&DeeCFunctionType_Type),
			/* .tp_name     = */ "Function",
			/* .tp_doc      = */ NULL,
			/* Don't inherit constructors, because this type cannot be constructed.
			 * The only way that function types can be used is through pointers or l-values. */
			/* .tp_flags    = */ TP_FNORMAL /*|TP_FINHERITCTOR*/ | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ DeeSType_AsType(&DeeStructured_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
					/* T:              */ DeeObject,
					/* tp_ctor:        */ NULL,
					/* tp_copy_ctor:   */ NULL,
					/* tp_deep_ctor:   */ NULL,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ NULL,
					/* tp_serialize:   */ NULL /* TODO */
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
			/* .tp_seq           = */ NULL,
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
			/* .tp_mro           = */ function_mro
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
		/* .st_align    = */ CONFIG_CTYPES_ALIGNOF_POINTER,
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
	},
#ifndef CONFIG_NO_CFUNCTION
	/* .ft_orig  = */ &DeeStructured_Type,
	/* .ft_chain = */ { NULL, NULL },
	/* .ft_hash  = */ 0,
	/* .ft_argc  = */ 0,
	/* .ft_argv  = */ NULL
	/* ... libffi (left uninitialized) */
#endif /* !CONFIG_NO_CFUNCTION */
};

#ifndef CONFIG_NO_CFUNCTION
PRIVATE WUNUSED DREF DeeObject *DCALL
generate_function_name(DeeSTypeObject *__restrict return_type,
                       ctypes_cc_t calling_convention, size_t argc,
                       DeeSTypeObject **__restrict argv) {
	size_t i;
	char const *cc_name;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (DeeObject_Print(DeeSType_AsObject(return_type),
	                    &ascii_printer_print,
	                    &printer) < 0)
		goto err;
	if (calling_convention != CC_DEFAULT) {
		if (ASCII_PRINTER_PRINT(&printer, " ") < 0)
			goto err;
		cc_name = cc_getname(calling_convention);
		if (!cc_name)
			cc_name = "?";
		if (ascii_printer_print(&printer, cc_name, strlen(cc_name)) < 0)
			goto err;
	}
	if (ASCII_PRINTER_PRINT(&printer, "(") < 0)
		goto err;
	if (!argc) {
		if (ASCII_PRINTER_PRINT(&printer, "void") < 0)
			goto err;
	} else {
		for (i = 0; i < argc; ++i) {
			if (i != 0 && ASCII_PRINTER_PRINT(&printer, ", ") < 0)
				goto err;
			if (DeeObject_Print(DeeSType_AsObject(argv[i]),
			                    &ascii_printer_print,
			                    &printer) < 0)
				goto err;
		}
	}
	if (ASCII_PRINTER_PRINT(&printer, ")") < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}

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
};


#ifndef __INTELLISENSE__
#define VARARGS
#include "cfunction-invoke.c.inl"
#include "cfunction-invoke.c.inl"
#endif /* !__INTELLISENSE__ */


PRIVATE WUNUSED DREF DeeCFunctionTypeObject *DCALL
cfunctiontype_new(DeeSTypeObject *__restrict return_type,
                  ctypes_cc_t calling_convention, size_t argc,
                  DeeSTypeObject **__restrict argv,
                  Dee_hash_t function_hash, bool inherit_argv) {
	DREF DeeCFunctionTypeObject *result;
	DREF DeeStringObject *name;
	DREF DeeSTypeObject **argv_copy;
	size_t i;
	if (inherit_argv) {
		argv_copy = argv; /* Inherit */
		Dee_Increfv((DeeObject **)argv_copy, argc);
	} else if (!argc) {
		argv_copy = NULL;
	} else {
		argv_copy = (DREF DeeSTypeObject **)Dee_Mallocc(argc, sizeof(DREF DeeSTypeObject *));
		if unlikely(!argv_copy)
			goto err;
#ifndef NDEBUG
		for (i = 0; i < argc; ++i)
			ASSERT_OBJECT_TYPE(DeeSType_AsType(argv[i]), &DeeSType_Type);
#endif /* !NDEBUG */
		Dee_Movrefv((DeeObject **)argv_copy, (DeeObject **)argv, argc);
	}

	result = DeeGCObject_CALLOC(DeeCFunctionTypeObject);
	if unlikely(!result)
		goto err_argv;

	/* Create the name of the resulting type. */
	name = (DREF DeeStringObject *)generate_function_name(return_type, calling_convention, argc, argv);
	if unlikely(!name)
		goto err_argv_r;

	/* Store a reference to the cfunction-base type. */
	Dee_Incref(DeeSType_AsType(return_type));
	Dee_Incref(DeeCFunctionType_AsType(&DeeCFunction_Type));

	/* Initialize fields. */
	result->ft_orig = return_type; /* Inherit reference. */
	result->ft_base.st_base.tp_init.tp_alloc.tp_instance_size = sizeof(DeeObject);
	result->ft_base.st_base.tp_name  = DeeString_STR(name); /* Inherit reference. */
	result->ft_base.st_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FNAMEOBJECT | TP_FHEAP | TP_FMOVEANY;
	result->ft_base.st_base.tp_base  = DeeCFunctionType_AsType(&DeeCFunction_Type); /* Inherit reference. */
	result->ft_hash                  = function_hash;
	result->ft_argc                  = argc;
	result->ft_argv                  = argv_copy; /* Inherit */

	/* Assign the proper call operator. */
#ifndef __INTELLISENSE__
	result->ft_base.st_call = (((unsigned int)calling_convention & (unsigned int)CC_FVARARGS)
	                           ? (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&cfunction_call_v
	                           : (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&cfunction_call);
#endif /* !__INTELLISENSE__ */

	/* Collect all the type descriptors used by libffi. */
	result->ft_ffi_return_type = stype_ffitype(return_type);
	if unlikely(!result->ft_ffi_return_type)
		goto err_argv_r_name;
	result->ft_ffi_arg_type_v = (ffi_type **)Dee_Mallocc(argc, sizeof(ffi_type *));
	if unlikely(!result->ft_ffi_arg_type_v)
		goto err_argv_r_name;
	for (i = 0; i < argc; ++i) {
		ffi_type *tp = stype_ffitype(argv_copy[i]);
		if unlikely(!tp)
			goto err_argv_r_name_ffi_typev;
		result->ft_ffi_arg_type_v[i] = tp;
	}

	{
		/* Initialize the FFI closure controller. */
		ffi_status error;
		DBG_ALIGNMENT_DISABLE();
		error = ffi_prep_cif(&result->ft_ffi_cif,
		                     (ffi_abi)((unsigned int)calling_convention &
		                               (unsigned int)CC_MTYPE),
		                     (unsigned int)argc,
		                     result->ft_ffi_return_type,
		                     result->ft_ffi_arg_type_v);
		DBG_ALIGNMENT_ENABLE();
		if (error != FFI_OK) {
			DeeError_Throwf(&DeeError_RuntimeError,
			                "Failed to create function closure (%d)",
			                (int)error);
			goto err_argv_r_name_ffi_typev;
		}
	}

	/* Calculate the wbuffer cache sizes */
	result->ft_wsize       = MAX(result->ft_ffi_return_type->size, sizeof(ffi_arg));
	result->ft_woff_argmem = result->ft_wsize;
	if ((unsigned int)calling_convention &
	    (unsigned int)CC_FVARARGS) {
		result->ft_woff_variadic_argmem = result->ft_wsize + (sizeof(union argument) * argc);
	} else {
		result->ft_wsize += (sizeof(union argument) * argc);
		result->ft_woff_argptr = result->ft_wsize;
		result->ft_wsize += (sizeof(void *) * argc);
	}
	if (!result->ft_wsize)
		result->ft_wsize = 1;
	if (!argc)
		result->ft_woff_argmem = 0,
		result->ft_woff_argptr = 0;

	/* Finalize the cfunction type. */
	DeeObject_Init(DeeCFunctionType_AsType(result), &DeeCFunctionType_Type);
	return DeeType_AsCFunctionType(DeeGC_TRACK(DeeTypeObject, DeeCFunctionType_AsType(result)));
err_argv_r_name_ffi_typev:
	Dee_Free(result->ft_ffi_arg_type_v);
err_argv_r_name:
	Dee_Decref(name);
err_argv_r:
	DeeGCObject_FREE(result);
err_argv:
	Dee_Decrefv((DREF DeeObject **)argv_copy, argc);
	Dee_Free(argv_copy);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
stype_cfunction_rehash(DeeSTypeObject *__restrict self,
                       Dee_hash_t new_mask) {
	struct cfunction_type_list *new_map, *dst;
	struct cfunction_type_list *biter, *bend;
	DeeCFunctionTypeObject *iter, *next;
again:
	new_map = (struct cfunction_type_list *)Dee_TryCallocc(new_mask + 1,
	                                                       sizeof(struct cfunction_type_list));
	if unlikely(!new_map) {
		/* Try again with a 1-element mask. */
		if (!self->st_cfunction.sf_list && new_mask != 0) {
			new_mask = 1;
			goto again;
		}
		return false;
	}

	/* Do the re-hash. */
	if (self->st_cfunction.sf_size) {
		ASSERT(self->st_cfunction.sf_list);
		bend = (biter = self->st_cfunction.sf_list) + (self->st_cfunction.sf_mask + 1);
		for (; biter < bend; ++biter) {
			iter = LIST_FIRST(biter);
			while (iter) {
				next = LIST_NEXT(iter, ft_chain);
				dst  = &new_map[iter->ft_hash & new_mask];
				/* Insert the entry into the new hash-map. */
				LIST_INSERT_HEAD(dst, iter, ft_chain);
				iter = next;
			}
		}
	}

	/* Delete the old map and install the new. */
	Dee_Free(self->st_cfunction.sf_list);
	self->st_cfunction.sf_mask = new_mask;
	self->st_cfunction.sf_list = new_map;
	return true;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
cfunction_hashof(DeeSTypeObject *__restrict return_type,
                 ctypes_cc_t calling_convention, size_t argc,
                 DeeSTypeObject *const *argv) {
	size_t i;
	Dee_hash_t result;
	result = Dee_HashPointer(return_type) ^ (Dee_hash_t)calling_convention ^ (Dee_hash_t)argc;
	for (i = 0; i < argc; ++i)
		result ^= Dee_HashPointer(argv[i]);
	return result;
}

PRIVATE bool DCALL
cfunction_equals(DeeCFunctionTypeObject *__restrict self,
                 DeeSTypeObject *__restrict return_type,
                 ctypes_cc_t calling_convention, size_t argc,
                 DeeSTypeObject *const *argv) {
	size_t i;
	if (self->ft_orig != return_type)
		goto nope;
	if (self->ft_argc != argc)
		goto nope;
	if (self->ft_cc != calling_convention)
		goto nope;
	for (i = 0; i < argc; ++i) {
		if (self->ft_argv[i] != argv[i])
			goto nope;
	}
	return true;
nope:
	return false;
}


#endif /* !CONFIG_NO_CFUNCTION */


/* Construct a C-function structured type that returns
 * an instance of `return_type' while taking `argc' arguments,
 * each of type `argv' when called using `calling_convention'
 * @param: calling_convention: One of `FFI_*' (Defaults to `CC_DEFAULT')
 * @param: inherit_argv: When `true', _always_ inherit the `argv' vector (even upon error)
 *                       Note however, that vector elements are not
 *                       inherited (as denoted by the lack of a DREF tag).
 * When `ctypes' has been built with `CONFIG_NO_CFUNCTION',
 * this function throws a NotImplemented error. */
INTERN WUNUSED NONNULL((1)) DREF DeeCFunctionTypeObject *DCALL
DeeSType_CFunction(DeeSTypeObject *__restrict return_type,
                   ctypes_cc_t calling_convention, size_t argc,
                   DeeSTypeObject **argv, bool inherit_argv) {
#ifdef CONFIG_NO_CFUNCTION
	(void)return_type;
	(void)calling_convention;
	(void)argc;
	(void)argv;
	if (inherit_argv)
		Dee_Free(argv);
	err_no_cfunction();
	return NULL;
#else /* CONFIG_NO_CFUNCTION */
	Dee_hash_t hash;
	DREF DeeCFunctionTypeObject *result, *new_result;
	DREF struct cfunction_type_list *bucket;
	ASSERT_OBJECT_TYPE(DeeSType_AsType(return_type), &DeeSType_Type);
	DeeSType_CacheLockRead(return_type);
	ASSERT(!return_type->st_cfunction.sf_size ||
	       return_type->st_cfunction.sf_mask);
	hash = cfunction_hashof(return_type, calling_convention, argc, argv);
	if (return_type->st_cfunction.sf_size) {
		result = LIST_FIRST(&return_type->st_cfunction.sf_list[hash & return_type->st_cfunction.sf_mask]);
		while (result &&
		       (result->ft_hash != hash ||
		        !cfunction_equals(result, return_type, calling_convention, argc, argv)))
			result = LIST_NEXT(result, ft_chain);
		/* Check if we can re-use an existing type. */
		if (result && Dee_IncrefIfNotZero(DeeCFunctionType_AsType(result))) {
			DeeSType_CacheLockEndRead(return_type);
			if (inherit_argv)
				Dee_Free(argv);
			return result;
		}
	}
	DeeSType_CacheLockEndRead(return_type);

	/* Construct a new cfunction type. */
	result = cfunctiontype_new(return_type, calling_convention,
	                           argc, argv, hash, inherit_argv);
	if unlikely(!result)
		goto done;

	/* Add the new type to the cache. */
register_type:
	DeeSType_CacheLockWrite(return_type);
	ASSERT(!return_type->st_cfunction.sf_size ||
	       return_type->st_cfunction.sf_mask);
	if (return_type->st_cfunction.sf_size) {
		new_result = LIST_FIRST(&return_type->st_cfunction.sf_list[hash & return_type->st_cfunction.sf_mask]);
		while (new_result &&
		       (new_result->ft_hash != hash ||
		        !cfunction_equals(new_result, return_type, calling_convention, argc, argv)))
			new_result = LIST_NEXT(new_result, ft_chain);

		/* Check if we can re-use an existing type. */
		if (new_result && Dee_IncrefIfNotZero(DeeCFunctionType_AsType(new_result))) {
			DeeSType_CacheLockEndRead(return_type);
			Dee_Decref(DeeCFunctionType_AsType(result));
			return new_result;
		}
	}

	/* Rehash when there are a lot of items. */
	if (return_type->st_cfunction.sf_size >= return_type->st_cfunction.sf_mask &&
	    (!stype_cfunction_rehash(return_type, return_type->st_cfunction.sf_mask ? (return_type->st_cfunction.sf_mask << 1) | 1 : 16 - 1) &&
	     !return_type->st_cfunction.sf_mask)) {

		/* No space at all! */
		DeeSType_CacheLockEndWrite(return_type);

		/* Collect enough memory for a 1-item hash map. */
		if (Dee_CollectMemory(sizeof(DeeCFunctionTypeObject *)))
			goto register_type;

		/* Failed to allocate the initial hash-map. */
		Dee_Decref(DeeCFunctionType_AsType(result));
		result = NULL;
		goto done;
	}

	/* Insert the new cfunction type into the hash-map. */
	bucket = &return_type->st_cfunction.sf_list[hash & return_type->st_cfunction.sf_mask];
	LIST_INSERT_HEAD(bucket, result, ft_chain); /* Weak reference. */
	DeeSType_CacheLockEndWrite(return_type);
done:
	return result;
#endif /* !CONFIG_NO_CFUNCTION */
}

DECL_END

#endif /* !GUARD_DEX_CTYPES_CFUNCTION_C */
