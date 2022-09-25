/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_SHLIB_C
#define GUARD_DEX_CTYPES_SHLIB_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

#include <hybrid/atomic.h>

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

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
	if (DeeArg_Unpack(argc, argv, "o|o:shlib", &name, &cc_name))
		goto err;
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

	self->sh_lib = DeeSystem_DlOpen((DeeObject *)name);
	if unlikely(!self->sh_lib)
		goto err;
	if unlikely(self->sh_lib == DEESYSTEM_DLOPEN_FAILED) {
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
	Dee_XDecref((DeeObject *)self->sh_vfunptr);
#endif /* !CONFIG_NO_CFUNCTION */
	DeeSystem_DlClose(self->sh_lib);
}

#ifndef CONFIG_NO_CFUNCTION
PRIVATE NONNULL((1, 2)) void DCALL
shlib_visit(Shlib *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit((DeeObject *)self->sh_vfunptr);
}
#endif /* !CONFIG_NO_CFUNCTION */


#ifndef CONFIG_NO_THREADS
PRIVATE rwlock_t static_type_lock = RWLOCK_INIT;
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

LOCAL WUNUSED DREF DeeSTypeObject *DCALL get_void_pointer(void) {
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
	if likely(!void_ptr) {
		Dee_Incref((DeeObject *)result);
		void_ptr = result;
	} else {
		ASSERT(void_ptr == result);
	}
	rwlock_endwrite(&static_type_lock);
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
	DeeObject_InitNoref(result, (DeeTypeObject *)result_type);
	result->p_ptr.ptr = symaddr;
	return (DREF DeeObject *)result;
err_type:
	Dee_Decref_unlikely((DeeObject *)result_type);
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
	return_bool_(symaddr != NULL);
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
		                                                        (ctypes_cc_t)((unsigned int)self->sh_defcc |
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
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&shlib_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&shlib_getitem
};

PRIVATE struct type_attr tpconst shlib_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&shlib_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))NULL,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))NULL,
	/* .tp_enumattr = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))NULL /* TODO */
};

PRIVATE WUNUSED DREF DeeObject *DCALL
shlib_base(Shlib *self, size_t argc,
           DeeObject *const *argv) {
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



PRIVATE struct type_method tpconst shlib_methods[] = {
	{ "base", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&shlib_base,
	  DOC("->?Aptr?Gvoid\n"
	      "Returns the base address of the shared library") },
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
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&shlib_init,
				TYPE_FIXED_ALLOCATOR(Shlib)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&shlib_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
#ifndef CONFIG_NO_CFUNCTION
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&shlib_visit,
#else /* !CONFIG_NO_CFUNCTION */
	/* .tp_visit         = */ NULL,
#endif /* CONFIG_NO_CFUNCTION */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &shlib_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ &shlib_attr,
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
