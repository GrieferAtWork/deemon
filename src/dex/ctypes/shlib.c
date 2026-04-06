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
#ifndef GUARD_DEX_CTYPES_SHLIB_C
#define GUARD_DEX_CTYPES_SHLIB_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>             /* DeeArg_Unpack* */
#include <deemon/bool.h>            /* return_bool */
#include <deemon/dex.h>             /* Dee_module_dexdata */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/int.h>             /* DeeInt_NewUInt64 */
#include <deemon/map.h>             /* DeeMap_Type */
#include <deemon/module.h>          /* DeeModule* */
#include <deemon/mro.h>             /* Dee_attrhint, Dee_attriter */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>          /* ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_AssertTypeExact, DeeTypeObject, Dee_AsObject, Dee_Decref*, Dee_Incref, Dee_XDecref, ITER_DONE, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/string.h>          /* DeeString* */
#include <deemon/system-features.h> /* DeeSystem_DlOpen_USE_STUB */
#include <deemon/system.h>          /* DeeSystem_* */
#include <deemon/tuple.h>           /* DeeTupleObject, DeeTuple_NewUninitialized */
#include <deemon/type.h>            /* DeeObject_*, DeeType_IsHeapType, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_XVisit, Dee_visit_t, METHOD_FNOREFESCAPE, TF_NONE, TP_FDEEPIMMUTABLE, TP_FNORMAL, TYPE_METHOD_END, TYPE_METHOD_F, type_* */
#include <deemon/util/atomic-ref.h> /* Dee_ATOMIC_XREF, Dee_atomic_xref_* */
#include <deemon/util/atomic.h>     /* atomic_cmpxch, atomic_read */
#include <deemon/util/lock.h>       /* Dee_SHARED_LOCK_INIT, Dee_shared_lock_* */
#include <deemon/util/once.h>       /* Dee_ONCE */

#include <hybrid/host.h> /* __i386__, __x86_64__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint64_t, uintptr_t */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject      *sh_lib_owner; /* [0..1][const] Owner of "sh_lib" (in case library was loaded from dex module) */
	void                *sh_lib;       /* [1..1][owned_if(sh_lib_owner == NULL)][const] Shared library handle. */
#ifndef CONFIG_NO_CFUNCTION
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	DREF CPointerType   *sh_vfunptr;   /* [0..1][lock(WRITE_ONCE)] void-function pointer type: "int(<sh_defcc> *)(...)" */
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	DREF DeeSTypeObject *sh_vfunptr;   /* [0..1][lock(WRITE_ONCE)] void-function pointer type. */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	ctypes_cc_t          sh_defcc;     /* [const] Default calling convention. */
#endif /* !CONFIG_NO_CFUNCTION */
} ShLib;


PRIVATE WUNUSED NONNULL((1)) int DCALL
shlib_init(ShLib *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeStringObject *name, *cc_name = NULL;
	DeeArg_Unpack1Or2(err, argc, argv, "ShLib", &name, &cc_name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	self->sh_lib_owner = NULL;

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
		if (DeeObject_AssertTypeExact(cc_name, &DeeString_Type))
			goto err;
		self->sh_defcc = cc_lookup(DeeString_STR(cc_name));
		if unlikely(self->sh_defcc == CC_INVALID)
			goto err;
#endif /* !CONFIG_NO_CFUNCTION */
	}

	self->sh_lib = DeeSystem_DlOpen(Dee_AsObject(name));
	if unlikely(!self->sh_lib)
		goto err;
	if unlikely(self->sh_lib == DeeSystem_DlOpen_FAILED) {
#ifdef DeeSystem_DlOpen_USE_STUB
#define DLOPEN_ERROR_TYPE  &DeeError_UnsupportedAPI
#else /* DeeSystem_DlOpen_USE_STUB */
#define DLOPEN_ERROR_TYPE  &DeeError_SystemError
#endif /* !DeeSystem_DlOpen_USE_STUB */
		DREF DeeStringObject *message;
		message = (DREF DeeStringObject *)DeeSystem_DlError();
		if unlikely(!message)
			goto err;
		if (message == (DREF DeeStringObject *)ITER_DONE) {
			DeeError_Throwf(DLOPEN_ERROR_TYPE,
			                "Failed to open shared library %r",
			                name);
		} else {
			DeeError_Throwf(DLOPEN_ERROR_TYPE,
			                "Failed to open shared library %r (%r)",
			                name, message);
			Dee_Decref(message);
		}
		goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
shlib_fini(ShLib *__restrict self) {
#ifndef CONFIG_NO_CFUNCTION
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	Dee_XDecref(CPointerType_AsType(self->sh_vfunptr));
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	Dee_XDecref(DeeSType_AsType(self->sh_vfunptr));
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
#endif /* !CONFIG_NO_CFUNCTION */
	if (self->sh_lib_owner) {
		Dee_Decref(self->sh_lib_owner);
	} else {
		DeeSystem_DlClose(self->sh_lib);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
shlib_visit(ShLib *__restrict self, Dee_visit_t proc, void *arg) {
#ifndef CONFIG_NO_CFUNCTION
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	Dee_XVisit(CPointerType_AsType(self->sh_vfunptr));
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	Dee_XVisit(DeeSType_AsType(self->sh_vfunptr));
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
#endif /* !CONFIG_NO_CFUNCTION */
	Dee_XVisit(self->sh_lib_owner);
}

#ifndef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
PRIVATE Dee_ATOMIC_XREF(DeeTypeObject) void_ptr = Dee_ATOMIC_XREF_INIT(NULL); /* `void.ptr' */

INTERN bool DCALL clear_void_pointer(void) {
	DREF DeeTypeObject *ptr;
	Dee_atomic_xref_xch_newNULL(&void_ptr, &ptr);
	Dee_XDecref(ptr);
	return ptr != NULL;
}

PRIVATE WUNUSED DREF DeeSTypeObject *DCALL get_void_pointer(void) {
	DREF DeeTypeObject *result;
again:
	Dee_atomic_xref_get(&void_ptr, &result);
	if (result)
		return DeeType_AsSType(result);
	result = DeePointerType_AsType(DeeSType_Pointer(&DeeCVoid_Type));
	if likely(result) {
		if (!Dee_atomic_xref_cmpxch_oldNULL_newNONNULL(&void_ptr, result)) {
			Dee_Decref(result);
			goto again;
		}
	}
	return DeeType_AsSType(result);
}
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */

#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
PRIVATE WUNUSED NONNULL((1, 2)) DREF CPointer *DCALL
shlib_getitem(ShLib *self, DeeObject *name)
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
shlib_getitem(ShLib *self, DeeObject *name)
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
{
#ifndef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	void *symaddr;
	char const *utf8_name;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(name);
	if unlikely(!utf8_name)
		goto err;
	symaddr = DeeSystem_DlSym(self->sh_lib, utf8_name);
	if unlikely(!symaddr) {
		DREF DeeObject *message;
		message = DeeSystem_DlError();
		if unlikely(!message)
			goto err;
		DeeError_Throwf(&DeeError_KeyError,
		                "No export named %r (%r)",
		                name, message);
		Dee_Decref(message);
		goto err;
	}
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	return CPointer_NewVoid(symaddr);
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	result_type = get_void_pointer();
	if unlikely(!result_type)
		goto err;
	result = pointer_object_malloc();
	if unlikely(!result)
		goto err_type;
	DeeObject_InitHeapInherited(result, DeeSType_AsType(result_type));
	result->p_ptr.ptr = symaddr;
	return Dee_AsObject(result);
err_type:
	Dee_Decref_unlikely(DeeSType_AsType(result_type));
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
shlib_contains(ShLib *self, DeeObject *name) {
	void *symaddr;
	char const *utf8_name;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	utf8_name = DeeString_AsUtf8(name);
	if unlikely(!utf8_name)
		goto err;
	symaddr = DeeSystem_DlSym(self->sh_lib, utf8_name);
	return_bool(symaddr != NULL);
err:
	return NULL;
}

#ifdef CONFIG_NO_CFUNCTION
#define shlib_getattr shlib_getitem
#else /* CONFIG_NO_CFUNCTION */
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
PRIVATE WUNUSED NONNULL((1, 2)) DREF CPointer *DCALL
shlib_getattr(ShLib *self, DeeObject *name)
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
shlib_getattr(ShLib *self, DeeObject *name)
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
{
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	DREF CPointerType *result_type;
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	void *symaddr;
	char const *utf8_name;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	utf8_name = DeeString_AsUtf8(name);
	if unlikely(!utf8_name)
		goto err;
	symaddr = DeeSystem_DlSym(self->sh_lib, utf8_name);
	if unlikely(!symaddr) {
		DREF DeeObject *message;
		message = DeeSystem_DlError();
		if unlikely(!message)
			goto err;
		DeeError_Throwf(&DeeError_KeyError,
		                "No export named %r (%r)",
		                name, message);
		Dee_Decref(message);
		goto err;
	}
	result_type = self->sh_vfunptr;
	if (result_type == NULL) {
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
		DREF CFunctionType *function_type;
		function_type = CFunctionType_Of(&CInt_Type,
		                                 (ctypes_cc_t)((unsigned int)self->sh_defcc |
		                                               (unsigned int)CC_FVARARGS),
		                                 0, NULL);
		if unlikely(!function_type)
			goto err;
		result_type = CPointerType_Of(CFunctionType_AsCType(function_type));
		Dee_Decref_unlikely(CFunctionType_AsObject(function_type));
		if unlikely(!result_type)
			goto err;
		ASSERT(DeeType_IsHeapType(CPointerType_AsType(result_type)));
		if (!atomic_cmpxch(&self->sh_vfunptr, NULL, result_type)) {
			Dee_Decref_unlikely(CPointerType_AsType(result_type));
			result_type = self->sh_vfunptr;
		}
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
		DREF DeeSTypeObject *new_type;
		result_type = DeeCFunctionType_AsSType(DeeSType_CFunction(&DeeCInt_Type,
		                                                          (ctypes_cc_t)((unsigned int)self->sh_defcc |
		                                                                        (unsigned int)CC_FVARARGS),
		                                                          0, NULL, true));
		if unlikely(!result_type)
			goto err;
		new_type = DeePointerType_AsSType(DeeSType_Pointer(result_type));
		Dee_Decref_unlikely(DeeSType_AsType(result_type));
		if unlikely(!new_type)
			goto err;
		result_type = new_type;

		/* Save the reference in the shlib descriptor. */
		ASSERT(DeeType_IsHeapType(DeeSType_AsType(result_type)));
		if (!atomic_cmpxch(&self->sh_vfunptr, NULL, result_type))
			Dee_DecrefNokill(DeeSType_AsType(result_type));
		ASSERT(self->sh_vfunptr == result_type);
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	}
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	return CPointer_New(result_type, symaddr);
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	result = pointer_object_malloc();
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, DeeSType_AsType(result_type));
	result->p_ptr.ptr = symaddr;
	return Dee_AsObject(result);
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
err:
	return NULL;
}
#endif /* !CONFIG_NO_CFUNCTION */

PRIVATE struct type_seq shlib_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL, /* TODO */
	/* .tp_sizeob   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&shlib_contains,
	/* .tp_getitem  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&shlib_getitem
};

PRIVATE struct type_attr shlib_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&shlib_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))NULL,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))NULL,
	/* .tp_iterattr = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))NULL, /* TODO */
};

#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
PRIVATE WUNUSED NONNULL((1)) DREF CPointer *DCALL
shlib_base(ShLib *self, size_t argc, DeeObject *const *argv)
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
shlib_base(ShLib *self, size_t argc, DeeObject *const *argv)
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
{
#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
	DeeArg_Unpack0(err, argc, argv, "base");
	return CPointer_NewVoid((void *)self->sh_lib);
err:
	return NULL;
#else /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
	DeeArg_Unpack0(err, argc, argv, "base");
	result_type = get_void_pointer();
	if unlikely(!result_type)
		goto err;
	result = pointer_object_malloc();
	if unlikely(!result)
		goto err_type;
	DeeObject_InitHeapInherited(result, DeeSType_AsType(result_type));
	/* Return the base address of the shared library. */
	result->p_ptr.ptr = (void *)self->sh_lib;

	return Dee_AsObject(result);
err_type:
	Dee_Decref_unlikely(DeeSType_AsType(result_type));
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_CTYPES */
}

#undef shlib_addr2name_impl_USE_Dbghelp_dll
#undef shlib_addr2name_impl_USE_libdebuginfo /* TODO: KOS-specific: libdebuginfo */
#undef shlib_addr2name_impl_USE_addr2line /* TODO: ipc.Process("addr2line -f") */
#undef shlib_addr2name_impl_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define shlib_addr2name_impl_USE_Dbghelp_dll
#else /* CONFIG_HOST_WINDOWS */
#define shlib_addr2name_impl_USE_STUB
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef shlib_addr2name_impl_USE_Dbghelp_dll
PRIVATE WCHAR const wDbghelp_dll[] = { 'D', 'b', 'g', 'h', 'e', 'l', 'p', '.', 'd', 'l', 'l', 0 };

PRIVATE HMODULE DCALL get_Dbghelp_dll(void) {
	static HMODULE hDbghelp = NULL;
	HMODULE result;
again:
	result = atomic_read(&hDbghelp);
	if (result == INVALID_HANDLE_VALUE)
		return NULL;
	if (result == NULL) {
		result = LoadLibraryW(wDbghelp_dll);
		if (result == NULL) {
			atomic_cmpxch(&hDbghelp, NULL, INVALID_HANDLE_VALUE);
			return NULL;
		}
		if (!atomic_cmpxch(&hDbghelp, NULL, result)) {
			(void)FreeLibrary(result);
			goto again;
		}
	}
	return result;
}

#ifndef IMAGEAPI
#define IMAGEAPI __ATTR_STDCALL
#endif /* !IMAGEAPI */

#ifndef MAX_SYM_NAME
#define MAX_SYM_NAME 2000
#endif /* !MAX_SYM_NAME */

typedef struct NT_SYMBOL_INFO {
	ULONG       SizeOfStruct;
	ULONG       TypeIndex;        // Type Index of symbol
	ULONG64     Reserved[2];
	ULONG       Index;
	ULONG       Size;
	ULONG64     ModBase;          // Base Address of module comtaining this symbol
	ULONG       Flags;
	ULONG64     Value;            // Value of symbol, ValuePresent should be 1
	ULONG64     Address;          // Address of symbol including base address of module
	ULONG       Register;         // register holding value or pointer to value
	ULONG       Scope;            // scope of the symbol
	ULONG       Tag;              // pdb classification
	ULONG       NameLen;          // Actual length of name
	ULONG       MaxNameLen;
	CHAR        Name[1];          // Name of symbol
} NT_SYMBOL_INFO, *NT_PSYMBOL_INFO;

typedef BOOL (IMAGEAPI *LPSYMFROMADDR)(HANDLE hProcess, uint64_t Address, uint64_t *Displacement, NT_PSYMBOL_INFO Symbol);
PRIVATE LPSYMFROMADDR pdyn_SymFromAddr = NULL;
#define SymFromAddr (*pdyn_SymFromAddr)

typedef BOOL (IMAGEAPI *LPSYMINITIALIZE)(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);
PRIVATE LPSYMINITIALIZE pdyn_SymInitialize = NULL;
#define SymInitialize (*pdyn_SymInitialize)

PRIVATE bool DCALL load_Dbghelp_dll(void) {
	HMODULE hModule;
	if (pdyn_SymFromAddr)
		return true;
	hModule = get_Dbghelp_dll();
	if unlikely(!hModule)
		return false;
	pdyn_SymInitialize = (LPSYMINITIALIZE)GetProcAddress(hModule, "SymInitialize");
	if unlikely(!pdyn_SymInitialize)
		goto err;
	pdyn_SymFromAddr = (LPSYMFROMADDR)GetProcAddress(hModule, "SymFromAddr");
	return pdyn_SymFromAddr != NULL;
err:
	return false;
}

PRIVATE bool DCALL init_Dbghelp_dll(void) {
	static bool success = false;
	Dee_ONCE({
		if (load_Dbghelp_dll())
			success = SymInitialize(GetCurrentProcess(), NULL, TRUE);
	});
	return success;
}

#endif /* shlib_addr2name_impl_USE_Dbghelp_dll */


PRIVATE DREF DeeObject *DCALL
shlib_addr2name_impl(void const *addr) {
#ifdef shlib_addr2name_impl_USE_Dbghelp_dll
	static Dee_shared_lock_t lock = Dee_SHARED_LOCK_INIT;
	if unlikely(Dee_shared_lock_acquire(&lock))
		goto err;
	if (init_Dbghelp_dll()) {
		uint64_t displacement = 0;
		char buffer[sizeof(NT_SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		NT_PSYMBOL_INFO pSymbol = (NT_PSYMBOL_INFO)buffer;
		pSymbol->SizeOfStruct = sizeof(NT_SYMBOL_INFO);
		pSymbol->MaxNameLen   = MAX_SYM_NAME;
		pSymbol->NameLen      = 0;
		if (SymFromAddr(GetCurrentProcess(), (uint64_t)(uintptr_t)addr, &displacement, pSymbol) &&
		    pSymbol->NameLen && pSymbol->NameLen < MAX_SYM_NAME) {
			DREF DeeObject *name;
			DREF DeeObject *delta;
			DREF DeeTupleObject *result;
			Dee_shared_lock_release(&lock);
			name = DeeString_NewSized(pSymbol->Name, pSymbol->NameLen);
			if unlikely(!name)
				goto err;
			delta = DeeInt_NewUInt64(displacement);
			if unlikely(!delta)
				goto err_name;
			result = DeeTuple_NewUninitialized(2);
			if unlikely(!result)
				goto err_name_delta;
			result->t_elem[0] = name;  /* Inherit reference */
			result->t_elem[1] = delta; /* Inherit reference */
			return Dee_AsObject(result);
err_name_delta:
			Dee_Decref(delta);
err_name:
			Dee_Decref_likely(name);
			goto err;
		}
	}
	Dee_shared_lock_release(&lock);
	return_none;
err:
	return NULL;
#endif /* shlib_addr2name_impl_USE_Dbghelp_dll */

#ifdef shlib_addr2name_impl_USE_STUB
	(void)addr;
	return_none;
#endif /* shlib_addr2name_impl_USE_STUB */
}

PRIVATE DREF DeeObject *DCALL
shlib_addr2name(DeeTypeObject *UNUSED(tp_self),
                size_t argc, DeeObject *const *argv) {
	union pointer addr;
	DeeObject *addrob;
	DeeArg_Unpack1(err, argc, argv, "addr2name", &addrob);
	if (DeeObject_AsPointer(addrob, &DeeCVoid_Type, &addr))
		goto err;
	return shlib_addr2name_impl(addr.ptr);
err:
	return NULL;
}


#ifndef CONFIG_NO_DEX
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE DREF ShLib *DCALL
shlib_ofmodule(DeeTypeObject *UNUSED(tp_self),
               size_t argc, DeeObject *const *argv) {
	DREF ShLib *result;
	DeeModuleObject *mod;
	struct Dee_module_dexdata *dexdata;
	DeeArg_Unpack1(err, argc, argv, "ofmodule", &mod);
	if (DeeObject_AssertTypeExact(mod, &DeeModuleDex_Type))
		goto err;
	dexdata = mod->mo_moddata.mo_dexdata;
	if (dexdata->mdx_handle == DeeSystem_DlOpen_FAILED) {
		/* This can happen if "mod" is the deemon core itself, and
		 * the core hadn't had a chance to initialize itself, yet. */
		COMPILER_UNUSED(DeeModule_GetNativeSymbol(mod, "DeeObject_Type"));
		if unlikely(dexdata->mdx_handle == DeeSystem_DlOpen_FAILED) {
			/* Weird... Throw an error */
			DeeError_Throwf(&DeeError_ValueError,
			                "Unable to determine native library handle of dex module %k",
			                mod);
			goto err;
		}
	}

	result = DeeObject_MALLOC(ShLib);
	if unlikely(!result)
		goto err;
	Dee_Incref(mod);
	result->sh_lib_owner = Dee_AsObject(mod);
#ifndef CONFIG_NO_CFUNCTION
	result->sh_vfunptr = NULL;
#if defined(__i386__) && !defined(__x86_64__)
#ifdef CONFIG_HAVE_FFI_STDCALL
	result->sh_defcc = FFI_STDCALL; /* Use STDCALL as default, since that's what "DCALL" is on i386 */
#endif /* CONFIG_HAVE_FFI_STDCALL */
#else /* ... */
	result->sh_defcc = CC_DEFAULT;
#endif /* !... */
#endif /* !CONFIG_NO_CFUNCTION */
	result->sh_lib = dexdata->mdx_handle;
	DeeObject_InitStatic(result, &ShLib_Type);
	return result;
err:
	return NULL;
}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
/* Could also be implemented without 'CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES', but don't bother */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#endif /* !CONFIG_NO_DEX */



PRIVATE struct type_method tpconst shlib_methods[] = {
	TYPE_METHOD_F("base", &shlib_base, METHOD_FNOREFESCAPE,
	              "->?Aptr?Gvoid\n"
	              "Returns the base address of the shared library"),
	TYPE_METHOD_END
};

PRIVATE struct type_method tpconst shlib_class_methods[] = {
	TYPE_METHOD_F("addr2name", &shlib_addr2name, METHOD_FNOREFESCAPE,
	              "(addr:?Aptr?Gvoid)->?X2?T2?Dstring?Dint?N\n"
	              "Using system-specific debug/export information, try to "
	              /**/ "convert a symbol address into that symbol's name.\n"
	              "On success, returns ${(symbolName, offsetFromSymbol)}"),
#ifndef CONFIG_NO_DEX
	TYPE_METHOD_F("ofmodule", &shlib_ofmodule, METHOD_FNOREFESCAPE,
	              "(mod:?DModule)->?.\n"
	              "#tValueError{Given @mod isn't the deemon core, or a ?Ert:DexModule}"
	              "Return a shared library descriptor for the underlying system library "
	              /**/ "descriptor of the deemon core, or a DEX module."),
#endif /* !CONFIG_NO_DEX */
	TYPE_METHOD_END
};

INTERN DeeTypeObject ShLib_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "ShLib",
	/* .tp_doc      = */ DOC("(filename:?Dstring,defcc?:?Dstring)\n"
	                         "#pdefcc{Default calling convention (s.a. ?Afunc?GCType)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FDEEPIMMUTABLE, /* Mainly set to "TP_FDEEPIMMUTABLE" to allow in nested object-trees */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMap_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ShLib,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ &shlib_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Can't be serialized */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&shlib_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&shlib_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &shlib_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &shlib_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ shlib_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ shlib_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_CTYPES_SHLIB_C */
