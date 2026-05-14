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
#ifndef GUARD_DEEMON_EXECUTE_MODULE_C
#define GUARD_DEEMON_EXECUTE_MODULE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_Free */
#include <deemon/arg.h>                /* DeeArg_Unpack1, DeeArg_UnpackStructKw */
#include <deemon/code.h>               /* DeeCodeObject, DeeCode_Empty, DeeCode_Type, DeeFunctionObject, DeeFunction_*, Dee_code_frame, Dee_code_object */
#include <deemon/computed-operators.h> /* DEFAULT_OPIMP, DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_ATTRIBUTE_ACCESS_DEL, DeeRT_ATTRIBUTE_ACCESS_SET, DeeRT_ErrRestrictedAttr, DeeRT_ErrUnboundAttrCStr */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/exec.h>               /* DeeExec_GetHome, DeeModule_GetLibPath, DeeModule_SetLibPath */
#include <deemon/format.h>             /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/gc.h>                 /* DeeGCObject_Free, DeeGC_Untrack, Dee_gc_head, _Dee_GC_HEAD_UNTRACKED_INIT */
#include <deemon/int.h>                /* DeeInt_NewUInt128 */
#include <deemon/module.h>             /* DeeModule*, Dee_MODSYM_F*, Dee_MODULE_F*, Dee_MODULE_HASHIT, Dee_MODULE_HASHNX, Dee_MODULE_HASHST, Dee_MODULE_INIT_INITIALIZED, Dee_MODULE_INIT_UNINITIALIZED, Dee_MODULE_MODDATA_INIT_CODE, Dee_MODULE_PROPERTY_DEL, Dee_MODULE_PROPERTY_GET, Dee_MODULE_PROPERTY_SET, Dee_MODULE_STRUCT_EX, Dee_MODULE_SYMBOL_EQUALS_STR, Dee_MODULE_SYMBOL_GETDOCSTR, Dee_MODULE_SYMBOL_GETNAMELEN, Dee_MODULE_SYMBOL_GETNAMESTR, Dee_module_*, _Dee_MODULE_* */
#include <deemon/mro.h>                /* DeeObject_GenericFindAttrInfoStringLenHash, DeeObject_TGenericFindAttr, DeeObject_TGenericIterAttr, Dee_ATTRINFO_CUSTOM, Dee_ATTRINFO_MODSYM, Dee_ATTRITER_HEAD, Dee_ATTRPERM_F_*, Dee_attrdesc, Dee_attrhint, Dee_attrinfo, Dee_attriter, Dee_attriter_init, Dee_attriter_type, Dee_attriterchain_builder, Dee_attriterchain_builder_*, Dee_attrperm_t, Dee_attrspec */
#include <deemon/none.h>               /* DeeNone_NewRef, Dee_None */
#include <deemon/object.h>             /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_Decref*, Dee_HAS_*, Dee_Incref, Dee_Movrefv, Dee_TYPE, Dee_WEAKREF_SUPPORT_ADDR, Dee_WEAKREF_SUPPORT_INIT, Dee_XDecref, Dee_XDecrefv, Dee_XIncref, Dee_XMovrefv, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, Dee_uint128_t, Dee_weakref_support_fini, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeRefVector_NewReadonly */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString*, STRING_ERROR_FIGNORE, WSTR_LENGTH */
#include <deemon/system-features.h>    /* DeeSystem_DEFINE_*, bcmpc, memcpy*, memset, strlen */
#include <deemon/system.h>             /* DeeSystem_HAVE_FS_ICASE */
#include <deemon/thread.h>             /* DeeThreadObject, DeeThread_Self */
#include <deemon/tuple.h>              /* DeeTuple*, Dee_EmptyTuple */
#include <deemon/type.h>               /* DeeObject_*, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visit, Dee_Visitv, Dee_XVisitv, Dee_visit_t, METHOD_F*, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* Dee_ATOMIC_ACQUIRE, Dee_atomic_cmpxch_val, atomic_* */
#include <deemon/util/futex.h>         /* DeeFutex_WaitPtr, DeeFutex_WakeAll */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint16_t, uintptr_t */

#ifndef CONFIG_NO_DEX
#include <deemon/dex.h> /* Dee_module_dexdata */
#endif /* !CONFIG_NO_DEX */

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

INTERN WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
Dee_module_symbol_getnameobj(struct Dee_module_symbol *__restrict self) {
	DREF DeeStringObject *result;
	char const *name_str;
	uint16_t flags;
again:
	name_str = self->ss_name;
	atomic_thread_fence(Dee_ATOMIC_ACQUIRE);
	flags = self->ss_flags;
	if (flags & Dee_MODSYM_FNAMEOBJ) {
		result = COMPILER_CONTAINER_OF(name_str,
		                               DeeStringObject,
		                               s_str);
		Dee_Incref(result);
	} else {
		/* Wrap the name string into a string object. */
		result = (DREF DeeStringObject *)DeeString_New(name_str);
		if unlikely(!result)
			goto done;

		/* Cache the name-string as part of the attribute structure. */
		if (!atomic_cmpxch_weak(&self->ss_name, name_str, DeeString_STR(result))) {
			Dee_Decref(result);
			goto again;
		}
		atomic_or(&self->ss_flags, Dee_MODSYM_FNAMEOBJ);
		Dee_Incref(result);
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
Dee_module_symbol_getdocobj(struct Dee_module_symbol *__restrict self) {
	DREF DeeStringObject *result;
	char const *doc_str;
	uint16_t flags;
again:
	doc_str = self->ss_doc;
	ASSERT(doc_str != NULL);
	atomic_thread_fence(Dee_ATOMIC_ACQUIRE);
	flags = self->ss_flags;
	if (flags & Dee_MODSYM_FDOCOBJ) {
		result = COMPILER_CONTAINER_OF(doc_str,
		                               DeeStringObject,
		                               s_str);
		Dee_Incref(result);
	} else {
		/* Wrap the doc string into a string object. */
		result = (DREF DeeStringObject *)DeeString_NewUtf8(doc_str,
		                                                   strlen(doc_str),
		                                                   STRING_ERROR_FIGNORE);
		if unlikely(!result)
			goto done;

		/* Cache the doc-string as part of the attribute structure. */
		if (!atomic_cmpxch_weak(&self->ss_doc, doc_str, DeeString_STR(result))) {
			Dee_Decref(result);
			goto again;
		}
		atomic_or(&self->ss_flags, Dee_MODSYM_FDOCOBJ);
		Dee_Incref(result);
	}
done:
	return result;
}




/* Ensure that imports of "self" have been initialized.
 * @return: 0 : Success.
 * @return: -1: An error was thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_InitializeImports(DeeModuleObject *__restrict self) {
	uint16_t i;
	for (i = 0; i < self->mo_importc; ++i) {
		DeeModuleObject *imp = self->mo_importv[i];
		int status = DeeModule_Initialize(imp);
		if (status < 0)
			return status;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_DoInitialize(DeeModuleObject *__restrict me) {
	DREF DeeFunctionObject *rootfunc;
	DREF DeeObject *rootresult;
#ifndef CONFIG_NO_DEX
	ASSERT(Dee_TYPE(me) == &DeeModuleDee_Type ||
	       Dee_TYPE(me) == &DeeModuleDex_Type);
	if (Dee_TYPE(me) == &DeeModuleDex_Type) {
		struct Dee_module_dexdata *dexdata;
		dexdata = me->mo_moddata.mo_dexdata;
		ASSERT(dexdata);
		ASSERT(dexdata->mdx_module == me);
		return dexdata->mdx_init ? (*dexdata->mdx_init)() : 0;
	}
#else /* !CONFIG_NO_DEX */
	ASSERT(Dee_TYPE(me) == &DeeModuleDee_Type);
#endif /* CONFIG_NO_DEX */
	/* Ensure that imports of "me" have been initialized */
	if unlikely(DeeModule_InitializeImports(me))
		goto err;
	rootfunc = (DREF DeeFunctionObject *)DeeModule_GetRootFunction(me);
	if unlikely(!rootfunc)
		goto err;
	rootresult = DeeFunction_Call(rootfunc, 0, NULL);
	Dee_Decref_likely(rootfunc);
	if unlikely(!rootresult)
		goto err;
	Dee_Decref_unlikely(rootresult); /* *_unlikely because it's probably "none" */
	return 0;
err:
	return -1;
}

/* Ensure that the initializer (aka. "mo_rootcode" code) of "self" has been run.
 * @return: 1 : The calling thread is already in the process of initializing "self".
 * @return: 0 : Success, or initializer was already executed.
 * @return: -1: An error was thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_Initialize(DeeModuleObject *__restrict self) {
	int init_status;
	DeeThreadObject *caller = DeeThread_Self();
	DeeThreadObject *status;
again:
	status = Dee_atomic_cmpxch_val(&self->mo_init, Dee_MODULE_INIT_UNINITIALIZED, caller);
	if (status == Dee_MODULE_INIT_INITIALIZED) {
		return 0; /* Initialization already complete */
	} else if (status == caller) {
		return 1; /* You're already doing the init right now (nested/self dependency case) */
	} else if (status != Dee_MODULE_INIT_UNINITIALIZED) {
		/* Some other thread is already doing an initialization */
		atomic_or(&self->mo_flags, Dee_MODULE_FWAITINIT);
		if (DeeFutex_WaitPtr(&self->mo_init, (uintptr_t)status))
			goto err;
		goto again;
	}

	/* Initialization started */
	init_status = DeeModule_DoInitialize(self);

	/* Propagate initialization status */
	if likely(init_status == 0) {
		atomic_write(&self->mo_init, Dee_MODULE_INIT_INITIALIZED);
	} else {
		atomic_write(&self->mo_init, Dee_MODULE_INIT_UNINITIALIZED);
	}
	if (atomic_fetchand(&self->mo_flags, ~Dee_MODULE_FWAITINIT))
		DeeFutex_WakeAll(&self->mo_init);

	/* Return indicative of initialization succeeding */
	ASSERT(init_status == 0 || init_status == -1);
	return init_status;
err:
	return -1;
}

/* Check if the given module's current stat is `Dee_MODULE_INIT_UNINITIALIZED',
 * and if so: change it to `Dee_MODULE_INIT_INITIALIZED' (even if the module
 * may not have already been initialized)
 * @return: * : One of `DeeModule_SetInitialized_*' */
PUBLIC NONNULL((1)) unsigned int DCALL
DeeModule_SetInitialized(DeeModuleObject *__restrict self) {
	DeeThreadObject *status;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeModuleDee_Type);
	status = Dee_atomic_cmpxch_val(&self->mo_init,
	                               Dee_MODULE_INIT_UNINITIALIZED,
	                               Dee_MODULE_INIT_INITIALIZED);
	if (status == Dee_MODULE_INIT_UNINITIALIZED) {
		return DeeModule_SetInitialized_SUCCESS;
	} else if (status == Dee_MODULE_INIT_INITIALIZED) {
		return DeeModule_SetInitialized_ALREADY;
	} else {
		return DeeModule_SetInitialized_INPRGRS;
	}
	__builtin_unreachable();
}


/* Return the root code object of a given module.
 * The caller must ensure that `self' is an instance of "DeeModuleDee_Type" */
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF struct Dee_code_object *DCALL
DeeModule_GetRootCode(DeeModuleObject *__restrict self) {
	DREF DeeCodeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeModuleDee_Type);
	DeeModule_LockRead(self);
	result = self->mo_moddata.mo_rootcode;
	Dee_Incref(result);
	DeeModule_LockEndRead(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF /*Function*/ DeeObject *DCALL
DeeModule_GetRootFunction(DeeModuleObject *__restrict self) {
	DREF /*Function*/ DeeObject *rootfunc;
	DREF DeeCodeObject *rootcode;
	rootcode = DeeModule_GetRootCode(self);
	ASSERT_OBJECT_TYPE_EXACT(rootcode, &DeeCode_Type);
	rootfunc = DeeFunction_NewNoRefs(rootcode);
	Dee_Decref_unlikely(rootcode);
	return rootfunc;
}


INTERN struct Dee_module_symbol empty_module_buckets[] = {
	{ NULL, 0, 0 }
};


PUBLIC WUNUSED NONNULL((1)) struct Dee_module_symbol *DCALL
DeeModule_GetSymbolID(DeeModuleObject const *__restrict self, uint16_t gid) {
	struct Dee_module_symbol *iter, *end;
	struct Dee_module_symbol *result = NULL;

	end = (iter = self->mo_bucketv) + (self->mo_bucketm + 1);
	for (; iter < end; ++iter) {
		if (!Dee_MODULE_SYMBOL_GETNAMESTR(iter))
			continue; /* Skip empty entries. */
		if (Dee_module_symbol_getindex(iter) == gid) {
			result = iter;

			/* If it's a symbol, we still stored it's name as
			 * the result value, meaning we won't return NULL
			 * but still keep on searching for another symbol
			 * that isn't an alias. */
			if (!(iter->ss_flags & Dee_MODSYM_FALIAS))
				break;
		}
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL
DeeModule_GetSymbol(DeeModuleObject const *__restrict self,
                    /*String*/ DeeObject *__restrict name) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(DeeString_STR(name), item->ss_name) != 0)
			continue; /* Differing strings. */
		return item;  /* Found it! */
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL
DeeModule_GetSymbolStringHash(DeeModuleObject const *__restrict self,
                              char const *__restrict attr_name,
                              Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name) != 0)
			continue;
		return item;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL
DeeModule_GetSymbolStringLenHash(DeeModuleObject const *__restrict self,
                                 char const *__restrict attr_name,
                                 size_t attrlen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) != 0)
			continue;
		return item;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrSymbol(DeeModuleObject *__restrict self,
                        struct Dee_module_symbol const *__restrict sym) {
	ASSERT(sym >= self->mo_bucketv &&
	       sym <= self->mo_bucketv + self->mo_bucketm);
	if likely(!(sym->ss_flags & (Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY))) {
		DREF DeeObject *result;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[Dee_module_symbol_getindex(sym)];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result)
			err_unbound_global(self, Dee_module_symbol_getindex(sym));
		return result;
	}

	/* External symbol, or property. */
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback) {
			err_module_cannot_read_property_string(self, Dee_MODULE_SYMBOL_GETNAMESTR(sym));
			return NULL;
		}

		/* Invoke the property callback. */
		return DeeObject_CallInherited(callback, 0, NULL);
	}

	/* External symbol. */
	ASSERT(sym->ss_impid < self->mo_importc);
	self = self->mo_importv[sym->ss_impid];
	goto read_symbol;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrSymbol(DeeModuleObject *__restrict self,
                          struct Dee_module_symbol const *__restrict sym) {
	ASSERT(sym >= self->mo_bucketv &&
	       sym <= self->mo_bucketv + self->mo_bucketm);
	if likely(!(sym->ss_flags & (Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY))) {
		DeeObject *value;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
		DeeModule_LockRead(self);
		value = self->mo_globalv[Dee_module_symbol_getindex(sym)];
		DeeModule_LockEndRead(self);
		return Dee_BOUND_FROMBOOL(value);
	}

	/* External symbol, or property. */
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		DREF DeeObject *callback, *callback_result;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return Dee_BOUND_NO;

		/* Invoke the property callback. */
		callback_result = DeeObject_CallInherited(callback, 0, NULL);
		if likely(callback_result) {
			Dee_Decref(callback_result);
			return Dee_BOUND_YES;
		}
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return Dee_BOUND_NO;
		return Dee_BOUND_ERR;
	}

	/* External symbol. */
	ASSERT(sym->ss_impid < self->mo_importc);
	self = self->mo_importv[sym->ss_impid];
	goto read_symbol;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrSymbol(DeeModuleObject *__restrict self,
                        struct Dee_module_symbol const *__restrict sym) {
	DREF DeeObject *old_value;
	ASSERT(sym >= self->mo_bucketv &&
	       sym <= self->mo_bucketv + self->mo_bucketm);
	if unlikely(sym->ss_flags & (Dee_MODSYM_FREADONLY | Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY)) {
		if (sym->ss_flags & Dee_MODSYM_FREADONLY)
			return err_module_readonly_global_string(self, Dee_MODULE_SYMBOL_GETNAMESTR(sym));
		if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
			DREF DeeObject *callback, *temp;
			DeeModule_LockRead(self);
			ASSERT(Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_DEL < self->mo_globalc);
			callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_DEL];
			Dee_XIncref(callback);
			DeeModule_LockEndRead(self);
			if unlikely(!callback)
				return err_module_cannot_delete_property_string(self, Dee_MODULE_SYMBOL_GETNAMESTR(sym));

			/* Invoke the property callback. */
			temp = DeeObject_CallInherited(callback, 0, NULL);
			Dee_XDecref(temp);
			return temp ? 0 : -1;
		}

		/* External symbol. */
		ASSERT(sym->ss_impid < self->mo_importc);
		self = self->mo_importv[sym->ss_impid];
	}
	ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
	DeeModule_LockWrite(self);
	old_value = self->mo_globalv[Dee_module_symbol_getindex(sym)];
	self->mo_globalv[Dee_module_symbol_getindex(sym)] = NULL;
	DeeModule_LockEndWrite(self);
	Dee_XDecref(old_value);
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeModule_SetAttrSymbol(DeeModuleObject *__restrict self,
                        struct Dee_module_symbol const *__restrict sym,
                        DeeObject *__restrict value) {
	DREF DeeObject *temp;
	ASSERT(sym >= self->mo_bucketv &&
	       sym <= self->mo_bucketv + self->mo_bucketm);
	if unlikely(sym->ss_flags & (Dee_MODSYM_FREADONLY | Dee_MODSYM_FPROPERTY | Dee_MODSYM_FEXTERN)) {
		if unlikely(sym->ss_flags & Dee_MODSYM_FEXTERN) {
			ASSERT(sym->ss_impid < self->mo_importc);
			self = self->mo_importv[sym->ss_impid];
		}
		if (sym->ss_flags & Dee_MODSYM_FREADONLY) {
			if (sym->ss_flags & Dee_MODSYM_FPROPERTY)
				goto err_is_readonly;
			ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
			DeeModule_LockWrite(self);
			/* Make sure not to allow write-access to global variables that
			 * have already been assigned, but are marked as read-only. */
			if unlikely(self->mo_globalv[Dee_module_symbol_getindex(sym)] != NULL) {
				DeeModule_LockEndWrite(self);
err_is_readonly:
				return err_module_readonly_global_string(self, Dee_MODULE_SYMBOL_GETNAMESTR(sym));
			}
			Dee_Incref(value);
			self->mo_globalv[Dee_module_symbol_getindex(sym)] = value;
			DeeModule_LockEndWrite(self);
			return 0;
		}
		if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
			DREF DeeObject *callback;
			DeeModule_LockWrite(self);
			callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_SET];
			Dee_XIncref(callback);
			DeeModule_LockEndWrite(self);
			if unlikely(!callback)
				return err_module_cannot_write_property_string(self, Dee_MODULE_SYMBOL_GETNAMESTR(sym));
			temp = DeeObject_CallInherited(callback, 1, (DeeObject **)&value);
			Dee_XDecref(temp);
			return temp ? 0 : -1;
		}
		/* External symbol. */
		ASSERT(sym->ss_impid < self->mo_importc);
		self = self->mo_importv[sym->ss_impid];
	}
	ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
	Dee_Incref(value);
	DeeModule_LockWrite(self);
	temp = self->mo_globalv[Dee_module_symbol_getindex(sym)];
	self->mo_globalv[Dee_module_symbol_getindex(sym)] = value;
	DeeModule_LockEndWrite(self);
	Dee_XDecref(temp);
	return 0;
}


#ifdef DeeSystem_HAVE_FS_ICASE
#define FS_strcmp  strcasecmp
#define FS_strcmpz strcasecmpz
#ifndef CONFIG_HAVE_strcasecmp
#define CONFIG_HAVE_strcasecmp
#undef strcasecmp
#define strcasecmp dee_strcasecmp
DeeSystem_DEFINE_strcasecmp(dee_strcasecmp)
#endif /* !CONFIG_HAVE_strcasecmp */
#ifndef CONFIG_HAVE_strcasecmpz
#define CONFIG_HAVE_strcasecmpz
#undef strcasecmpz
#define strcasecmpz dee_strcasecmpz
DeeSystem_DEFINE_strcasecmpz(dee_strcasecmpz)
#endif /* !CONFIG_HAVE_strcasecmpz */
#else /* DeeSystem_HAVE_FS_ICASE */
#define FS_strcmp  strcmp
#define FS_strcmpz strcmpz
#ifndef CONFIG_HAVE_strcmpz
#define CONFIG_HAVE_strcmpz
#undef strcmpz
#define strcmpz dee_strcmpz
DeeSystem_DEFINE_strcmpz(dee_strcmpz)
#endif /* !CONFIG_HAVE_strcmpz */
#endif /* !DeeSystem_HAVE_FS_ICASE */



#define DeeModule_DIRECTORY_ATTR_NO  ((DeeStringObject *)NULL)
#define DeeModule_DIRECTORY_ATTR_ERR ((DeeStringObject *)ITER_DONE)

/* @return: * :                           Yes
 * @return: DeeModule_DIRECTORY_ATTR_NO:  No
 * @return: DeeModule_DIRECTORY_ATTR_ERR: Error */
PRIVATE WUNUSED NONNULL((1)) DeeStringObject *DCALL
DeeModule_FindDirectoryAttrString(DeeModuleObject *__restrict self,
                                  char const *__restrict attr_name) {
	size_t lo, hi;
	DeeTupleObject *dir;
	dir = (DeeTupleObject *)DeeModule_GetDirectory(self);
	if unlikely(!dir)
		goto err;
	lo = 0;
	hi = DeeTuple_SIZE(dir);
	while (lo < hi) {
		int diff;
		size_t mid = (lo + hi) / 2;
		DeeStringObject *name = (DeeStringObject *)DeeTuple_GET(dir, mid);
		char const *name_utf8 = DeeString_AsUtf8(name);
		if unlikely(!name_utf8)
			goto err;
		diff = FS_strcmp(name_utf8, attr_name);
		if (diff > 0) {
			hi = mid;
		} else if (diff < 0) {
			lo = mid + 1;
		} else {
			return name;
		}
	}
	return DeeModule_DIRECTORY_ATTR_NO;
err:
	return DeeModule_DIRECTORY_ATTR_ERR;
}

/* @return: * :                           Yes
 * @return: DeeModule_DIRECTORY_ATTR_NO:  No
 * @return: DeeModule_DIRECTORY_ATTR_ERR: Error */
PRIVATE WUNUSED NONNULL((1)) DeeStringObject *DCALL
DeeModule_FindDirectoryAttrStringLen(DeeModuleObject *__restrict self,
                                     char const *__restrict attr_name,
                                     size_t attrlen) {
	size_t lo, hi;
	DeeTupleObject *dir;
	dir = (DeeTupleObject *)DeeModule_GetDirectory(self);
	if unlikely(!dir)
		goto err;
	lo = 0;
	hi = DeeTuple_SIZE(dir);
	while (lo < hi) {
		int diff;
		size_t mid = (lo + hi) / 2;
		DeeStringObject *name = (DeeStringObject *)DeeTuple_GET(dir, mid);
		char const *name_utf8 = DeeString_AsUtf8(name);
		if unlikely(!name_utf8)
			goto err;
		diff = FS_strcmpz(name_utf8, attr_name, attrlen);
		if (diff > 0) {
			hi = mid;
		} else if (diff < 0) {
			lo = mid + 1;
		} else {
			return name;
		}
	}
	return DeeModule_DIRECTORY_ATTR_NO;
err:
	return DeeModule_DIRECTORY_ATTR_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DeeStringObject *DCALL
DeeModule_FindDirectoryAttr(DeeModuleObject *__restrict self,
                            /*String*/ DeeObject *__restrict attr_name) {
	char const *name = DeeString_AsUtf8(attr_name);
	if unlikely(!name)
		goto err;
	return DeeModule_FindDirectoryAttrStringLen(self, name, WSTR_LENGTH(name));
err:
	return DeeModule_DIRECTORY_ATTR_ERR;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttr(DeeModuleObject *self, /*String*/ DeeObject *attr) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb, hash;
	DREF DeeObject *result;
	hash = DeeString_Hash(attr);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_EQUALS_STR(item, attr))
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttr(Dee_AsObject(self), attr);
	if (result != ITER_DONE)
		return result;

	/* Check for a directory element */
	if (!self->mo_dir) {
		/* Directory not yet loaded -- try to do the import so we don't have to load it now */
		result = Dee_AsObject(DeeModule_ImportChild(self, attr,
		                                            DeeModule_IMPORT_F_NORMAL |
		                                            DeeModule_IMPORT_F_CTXDIR |
		                                            DeeModule_IMPORT_F_ENOENT));
		if (result != Dee_AsObject(DeeModule_IMPORT_ENOENT))
			return result;
	} else {
		dir_status = DeeModule_FindDirectoryAttr(self, attr);
		if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
			if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
				goto err;
			return Dee_AsObject(DeeModule_ImportChild(self, attr,
			                                          DeeModule_IMPORT_F_NORMAL |
			                                          DeeModule_IMPORT_F_CTXDIR));
		}
	}
	err_module_no_such_global(self, attr, ATTR_ACCESS_GET);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringHash(Dee_AsObject(self), attr_name, hash);
	if (result != ITER_DONE)
		return result;

	/* Check for a directory element */
	if (!self->mo_dir) {
		/* Directory not yet loaded -- try to do the import so we don't have to load it now */
		result = Dee_AsObject(DeeModule_ImportChildEx(self, attr_name, strlen(attr_name),
		                                              DeeModule_IMPORT_F_NORMAL |
		                                              DeeModule_IMPORT_F_CTXDIR |
		                                              DeeModule_IMPORT_F_ENOENT,
		                                              NULL));
		if (result != Dee_AsObject(DeeModule_IMPORT_ENOENT))
			return result;
	} else {
		dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
		if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
			if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
				goto err;
			return Dee_AsObject(DeeModule_ImportChildEx(self, attr_name, strlen(attr_name),
			                                            DeeModule_IMPORT_F_NORMAL |
			                                            DeeModule_IMPORT_F_CTXDIR,
			                                            NULL));
		}
	}
	err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_GET);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringLenHash(Dee_AsObject(self), attr_name, attrlen, hash);
	if (result != ITER_DONE)
		return result;

	/* Check for a directory element */
	if (!self->mo_dir) {
		/* Directory not yet loaded -- try to do the import so we don't have to load it now */
		result = Dee_AsObject(DeeModule_ImportChildEx(self, attr_name, attrlen,
		                                              DeeModule_IMPORT_F_NORMAL |
		                                              DeeModule_IMPORT_F_CTXDIR |
		                                              DeeModule_IMPORT_F_ENOENT,
		                                              NULL));
		if (result != Dee_AsObject(DeeModule_IMPORT_ENOENT))
			return result;
	} else {
		dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
		if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
			if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
				goto err;
			return Dee_AsObject(DeeModule_ImportChildEx(self, attr_name, attrlen,
			                                            DeeModule_IMPORT_F_NORMAL |
			                                            DeeModule_IMPORT_F_CTXDIR,
			                                            NULL));
		}
	}
	err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_GET);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttr(DeeModuleObject *self, /*String*/ DeeObject *attr) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(attr);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_EQUALS_STR(item, attr))
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericBoundAttr(Dee_AsObject(self), attr);
	if (result != Dee_BOUND_MISSING)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttr(self, attr);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_BOUND_YES;
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringHash(DeeModuleObject *__restrict self,
                              char const *__restrict attr_name,
                              Dee_hash_t hash) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericBoundAttrStringHash(Dee_AsObject(self), attr_name, hash);
	if (result != Dee_BOUND_MISSING)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_BOUND_YES;
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringLenHash(DeeModuleObject *__restrict self,
                                 char const *__restrict attr_name,
                                 size_t attrlen, Dee_hash_t hash) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericBoundAttrStringLenHash(Dee_AsObject(self), attr_name, attrlen, hash);
	if (result != Dee_BOUND_MISSING)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_BOUND_YES;
	}

	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttr(DeeModuleObject *self, /*String*/ DeeObject *attr) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(attr);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_EQUALS_STR(item, attr))
			return Dee_HAS_YES;
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericHasAttr(Dee_AsObject(self), attr);
	if (result != Dee_HAS_NO)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttr(self, attr);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_HAS_YES;
	}
	return Dee_HAS_NO;
err:
	return Dee_HAS_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return Dee_HAS_YES;
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericHasAttrStringHash(Dee_AsObject(self), attr_name, hash);
	if (result != Dee_HAS_NO)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_HAS_YES;
	}
	return Dee_HAS_NO;
err:
	return Dee_HAS_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return Dee_HAS_YES;
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericHasAttrStringLenHash(Dee_AsObject(self), attr_name, attrlen, hash);
	if (result != Dee_HAS_NO)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_HAS_YES;
	}
	return Dee_HAS_NO;
err:
	return Dee_HAS_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttr(DeeModuleObject *self, /*String*/ DeeObject *attr) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(attr);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_EQUALS_STR(item, attr))
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericDelAttr(Dee_AsObject(self), attr);
	if unlikely(result <= 0)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttr(self, attr);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_DEL);
	}
	return err_module_no_such_global(self, attr, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericDelAttrStringHash(Dee_AsObject(self), attr_name, hash);
	if unlikely(result <= 0)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_DEL);
	}
	return err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericDelAttrStringLenHash(Dee_AsObject(self), attr_name, attrlen, hash);
	if unlikely(result <= 0)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_DEL);
	}
	return err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeModule_SetAttr(DeeModuleObject *self, /*String*/ DeeObject *attr, DeeObject *value) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(attr);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_EQUALS_STR(item, attr))
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericSetAttr(Dee_AsObject(self), attr, value);
	if unlikely(result <= 0)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttr(self, attr);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_SET);
	}
	return err_module_no_such_global(self, attr, ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeModule_SetAttrStringHash(DeeModuleObject *self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash, DeeObject *value) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericSetAttrStringHash(Dee_AsObject(self), attr_name, hash, value);
	if unlikely(result <= 0)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_SET);
	}
	return err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeModule_SetAttrStringLenHash(DeeModuleObject *self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash,
                               DeeObject *value) {
	int result;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (Dee_MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(Dee_MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Do a generic attribute lookup on the module. */
	result = DeeObject_GenericSetAttrStringLenHash(Dee_AsObject(self), attr_name, attrlen, hash, value);
	if unlikely(result <= 0)
		return result;

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_SET);
	}
	return err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_SET);
}



/* Return the name of a global variable in the given module.
 * @return: NULL: The given `gid' is not recognized, or the module hasn't finished/started loading yet.
 * @return: * :   The name of the global associated with `gid'.
 *                Note that in the case of aliases existing for `gid', this function prefers not to
 *                return the name of an alias, but that of the original symbol itself, so long as that
 *                symbol actually exist, which if it doesn't, it will return the name of a random alias. */
PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GlobalName(DeeModuleObject *__restrict self, uint16_t gid) {
	struct Dee_module_symbol *sym;
	sym = DeeModule_GetSymbolID(self, gid);
	return sym ? sym->ss_name : NULL;
}


DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_str(DeeObject *__restrict self) { /* TODO: Refactor to "tp_print" */
	DeeModuleObject *me = (DeeModuleObject *)self;
	char const *name;
	DREF DeeObject *libname = DeeModule_GetLibName(me, 0);
	if (libname != ITER_DONE)
		return libname;
	name = me->mo_absname;
	if (!name)
		return DeeString_New("<Anonymous module>");
	if (!*name)
		return DeeString_New("<Filesystem root module>");
	return DeeString_Newf("<Module %q>", name);
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
module_printrepr(DeeObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	DeeModuleObject *me = (DeeModuleObject *)self;
	DREF DeeObject *libname = DeeModule_GetLibName(me, 0);
	if (ITER_ISOK(libname))
		return DeeFormat_Printf(printer, arg, "import.%K", libname);
	if unlikely(!libname)
		return -1;
	if (me->mo_absname) {
		if (!*me->mo_absname) {
			/* TODO: import("/") on linux, but no way to easily express on windows,
			 *       where the module needs to be accessed via `import("......")',
			 *       with the # of '.' needing to be 1+ the number of '/' in the
			 *       caller's module. */
		}
		return DeeFormat_Printf(printer, arg, "import(%q)", me->mo_absname);
	}
	return DeeFormat_PRINT(printer, arg, "Module()");
}


#define module_getattr   DeeModule_GetAttr
#define module_delattr   DeeModule_DelAttr
#define module_setattr   DeeModule_SetAttr
#define module_hasattr   DeeModule_HasAttr
#define module_boundattr DeeModule_BoundAttr

struct module_attriter {
	Dee_ATTRITER_HEAD
	DeeModuleObject *mai_mod;  /* [1..1][const] The module whose attributes to enumerate. */
	size_t           mai_didx; /* [lock(ATOMIC)] Index into `mai_mod->mo_dir->md_files' */
	uint16_t         mai_hidx; /* [lock(ATOMIC)] Index into `mai_mod->mo_bucketv' */
};

INTDEF struct type_attr module_attr;

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_attriter_next(struct module_attriter *__restrict self,
                     /*out*/ struct Dee_attrdesc *__restrict desc) {
	DeeModuleObject *mod = self->mai_mod;
	struct Dee_module_symbol *sym, *doc_sym;
	uint16_t old_hidx, new_hidx;
	uint16_t perm;
	do {
		old_hidx = atomic_read(&self->mai_hidx);
		new_hidx = old_hidx;
		do {
			if unlikely(new_hidx > mod->mo_bucketm) {
				size_t didx;
				DeeTupleObject *dir;
				DeeStringObject *file;
				dir = (DeeTupleObject *)DeeModule_GetDirectory(mod);
				if unlikely(!dir)
					goto err;
				do {
					didx = atomic_read(&self->mai_didx);
					if (didx >= DeeTuple_SIZE(dir))
						return 1;
				} while (!atomic_cmpxch_or_write(&self->mai_didx, didx, didx + 1));
				file = (DeeStringObject *)DeeTuple_GET(dir, didx);
				ASSERT_OBJECT_TYPE_EXACT(file, &DeeString_Type);
				Dee_Incref(file);
				desc->ad_name = DeeString_STR(file);
				desc->ad_doc  = NULL;
				desc->ad_perm = Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_IMEMBER |
				                Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_CANGET;
				desc->ad_info.ai_decl = (DeeObject *)mod;
				desc->ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
				desc->ad_info.ai_value.v_custom = &module_attr;
				Dee_Incref(&DeeModule_Type);
				desc->ad_type = &DeeModule_Type;
				return 0;
			}
			sym = &mod->mo_bucketv[new_hidx];
			++new_hidx;
			/* Skip empty and hidden entries. */
		} while (!Dee_MODULE_SYMBOL_GETNAMESTR(sym) || (sym->ss_flags & Dee_MODSYM_FHIDDEN));
	} while (!atomic_cmpxch_or_write(&self->mai_hidx, old_hidx, new_hidx));

	perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET;
	ASSERT(Dee_module_symbol_getindex(sym) < mod->mo_globalc);
	if (!(sym->ss_flags & Dee_MODSYM_FREADONLY))
		perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		perm &= ~(Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if (!(sym->ss_flags & Dee_MODSYM_FCONSTEXPR))
			perm |= Dee_ATTRPERM_F_PROPERTY;
	}

#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
	/* For constant-expression symbols, we can predict
	 * their type (as well as their value)... */
	if (sym->ss_flags & Dee_MODSYM_FCONSTEXPR)
#endif
	{
		int init_state = DeeModule_Initialize(mod);
		if unlikely(init_state < 0)
			goto err;
		if likely(init_state == 0) {
			DeeModule_LockRead(mod);
			if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
				/* Check which property operations have been bound. */
				if (mod->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET])
					perm |= Dee_ATTRPERM_F_CANGET;
				if (!(sym->ss_flags & Dee_MODSYM_FREADONLY)) {
					/* These callbacks are only allocated if the READONLY flag isn't set. */
					if (mod->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_DEL])
						perm |= Dee_ATTRPERM_F_CANDEL;
					if (mod->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_SET])
						perm |= Dee_ATTRPERM_F_CANSET;
				}
			}
			DeeModule_LockEndRead(mod);
		}
	}
	doc_sym = sym;
	if (!doc_sym->ss_doc && (doc_sym->ss_flags & Dee_MODSYM_FALIAS)) {
		doc_sym = DeeModule_GetSymbolID(mod, Dee_module_symbol_getindex(doc_sym));
		ASSERT(doc_sym != NULL);
	}

	/* NOTE: Pass the module instance as declarator! */
	if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ)
		perm |= Dee_ATTRPERM_F_NAMEOBJ;
	if (doc_sym->ss_flags & Dee_MODSYM_FDOCOBJ)
		perm |= Dee_ATTRPERM_F_DOCOBJ;
	desc->ad_name = Dee_MODULE_SYMBOL_GETNAMESTR(sym);
	desc->ad_doc  = Dee_MODULE_SYMBOL_GETDOCSTR(doc_sym);
	if (perm & Dee_ATTRPERM_F_NAMEOBJ)
		Dee_Incref(COMPILER_CONTAINER_OF(desc->ad_name, DeeStringObject, s_str));
	if ((perm & Dee_ATTRPERM_F_DOCOBJ) && desc->ad_doc)
		Dee_Incref(COMPILER_CONTAINER_OF(desc->ad_doc, DeeStringObject, s_str));
	desc->ad_perm = perm;
	desc->ad_info.ai_decl = (DeeObject *)mod;
	desc->ad_info.ai_type = Dee_ATTRINFO_MODSYM;
	desc->ad_info.ai_value.v_modsym = sym;
	desc->ad_type = NULL;
	return 0;
err:
	return -1;
}

PRIVATE struct Dee_attriter_type tpconst module_attriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&module_attriter_next,
};

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
module_iterattr(DeeTypeObject *UNUSED(tp_self), DeeModuleObject *self,
                struct Dee_attriter *iterbuf, size_t bufsize,
                struct Dee_attrhint const *__restrict hint) {
	size_t temp;
	struct Dee_attriterchain_builder builder;
	ASSERT_OBJECT(self);

	Dee_attriterchain_builder_init(&builder, iterbuf, bufsize);
	if (Dee_attriterchain_builder_getbufsize(&builder) >= sizeof(struct module_attriter)) {
		struct module_attriter *iter;
		iter = (struct module_attriter *)Dee_attriterchain_builder_getiterbuf(&builder);
		Dee_attriter_init(iter, &module_attriter_type);
		iter->mai_didx = 0;
		iter->mai_hidx = 0;
		iter->mai_mod  = self;
	}
	Dee_attriterchain_builder_consume(&builder, sizeof(struct module_attriter));
	temp = DeeObject_TGenericIterAttr(&DeeModule_Type,
	                                  Dee_attriterchain_builder_getiterbuf(&builder),
	                                  Dee_attriterchain_builder_getbufsize(&builder),
	                                  hint);
	if unlikely(temp == (size_t)-1)
		goto err_temp;
	return Dee_attriterchain_builder_pack(&builder);
err_temp:
	Dee_attriterchain_builder_fini(&builder);
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
module_findattr_impl(DeeModuleObject *__restrict self,
                     struct Dee_attrspec const *__restrict specs,
                     struct Dee_attrdesc *__restrict result) {
	ASSERT_OBJECT(self);
	if (!specs->as_decl || specs->as_decl == Dee_AsObject(self)) {
		struct Dee_module_symbol *sym;
		sym = DeeModule_GetSymbolStringHash(self,
		                                    specs->as_name,
		                                    specs->as_hash);
		if (sym) {
			struct Dee_module_symbol *doc_sym;
			Dee_attrperm_t perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET;
			ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
			if (!(sym->ss_flags & Dee_MODSYM_FREADONLY))
				perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
			if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
				perm &= ~(Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
				if (!(sym->ss_flags & Dee_MODSYM_FCONSTEXPR))
					perm |= Dee_ATTRPERM_F_PROPERTY;
			}
#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
			/* For constant-expression symbols, we can predict
			 * their type (as well as their value)... */
			if (sym->ss_flags & Dee_MODSYM_FCONSTEXPR)
#endif
			{
				int init_state = DeeModule_Initialize(self);
				if unlikely(init_state < 0)
					goto err;
				if likely(init_state == 0) {
					DeeModule_LockRead(self);
					if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
						/* Check which property operations have been bound. */
						if (self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET])
							perm |= Dee_ATTRPERM_F_CANGET;
						if (!(sym->ss_flags & Dee_MODSYM_FREADONLY)) {
							/* These callbacks are only allocated if the READONLY flag isn't set. */
							if (self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_DEL])
								perm |= Dee_ATTRPERM_F_CANDEL;
							if (self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_SET])
								perm |= Dee_ATTRPERM_F_CANSET;
						}
					}
					DeeModule_LockEndRead(self);
				}
			}
			doc_sym = sym;
			if (!doc_sym->ss_doc && (doc_sym->ss_flags & Dee_MODSYM_FALIAS)) {
				doc_sym = DeeModule_GetSymbolID(self, Dee_module_symbol_getindex(doc_sym));
				ASSERT(doc_sym != NULL);
			}
			if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ)
				perm |= Dee_ATTRPERM_F_NAMEOBJ;
			if (doc_sym->ss_flags & Dee_MODSYM_FDOCOBJ)
				perm |= Dee_ATTRPERM_F_DOCOBJ;
			if ((perm & specs->as_perm_mask) == specs->as_perm_value) {
				/* NOTE: Pass the module instance as declarator! */
				result->ad_name = Dee_MODULE_SYMBOL_GETNAMESTR(sym);
				result->ad_doc  = Dee_MODULE_SYMBOL_GETDOCSTR(doc_sym);
				result->ad_perm = perm;
				result->ad_info.ai_decl = Dee_AsObject(self);
				result->ad_info.ai_type = Dee_ATTRINFO_MODSYM;
				result->ad_info.ai_value.v_modsym = sym;
				if (perm & Dee_ATTRPERM_F_NAMEOBJ)
					Dee_Incref(COMPILER_CONTAINER_OF(result->ad_name, DeeStringObject, s_str));
				if ((perm & Dee_ATTRPERM_F_DOCOBJ) && result->ad_doc)
					Dee_Incref(COMPILER_CONTAINER_OF(result->ad_doc, DeeStringObject, s_str));
				result->ad_type = NULL;
				return 0;
			}
		}
#define DIRATTR_perm (Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_CANGET)
		if ((DIRATTR_perm & specs->as_perm_mask) == specs->as_perm_value) {
			DeeStringObject *dir_status;
			dir_status = DeeModule_FindDirectoryAttrString(self, specs->as_name);
			if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
				if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
					goto err;
				Dee_Incref(dir_status);
				result->ad_name = DeeString_STR(dir_status);
				result->ad_doc  = NULL;
				result->ad_perm = Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_IMEMBER |
				                  Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_CANGET;
				result->ad_info.ai_decl = Dee_AsObject(self);
				result->ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
				result->ad_info.ai_value.v_custom = &module_attr;
				Dee_Incref(&DeeModule_Type);
				result->ad_type = &DeeModule_Type;
				return 0;
			}
		}
#undef DIRATTR_perm
	}
	return DeeObject_TGenericFindAttr(&DeeModule_Type,
	                                  Dee_AsObject(self),
	                                  specs, result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) bool DCALL
DeeModule_FindAttrInfoStringLenHash(DeeModuleObject *self, char const *__restrict attr, size_t attrlen,
                                    Dee_hash_t hash, struct Dee_attrinfo *__restrict retinfo) {
	struct Dee_module_symbol const *sym;
	sym = DeeModule_GetSymbolStringLenHash(self, attr, attrlen, hash);
	if (sym) {
		retinfo->ai_type = Dee_ATTRINFO_MODSYM;
		retinfo->ai_decl = Dee_AsObject(self);
		retinfo->ai_value.v_modsym = sym;
		return true;
	}
	return DeeObject_GenericFindAttrInfoStringLenHash(self, attr, attrlen, hash, retinfo);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
module_findattr(DeeTypeObject *UNUSED(tp_self), DeeModuleObject *self,
                struct Dee_attrspec const *__restrict specs,
                struct Dee_attrdesc *__restrict result) {
	return module_findattr_impl(self, specs, result);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 6)) bool DCALL
module_findattr_info_string_len_hash(DeeTypeObject *tp_self, DeeModuleObject *self,
                                     char const *__restrict attr, size_t attrlen, Dee_hash_t hash,
                                     struct Dee_attrinfo *__restrict retinfo) {
	(void)tp_self;
	return DeeModule_FindAttrInfoStringLenHash(self, attr, attrlen, hash, retinfo);
}

INTERN struct type_attr module_attr = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&module_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&module_delattr,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&module_setattr,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&module_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&module_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&module_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&module_boundattr,
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeModule_GetAttrStringHash,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeModule_DelAttrStringHash,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&DeeModule_SetAttrStringHash,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeModule_HasAttrStringHash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeModule_BoundAttrStringHash,
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeModule_GetAttrStringLenHash,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeModule_DelAttrStringLenHash,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&DeeModule_SetAttrStringLenHash,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeModule_HasAttrStringLenHash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeModule_BoundAttrStringLenHash,
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
	/* .tp_findattr_info_string_len_hash = */ (bool (DCALL *)(DeeTypeObject *, DeeObject *, char const *__restrict, size_t, Dee_hash_t, struct Dee_attrinfo *__restrict))&module_findattr_info_string_len_hash,
};

#define module_get_code DeeModule_GetRootCode

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_name(DeeModuleObject *__restrict self) {
	char const *name;
	if ((self->mo_flags & Dee_MODULE_FABSFILE) && Dee_TYPE(self) != &DeeModuleDir_Type)
		goto unbound;
	name = DeeModule_GetShortName(self);
	if unlikely(!name)
		goto unbound;
	return DeeString_NewUtf8(name, strlen(name), STRING_ERROR_FIGNORE);
unbound:
	return DeeRT_ErrUnboundAttrCStr(self, "__name__");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_buildid(DeeModuleObject *__restrict self) {
	Dee_uint128_t buildid_int;
	union Dee_module_buildid const *buildid;
	buildid = DeeModule_GetBuildId(self);
	if unlikely(!buildid)
		goto err;
	memcpy(&buildid_int, buildid, sizeof(buildid_int));
	return DeeInt_NewUInt128(buildid_int);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_libname(DeeModuleObject *__restrict self) {
	DREF /*String*/ DeeObject *result;
	result = DeeModule_GetLibName(self, 0);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	return DeeRT_ErrUnboundAttrCStr(self, "__libname__");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
module_bound_libname(DeeModuleObject *__restrict self) {
	DREF /*String*/ DeeObject *result;
	result = DeeModule_GetLibName(self, 0);
	if (!ITER_ISOK(result))
		return Dee_BOUND_FROMITERNOK(result); /* Unbound or error */
	Dee_Decref(result);
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_directory(DeeModuleObject *__restrict self) {
	DREF DeeObject *result = DeeModule_GetDirectory(self);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_filename(DeeModuleObject *__restrict self) {
	DREF DeeObject *result = DeeModule_GetFileName(self);
	if (result == ITER_DONE)
		result = DeeRT_ErrUnboundAttrCStr(self, "__filename__");
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
module_bound_filename(DeeModuleObject *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->mo_absname != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_path(DeeModuleObject *__restrict self) {
	DREF DeeObject *result = DeeModule_GetFileName(self);
	if (result == ITER_DONE)
		result = DeeNone_NewRef();
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_relname(DeeModuleObject *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	unsigned int flags;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__relname__", params: """
	DeeObject *to:?X4?DModule?Dstring?DType?N=!N;
	bool libname = false;
	bool todir = false;
""", docStringPrefix: "module");]]]*/
#define module___relname___params "to:?X4?DModule?Dstring?DType?N=!N,libname=!f,todir=!f"
	struct {
		DeeObject *to;
		bool libname;
		bool todir;
	} args;
	args.to = Dee_None;
	args.libname = false;
	args.todir = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__to_libname_todir, "|obb:__relname__", &args))
		goto err;
/*[[[end]]]*/
	flags = DeeModule_RELNAME_F_NORMAL;
	if (args.libname)
		flags |= DeeModule_RELNAME_F_LIBNAM;
	if (args.todir)
		flags |= DeeModule_RELNAME_F_CTXDIR;
	result = DeeModule_GetRelName(self, args.to, flags);
	if unlikely(result == ITER_DONE)
		goto err_noname;
	return result;
err_noname:
	DeeError_Throwf(&DeeError_ValueError, "Cannot form relative name for module %k", self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_imports(DeeModuleObject *__restrict self) {
	return DeeRefVector_NewReadonly(self, self->mo_importc,
	                                (DeeObject **)self->mo_importv);
}

PRIVATE struct type_member tpconst module_members[] = {
	TYPE_MEMBER_FIELD_DOC("__absname__", STRUCT_CSTR, offsetof(DeeModuleObject, mo_absname),
	                      "The absolute name of this module (as can be passed to "
	                      /**/ "$import to access this module from any context)"),
	TYPE_MEMBER_FIELD_DOC("__haspath__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeModuleObject, mo_absname),
	                      "Deprecated alias for ${this.__filename__ is bound}"),
	TYPE_MEMBER_END
};

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_ViewExports(DeeModuleObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_LibNames(DeeModuleObject *__restrict self);

PRIVATE struct type_method tpconst module_methods[] = {
	TYPE_KWMETHOD_F("__relname__", &module_relname, METHOD_FNOREFESCAPE,
	                "(to:?X4?DModule?Dstring?DType?N=!N,libname=!f,todir=!f)->?Dstring\n"
	                "#tValueError{?#__filename__ is unbound, or @to is an empty string and @libname is false, or @this has no libnames}"
	                "Form the import name of @this module relative to @to. When @to is a string, it "
	                /**/ "should be the filename (like ?#__filename__) of a deemon script relative to "
	                /**/ "which the string should be generated. However, when @todir is $true and @to "
	                /**/ "is a string, then @to is the directory of the reference file instead of the "
	                /**/ "reference file itself\n"
	                "When @libname is true, try to return ${__libnames__.first} instead (s.a. ?#__libnames__)"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst module_getsets[] = {
	TYPE_GETTER_AB("__exports__", &DeeModule_ViewExports,
	               "->?M?X2?Dstring?Dint?O\n"
	               "Returns a modifiable mapping-like object containing @this "
	               "Module's global variables accessible by name (and enumerable)\n"
	               "Note that only existing exports can be modified, however no new symbols can be added:\n"
	               "${"
	               /**/ "import util;\n"
	               /**/ "print util.min;                /* function */\n"
	               /**/ "print util.__exports__[\"min\"]; /* function */\n"
	               /**/ "del util.min;\n"
	               /**/ "assert util.__exports__[\"min\"] !is bound;\n"
	               /**/ "util.__exports__[\"min\"] = 42;\n"
	               /**/ "print util.min;                /* 42 */"
	               "}"),
	TYPE_GETTER_AB("__imports__", &module_get_imports,
	               "->?S?DModule\n"
	               "Returns an immutable sequence of all other modules imported by this one"),
	TYPE_GETTER_AB("__globals__", &DeeModule_ViewGlobals,
	               "->?S?O\n"
	               "Similar to ?#__exports__, however global variables are addressed using their "
	               /**/ "internal index. Using this, anonymous global variables (such as property "
	               /**/ "callbacks) can be accessed and modified"),
	TYPE_GETTER_AB_F("__directory__", &module_get_directory, METHOD_FNOREFESCAPE,
	                 "->?S?Dstring\n"
	                 "Returns the names of all child-modules located "
	                 /**/ "within the directory specified by ?#__absname__\n"
	                 "These strings can be used as attribute names with "
	                 /**/ "?#{op:getattr} to retrieve the relevant module"),
	TYPE_GETTER_BOUND_F("__filename__", &module_get_filename, &module_bound_filename, METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#tUnboundAttribute{Module does not originate from the filesystem}"
	                    "Returns the actual filename (as opposed to the unique / "
	                    /**/ "directory-name returned by ?#__absname__) of this module"),
	TYPE_GETTER_AB_F("__path__", &module_get_path, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Deprecated alias for ${this.__filename__ is bound ? this.__filename__ : none}"),
	TYPE_GETTER_F("__name__", &module_get_name, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "'Simple' name of the module (that is: the last component of ?#__absname__)"),
	TYPE_GETTER_AB_F("__buildid__", &module_get_buildid, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "The module's #I{Build ID}, which is a unique, 128-bit "
	                 /**/ "identifier for this specific build of the module. "
	                 /**/ "This number may be assumed to change whenever "
	                 /**/ "something about the underlying structure of the "
	                 /**/ "module changes."),
	TYPE_GETTER_BOUND_F("__libname__", &module_get_libname, &module_bound_libname, METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#tUnboundAttribute{Module isn't located in a directory reachable from any ?#{c:path}}"
	                    "Returns the #Cfirst libname for this module. Note that the order of "
	                    /**/ "?#__libnames__ of a module is random. Same as ${this.__libnames__.first}"),
	TYPE_GETTER_AB_F("__libnames__", &DeeModule_LibNames, METHOD_FCONSTCALL,
	                 "->?S?Dstring\n"
	                 "Returns a sequence of all lib names assigned to this "
	                 /**/ "module. These come in no particular order"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst module_dee_getsets[] = {
	TYPE_GETTER_AB_F("__code__", &module_get_code, METHOD_FCONSTCALL | METHOD_FNOTHROW,
	                 "->?Ert:Code\n"
	                 "#tValueError{The Module hasn't been fully loaded}"
	                 "Returns the code object for the Module's root initializer"),
	TYPE_GETSET_END
};

#define module_dee_members module_addr_members
#define module_dex_members module_addr_members
PRIVATE struct type_member tpconst module_addr_members[] = {
	TYPE_MEMBER_FIELD("__minaddr__", STRUCT_UINTPTR_T | STRUCT_CONST, offsetof(DeeModuleObject, mo_minaddr)),
	TYPE_MEMBER_FIELD("__maxaddr__", STRUCT_UINTPTR_T | STRUCT_CONST, offsetof(DeeModuleObject, mo_maxaddr)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_class_getpath(DeeObject *__restrict UNUSED(self)) {
	return DeeModule_GetLibPath();
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_class_setpath(DeeObject *UNUSED(self),
                     DeeObject *value) {
	if (DeeObject_AssertTypeExact(value, &DeeTuple_Type))
		goto err;
	return DeeModule_SetLibPath(value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_class_gethome(DeeObject *__restrict UNUSED(self)) {
	return DeeExec_GetHome();
}

PRIVATE struct type_getset tpconst module_class_getsets[] = {
	TYPE_GETTER_AB("home", &module_class_gethome,
	               "->?Dstring\n"
	               "The deemon home path (usually the path where the deemon executable resides)"),
	TYPE_GETSET_AB("path", &module_class_getpath, NULL, &module_class_setpath,
	               "->?DTuple\n"
	               "A tuple of strings describing the search path for system libraries"),
	/* TODO: User-code access to "DeeModule_AddLibPath" */
	/* TODO: User-code access to "DeeModule_RemoveLibPath" */
	/* TODO: User-code access to "DeeModule_NextAbsTree" */
	/* TODO: User-code access to "DeeModule_NextAdrTree" */
	/* TODO: User-code access to "DeeModule_NextLibTree" */

#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Deprecated aliases to emulate the old `dexmodule' builtin type. */
	TYPE_GETSET_AB("search_path", &module_class_getpath, NULL, &module_class_setpath,
	               "->?DTuple\n"
	               "Deprecated alias for ?#path"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETSET_END
};

/* WARNING: This right here doesn't work in _hostasm code (because that doesn't produce frames) */
PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
module_import_with_frame_base(DeeObject *__restrict module_name) {
	DREF DeeModuleObject *result;
	struct Dee_code_frame *frame = DeeThread_Self()->t_exec;
	if (frame) {
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func->fo_code, &DeeCode_Type);
		ASSERT_OBJECT_TYPE(frame->cf_func->fo_code->co_module, &DeeModule_Type);
		result = DeeModule_Import(module_name, Dee_AsObject(frame->cf_func->fo_code->co_module), DeeModule_IMPORT_F_NORMAL);
	} else {
		result = DeeModule_Import(module_name, NULL, DeeModule_IMPORT_F_NORMAL);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
module_class_open(DeeObject *UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	/* This is pretty much the same as the builtin `import()' function.
	 * The only reason it exist is to be a deprecated alias for backwards
	 * compatibility with the old deemon. */
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("open", params: """
	DeeStringObject *name;
""", docStringPrefix: "module_class");]]]*/
#define module_class_open_params "name:?Dstring"
	struct {
		DeeStringObject *name;
	} args;
	DeeArg_Unpack1(err, argc, argv, "open", &args.name);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	return module_import_with_frame_base(Dee_AsObject(args.name));
err:
	return NULL;
}


PRIVATE struct type_method tpconst module_class_methods[] = {
	/* Deprecated aliases to emulate the old `dexmodule' builtin type. */
	TYPE_METHOD("open", &module_class_open,
	            "(" module_class_open_params ")->?DModule\n"
	            "Deprecated alias for ?D__import__ and the $import statement"),
	TYPE_METHOD_END
};


/* Unbind "self" from relevant trees.
 * Caller must ensure that `self->mo_absname != NULL' */
INTDEF NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self);

PRIVATE NONNULL((1)) void DCALL
module_common_destroy(DeeModuleObject *__restrict self) {
	Dee_weakref_support_fini(self);
	if (self->mo_absname) {
		module_unbind(self);
		Dee_Free(self->mo_absname);
	}
	Dee_XDecref(self->mo_dir);
}


/************************************************************************/
/* DeeModuleDir_Type -- /opt                                            */
/************************************************************************/
PRIVATE NONNULL((1)) void DCALL
module_dir_destroy(DeeModuleObject *__restrict self) {
	/* Assert always-true invariants for "DeeModuleDir_Type" */
	ASSERT(Dee_TYPE(self) == &DeeModuleDir_Type);
	ASSERT(self->mo_importc == 0);
//	ASSERT(self->mo_importv == NULL); /* Not alllocated */
	ASSERT(self->mo_globalc == 0);
	ASSERT(self->mo_bucketm == 0);
	ASSERT(self->mo_bucketv == empty_module_buckets);
	ASSERT(self->mo_init == Dee_MODULE_INIT_INITIALIZED);
	self = (DeeModuleObject *)DeeGC_Untrack(Dee_AsObject(self));
	module_common_destroy(self);
	DeeGCObject_Free(self);
}



/************************************************************************/
/* DeeModuleDee_Type -- /opt/script.dee                                 */
/************************************************************************/
PRIVATE NONNULL((1, 2)) void DCALL
module_dee_visit(DeeModuleObject *__restrict self,
                 Dee_visit_t proc, void *arg) {
	ASSERT(Dee_TYPE(self) == &DeeModuleDee_Type);
	DeeModule_LockRead(self);
	Dee_XVisitv(self->mo_globalv, self->mo_globalc);
	Dee_Visit(self->mo_moddata.mo_rootcode);
	Dee_Visitv(self->mo_importv, self->mo_importc);
	DeeModule_LockEndRead(self);
}

#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
/* Unbind from `module_byaddr_tree' */
INTDEF NONNULL((1)) void DCALL
module_dee_unbind(DeeModuleObject *__restrict self);
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */

PRIVATE NONNULL((1)) void DCALL
module_dee_destroy(DeeModuleObject *__restrict self) {
	/* Assert always-true invariants for "DeeModuleDee_Type" */
	ASSERT(Dee_TYPE(self) == &DeeModuleDee_Type);
	ASSERT(self->mo_init == Dee_MODULE_INIT_INITIALIZED ||
	       self->mo_init == Dee_MODULE_INIT_UNINITIALIZED);
	self = (DeeModuleObject *)DeeGC_Untrack(Dee_AsObject(self));

	/* Unbind from `module_byaddr_tree' */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	module_dee_unbind(self);
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */

	/* Destroy Dee-specific data */
	Dee_XDecrefv(self->mo_globalv, self->mo_globalc);
	Dee_Decref(self->mo_moddata.mo_rootcode);
	Dee_Decrefv(self->mo_importv, self->mo_importc);
	Dee_Free((void *)self->mo_importv);

	/* Destroy symbol table */
	if (self->mo_bucketv != empty_module_buckets) {
		uint16_t i;
		struct Dee_module_symbol *bucketv = self->mo_bucketv;
		for (i = 0; i <= self->mo_bucketm; ++i) {
			struct Dee_module_symbol *item = &bucketv[i];
			if (!Dee_MODULE_SYMBOL_GETNAMESTR(item))
				continue;
			if (item->ss_flags & Dee_MODSYM_FNAMEOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(Dee_MODULE_SYMBOL_GETNAMESTR(item), DeeStringObject, s_str));
			if (item->ss_flags & Dee_MODSYM_FDOCOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(item->ss_doc, DeeStringObject, s_str));
		}
		Dee_Free(bucketv);
	}

	/* Destroy common data */
	module_common_destroy(self);

	/* Free module object. Note that this may-or-may-not also free the `DeeMapFile'
	 * associated with the .dec image. Said image (and thus the backing storage of
	 * "self" and all other objects belonging to the module) will be freed once the
	 * last sub-heap-block apart of the image has been freed.
	 *
	 * Of course, that **could** be "self", but it might also be some random string
	 * constant that is still being referenced elsewhere.
	 *
	 * For more information on how this whole "last-free-destroys-the-region" thing
	 * works, take a look at file ../../../include/deemon/heap.h */
	DeeGCObject_Free(self);
}

PRIVATE NONNULL((1)) void DCALL
module_dee_clear(DeeModuleObject *__restrict self) {
	uint16_t i;
	DREF DeeCodeObject *code;
	DeeModule_LockWrite(self);
	for (i = self->mo_globalc; i--;) {
		/* Operate in reverse order, better mirrors the
		 * (likely) order in which stuff was initialized in. */
		DREF DeeObject *ob = self->mo_globalv[i];
		if (ob == NULL)
			continue;
		self->mo_globalv[i] = NULL;

		/* Temporarily unlock the module to immediately
		 * destroy a global variable when the priority
		 * level isn't encompassing _all_ objects, yet. */
		DeeModule_LockEndWrite(self);
		Dee_Decref(ob);
		DeeModule_LockWrite(self);
	}
	code = self->mo_moddata.mo_rootcode;
	self->mo_moddata.mo_rootcode = &DeeCode_Empty;
	Dee_Incref(&DeeCode_Empty);
	DeeModule_LockEndWrite(self);
	Dee_Decref_likely(code);
}

PRIVATE struct type_gc tpconst module_dee_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&module_dee_clear,
};


PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
module_dee_serialize(DeeModuleObject *__restrict self,
                     DeeSerial *__restrict writer) {
	union Dee_module_buildid const *build_id;
	uint16_t globalc = self->mo_globalc;
	size_t sizeof_module = offsetof(DeeModuleObject, mo_globalv) +
	                       globalc * sizeof(DREF DeeObject *);
	Dee_seraddr_t out_addr = DeeSerial_GCObject_Malloc(writer, sizeof_module, self);
	DeeModuleObject *out;
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, DeeModuleObject);
	out->mo_absname = NULL; /* Will be initialized by dex loader */
	memset(&out->mo_absnode, 0xcc, sizeof(out->mo_absnode));
	out->mo_libname.mle_name        = NULL;
	out->mo_libname.mle_dat.mle_mod = NULL; /* Will be initialized by dex loader */
	out->mo_libname.mle_next        = NULL;
	memset(&out->mo_libname.mle_node, 0xcc, sizeof(out->mo_libname.mle_node));
	out->mo_dir   = NULL; /* Will be initialized by dex loader */
	out->mo_init  = Dee_MODULE_INIT_UNINITIALIZED;
	build_id = DeeModule_GetBuildId(self);
	if unlikely(!build_id)
		goto err;
	out->mo_buildid = *build_id;
	out->mo_flags = atomic_read(&self->mo_flags);
	out->mo_flags &= ~(Dee_MODULE_FABSFILE | Dee_MODULE_FWAITINIT |
	                   Dee_MODULE_FABSRED | Dee_MODULE_FADRRED |
	                   _Dee_MODULE_FLIBALL | _Dee_MODULE_FCLEARED);
	out->mo_importc = self->mo_importc;
	out->mo_globalc = globalc;
	out->mo_bucketm = self->mo_bucketm;
	Dee_atomic_rwlock_init(&out->mo_lock);
#if !defined(CONFIG_NO_DEC) || !defined(CONFIG_NO_DEX)
	out->mo_minaddr = NULL;
	out->mo_maxaddr = NULL;
	memset(&out->mo_adrnode, 0xcc, sizeof(out->mo_adrnode));
#endif /* !CONFIG_NO_DEC || !CONFIG_NO_DEX */

	DeeModule_LockRead(self);
	out->mo_moddata.mo_rootcode = self->mo_moddata.mo_rootcode;
	Dee_Incref(out->mo_moddata.mo_rootcode);
	Dee_XMovrefv(out->mo_globalv, self->mo_globalv, globalc);
	DeeModule_LockEndRead(self);
	if (DeeSerial_InplacePutObject(writer, out_addr + offsetof(DeeModuleObject, mo_moddata.mo_rootcode))) {
		out = DeeSerial_Addr2Mem(writer, out_addr, DeeModuleObject);
		Dee_XDecrefv(out->mo_globalv, globalc);
		goto err;
	}
	if (DeeSerial_XInplacePutObjectv(writer, out_addr + offsetof(DeeModuleObject, mo_globalv), globalc))
		goto err;

	/* Duplicate "mo_bucketv" */
	if (self->mo_bucketv == empty_module_buckets) {
		if (DeeSerial_PutStaticDeemon(writer, out_addr + offsetof(DeeModuleObject, mo_bucketv), empty_module_buckets))
			goto err;
	} else {
		size_t i, count = self->mo_bucketm + 1;
		Dee_seraddr_t out__mo_bucketv;
		struct Dee_module_symbol *in__mo_bucketv = self->mo_bucketv;
		struct Dee_module_symbol *ou__mo_bucketv;
		out__mo_bucketv = DeeSerial_Malloc(writer, count * sizeof(struct Dee_module_symbol), in__mo_bucketv);
		if (!Dee_SERADDR_ISOK(out__mo_bucketv))
			goto err;
		if (DeeSerial_PutAddr(writer, out_addr + offsetof(DeeModuleObject, mo_bucketv), out__mo_bucketv))
			goto err;
		ou__mo_bucketv = DeeSerial_Addr2Mem(writer, out__mo_bucketv, struct Dee_module_symbol);
		memcpyc(ou__mo_bucketv, in__mo_bucketv, count, sizeof(struct Dee_module_symbol));
		for (i = 0; i < count; ++i, ++in__mo_bucketv) {
			Dee_seraddr_t out__mo_bucketv_i = out__mo_bucketv + i * sizeof(struct Dee_module_symbol);
			if (in__mo_bucketv->ss_name) {
				int status;
				if (in__mo_bucketv->ss_flags & Dee_MODSYM_FNAMEOBJ) {
					DeeStringObject *ob = container_of(in__mo_bucketv->ss_name, DeeStringObject, s_str);
					status = DeeSerial_PutObjectEx(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_name),
					                               ob, offsetof(DeeStringObject, s_str));
				} else {
					status = DeeSerial_PutPointer(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_name),
					                              in__mo_bucketv->ss_name);
				}
				if unlikely(status)
					goto err;
			}
			if (in__mo_bucketv->ss_doc) {
				int status;
				if (in__mo_bucketv->ss_flags & Dee_MODSYM_FDOCOBJ) {
					DeeStringObject *ob = container_of(in__mo_bucketv->ss_doc, DeeStringObject, s_str);
					status = DeeSerial_PutObjectEx(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_doc),
					                               ob, offsetof(DeeStringObject, s_str));
				} else {
					status = DeeSerial_PutPointer(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_doc),
					                              in__mo_bucketv->ss_doc);
				}
				if unlikely(status)
					goto err;
			}
		}
	}

	/* Duplicate "mo_importv" */
	if (self->mo_importv) {
		uint16_t count = self->mo_importc;
		Dee_seraddr_t out__mo_importv;
		DREF DeeModuleObject **ou__mo_importv;
		DREF DeeModuleObject *const *in__mo_importv = self->mo_importv;
		out__mo_importv = DeeSerial_Malloc(writer, count * sizeof(DREF DeeModuleObject *), (void *)in__mo_importv);
		if (!Dee_SERADDR_ISOK(out__mo_importv))
			goto err;
		if (DeeSerial_PutAddr(writer, out_addr + offsetof(DeeModuleObject, mo_importv), out__mo_importv))
			goto err;
		ou__mo_importv = DeeSerial_Addr2Mem(writer, out__mo_importv, DREF DeeModuleObject *);
		Dee_Movrefv(ou__mo_importv, in__mo_importv, count);
		if (DeeSerial_InplacePutObjectv(writer, out__mo_importv, count))
			goto err;
	}

	return out_addr;
err:
	return Dee_SERADDR_INVALID;
}



#ifndef CONFIG_NO_DEX
PRIVATE NONNULL((1, 2)) void DCALL
module_dex_visit(DeeModuleObject *__restrict self,
                 Dee_visit_t proc, void *arg) {
	ASSERT(Dee_TYPE(self) == &DeeModuleDex_Type);
	ASSERT(self->mo_importc == 0);
	ASSERT(self->mo_importv == NULL);
	DeeModule_LockRead(self);
	Dee_XVisitv(self->mo_globalv, self->mo_globalc);
	DeeModule_LockEndRead(self);
}

/* Called during finalization of the associated dex module */
INTDEF NONNULL((1)) void DCALL
Dee_module_dexdata_fini(struct Dee_module_dexdata *__restrict self);

/* Called during finalization of a dex module "mod"
 * -> used to finalize+remove all statically allocated type member
 *    caches whose address indicates that they are part of "mod". */
INTDEF NONNULL((1)) void DCALL
Dee_membercache_clearall_of_module(DeeModuleObject *__restrict mod);


PRIVATE NONNULL((1)) void DCALL
module_dex_destroy(DeeModuleObject *__restrict self) {
	struct Dee_module_dexdata *dexdata;
	/* Assert always-true invariants for "DeeModuleDex_Type" */
	ASSERT(Dee_TYPE(self) == &DeeModuleDex_Type);
	ASSERT(self->mo_init == Dee_MODULE_INIT_INITIALIZED ||
	       self->mo_init == Dee_MODULE_INIT_UNINITIALIZED);
	ASSERT(self->mo_importc == 0);
	ASSERT(self->mo_importv == NULL);
	self = (DeeModuleObject *)DeeGC_Untrack(Dee_AsObject(self));
	dexdata = self->mo_moddata.mo_dexdata;
	ASSERT(dexdata);
	ASSERT(dexdata->mdx_module == self);

	/* Invoke finalizer if dex was ever initialized. */
	if ((dexdata->mdx_fini != NULL) &&
	    (self->mo_init == Dee_MODULE_INIT_INITIALIZED))
		(*dexdata->mdx_fini)();

	/* NOTE: No need to unlink from "dex_byaddr_tree" -- that tree used to
	 *       hold a reference to our module, so the fact that we go here
	 *       has to mean that the module has already been removed from that
	 *       tree! */

	/* Cleanup string object references in the module's symbol table.
	 * Since "dex_add_symbol()" asserts that DEX modules can't pre-define
	 * symbols that use string objects as their names, such strings could
	 * only have been created lazily via `Dee_module_symbol_getnameobj()'
	 *
	 * The same obviously also goes for `Dee_module_symbol_getdocobj()'. */
	{
		struct Dee_module_symbol *buckets = self->mo_bucketv;
		if (buckets != empty_module_buckets) {
			uint16_t i;
			for (i = 0; i <= self->mo_bucketm; ++i) {
				struct Dee_module_symbol *sym = &buckets[i];
				if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ)
					Dee_Decref(container_of(sym->ss_name, DeeStringObject, s_str));
				if (sym->ss_flags & Dee_MODSYM_FDOCOBJ)
					Dee_Decref(container_of(sym->ss_doc, DeeStringObject, s_str));
			}
		}
	}

	/* Cleanup globals and dex data. */
	Dee_XDecrefv(self->mo_globalv, self->mo_globalc);
	Dee_module_dexdata_fini(dexdata);

	/* Destroy common data */
	module_common_destroy(self);

	/* Go through "membercache_list" and remove all entries
	 * with static addresses that map into this module.
	 *
	 * NOTE: This is an O(N) operation where "N" is the number
	 *       of member caches registered globally. However:
	 *       any solution that would make this faster would
	 *       most definitely end up making the process of
	 *       registering type member caches slower than the
	 *       O(1) it currently is:
	 *       >> membercache_list_lock_acquire();
	 *       >> LIST_INSERT_HEAD(&membercache_list, mc, mc_link);
	 *       >> membercache_list_lock_release(); */
	Dee_membercache_clearall_of_module(self);

	/* TODO: Somehow free all lazily allocated caches of static objects from "self"
	 * >> PRIVATE DEFINE_STRING(my_string, "Hi there!");
	 * >> ...
	 * >> PRIVATE void DCALL i_get_called_from_usercode(void) {
	 * >>     Dee_wchar_t *wstr = DeeString_AsWide(&my_string);
	 * >>     ...
	 * >> }
	 *
	 * In the above code, "wstr" will eventually leak, because "my_string" is
	 * statically allocated and is never freed. Dee modules don't have this
	 * problem because those use a smarter approach of emulating a proper heap
	 * across the whole module.
	 *
	 * One idea (that I DON'T want to go for) would be to define an API that
	 * allows one to allocate "static" memory that will be free'd when some
	 * associated module is unloaded:
	 * >> void *DCALL Dee_StaticMalloc(void *reference, size_t num_bytes);
	 *
	 * Here, "reference" would be a pointer to the "T *" that is being lazily
	 * allocated, and would be used with `DeeModule_OfPointer(reference)' to
	 * determine which dex module (if any) must free the heap block once that
	 * module gets unloaded.
	 *
	 * However: therein lies the problem: `DeeModule_OfPointer()' is O(log(N))
	 *          for the # of currently loaded dex modules, which is simply too
	 *          slow. The overhead added by this additional tracking *MUST*
	 *          remain O(1), since it's only needed for the case where the
	 *          user wants to unload dex modules, but not exit deemon entirely.
	 *          (Currently, that doesn't even work at all; see FIXME below),
	 *          but regular (hot) execution can't be made slower just to solve
	 *          a memory leak that only happens during special-case (cold)
	 *          execution.
	 *
	 * NOTES:
	 * - The only memory leaks affected by this are allocations that
	 *   currently get piped through `Dee_UntrackAlloc()'.
	 * - The reason why these allocations don't qualify as memory leaks is:
	 *   - the total sum of memory they can allocate is O(1), because...
	 *   - ... dex modules are never unloaded, so a cache that was allocated
	 *     once, will remain allocated until deemon exits
	 * - however:
	 *   - by working towards the goal of allowing dex modules to be unloaded
	 *     prior to deemon exiting, dex modules can be re-loaded an arbitrary
	 *     number of times,
	 *   - which means that these cache allocations can be repeated
	 *   - which means that the O(1) above becomes O(n), where "n" is the
	 *     number of times that some dex module with a lazily allocated
	 *     cache was loaded+unloaded
	 *   - thus turning the allocation into a real leak
	 */

#if 0
	/* FIXME: Work-around for preventing DEX modules being unloaded
	 *        while objects referring to statically defined types inside
	 *        still exist.
	 *        The proper way to fix this would be to use a GC-like mechanism
	 *        for this that delays the actual DEX unloading until no objects
	 *        exist that are apart of, or use types from the DEX.
	 *        Note that for this, we must also change the ruling that objects
	 *        pointing to other objects don't have to implement GC-visit if it
	 *        is known that none of those objects are ever GC objects. With this
	 *        change, _all_ objects have to implement GC-visit! (though we could
	 *        add a type-flag to indicate that pointed-to objects can never form
	 *        a ~conventional~ loop) */
	DeeSystem_DlClose(dexdata->mdx_handle);
#endif
}

PRIVATE NONNULL((1)) void DCALL
module_dex_clear(DeeModuleObject *__restrict self) {
	uint16_t i;
	struct Dee_module_dexdata *dexdata = self->mo_moddata.mo_dexdata;
	if (dexdata->mdx_clear)
		(*dexdata->mdx_clear)();
	DeeModule_LockWrite(self);
	for (i = self->mo_globalc; i--;) {
		/* Operate in reverse order, better mirrors the
		 * (likely) order in which stuff was initialized in. */
		DREF DeeObject *ob = self->mo_globalv[i];
		if (ob == NULL)
			continue;
		self->mo_globalv[i] = NULL;

		/* Temporarily unlock the module to immediately
		 * destroy a global variable when the priority
		 * level isn't encompassing _all_ objects, yet. */
		DeeModule_LockEndWrite(self);
		Dee_Decref(ob);
		DeeModule_LockWrite(self);
	}
	DeeModule_LockEndWrite(self);
}

PRIVATE struct type_gc tpconst module_dex_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&module_dex_clear,
};
#endif /* !CONFIG_NO_DEX */

#ifdef CONFIG_NO_DEX
#define module_doc_kind_listing "?Ert:ModuleDee and ?Ert:ModuleDir"
#else /* CONFIG_NO_DEX */
#define module_doc_kind_listing "?Ert:ModuleDee, ?Ert:ModuleDir and ?Ert:ModuleDex"
#endif /* !CONFIG_NO_DEX */

PUBLIC DeeTypeObject DeeModule_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Module),
	/* .tp_doc      = */ DOC("Abstract base-class for " module_doc_kind_listing "\n"
	                         "\n"
	                         "str->\n"
	                         "Returns the name of the module"),
	/* Same as with DeeType_Type: Modules may not be immutable, but still shouldn't be replicated during deepcopy */
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT | TP_FVARIABLE | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL, /* !!! Not constructible !!! */
			/* tp_copy_ctor:   */ NULL, /* !!! Not constructible !!! */
			/* tp_any_ctor:    */ NULL, /* !!! Not constructible !!! */
			/* tp_any_ctor_kw: */ NULL, /* !!! Not constructible !!! */
			/* tp_serialize:   */ NULL, /* !!! Not constructible !!! */
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&module_str,
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&module_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeObject_GenericCmpByAddr,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ module_methods,
	/* .tp_getsets       = */ module_getsets,
	/* .tp_members       = */ module_members,
	/* .tp_class_methods = */ module_class_methods,
	/* .tp_class_getsets = */ module_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

PUBLIC DeeTypeObject DeeModuleDir_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleDir",
	/* .tp_doc      = */ NULL,
	/* Same as with DeeType_Type: Modules may not be immutable, but still shouldn't be replicated during deepcopy */
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL, /* Not serializable */
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_new         = */ NULL,
		/* .tp_new_kw      = */ NULL,
		/* .tp_new_copy    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&module_dir_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&module_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&module_printrepr),
	},
	/* .tp_visit         = */ NULL, // (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_dir_visit, /* Wouldn't have anything to visit! */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__FEC430738D08383C),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ &module_attr, /* TODO: Custom operators (that only look at directory files) */
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

PUBLIC DeeTypeObject DeeModuleDee_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleDee",
	/* .tp_doc      = */ NULL,
	/* Same as with DeeType_Type: Modules may not be immutable, but still shouldn't be replicated during deepcopy */
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &module_dee_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_new         = */ NULL,
		/* .tp_new_kw      = */ NULL,
		/* .tp_new_copy    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&module_dee_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&module_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&module_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_dee_visit,
	/* .tp_gc            = */ &module_dee_gc,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__FEC430738D08383C),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ module_dee_getsets,
	/* .tp_members       = */ module_dee_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#ifndef CONFIG_NO_DEX
PUBLIC DeeTypeObject DeeModuleDex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleDex",
	/* .tp_doc      = */ NULL,
	/* Same as with DeeType_Type: Modules may not be immutable, but still shouldn't be replicated during deepcopy */
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL, /* !!! Not constructible !!! */
			/* tp_copy_ctor:   */ NULL, /* !!! Not constructible !!! */
			/* tp_any_ctor:    */ NULL, /* !!! Not constructible !!! */
			/* tp_any_ctor_kw: */ NULL, /* !!! Not constructible !!! */
			/* tp_serialize:   */ NULL, /* Not serializable */
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_new         = */ NULL,
		/* .tp_new_kw      = */ NULL,
		/* .tp_new_copy    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&module_dex_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&module_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&module_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_dex_visit,
	/* .tp_gc            = */ &module_dex_gc,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__FEC430738D08383C),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ module_dex_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};
#endif /* !CONFIG_NO_DEX */


struct Dee_empty_module_struct {
	/* Even though never tracked, static modules still need the GC header for visiting. */
	struct Dee_gc_head               m_head;
	Dee_MODULE_STRUCT_EX(/**/, /**/) m_module;
};

#undef DeeModule_Empty
INTERN struct Dee_empty_module_struct DeeModule_Empty = {
	{ _Dee_GC_HEAD_UNTRACKED_INIT }, {
		OBJECT_HEAD_INIT(&DeeModuleDee_Type),
		/* .mo_absname = */ NULL,
		/* .mo_absnode = */ { NULL, NULL, NULL },
		/* .mo_libname = */ {
			/* .mle_name = */ NULL,
			/* .mle_dat  = */ { (DeeModuleObject *)&DeeModule_Empty.m_module },
			/* .mle_node = */ { NULL, NULL, NULL },
			/* .mle_next = */ NULL
		},
		/* .mo_dir     = */ (DeeTupleObject *)Dee_EmptyTuple,
		/* .mo_init    = */ Dee_MODULE_INIT_INITIALIZED,
		/* .mo_buildid = */ { {0} },
		/* .mo_flags   = */ Dee_MODULE_FNORMAL | Dee_MODULE_FHASBUILDID,
		/* .mo_importc = */ 0,
		/* .mo_globalc = */ 0,
		/* .mo_bucketm = */ 0,
		/* .mo_bucketv = */ empty_module_buckets,
		_Dee_MODULE_INIT_mo_lock
		Dee_WEAKREF_SUPPORT_INIT,
		/* .mo_moddata = */ Dee_MODULE_MODDATA_INIT_CODE(&DeeCode_Empty),
		/* .mo_importv = */ NULL,
		/* .mo_minaddr = */ NULL,
		/* .mo_maxaddr = */ NULL,
		/* .mo_adrnode = */ { NULL, NULL, NULL },
	}
};

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_C */
