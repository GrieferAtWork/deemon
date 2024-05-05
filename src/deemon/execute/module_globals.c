/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C
#define GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

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
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexportsiter_compare,
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
	TYPE_GETTER_F(STR_seq, &modexportsiter_getseq,
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&modexportsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &modexportsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexportsiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ modexportsiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexports_size(ModuleExports *__restrict self) {
	size_t i, total_symbols = 0;
	DeeModuleObject *mod = self->me_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	for (i = 0; i <= mod->mo_bucketm; ++i)
		if (mod->mo_bucketv[i].ss_name)
			++total_symbols;
	DeeModule_UnlockSymbols(mod);
	return DeeInt_NewSize(total_symbols);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_contains(ModuleExports *self, DeeObject *key) {
	bool result;
	DeeModuleObject *mod = self->me_module;
	if (!DeeString_Check(key)) /* TODO: Also allow gid:?Dint instead of only ?Dstring */
		return_false;
	if (DeeModule_LockSymbols(mod))
		goto err;
	result = DeeModule_GetSymbol(mod, key) != NULL;
	DeeModule_UnlockSymbols(mod);
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
module_my_getattr_symbol(ModuleExports *__restrict exports_map,
                         DeeModuleObject *__restrict self,
                         struct module_symbol *__restrict symbol) {
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
modexports_get(ModuleExports *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (!DeeString_Check(key)) /* TODO: Also allow gid:?Dint instead of only ?Dstring */
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbol(mod, key);
	if unlikely(!symbol)
		goto err_unknown_key_unlock;
	result = module_my_getattr_symbol(self, mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_unknown_key_unlock:
	DeeModule_UnlockSymbols(mod);
err_unknown_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexports_get_f(ModuleExports *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	DeeObject *defl = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &defl))
		goto err;
	if (!DeeString_Check(key)) /* TODO: Also allow gid:?Dint instead of only ?Dstring */
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbol(mod, key);
	if unlikely(!symbol)
		goto err_unknown_key_unlock;
	result = module_it_getattr_symbol(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	if (result == ITER_DONE) {
		result = defl;
		Dee_Incref(defl);
	}
	return result;
err_unknown_key_unlock:
	DeeModule_UnlockSymbols(mod);
err_unknown_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_del(ModuleExports *__restrict self,
               DeeObject *__restrict key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (!DeeString_Check(key)) /* TODO: Also allow gid:?Dint instead of only ?Dstring */
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbol(mod, key);
	if unlikely(!symbol)
		goto err_unknown_key_unlock;
	result = DeeModule_DelAttrSymbol(mod, symbol);
	DeeModule_UnlockSymbols(mod);
	return result;
err_unknown_key_unlock:
	DeeModule_UnlockSymbols(mod);
err_unknown_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
modexports_set(ModuleExports *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (!DeeString_Check(key)) /* TODO: Also allow gid:?Dint instead of only ?Dstring */
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbol(mod, key);
	if unlikely(!symbol)
		goto err_unknown_key_unlock;
	result = DeeModule_SetAttrSymbol(mod, symbol, value);
	DeeModule_UnlockSymbols(mod);
	return result;
err_unknown_key_unlock:
	DeeModule_UnlockSymbols(mod);
err_unknown_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return -1;
}

PRIVATE struct type_seq modexports_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexports_iter,
	/* .tp_sizeob       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexports_size,
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&modexports_contains,
	/* .tp_getitem      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&modexports_get,
	/* .tp_delitem      = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexports_del,
	/* .tp_setitem      = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&modexports_set,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_nsi          = */ NULL,
	/* .tp_foreach      = */ NULL,
	/* .tp_foreach_pair = */ NULL, /* TODO */
};

DOC_REF(map_get_doc);

PRIVATE struct type_method tpconst modexports_methods[] = {
	TYPE_METHOD(STR_get, &modexports_get_f, DOC_GET(map_get_doc)),
	TYPE_METHOD_END
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&modexports_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&modexports_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &modexports_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ modexports_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ modexports_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ modexports_class_members
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
	DREF DeeModuleObject *modglobals_module; /* [1..1] The module who's exports are being viewed. */
} ModuleGlobals;

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_init(ModuleGlobals *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ModuleGlobals", &self->modglobals_module))
		goto err;
	if (DeeObject_AssertType(self->modglobals_module, &DeeModule_Type))
		goto err;
	Dee_Incref(&DeeModule_Empty);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleExports, me_module) == offsetof(ModuleGlobals, modglobals_module));
STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_module) == offsetof(ModuleGlobals, modglobals_module));
#define modglobals_ctor    modexports_ctor
#define modglobals_fini    modexportsiter_fini
#define modglobals_bool    modexports_bool
#define modglobals_visit   modexportsiter_visit
#define modglobals_members modexports_members


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
modglobals_nsi_getsize(ModuleGlobals *__restrict self) {
	return atomic_read(&self->modglobals_module->mo_globalc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modglobals_nsi_getitem(ModuleGlobals *self, size_t index) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->modglobals_module;
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
modglobals_nsi_setitem(ModuleGlobals *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeModuleObject *mod = self->modglobals_module;
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
modglobals_nsi_bounditem(ModuleGlobals *self, size_t index) {
	DeeObject *value;
	DeeModuleObject *mod = self->modglobals_module;
	if (DeeModule_LockSymbols(mod))
		goto err;
	if (index >= mod->mo_globalc) {
		DeeModule_UnlockSymbols(mod);
		return -2;
	}
	DeeModule_LockRead(mod);
	value = mod->mo_globalv[index];
	DeeModule_LockEndRead(mod);
	DeeModule_UnlockSymbols(mod);
	return value ? 1 : 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
modglobals_nsi_xchitem(ModuleGlobals *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeModuleObject *mod = self->modglobals_module;
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
modglobals_nsi_delitem(ModuleGlobals *self, size_t index) {
	return modglobals_nsi_setitem(self, index, NULL);
}

PRIVATE struct type_nsi tpconst modglobals_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&modglobals_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&modglobals_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&modglobals_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)&modglobals_nsi_delitem,
			/* .nsi_setitem      = */ (dfunptr_t)&modglobals_nsi_setitem,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)&modglobals_nsi_xchitem,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq modglobals_seq = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &modglobals_nsi,
	/* .tp_foreach            = */ NULL, /* TODO */
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&modglobals_nsi_getsize,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&modglobals_nsi_getsize,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&modglobals_nsi_getitem,
	/* .tp_getitem_index_fast = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&modglobals_nsi_delitem,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&modglobals_nsi_setitem,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&modglobals_nsi_bounditem,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&modglobals_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&modglobals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &modglobals_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ modglobals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTERN WUNUSED NONNULL((1)) DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self) {
	DREF ModuleGlobals *result;
	result = DeeObject_MALLOC(ModuleGlobals);
	if unlikely(!result)
		goto done;
	result->modglobals_module = self;
	Dee_Incref(self);
	DeeObject_Init(result, &ModuleGlobals_Type);
done:
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C */
