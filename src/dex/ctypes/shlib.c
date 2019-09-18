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
#ifndef GUARD_DEX_CTYPES_SHLIB_C
#define GUARD_DEX_CTYPES_SHLIB_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <string.h>

#ifndef CONFIG_NO_SHLIB
#if defined(CONFIG_HOST_WINDOWS) && !defined(__CYGWIN__)
#define USE_LOADLIBRARY 1
#include <Windows.h>
#elif !defined(CONFIG_NO_DLFCN) && \
      (defined(CONFIG_HOST_UNIX) || defined(CONFIG_HAVE_DLFCN))
#define USE_DLFCN 1
#include <dlfcn.h>
#include <errno.h>
#else
#define CONFIG_NO_SHLIB 1
#endif
#endif /* !CONFIG_NO_SHLIB */


DECL_BEGIN

typedef struct {
	OBJECT_HEAD
#ifdef USE_LOADLIBRARY
	HMODULE              sh_lib;     /* [1..1] Shared library handle. */
#elif defined(USE_DLFCN)
	void                *sh_lib;     /* [1..1] Shared library handle. */
#endif
#ifndef CONFIG_NO_SHLIB
#ifndef CONFIG_NO_CFUNCTION
	DREF DeeSTypeObject *sh_vfunptr; /* [0..1][lock(WRITE_ONCE)] void-function pointer type. */
	cc_t                 sh_defcc;   /* [const] Default calling convention. */
#endif /* !CONFIG_NO_CFUNCTION */
#endif /* !CONFIG_NO_SHLIB */
} Shlib;


#ifndef CONFIG_NO_SHLIB
PRIVATE int DCALL
shlib_init(Shlib *__restrict self, size_t argc,
           DREF DeeObject **__restrict argv) {
	DeeStringObject *name, *cc_name = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:shlib", &name, &cc_name) ||
	    DeeObject_AssertTypeExact((DeeObject *)name, &DeeString_Type))
		goto err;
		/* Parse the given default calling convention. */
#ifndef CONFIG_NO_CFUNCTION
	self->sh_vfunptr = NULL;
	self->sh_defcc   = CC_DEFAULT;
#endif /* !CONFIG_NO_CFUNCTION */
	if (cc_name) {
#ifdef CONFIG_NO_CFUNCTION
		err_no_cfunction();
		goto err;
#else /* CONFIG_NO_CFUNCTION */
		if (DeeObject_AssertTypeExact((DeeObject *)cc_name, &DeeString_Type))
			goto err;
		self->sh_defcc = cc_lookup(DeeString_STR(cc_name));
		if unlikely(self->sh_defcc == CC_INVALID)
			goto err;
#endif /* !CONFIG_NO_CFUNCTION */
	}

#ifdef USE_LOADLIBRARY
	{
		LPWSTR wname = (LPWSTR)DeeString_AsWide((DeeObject *)name);
		if unlikely(!wname)
			goto err;
		self->sh_lib = LoadLibraryW(wname);
		if (!self->sh_lib && nt_IsUncError(GetLastError())) {
			name = (DeeStringObject *)nt_FixUncPath((DeeObject *)name);
			if unlikely(!name)
				goto err;
			wname = (LPWSTR)DeeString_AsWide((DeeObject *)name);
			if unlikely(!wname) {
				Dee_Decref(name);
				goto err;
			}
			self->sh_lib = LoadLibraryW(wname);
			Dee_Decref(name);
			if unlikely(!self->sh_lib) {
				DWORD error = GetLastError();
				if (nt_IsFileNotFound(error)) {
					DeeError_SysThrowf(&DeeError_FileNotFound, error,
					                   "Shared library %r could not be found",
					                   name);
				} else {
					DeeError_SysThrowf(&DeeError_SystemError, error,
					                   "Failed to open shared library %r",
					                   name);
				}
				goto err;
			}
		}
	}
#elif defined(USE_DLFCN)
	self->sh_lib = dlopen(DeeString_STR(name),
	                      RTLD_LOCAL |
#ifdef RTLD_LAZY
	                      RTLD_LAZY
#else /* RTLD_LAZY */
	                      RTLD_NOW
#endif /* !RTLD_LAZY */
	                      );
	if (!self->sh_lib) {
		DeeError_SysThrowf(&DeeError_SystemError, errno,
		                   "Failed to open shared library %r",
		                   name);
		goto err;
	}
#endif
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
shlib_fini(Shlib *__restrict self) {
#ifndef CONFIG_NO_CFUNCTION
	Dee_XDecref((DeeObject *)self->sh_vfunptr);
#endif /* !CONFIG_NO_CFUNCTION */
#ifdef USE_LOADLIBRARY
	FreeLibrary(self->sh_lib);
#elif defined(USE_DLFCN)
	dlclose(self->sh_lib);
#endif
}
#ifndef CONFIG_NO_CFUNCTION
PRIVATE void DCALL
shlib_visit(Shlib *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit((DeeObject *)self->sh_vfunptr);
}
#endif /* !CONFIG_NO_CFUNCTION */

#ifdef _WIN32_WCE
#undef  GetProcAddress
#define GetProcAddress GetProcAddressA
#endif

PRIVATE void *DCALL
shlib_dlsym(Shlib *__restrict self,
            char const *__restrict name) {
#ifdef USE_LOADLIBRARY
	void *result;
	*(FARPROC *)&result = GetProcAddress(self->sh_lib, name);
	return result;
#elif defined(USE_DLFCN)
	void *result = dlsym(self->sh_lib, name);
#if defined(__i386__) || defined(__x86_64__) || 1
	if (!result) {
		size_t name_len = strlen(name);
		char *name_copy = (char *)Dee_ATryMalloc((name_len + 2) * sizeof(char));
		if unlikely(!name_copy)
			goto done;
		memcpy(name_copy + 1, name, (name_len + 1) * sizeof(char));
		name_copy[0] = '_';
		/* Lookup after prepending a leading underscore. */
		result = dlsym(self->sh_lib, name_copy);
		Dee_AFree(name_copy);
	}
done:
#endif
	return result;
#endif
}


#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(static_type_lock);
#endif /* !CONFIG_NO_THREADS */

PRIVATE DREF DeeSTypeObject *void_ptr = NULL; /* `void.ptr' */
INTERN bool DCALL clear_void_pointer(void) {
	DREF DeeSTypeObject *ptr;
	rwlock_write(&static_type_lock);
	ptr      = void_ptr;
	void_ptr = NULL;
	rwlock_endwrite(&static_type_lock);
	if (!ptr)
		return false;
	Dee_Decref_likely((DeeObject *)ptr);
	return true;
}

LOCAL DREF DeeSTypeObject *DCALL get_void_pointer(void) {
	DREF DeeSTypeObject *result;
	rwlock_read(&static_type_lock);
	result = void_ptr;
	if (result) {
		Dee_Incref((DeeObject *)result);
		rwlock_endread(&static_type_lock);
		goto done;
	}
	rwlock_endread(&static_type_lock);
	result = (DREF DeeSTypeObject *)DeeSType_Pointer(&DeeCVoid_Type);
	if unlikely(!result)
		goto done;
	rwlock_write(&static_type_lock);
	if (!void_ptr) {
		Dee_Incref((DeeObject *)result);
		void_ptr = result;
	} else {
		ASSERT(void_ptr == result);
	}
	rwlock_endwrite(&static_type_lock);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
shlib_getitem(Shlib *__restrict self,
              DeeObject *__restrict name) {
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
	void *symaddr;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	symaddr = shlib_dlsym(self, DeeString_STR(name));
	if unlikely(!symaddr) {
		DeeError_Throwf(&DeeError_KeyError,
		                "No export named %r", name);
		goto err;
	}
	result_type = get_void_pointer();
	if unlikely(!result_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err_type;
	DeeObject_InitNoref(result, (DeeTypeObject *)result_type);
	result->p_ptr.ptr = symaddr;
	return (DREF DeeObject *)result;
err_type:
	Dee_Decref_unlikely((DeeObject *)result_type);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
shlib_getattr(Shlib *__restrict self,
              DeeObject *__restrict name) {
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
	void *symaddr;
	symaddr = shlib_dlsym(self, DeeString_STR(name));
	if unlikely(!symaddr) {
		DeeError_Throwf(&DeeError_AttributeError,
		                "No export named %r", name);
		goto err;
	}
#ifdef CONFIG_NO_CFUNCTION
	result_type = (DREF DeeSTypeObject *)DeeSType_Pointer(&DeeCVoid_Type);
	if unlikely(!result_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result) {
		Dee_Decref((DeeObject *)result_type);
		goto err;
	}
	DeeObject_InitNoref(result, (DeeTypeObject *)result_type);
#else /* CONFIG_NO_CFUNCTION */
	result_type = self->sh_vfunptr;
	if (!result_type) {
		DREF DeeSTypeObject *new_type;
		result_type = (DREF DeeSTypeObject *)DeeSType_CFunction(&DeeCInt_Type,
		                                                        (cc_t)((unsigned int)self->sh_defcc |
		                                                               (unsigned int)CC_FVARARGS),
		                                                        0, NULL, true);
		if unlikely(!result_type)
			goto err;
		new_type = (DREF DeeSTypeObject *)DeeSType_Pointer(result_type);
		Dee_Decref_unlikely((DeeObject *)result_type);
		if unlikely(!new_type)
			goto err;
		result_type = new_type;
		/* Save the reference in the shlib descriptor. */
		if (!ATOMIC_CMPXCH(self->sh_vfunptr, NULL, result_type))
			Dee_DecrefNokill((DeeObject *)result_type);
		ASSERT(self->sh_vfunptr == result_type);
	}
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, (DeeTypeObject *)result_type);
#endif /* !CONFIG_NO_CFUNCTION */
	result->p_ptr.ptr = symaddr;
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE struct type_seq shlib_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL, /* TODO */
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))NULL, /* TODO */
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&shlib_getitem
};

PRIVATE struct type_attr shlib_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&shlib_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))NULL,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))NULL,
	/* .tp_enumattr = */ (dssize_t (DCALL *)(DeeTypeObject *__restrict, DeeObject *__restrict, denum_t, void *))NULL /* TODO */
};

PRIVATE DREF DeeObject *DCALL
shlib_base(Shlib *__restrict self, size_t argc,
           DeeObject **__restrict argv) {
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
	if (DeeArg_Unpack(argc, argv, ":base"))
		goto err;
	result_type = get_void_pointer();
	if unlikely(!result_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err_type;
	DeeObject_InitNoref(result, (DeeTypeObject *)result_type);
	/* Return the base address of the shared library. */
	result->p_ptr.ptr = (void *)self->sh_lib;

	return (DREF DeeObject *)result;
err_type:
	Dee_Decref_unlikely((DeeObject *)result_type);
err:
	return NULL;
}

#else /* !CONFIG_NO_SHLIB */
PRIVATE void DCALL err_shlib_unsupported(void) {
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "Shared libraries are not supported by the host");
}


INTERN bool DCALL clear_void_pointer(void) {
	return false;
}

PRIVATE DREF DeeObject *DCALL
shlib_base(Shlib *__restrict self, size_t argc,
           DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":base"))
		goto err;
	err_shlib_unsupported();
err:
	return NULL;
}
#endif /* CONFIG_NO_SHLIB */


PRIVATE struct type_method shlib_methods[] = {
	{ "base", (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict))&shlib_base,
	  DOC("->?Aptr?Gvoid\nReturns the base address of the shared library") },
	{ NULL }
};

INTERN DeeTypeObject DeeShlib_Type = {
	OBJECT_HEAD_INIT((DeeTypeObject *)&DeeType_Type),
	/* .tp_name     = */ "ShLib",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ (DeeTypeObject *)&DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
#ifndef CONFIG_NO_SHLIB
				/* .tp_any_ctor  = */ (void *)&shlib_init,
#else /* !CONFIG_NO_SHLIB */
				/* .tp_any_ctor  = */ NULL,
#endif /* CONFIG_NO_SHLIB */
				/* .tp_free      = */ NULL,
				{
					/* ..tp_alloc.tp_instance_size = */sizeof(Shlib)
				}
			}
		},
#ifndef CONFIG_NO_SHLIB
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&shlib_fini,
#else /* !CONFIG_NO_SHLIB */
		/* .tp_dtor        = */ NULL,
#endif /* CONFIG_NO_SHLIB */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
#if !defined(CONFIG_NO_SHLIB) && !defined(CONFIG_NO_CFUNCTION)
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&shlib_visit,
#else /* !CONFIG_NO_SHLIB && !CONFIG_NO_CFUNCTION */
	/* .tp_visit         = */ NULL,
#endif /* CONFIG_NO_SHLIB || CONFIG_NO_CFUNCTION */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
#ifndef CONFIG_NO_SHLIB
	/* .tp_seq           = */ &shlib_seq,
#else /* !CONFIG_NO_SHLIB */
	/* .tp_seq           = */ NULL,
#endif /* CONFIG_NO_SHLIB */
	/* .tp_iter_next     = */ NULL,
#ifndef CONFIG_NO_SHLIB
	/* .tp_attr          = */ &shlib_attr,
#else /* !CONFIG_NO_SHLIB */
	/* .tp_attr          = */ NULL,
#endif /* CONFIG_NO_SHLIB */
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ shlib_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END


#endif /* !GUARD_DEX_CTYPES_SHLIB_C */
