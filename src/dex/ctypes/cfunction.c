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
#ifndef GUARD_DEX_CTYPES_CFUNCTION_C
#define GUARD_DEX_CTYPES_CFUNCTION_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/super.h>

#include <hybrid/minmax.h>

DECL_BEGIN


#ifdef CONFIG_NO_CFUNCTION
INTERN cc_t DCALL
cc_trylookup(char const *__restrict UNUSED(name)) {
	return CC_INVALID;
}

INTERN char const *DCALL
cc_getname(cc_t UNUSED(cc)) {
	return NULL;
}
#else /* CONFIG_NO_CFUNCTION */

struct cc_entry {
	char name[12];
	cc_t cc;
};

PRIVATE struct cc_entry const cc_db[] = {
#ifdef X86_WIN32
	{ "sysv", FFI_SYSV },
	{ "stdcall", FFI_STDCALL },
	{ "thiscall", FFI_THISCALL },
	{ "fastcall", FFI_FASTCALL },
	{ "ms_cdecl", FFI_MS_CDECL },
	{ "pascal", FFI_PASCAL },
	{ "register", FFI_REGISTER },
#elif defined(X86_WIN64)
	{ "win64", FFI_WIN64 },
#else
	{ "sysv", FFI_SYSV },
	{ "unix64", FFI_UNIX64 },
	{ "thiscall", FFI_THISCALL },
	{ "fastcall", FFI_FASTCALL },
	{ "stdcall", FFI_STDCALL },
	{ "pascal", FFI_PASCAL },
	{ "register", FFI_REGISTER },
#endif
	{ "", FFI_DEFAULT_ABI },
	{ "default", FFI_DEFAULT_ABI },
};


INTERN cc_t DCALL
cc_trylookup(char const *__restrict name) {
	size_t i;
	/* Search for a calling convention matching the given name. */
	for (i = 0; i < COMPILER_LENOF(cc_db); ++i) {
		if (strcmp(cc_db[i].name, name) == 0)
			return cc_db[i].cc;
	}
	return CC_INVALID;
}

INTERN char const *DCALL
cc_getname(cc_t cc) {
	size_t i;
	/* Search for the database entry for the given CC. */
	for (i = 0; i < COMPILER_LENOF(cc_db); ++i) {
		if (cc_db[i].cc == cc)
			return cc_db[i].name;
	}
	return NULL;
}

PRIVATE ffi_type *DCALL
stype_ffitype(DeeSTypeObject *__restrict self) {
	if (self->st_ffitype)
		return self->st_ffitype;
	/* TODO: Lazily allocate struct descriptors. */

	DERROR_NOTIMPLEMENTED();
	return NULL;
}

#endif /* !CONFIG_NO_CFUNCTION */

INTERN cc_t DCALL
cc_lookup(char const *__restrict name) {
#ifdef CONFIG_NO_CFUNCTION
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized calling convention %q",
	                name);
	return CC_INVALID;
#else /* CONFIG_NO_CFUNCTION */
	cc_t result = cc_trylookup(name);
	if (result == CC_INVALID) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Unrecognized calling convention %q",
		                name);
	}
	return result;
#endif /* !CONFIG_NO_CFUNCTION */
}



INTERN DeeCFunctionTypeObject DeeCFunction_Type = {
	/* .ft_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT((DeeTypeObject *)&DeeCFunctionType_Type),
			/* .tp_name     = */ "Function",
			/* .tp_doc      = */ NULL,
			/* Don't inherit constructors, because this type cannot be constructed.
			 * The only way that function types can be used is through pointers or l-values. */
			/* .tp_flags    = */ TP_FNORMAL /*|TP_FINHERITCTOR*/ | TP_FTRUNCATE | TP_FMOVEANY,
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
						TYPE_FIXED_ALLOCATOR_S(DeeObject)
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
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ NULL,
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
		/* .st_ffitype  = */ &ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
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
                       cc_t calling_convention, size_t argc,
                       DeeSTypeObject **__restrict argv) {
	size_t i;
	char const *cc_name;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (DeeObject_Print((DeeObject *)return_type,
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
			if (DeeObject_Print((DeeObject *)argv[i],
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
                  cc_t calling_convention, size_t argc,
                  DeeSTypeObject **__restrict argv,
                  dhash_t function_hash, bool inherit_argv) {
	DREF DeeCFunctionTypeObject *result;
	DREF DeeStringObject *name;
	DREF DeeSTypeObject **argv_copy;
	size_t i;
	if (inherit_argv) {
		argv_copy = argv; /* Inherit */
		for (i = 0; i < argc; ++i)
			Dee_Incref((DeeObject *)argv_copy[i]);
	} else if (!argc) {
		argv_copy = NULL;
	} else {
		argv_copy = (DREF DeeSTypeObject **)Dee_Malloc(argc * sizeof(DREF DeeSTypeObject *));
		if unlikely(!argv_copy)
			goto err;
		for (i = 0; i < argc; ++i) {
			ASSERT_OBJECT_TYPE((DeeObject *)argv[i], &DeeSType_Type);
			Dee_Incref((DeeObject *)argv[i]);
			argv_copy[i] = argv[i];
		}
	}

	result = DeeGCObject_CALLOC(DeeCFunctionTypeObject);
	if unlikely(!result)
		goto err_argv;
	/* Create the name of the resulting type. */
	name = (DREF DeeStringObject *)generate_function_name(return_type, calling_convention, argc, argv);
	if unlikely(!name)
		goto err_argv_r;
	/* Store a reference to the cfunction-base type. */
	Dee_Incref((DeeObject *)return_type);
	Dee_Incref((DeeObject *)&DeeCFunction_Type);
	/* Initialize fields. */
	result->ft_orig = return_type; /* Inherit reference. */
	result->ft_base.st_base.tp_init.tp_alloc.tp_instance_size += sizeof(DeeObject);
	result->ft_base.st_base.tp_name  = DeeString_STR(name); /* Inherit reference. */
	result->ft_base.st_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FNAMEOBJECT | TP_FHEAP | TP_FMOVEANY;
	result->ft_base.st_base.tp_base  = &DeeCFunction_Type.ft_base.st_base; /* Inherit reference. */
	result->ft_hash                  = function_hash;
	result->ft_argc                  = argc;
	result->ft_argv                  = argv_copy; /* Inherit */

	/* Assign the proper call operator. */
#ifndef __INTELLISENSE__
	result->ft_base.st_call = (((unsigned int)calling_convention & (unsigned int)CC_FVARARGS)
	                           ? (DREF DeeObject *(DCALL *)(DeeSTypeObject * __restrict, void *, size_t, DeeObject **__restrict))&cfunction_call_v
	                           : (DREF DeeObject *(DCALL *)(DeeSTypeObject * __restrict, void *, size_t, DeeObject **__restrict))&cfunction_call);
#endif /* !__INTELLISENSE__ */

	/* Collect all the type descriptors used by libffi. */
	result->ft_ffi_return_type = stype_ffitype(return_type);
	if unlikely(!result->ft_ffi_return_type)
		goto err_argv_r_name;
	result->ft_ffi_arg_type_v = (ffi_type **)Dee_Malloc(argc * sizeof(ffi_type *));
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
	DeeObject_Init((DeeTypeObject *)result, &DeeCFunctionType_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_argv_r_name_ffi_typev:
	Dee_Free(result->ft_ffi_arg_type_v);
err_argv_r_name:
	Dee_Decref(name);
err_argv_r:
	DeeGCObject_FREE(result);
err_argv:
	while (argc--)
		Dee_Decref((DeeObject *)argv_copy[argc]);
	Dee_Free(argv_copy);
err:
	return NULL;
}

INTERN NONNULL((1)) bool DCALL
stype_cfunction_rehash(DeeSTypeObject *__restrict self,
                       size_t new_mask) {
	DeeCFunctionTypeObject **new_map, **dst;
	DeeCFunctionTypeObject **biter, **bend, *iter, *next;
again:
	new_map = (DeeCFunctionTypeObject **)Dee_TryCalloc((new_mask + 1) *
	                                                   sizeof(DeeCFunctionTypeObject *));
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
		for (; biter != bend; ++biter) {
			iter = *biter;
			while (iter) {
				next = LLIST_NEXT(iter, ft_chain);
				dst  = &new_map[iter->ft_hash & new_mask];
				/* Insert the entry into the new hash-map. */
				LLIST_INSERT(*dst, iter, ft_chain);
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

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
cfunction_hashof(DeeSTypeObject *__restrict return_type,
                 cc_t calling_convention, size_t argc,
                 DeeSTypeObject **__restrict argv) {
	dhash_t result;
	size_t i;
	result = Dee_HashPointer(return_type) ^ (dhash_t)calling_convention ^ (dhash_t)argc;
	for (i = 0; i < argc; ++i)
		result ^= Dee_HashPointer(argv[i]);
	return result;
}

PRIVATE bool DCALL
cfunction_equals(DeeCFunctionTypeObject *__restrict self,
                 DeeSTypeObject *__restrict return_type,
                 cc_t calling_convention, size_t argc,
                 DeeSTypeObject **__restrict argv) {
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


INTERN WUNUSED DREF DeeCFunctionTypeObject *DCALL
DeeSType_CFunction(DeeSTypeObject *__restrict return_type,
                   cc_t calling_convention, size_t argc,
                   DeeSTypeObject **__restrict argv,
                   bool inherit_argv) {
#ifdef CONFIG_NO_CFUNCTION
	(void)return_type;
	(void)calling_convention;
	(void)argc, (void)argv;
	if (inherit_argv)
		Dee_Free(argv);
	err_no_cfunction();
	return NULL;
#else /* CONFIG_NO_CFUNCTION */
	dhash_t hash;
	DREF DeeCFunctionTypeObject *result, *new_result, **pbucket;
	ASSERT_OBJECT_TYPE((DeeObject *)return_type, &DeeSType_Type);
	rwlock_read(&return_type->st_cachelock);
	ASSERT(!return_type->st_cfunction.sf_size ||
	       return_type->st_cfunction.sf_mask);
	hash = cfunction_hashof(return_type, calling_convention, argc, argv);
	if (return_type->st_cfunction.sf_size) {
		result = return_type->st_cfunction.sf_list[hash & return_type->st_cfunction.sf_mask];
		while (result &&
		       (result->ft_hash != hash ||
		        !cfunction_equals(result, return_type, calling_convention, argc, argv)))
			result = LLIST_NEXT(result, ft_chain);
		/* Check if we can re-use an existing type. */
		if (result && Dee_IncrefIfNotZero((DeeObject *)result)) {
			rwlock_endread(&return_type->st_cachelock);
			if (inherit_argv)
				Dee_Free(argv);
			return result;
		}
	}
	rwlock_endread(&return_type->st_cachelock);
	/* Construct a new cfunction type. */
	result = cfunctiontype_new(return_type, calling_convention,
	                           argc, argv, hash, inherit_argv);
	if unlikely(!result)
		goto done;
	/* Add the new type to the cache. */
register_type:
	rwlock_write(&return_type->st_cachelock);
	ASSERT(!return_type->st_cfunction.sf_size ||
	       return_type->st_cfunction.sf_mask);
	if (return_type->st_cfunction.sf_size) {
		new_result = return_type->st_cfunction.sf_list[hash & return_type->st_cfunction.sf_mask];
		while (new_result &&
		       (new_result->ft_hash != hash ||
		        !cfunction_equals(new_result, return_type, calling_convention, argc, argv)))
			new_result = LLIST_NEXT(new_result, ft_chain);
		/* Check if we can re-use an existing type. */
		if (new_result && Dee_IncrefIfNotZero((DeeObject *)new_result)) {
			rwlock_endread(&return_type->st_cachelock);
			Dee_Decref((DeeObject *)result);
			return new_result;
		}
	}
	/* Rehash when there are a lot of items. */
	if (return_type->st_cfunction.sf_size >= return_type->st_cfunction.sf_mask &&
	    (!stype_cfunction_rehash(return_type, return_type->st_cfunction.sf_mask ? (return_type->st_cfunction.sf_mask << 1) | 1 : 16 - 1) &&
	     !return_type->st_cfunction.sf_mask)) {
		/* No space at all! */
		rwlock_endwrite(&return_type->st_cachelock);
		/* Collect enough memory for a 1-item hash map. */
		if (Dee_CollectMemory(sizeof(DeeCFunctionTypeObject *)))
			goto register_type;
		/* Failed to allocate the initial hash-map. */
		Dee_Decref((DeeObject *)result);
		result = NULL;
		goto done;
	}
	/* Insert the new cfunction type into the hash-map. */
	pbucket = &return_type->st_cfunction.sf_list[hash & return_type->st_cfunction.sf_mask];
	LLIST_INSERT(*pbucket, result, ft_chain); /* Weak reference. */
	rwlock_endwrite(&return_type->st_cachelock);
done:
	return result;
#endif /* !CONFIG_NO_CFUNCTION */
}

DECL_END


#endif /* !GUARD_DEX_CTYPES_CFUNCTION_C */
