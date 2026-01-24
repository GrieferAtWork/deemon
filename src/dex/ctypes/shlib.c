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

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h> /* atomic_cmpxch, atomic_read */
#include <deemon/util/lock.h>   /* Dee_ATOMIC_RWLOCK_INIT, Dee_SHARED_LOCK_INIT, Dee_atomic_rwlock_*, Dee_shared_lock_* */
#include <deemon/util/once.h>   /* Dee_ONCE */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint64_t, uintptr_t */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	void                *sh_lib;     /* [1..1] Shared library handle. */
#ifndef CONFIG_NO_CFUNCTION
	DREF DeeSTypeObject *sh_vfunptr; /* [0..1][lock(WRITE_ONCE)] void-function pointer type. */
	ctypes_cc_t          sh_defcc;   /* [const] Default calling convention. */
#endif /* !CONFIG_NO_CFUNCTION */
} Shlib;


PRIVATE int DCALL
shlib_init(Shlib *__restrict self, size_t argc,
           DREF DeeObject *const *argv) {
	DeeStringObject *name, *cc_name = NULL;
	DeeArg_Unpack1Or2(err, argc, argv, "shlib", &name, &cc_name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
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
shlib_fini(Shlib *__restrict self) {
#ifndef CONFIG_NO_CFUNCTION
	Dee_XDecref(DeeSType_AsType(self->sh_vfunptr));
#endif /* !CONFIG_NO_CFUNCTION */
	DeeSystem_DlClose(self->sh_lib);
}

#ifndef CONFIG_NO_CFUNCTION
PRIVATE NONNULL((1, 2)) void DCALL
shlib_visit(Shlib *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(DeeSType_AsType(self->sh_vfunptr));
}
#endif /* !CONFIG_NO_CFUNCTION */


#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t static_type_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define static_type_lock_reading()    Dee_atomic_rwlock_reading(&static_type_lock)
#define static_type_lock_writing()    Dee_atomic_rwlock_writing(&static_type_lock)
#define static_type_lock_tryread()    Dee_atomic_rwlock_tryread(&static_type_lock)
#define static_type_lock_trywrite()   Dee_atomic_rwlock_trywrite(&static_type_lock)
#define static_type_lock_canread()    Dee_atomic_rwlock_canread(&static_type_lock)
#define static_type_lock_canwrite()   Dee_atomic_rwlock_canwrite(&static_type_lock)
#define static_type_lock_waitread()   Dee_atomic_rwlock_waitread(&static_type_lock)
#define static_type_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&static_type_lock)
#define static_type_lock_read()       Dee_atomic_rwlock_read(&static_type_lock)
#define static_type_lock_write()      Dee_atomic_rwlock_write(&static_type_lock)
#define static_type_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&static_type_lock)
#define static_type_lock_upgrade()    Dee_atomic_rwlock_upgrade(&static_type_lock)
#define static_type_lock_downgrade()  Dee_atomic_rwlock_downgrade(&static_type_lock)
#define static_type_lock_endwrite()   Dee_atomic_rwlock_endwrite(&static_type_lock)
#define static_type_lock_endread()    Dee_atomic_rwlock_endread(&static_type_lock)
#define static_type_lock_end()        Dee_atomic_rwlock_end(&static_type_lock)

PRIVATE DREF DeeSTypeObject *void_ptr = NULL; /* `void.ptr' */
INTERN bool DCALL clear_void_pointer(void) {
	DREF DeeSTypeObject *ptr;
	static_type_lock_write();
	ptr      = void_ptr;
	void_ptr = NULL;
	static_type_lock_endwrite();
	if (!ptr)
		return false;
	Dee_Decref_likely((DeeObject *)ptr);
	return true;
}

PRIVATE WUNUSED DREF DeeSTypeObject *DCALL get_void_pointer(void) {
	DREF DeeSTypeObject *result;
	static_type_lock_read();
	result = void_ptr;
	if (result) {
		Dee_Incref((DeeObject *)result);
		static_type_lock_endread();
		goto done;
	}
	static_type_lock_endread();
	result = (DREF DeeSTypeObject *)DeeSType_Pointer(&DeeCVoid_Type);
	if unlikely(!result)
		goto done;
	static_type_lock_write();
	if likely(!void_ptr) {
		Dee_Incref((DeeObject *)result);
		void_ptr = result;
	} else {
		ASSERT(void_ptr == result);
	}
	static_type_lock_endwrite();
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
shlib_getitem(Shlib *self, DeeObject *name) {
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
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
	result_type = get_void_pointer();
	if unlikely(!result_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err_type;
	DeeObject_InitInherited(result, DeeSType_AsType(result_type));
	result->p_ptr.ptr = symaddr;
	return Dee_AsObject(result);
err_type:
	Dee_Decref_unlikely(DeeSType_AsType(result_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
shlib_contains(Shlib *self,
               DeeObject *name) {
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
shlib_getattr(Shlib *self,
              DeeObject *name) {
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
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
#ifdef CONFIG_NO_CFUNCTION
	result_type = DeePointerType_AsSType(DeeSType_Pointer(&DeeCVoid_Type));
	if unlikely(!result_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result) {
		Dee_Decref(DeeSType_AsType(result_type));
		goto err;
	}
	DeeObject_InitInherited(result, DeeSType_AsType(result_type));
#else /* CONFIG_NO_CFUNCTION */
	result_type = self->sh_vfunptr;
	if (!result_type) {
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
		if (!atomic_cmpxch(&self->sh_vfunptr, NULL, result_type))
			Dee_DecrefNokill(DeeSType_AsType(result_type));
		ASSERT(self->sh_vfunptr == result_type);
	}
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, DeeSType_AsType(result_type));
#endif /* !CONFIG_NO_CFUNCTION */
	result->p_ptr.ptr = symaddr;
	return Dee_AsObject(result);
err:
	return NULL;
}

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

PRIVATE WUNUSED DREF DeeObject *DCALL
shlib_base(Shlib *self, size_t argc,
           DeeObject *const *argv) {
	DREF struct pointer_object *result;
	DREF DeeSTypeObject *result_type;
	DeeArg_Unpack0(err, argc, argv, "base");
	result_type = get_void_pointer();
	if unlikely(!result_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err_type;
	DeeObject_InitInherited(result, DeeSType_AsType(result_type));
	/* Return the base address of the shared library. */
	result->p_ptr.ptr = (void *)self->sh_lib;

	return Dee_AsObject(result);
err_type:
	Dee_Decref_unlikely(DeeSType_AsType(result_type));
err:
	return NULL;
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
	TYPE_METHOD_END
};

INTERN DeeTypeObject DeeShLib_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "ShLib",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ Shlib,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
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
#ifndef CONFIG_NO_CFUNCTION
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&shlib_visit,
#else /* !CONFIG_NO_CFUNCTION */
	/* .tp_visit         = */ NULL,
#endif /* CONFIG_NO_CFUNCTION */
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
