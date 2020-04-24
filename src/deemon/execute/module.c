/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
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
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>

#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif /* !CONFIG_NO_DEX */

#include <string.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN


INTERN WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
module_symbol_getnameobj(struct module_symbol *__restrict self) {
	DREF DeeStringObject *result;
	char const *name_str;
	uint16_t flags;
again:
	name_str = self->ss_name;
	__hybrid_atomic_thread_fence(__ATOMIC_ACQUIRE);
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
		if (!ATOMIC_CMPXCH(self->ss_name, name_str, DeeString_STR(result))) {
			Dee_Decref(result);
			goto again;
		}
		ATOMIC_FETCHOR(self->ss_flags, MODSYM_FNAMEOBJ);
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
	__hybrid_atomic_thread_fence(__ATOMIC_ACQUIRE);
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
		if (!ATOMIC_CMPXCH(self->ss_doc, doc_str, DeeString_STR(result))) {
			Dee_Decref(result);
			goto again;
		}
		ATOMIC_FETCHOR(self->ss_flags, MODSYM_FDOCOBJ);
		Dee_Incref(result);
	}
done:
	return result;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_GetRoot(DeeObject *__restrict self,
                  bool set_initialized) {
	DREF DeeFunctionObject *result;
	DeeModuleObject *me = (DeeModuleObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	/* Check if this module has been loaded. */
	if unlikely(DeeModule_InitImports(self))
		return NULL;
	result = (DREF DeeFunctionObject *)DeeObject_Malloc(offsetof(DeeFunctionObject, fo_refv));
	if unlikely(!result)
		return NULL;
	rwlock_read(&me->mo_lock);
	result->fo_code = me->mo_root;
	Dee_XIncref(result->fo_code);
	rwlock_endread(&me->mo_lock);
	if (!result->fo_code) {
		result->fo_code = &empty_code;
		Dee_Incref(&empty_code);
	}
	DeeObject_Init(result, &DeeFunction_Type);
	if (set_initialized) {
		uint16_t flags;
		/* Try to set the `MODULE_FDIDINIT' flag */
		do {
			flags = ATOMIC_READ(me->mo_flags);
			if (flags & MODULE_FINITIALIZING)
				break; /* Don't interfere with an on-going initialization. */
		} while (!ATOMIC_CMPXCH_WEAK(me->mo_flags, flags, flags | MODULE_FDIDINIT));
	}
	return (DREF DeeObject *)result;
}


INTERN struct module_symbol empty_module_buckets[] = {
	{ NULL, 0, 0 }
};



PUBLIC WUNUSED NONNULL((1)) struct module_symbol *DCALL
DeeModule_GetSymbolID(DeeModuleObject *__restrict self, uint16_t gid) {
	struct module_symbol *iter, *end;
	struct module_symbol *result = NULL;
	/* Check if the module has actually been loaded yet.
	 * This needs to be done to prevent a race condition
	 * when reading the bucket fields below, as they only
	 * become immutable once this flag has been set. */
	if unlikely(!(self->mo_flags & MODULE_FDIDLOAD))
		return NULL;
	end = (iter = self->mo_bucketv) + (self->mo_bucketm + 1);
	for (; iter != end; ++iter) {
		if (!MODULE_SYMBOL_GETNAMESTR(iter))
			continue; /* Skip empty entries. */
		if (iter->ss_index == gid) {
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
DeeModule_GetSymbolString(DeeModuleObject *__restrict self,
                          char const *__restrict attr_name,
                          dhash_t hash) {
	dhash_t i, perturb;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	ASSERT(!DeeInteractiveModule_Check(self));
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name))
			continue;
		return item;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
DeeModule_GetSymbolStringLen(DeeModuleObject *__restrict self,
                             char const *__restrict attr_name,
                             size_t attrlen, dhash_t hash) {
	dhash_t i, perturb;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (memcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen * sizeof(char)) != 0)
			continue;
		return item;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrSymbol(DeeModuleObject *__restrict self,
                        struct module_symbol *__restrict symbol) {
	DREF DeeObject *result;
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
read_symbol:
		ASSERT(symbol->ss_index < self->mo_globalc);
		rwlock_read(&self->mo_lock);
		result = self->mo_globalv[symbol->ss_index];
		Dee_XIncref(result);
		rwlock_endread(&self->mo_lock);
		if unlikely(!result)
			err_unbound_global(self, symbol->ss_index);
		return result;
	}
	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		rwlock_read(&self->mo_lock);
		callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		rwlock_endread(&self->mo_lock);
		if unlikely(!callback) {
			err_module_cannot_read_property(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			return NULL;
		}
		/* Invoke the property callback. */
		result = DeeObject_Call(callback, 0, NULL);
		Dee_Decref(callback);
		return result;
	}
	/* External symbol. */
	ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
	self = self->mo_importv[symbol->ss_extern.ss_impid];
	goto read_symbol;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrSymbol(DeeModuleObject *__restrict self,
                          struct module_symbol *__restrict symbol) {
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
		bool result;
read_symbol:
		ASSERT(symbol->ss_index < self->mo_globalc);
		rwlock_read(&self->mo_lock);
		result = self->mo_globalv[symbol->ss_index] != NULL;
		rwlock_endread(&self->mo_lock);
		return result;
	}
	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback, *callback_result;
		rwlock_read(&self->mo_lock);
		callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		rwlock_endread(&self->mo_lock);
		if unlikely(!callback)
			return 0;
		/* Invoke the property callback. */
		callback_result = DeeObject_Call(callback, 0, NULL);
		Dee_Decref(callback);
		if likely(callback_result) {
			Dee_Decref(callback_result);
			return 1;
		}
		if (CATCH_ATTRIBUTE_ERROR())
			return -3;
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return 0;
		return -1;
	}
	/* External symbol. */
	ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
	self = self->mo_importv[symbol->ss_extern.ss_impid];
	goto read_symbol;
}

LOCAL WUNUSED DREF DeeObject *DCALL
module_getattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
	dhash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (!strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name))
			return DeeModule_GetAttrSymbol(self, item);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrString((DeeObject *)self, attr_name, hash);
	if (result != ITER_DONE)
		return result;
	err_module_no_such_global(self, attr_name, ATTR_ACCESS_GET);
	return NULL;
}

LOCAL WUNUSED DREF DeeObject *DCALL
module_getattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash) {
	dhash_t i, perturb;
	DREF DeeObject *result;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (memcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen * sizeof(char)) == 0)
			return DeeModule_GetAttrSymbol(self, item);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	result = DeeObject_GenericGetAttrStringLen((DeeObject *)self,
	                                           attr_name,
	                                           attrlen,
	                                           hash);
	if (result != ITER_DONE)
		return result;
	err_module_no_such_global_len(self,
	                              attr_name,
	                              attrlen,
	                              ATTR_ACCESS_GET);
	return NULL;
}

LOCAL int DCALL
module_boundattr_impl(DeeModuleObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash) {
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (!strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name))
			return DeeModule_BoundAttrSymbol(self, item);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericBoundAttrString((DeeObject *)self, attr_name, hash);
}

LOCAL int DCALL
module_boundattr_len_impl(DeeModuleObject *__restrict self,
                          char const *__restrict attr_name,
                          size_t attrlen, dhash_t hash) {
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (memcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen * sizeof(char)) == 0)
			return DeeModule_BoundAttrSymbol(self, item);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericBoundAttrStringLen((DeeObject *)self,
	                                           attr_name,
	                                           attrlen,
	                                           hash);
}

LOCAL bool DCALL
module_hasattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (!strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name))
			return true;
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericHasAttrString((DeeObject *)self, attr_name, hash);
}

LOCAL bool DCALL
module_hasattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash) {
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (memcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen * sizeof(char)) == 0)
			return true;
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	return DeeObject_GenericHasAttrStringLen((DeeObject *)self,
	                                         attr_name,
	                                         attrlen,
	                                         hash);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrSymbol(DeeModuleObject *__restrict self,
                        struct module_symbol *__restrict symbol) {
	DREF DeeObject *old_value;
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if unlikely(symbol->ss_flags & (MODSYM_FREADONLY | MODSYM_FEXTERN | MODSYM_FPROPERTY)) {
		if (symbol->ss_flags & MODSYM_FREADONLY)
			return err_module_readonly_global(self, MODULE_SYMBOL_GETNAMESTR(symbol));
		if (symbol->ss_flags & MODSYM_FPROPERTY) {
			DREF DeeObject *callback, *temp;
			rwlock_read(&self->mo_lock);
			ASSERT(symbol->ss_index + MODULE_PROPERTY_DEL < self->mo_globalc);
			callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_DEL];
			Dee_XIncref(callback);
			rwlock_endread(&self->mo_lock);
			if unlikely(!callback)
				return err_module_cannot_delete_property(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			/* Invoke the property callback. */
			temp = DeeObject_Call(callback, 0, NULL);
			Dee_Decref(callback);
			Dee_XDecref(temp);
			return temp ? 0 : -1;
		}
		/* External symbol. */
		ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
		self = self->mo_importv[symbol->ss_extern.ss_impid];
	}
	ASSERT(symbol->ss_index < self->mo_globalc);
	rwlock_write(&self->mo_lock);
	old_value                          = self->mo_globalv[symbol->ss_index];
	self->mo_globalv[symbol->ss_index] = NULL;
	rwlock_endwrite(&self->mo_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	if unlikely(!old_value) {
		err_unbound_global(self, symbol->ss_index);
		return -1;
	}
	Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
	Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
}

LOCAL int DCALL
module_delattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
	int error;
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (!strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name))
			return DeeModule_DelAttrSymbol(self, item);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericDelAttrString((DeeObject *)self, attr_name, hash);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global(self, attr_name, ATTR_ACCESS_DEL);
}

LOCAL int DCALL
module_delattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash) {
	int error;
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (memcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen * sizeof(char)) == 0)
			return DeeModule_DelAttrSymbol(self, item);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericDelAttrStringLen((DeeObject *)self,
	                                          attr_name,
	                                          attrlen,
	                                          hash);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_len(self,
	                                     attr_name,
	                                     attrlen,
	                                     ATTR_ACCESS_DEL);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeModule_SetAttrSymbol(DeeModuleObject *__restrict self,
                        struct module_symbol *__restrict symbol,
                        DeeObject *__restrict value) {
	DREF DeeObject *temp;
	ASSERT(symbol >= self->mo_bucketv &&
	       symbol <= self->mo_bucketv + self->mo_bucketm);
	if unlikely(symbol->ss_flags & (MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FEXTERN)) {
		if unlikely(symbol->ss_flags & MODSYM_FEXTERN) {
			ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
			self = self->mo_importv[symbol->ss_extern.ss_impid];
		}
		if (symbol->ss_flags & MODSYM_FREADONLY) {
			if (symbol->ss_flags & MODSYM_FPROPERTY)
				goto err_is_readonly;
			ASSERT(symbol->ss_index < self->mo_globalc);
			rwlock_write(&self->mo_lock);
			/* Make sure not to allow write-access to global variables that
			 * have already been assigned, but are marked as read-only. */
			if unlikely(self->mo_globalv[symbol->ss_index] != NULL) {
				rwlock_endwrite(&self->mo_lock);
err_is_readonly:
				return err_module_readonly_global(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			}
			Dee_Incref(value);
			self->mo_globalv[symbol->ss_index] = value;
			rwlock_endwrite(&self->mo_lock);
			return 0;
		}
		if (symbol->ss_flags & MODSYM_FPROPERTY) {
			DREF DeeObject *callback;
			rwlock_write(&self->mo_lock);
			callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_SET];
			Dee_XIncref(callback);
			rwlock_endwrite(&self->mo_lock);
			if unlikely(!callback)
				return err_module_cannot_write_property(self, MODULE_SYMBOL_GETNAMESTR(symbol));
			temp = DeeObject_Call(callback, 1, (DeeObject **)&value);
			Dee_Decref(callback);
			Dee_XDecref(temp);
			return temp ? 0 : -1;
		}
		/* External symbol. */
		ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
		self = self->mo_importv[symbol->ss_extern.ss_impid];
	}
	ASSERT(symbol->ss_index < self->mo_globalc);
	Dee_Incref(value);
	rwlock_write(&self->mo_lock);
	temp                               = self->mo_globalv[symbol->ss_index];
	self->mo_globalv[symbol->ss_index] = value;
	rwlock_endwrite(&self->mo_lock);
	Dee_XDecref(temp);
	return 0;
}

LOCAL int DCALL
module_setattr_impl(DeeModuleObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash,
                    DeeObject *__restrict value) {
	int error;
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (!strcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name))
			return DeeModule_SetAttrSymbol(self, item, value);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericSetAttrString((DeeObject *)self, attr_name, hash, value);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global(self, attr_name, ATTR_ACCESS_SET);
}

LOCAL int DCALL
module_setattr_len_impl(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash,
                        DeeObject *__restrict value) {
	int error;
	dhash_t i, perturb;
	perturb = i = MODULE_HASHST(self, hash);
	for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
		struct module_symbol *item = MODULE_HASHIT(self, i);
		if (!item->ss_name)
			break; /* Not found */
		if (item->ss_hash != hash)
			continue; /* Non-matching hash */
		if (MODULE_SYMBOL_GETNAMELEN(item) != attrlen)
			continue; /* Non-matching length */
		if (memcmp(MODULE_SYMBOL_GETNAMESTR(item), attr_name, attrlen * sizeof(char)) == 0)
			return DeeModule_SetAttrSymbol(self, item, value);
	}
	/* Fallback: Do a generic attribute lookup on the module. */
	error = DeeObject_GenericSetAttrStringLen((DeeObject *)self,
	                                          attr_name,
	                                          attrlen,
	                                          hash,
	                                          value);
	if unlikely(error <= 0)
		return error;
	return err_module_no_such_global_len(self,
	                                     attr_name,
	                                     attrlen,
	                                     ATTR_ACCESS_SET);
}


#ifndef CONFIG_NO_THREADS
INTDEF NONNULL((1)) void DCALL interactivemodule_lockread(DeeModuleObject *__restrict self);
INTDEF NONNULL((1)) void DCALL interactivemodule_lockwrite(DeeModuleObject *__restrict self);
INTDEF NONNULL((1)) void DCALL interactivemodule_lockendread(DeeModuleObject *__restrict self);
INTDEF NONNULL((1)) void DCALL interactivemodule_lockendwrite(DeeModuleObject *__restrict self);
#else /* !CONFIG_NO_THREADS */
#deifne interactivemodule_lockread(self)     (void)0
#deifne interactivemodule_lockwrite(self)    (void)0
#deifne interactivemodule_lockendread(self)  (void)0
#deifne interactivemodule_lockendwrite(self) (void)0
#endif /* CONFIG_NO_THREADS */


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			DREF DeeObject *result;
			interactivemodule_lockread(self);
			result = module_getattr_impl(self, attr_name, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		err_module_not_loaded_attr(self, attr_name, ATTR_ACCESS_GET);
		return NULL;
	}
	return module_getattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetAttrStringLen(DeeModuleObject *__restrict self,
                           char const *__restrict attr_name,
                           size_t attrlen, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			DREF DeeObject *result;
			interactivemodule_lockread(self);
			result = module_getattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		err_module_not_loaded_attr_len(self,
		                               attr_name,
		                               attrlen,
		                               ATTR_ACCESS_GET);
		return NULL;
	}
	return module_getattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrString(DeeModuleObject *__restrict self,
                          char const *__restrict attr_name, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			interactivemodule_lockread(self);
			result = module_boundattr_impl(self, attr_name, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return -2;
	}
	return module_boundattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrStringLen(DeeModuleObject *__restrict self,
                             char const *__restrict attr_name,
                             size_t attrlen, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			interactivemodule_lockread(self);
			result = module_boundattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return -2;
	}
	return module_boundattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeModule_HasAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			bool result;
			interactivemodule_lockread(self);
			result = module_hasattr_impl(self, attr_name, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return false;
	}
	return module_hasattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeModule_HasAttrStringLen(DeeModuleObject *__restrict self,
                           char const *__restrict attr_name,
                           size_t attrlen, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			bool result;
			interactivemodule_lockread(self);
			result = module_hasattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendread(self);
			return result;
		}
		return false;
	}
	return module_hasattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			interactivemodule_lockwrite(self);
			result = module_delattr_impl(self, attr_name, hash);
			interactivemodule_lockendwrite(self);
			return result;
		}
		return err_module_not_loaded_attr(self,
		                                  attr_name,
		                                  ATTR_ACCESS_DEL);
	}
	return module_delattr_impl(self, attr_name, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_DelAttrStringLen(DeeModuleObject *__restrict self,
                           char const *__restrict attr_name,
                           size_t attrlen, dhash_t hash) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			interactivemodule_lockwrite(self);
			result = module_delattr_len_impl(self, attr_name, attrlen, hash);
			interactivemodule_lockendwrite(self);
			return result;
		}
		return err_module_not_loaded_attr_len(self,
		                                      attr_name,
		                                      attrlen,
		                                      ATTR_ACCESS_DEL);
	}
	return module_delattr_len_impl(self, attr_name, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeModule_SetAttrString(DeeModuleObject *self,
                        char const *__restrict attr_name, dhash_t hash,
                        DeeObject *value) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			interactivemodule_lockwrite(self);
			result = module_setattr_impl(self, attr_name, hash, value);
			interactivemodule_lockendwrite(self);
			return result;
		}
		return err_module_not_loaded_attr(self,
		                                  attr_name,
		                                  ATTR_ACCESS_SET);
	}
	return module_setattr_impl(self, attr_name, hash, value);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeModule_SetAttrStringLen(DeeModuleObject *self,
                           char const *__restrict attr_name,
                           size_t attrlen, dhash_t hash,
                           DeeObject *value) {
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			int result;
			interactivemodule_lockwrite(self);
			result = module_setattr_len_impl(self, attr_name, attrlen, hash, value);
			interactivemodule_lockendwrite(self);
			return result;
		}
		return err_module_not_loaded_attr_len(self,
		                                      attr_name,
		                                      attrlen,
		                                      ATTR_ACCESS_SET);
	}
	return module_setattr_len_impl(self, attr_name, attrlen, hash, value);
}


PRIVATE NONNULL((1)) void DCALL
err_module_not_loaded(DeeModuleObject *__restrict self) {
	DeeError_Throwf(&DeeError_RuntimeError,
	                "Cannot initialized Module `%k' that hasn't been loaded yet",
	                self->mo_name);
}


PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeModule_RunInit(DeeObject *__restrict self) {
	uint16_t flags;
	DeeThreadObject *caller;
	DeeModuleObject *me = (DeeModuleObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	/* Quick check: Don't do anything else if the module has already been loaded. */
	if (me->mo_flags & MODULE_FDIDINIT)
		return 0;
	/* Make sure not to tinker with an interactive module's root code object. */
	if ((me->mo_flags & MODULE_FINITIALIZING) &&
	    DeeInteractiveModule_Check(self))
		return 0;

	COMPILER_READ_BARRIER();
	caller = DeeThread_Self();
begin_init:
	do {
		flags = ATOMIC_READ(me->mo_flags);
		if (flags & MODULE_FDIDINIT)
			return 0;
		/* Check if the module has been loaded yet. */
		if (!(flags & MODULE_FDIDLOAD)) {
			if (flags & MODULE_FLOADING) {
#ifdef CONFIG_HOST_WINDOWS
				/* Sleep a bit longer than usually. */
				__NAMESPACE_INT_SYM SleepEx(1000, 0);
#else /* CONFIG_HOST_WINDOWS */
				SCHED_YIELD();
#endif /* !CONFIG_HOST_WINDOWS */
				goto begin_init;
			}
			err_module_not_loaded(me);
			return -1; /* Not loaded yet. */
		}
	} while (!ATOMIC_CMPXCH(me->mo_flags, flags, flags | MODULE_FINITIALIZING));
	if (flags & MODULE_FINITIALIZING) {
		/* Module is already being loaded. */
		while ((flags = ATOMIC_READ(me->mo_flags),
		        (flags & (MODULE_FINITIALIZING | MODULE_FDIDINIT)) ==
		        MODULE_FINITIALIZING)) {
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
		if (flags & MODULE_FDIDINIT)
			return 0;
		goto begin_init;
	}
	/* Setup the module to indicate that we're the ones loading it. */
	me->mo_loader = caller;

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
	 *             run the initialization code of `module_a'
	 *        #2: `thread_2' gets here second and start to 
	 *             run the initialization code of `module_b'
	 *        #3: `thread_1' executing `module_a' starts to import `module_b'
	 *             It succeeds as the module is already in-cache and calls
	 *            `DeeModule_RunInit()' to make sure that the module has been
	 *             initialized, entering the idle-wait loop above that is
	 *             designed to wait when another thread is already initializing
	 *             the same module.
	 *        #4: `thread_2' does the same as `thread_1', and enters the wait
	 *             loop, idly waiting for `thread_1' to finish initializing
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
	 *        "Yeah. We've got thread... Only one of them can ever be
	 *         executed at the same time, but it's still multi-threading..." 
	 *     me: "NO!!! THIS IS MADNESS AND DEFEATS THE POINT!!!"
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
		argv        = Dee_GetArgv();
		init_result = DeeObject_Call(init_function,
		                             DeeTuple_SIZE(argv),
		                             DeeTuple_ELEM(argv));
		Dee_Decref(argv);
		Dee_Decref(init_function);
		if unlikely(!init_result)
			goto err;
		Dee_Decref(init_result);
	}

	/* Set the did-init flag when we're done now. */
	ATOMIC_FETCHOR(me->mo_flags, MODULE_FDIDINIT);
	return 0;
err:
	ATOMIC_FETCHAND(me->mo_flags, ~(MODULE_FINITIALIZING));
	return -1;
}


PUBLIC int (DCALL DeeModule_InitImports)(DeeObject *__restrict self) {
	DeeModuleObject *me = (DeeModuleObject *)self;
	size_t i;
	uint16_t flags;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	flags = ATOMIC_READ(me->mo_flags);
	/* Quick check: When the did-init flag has been set, we can
	 *              assume that all other modules imported by
	 *              this one have also been initialized. */
	if (flags & MODULE_FDIDINIT)
		return 0;
	/* Make sure not to tinker with the imports of an interactive module. */
	if (flags & MODULE_FINITIALIZING &&
	    DeeInteractiveModule_Check(self))
		return 0;
	/* Make sure the module has been loaded. */
	if unlikely(!(flags & MODULE_FDIDLOAD)) {
		/* The module hasn't been loaded yet. */
		err_module_not_loaded(me);
		return -1;
	}
	/* Go though and run initializers for all imported modules. */
	for (i = 0; i < me->mo_importc; ++i) {
		DeeModuleObject *import = me->mo_importv[i];
		ASSERT_OBJECT_TYPE(import, &DeeModule_Type);
		if unlikely(DeeModule_RunInit((DeeObject *)import) < 0)
			return -1;
	}
	return 0;
}


PUBLIC WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GlobalName(DeeObject *__restrict self, uint16_t gid) {
	struct module_symbol *sym;
	sym = DeeModule_GetSymbolID((DeeModuleObject *)self, gid);
	return sym ? sym->ss_name : NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_str(DeeModuleObject *__restrict self) {
	return_reference_((DeeObject *)self->mo_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_repr(DeeModuleObject *__restrict self) {
	return DeeString_Newf("import(%r)", self->mo_name);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_getattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name) {
	return DeeModule_GetAttrString(self,
	                               DeeString_STR(name),
	                               DeeString_Hash(name));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
module_delattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name) {
	return DeeModule_DelAttrString(self,
	                               DeeString_STR(name),
	                               DeeString_Hash(name));
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
module_setattr(DeeModuleObject *__restrict self,
               /*String*/ DeeObject *__restrict name,
               DeeObject *__restrict value) {
	return DeeModule_SetAttrString(self,
	                               DeeString_STR(name),
	                               DeeString_Hash(name), value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
module_enumattr(DeeTypeObject *UNUSED(tp_self),
                DeeModuleObject *self, denum_t proc, void *arg) {
	struct module_symbol *iter, *end, *doc_iter;
	dssize_t temp, result = 0;
	ASSERT_OBJECT(self);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			/* TODO: Special handling for enumerating the globals of an interactive module. */
		}
		return 0;
	}
	end = (iter = self->mo_bucketv) + (self->mo_bucketm + 1);
	for (; iter != end; ++iter) {
		uint16_t perm;
		DREF DeeTypeObject *attr_type;
		/* Skip empty and hidden entries. */
		if (!MODULE_SYMBOL_GETNAMESTR(iter) || (iter->ss_flags & MODSYM_FHIDDEN))
			continue;
		perm = ATTR_IMEMBER | ATTR_ACCESS_GET;
		ASSERT(iter->ss_index < self->mo_globalc);
		if (!(iter->ss_flags & MODSYM_FREADONLY))
			perm |= (ATTR_ACCESS_DEL | ATTR_ACCESS_SET);
		if (iter->ss_flags & MODSYM_FPROPERTY) {
			perm &= ~(ATTR_ACCESS_GET | ATTR_ACCESS_DEL | ATTR_ACCESS_SET);
			if (!(iter->ss_flags & MODSYM_FCONSTEXPR))
				perm |= ATTR_PROPERTY;
		}
		attr_type = NULL;
#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
		/* For constant-expression symbols, we can predict
		 * their type (as well as their value)... */
		if (iter->ss_flags & MODSYM_FCONSTEXPR)
#endif
		{
			DeeObject *symbol_object;
			if unlikely(DeeModule_RunInit((DeeObject *)self) < 0)
				goto err_m1;
			if (self->mo_flags & MODULE_FDIDINIT) {
				rwlock_read(&self->mo_lock);
				if (iter->ss_flags & MODSYM_FPROPERTY) {
					/* Check which property operations have been bound. */
					if (self->mo_globalv[iter->ss_index + MODULE_PROPERTY_GET])
						perm |= ATTR_ACCESS_GET;
					if (!(iter->ss_flags & MODSYM_FREADONLY)) {
						/* These callbacks are only allocated if the READONLY flag isn't set. */
						if (self->mo_globalv[iter->ss_index + MODULE_PROPERTY_DEL])
							perm |= ATTR_ACCESS_DEL;
						if (self->mo_globalv[iter->ss_index + MODULE_PROPERTY_SET])
							perm |= ATTR_ACCESS_SET;
					}
				} else {
					symbol_object = self->mo_globalv[iter->ss_index];
					if (symbol_object) {
						attr_type = Dee_TYPE(symbol_object);
						Dee_Incref(attr_type);
					}
				}
				rwlock_endread(&self->mo_lock);
			}
		}
		doc_iter = iter;
		if (!doc_iter->ss_doc && (doc_iter->ss_flags & MODSYM_FALIAS)) {
			doc_iter = DeeModule_GetSymbolID(self, doc_iter->ss_index);
			ASSERT(doc_iter);
		}
		/* NOTE: Pass the module instance as declarator! */
		if (iter->ss_flags & MODSYM_FNAMEOBJ)
			perm |= ATTR_NAMEOBJ;
		if (doc_iter->ss_flags & MODSYM_FDOCOBJ)
			perm |= ATTR_DOCOBJ;
		temp = (*proc)((DeeObject *)self,
		               MODULE_SYMBOL_GETNAMESTR(iter),
		               doc_iter->ss_doc,
		               perm, attr_type, arg);
		Dee_XDecref(attr_type);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	temp = DeeObject_GenericEnumAttr(&DeeModule_Type, proc, arg);
	if unlikely(temp < 0)
		goto err;
	return result + temp;
err:
	return temp;
err_m1:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeModule_FindAttrString(DeeModuleObject *__restrict self,
                         struct attribute_info *__restrict result,
                         struct attribute_lookup_rules const *__restrict rules) {
	ASSERT_OBJECT(self);
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		if (DeeInteractiveModule_Check(self)) {
			/* TODO: Special handling for enumerating the globals of an interactive module. */
		}
		return 1;
	}
	if (!rules->alr_decl || rules->alr_decl == (DeeObject *)self) {
		struct module_symbol *sym, *doc_sym;
		sym = DeeModule_GetSymbolString(self,
		                                rules->alr_name,
		                                rules->alr_hash);
		if (sym) {
			uint16_t perm;
			DREF DeeTypeObject *attr_type;
			perm = ATTR_IMEMBER | ATTR_ACCESS_GET;
			ASSERT(sym->ss_index < self->mo_globalc);
			if (!(sym->ss_flags & MODSYM_FREADONLY))
				perm |= (ATTR_ACCESS_DEL | ATTR_ACCESS_SET);
			if (sym->ss_flags & MODSYM_FPROPERTY) {
				perm &= ~(ATTR_ACCESS_GET | ATTR_ACCESS_DEL | ATTR_ACCESS_SET);
				if (!(sym->ss_flags & MODSYM_FCONSTEXPR))
					perm |= ATTR_PROPERTY;
			}
			attr_type = NULL;
#if 0 /* Always allow this! (we allow it for user-classes, as well!) */
			/* For constant-expression symbols, we can predict
			 * their type (as well as their value)... */
			if (sym->ss_flags & MODSYM_FCONSTEXPR)
#endif
			{
				DeeObject *symbol_object;
				if unlikely(DeeModule_RunInit((DeeObject *)self) < 0)
					goto err;
				if (self->mo_flags & MODULE_FDIDINIT) {
					rwlock_read(&self->mo_lock);
					if (sym->ss_flags & MODSYM_FPROPERTY) {
						/* Check which property operations have been bound. */
						if (self->mo_globalv[sym->ss_index + MODULE_PROPERTY_GET])
							perm |= ATTR_ACCESS_GET;
						if (!(sym->ss_flags & MODSYM_FREADONLY)) {
							/* These callbacks are only allocated if the READONLY flag isn't set. */
							if (self->mo_globalv[sym->ss_index + MODULE_PROPERTY_DEL])
								perm |= ATTR_ACCESS_DEL;
							if (self->mo_globalv[sym->ss_index + MODULE_PROPERTY_SET])
								perm |= ATTR_ACCESS_SET;
						}
					} else {
						symbol_object = self->mo_globalv[sym->ss_index];
						if (symbol_object) {
							attr_type = Dee_TYPE(symbol_object);
							Dee_Incref(attr_type);
						}
					}
					rwlock_endread(&self->mo_lock);
				}
			}
			doc_sym = sym;
			if (!doc_sym->ss_doc && (doc_sym->ss_flags & MODSYM_FALIAS)) {
				doc_sym = DeeModule_GetSymbolID(self, doc_sym->ss_index);
				ASSERT(doc_sym);
			}
			if (doc_sym->ss_flags & MODSYM_FDOCOBJ)
				perm |= ATTR_DOCOBJ;
			if ((perm & rules->alr_perm_mask) == rules->alr_perm_value) {
				/* NOTE: Pass the module instance as declarator! */
				result->a_decl     = (DREF DeeObject *)self;
				result->a_doc      = MODULE_SYMBOL_GETDOCSTR(doc_sym);
				result->a_perm     = perm;
				result->a_attrtype = attr_type; /* Inherit reference. */
				if (perm & ATTR_DOCOBJ)
					Dee_Incref(Dee_attribute_info_docobj(result));
				Dee_Incref(self);
				return 0;
			}
			Dee_XDecref(attr_type);
		}
	}
	return DeeObject_GenericFindAttrString(&DeeModule_Type,
	                                       (DeeObject *)self,
	                                       result,
	                                       rules);
err:
	return -1;
}

PRIVATE struct type_attr module_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&module_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&module_delattr,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&module_setattr,
	/* .tp_enumattr = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&module_enumattr
};

PRIVATE NONNULL((1)) int DCALL
err_module_not_fully_loaded(DeeModuleObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Module %k has not been fully loaded, yet",
	                       self->mo_name);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_code(DeeModuleObject *__restrict self) {
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		err_module_not_fully_loaded(self);
		return NULL;
	}
	return_reference_((DREF DeeObject *)self->mo_root);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_path(DeeModuleObject *__restrict self) {
	if (!(self->mo_flags & MODULE_FDIDLOAD)) {
		err_module_not_fully_loaded(self);
		return NULL;
	}
	if (!self->mo_path)
		return_none;
	return_reference_((DREF DeeObject *)self->mo_path);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_isglobal(DeeModuleObject *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return_bool(self->mo_globpself != NULL);
#else /* CONFIG_NO_THREADS */
	return_bool(ATOMIC_READ(self->mo_globpself) != NULL);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_haspath(DeeModuleObject *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return_bool(self->mo_pself != NULL);
#else /* CONFIG_NO_THREADS */
	return_bool(ATOMIC_READ(self->mo_pself) != NULL);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
module_get_imports(DeeModuleObject *__restrict self) {
	return DeeRefVector_NewReadonly((DeeObject *)self, self->mo_importc,
	                                (DeeObject **)self->mo_importv);
}

PRIVATE struct type_member module_members[] = {
	TYPE_MEMBER_FIELD_DOC("__name__", STRUCT_OBJECT, offsetof(DeeModuleObject, mo_name),
	                      "->?Dstring\n"
	                      "The name of @this Module"),
	TYPE_MEMBER_END
};

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_ViewExports(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_ViewGlobals(DeeObject *__restrict self);

PRIVATE struct type_getset module_getsets[] = {
	{ "__exports__", &DeeModule_ViewExports, NULL, NULL,
	  DOC("->?S?T2?Dstring?O\n"
	      "Returns a modifiable mapping-like object containing @this "
	      "Module's global variables accessible by name (and enumerable)\n"
	      "Note that only existing exports can be modified, however no new symbols can be added:\n"
	      "${"
	      "import util;\n"
	      "print util.min;                /* function */\n"
	      "print util.__exports__[\"min\"]; /* function */\n"
	      "del util.min;\n"
	      "assert \"min\" !in util.__exports__;\n"
	      "util.__exports__[\"min\"] = 42;\n"
	      "print util.min;                /* 42 */"
	      "}") },
	{ "__imports__", (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_imports, NULL, NULL,
	  DOC("->?S?DModule\n"
	      "Returns an immutable sequence of all other modules imported by this one") },
	{ "__globals__", &DeeModule_ViewGlobals, NULL, NULL,
	  DOC("->?S?O\n"
	      "Similar to ?#__exports__, however global variables are addressed using their "
	      "internal index. Using this, anonymous global variables (such as property callbacks) "
	      "can be accessed and modified") },
	{ "__code__",
	  (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_code, NULL, NULL,
	  DOC("->?Ert:Code\n"
	      "@throw ValueError The Module hasn't been fully loaded\n"
	      "Returns the code object for the Module's root initializer") },
	{ "__path__",
	  (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_path, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "@throw ValueError The Module hasn't been fully loaded\n"
	      "Returns the absolute filesystem path of the Module's source file, or ?N "
	      "if the Module wasn't created from a file accessible via the filesystem") },
	{ "__isglobal__",
	  (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_isglobal, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this Module is global (i.e. can be accessed as ${import(this.__name__)})") },
	{ "__haspath__",
	  (DREF DeeObject *(DCALL *)(DREF DeeObject *__restrict))&module_get_haspath, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns ?t if @this Module has a path found within the filesystem") },
	{ NULL }
};

PRIVATE WUNUSED DREF DeeObject *DCALL
module_class_getpath(DeeObject *__restrict UNUSED(self)) {
	return_reference(DeeModule_GetPath());
}

PRIVATE int DCALL
module_class_setpath(DeeObject *__restrict UNUSED(self),
                     DeeObject *__restrict value) {
	return DeeObject_Assign(DeeModule_GetPath(), value);
}

PRIVATE struct type_getset module_class_getsets[] = {
	{ "path", &module_class_getpath, NULL, &module_class_setpath,
	  DOC("->?DList\n"
	      "A list of strings describing the search path for system libraries") },
	/* Deprecated aliases to emulate the old `dexmodule' builtin type. */
	{ "search_path", &module_class_getpath, NULL, &module_class_setpath,
	  DOC("->?DList\n"
	      "Deprecated alias for ?#path") },
	{ NULL }
};

PRIVATE WUNUSED DREF DeeObject *DCALL
module_class_open(DeeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	/* This is pretty much the same as the builtin `import()' function.
	 * The only reason it exist is to be a deprecated alias for backwards
	 * compatibility with the old deemon. */
	DeeObject *module_name;
	if (DeeArg_Unpack(argc, argv, "o:open", &module_name) ||
	    DeeObject_AssertTypeExact(module_name, &DeeString_Type))
		return NULL;
	return DeeModule_Import(module_name, NULL, true);
}


PRIVATE struct type_method module_class_methods[] = {
	/* Deprecated aliases to emulate the old `dexmodule' builtin type. */
	{ "open", &module_class_open,
	  DOC("(name:?Dstring)->?DModule\n"
	      "Deprecated alias for :import") },
	{ NULL }
};


INTDEF NONNULL((1)) void DCALL module_unbind(DeeModuleObject *__restrict self);

PRIVATE NONNULL((1)) void DCALL
module_fini(DeeModuleObject *__restrict self) {
	struct module_symbol *iter, *end;
	DREF DeeModuleObject **miter, **mend;
	size_t i;
	weakref_support_fini(self);
	module_unbind(self);
	Dee_Decref(self->mo_name);
	Dee_XDecref(self->mo_root);
	Dee_XDecref(self->mo_path);
	if (self->mo_flags & MODULE_FDIDLOAD) {
		for (i = 0; i < self->mo_globalc; ++i)
			Dee_XDecref(self->mo_globalv[i]);
		Dee_Free(self->mo_globalv);
	}
	if (self->mo_bucketv != empty_module_buckets) {
		end = (iter = self->mo_bucketv) + self->mo_bucketm + 1;
		for (; iter != end; ++iter) {
			if (!MODULE_SYMBOL_GETNAMESTR(iter))
				continue;
			if (iter->ss_flags & MODSYM_FNAMEOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(MODULE_SYMBOL_GETNAMESTR(iter), DeeStringObject, s_str));
			if (iter->ss_flags & MODSYM_FDOCOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(iter->ss_doc, DeeStringObject, s_str));
		}
		Dee_Free((void *)self->mo_bucketv);
	}

	mend = (miter = (DREF DeeModuleObject **)self->mo_importv) + self->mo_importc;
	for (; miter != mend; ++miter)
		Dee_Decref(*miter);
	Dee_Free((void *)self->mo_importv);
}

PRIVATE NONNULL((1, 2)) void DCALL
module_visit(DeeModuleObject *__restrict self,
             dvisit_t proc, void *arg) {
	size_t i;
	rwlock_read(&self->mo_lock);

	/* Visit the root-code object. */
	Dee_XVisit(self->mo_root);

	Dee_Visit(self->mo_name);
	Dee_XVisit(self->mo_path);

	/* Visit global variables. */
	if (self->mo_flags & MODULE_FDIDLOAD) {
		for (i = 0; i < self->mo_globalc; ++i)
			Dee_XVisit(self->mo_globalv[i]);
	}

	rwlock_endread(&self->mo_lock);
	/* Visit imported modules. */
	for (i = 0; i < self->mo_importc; ++i)
		Dee_XVisit(self->mo_importv[i]);
}

PRIVATE NONNULL((1)) void DCALL
module_clear(DeeModuleObject *__restrict self) {
	DREF DeeObject *buffer[16];
	DREF DeeCodeObject *root_code;
	size_t i, bufi = 0;
	rwlock_write(&self->mo_lock);
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
				rwlock_endwrite(&self->mo_lock);
				Dee_Decrefv(buffer, bufi);
				bufi = 0;
				rwlock_write(&self->mo_lock);
				goto restart;
			}
		}
	}
	rwlock_endwrite(&self->mo_lock);
	Dee_Decrefv(buffer, bufi);
	Dee_XDecref(root_code);
}

PRIVATE void DCALL
module_pclear(DeeModuleObject *__restrict self,
              unsigned int priority) {
	size_t i;
	rwlock_write(&self->mo_lock);
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
		rwlock_endwrite(&self->mo_lock);
		Dee_Decref(ob);
		rwlock_write(&self->mo_lock);
	}
	rwlock_endwrite(&self->mo_lock);
}


INTERN struct type_gc module_gc = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&module_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&module_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_MODULE
};



/* Single module objects are unique, comparing/hashing
 * them is as single as comparing their memory locations. */
PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
module_hash(DeeModuleObject *__restrict self) {
	return DeeObject_HashGeneric(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_eq(DeeModuleObject *self,
          DeeModuleObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, &DeeModule_Type))
		return NULL;
	return_bool_(self == other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_ne(DeeModuleObject *self,
          DeeModuleObject *other) {
	if (DeeObject_AssertType((DeeObject *)other, &DeeModule_Type))
		return NULL;
	return_bool_(self != other);
}

PRIVATE struct type_cmp module_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&module_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&module_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&module_ne
};

PUBLIC DeeTypeObject DeeModule_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Module),
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR_GC(DeeModuleObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&module_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&module_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&module_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&module_visit,
	/* .tp_gc            = */ &module_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &module_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ &module_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ module_getsets,
	/* .tp_members       = */ module_members,
	/* .tp_class_methods = */ module_class_methods,
	/* .tp_class_getsets = */ module_class_getsets,
	/* .tp_class_members = */ NULL
};

INTERN struct static_module_struct empty_module_head = {
	{
		/* ... */
		NULL,
		NULL
	}, {
		OBJECT_HEAD_INIT(&DeeModule_Type),
		/* .mo_name      = */ (DeeStringObject *)Dee_EmptyString,
		/* .mo_pself     = */ NULL,
		/* .mo_next      = */ NULL,
		/* .mo_path      = */ NULL,
#ifdef CONFIG_HOST_WINDOWS
		/* .mo_pathhash  = */ 0,
#endif /* CONFIG_HOST_WINDOWS */
		/* .mo_globpself = */ NULL,
		/* .mo_globnext  = */ NULL,
		/* .mo_importc   = */ 0,
		/* .mo_globalc   = */ 0,
#ifndef CONFIG_NO_DEC
		/* .mo_flags     = */ MODULE_FDIDINIT | MODULE_FDIDLOAD | MODULE_FHASCTIME,
#else /* !CONFIG_NO_DEC */
		/* .mo_flags     = */ MODULE_FDIDINIT | MODULE_FDIDLOAD,
#endif /* CONFIG_NO_DEC */
		/* .mo_bucketm   = */ 0,
		/* .mo_bucketv   = */ empty_module_buckets,
		/* .mo_importv   = */ NULL,
		/* .mo_globalv   = */ NULL,
		/* .mo_root      = */ &empty_code,
#ifndef CONFIG_NO_THREADS
		/* .mo_lock      = */ RWLOCK_INIT,
		/* .mo_loader    = */ NULL,
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEC
		/* .mo_ctime     = */ 0,
#endif /* !CONFIG_NO_DEC */
		WEAKREF_SUPPORT_INIT
	}
};


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_C */
