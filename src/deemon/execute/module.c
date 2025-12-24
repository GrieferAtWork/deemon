/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_EXECUTE_MODULE_C
#define GUARD_DEEMON_EXECUTE_MODULE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/format.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/sched/yield.h> /* SCHED_YIELD */
#include <hybrid/sequence/list.h>

#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif /* !CONFIG_NO_DEX */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

INTERN WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
module_symbol_getnameobj(struct module_symbol *__restrict self) {
	DREF DeeStringObject *result;
	char const *name_str;
	uint16_t flags;
again:
	name_str = self->ss_name;
	atomic_thread_fence(Dee_ATOMIC_ACQUIRE);
	flags = self->ss_flags;
	if (flags & MODSYM_FNAMEOBJ) {
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
		atomic_or(&self->ss_flags, MODSYM_FNAMEOBJ);
		Dee_Incref(result);
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
module_symbol_getdocobj(struct module_symbol *__restrict self) {
	DREF DeeStringObject *result;
	char const *doc_str;
	uint16_t flags;
again:
	doc_str = self->ss_doc;
	ASSERT(doc_str != NULL);
	atomic_thread_fence(Dee_ATOMIC_ACQUIRE);
	flags = self->ss_flags;
	if (flags & MODSYM_FDOCOBJ) {
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
		atomic_or(&self->ss_flags, MODSYM_FDOCOBJ);
		Dee_Incref(result);
	}
done:
	return result;
}




#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_DoInitializeImports(DeeModuleObject *__restrict me) {
	uint16_t i;
	for (i = 0; i < me->mo_importc; ++i) {
		DeeModuleObject *imp = me->mo_importv[i];
		int status = DeeModule_Initialize((DeeObject *)imp);
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
	if unlikely(DeeModule_DoInitializeImports(me))
		goto err;
	rootfunc = (DREF DeeFunctionObject *)DeeModule_GetRootFunction((DeeObject *)me);
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
DeeModule_Initialize(/*Module*/ DeeObject *__restrict self) {
	int init_status;
	DeeModuleObject *me = (DeeModuleObject *)self;
	DeeThreadObject *caller = DeeThread_Self();
	DeeThreadObject *status;
again:
	status = Dee_atomic_cmpxch_val(&me->mo_init, Dee_MODULE_INIT_UNINITIALIZED, caller);
	if (status == Dee_MODULE_INIT_INITIALIZED) {
		return 0; /* Initialization already complete */
	} else if (status == caller) {
		return 1; /* You're already doing the init right now (nested/self dependency case) */
	} else if (status != Dee_MODULE_INIT_UNINITIALIZED) {
		/* Some other thread is already doing an initialization */
		atomic_or(&me->mo_flags, Dee_MODULE_FWAITINIT);
		if (DeeFutex_WaitPtr(&me->mo_init, (uintptr_t)status))
			goto err;
		goto again;
	}

	/* Initialization started */
	init_status = DeeModule_DoInitialize(me);

	/* Propagate initialization status */
	if likely(init_status == 0) {
		atomic_write(&me->mo_init, Dee_MODULE_INIT_INITIALIZED);
	} else {
		atomic_write(&me->mo_init, Dee_MODULE_INIT_UNINITIALIZED);
	}
	if (atomic_fetchand(&me->mo_flags, ~Dee_MODULE_FWAITINIT))
		DeeFutex_WakeAll(&me->mo_init);

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
PUBLIC WUNUSED NONNULL((1)) unsigned int DCALL
DeeModule_SetInitialized(/*Module*/ DeeObject *__restrict self) {
	DeeModuleObject *me = (DeeModuleObject *)self;
	DeeThreadObject *status;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeModuleDee_Type);
	status = Dee_atomic_cmpxch_val(&me->mo_init,
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
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF /*Code*/ DeeObject *DCALL
DeeModule_GetRootCode(/*Module*/ DeeObject *__restrict self) {
	DREF DeeCodeObject *result;
	DeeModuleObject *me = (DeeModuleObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeModuleDee_Type);
	DeeModule_LockRead(me);
	result = me->mo_moddata.mo_rootcode;
	Dee_Incref(result);
	DeeModule_LockEndRead(me);
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF /*Function*/ DeeObject *DCALL
DeeModule_GetRootFunction(/*Module*/ DeeObject *__restrict self) {
	DREF DeeCodeObject *rootcode;
	DREF DeeFunctionObject *rootfunc;
	rootcode = (DREF DeeCodeObject *)DeeModule_GetRootCode(self);
	ASSERT_OBJECT_TYPE_EXACT(rootcode, &DeeCode_Type);
	rootfunc = (DREF DeeFunctionObject *)DeeFunction_NewNoRefs((DeeObject *)rootcode);
	Dee_Decref_unlikely(rootcode);
	return (DREF DeeObject *)rootfunc;
}


PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) uint64_t DCALL
DeeModule_GetCTime_uncached(DeeModuleObject *__restrict self) {
	DeeTypeObject *tp = Dee_TYPE(self);
	if (self == &DeeModule_Deemon)
		return DeeExec_GetTimestamp();

	/* TODO: special handling for dex modules (which should embed a build timestamp) */
	/* TODO: fallback handling: use last-modified timestamp of module filename */
	(void)tp;
	return 0;
}

/* Returns the compile-time of a given module (in microseconds
 * since 01-01-1970), or (uint64_t)-1 if an error occurred.
 * Use this function instead of directly reading `self->mo_ctime',
 * as this function will lazily initialize the timestamp in cases
 * where it may not be loaded, yet. */
PUBLIC WUNUSED NONNULL((1)) uint64_t DCALL
DeeModule_GetCTime(/*Module*/ DeeObject *__restrict self) {
	uint64_t result;
	DeeModuleObject *me = (DeeModuleObject *)self;
	uint16_t flags = atomic_read(&me->mo_flags);
	if (flags & Dee_MODULE_FHASCTIME) {
		COMPILER_READ_BARRIER();
		return me->mo_ctime;
	}
	result = DeeModule_GetCTime_uncached(me);
	me->mo_ctime = result;
	COMPILER_WRITE_BARRIER();
	atomic_or(&me->mo_flags, Dee_MODULE_FHASCTIME);
	return result;
}

#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

/* Initialize all modules imported by the given one.
 * @throws: Error.RuntimeError: The module has not been loaded yet. (aka. no source code was assigned)
 * @return: -1: An error occurred during initialization.
 * @return:  0: All modules imported by the given one are now initialized. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_InitImports(/*Module*/ DeeObject *__restrict self);


/* Create a new function object for the root code-object of a given module.
 * NOTE: This function will also ensure that all modules
 *       imported by this one have been fully initialized.
 * @param: set_initialized: When true, also set the `Dee_MODULE_FDIDINIT' flag if
 *                          it, or `Dee_MODULE_FINITIALIZING' hasn't been set already
 * @return: * : A callable object which, when invoked, will execute the module's root code,
 *              while passing any arguments given to it to the module's root where they
 *              are available as `...' (3 dots using in an expression)
 *              my_module.dee:
 *              >> print [...];  // [10, 20, 30]
 *              DeeObject_Callf(DeeModule_GetRoot(my_module, true), "ddd", 10, 20, 30);
 * @return: NULL: Failed to create a function object for the module's root code object. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_GetRoot(DeeObject *__restrict self,
                  bool set_initialized) {
	DREF DeeFunctionObject *result;
	DREF DeeCodeObject *code;
	DeeModuleObject *me = (DeeModuleObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);

	/* Check if this module has been loaded. */
	if unlikely(DeeModule_InitImports(self))
		goto err;

	/* Load the code object of the module initializer. */
	DeeModule_LockRead(me);
	code = me->mo_root;
	if unlikely(!code)
		code = &DeeCode_Empty;
	Dee_Incref(code);
	DeeModule_LockEndRead(me);

	/* Wrap the module's code object in a Function. */
	ASSERT(code->co_refc == 0);
	if likely(code->co_refstaticc == 0) {
		result = (DREF DeeFunctionObject *)DeeGCObject_Malloc(offsetof(DeeFunctionObject, fo_refv));
	} else {
		result = (DREF DeeFunctionObject *)DeeGCObject_Callocc(offsetof(DeeFunctionObject, fo_refv),
		                                                       code->co_refstaticc, sizeof(DREF DeeObject *));
	}
	if unlikely(!result)
		goto err;
	result->fo_code = code; /* Inherit reference */
	DeeObject_Init(result, &DeeFunction_Type);
#ifdef CONFIG_HAVE_CODE_METRICS
	atomic_inc(&result->fo_code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_function_init(&result->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	Dee_atomic_rwlock_init(&result->fo_reflock);
	result = (DREF DeeFunctionObject *)DeeGC_Track((DeeObject *)result);
	if (set_initialized) {
		uint16_t flags;
		/* Try to set the `Dee_MODULE_FDIDINIT' flag */
		do {
			flags = atomic_read(&me->mo_flags);
			if (flags & Dee_MODULE_FINITIALIZING)
				break; /* Don't interfere with an on-going initialization. */
		} while (!atomic_cmpxch_weak(&me->mo_flags, flags, flags | Dee_MODULE_FDIDINIT));
	}
	return (DREF DeeObject *)result;
err:
	return NULL;
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


INTERN struct module_symbol empty_module_buckets[] = {
	{ NULL, 0, 0 }
};



PUBLIC WUNUSED NONNULL((1)) struct module_symbol *DCALL
DeeModule_GetSymbolID(DeeModuleObject const *__restrict self, uint16_t gid) {
	struct module_symbol *iter, *end;
	struct module_symbol *result = NULL;

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	/* Check if the module has actually been loaded yet.
	 * This needs to be done to prevent a race condition
	 * when reading the bucket fields below, as they only
	 * become immutable once this flag has been set. */
	if unlikely(!(self->mo_flags & Dee_MODULE_FDIDLOAD))
		return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	end = (iter = self->mo_bucketv) + (self->mo_bucketm + 1);
	for (; iter < end; ++iter) {
		if (!MODULE_SYMBOL_GETNAMESTR(iter))
			continue; /* Skip empty entries. */
		if (Dee_module_symbol_getindex(iter) == gid) {
			result = iter;

			/* If it's a symbol, we still stored it's name as
			 * the result value, meaning we won't return NULL
			 * but still keep on searching for another symbol
			 * that isn't an alias. */
			if (!(iter->ss_flags & MODSYM_FALIAS))
				break;
		}
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
DeeModule_GetSymbol(DeeModuleObject const *__restrict self,
                    /*String*/ DeeObject *__restrict name) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = DeeString_Hash(name);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
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

PUBLIC WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
DeeModule_GetSymbolStringHash(DeeModuleObject const *__restrict self,
                              char const *__restrict attr_name,
                              Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) != 0)
			continue;
		return item;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
DeeModule_GetSymbolStringLenHash(DeeModuleObject const *__restrict self,
                                 char const *__restrict attr_name,
                                 size_t attrlen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) != 0)
			continue;
		return item;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrSymbol(DeeModuleObject *__restrict self,
                        struct module_symbol const *__restrict symbol) {
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
		DREF DeeObject *result;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(symbol) < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[Dee_module_symbol_getindex(symbol)];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result)
			err_unbound_global(self, Dee_module_symbol_getindex(symbol));
		return result;
	}

	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(symbol) + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback) {
			err_module_cannot_read_property_string(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			return NULL;
		}

		/* Invoke the property callback. */
		return DeeObject_CallInherited(callback, 0, NULL);
	}

	/* External symbol. */
	ASSERT(symbol->ss_impid < self->mo_importc);
	self = self->mo_importv[symbol->ss_impid];
	goto read_symbol;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrSymbol(DeeModuleObject *__restrict self,
                          struct module_symbol const *__restrict symbol) {
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
		DeeObject *value;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(symbol) < self->mo_globalc);
		DeeModule_LockRead(self);
		value = self->mo_globalv[Dee_module_symbol_getindex(symbol)];
		DeeModule_LockEndRead(self);
		return Dee_BOUND_FROMBOOL(value);
	}

	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback, *callback_result;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(symbol) + MODULE_PROPERTY_GET];
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
	ASSERT(symbol->ss_impid < self->mo_importc);
	self = self->mo_importv[symbol->ss_impid];
	goto read_symbol;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrSymbol(DeeModuleObject *__restrict self,
                        struct module_symbol const *__restrict symbol) {
	DREF DeeObject *old_value;
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if unlikely(symbol->ss_flags & (MODSYM_FREADONLY | MODSYM_FEXTERN | MODSYM_FPROPERTY)) {
		if (symbol->ss_flags & MODSYM_FREADONLY)
			return err_module_readonly_global_string(self, MODULE_SYMBOL_GETNAMESTR(symbol));
		if (symbol->ss_flags & MODSYM_FPROPERTY) {
			DREF DeeObject *callback, *temp;
			DeeModule_LockRead(self);
			ASSERT(Dee_module_symbol_getindex(symbol) + MODULE_PROPERTY_DEL < self->mo_globalc);
			callback = self->mo_globalv[Dee_module_symbol_getindex(symbol) + MODULE_PROPERTY_DEL];
			Dee_XIncref(callback);
			DeeModule_LockEndRead(self);
			if unlikely(!callback)
				return err_module_cannot_delete_property_string(self, MODULE_SYMBOL_GETNAMESTR(symbol));

			/* Invoke the property callback. */
			temp = DeeObject_CallInherited(callback, 0, NULL);
			Dee_XDecref(temp);
			return temp ? 0 : -1;
		}

		/* External symbol. */
		ASSERT(symbol->ss_impid < self->mo_importc);
		self = self->mo_importv[symbol->ss_impid];
	}
	ASSERT(Dee_module_symbol_getindex(symbol) < self->mo_globalc);
	DeeModule_LockWrite(self);
	old_value = self->mo_globalv[Dee_module_symbol_getindex(symbol)];
	self->mo_globalv[Dee_module_symbol_getindex(symbol)] = NULL;
	DeeModule_LockEndWrite(self);
	Dee_XDecref(old_value);
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeModule_SetAttrSymbol(DeeModuleObject *__restrict self,
                        struct module_symbol const *__restrict symbol,
                        DeeObject *__restrict value) {
	DREF DeeObject *temp;
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if unlikely(symbol->ss_flags & (MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FEXTERN)) {
		if unlikely(symbol->ss_flags & MODSYM_FEXTERN) {
			ASSERT(symbol->ss_impid < self->mo_importc);
			self = self->mo_importv[symbol->ss_impid];
		}
		if (symbol->ss_flags & MODSYM_FREADONLY) {
			if (symbol->ss_flags & MODSYM_FPROPERTY)
				goto err_is_readonly;
			ASSERT(Dee_module_symbol_getindex(symbol) < self->mo_globalc);
			DeeModule_LockWrite(self);
			/* Make sure not to allow write-access to global variables that
			 * have already been assigned, but are marked as read-only. */
			if unlikely(self->mo_globalv[Dee_module_symbol_getindex(symbol)] != NULL) {
				DeeModule_LockEndWrite(self);
err_is_readonly:
				return err_module_readonly_global_string(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			}
			Dee_Incref(value);
			self->mo_globalv[Dee_module_symbol_getindex(symbol)] = value;
			DeeModule_LockEndWrite(self);
			return 0;
		}
		if (symbol->ss_flags & MODSYM_FPROPERTY) {
			DREF DeeObject *callback;
			DeeModule_LockWrite(self);
			callback = self->mo_globalv[Dee_module_symbol_getindex(symbol) + MODULE_PROPERTY_SET];
			Dee_XIncref(callback);
			DeeModule_LockEndWrite(self);
			if unlikely(!callback)
				return err_module_cannot_write_property_string(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			temp = DeeObject_CallInherited(callback, 1, (DeeObject **)&value);
			Dee_XDecref(temp);
			return temp ? 0 : -1;
		}
		/* External symbol. */
		ASSERT(symbol->ss_impid < self->mo_importc);
		self = self->mo_importv[symbol->ss_impid];
	}
	ASSERT(Dee_module_symbol_getindex(symbol) < self->mo_globalc);
	Dee_Incref(value);
	DeeModule_LockWrite(self);
	temp = self->mo_globalv[Dee_module_symbol_getindex(symbol)];
	self->mo_globalv[Dee_module_symbol_getindex(symbol)] = value;
	DeeModule_LockEndWrite(self);
	Dee_XDecref(temp);
	return 0;
}


#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifdef DEE_SYSTEM_FS_ICASE
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
#else /* DEE_SYSTEM_FS_ICASE */
#define FS_strcmp  strcmp
#define FS_strcmpz strcmpz
#ifndef CONFIG_HAVE_strcmpz
#define CONFIG_HAVE_strcmpz
#undef strcmpz
#define strcmpz dee_strcmpz
DeeSystem_DEFINE_strcmpz(dee_strcmpz)
#endif /* !CONFIG_HAVE_strcmpz */
#endif /* !DEE_SYSTEM_FS_ICASE */



#define DeeModule_DIRECTORY_ATTR_NO  ((DeeStringObject *)NULL)
#define DeeModule_DIRECTORY_ATTR_ERR ((DeeStringObject *)ITER_DONE)

/* @return: * :                           Yes
 * @return: DeeModule_DIRECTORY_ATTR_NO:  No
 * @return: DeeModule_DIRECTORY_ATTR_ERR: Error */
PRIVATE WUNUSED NONNULL((1)) DeeStringObject *DCALL
DeeModule_FindDirectoryAttrString(DeeModuleObject *__restrict self,
                                  char const *__restrict attr_name) {
	size_t lo, hi;
	struct Dee_module_directory *dir = DeeModule_GetDirectory(self);
	if unlikely(!dir)
		goto err;
	lo = 0;
	hi = dir->md_count;
	while (lo < hi) {
		int diff;
		size_t mid = (lo + hi) / 2;
		DeeStringObject *name = dir->md_files[mid];
		char const *name_utf8 = DeeString_AsUtf8((DeeObject *)name);
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
	struct Dee_module_directory *dir = DeeModule_GetDirectory(self);
	if unlikely(!dir)
		goto err;
	lo = 0;
	hi = dir->md_count;
	while (lo < hi) {
		int diff;
		size_t mid = (lo + hi) / 2;
		DeeStringObject *name = dir->md_files[mid];
		char const *name_utf8 = DeeString_AsUtf8((DeeObject *)name);
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


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return DeeModule_ImportEx(attr_name, strlen(attr_name),
		                          self->mo_absname, strlen(self->mo_absname),
		                          DeeModule_IMPORT_F_NORMAL |
		                          DeeModule_IMPORT_F_CTXDIR,
		                          NULL);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringHash((DeeObject *)self, attr_name, hash);
	if (result != ITER_DONE)
		return result;
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
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return DeeModule_ImportEx(attr_name, attrlen,
		                          self->mo_absname, strlen(self->mo_absname),
		                          DeeModule_IMPORT_F_NORMAL |
		                          DeeModule_IMPORT_F_CTXDIR,
		                          NULL);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
	if (result != ITER_DONE)
		return result;
	err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_GET);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringHash(DeeModuleObject *__restrict self,
                              char const *__restrict attr_name,
                              Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_BOUND_YES;
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericBoundAttrStringHash((DeeObject *)self, attr_name, hash);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringLenHash(DeeModuleObject *__restrict self,
                                 char const *__restrict attr_name,
                                 size_t attrlen, Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if unlikely(dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_BOUND_YES;
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericBoundAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return Dee_HAS_YES;
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_HAS_YES;
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericHasAttrStringHash((DeeObject *)self, attr_name, hash);
err:
	return Dee_HAS_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return Dee_HAS_YES;
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			goto err;
		return Dee_HAS_YES;
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericHasAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
err:
	return Dee_HAS_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash) {
	int error;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_DEL);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericDelAttrStringHash((DeeObject *)self, attr_name, hash);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	int error;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_DEL);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericDelAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeModule_SetAttrStringHash(DeeModuleObject *self,
                            char const *__restrict attr_name,
                            Dee_hash_t hash, DeeObject *value) {
	int error;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrString(self, attr_name);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_SET);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericSetAttrStringHash((DeeObject *)self, attr_name, hash, value);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeModule_SetAttrStringLenHash(DeeModuleObject *self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash,
                               DeeObject *value) {
	int error;
	DeeStringObject *dir_status;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Check for a directory element */
	dir_status = DeeModule_FindDirectoryAttrStringLen(self, attr_name, attrlen);
	if (dir_status != DeeModule_DIRECTORY_ATTR_NO) {
		if (dir_status == DeeModule_DIRECTORY_ATTR_ERR)
			return -1;
		return DeeRT_ErrRestrictedAttr(self, dir_status, DeeRT_ATTRIBUTE_ACCESS_SET);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericSetAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash, value);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_SET);
}

#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_getattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringHash((DeeObject *)self, attr_name, hash);
	if (result != ITER_DONE)
		return result;
	err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_GET);
	return NULL;
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_getattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
	if (result != ITER_DONE)
		return result;
	err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_GET);
	return NULL;
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
module_boundattr_impl(DeeModuleObject *__restrict self,
                      char const *__restrict attr_name, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericBoundAttrStringHash((DeeObject *)self, attr_name, hash);
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
module_boundattr_len_impl(DeeModuleObject *__restrict self,
                          char const *__restrict attr_name,
                          size_t attrlen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericBoundAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
}

LOCAL WUNUSED NONNULL((1, 2)) bool DCALL
module_hasattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return true;
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericHasAttrStringHash((DeeObject *)self, attr_name, hash);
}

LOCAL WUNUSED NONNULL((1, 2)) bool DCALL
module_hasattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return true;
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericHasAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
module_delattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name,
                    Dee_hash_t hash) {
	int error;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericDelAttrStringHash((DeeObject *)self, attr_name, hash);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_DEL);
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
module_delattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, Dee_hash_t hash) {
	int error;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericDelAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_DEL);
}

LOCAL WUNUSED NONNULL((1, 2, 4)) int DCALL
module_setattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, Dee_hash_t hash,
                    DeeObject *__restrict value) {
	int error;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericSetAttrStringHash((DeeObject *)self, attr_name, hash, value);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string(self, attr_name, ATTR_ACCESS_SET);
}

LOCAL int DCALL
module_setattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, Dee_hash_t hash,
                        DeeObject *__restrict value) {
	int error;
	Dee_hash_t i, perturb;
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (bcmpc(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen, sizeof(char)) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}

	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericSetAttrStringLenHash((DeeObject *)self, attr_name, attrlen, hash, value);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_string_len(self, attr_name, attrlen, ATTR_ACCESS_SET);
}


#ifndef CONFIG_NO_THREADS
INTDEF WUNUSED NONNULL((1)) int DCALL interactivemodule_lockread(DeeModuleObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL interactivemodule_lockwrite(DeeModuleObject *__restrict self);
INTDEF NONNULL((1)) void DCALL interactivemodule_lockendread(DeeModuleObject *__restrict self);
INTDEF NONNULL((1)) void DCALL interactivemodule_lockendwrite(DeeModuleObject *__restrict self);
#else /* !CONFIG_NO_THREADS */
#define interactivemodule_lockread(self)     0
#define interactivemodule_lockwrite(self)    0
#define interactivemodule_lockendread(self)  (void)0
#define interactivemodule_lockendwrite(self) (void)0
#endif /* CONFIG_NO_THREADS */


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			DREF DeeObject *result;
			if unlikely(interactivemodule_lockread(self))
				return NULL;
			result = module_getattr_impl(self, attr_name, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		err_module_not_loaded_attr_string(self, attr_name, ATTR_ACCESS_GET);
		return NULL;
	}
	return module_getattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			DREF DeeObject *result;
			if unlikely(interactivemodule_lockread(self))
				return NULL;
			result = module_getattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		err_module_not_loaded_attr_string_len(self, attr_name, attrlen, ATTR_ACCESS_GET);
		return NULL;
	}
	return module_getattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringHash(DeeModuleObject *__restrict self,
                              char const *__restrict attr_name, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			if unlikely(interactivemodule_lockread(self))
				return Dee_BOUND_ERR;
			result = module_boundattr_impl(self, attr_name, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return Dee_BOUND_MISSING;
	}
	return module_boundattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringLenHash(DeeModuleObject *__restrict self,
                                 char const *__restrict attr_name,
                                 size_t attrlen, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			if unlikely(interactivemodule_lockread(self))
				return Dee_BOUND_ERR;
			result = module_boundattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return Dee_BOUND_MISSING;
	}
	return module_boundattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			bool result;
			if (interactivemodule_lockread(self))
				return -1;
			result = module_hasattr_impl(self, attr_name, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return 0;
	}
	return module_hasattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_HasAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			bool result;
			if (interactivemodule_lockread(self))
				return -1;
			result = module_hasattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return false;
	}
	return module_hasattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringHash(DeeModuleObject *__restrict self,
                            char const *__restrict attr_name, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			result = interactivemodule_lockwrite(self);
			if (result == 0) {
				result = module_delattr_impl(self, attr_name, hash);
				interactivemodule_lockendwrite(self);
			}
			return result;
		}
		return err_module_not_loaded_attr_string(self, attr_name, ATTR_ACCESS_DEL);
	}
	return module_delattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringLenHash(DeeModuleObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			result = interactivemodule_lockwrite(self);
			if (result == 0) {
				result = module_delattr_len_impl(self, attr_name, attrlen, hash);
				interactivemodule_lockendwrite(self);
			}
			return result;
		}
		return err_module_not_loaded_attr_string_len(self, attr_name, attrlen, ATTR_ACCESS_DEL);
	}
	return module_delattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeModule_SetAttrStringHash(DeeModuleObject *self,
                            char const *__restrict attr_name, Dee_hash_t hash,
                            DeeObject *value) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			result = interactivemodule_lockwrite(self);
			if (result == 0) {
				result = module_setattr_impl(self, attr_name, hash, value);
				interactivemodule_lockendwrite(self);
			}
			return result;
		}
		return err_module_not_loaded_attr_string(self, attr_name, ATTR_ACCESS_SET);
	}
	return module_setattr_impl(self, attr_name, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeModule_SetAttrStringLenHash(DeeModuleObject *self,
                               char const *__restrict attr_name,
                               size_t attrlen, Dee_hash_t hash,
                               DeeObject *value) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			result = interactivemodule_lockwrite(self);
			if (result == 0) {
				result = module_setattr_len_impl(self, attr_name, attrlen, hash, value);
				interactivemodule_lockendwrite(self);
			}
			return result;
		}
		return err_module_not_loaded_attr_string_len(self, attr_name, attrlen, ATTR_ACCESS_SET);
	}
	return module_setattr_len_impl(self, attr_name, attrlen, hash, value);
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */



#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_module_not_loaded(DeeModuleObject *__restrict self) {
	return DeeError_Throwf(&DeeError_RuntimeError,
	                       "Cannot initialized Module `%k' that hasn't been loaded yet",
	                       self->mo_name);
}


/* Try to run the initializer of a module, should it not have been run yet.
 * This function will atomically ensure that the initializer
 * is only run once, and only so in a single thread.
 * Additionally, this function will also call itself recursively on
 * all other modules imported by the given one before actually invoking
 * the module's own initializer.
 * NOTE: When `DeeModule_GetRoot()' is called with `set_initialized' set to `true', the
 *       module was-initialized flag is set the same way it would be by this function.
 * @throws: Error.RuntimeError: The module has not been loaded yet. (aka. no source code was assigned)
 * @return: -1: An error occurred during initialization.
 * @return:  0: Successfully initialized the module/the module was already initialized.
 * @return:  1: You are already in the process of initializing this module (not an error). */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeModule_RunInit)(DeeObject *__restrict self) {
	uint16_t flags;
	DeeThreadObject *caller;
	DeeModuleObject *me = (DeeModuleObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);

	/* Quick check: Don't do anything else if the module has already been loaded. */
	if (me->mo_flags & Dee_MODULE_FDIDINIT)
		return 0;

	/* Make sure not to tinker with an interactive module's root code object. */
	if ((me->mo_flags & Dee_MODULE_FINITIALIZING) &&
	    DeeInteractiveModule_Check(self))
		return 0;

	COMPILER_READ_BARRIER();
	caller = DeeThread_Self();
begin_init:
	do {
		flags = atomic_read(&me->mo_flags);
		if (flags & Dee_MODULE_FDIDINIT)
			return 0;

		/* Check if the module has been loaded yet. */
		if (!(flags & Dee_MODULE_FDIDLOAD)) {
			if (flags & Dee_MODULE_FLOADING) {
#ifdef CONFIG_HOST_WINDOWS
				/* Sleep a bit longer than usually. */
				__NAMESPACE_INT_SYM SleepEx(1000, 0);
#else /* CONFIG_HOST_WINDOWS */
				SCHED_YIELD();
#endif /* !CONFIG_HOST_WINDOWS */
				goto begin_init;
			}
			return err_module_not_loaded(me); /* Not loaded yet. */
		}
	} while (!atomic_cmpxch_weak(&me->mo_flags, flags,
	                             flags | Dee_MODULE_FINITIALIZING));

	if (flags & Dee_MODULE_FINITIALIZING) {
		/* Module is already being loaded. */
#ifdef CONFIG_NO_THREADS
		return 1;
#else /* CONFIG_NO_THREADS */
		while ((flags = atomic_read(&me->mo_flags),
		        (flags & (Dee_MODULE_FINITIALIZING | Dee_MODULE_FDIDINIT)) ==
		        Dee_MODULE_FINITIALIZING)) {
			/* Check if the module is being loaded in the current thread. */
			if (me->mo_loader == caller)
				return 1;

#ifdef CONFIG_HOST_WINDOWS
			/* Sleep a bit longer than usually. */
			__NAMESPACE_INT_SYM SleepEx(1000, 0);
#else /* CONFIG_HOST_WINDOWS */
			SCHED_YIELD();
#endif /* !CONFIG_HOST_WINDOWS */
		}

		/* If the module has now been marked as having finished loading,
		 * then simply act as though it was us that did it. */
		if (flags & Dee_MODULE_FDIDINIT)
			return 0;
		goto begin_init;
#endif /* !CONFIG_NO_THREADS */
	}

	/* Setup the module to indicate that we're the ones loading it. */
#ifndef CONFIG_NO_THREADS
	me->mo_loader = caller;
#endif /* !CONFIG_NO_THREADS */

	/* FIXME: Technically, we'd need to acquire a global lock at this point
	 *        and only call `init_function' while hold it in a non-sharing
	 *        manner. Otherwise, there is a slim chance for a deadlock:
	 *        thread_1:
	 *           initialize_module("module_a"):
	 *              >> import("module_b"); // Dynamic import to prevent compiler error due to recursion.
	 *        thread_2:
	 *           initialize_module("module_b"):
	 *              >> import("module_a"); // see above...
	 *        #1: `thread_1' gets here first and starts to
	 *            run the initialization code of `module_a'
	 *        #2: `thread_2' gets here second and start to 
	 *            run the initialization code of `module_b'
	 *        #3: `thread_1' executing `module_a' starts to import `module_b'
	 *            It succeeds as the module is already in-cache and calls
	 *            `DeeModule_RunInit()' to make sure that the module has been
	 *            initialized, entering the idle-wait loop above that is
	 *            designed to wait when another thread is already initializing
	 *            the same module.
	 *        #4: `thread_2' does the same as `thread_1', and enters the wait
	 *            loop, idly waiting for `thread_1' to finish initializing
	 *            `module_a', which it never will because of the obvious DEADLOCK!
	 * NOTE:  This problem can also happen when loading a module, but is just much
	 *        more difficult to invoke as it requires execution of multi-threaded
	 *        user-code while the compiler is running, but is possible through
	 *        GC-callbacks that may be invoked when memory allocation fails during
	 *        the compilation process.
	 *  >> One sollution would be to put a global, re-en lock around this part,
	 *     but I really don't like the idea of that, especially considering how
	 *     it would make it impossible to load different modules concurrently
	 *     just on the off-chance that they might be importing each other.
	 *     However, I also don't wish get rid of dynamic import mechanics, not
	 *     only because any exposure to them at any point would lead back to
	 *     the problem of this deadlock, but also because it would greatly
	 *     hinder efficiency when there was no way of dynamically importing
	 *     modules, allowing for runtime checks on the presence of symbols.
	 * ... Python could get away with something like that thanks to
	 *     its GIL, but I would never stoop so low as to simply say:
	 *        "Yeah. We've got threads... Only one of them can ever be
	 *        executed at the same time, but it's still multi-threading..." 
	 *     me: "NO!!! THAT'S NOT TREADING! THATS JUST MADNESS AND DEFEATS THE POINT!!!"
	 * T0D0: Stop going off-topic...
	 */
#ifndef CONFIG_NO_DEX
	if (DeeDex_Check(self)) {
		if (dex_initialize((DeeDexObject *)self))
			goto err;
	} else
#endif /* !CONFIG_NO_DEX */
	{
		/* Create and run the module's initializer function. */
		DREF DeeObject *init_function, *init_result;
		DREF DeeObject *argv;
		init_function = DeeModule_GetRoot(self, false);
		if unlikely(!init_function)
			goto err;

		/* Call the module's main function with globally registered argv. */
		argv = Dee_GetArgv();
		init_result = DeeObject_CallTupleInherited(init_function, argv);
		Dee_Decref(argv);
		if unlikely(!init_result)
			goto err;
		Dee_Decref(init_result);
	}

	/* Set the did-init flag when we're done now. */
	atomic_or(&me->mo_flags, Dee_MODULE_FDIDINIT);
	return 0;
err:
	atomic_and(&me->mo_flags, ~(Dee_MODULE_FINITIALIZING));
	return -1;
}


/* Initialize all modules imported by the given one.
 * @throws: Error.RuntimeError: The module has not been loaded yet. (aka. no source code was assigned)
 * @return: -1: An error occurred during initialization.
 * @return:  0: All modules imported by the given one are now initialized. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_InitImports(/*Module*/ DeeObject *__restrict self) {
	DeeModuleObject *me = (DeeModuleObject *)self;
	size_t i;
	uint16_t flags;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	flags = atomic_read(&me->mo_flags);

	/* Quick check: When the did-init flag has been set, we can
	 *              assume that all other modules imported by
	 *              this one have also been initialized. */
	if (flags & Dee_MODULE_FDIDINIT)
		goto done;

	/* Make sure not to tinker with the imports of an interactive module. */
	if (flags & Dee_MODULE_FINITIALIZING &&
	    DeeInteractiveModule_Check(self))
		goto done;

	/* Make sure the module has been loaded. */
	if unlikely(!(flags & Dee_MODULE_FDIDLOAD))
		goto err_not_loaded; /* The module hasn't been loaded yet. */

	/* Go though and run initializers for all imported modules. */
	for (i = 0; i < me->mo_importc; ++i) {
		DeeModuleObject *import = me->mo_importv[i];
		ASSERT_OBJECT_TYPE(import, &DeeModule_Type);
		if unlikely(DeeModule_RunInit((DeeObject *)import) < 0)
			goto err;
	}

done:
	return 0;
err_not_loaded:
	err_module_not_loaded(me);
err:
	return -1;
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


/* Return the name of a global variable in the given module.
 * @return: NULL: The given `gid' is not recognized, or the module hasn't finished/started loading yet.
 * @return: * :   The name of the global associated with `gid'.
 *                Note that in the case of aliases existing for `gid', this function prefers not to
 *                return the name of an alias, but that of the original symbol itself, so long as that
 *                symbol actually exist, which if it doesn't, it will return the name of a random alias. */
PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GlobalName(DeeObject *__restrict self, uint16_t gid) {
	struct module_symbol *sym;
	sym = DeeModule_GetSymbolID((DeeModuleObject *)self, gid);
	return sym ? sym->ss_name : NULL;
}


DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_str(DeeObject *__restrict self) { /* TODO: Refactor to "tp_print" */
	DeeModuleObject *me = (DeeModuleObject *)self;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	char const *name = me->mo_absname;
	if (!name)
		return DeeString_New("<anonymous module>");
	name = DeeSystem_BaseName(name, strlen(name));
	return DeeString_New(name);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return_reference_((DeeObject *)me->mo_name);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
module_printrepr(DeeObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	DeeModuleObject *me = (DeeModuleObject *)self;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DREF DeeObject *libname = DeeModule_GetLibName((DeeObject *)me, 0);
	if (ITER_ISOK(libname))
		return DeeFormat_Printf(printer, arg, "import.%K", libname);
	if unlikely(!libname)
		return -1;
	if (me->mo_absname)
		return DeeFormat_Printf(printer, arg, "import(%q)", me->mo_absname);
	return DeeFormat_PRINT(printer, arg, "Module()");
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return DeeFormat_Printf(printer, arg, "import(%r)", me->mo_name);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_getattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name) {
	return DeeModule_GetAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_delattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name) {
	return DeeModule_DelAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
module_setattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name,
               DeeObject *__restrict value) {
	return DeeModule_SetAttr(self, name, value);
}

struct module_attriter {
	Dee_ATTRITER_HEAD
	DeeModuleObject *mai_mod;  /* [1..1][const] The module whose attributes to enumerate. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	size_t           mai_didx; /* [lock(ATOMIC)] Index into `mai_mod->mo_dir->md_files' */
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
				size_t didx;
				struct Dee_module_directory *dir = DeeModule_GetDirectory(mod);
				DeeStringObject *file;
				if unlikely(!dir)
					goto err;
				do {
					didx = atomic_read(&self->mai_didx);
					if (didx >= dir->md_count)
						return 1;
				} while (!atomic_cmpxch_or_write(&self->mai_didx, didx, didx + 1));
				file = dir->md_files[didx];
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
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
				return 1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			}
			sym = &mod->mo_bucketv[new_hidx];
			++new_hidx;
			/* Skip empty and hidden entries. */
		} while (!MODULE_SYMBOL_GETNAMESTR(sym) || (sym->ss_flags & MODSYM_FHIDDEN));
	} while (!atomic_cmpxch_or_write(&self->mai_hidx, old_hidx, new_hidx));

	perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET;
	ASSERT(Dee_module_symbol_getindex(sym) < mod->mo_globalc);
	if (!(sym->ss_flags & MODSYM_FREADONLY))
		perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	if (sym->ss_flags & MODSYM_FPROPERTY) {
		perm &= ~(Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if (!(sym->ss_flags & MODSYM_FCONSTEXPR))
			perm |= Dee_ATTRPERM_F_PROPERTY;
	}

#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
	/* For constant-expression symbols, we can predict
	 * their type (as well as their value)... */
	if (sym->ss_flags & MODSYM_FCONSTEXPR)
#endif
	{
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		int init_state = DeeModule_Initialize((DeeObject *)mod);
		if unlikely(init_state < 0)
			goto err;
		if likely(init_state == 0)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		if unlikely(DeeModule_RunInit((DeeObject *)mod) < 0)
			goto err;
		if (mod->mo_flags & Dee_MODULE_FDIDINIT)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		{
			DeeModule_LockRead(mod);
			if (sym->ss_flags & MODSYM_FPROPERTY) {
				/* Check which property operations have been bound. */
				if (mod->mo_globalv[Dee_module_symbol_getindex(sym) + MODULE_PROPERTY_GET])
					perm |= Dee_ATTRPERM_F_CANGET;
				if (!(sym->ss_flags & MODSYM_FREADONLY)) {
					/* These callbacks are only allocated if the READONLY flag isn't set. */
					if (mod->mo_globalv[Dee_module_symbol_getindex(sym) + MODULE_PROPERTY_DEL])
						perm |= Dee_ATTRPERM_F_CANDEL;
					if (mod->mo_globalv[Dee_module_symbol_getindex(sym) + MODULE_PROPERTY_SET])
						perm |= Dee_ATTRPERM_F_CANSET;
				}
			}
			DeeModule_LockEndRead(mod);
		}
	}
	doc_sym = sym;
	if (!doc_sym->ss_doc && (doc_sym->ss_flags & MODSYM_FALIAS)) {
		doc_sym = DeeModule_GetSymbolID(mod, Dee_module_symbol_getindex(doc_sym));
		ASSERT(doc_sym != NULL);
	}

	/* NOTE: Pass the module instance as declarator! */
	if (sym->ss_flags & MODSYM_FNAMEOBJ)
		perm |= Dee_ATTRPERM_F_NAMEOBJ;
	if (doc_sym->ss_flags & MODSYM_FDOCOBJ)
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
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			/* TODO: Special handling for enumerating the globals of an interactive module. */
		}
		return 0;
	}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	Dee_attriterchain_builder_init(&builder, iterbuf, bufsize);
	if (Dee_attriterchain_builder_getbufsize(&builder) >= sizeof(struct module_attriter)) {
		struct module_attriter *iter;
		iter = (struct module_attriter *)Dee_attriterchain_builder_getiterbuf(&builder);
		Dee_attriter_init(iter, &module_attriter_type);
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
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			/* TODO: Special handling for enumerating the globals of an interactive module. */
		}
		return 1;
	}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if (!specs->as_decl || specs->as_decl == (DeeObject *)self) {
		struct module_symbol *sym;
		sym = DeeModule_GetSymbolStringHash(self,
		                                    specs->as_name,
		                                    specs->as_hash);
		if (sym) {
			struct module_symbol *doc_sym;
			Dee_attrperm_t perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET;
			ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
			if (!(sym->ss_flags & MODSYM_FREADONLY))
				perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
			if (sym->ss_flags & MODSYM_FPROPERTY) {
				perm &= ~(Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
				if (!(sym->ss_flags & MODSYM_FCONSTEXPR))
					perm |= Dee_ATTRPERM_F_PROPERTY;
			}
#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
			/* For constant-expression symbols, we can predict
			 * their type (as well as their value)... */
			if (sym->ss_flags & MODSYM_FCONSTEXPR)
#endif
			{
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
				int init_state = DeeModule_Initialize((DeeObject *)self);
				if unlikely(init_state < 0)
					goto err;
				if likely(init_state == 0)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
				if unlikely(DeeModule_RunInit((DeeObject *)self) < 0)
					goto err;
				if (self->mo_flags & Dee_MODULE_FDIDINIT)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
				{
					DeeModule_LockRead(self);
					if (sym->ss_flags & MODSYM_FPROPERTY) {
						/* Check which property operations have been bound. */
						if (self->mo_globalv[Dee_module_symbol_getindex(sym) + MODULE_PROPERTY_GET])
							perm |= Dee_ATTRPERM_F_CANGET;
						if (!(sym->ss_flags & MODSYM_FREADONLY)) {
							/* These callbacks are only allocated if the READONLY flag isn't set. */
							if (self->mo_globalv[Dee_module_symbol_getindex(sym) + MODULE_PROPERTY_DEL])
								perm |= Dee_ATTRPERM_F_CANDEL;
							if (self->mo_globalv[Dee_module_symbol_getindex(sym) + MODULE_PROPERTY_SET])
								perm |= Dee_ATTRPERM_F_CANSET;
						}
					}
					DeeModule_LockEndRead(self);
				}
			}
			doc_sym = sym;
			if (!doc_sym->ss_doc && (doc_sym->ss_flags & MODSYM_FALIAS)) {
				doc_sym = DeeModule_GetSymbolID(self, Dee_module_symbol_getindex(doc_sym));
				ASSERT(doc_sym != NULL);
			}
			if (sym->ss_flags & MODSYM_FNAMEOBJ)
				perm |= Dee_ATTRPERM_F_NAMEOBJ;
			if (doc_sym->ss_flags & MODSYM_FDOCOBJ)
				perm |= Dee_ATTRPERM_F_DOCOBJ;
			if ((perm & specs->as_perm_mask) == specs->as_perm_value) {
				/* NOTE: Pass the module instance as declarator! */
				result->ad_name = Dee_MODULE_SYMBOL_GETNAMESTR(sym);
				result->ad_doc  = Dee_MODULE_SYMBOL_GETDOCSTR(doc_sym);
				result->ad_perm = perm;
				result->ad_info.ai_decl = (DeeObject *)self;
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
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
				result->ad_info.ai_decl = (DeeObject *)self;
				result->ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
				result->ad_info.ai_value.v_custom = &module_attr;
				Dee_Incref(&DeeModule_Type);
				result->ad_type = &DeeModule_Type;
				return 0;
			}
		}
#undef DIRATTR_perm
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}
	return DeeObject_TGenericFindAttr(&DeeModule_Type,
	                                  (DeeObject *)self,
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
		retinfo->ai_decl = (DeeObject *)self;
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_hasattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name) {
	return DeeModule_HasAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_boundattr(DeeModuleObject *__restrict self,
                 /*String*/ DeeObject *__restrict name) {
	return DeeModule_BoundAttr(self, name);
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

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#define module_get_code DeeModule_GetRootCode
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
PRIVATE NONNULL((1)) int DCALL
err_module_not_fully_loaded(DeeModuleObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Module %k has not been fully loaded, yet",
	                       self->mo_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_code(DeeModuleObject *__restrict self) {
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		err_module_not_fully_loaded(self);
		return NULL;
	}
	return_reference_((DREF DeeObject *)self->mo_root);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_path(DeeModuleObject *__restrict self) {
	if (!(self->mo_flags & Dee_MODULE_FDIDLOAD)) {
		err_module_not_fully_loaded(self);
		return NULL;
	}
	if (!self->mo_path)
		return_none;
	return_reference_((DREF DeeObject *)self->mo_path);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_isglobal(DeeModuleObject *__restrict self) {
	return_bool(DeeModule_IsGlobal(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_haspath(DeeModuleObject *__restrict self) {
	return_bool(DeeModule_HasPath(self));
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_imports(DeeModuleObject *__restrict self) {
	return DeeRefVector_NewReadonly((DeeObject *)self, self->mo_importc,
	                                (DeeObject **)self->mo_importv);
}

PRIVATE struct type_member tpconst module_members[] = {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	TYPE_MEMBER_FIELD_DOC("__absname__", STRUCT_STRING, offsetof(DeeModuleObject, mo_absname),
	                      "The absolute name of this module (as can be passed to "
	                      /**/ "$import to access this module from any context)"),
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TYPE_MEMBER_FIELD_DOC(STR___name__, STRUCT_OBJECT, offsetof(DeeModuleObject, mo_name),
	                      "->?Dstring\n"
	                      "The name of @this Module"),
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TYPE_MEMBER_END
};

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_ViewExports(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_ViewGlobals(DeeObject *__restrict self);

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
	               /**/ "internal index. Using this, anonymous global variables (such as property callbacks) "
	               /**/ "can be accessed and modified"),
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	TYPE_GETTER_F("__code__", &module_get_code, METHOD_FNOREFESCAPE,
	              "->?Ert:Code\n"
	              "#tValueError{The Module hasn't been fully loaded}"
	              "Returns the code object for the Module's root initializer"),
	TYPE_GETTER_F("__path__", &module_get_path, METHOD_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "#tValueError{The Module hasn't been fully loaded}"
	              "Returns the absolute filesystem path of the Module's source file, or ?N "
	              /**/ "if the Module wasn't created from a file accessible via the filesystem"),
	TYPE_GETTER_AB_F("__isglobal__", &module_get_isglobal, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this Module is global (i.e. can be accessed as ${import(this.__name__)})"),
	TYPE_GETTER_AB_F("__haspath__", &module_get_haspath, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this Module has a path found within the filesystem"),
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TYPE_GETSET_END
};

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE struct type_getset tpconst module_dee_getsets[] = {
	TYPE_GETTER_F("__code__", &module_get_code, METHOD_FCONSTCALL | METHOD_FNOTHROW,
	              "->?Ert:Code\n"
	              "#tValueError{The Module hasn't been fully loaded}"
	              "Returns the code object for the Module's root initializer"),
	TYPE_GETSET_END
};
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
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
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_class_getpath(DeeObject *__restrict UNUSED(self)) {
	DeeListObject *result = DeeModule_GetPath();
	return_reference_((DeeObject *)result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
module_class_setpath(DeeObject *UNUSED(self),
                     DeeObject *value) {
	DeeListObject *paths = DeeModule_GetPath();
	return DeeObject_Assign((DeeObject *)paths, value);
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_class_gethome(DeeObject *__restrict UNUSED(self)) {
	return DeeExec_GetHome();
}

PRIVATE struct type_getset tpconst module_class_getsets[] = {
	TYPE_GETTER_AB("home", &module_class_gethome,
	               "->?Dstring\n"
	               "The deemon home path (usually the path where the deemon executable resides)"),
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	TYPE_GETSET_AB("path", &module_class_getpath, NULL, &module_class_setpath,
	               "->?DTuple\n"
	               "A tuple of strings describing the search path for system libraries"),
	/* TODO: User-code access to "DeeModule_AddLibPath" */
	/* TODO: User-code access to "DeeModule_RemoveLibPath" */
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TYPE_GETSET_AB("path", &module_class_getpath, NULL, &module_class_setpath,
	               "->?DList\n"
	               "A list of strings describing the search path for system libraries"),

#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Deprecated aliases to emulate the old `dexmodule' builtin type. */
	TYPE_GETSET_AB("search_path", &module_class_getpath, NULL, &module_class_setpath,
	               "->?DList\n"
	               "Deprecated alias for ?#path"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TYPE_GETSET_END
};

/* WARNING: This right here doesn't work in _hostasm code (because that doesn't produce frames) */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_import_with_frame_base(DeeObject *__restrict module_name) {
	DREF DeeObject *result;
	struct code_frame *frame = DeeThread_Self()->t_exec;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (frame) {
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func->fo_code, &DeeCode_Type);
		ASSERT_OBJECT_TYPE(frame->cf_func->fo_code->co_module, &DeeModule_Type);
		result = DeeModule_Import(module_name, (DeeObject *)frame->cf_func->fo_code->co_module, DeeModule_IMPORT_F_NORMAL);
	} else {
		result = DeeModule_Import(module_name, NULL, DeeModule_IMPORT_F_NORMAL);
	}
	return result;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if (frame) {
		DeeStringObject *path;
		char const *begin, *end;

		/* Load the path of the currently executing code (for relative imports). */
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func->fo_code, &DeeCode_Type);
		ASSERT_OBJECT_TYPE(frame->cf_func->fo_code->co_module, &DeeModule_Type);
		path = frame->cf_func->fo_code->co_module->mo_path;
		if unlikely(!path)
			goto open_normal;
		ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
		begin = DeeString_AsUtf8((DeeObject *)path);
		if unlikely(!begin)
			goto err;
		end = (begin = DeeString_STR(path)) + DeeString_SIZE(path);

		/* Find the end of the current path. */
		while (end > begin && !DeeSystem_IsSep(end[-1]))
			--end;
		result = DeeModule_OpenRelative(module_name, begin, (size_t)(end - begin), NULL, true);
	} else {
open_normal:
		/* Without an execution frame, dismiss the relative import() code handling. */
		result = DeeModule_OpenGlobal(module_name, NULL, true);
	}
	if likely(result) {
		if unlikely(DeeModule_RunInit(result) < 0)
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_class_open(DeeObject *UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	/* This is pretty much the same as the builtin `import()' function.
	 * The only reason it exist is to be a deprecated alias for backwards
	 * compatibility with the old deemon. */
	DeeObject *module_name;
	DeeArg_Unpack1(err, argc, argv, "open", &module_name);
	if (DeeObject_AssertTypeExact(module_name, &DeeString_Type))
		goto err;
	return module_import_with_frame_base(module_name);
err:
	return NULL;
}


PRIVATE struct type_method tpconst module_class_methods[] = {
	/* Deprecated aliases to emulate the old `dexmodule' builtin type. */
	TYPE_METHOD("open", &module_class_open,
	            "(name:?Dstring)->?DModule\n"
	            "Deprecated alias for ?Dimport"),
	TYPE_METHOD_END
};


#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
INTDEF NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self);

PRIVATE NONNULL((1)) void DCALL
module_fini(DeeModuleObject *__restrict self) {
	weakref_support_fini(self);
	module_unbind(self);
	Dee_Decref(self->mo_name);
	Dee_XDecref(self->mo_root);
	Dee_XDecref(self->mo_path);
	if (self->mo_flags & Dee_MODULE_FDIDLOAD) {
		Dee_XDecrefv(self->mo_globalv, self->mo_globalc);
		Dee_Free(self->mo_globalv);
	}
	if (self->mo_bucketv != empty_module_buckets) {
		struct module_symbol *iter, *end;
		iter = self->mo_bucketv;
		end  = iter + self->mo_bucketm + 1;
		for (; iter < end; ++iter) {
			if (!MODULE_SYMBOL_GETNAMESTR(iter))
				continue;
			if (iter->ss_flags & MODSYM_FNAMEOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(MODULE_SYMBOL_GETNAMESTR(iter), DeeStringObject, s_str));
			if (iter->ss_flags & MODSYM_FDOCOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(iter->ss_doc, DeeStringObject, s_str));
		}
		Dee_Free((void *)self->mo_bucketv);
	}
	Dee_Decrefv(self->mo_importv, self->mo_importc);
	Dee_Free((void *)self->mo_importv);
}

PRIVATE NONNULL((1, 2)) void DCALL
module_visit(DeeModuleObject *__restrict self,
             Dee_visit_t proc, void *arg) {
	DeeModule_LockRead(self);
	/* Visit the root-code object. */
	Dee_XVisit(self->mo_root);

	Dee_Visit(self->mo_name);
	Dee_XVisit(self->mo_path);

	/* Visit global variables. */
	if (self->mo_flags & Dee_MODULE_FDIDLOAD) {
		Dee_XVisitv(self->mo_globalv, self->mo_globalc);
	}

	DeeModule_LockEndRead(self);

	/* Visit imported modules. */
	Dee_XVisitv(self->mo_importv, self->mo_importc);
}

PRIVATE NONNULL((1)) void DCALL
module_clear(DeeModuleObject *__restrict self) {
	DREF DeeObject *buffer[16];
	DREF DeeCodeObject *root_code;
	size_t i, bufi = 0;
	DeeModule_LockWrite(self);
	root_code     = self->mo_root;
	self->mo_root = NULL;
restart:
	i = self->mo_globalc;
	while (i--) {
		/* Operate in reverse order, better mirrors the
		 * (likely) order in which stuff was initialized in. */
		DREF DeeObject *ob = self->mo_globalv[i];
		if (ob == NULL)
			continue;
		self->mo_globalv[i] = NULL;
		if (!Dee_DecrefIfNotOne(ob)) {
			buffer[bufi] = ob;
			++bufi;
			if (bufi >= COMPILER_LENOF(buffer)) {
				DeeModule_LockEndWrite(self);
				Dee_Decrefv(buffer, bufi);
				bufi = 0;
				DeeModule_LockWrite(self);
				goto restart;
			}
		}
	}
	DeeModule_LockEndWrite(self);
	Dee_Decrefv(buffer, bufi);
	Dee_XDecref(root_code);
}

PRIVATE NONNULL((1)) void DCALL
module_pclear(DeeModuleObject *__restrict self,
              unsigned int priority) {
	size_t i;
	DeeModule_LockWrite(self);
	i = self->mo_globalc;
	while (i--) {
		/* Operate in reverse order, better mirrors the
		 * (likely) order in which stuff was initialized in. */
		DREF DeeObject *ob = self->mo_globalv[i];
		if (ob == NULL)
			continue;
		if (DeeObject_GCPriority(ob) < priority)
			continue; /* Clear this object in a later pass. */
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


PRIVATE struct type_gc tpconst module_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&module_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&module_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_MODULE
};
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */



#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES

/* Unbind "self" from relevant trees.
 * Caller must ensure that `self->mo_absname != NULL' */
INTDEF NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self);

PRIVATE NONNULL((1)) void DCALL
module_common_destroy(DeeModuleObject *__restrict self) {
	if (self->mo_absname) {
		module_unbind(self);
		Dee_Free(self->mo_absname);
	}
	if (self->mo_dir != NULL &&
	    self->mo_dir != (struct Dee_module_directory *)&empty_module_directory)
		Dee_module_directory_destroy(self->mo_dir);
}


/************************************************************************/
/* DeeModuleDir_Type -- /opt                                            */
/************************************************************************/
PRIVATE NONNULL((1)) void DCALL
module_dir_destroy(DeeModuleObject *__restrict self) {
	/* Assert always-true invariants for "DeeModuleDir_Type" */
	ASSERT(Dee_TYPE(self) == &DeeModuleDir_Type);
	ASSERT(self->mo_importc == 0);
	ASSERT(self->mo_importv == NULL);
	ASSERT(self->mo_globalc == 0);
	ASSERT(self->mo_bucketm == 0);
	ASSERT(self->mo_bucketv == empty_module_buckets);
	ASSERT(self->mo_init == Dee_MODULE_INIT_INITIALIZED);
	self = (DeeModuleObject *)DeeGC_Untrack((DeeObject *)self);
	module_common_destroy(self);
	DeeGCObject_Free(self);
}

PRIVATE struct type_gc tpconst module_dir_gc = {
	/* .tp_clear  = */ NULL,
	/* .tp_pclear = */ NULL,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_MODULE
};



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
	self = (DeeModuleObject *)DeeGC_Untrack((DeeObject *)self);

	/* Unbind from `module_byaddr_tree' */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	module_dee_unbind(self);
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */

	/* Destroy Dee-specific data */
	Dee_XDecrefv(self->mo_globalv, self->mo_globalc);
	Dee_Decref(self->mo_moddata.mo_rootcode);
	Dee_Decrefv(self->mo_importv, self->mo_importc);
	Dee_Free((void *)self->mo_importv);

	/* Destroy common data */
	module_common_destroy(self);
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	/* TODO: Custom version of Dee_Free() that doesn't DBG_memset() the payload.
	 * Reason: the module's "ob_refcnt" must remain set to "0" so that DeeHeap_GetRegionOf()
	 *         can still locate the module of an object, but see that it's ob_refcnt==0,
	 *         and then not trying to incref it again (which would be possible if DBG_memset
	 *         were to fill ob_refcnt with some non-zero debug pattern)
	 * @see TODO in DeeDecWriter_AppendModule */
	DeeGCObject_Free(self);
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	DeeGCObject_Free(self);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
}

PRIVATE NONNULL((1)) void DCALL
module_dee_clear(DeeModuleObject *__restrict self) {
	uint16_t i;
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

PRIVATE NONNULL((1)) void DCALL
module_dee_pclear(DeeModuleObject *__restrict self, unsigned int priority) {
	uint16_t i;
	DeeModule_LockWrite(self);
	for (i = self->mo_globalc; i--;) {
		/* Operate in reverse order, better mirrors the
		 * (likely) order in which stuff was initialized in. */
		DREF DeeObject *ob = self->mo_globalv[i];
		if (ob == NULL)
			continue;
		if (DeeObject_GCPriority(ob) < priority)
			continue; /* Clear this object in a later pass. */
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

PRIVATE struct type_gc tpconst module_dee_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&module_dee_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&module_dee_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_MODULE
};




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

PRIVATE NONNULL((1)) void DCALL
module_dex_destroy(DeeModuleObject *__restrict self) {
	struct Dee_module_dexdata *dexdata;
	/* Assert always-true invariants for "DeeModuleDex_Type" */
	ASSERT(Dee_TYPE(self) == &DeeModuleDex_Type);
	ASSERT(self->mo_init == Dee_MODULE_INIT_INITIALIZED ||
	       self->mo_init == Dee_MODULE_INIT_UNINITIALIZED);
	ASSERT(self->mo_importc == 0);
	ASSERT(self->mo_importv == NULL);
	self = (DeeModuleObject *)DeeGC_Untrack((DeeObject *)self);
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

	/* Destroy common data */
	Dee_XDecrefv(self->mo_globalv, self->mo_globalc);
	module_common_destroy(self);
	DeeSystem_DlClose(dexdata->mdx_handle);
}

PRIVATE NONNULL((1)) void DCALL
module_dex_clear_common(DeeModuleObject *__restrict self) {
	struct Dee_module_dexdata *dexdata = self->mo_moddata.mo_dexdata;
	if (dexdata->mdx_clear)
		(*dexdata->mdx_clear)();
}

PRIVATE NONNULL((1)) void DCALL
module_dex_clear(DeeModuleObject *__restrict self) {
	module_dex_clear_common(self);
	module_dee_clear(self);
}

PRIVATE NONNULL((1)) void DCALL
module_dex_pclear(DeeModuleObject *__restrict self, unsigned int priority) {
	module_dex_clear_common(self);
	module_dee_pclear(self, priority);
}

PRIVATE struct type_gc tpconst module_dex_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&module_dex_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&module_dex_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_MODULE
};
#endif /* !CONFIG_NO_DEX */

PUBLIC DeeTypeObject DeeModule_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Module),
	/* .tp_doc      = */ DOC("str->\n"
	                         "Returns the name of the module"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT | TP_FVARIABLE,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,   /* !!! Not constructible !!! */
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,   /* !!! Not constructible !!! */
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,   /* !!! Not constructible !!! */
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,   /* !!! Not constructible !!! */
				/* .tp_free      = */ (Dee_funptr_t)NULL, { NULL },
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL, /* !!! Not constructible !!! */
				/* .tp_writedec    = */ (Dee_funptr_t)NULL
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&module_str,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&module_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &DeeObject_GenericCmpByAddr,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ module_getsets,
	/* .tp_members       = */ module_members,
	/* .tp_class_methods = */ module_class_methods,
	/* .tp_class_getsets = */ module_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};

PUBLIC DeeTypeObject DeeModuleDir_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleDir",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				/* .tp_free      = */ (Dee_funptr_t)NULL, { NULL },
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL, /* TODO */
				/* .tp_writedec    = */ (Dee_funptr_t)NULL  /* TODO */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&module_dir_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ NULL, // (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_dir_visit, /* Wouldn't have anything to visit! */ 
	/* .tp_gc            = */ &module_dir_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &module_attr, /* TODO: Custom operators (that only look at directory files) */
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
};

PUBLIC DeeTypeObject DeeModuleDee_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleDee",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				/* .tp_free      = */ (Dee_funptr_t)NULL, { NULL },
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL, /* TODO */
				/* .tp_writedec    = */ (Dee_funptr_t)NULL  /* TODO */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&module_dee_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_dee_visit,
	/* .tp_gc            = */ &module_dee_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ module_dee_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};

#ifndef CONFIG_NO_DEX
PUBLIC DeeTypeObject DeeModuleDex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleDex",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				/* .tp_free      = */ (Dee_funptr_t)NULL, { NULL },
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL, /* TODO */
				/* .tp_writedec    = */ (Dee_funptr_t)NULL  /* TODO */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
		/* .tp_destroy     = */ (void (DCALL *)(DeeObject *__restrict))&module_dex_destroy,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_dex_visit,
	/* .tp_gc            = */ &module_dex_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &module_attr,
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
};
#endif /* !CONFIG_NO_DEX */


struct Dee_empty_module_struct {
	/* Even though never tracked, static modules still need the GC header for visiting. */
	struct Dee_gc_head_link          m_head;
	Dee_MODULE_STRUCT_EX(/**/, /**/) m_module;
};

#undef DeeModule_Empty
INTERN struct Dee_empty_module_struct DeeModule_Empty = {
	{
		/* ... */
		NULL,
		NULL
	}, {
		OBJECT_HEAD_INIT(&DeeModuleDee_Type),
		/* .mo_absname = */ NULL,
		/* .mo_absnode = */ { NULL, NULL, NULL },
		/* .mo_libname = */ {
			/* .mle_name = */ NULL,
			/* .mle_dat  = */ { (DeeModuleObject *)&DeeModule_Empty.m_module },
			/* .mle_node = */ { NULL, NULL, NULL },
			/* .mle_next = */ NULL
		},
		/* .mo_dir     = */ (struct Dee_module_directory *)&empty_module_directory,
		/* .mo_init    = */ Dee_MODULE_INIT_INITIALIZED,
		/* .mo_ctime   = */ 0,
		/* .mo_flags   = */ Dee_MODULE_FNORMAL | Dee_MODULE_FHASCTIME,
		/* .mo_importc = */ 0,
		/* .mo_globalc = */ 0,
		/* .mo_bucketm = */ 0,
		/* .mo_bucketv = */ empty_module_buckets,
		_Dee_MODULE_INIT_mo_lock
		WEAKREF_SUPPORT_INIT,
		/* .mo_moddata = */ Dee_MODULE_MODDATA_INIT_CODE(&DeeCode_Empty),
		/* .mo_importv = */ NULL,
#if !defined(CONFIG_NO_DEC) || !defined(CONFIG_NO_DEX)
		/* .mo_minaddr = */ NULL,
		/* .mo_maxaddr = */ NULL,
		/* .mo_adrnode = */ NULL,
#endif /* !CONFIG_NO_DEC || !CONFIG_NO_DEX */
	}
};

#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
PUBLIC DeeTypeObject DeeModule_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Module),
	/* .tp_doc      = */ DOC("str->\n"
	                         "Returns the name of the module"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_GC(DeeModuleObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&module_fini,
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&module_visit,
	/* .tp_gc            = */ &module_gc,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeObject_GenericCmpByAddr,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ module_getsets,
	/* .tp_members       = */ module_members,
	/* .tp_class_methods = */ module_class_methods,
	/* .tp_class_getsets = */ module_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#ifdef __INTELLISENSE__
INTERN struct Dee_static_module_struct DeeModule_Empty_real =
#else /* __INTELLISENSE__ */
#undef DeeModule_Empty
INTERN struct Dee_static_module_struct DeeModule_Empty =
#endif /* !__INTELLISENSE__ */
{
	{
		/* ... */
		NULL,
		NULL
	}, {
		OBJECT_HEAD_INIT(&DeeModule_Type),
		/* .mo_name      = */ (DeeStringObject *)Dee_EmptyString,
		/* .mo_link      = */ LIST_ENTRY_UNBOUND_INITIALIZER,
		/* .mo_path      = */ NULL,
#ifdef DEE_SYSTEM_FS_ICASE
		/* .mo_pathihash  = */ 0,
#endif /* DEE_SYSTEM_FS_ICASE */
		/* .mo_globlink  = */ LIST_ENTRY_UNBOUND_INITIALIZER,
		/* .mo_importc   = */ 0,
		/* .mo_globalc   = */ 0,
#ifndef CONFIG_NO_DEC
		/* .mo_flags     = */ Dee_MODULE_FDIDINIT | Dee_MODULE_FDIDLOAD | Dee_MODULE_FHASCTIME,
#else /* !CONFIG_NO_DEC */
		/* .mo_flags     = */ Dee_MODULE_FDIDINIT | Dee_MODULE_FDIDLOAD,
#endif /* CONFIG_NO_DEC */
		/* .mo_bucketm   = */ 0,
		/* .mo_bucketv   = */ empty_module_buckets,
		/* .mo_importv   = */ NULL,
		/* .mo_globalv   = */ NULL,
		/* .mo_root      = */ &DeeCode_Empty,
#ifndef CONFIG_NO_THREADS
		/* .mo_lock      = */ Dee_ATOMIC_RWLOCK_INIT,
		/* .mo_loader    = */ NULL,
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEC
		/* .mo_ctime     = */ 0,
#endif /* !CONFIG_NO_DEC */
		WEAKREF_SUPPORT_INIT
	}
};
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_C */
