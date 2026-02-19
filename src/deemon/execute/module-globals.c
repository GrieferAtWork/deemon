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
#ifndef GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C
#define GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* Dee_True */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/map.h>                /* DeeMapping_Type */
#include <deemon/method-hints.h>       /* Dee_seq_enumerate_index_t, Dee_seq_enumerate_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/module.h>             /* DeeModule*, Dee_MODSYM_F*, Dee_MODULE_PROPERTY_GET, Dee_module_* */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_COMPARE_ERR, Dee_Decref, Dee_Decref_unlikely, Dee_HAS_*, Dee_Incref, Dee_TYPE, Dee_XDecref, Dee_XIncref, Dee_hash_t, Dee_return_compareT, Dee_ssize_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT */
#include <deemon/pair.h>               /* DeeSeq_OfPairInherited */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_visit_t, METHOD_FCONSTCALL, METHOD_FNOREFESCAPE, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak, atomic_read */

#include "../objects/generic-proxy.h"
#include "../objects/seq/default-map-proxy.h"
#include "../runtime/strings.h"

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* UINT16_MAX, uint16_t */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#define do_DeeModule_LockSymbols(err, mod) (void)0
#define do_DeeModule_UnlockSymbols(mod)    (void)0
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#define do_DeeModule_LockSymbols(err, mod) \
	do {                                   \
		if (DeeModule_LockSymbols(mod))    \
			goto err;                      \
	}	__WHILE0
#define do_DeeModule_UnlockSymbols(mod) \
	DeeModule_UnlockSymbols(mod)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeModuleObject, mei_module); /* [1..1][const] The module who's exports are being iterated. */
	DWEAK uint16_t                        mei_index;   /* The current global variable index. */
} ModuleExportsIterator;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeModuleObject, me_module); /* [1..1][const] The module who's exports are being viewed. */
} ModuleExports;

#define ModuleExports_New(mod) \
	((DREF ModuleExports *)ProxyObject_New(&ModuleExports_Type, Dee_AsObject(mod)))

INTDEF DeeTypeObject ModuleExports_Type;
INTDEF DeeTypeObject ModuleExportsIterator_Type;
INTDEF DeeTypeObject ModuleExportsKeysIterator_Type;
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
	DeeArg_Unpack1(err, argc, argv, "_ModuleExportsIterator", &exports_map);
	if (DeeObject_AssertTypeExact(exports_map, &ModuleExports_Type))
		goto err;
	self->mei_index  = 0;
	self->mei_module = exports_map->me_module;
	Dee_Incref(self->mei_module);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexportskeysiter_init(ModuleExportsIterator *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	DefaultSequence_MapProxy *proxy;
	DeeArg_Unpack1(err, argc, argv, "_ModuleExportsKeysIterator", &proxy);
	if (DeeObject_AssertTypeExact(proxy, &DefaultSequence_MapKeys_Type))
		goto err;
	if (DeeObject_AssertTypeExact(proxy->dsmp_map, &ModuleExports_Type))
		goto err;
	self->mei_index  = 0;
	self->mei_module = ((ModuleExports *)proxy->dsmp_map)->me_module;
	Dee_Incref(self->mei_module);
	return 0;
err:
	return -1;
}

#define modexportsiter_fini      generic_proxy__fini_unlikely
#define modexportsiter_visit     generic_proxy__visit
#define modexportsiter_serialize generic_proxy__serialize_and_wordcopy_atomic16

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
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&modexportsiter_compare,
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
                         struct Dee_module_symbol *__restrict sym) {
	if likely(!(sym->ss_flags & (Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY))) {
		DREF DeeObject *result;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[Dee_module_symbol_getindex(sym)];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result)
			result = ITER_DONE; /* Unbound */
		return result;
	}

	/* External symbol, or property. */
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return ITER_DONE;
		/* Invoke the property callback. */
		return DeeObject_CallInherited(callback, 0, NULL);
	}

	/* External symbol. */
	ASSERT(sym->ss_impid < self->mo_importc);
	self = self->mo_importv[sym->ss_impid];
	goto read_symbol;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexportsiter_next(ModuleExportsIterator *__restrict self) {
	DREF DeeObject *result_name, *result_value;
	DeeModuleObject *mod = self->mei_module;
	uint16_t old_index, new_index;
	do_DeeModule_LockSymbols(err, mod);
again:
	for (;;) {
		struct Dee_module_symbol *sym;
		old_index = atomic_read(&self->mei_index);
		if (old_index > mod->mo_bucketm) {
			do_DeeModule_UnlockSymbols(mod);
			return ITER_DONE;
		}
		new_index = old_index;
		for (;;) {
			sym = &mod->mo_bucketv[new_index];
			if (sym->ss_name) {
				result_name = Dee_AsObject(Dee_module_symbol_getnameobj(sym));
				if unlikely(!result_name) {
					do_DeeModule_UnlockSymbols(mod);
					return NULL;
				}
				break;
			}
continue_symbol_search:
			++new_index;
			if (new_index > mod->mo_bucketm) {
				if (!atomic_cmpxch_weak(&self->mei_index, old_index, new_index))
					goto again;
				do_DeeModule_UnlockSymbols(mod);
				return ITER_DONE;
			}
		}
		/* Read the current value of this symbol. */
		result_value = module_it_getattr_symbol(mod, sym);
		if (!ITER_ISOK(result_value)) {
			Dee_Decref_unlikely(result_name);
			if unlikely(!result_value)
				goto err;
			goto continue_symbol_search;
		}
		if (atomic_cmpxch_weak(&self->mei_index, old_index, new_index + 1))
			break;
		Dee_Decref_unlikely(result_name);
		Dee_Decref_unlikely(result_value);
	}
	do_DeeModule_UnlockSymbols(mod);
	return DeeSeq_OfPairInherited(result_name, result_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexportskeysiter_next(ModuleExportsIterator *__restrict self) {
	DREF DeeObject *result_name;
	DeeModuleObject *mod = self->mei_module;
	uint16_t old_index, new_index;
	do_DeeModule_LockSymbols(err, mod);
again:
	old_index = atomic_read(&self->mei_index);
	if (old_index > mod->mo_bucketm) {
		do_DeeModule_UnlockSymbols(mod);
		return ITER_DONE;
	}
	new_index = old_index;
	for (;;) {
		struct Dee_module_symbol *sym;
		sym = &mod->mo_bucketv[new_index];
		if (sym->ss_name) {
			result_name = Dee_AsObject(Dee_module_symbol_getnameobj(sym));
			if unlikely(!result_name) {
				do_DeeModule_UnlockSymbols(mod);
				return NULL;
			}
			break;
		}
		++new_index;
		if (new_index > mod->mo_bucketm) {
			if (!atomic_cmpxch_weak(&self->mei_index, old_index, new_index))
				goto again;
			do_DeeModule_UnlockSymbols(mod);
			return ITER_DONE;
		}
	}
	if (!atomic_cmpxch_weak(&self->mei_index, old_index, new_index + 1)) {
		Dee_Decref_unlikely(result_name);
		goto again;
	}
	do_DeeModule_UnlockSymbols(mod);
	return result_name;
}

PRIVATE WUNUSED NONNULL((1)) DREF ModuleExports *DCALL
modexportsiter_getseq(ModuleExportsIterator *__restrict self) {
	return DeeModule_ViewExports(self->mei_module);
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_MapProxy *DCALL
modexportskeysiter_getseq(ModuleExportsIterator *__restrict self) {
	DREF ModuleExports *exports = modexportsiter_getseq(self);
	if unlikely(!exports)
		goto err;
	return DefaultSequence_MapKeys_NewInherited(exports);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst modexportsiter_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &modexportsiter_getseq,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Ert:ModuleExports"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst modexportskeysiter_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &modexportskeysiter_getseq,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Ert:MapKeys"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst modexportsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___module__, STRUCT_OBJECT_AB,
	                      offsetof(ModuleExportsIterator, mei_module),
	                      "->?DModule"),
	TYPE_MEMBER_END
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ModuleExportsIterator,
			/* tp_ctor:        */ &modexportsiter_ctor,
			/* tp_copy_ctor:   */ &modexportsiter_copy,
			/* tp_any_ctor:    */ &modexportsiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &modexportsiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&modexportsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__iter_operator_bool__with__iter_peek),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&modexportsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &modexportsiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexportsiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ modexportsiter_getsets,
	/* .tp_members       = */ modexportsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject ModuleExportsKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleExportsKeysIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ModuleExportsIterator,
			/* tp_ctor:        */ &modexportsiter_ctor,
			/* tp_copy_ctor:   */ &modexportsiter_copy,
			/* tp_any_ctor:    */ &modexportskeysiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &modexportsiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&modexportsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__iter_operator_bool__with__iter_peek),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&modexportsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &modexportsiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&modexportskeysiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ modexportskeysiter_getsets,
	/* .tp_members       = */ modexportsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
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
	DeeArg_Unpack1(err, argc, argv, "_ModuleExports", &self->me_module);
	if (DeeObject_AssertType(self->me_module, &DeeModule_Type))
		goto err;
	Dee_Incref(self->me_module);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleExports, me_module) == offsetof(ProxyObject, po_obj));
#define modexports_copy      generic_proxy__copy_alias
#define modexports_fini      generic_proxy__fini_unlikely
#define modexports_visit     generic_proxy__visit
#define modexports_serialize generic_proxy__serialize

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

PRIVATE WUNUSED NONNULL((1)) DREF ModuleExportsIterator *DCALL
modexports_iterkeys(ModuleExports *__restrict self) {
	DREF ModuleExportsIterator *result;
	result = DeeObject_MALLOC(ModuleExportsIterator);
	if unlikely(!result)
		goto done;
	result->mei_module = self->me_module;
	result->mei_index  = 0;
	Dee_Incref(result->mei_module);
	DeeObject_Init(result, &ModuleExportsKeysIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
modexports_size(ModuleExports *__restrict self) {
	size_t i, total_symbols = 0;
	DeeModuleObject *mod = self->me_module;
	do_DeeModule_LockSymbols(err, mod);
	for (i = 0; i <= mod->mo_bucketm; ++i) {
		if (mod->mo_bucketv[i].ss_name)
			++total_symbols;
	}
	do_DeeModule_UnlockSymbols(mod);
	return total_symbols;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return (size_t)-1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeModule_GetAttrSymbol_asitem(ModuleExports *exports_map,
                               DeeModuleObject *self,
                               struct Dee_module_symbol *sym) {
	if likely(!(sym->ss_flags & (Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY))) {
		DREF DeeObject *result;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[Dee_module_symbol_getindex(sym)];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result) {
			if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ) {
				DeeRT_ErrUnboundKey(exports_map,
				                    COMPILER_CONTAINER_OF(sym->ss_name,
				                                          DeeStringObject,
				                                          s_str));
			} else {
				DeeRT_ErrUnboundKeyStr(exports_map, sym->ss_name);
			}
		}
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
			if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ) {
				DeeRT_ErrUnboundKey(exports_map,
				                    COMPILER_CONTAINER_OF(sym->ss_name,
				                                          DeeStringObject,
				                                          s_str));
			} else {
				DeeRT_ErrUnboundKeyStr(exports_map, sym->ss_name);
			}
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_TryGetAttrSymbol_asitem(DeeModuleObject *self,
                                  struct Dee_module_symbol *sym) {
	if likely(!(sym->ss_flags & (Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY))) {
		DREF DeeObject *result;
read_symbol:
		ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
		DeeModule_LockRead(self);
		result = self->mo_globalv[Dee_module_symbol_getindex(sym)];
		Dee_XIncref(result);
		DeeModule_LockEndRead(self);
		if unlikely(!result)
			return ITER_DONE;
		return result;
	}

	/* External symbol, or property. */
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		DREF DeeObject *result;
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return ITER_DONE;

		/* Invoke the property callback. */
		result = DeeObject_CallInherited(callback, 0, NULL);
		if unlikely(!result) {
			if (DeeError_Catch(&DeeError_UnboundAttribute) ||
			    DeeError_Catch(&DeeError_UnboundLocal) ||
			    DeeError_Catch(&DeeError_UnboundItem))
				return ITER_DONE;
		}
		return result;
	}

	/* External symbol. */
	ASSERT(sym->ss_impid < self->mo_importc);
	self = self->mo_importv[sym->ss_impid];
	goto read_symbol;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_BoundAttrSymbol_asitem(DeeModuleObject *self,
                                 struct Dee_module_symbol *sym) {
	if likely(!(sym->ss_flags & (Dee_MODSYM_FEXTERN | Dee_MODSYM_FPROPERTY))) {
read_symbol:
		ASSERT(Dee_module_symbol_getindex(sym) < self->mo_globalc);
		return Dee_BOUND_FROMBOOL(atomic_read(&self->mo_globalv[Dee_module_symbol_getindex(sym)]));
	}

	/* External symbol, or property. */
	if (sym->ss_flags & Dee_MODSYM_FPROPERTY) {
		DREF DeeObject *result;
		DREF DeeObject *callback;
		DeeModule_LockRead(self);
		callback = self->mo_globalv[Dee_module_symbol_getindex(sym) + Dee_MODULE_PROPERTY_GET];
		Dee_XIncref(callback);
		DeeModule_LockEndRead(self);
		if unlikely(!callback)
			return Dee_BOUND_NO;

		/* Invoke the property callback. */
		result = DeeObject_CallInherited(callback, 0, NULL);
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
	ASSERT(sym->ss_impid < self->mo_importc);
	self = self->mo_importv[sym->ss_impid];
	goto read_symbol;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_getitem(ModuleExports *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if (DeeString_Check(key)) {
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err_handle_overflow;
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	DeeRT_ErrUnknownKey(self, key);
err:
	return NULL;
	{
		DREF DeeObject *error;
err_handle_overflow:
		error = DeeError_CatchError(&DeeError_IntegerOverflow);
		if (error)
			DeeRT_ErrUnknownKeyWithCause(self, key, error);
	}
	goto err;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_bounditem(ModuleExports *self, DeeObject *key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if (DeeString_Check(key)) {
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err_handle_overflow;
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return Dee_BOUND_MISSING;
err_handle_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		goto nokey;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_hasitem(ModuleExports *self, DeeObject *key) {
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if (DeeString_Check(key)) {
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err_handle_overflow;
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!sym)
		goto nokey_unlock;
	do_DeeModule_UnlockSymbols(mod);
	return Dee_HAS_YES;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return Dee_HAS_NO;
err_handle_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		goto nokey;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return Dee_HAS_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_trygetitem(ModuleExports *self, DeeObject *key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if (DeeString_Check(key)) {
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err_handle_overflow;
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return ITER_DONE;
err_handle_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		goto nokey;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_delitem(ModuleExports *self, DeeObject *key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if (DeeString_Check(key)) {
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err_handle_overflow;
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return 0;
err:
	return -1;
err_handle_overflow:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		goto nokey;
	goto err;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
modexports_setitem(ModuleExports *self, DeeObject *key, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if (DeeString_Check(key)) {
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbol(mod, key);
	} else {
		uint16_t gid;
		if (DeeObject_AsUInt16(key, &gid))
			goto err_handle_overflow;
		do_DeeModule_LockSymbols(err, mod);
		sym = DeeModule_GetSymbolID(mod, gid);
	}
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, sym, value);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	DeeRT_ErrUnknownKey(self, key);
err:
	return -1;
	{
		DREF DeeObject *error;
err_handle_overflow:
		error = DeeError_CatchError(&DeeError_IntegerOverflow);
		if (error)
			DeeRT_ErrUnknownKeyWithCause(self, key, error);
	}
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexports_getitem_index(ModuleExports *self, size_t key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
err_nokey:
	DeeRT_ErrUnknownKeyInt(Dee_AsObject(self), key);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_bounditem_index(ModuleExports *self, size_t key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if unlikely(key > UINT16_MAX)
		goto nokey;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return Dee_BOUND_MISSING;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_BOUND_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_hasitem_index(ModuleExports *self, size_t key) {
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if unlikely(key > UINT16_MAX)
		goto nokey;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!sym)
		goto nokey_unlock;
	do_DeeModule_UnlockSymbols(mod);
	return Dee_HAS_YES;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return Dee_HAS_NO;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_HAS_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modexports_trygetitem_index(ModuleExports *self, size_t key) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if unlikely(key > UINT16_MAX)
		goto nokey;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return ITER_DONE;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modexports_delitem_index(ModuleExports *self, size_t key) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if unlikely(key > UINT16_MAX)
		goto nokey;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
nokey:
	return 0;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
modexports_setitem_index(ModuleExports *self, size_t key, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	if unlikely(key > UINT16_MAX)
		goto err_nokey;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolID(mod, (uint16_t)key);
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, sym, value);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
err_nokey:
	return DeeRT_ErrUnknownKeyInt(Dee_AsObject(self), key);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}




PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_getitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	DeeRT_ErrUnknownKeyStr(Dee_AsObject(self), key);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_bounditem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_MISSING;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_BOUND_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_hasitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	do_DeeModule_UnlockSymbols(mod);
	return Dee_HAS_YES;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return Dee_HAS_NO;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_HAS_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_trygetitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return ITER_DONE;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_delitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return 0;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
modexports_setitem_string_hash(ModuleExports *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringHash(mod, key, hash);
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, sym, value);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return DeeRT_ErrUnknownKeyStr(Dee_AsObject(self), key);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_getitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_GetAttrSymbol_asitem(self, mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	DeeRT_ErrUnknownKeyStrLen(Dee_AsObject(self), key, keylen);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_bounditem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_BoundAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_MISSING;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_BOUND_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_hasitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	do_DeeModule_UnlockSymbols(mod);
	return Dee_HAS_YES;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return Dee_HAS_NO;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_HAS_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
modexports_trygetitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_TryGetAttrSymbol_asitem(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return ITER_DONE;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
modexports_delitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!sym)
		goto nokey_unlock;
	result = DeeModule_DelAttrSymbol(mod, sym);
	do_DeeModule_UnlockSymbols(mod);
	return result;
nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return 0;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
modexports_setitem_string_len_hash(ModuleExports *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	int result;
	DeeModuleObject *mod = self->me_module;
	struct Dee_module_symbol *sym;
	do_DeeModule_LockSymbols(err, mod);
	sym = DeeModule_GetSymbolStringLenHash(mod, key, keylen, hash);
	if unlikely(!sym)
		goto err_nokey_unlock;
	result = DeeModule_SetAttrSymbol(mod, sym, value);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_nokey_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return DeeRT_ErrUnknownKeyStrLen(Dee_AsObject(self), key, keylen);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}



PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
modexports_mh_map_enumerate(ModuleExports *self, Dee_seq_enumerate_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeModuleObject *mod = self->me_module;
	Dee_hash_t i;
	do_DeeModule_LockSymbols(err, mod);
	for (i = 0; i <= mod->mo_bucketm; ++i) {
		DREF DeeStringObject *item_name;
		DREF DeeObject *item_value;
		struct Dee_module_symbol *item = &mod->mo_bucketv[i];
		if (!item->ss_name)
			continue;
		item_name = Dee_module_symbol_getnameobj(item);
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
	do_DeeModule_UnlockSymbols(mod);
	return result;
err_temp_unlock:
	do_DeeModule_UnlockSymbols(mod);
	return temp;
err_unlock:
	do_DeeModule_UnlockSymbols(mod);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
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
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
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

PRIVATE struct type_getset tpconst modexports_getsets[] = {
	TYPE_GETTER_AB_F(STR___map_iterkeys__, &modexports_iterkeys, METHOD_FNOREFESCAPE,
	                 "->?Ert:MapFromAttrKeysIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst modexports_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___module__, STRUCT_OBJECT_AB,
	                      offsetof(ModuleExports, me_module), "->?DModule"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst modexports_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ModuleExportsIterator_Type),
	TYPE_MEMBER_CONST(STR_KeysIterator, &ModuleExportsKeysIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ModuleExports_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleExports",
	/* .tp_doc      = */ DOC("()\n"
	                         "(mod:?DModule)\n"
	                         "\n"
	                         "[](key:?X2?Dstring?Dint)->"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ModuleExports,
			/* tp_ctor:        */ &modexports_ctor,
			/* tp_copy_ctor:   */ &modexports_copy,
			/* tp_any_ctor:    */ &modexports_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &modexports_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&modexports_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E66FA6851AAFE176),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__4CF94EE41850B0EF),
	/* .tp_seq           = */ &modexports_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ modexports_methods,
	/* .tp_getsets       = */ modexports_getsets,
	/* .tp_members       = */ modexports_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ modexports_class_members,
	/* .tp_method_hints  = */ modexports_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1)) DREF ModuleExports *DCALL
DeeModule_ViewExports(DeeModuleObject *__restrict self) {
	return ModuleExports_New(self);
}





typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeModuleObject, mg_module); /* [1..1] The module who's exports are being viewed. */
} ModuleGlobals;

#define ModuleGlobals_New(mod) \
	((DREF ModuleGlobals *)ProxyObject_New(&ModuleGlobals_Type, Dee_AsObject(mod)))

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_init(ModuleGlobals *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_ModuleGlobals", &self->mg_module);
	if (DeeObject_AssertType(self->mg_module, &DeeModule_Type))
		goto err;
	Dee_Incref(self->mg_module);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleGlobals, mg_module) == offsetof(ProxyObject, po_obj));
#define modglobals_copy      generic_proxy__copy_alias
#define modglobals_fini      generic_proxy__fini_unlikely
#define modglobals_visit     generic_proxy__visit
#define modglobals_serialize generic_proxy__serialize

STATIC_ASSERT(offsetof(ModuleGlobals, mg_module) == offsetof(ModuleExports, me_module));
#define modglobals_ctor    modexports_ctor
#define modglobals_bool    modexports_bool
#define modglobals_members modexports_members


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
modglobals_size(ModuleGlobals *__restrict self) {
	return atomic_read(&self->mg_module->mo_globalc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modglobals_getitem_index(ModuleGlobals *self, size_t index) {
	DREF DeeObject *result;
	DeeModuleObject *mod = self->mg_module;
	do_DeeModule_LockSymbols(err, mod);
	if unlikely(index >= mod->mo_globalc) {
		size_t num_globals = mod->mo_globalc;
		do_DeeModule_UnlockSymbols(mod);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, num_globals);
		goto err;
	}
	DeeModule_LockRead(mod);
	result = mod->mo_globalv[index];
	if unlikely(!result) {
		DeeModule_LockEndRead(mod);
		do_DeeModule_UnlockSymbols(mod);
		DeeRT_ErrUnboundIndex(self, index);
		goto err;
	}
	Dee_Incref(result);
	DeeModule_LockEndRead(mod);
	do_DeeModule_UnlockSymbols(mod);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_setitem_index(ModuleGlobals *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldvalue;
	DeeModuleObject *mod = self->mg_module;
	do_DeeModule_LockSymbols(err, mod);
	if unlikely(index >= mod->mo_globalc) {
		size_t num_globals = mod->mo_globalc;
		do_DeeModule_UnlockSymbols(mod);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, num_globals);
		goto err;
	}
	DeeModule_LockWrite(mod);
	Dee_XIncref(value);
	oldvalue = mod->mo_globalv[index];
	mod->mo_globalv[index] = value;
	DeeModule_LockEndWrite(mod);
	do_DeeModule_UnlockSymbols(mod);
	Dee_XDecref(oldvalue);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
modglobals_bounditem_index(ModuleGlobals *self, size_t index) {
	DeeObject *value;
	DeeModuleObject *mod = self->mg_module;
	do_DeeModule_LockSymbols(err, mod);
	if (index >= mod->mo_globalc) {
		do_DeeModule_UnlockSymbols(mod);
		return Dee_BOUND_MISSING;
	}
	DeeModule_LockRead(mod);
	value = mod->mo_globalv[index];
	DeeModule_LockEndRead(mod);
	do_DeeModule_UnlockSymbols(mod);
	return Dee_BOUND_FROMBOOL(value);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return Dee_BOUND_ERR;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
	do_DeeModule_LockSymbols(err, mod);
	if unlikely(index >= mod->mo_globalc) {
		size_t num_globals = mod->mo_globalc;
		do_DeeModule_UnlockSymbols(mod);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, num_globals);
		goto err;
	}
	DeeModule_LockWrite(mod);
	oldvalue = mod->mo_globalv[index];
	if unlikely(!oldvalue) {
		do_DeeModule_UnlockSymbols(mod);
		DeeRT_ErrUnboundIndex(self, index);
		goto err;
	}
	Dee_Incref(value);
	mod->mo_globalv[index] = value;
	DeeModule_LockEndWrite(mod);
	do_DeeModule_UnlockSymbols(mod);
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
		do_DeeModule_LockSymbols(err, mod);
		if (end > mod->mo_globalc) {
			end = mod->mo_globalc;
			if (start >= end) {
				do_DeeModule_UnlockSymbols(mod);
				break;
			}
		}
		DeeModule_LockRead(mod);
		item = mod->mo_globalv[start];
		Dee_XIncref(item);
		DeeModule_LockEndRead(mod);
		do_DeeModule_UnlockSymbols(mod);
		temp = (*proc)(arg, start, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
	/* .tp_doc      = */ DOC("()\n"
	                         "(mod:?DModule)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ModuleExports,
			/* tp_ctor:        */ &modglobals_ctor,
			/* tp_copy_ctor:   */ &modglobals_copy,
			/* tp_any_ctor:    */ &modglobals_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &modglobals_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&modglobals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__414C2E3C21EC29FD),
	/* .tp_seq           = */ &modglobals_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ modglobals_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ modglobals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ modglobals_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1)) DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self) {
	return ModuleGlobals_New(self);
}



#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeModuleObject, mln_module); /* [1..1] The module who's libnames are being viewed. */
} ModuleLibNames;

#define ModuleLibNames_New(mod) \
	((DREF ModuleLibNames *)ProxyObject_New(&ModuleLibNames_Type, Dee_AsObject(mod)))

PRIVATE WUNUSED NONNULL((1)) int DCALL
modlibnames_init(ModuleLibNames *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_ModuleLibNames", &self->mln_module);
	if (DeeObject_AssertType(self->mln_module, &DeeModule_Type))
		goto err;
	Dee_Incref(self->mln_module);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(ModuleLibNames, mln_module) == offsetof(ProxyObject, po_obj));
#define modlibnames_copy      generic_proxy__copy_alias
#define modlibnames_fini      generic_proxy__fini_unlikely
#define modlibnames_visit     generic_proxy__visit
#define modlibnames_serialize generic_proxy__serialize

STATIC_ASSERT(offsetof(ModuleLibNames, mln_module) == offsetof(ModuleExports, me_module));
#define modlibnames_ctor    modexports_ctor
#define modlibnames_members modexports_members

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
modlibnames_size(ModuleLibNames *__restrict self) {
	return DeeModule_GetLibNameCount(self->mln_module);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
modlibnames_getitem_index(ModuleLibNames *self, size_t index) {
	size_t size;
	DeeModuleObject *mod = self->mln_module;
	DREF DeeObject *result;
again:
	result = DeeModule_GetLibName(mod, index);
	if (result != ITER_DONE)
		return result;
	size = DeeModule_GetLibNameCount(mod);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index < size)
		goto again;
	DeeRT_ErrIndexOutOfBounds(self, index, size);
err:
	return NULL;
}

PRIVATE struct type_member tpconst modlibnames_class_members[] = {
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_seq modlibnames_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&modlibnames_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&modlibnames_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

INTERN DeeTypeObject ModuleLibNames_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ModuleLibNames",
	/* .tp_doc      = */ DOC("()\n"
	                         "(mod:?DModule)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ModuleExports,
			/* tp_ctor:        */ &modlibnames_ctor,
			/* tp_copy_ctor:   */ &modlibnames_copy,
			/* tp_any_ctor:    */ &modlibnames_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &modlibnames_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&modlibnames_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&modlibnames_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__414C2E3C21EC29FD),
	/* .tp_seq           = */ &modlibnames_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ modlibnames_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ modlibnames_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN WUNUSED NONNULL((1)) DREF ModuleLibNames *DCALL
DeeModule_LibNames(DeeModuleObject *__restrict self) {
	return ModuleLibNames_New(self);
}
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_C */
