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
#ifndef GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C
#define GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
/**/

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN


typedef struct {
	OBJECT_HEAD
	DREF DeeModuleObject *mei_module; /* [1..1][const] The module who's exports are being iterated. */
	DWEAK uint16_t        mei_index;  /* The current global variable index. */
} ModuleExportsIterator;

typedef struct {
	OBJECT_HEAD
	DREF DeeModuleObject *me_module;  /* [1..1][const] The module who's exports are being viewed. */
} ModuleExports;

INTDEF DeeTypeObject ModuleExports_Type;
INTDEF DeeTypeObject ModuleExportsIterator_Type;
INTDEF DeeTypeObject ModuleGlobals_Type;
INTDEF WUNUSED NONNULL((1)) DREF ModuleExports *DCALL
DeeModule_ViewExports(DeeModuleObject *__restrict self);


PRIVATE WUNUSED NONNULL((1)) int DCALL
modexportsiter_ctor(ModuleExportsIterator *__restrict self) {
	self->mei_index  = 0;
	self->mei_module = &DeeModule_Empty;
	Dee_Incref(&DeeModule_Empty);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexportsiter_copy(ModuleExportsIterator *__restrict self,
                    ModuleExportsIterator *__restrict other) {
	self->mei_index  = atomic_read(&other->mei_index);
	self->mei_module = other->mei_module;
	Dee_Incref(self->mei_module);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexportsiter_init(ModuleExportsIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	ModuleExports *exports_map;
	if (DeeArg_Unpack(argc, argv, "o:_ModuleExportsIterator", &exports_map))
		goto err;
	if (DeeObject_AssertTypeExact(exports_map, &ModuleExports_Type))
		goto err;
	self->mei_index  = 0;
	self->mei_module = exports_map->me_module;
	Dee_Incref(self->mei_module);
	return 0;
err:
	return -1;
}


PRIVATE NONNULL((1)) void DCALL
modexportsiter_fini(ModuleExportsIterator *__restrict self) {
	Dee_Decref_unlikely(self->mei_module);
}

PRIVATE NONNULL((1, 2)) void DCALL
modexportsiter_visit(ModuleExportsIterator *__restrict self,
                     dvisit_t proc, void *arg) {
	Dee_Visit(self->mei_module);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexportsiter_compare(ModuleExportsIterator *lhs,
                       ModuleExportsIterator *rhs) {
	if (DeeObject_AssertTypeExact(rhs, Dee_TYPE(lhs)))
		goto err;
	Dee_return_compareT(uint16_t, atomic_read(&lhs->mei_index),
	                    /*     */ atomic_read(&rhs->mei_index));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp modexportsiter_cmp = {
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexportsiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
module_it_getattr_symbol(DeeModuleObject *__restrict self,
                         struct module_symbol *__restrict symbol) {
	DREF DeeObject *result;
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
read_symbol:
		ASSERT(symbol->ss_index < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[symbol->ss_index];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result)
			result = ITER_DONE; /* Unbound */
		return result;
	}
	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return ITER_DONE;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexportsiter_next(ModuleExportsIterator *__restrict self) {
	DREF DeeObject *result, *result_name, *result_value;
	DeeModuleObject *mod = self->mei_module;
	uint16_t old_index, new_index;
	if (DeeModule_LockSymbols(mod))
		goto err;
again:
	for (;;) {
		struct module_symbol *symbol;
		old_index = atomic_read(&self->mei_index);
		if (old_index > mod->mo_bucketm) {
			DeeModule_UnlockSymbols(mod);
			return ITER_DONE;
		}
		new_index = old_index;
		for (;;) {
			symbol = &mod->mo_bucketv[new_index];
			if (symbol->ss_name) {
				result_name = (DREF DeeObject *)module_symbol_getnameobj(symbol);
				if unlikely(!result_name) {
					DeeModule_UnlockSymbols(mod);
					return NULL;
				}
				break;
			}
continue_symbol_search:
			++new_index;
			if (new_index > mod->mo_bucketm) {
				if (!atomic_cmpxch_weak(&self->mei_index, old_index, new_index))
					goto again;
				DeeModule_UnlockSymbols(mod);
				return ITER_DONE;
			}
		}
		/* Read the current value of this symbol. */
		result_value = module_it_getattr_symbol(mod, symbol);
		if (!ITER_ISOK(result_value)) {
			if unlikely(!result_value)
				goto err;
			goto continue_symbol_search;
		}
		if (atomic_cmpxch_weak(&self->mei_index, old_index, new_index + 1))
			break;
		Dee_Decref_unlikely(result_value);
	}
	Dee_Incref(result_name);
	DeeModule_UnlockSymbols(mod);
	result = DeeTuple_PackSymbolic(2, result_name, result_value);
	if unlikely(!result) {
		Dee_Decref_unlikely(result_name);
		Dee_Decref_unlikely(result_value);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF ModuleExports *DCALL
modexportsiter_getseq(ModuleExportsIterator *__restrict self) {
	return DeeModule_ViewExports(self->mei_module);
}

PRIVATE struct type_getset tpconst modexportsiter_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &modexportsiter_getseq,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Ert:ModuleExports"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject ModuleExportsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleExportsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&modexportsiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&modexportsiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&modexportsiter_init,
				TYPE_FIXED_ALLOCATOR(ModuleExportsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&modexportsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&modexportsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &modexportsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexportsiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ modexportsiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__E31EBEB26CC72F83),
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_ctor(ModuleExports *__restrict self) {
	self->me_module = &DeeModule_Empty;
	Dee_Incref(&DeeModule_Empty);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_init(ModuleExports *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ModuleExports", &self->me_module))
		goto err;
	if (DeeObject_AssertType(self->me_module, &DeeModule_Type))
		goto err;
	Dee_Incref(&DeeModule_Empty);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_module) ==
              offsetof(ModuleExports, me_module));
#define modexports_fini  modexportsiter_fini
#define modexports_visit modexportsiter_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_bool(ModuleExports *__restrict self) {
	return self->me_module->mo_globalc;
}

PRIVATE WUNUSED NONNULL((1)) DREF ModuleExportsIterator *DCALL
modexports_iter(ModuleExports *__restrict self) {
	DREF ModuleExportsIterator *result;
	result = DeeObject_MALLOC(ModuleExportsIterator);
	if unlikely(!result)
		goto done;
	result->mei_module = self->me_module;
	result->mei_index  = 0;
	Dee_Incref(result->mei_module);
	DeeObject_Init(result, &ModuleExportsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
modexports_size(ModuleExports *__restrict self) {
	size_t i, total_symbols = 0;
	DeeModuleObject *mod = self->me_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	for (i = 0; i <= mod->mo_bucketm; ++i) {
		if (mod->mo_bucketv[i].ss_name)
			++total_symbols;
	}
	DeeModule_UnlockSymbols(mod);
	return total_symbols;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeModule_GetAttrSymbol_asitem(ModuleExports *exports_map,
                               DeeModuleObject *self,
                               struct module_symbol *symbol) {
	DREF DeeObject *result;
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
read_symbol:
		ASSERT(symbol->ss_index < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[symbol->ss_index];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result) {
			if (symbol->ss_flags & MODSYM_FNAMEOBJ) {
				err_unknown_key((DeeObject *)exports_map,
				                (DeeObject *)COMPILER_CONTAINER_OF(symbol->ss_name, DeeStringObject, s_str));
			} else {
				err_unknown_key_str((DeeObject *)exports_map, symbol->ss_name);
			}
		}
		return result;
	}
	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback) {
			if (symbol->ss_flags & MODSYM_FNAMEOBJ) {
				err_unknown_key((DeeObject *)exports_map,
				                (DeeObject *)COMPILER_CONTAINER_OF(symbol->ss_name, DeeStringObject, s_str));
			} else {
				err_unknown_key_str((DeeObject *)exports_map, symbol->ss_name);
			}
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_TryGetAttrSymbol_asitem(DeeModuleObject *self,
                                  struct module_symbol *symbol) {
	DREF DeeObject *result;
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
read_symbol:
		ASSERT(symbol->ss_index < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[symbol->ss_index];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result)
			return ITER_DONE;
		return result;
	}
	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return ITER_DONE;
		/* Invoke the property callback. */
		result = DeeObject_Call(callback, 0, NULL);
		Dee_Decref(callback);
		if unlikely(!result) {
			if (DeeError_Catch(&DeeError_UnboundAttribute) ||
			    DeeError_Catch(&DeeError_UnboundLocal) ||
			    DeeError_Catch(&DeeError_UnboundItem))
				return ITER_DONE;
		}
		return result;
	}
	/* External symbol. */
	ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
	self = self->mo_importv[symbol->ss_extern.ss_impid];
	goto read_symbol;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrSymbol_asitem(DeeModuleObject *self,
                                 struct module_symbol *symbol) {
	DREF DeeObject *result;
	if likely(!(symbol->ss_flags & (MODSYM_FEXTERN | MODSYM_FPROPERTY))) {
read_symbol:
		ASSERT(symbol->ss_index < self->mo_globalc);
		return Dee_BOUND_FROMBOOL(atomic_read(&self->mo_globalv[symbol->ss_index]));
	}

	/* External symbol, or property. */
	if (symbol->ss_flags & MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[symbol->ss_index + MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return Dee_BOUND_NO;
		/* Invoke the property callback. */
		result = DeeObject_Call(callback, 0, NULL);
		Dee_Decref(callback);
		if (result) {
			Dee_Decref(callback);
			return Dee_BOUND_YES;
		}
		if (DeeError_Catch(&DeeError_UnboundAttribute) ||
		    DeeError_Catch(&DeeError_UnboundLocal) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			return Dee_BOUND_NO;
		return Dee_BOUND_ERR;
	}
	/* External symbol. */
	ASSERT(symbol->ss_extern.ss_impid < self->mo_importc);
	self = self->mo_importv[symbol->ss_extern.ss_impid];
	goto read_symbol;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_getitem(ModuleExports *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeString_Check(key)) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err;
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_bounditem(ModuleExports *self, DeeObject *key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeString_Check(key)) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err;
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_hasitem(ModuleExports *self, DeeObject *key) {
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeString_Check(key)) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err;
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!symbol)
		goto err_nokey_unlock;
	DeeModule_UnlockSymbols(mod);
	return 1;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_trygetitem(ModuleExports *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeString_Check(key)) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err;
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_delitem(ModuleExports *self, DeeObject *key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeString_Check(key)) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err;
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
modexports_setitem(ModuleExports *self, DeeObject *key, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeString_Check(key)) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err;
		if (DeeModule_LockSymbols(mod))
			goto err;
		symbol = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, symbol, value);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexports_getitem_index(ModuleExports *self, size_t key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
err_nokey:
	err_unknown_key_int((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_bounditem_index(ModuleExports *self, size_t key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
err_nokey:
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_hasitem_index(ModuleExports *self, size_t key) {
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	DeeModule_UnlockSymbols(mod);
	return 1;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
err_nokey:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexports_trygetitem_index(ModuleExports *self, size_t key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
err_nokey:
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_delitem_index(ModuleExports *self, size_t key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
err_nokey:
	err_unknown_key_int((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
modexports_setitem_index(ModuleExports *self, size_t key, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, symbol, value);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
err_nokey:
	err_unknown_key_int((DeeObject *)self, key);
err:
	return -1;
}




PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_getitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key_str((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_bounditem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_hasitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	DeeModule_UnlockSymbols(mod);
	return 1;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_trygetitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_delitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key_str((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
modexports_setitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, symbol, value);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key_str((DeeObject *)self, key);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_getitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_bounditem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_hasitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	DeeModule_UnlockSymbols(mod);
	return 1;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_trygetitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_delitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
modexports_setitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!symbol)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, symbol, value);
	DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	DeeModule_UnlockSymbols(mod);
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
modexports_mh_map_enumerate(ModuleExports *self, Dee_seq_enumerate_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeModuleObject *mod = self->me_module;
	Dee_hash_t i;
	if (DeeModule_LockSymbols(mod))
		goto err;
	for (i = 0; i <= mod->mo_bucketm; ++i) {
		DREF DeeStringObject *item_name;
		DREF DeeObject *item_value;
		struct module_symbol *item = &mod->mo_bucketv[i];
		if (!item->ss_name)
			continue;
		item_name = module_symbol_getnameobj(item);
		if unlikely(!item_name)
			goto err_unlock;
		item_value = DeeModule_TryGetAttrSymbol_asitem(mod, item);
		if likely(ITER_ISOK(item_value)) {
			temp = (*proc)(arg, (DeeObject *)item_name, item_value);
			Dee_Decref(item_value);
		} else if (item_value == ITER_DONE) {
			temp = (*proc)(arg, (DeeObject *)item_name, NULL);
		} else {
			temp = -1;
		}
		Dee_Decref(item_name);
		if unlikely(temp < 0)
			goto err_temp_unlock;
		result += temp;
	}
	DeeModule_UnlockSymbols(mod);
	return result;
err_temp_unlock:
	DeeModule_UnlockSymbols(mod);
	return temp;
err_unlock:
	DeeModule_UnlockSymbols(mod);
err:
	return -1;
}

PRIVATE struct type_method tpconst modexports_methods[] = {
	TYPE_METHOD_HINTREF(__map_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst modexports_method_hints[] = {
	TYPE_METHOD_HINT(map_enumerate, &modexports_mh_map_enumerate),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq modexports_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexports_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__map_operator_contains__with__map_operator_bounditem),
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&modexports_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexports_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&modexports_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexports_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexports_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&modexports_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&modexports_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&modexports_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&modexports_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&modexports_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&modexports_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&modexports_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&modexports_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&modexports_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&modexports_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&modexports_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&modexports_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&modexports_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&modexports_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&modexports_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&modexports_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&modexports_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&modexports_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&modexports_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&modexports_hasitem_string_len_hash,
};

PRIVATE struct type_member tpconst modexports_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___module__, STRUCT_OBJECT, offsetof(ModuleExports, me_module), "->?DModule"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst modexports_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ModuleExportsIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ModuleExports_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleExports",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&modexports_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&modexports_init,
				TYPE_FIXED_ALLOCATOR(ModuleExports)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&modexports_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&modexports_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&modexports_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__56685E2B01B76756),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__C6AA9DC8372C283F),
	/* .tp_seq           = */ &modexports_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ modexports_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ modexports_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ modexports_class_members,
	/* .tp_method_hints  = */ modexports_method_hints,
};


INTERN WUNUSED NONNULL((1)) DREF ModuleExports *DCALL
DeeModule_ViewExports(DeeModuleObject *__restrict self) {
	DREF ModuleExports *result;
	result = DeeObject_MALLOC(ModuleExports);
	if unlikely(!result)
		goto done;
	result->me_module = self;
	Dee_Incref(self);
	DeeObject_Init(result, &ModuleExports_Type);
done:
	return result;
}





typedef struct {
	OBJECT_HEAD
	DREF DeeModuleObject *mg_module; /* [1..1] The module who's exports are being viewed. */
} ModuleGlobals;

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_init(ModuleGlobals *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ModuleGlobals", &self->mg_module))
		goto err;
	if (DeeObject_AssertType(self->mg_module, &DeeModule_Type))
		goto err;
	Dee_Incref(&DeeModule_Empty);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleExports, me_module) == offsetof(ModuleGlobals, mg_module));
STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_module) == offsetof(ModuleGlobals, mg_module));
#define modglobals_ctor    modexports_ctor
#define modglobals_fini    modexportsiter_fini
#define modglobals_bool    modexports_bool
#define modglobals_visit   modexportsiter_visit
#define modglobals_members modexports_members


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
modglobals_size(ModuleGlobals *__restrict self) {
	return atomic_read(&self->mg_module->mo_globalc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modglobals_getitem_index(ModuleGlobals *self, size_t index) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->mg_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	if (index >= mod->mo_globalc) {
		size_t num_globals = mod->mo_globalc;
		DeeModule_UnlockSymbols(mod);
		err_index_out_of_bounds((DeeObject *)self, index, num_globals);
		goto err;
	}
	DeeModule_LockRead(mod);
	result = mod->mo_globalv[index];
	if unlikely(!result) {
		DeeModule_LockEndRead(mod);
		DeeModule_UnlockSymbols(mod);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(result);
	DeeModule_LockEndRead(mod);
	DeeModule_UnlockSymbols(mod);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_setitem_index(ModuleGlobals *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeModuleObject *mod = self->mg_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	if (index >= mod->mo_globalc) {
		size_t num_globals = mod->mo_globalc;
		DeeModule_UnlockSymbols(mod);
		err_index_out_of_bounds((DeeObject *)self, index, num_globals);
		goto err;
	}
	DeeModule_LockWrite(mod);
	Dee_XIncref(value);
	oldvalue = mod->mo_globalv[index];
	mod->mo_globalv[index] = value;
	DeeModule_LockEndWrite(mod);
	DeeModule_UnlockSymbols(mod);
	Dee_XDecref(oldvalue);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_bounditem_index(ModuleGlobals *self, size_t index) {
	DeeObject *value;
	DeeModuleObject *mod = self->mg_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	if (index >= mod->mo_globalc) {
		DeeModule_UnlockSymbols(mod);
		return Dee_BOUND_MISSING;
	}
	DeeModule_LockRead(mod);
	value = mod->mo_globalv[index];
	DeeModule_LockEndRead(mod);
	DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_FROMBOOL(value);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_hasitem_index(ModuleGlobals *self, size_t index) {
	DeeModuleObject *mod = self->mg_module;
	return index < atomic_read(&mod->mo_globalc) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
modglobals_xchitem_index(ModuleGlobals *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeModuleObject *mod = self->mg_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	if (index >= mod->mo_globalc) {
		size_t num_globals = mod->mo_globalc;
		DeeModule_UnlockSymbols(mod);
		err_index_out_of_bounds((DeeObject *)self, index, num_globals);
		goto err;
	}
	DeeModule_LockWrite(mod);
	oldvalue = mod->mo_globalv[index];
	if unlikely(!oldvalue) {
		DeeModule_UnlockSymbols(mod);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	mod->mo_globalv[index] = value;
	DeeModule_LockEndWrite(mod);
	DeeModule_UnlockSymbols(mod);
	return oldvalue;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_delitem_index(ModuleGlobals *self, size_t index) {
	return modglobals_setitem_index(self, index, NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
modglobals_mh_seq_enumerate_index(ModuleGlobals *__restrict self,
                           Dee_seq_enumerate_index_t proc,
                           void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *item;
	DeeModuleObject *mod = self->mg_module;
	for (; start < end; ++start) {
		if (DeeModule_LockSymbols(mod))
			goto err;
		if (end > mod->mo_globalc) {
			end = mod->mo_globalc;
			if (start >= end) {
				DeeModule_UnlockSymbols(mod);
				break;
			}
		}
		DeeModule_LockRead(mod);
		item = mod->mo_globalv[start];
		Dee_XIncref(item);
		DeeModule_LockEndRead(mod);
		DeeModule_UnlockSymbols(mod);
		temp = (*proc)(arg, start, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE struct type_method tpconst modglobals_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst modglobals_method_hints[] = {
	TYPE_METHOD_HINT(seq_xchitem_index, &modglobals_xchitem_index),
	TYPE_METHOD_HINT(seq_enumerate_index, &modglobals_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq modglobals_seq = {
	/* .tp_iter               = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&modglobals_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&modglobals_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&modglobals_getitem_index,
	/* .tp_getitem_index_fast = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&modglobals_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&modglobals_setitem_index,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&modglobals_bounditem_index,
	/* .tp_hasitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&modglobals_hasitem_index,
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

INTERN DeeTypeObject ModuleGlobals_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleGlobals",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&modglobals_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&modglobals_init,
				TYPE_FIXED_ALLOCATOR(ModuleExports)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&modglobals_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&modglobals_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&modglobals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &modglobals_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ modglobals_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ modglobals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ modglobals_method_hints,
};


INTERN WUNUSED NONNULL((1)) DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self) {
	DREF ModuleGlobals *result;
	result = DeeObject_MALLOC(ModuleGlobals);
	if unlikely(!result)
		goto done;
	result->mg_module = self;
	Dee_Incref(self);
	DeeObject_Init(result, &ModuleGlobals_Type);
done:
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C */
