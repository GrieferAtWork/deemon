/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
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
INTDEF DeeTypeObject ModuleGlobalsIterator_Type;
INTDEF WUNUSED NONNULL((1)) DREF ModuleExports *DCALL DeeModule_ViewExports(DeeModuleObject *__restrict self);
#define READ_INDEX(x) atomic_read(&(x)->mei_index)


PRIVATE WUNUSED NONNULL((1)) int DCALL
mei_ctor(ModuleExportsIterator *__restrict self) {
	self->mei_index  = 0;
	self->mei_module = &empty_module;
	Dee_Incref(&empty_module);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mei_copy(ModuleExportsIterator *__restrict self,
         ModuleExportsIterator *__restrict other) {
	self->mei_index  = READ_INDEX(other);
	self->mei_module = other->mei_module;
	Dee_Incref(self->mei_module);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mei_init(ModuleExportsIterator *__restrict self,
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
mei_fini(ModuleExportsIterator *__restrict self) {
	Dee_Decref_unlikely(self->mei_module);
}

PRIVATE NONNULL((1, 2)) void DCALL
mei_visit(ModuleExportsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mei_module);
}


#define DEFINE_MEI_COMPARE(name, op)                                  \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL             \
	name(ModuleExportsIterator *self, ModuleExportsIterator *other) { \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))         \
			return NULL;                                              \
		return_bool(READ_INDEX(self) op READ_INDEX(other));           \
	}
DEFINE_MEI_COMPARE(mei_eq, ==)
DEFINE_MEI_COMPARE(mei_ne, !=)
DEFINE_MEI_COMPARE(mei_lo, <)
DEFINE_MEI_COMPARE(mei_le, <=)
DEFINE_MEI_COMPARE(mei_gr, >)
DEFINE_MEI_COMPARE(mei_ge, >=)
#undef DEFINE_MEI_COMPARE

PRIVATE struct type_cmp mei_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mei_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mei_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mei_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mei_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mei_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mei_ge,
};

LOCAL WUNUSED DREF DeeObject *DCALL
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
mei_next(ModuleExportsIterator *__restrict self) {
	DREF DeeObject *result, *result_name, *result_value;
	DeeModuleObject *mod = self->mei_module;
	uint16_t old_index, new_index;
	if (DeeModule_LockSymbols(mod))
		goto err;
again:
	for (;;) {
		struct module_symbol *symbol;
		old_index = READ_INDEX(self);
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
mei_getseq(ModuleExportsIterator *__restrict self) {
	return DeeModule_ViewExports(self->mei_module);
}

PRIVATE struct type_getset tpconst mei_getsets[] = {
	TYPE_GETTER(STR_seq, &mei_getseq, "->?Ert:ModuleExports"),
	TYPE_GETSET_END
};

#undef READ_INDEX
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
				/* .tp_ctor      = */ (dfunptr_t)&mei_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mei_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&mei_init,
				TYPE_FIXED_ALLOCATOR(ModuleExportsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mei_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mei_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &mei_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mei_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mei_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
me_ctor(ModuleExports *__restrict self) {
	self->me_module = &empty_module;
	Dee_Incref(&empty_module);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
me_init(ModuleExports *__restrict self,
        size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ModuleExports", &self->me_module))
		goto err;
	if (DeeObject_AssertType(self->me_module, &DeeModule_Type))
		goto err;
	Dee_Incref(&empty_module);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_module) ==
              offsetof(ModuleExports, me_module));
#define me_fini  mei_fini
#define me_visit mei_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
me_bool(ModuleExports *__restrict self) {
	return self->me_module->mo_globalc;
}

PRIVATE WUNUSED NONNULL((1)) DREF ModuleExportsIterator *DCALL
me_iter(ModuleExports *__restrict self) {
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
me_size(ModuleExports *__restrict self) {
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
me_contains(ModuleExports *self, DeeObject *key) {
	bool result;
	DeeModuleObject *mod = self->me_module;
	if (!DeeString_Check(key))
		return_false;
	if (DeeModule_LockSymbols(mod))
		goto err;
	result = DeeModule_GetSymbolString(mod,
	                                   DeeString_STR(key),
	                                   DeeString_Hash(key)) != NULL;
	DeeModule_UnlockSymbols(mod);
	return_bool_(result);
err:
	return NULL;
}

LOCAL WUNUSED DREF DeeObject *DCALL
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
me_get(ModuleExports *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (!DeeString_Check(key))
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolString(mod,
	                                   DeeString_STR(key),
	                                   DeeString_Hash(key));
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
me_get_f(ModuleExports *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	DeeObject *defl = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &defl))
		goto err;
	if (!DeeString_Check(key))
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolString(mod,
	                                   DeeString_STR(key),
	                                   DeeString_Hash(key));
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
me_del(ModuleExports *__restrict self, DeeObject *__restrict key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (!DeeString_Check(key))
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolString(mod,
	                                   DeeString_STR(key),
	                                   DeeString_Hash(key));
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
me_set(ModuleExports *__restrict self,
       DeeObject *__restrict key,
       DeeObject *__restrict value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct module_symbol *symbol;
	if (!DeeString_Check(key))
		goto err_unknown_key;
	if (DeeModule_LockSymbols(mod))
		goto err;
	symbol = DeeModule_GetSymbolString(mod,
	                                   DeeString_STR(key),
	                                   DeeString_Hash(key));
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

PRIVATE struct type_seq me_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&me_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&me_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&me_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&me_get,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&me_del,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&me_set
};



DOC_REF(map_get_doc);

PRIVATE struct type_method tpconst me_methods[] = {
	TYPE_METHOD(STR_get, &me_get_f, DOC_GET(map_get_doc)),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst me_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___module__, STRUCT_OBJECT, offsetof(ModuleExports, me_module), "->?DModule"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst me_class_members[] = {
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
				/* .tp_ctor      = */ (dfunptr_t)&me_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&me_init,
				TYPE_FIXED_ALLOCATOR(ModuleExports)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&me_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&me_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&me_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &me_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ me_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ me_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ me_class_members
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
	DREF DeeModuleObject *mgi_module; /* [1..1] The module who's exports are being viewed. */
	DWEAK uint16_t        mgi_index;  /* The current global variable index. */
} ModuleGlobalsIterator;

STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_module) ==
              offsetof(ModuleGlobalsIterator, mgi_module));
STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_index) ==
              offsetof(ModuleGlobalsIterator, mgi_index));
#define mgi_ctor  mei_ctor
#define mgi_copy  mei_copy
#define mgi_fini  mei_fini
#define mgi_visit mei_visit
#define mgi_cmp   mei_cmp

#define READ_INDEX(x) atomic_read(&(x)->mgi_index)

typedef struct {
	OBJECT_HEAD
	DREF DeeModuleObject *mg_module; /* [1..1] The module who's exports are being viewed. */
} ModuleGlobals;

INTDEF WUNUSED NONNULL((1)) DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self);


PRIVATE WUNUSED NONNULL((1)) int DCALL
mgi_init(ModuleGlobalsIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	ModuleGlobals *globals;
	if (DeeArg_Unpack(argc, argv, "o:_ModuleGlobalsIterator", &globals))
		goto err;
	if (DeeObject_AssertTypeExact(globals, &ModuleGlobals_Type))
		goto err;
	self->mgi_index  = 0;
	self->mgi_module = globals->mg_module;
	Dee_Incref(self->mgi_module);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mgi_next(ModuleGlobalsIterator *__restrict self) {
	DeeModuleObject *mod = self->mgi_module;
	DREF DeeObject *result;
	uint16_t old_index, new_index;
	if (DeeModule_LockSymbols(mod))
		goto err;
again:
	for (;;) {
		old_index = atomic_read(&self->mgi_index);
		if (old_index >= mod->mo_globalc) {
			DeeModule_UnlockSymbols(mod);
			return ITER_DONE;
		}
		new_index = old_index;
		DeeModule_LockRead(mod);
		while ((result = mod->mo_globalv[new_index]) == NULL) {
			++new_index;
			if (new_index >= mod->mo_globalc) {
				DeeModule_LockEndRead(mod);
				if (!atomic_cmpxch_weak_or_write(&self->mgi_index, old_index, new_index))
					goto again;
				return ITER_DONE;
			}
		}
		Dee_Incref(result);
		DeeModule_LockEndRead(mod);
		if (atomic_cmpxch_weak_or_write(&self->mgi_index, old_index, new_index + 1))
			break;
		Dee_Decref(result);
	}
	DeeModule_UnlockSymbols(mod);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF ModuleGlobals *DCALL
mgi_getseq(ModuleGlobalsIterator *__restrict self) {
	return DeeModule_ViewGlobals(self->mgi_module);
}

PRIVATE struct type_getset tpconst mgi_getsets[] = {
	TYPE_GETTER_NODOC(STR_seq, &mgi_getseq),
	TYPE_GETSET_END
};
#undef READ_INDEX


INTERN DeeTypeObject ModuleGlobalsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleGlobalsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mgi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mgi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&mgi_init,
				TYPE_FIXED_ALLOCATOR(ModuleGlobalsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &mgi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mgi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mgi_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
mg_init(ModuleGlobals *__restrict self,
        size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ModuleGlobals", &self->mg_module))
		goto err;
	if (DeeObject_AssertType(self->mg_module, &DeeModule_Type))
		goto err;
	Dee_Incref(&empty_module);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleExports, me_module) == offsetof(ModuleGlobals, mg_module));
STATIC_ASSERT(offsetof(ModuleExportsIterator, mei_module) == offsetof(ModuleGlobals, mg_module));
#define mg_ctor    me_ctor
#define mg_fini    mei_fini
#define mg_bool    me_bool
#define mg_visit   mei_visit
#define mg_members me_members


PRIVATE WUNUSED NONNULL((1)) DREF ModuleGlobalsIterator *DCALL
mg_iter(ModuleGlobals *__restrict self) {
	DREF ModuleGlobalsIterator *result;
	result = DeeObject_MALLOC(ModuleGlobalsIterator);
	if unlikely(!result)
		goto done;
	result->mgi_index  = 0;
	result->mgi_module = self->mg_module;
	Dee_Incref(self->mg_module);
	DeeObject_Init(result, &ModuleGlobalsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mg_size(ModuleGlobals *__restrict self) {
	return DeeInt_NewU16(atomic_read(&self->mg_module->mo_globalc));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mg_get(ModuleGlobals *self, DeeObject *index_ob) {
	size_t index;
	DREF DeeObject *result;
	DeeModuleObject *mod = self->mg_module;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
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

PRIVATE NONNULL((1, 2)) int DCALL
mg_set(ModuleGlobals *self, DeeObject *index_ob, DeeObject *value) {
	size_t index;
	DREF DeeObject *result;
	DeeModuleObject *mod = self->mg_module;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
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
	result = mod->mo_globalv[index];
	mod->mo_globalv[index] = value;
	DeeModule_LockEndWrite(mod);
	DeeModule_UnlockSymbols(mod);
	Dee_XDecref(result);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mg_del(ModuleGlobals *self, DeeObject *index_ob) {
	return mg_set(self, index_ob, NULL);
}

PRIVATE struct type_seq mg_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mg_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mg_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mg_get,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&mg_del,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&mg_set
};

PRIVATE struct type_member tpconst mg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ModuleGlobalsIterator_Type),
	TYPE_MEMBER_END
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
				/* .tp_ctor      = */ (dfunptr_t)&mg_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&mg_init,
				TYPE_FIXED_ALLOCATOR(ModuleExports)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mg_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &mg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mg_class_members
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
