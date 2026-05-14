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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_C
#define GUARD_DEEMON_EXECUTE_FUNCTION_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_*alloc*, Dee_CollectMemory, Dee_Free, Dee_Freea, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, _Dee_MallococBufsize */
#include <deemon/arg.h>                /* DeeArg_Unpack1, DeeArg_UnpackStruct0Or1Or2 */
#include <deemon/attribute.h>          /* DeeAttributeObject, DeeAttribute_Type */
#include <deemon/bool.h>               /* DeeBool_Check, return_bool */
#include <deemon/callable.h>           /* DeeCallable_Type */
#include <deemon/class.h>              /* DeeClassDescriptorObject, DeeClass_DESC, Dee_CLASS_*, Dee_class_attribute, Dee_class_desc, Dee_class_desc_lock_endread, Dee_class_desc_lock_read, Dee_class_operator */
#include <deemon/code.h>               /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS, CONFIG_HAVE_CODE_METRICS, CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE, DeeCodeObject, DeeCode_*, DeeFunctionObject, DeeFunction_*, DeeYieldFunctionIteratorObject, DeeYieldFunctionIterator_*, DeeYieldFunctionObject, DeeYieldFunction_Sizeof, Dee_CODE_F*, Dee_EXCEPTION_HANDLER_FFINALLY, Dee_code_frame, Dee_code_frame_kwds, Dee_except_handler, Dee_function_info, Dee_function_info_fini, Dee_hostasm_function_data_destroy, Dee_hostasm_function_init, Dee_instruction_t, code_addr_t */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/deepcopy.h>           /* DeeDeepCopyContext, DeeDeepCopyContext_AddHook, DeeDeepCopy_*, Dee_DEEPCOPY_V_MODE_REF, Dee_deepcopy_hook, Dee_deepcopy_vars */
#include <deemon/error-rt.h>           /* DeeRT_ErrUnboundAttr, DeeRT_ErrUnboundAttrCStr */
#include <deemon/error.h>              /* DeeError_*, ERROR_PRINT_DOHANDLE */
#include <deemon/format.h>             /* DeeFormat_*, PRFu16 */
#include <deemon/gc.h>                 /* DeeGCObject_*alloc*, DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/int.h>                /* DeeInt_* */
#include <deemon/kwds.h>               /* DeeKwBlackList_Decref */
#include <deemon/module.h>             /* DeeInteractiveModule_Check, DeeModule*, Dee_module_* */
#include <deemon/mro.h>                /* Dee_ATTRINFO_ATTR, Dee_ATTRINFO_MODSYM, Dee_ATTRPERM_F_* */
#include <deemon/none.h>               /* DeeNone_Check, DeeNone_NewRef, return_none */
#include <deemon/object.h>             /* ASSERT_OBJECT_*, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_COMPARE_*, Dee_Decprefv, Dee_Decref*, Dee_Incref, Dee_Movrefv, Dee_TYPE, Dee_XDecref, Dee_XDecrefv, Dee_XIncref, Dee_XMovprefv, Dee_XMovrefv, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeRefVector_NewReadonly, DeeSeq_* */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/system-features.h>    /* bzero, memcpy*, memset */
#include <deemon/traceback.h>          /* DeeFrameObject, DeeFrame_NewReferenceWithLock, Dee_FRAME_F* */
#include <deemon/tuple.h>              /* DeeTuple_Type, Dee_EmptyTuple */
#include <deemon/serial.h>              /* DeeTuple_Type, Dee_EmptyTuple */
#include <deemon/type.h>        /* DeeObject_Init, DeeObject_InitStatic, DeeTypeType_GetOperatorById, DeeType_*, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visit, Dee_Visitv, Dee_XVisit, Dee_XVisitv, Dee_operator_t, Dee_opinfo, Dee_visit_t, METHOD_F*, STRUCT_OBJECT, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h> /* atomic_inc */
#include <deemon/util/futex.h>  /* DeeFutex_WakeAll */
#include <deemon/util/hash.h>   /* Dee_HASHOF_UNBOUND_ITEM, Dee_HashCombine */
#include <deemon/util/lock.h>   /* Dee_atomic_rwlock_init */
#include <deemon/util/rlock.h>  /* Dee_rshared_rwlock_init */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "function-wrappers.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint16_t, uintptr_t */

DECL_BEGIN

#undef container_of
#define container_of COMPILER_CONTAINER_OF

#undef byte_t
#define byte_t __BYTE_TYPE__

#define PTR_iadd(T, ptr, delta) (void)((ptr) = (T *)((byte_t *)(ptr) + (delta)))
#define PTR_isub(T, ptr, delta) (void)((ptr) = (T *)((byte_t *)(ptr) - (delta)))

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeFunctionObject              Function;
typedef DeeYieldFunctionIteratorObject YFIterator;
typedef DeeYieldFunctionObject         YFunction;


PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
lookup_code_info_in_class(DeeTypeObject *type,
                          DeeCodeObject *code,
                          Function *function,
                          struct Dee_function_info *__restrict info) {
	struct Dee_class_desc *my_class;
	DeeClassDescriptorObject *desc;
	uint16_t addr;
	size_t i;
	my_class = DeeClass_DESC(type);
	desc     = my_class->cd_desc;
	Dee_class_desc_lock_read(my_class);
	for (addr = 0; addr < desc->cd_cmemb_size; ++addr) {
		if (my_class->cd_members[addr] == Dee_AsObject(function) ||
		    (my_class->cd_members[addr] &&
		     DeeFunction_Check(my_class->cd_members[addr]) &&
		     DeeFunction_CODE(my_class->cd_members[addr]) == code)) {
			Dee_class_desc_lock_endread(my_class);
			for (i = 0; i <= desc->cd_iattr_mask; ++i) {
				struct Dee_class_attribute *attr;
				attr = &desc->cd_iattr_list[i];
				if (!attr->ca_name)
					continue;
				if (addr < attr->ca_addr)
					continue;
				if ((attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FMETHOD)) !=
				    (Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FMETHOD))
					continue;
				if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FGETSET) {
					if (addr > ((attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY) ? attr->ca_addr : attr->ca_addr + 2))
						continue;
					info->fi_getset = (uint16_t)(addr - attr->ca_addr);
				} else {
					if (addr > attr->ca_addr)
						continue;
				}
				/* Found it! (it's an instance method) */
				info->fi_type = type;
				info->fi_name = attr->ca_name;
				info->fi_doc  = attr->ca_doc;
				info->fi_attr = attr;
				Dee_Incref(type);
				Dee_Incref(attr->ca_name);
				Dee_XIncref(attr->ca_doc);
				return 0;
			}
			for (i = 0; i <= desc->cd_cattr_mask; ++i) {
				struct Dee_class_attribute *attr;
				attr = &desc->cd_cattr_list[i];
				if (!attr->ca_name)
					continue;
				if (addr < attr->ca_addr)
					continue;
				if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FGETSET) {
					if (addr > ((attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY) ? attr->ca_addr : attr->ca_addr + 2))
						continue;
					info->fi_getset = (uint16_t)(addr - attr->ca_addr);
				} else {
					if (addr > attr->ca_addr)
						continue;
				}
				/* Found it! (it's a class method) */
				info->fi_type = type;
				info->fi_name = attr->ca_name;
				info->fi_doc  = attr->ca_doc;
				info->fi_attr = attr;
				Dee_Incref(type);
				Dee_Incref(attr->ca_name);
				Dee_XIncref(attr->ca_doc);
				return 0;
			}
			/* Check if we can find the address as an operator binding. */
			for (i = 0; i <= desc->cd_clsop_mask; ++i) {
				struct Dee_class_operator *op;
				op = &desc->cd_clsop_list[i];
				if (op->co_name == (Dee_operator_t)-1)
					continue;
				if (op->co_addr != addr)
					continue;
				/* Found it! */
				info->fi_type  = type;
				info->fi_clsop = op;
				Dee_Incref(type);
				return 0;
			}
			Dee_class_desc_lock_read(my_class);
		}
	}
	Dee_class_desc_lock_endread(my_class);
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
lookup_code_info(/*[in]*/ DeeCodeObject *code,
                 /*[in]*/ Function *function,
                 /*[out]*/ struct Dee_function_info *__restrict info) {
	DeeModuleObject *mod;
	uint16_t addr;
	int result;
	info->fi_mod    = NULL;
	info->fi_modsym = NULL;
	info->fi_type   = NULL;
	info->fi_name   = NULL;
	info->fi_doc    = NULL;
	info->fi_clsop  = NULL;
	info->fi_getset = (uint16_t)-1;

	/* Step #1: Search the code object's module for the given `function' */
	mod = code->co_module;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if unlikely(!mod) {
		mod = DeeModule_OfPointer(code);
		if (!mod)
			goto without_module;
	} else {
		Dee_Incref(mod);
	}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if unlikely(!mod)
		goto without_module;
	if unlikely(DeeInteractiveModule_Check(mod))
		goto without_module;
	Dee_Incref(mod);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	info->fi_mod = mod; /* Inherit reference */
	DeeModule_LockRead(mod);
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		if (!mod->mo_globalv[addr])
			continue;
		if (mod->mo_globalv[addr] == Dee_AsObject(function) ||
		    (DeeFunction_Check(mod->mo_globalv[addr]) &&
		     DeeFunction_CODE(mod->mo_globalv[addr]) == code)) {
			struct Dee_module_symbol *function_symbol;
			DeeModule_LockEndRead(mod);
			function_symbol = DeeModule_GetSymbolID(mod, addr);
			if (function_symbol) {
				/* Found it! (it's a global) */
				info->fi_modsym = function_symbol;
				info->fi_name = Dee_module_symbol_getnameobj(function_symbol);
				if unlikely(!info->fi_name)
					goto err_mod;
				if (function_symbol->ss_doc) {
					info->fi_doc = Dee_module_symbol_getdocobj(function_symbol);
					if unlikely(!info->fi_doc)
						goto err_mod_name;
				}
				return 0;
			}
			DeeModule_LockRead(mod);
		}
	}

	/* Do another pass, this time looking for class objects
	 * which the function may be defined to be apart of. */
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		DeeObject *glob = mod->mo_globalv[addr];
		if (!glob)
			continue;
		if (!DeeType_Check(glob))
			continue;
		if (!DeeType_IsClass(glob))
			continue;
		Dee_Incref(glob);
		DeeModule_LockEndRead(mod);
		result = lookup_code_info_in_class((DeeTypeObject *)glob, code, function, info);
		Dee_Decref(glob);
		if (result <= 0) {
			if (result < 0)
				Dee_Decref(mod);
			return result;
		}
		DeeModule_LockRead(mod);
	}
	DeeModule_LockEndRead(mod);
without_module:
	if (function) {
		/* Search though the function's references for class types, and
		 * check if the given code object may be referring to one of them. */
		uint16_t i, count = code->co_refc;
		for (i = 0; i < count; ++i) {
			DeeObject *ref = function->fo_refv[i];
			if (!ref)
				continue;
			if (!DeeType_Check(ref))
				continue;
			if (!DeeType_IsClass(ref))
				continue;
			result = lookup_code_info_in_class((DeeTypeObject *)ref, code, function, info);
			if (result <= 0) {
				if (result < 0)
					Dee_XDecref(info->fi_mod);
				return result;
			}
		}
	}

	/* If we still haven't found anything about the function it's
	 * probably been locally created as part of a caller's stack-frame.
	 * That, or it's been bound externally, before being deleted from its
	 * declaration module.
	 * In either case: we probably won't be able to find it, so the best
	 * we can do is to check what kind of DDI information is provided by
	 * the function's code. */
	{
		char const *name = DeeCode_NAME(code);
		if (name) {
			/* Well... At least we got the name. - That's something. */
			info->fi_name = (DREF DeeStringObject *)DeeString_New(name);
			if unlikely(!info->fi_name)
				goto err_mod;
			return 0;
		}
	}
	Dee_XDecref(info->fi_mod);
	return 1;
err_mod_name:
	Dee_Decref(info->fi_name);
err_mod:
	Dee_XDecref(info->fi_mod);
/*err:*/
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeCode_GetInfo(/*Code*/ DeeObject *__restrict self,
                /*[out]*/ struct Dee_function_info *__restrict info) {
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeCode_Type);
	return lookup_code_info((DeeCodeObject *)self, NULL, info);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeFunction_GetInfo(/*Function*/ DeeObject *__restrict self,
                    /*[out]*/ struct Dee_function_info *__restrict info) {
	Function *me = (Function *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeFunction_Type);
	return lookup_code_info(me->fo_code, me, info);
}





PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_New(DeeCodeObject *code, size_t refc,
                DeeObject *const *refv) {
	DREF Function *result;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERTF(code->co_refc == refc,
	        "code->co_refc = %" PRFu16 "\n"
	        "refc          = %" PRFu16 "\n"
	        "name          = %s\n",
	        code->co_refc, refc,
	        DeeCode_NAME(code));
	ASSERT(code->co_refstaticc >= refc);
	if likely(code->co_refstaticc == refc) {
		result = (DREF Function *)DeeGCObject_Mallocc(offsetof(Function, fo_refv),
		                                              refc, sizeof(DREF DeeObject *));
	} else {
		result = (DREF Function *)DeeGCObject_Callocc(offsetof(Function, fo_refv),
		                                              code->co_refstaticc, sizeof(DREF DeeObject *));
	}
	if unlikely(!result)
		goto done;
	result->fo_code = code;
	Dee_Incref(code);
#ifdef CONFIG_HAVE_CODE_METRICS
	atomic_inc(&code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_function_init(&result->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	Dee_atomic_rwlock_init(&result->fo_reflock);
	Dee_Movrefv(result->fo_refv, refv, refc);
	DeeObject_InitStatic(result, &DeeFunction_Type);
	result = DeeGC_TRACK(Function, result);
done:
	return Dee_AsObject(result);
}


INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeFunction_NewInherited(DeeCodeObject *code, size_t refc,
                         /*inherit(on_success)*/ DREF DeeObject *const *__restrict refv) {
	DREF Function *result;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERTF(code->co_refc == refc,
	        "code->co_refc = %" PRFu16 "\n"
	        "refc          = %" PRFu16 "\n"
	        "name          = %s\n",
	        code->co_refc, refc,
	        DeeCode_NAME(code));
	ASSERT(code->co_refstaticc >= refc);
	if likely(code->co_refstaticc == refc) {
		result = (DREF Function *)DeeGCObject_Mallocc(offsetof(Function, fo_refv),
		                                              refc, sizeof(DREF DeeObject *));
	} else {
		result = (DREF Function *)DeeGCObject_Callocc(offsetof(Function, fo_refv),
		                                              code->co_refstaticc, sizeof(DREF DeeObject *));
	}
	if unlikely(!result)
		goto done;
	result->fo_code = code;
	Dee_Incref(code);
#ifdef CONFIG_HAVE_CODE_METRICS
	atomic_inc(&code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_function_init(&result->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	Dee_atomic_rwlock_init(&result->fo_reflock);
	memcpyc(result->fo_refv, refv, refc, sizeof(DREF DeeObject *));
	DeeObject_InitStatic(result, &DeeFunction_Type);
	result = DeeGC_TRACK(Function, result);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_NewNoRefs(DeeCodeObject *__restrict code) {
	DREF Function *result;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(code->co_refc == 0);
	if likely(code->co_refstaticc == 0) {
		result = (DREF Function *)DeeGCObject_Malloc(offsetof(Function, fo_refv));
	} else {
		result = (DREF Function *)DeeGCObject_Callocc(offsetof(Function, fo_refv),
		                                              code->co_refstaticc,
		                                              sizeof(DREF DeeObject *));
	}
	if unlikely(!result)
		goto done;
	result->fo_code = code;
	Dee_Incref(code);
#ifdef CONFIG_HAVE_CODE_METRICS
	atomic_inc(&code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_function_init(&result->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	Dee_atomic_rwlock_init(&result->fo_reflock);
	DeeObject_InitStatic(result, &DeeFunction_Type);
	result = DeeGC_TRACK(Function, result);
done:
	return Dee_AsObject(result);
}


PRIVATE WUNUSED DREF Function *DCALL
function_init(size_t argc, DeeObject *const *argv) {
	DREF Function *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("Function", params: """
	DeeCodeObject *code = &DeeCode_Empty;
	DeeObject *refs     = Dee_EmptyTuple;
""", docStringPrefix: "function");]]]*/
#define function_Function_params "code:?Ert:Code=!Ert:Code_empty,refs=!T0"
	struct {
		DeeCodeObject *code;
		DeeObject *refs;
	} args;
	args.code = &DeeCode_Empty;
	args.refs = Dee_EmptyTuple;
	DeeArg_UnpackStruct0Or1Or2(err, argc, argv, "Function", &args, &args.code, &args.refs);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.code, &DeeCode_Type))
		goto err;
	ASSERT(args.code->co_refc <= args.code->co_refstaticc);
	if likely(args.code->co_refc == args.code->co_refstaticc) {
		result = (DREF Function *)DeeGCObject_Mallocc(offsetof(Function, fo_refv),
		                                              args.code->co_refc, sizeof(DREF DeeObject *));
	} else {
		result = (DREF Function *)DeeGCObject_Callocc(offsetof(Function, fo_refv),
		                                              args.code->co_refstaticc, sizeof(DREF DeeObject *));
	}
	if unlikely(!result)
		goto err;
	if (DeeSeq_Unpack(args.refs, args.code->co_refc, result->fo_refv))
		goto err_r;
	result->fo_code = args.code;
	Dee_Incref(args.code);
#ifdef CONFIG_HAVE_CODE_METRICS
	atomic_inc(&args.code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_function_init(&result->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	Dee_atomic_rwlock_init(&result->fo_reflock);
	DeeObject_InitStatic(result, &DeeFunction_Type);
	result = DeeGC_TRACK(Function, result);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}


PRIVATE struct type_member tpconst function_class_members[] = {
	TYPE_MEMBER_CONST("YieldFunction", &DeeYieldFunction_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_refs(Function *__restrict self) {
	return DeeRefVector_NewReadonly(self,
	                                self->fo_code->co_refc,
	                                self->fo_refv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_kwds(Function *__restrict self) {
	DeeCodeObject *code = self->fo_code;
	if likely(code->co_keywords) {
		return DeeRefVector_NewReadonly(Dee_AsObject(code),
		                                (size_t)code->co_argc_max,
		                                (DeeObject *const *)code->co_keywords);
	}
	if (code->co_argc_max == 0)
		return DeeSeq_NewEmpty();
	return DeeRT_ErrUnboundAttr(self, &str___kwds__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_kwds(Function *__restrict self) {
	DeeCodeObject *code = self->fo_code;
	return Dee_BOUND_FROMBOOL(code->co_keywords || code->co_argc_max == 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_name(Function *__restrict self) {
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	Dee_XDecref(info.fi_mod);
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_doc);
	if (info.fi_name)
		return Dee_AsObject(info.fi_name);
	DeeRT_ErrUnboundAttr(self, &str___name__);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_name(Function *__restrict self) {
	bool result;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	result = info.fi_name != NULL;
	Dee_function_info_fini(&info);
	return Dee_BOUND_FROMBOOL(result);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_doc(Function *__restrict self) {
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	Dee_XDecref(info.fi_mod);
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	if (info.fi_doc)
		return Dee_AsObject(info.fi_doc);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
function_get_type(Function *__restrict self) {
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	Dee_XDecref(info.fi_mod);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_type)
		return info.fi_type;
	DeeRT_ErrUnboundAttr(self, &str___type__);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_type(Function *__restrict self) {
	bool result;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	result = info.fi_type != NULL;
	Dee_function_info_fini(&info);
	return Dee_BOUND_FROMBOOL(result);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeAttributeObject *DCALL
function_get_attr(Function *__restrict self) {
	DREF DeeAttributeObject *result;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	if unlikely(!info.fi_name)
		goto err_unbound_info;
	result = DeeObject_MALLOC(DeeAttributeObject);
	if unlikely(!result)
		goto err_info;
	if (info.fi_modsym) {
		ASSERT(info.fi_mod);
		Dee_Incref(info.fi_mod);
		result->a_desc.ad_info.ai_decl = Dee_AsObject(info.fi_mod);
		result->a_desc.ad_info.ai_type = Dee_ATTRINFO_MODSYM;
		result->a_desc.ad_info.ai_value.v_modsym = info.fi_modsym;
		result->a_desc.ad_perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL;
	} else if (info.fi_attr) {
		DeeClassDescriptorObject *desc;
		ASSERT(info.fi_type);
		Dee_Incref(info.fi_type);
		result->a_desc.ad_info.ai_decl = Dee_AsObject(info.fi_type);
		result->a_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR;
		result->a_desc.ad_info.ai_value.v_attr = info.fi_attr;
		ASSERT(DeeType_IsClass(info.fi_type));
		desc = DeeClass_DESC(info.fi_type)->cd_desc;
		result->a_desc.ad_perm = Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL;
		if (info.fi_attr >= (desc->cd_iattr_list) &&
		    info.fi_attr <= (desc->cd_iattr_list + desc->cd_iattr_mask)) {
			result->a_desc.ad_perm = Dee_ATTRPERM_F_IMEMBER;
		} else {
			result->a_desc.ad_perm = Dee_ATTRPERM_F_CMEMBER;
		}
	} else if (info.fi_clsop) {
		/* XXX: How should this be represented? */
		goto err_unbound_info_r;
	} else {
		goto err_unbound_info_r;
	}

	DeeObject_Init(result, &DeeAttribute_Type);
	result->a_desc.ad_type = NULL;
	Dee_Incref(info.fi_name);
	result->a_desc.ad_name = DeeString_STR(info.fi_name);
	result->a_desc.ad_perm |= Dee_ATTRPERM_F_NAMEOBJ;
	result->a_desc.ad_doc = NULL;
	if (info.fi_doc) {
		Dee_Incref(info.fi_doc);
		result->a_desc.ad_doc = DeeString_STR(info.fi_doc);
		result->a_desc.ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
	}
	Dee_function_info_fini(&info);
	return result;
err_unbound_info_r:
	DeeObject_FREE(result);
err_unbound_info:
	DeeRT_ErrUnboundAttr(self, &str___attr__);
err_info:
	Dee_function_info_fini(&info);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_attr(Function *__restrict self) {
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	if (info.fi_modsym) {
		/* ... */
	} else if (info.fi_attr) {
		/* ... */
	} else if (info.fi_clsop) {
		/* XXX: How should this be represented? */
		goto unbound_info;
	} else {
		goto unbound_info;
	}
	Dee_function_info_fini(&info);
	return Dee_BOUND_YES;
unbound_info:
	Dee_function_info_fini(&info);
/*unbound:*/
	return Dee_BOUND_NO;
err:
	return Dee_BOUND_ERR;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_module(Function *__restrict self) {
	if likely(self->fo_code->co_module)
		return_reference(self->fo_code->co_module);
	/* Shouldn't happen... */
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_module(Function *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->fo_code->co_module != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_operator(Function *__restrict self) {
	Dee_operator_t opname = (Dee_operator_t)-1;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	if (info.fi_clsop)
		opname = info.fi_clsop->co_name;
	Dee_function_info_fini(&info);
	if (opname != (Dee_operator_t)-1)
		return DeeInt_NewUInt16(opname);
	DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__operator__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_operator(Function *__restrict self) {
	bool result;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	result = info.fi_clsop != NULL;
	Dee_function_info_fini(&info);
	return Dee_BOUND_FROMBOOL(result);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_operatorname(Function *__restrict self) {
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	Dee_XDecref(info.fi_mod);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_clsop != NULL) {
		DREF DeeObject *result;
		struct Dee_opinfo const *op;
		op = DeeTypeType_GetOperatorById(info.fi_type ? Dee_TYPE(info.fi_type)
		                                              : &DeeType_Type,
		                                 info.fi_clsop->co_name);
		result = op ? DeeString_New(op->oi_sname)
		            : DeeInt_NewUInt16(info.fi_clsop->co_name);
		Dee_XDecref(info.fi_type);
		return result;
	}
	DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__operatorname__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_property(Function *__restrict self) {
	uint16_t getset;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	getset = info.fi_getset;
	Dee_function_info_fini(&info);
	if (getset != (uint16_t)-1)
		return DeeInt_NewUInt16(getset);
	DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__property__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_bound_property(Function *__restrict self) {
	bool result;
	struct Dee_function_info info;
	if (DeeFunction_GetInfo(Dee_AsObject(self), &info) < 0)
		goto err;
	result = info.fi_getset != (uint16_t)-1;
	Dee_function_info_fini(&info);
	return Dee_BOUND_FROMBOOL(result);
err:
	return Dee_BOUND_ERR;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL code_getdefaults(DeeCodeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL code_getconstants(DeeCodeObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_defaults(Function *__restrict self) {
	return code_getdefaults(self->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_constants(Function *__restrict self) {
	return code_getconstants(self->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_argc_min(Function *__restrict self) {
	return DeeInt_NewUInt16(self->fo_code->co_argc_min);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_argc_max(Function *__restrict self) {
	return DeeInt_NewUInt16(self->fo_code->co_argc_max);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_hasvarargs(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FVARARGS);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_hasvarkwds(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FVARKWDS);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_isyielding(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FYIELDING);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_iscopyable(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FCOPYABLE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_isthiscall(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FTHISCALL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_hasassembly(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FASSEMBLY);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_islenient(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FLENIENT);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_hasheapframe(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FHEAPFRAME);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_hasfinally(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FFINALLY);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_isconstructor(Function *__restrict self) {
	return_bool(self->fo_code->co_flags & Dee_CODE_FCONSTRUCTOR);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_nlocal(Function *__restrict self) {
	return DeeInt_NewUInt16(self->fo_code->co_localc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_nconst(Function *__restrict self) {
	return DeeInt_NewUInt16(self->fo_code->co_constc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_nref(Function *__restrict self) {
	return DeeInt_NewUInt16(self->fo_code->co_refc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_nexcept(Function *__restrict self) {
	return DeeInt_NewUInt16(self->fo_code->co_exceptc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_code_nstatic(Function *__restrict self) {
	DeeCodeObject *code = self->fo_code;
	ASSERT(code->co_refstaticc >= code->co_refc);
	return DeeInt_NewUInt16(code->co_refstaticc - code->co_refc);
}


DOC_REF(code_optimize_doc);
DOC_REF(code_optimized_doc);

INTDEF NONNULL((1)) DREF DeeObject *DCALL
function_optimize(DeeFunctionObject *__restrict self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw);
INTDEF NONNULL((1)) DREF DeeObject *DCALL
function_optimized(DeeFunctionObject *__restrict self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw);

PRIVATE struct type_method tpconst function_methods[] = {
	TYPE_KWMETHOD("optimize", &function_optimize, DOC_GET(code_optimize_doc)),
	TYPE_KWMETHOD("optimized", &function_optimized, DOC_GET(code_optimized_doc)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst function_getsets[] = {
	TYPE_GETTER_BOUND_F(STR___name__, &function_get_name, &function_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the name of @this ?."),
	TYPE_GETTER_AB_F(STR___doc__, &function_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Returns the documentation string of @this ?., or ?N if there is none"),
	TYPE_GETTER_BOUND_F(STR___type__, &function_get_type, &function_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Try to determine if @this ?. is defined as part of a user-defined class, "
	                    /**/ "and if it is, return that class type, or throw :UnboundAttribute if that "
	                    /**/ "class couldn't be found, or if @this ?. is defined as stand-alone"),
	TYPE_GETTER_BOUND_F(STR___attr__, &function_get_attr, &function_bound_attr,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DAttribute\n"
	                    "#t{UnboundAttribute}"
	                    "Return an attribute descriptor for @this ?."),
	TYPE_GETTER_BOUND_F(STR___module__, &function_get_module, &function_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Return the module as part of which @this ?.'s code was originally written"),
	TYPE_GETTER_BOUND_F("__operator__", &function_get_operator, &function_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Try to determine if @this ?. is defined as part of a user-defined class, "
	                    /**/ "and if so, if it is used to define an operator callback. If that is the case, "
	                    /**/ "return the internal ID of the operator that @this ?. provides, or throw "
	                    /**/ ":UnboundAttribute if that class couldn't be found, @this ?. is defined "
	                    /**/ "as stand-alone, or defined as a class- or instance-method"),
	TYPE_GETTER_BOUND_F("__operatorname__", &function_get_operatorname, &function_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?X2?Dstring?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Same as ?#__operator__, but instead try to return the unambiguous name of the "
	                    /**/ "operator, though still return its ID if the operator isn't recognized as being "
	                    /**/ "part of the standard"),
	TYPE_GETTER_BOUND_F("__property__", &function_get_property, &function_bound_property,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Returns an integer describing the kind if @this ?. is part of a property or getset, "
	                    /**/ "or throw :UnboundAttribute if the function's property could not be found, or if the "
	                    /**/ "function isn't declared as a property callback\n"
	                    "#T{Id|Callback|Compatible prototype~"
	                    /**/ "$" PP_STR(Dee_CLASS_GETSET_GET) "|Getter callback|${get(): Object}&"
	                    /**/ "$" PP_STR(Dee_CLASS_GETSET_DEL) "|Delete callback|${del(): none}&"
	                    /**/ "$" PP_STR(Dee_CLASS_GETSET_SET) "|Setter callback|${set(value: Object): none}"
	                    "}"),
	TYPE_GETTER_AB_F("__refs__", &function_get_refs, METHOD_FCONSTCALL,
	                 "->?S?O\n"
	                 "Returns a sequence of all of the references used by @this ?."),
	TYPE_GETTER_AB_F("__statics__", &DeeFunction_GetStaticsWrapper, METHOD_FCONSTCALL,
	                 "->?S?O\n"
	                 "Returns a (writable) sequence of all of the static variables that appear in @this ?."),
	TYPE_GETTER_AB_F("__refsbyname__", &DeeFunction_GetRefsByNameWrapper, METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Returns a read-only mapping to access ?#__refs__ by their name "
	                 /**/ "(requires debug information to be present)"),
	TYPE_GETTER_AB_F("__staticsbyname__", &DeeFunction_GetStaticsByNameWrapper, METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Returns a writable mapping to access ?#__statics__ by their name "
	                 /**/ "(requires debug information to be present)"),
	TYPE_GETTER_AB_F("__symbols__", &DeeFunction_GetSymbolsByNameWrapper, METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "The combination of ?#__refsbyname__ and ?#__staticsbyname__, allowing "
	                 /**/ "access to all named symbol that need to maintain their value across "
	                 /**/ "different calls to ?. (requires debug information to be present)"),
	TYPE_GETTER_AB_F("__nstatic__", &function_get_code_nstatic,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Number of static variables during execution"),
	TYPE_GETTER_BOUND_F(STR___kwds__, &function_get_kwds, &function_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns a sequence of keyword argument names accepted by @this ?.\n"
	                    "If @this ?. doesn't accept keyword arguments, throw :UnboundAttribute"),
	TYPE_GETTER_AB_F("__defaults__", &function_get_code_defaults, METHOD_FCONSTCALL,
	                 "->?S?O\n"
	                 "Access to the default values of arguments"),
	TYPE_GETTER_AB_F("__constants__", &function_get_code_constants, METHOD_FCONSTCALL,
	                 "->?S?O\n"
	                 "Access to the constants of @this ?."),
	TYPE_GETTER_AB_F("__argc_min__", &function_get_code_argc_min, METHOD_FCONSTCALL,
	                 "->?Dint\n"
	                 "Min amount of arguments required to execute @this ?."),
	TYPE_GETTER_AB_F("__argc_max__", &function_get_code_argc_max, METHOD_FCONSTCALL,
	                 "->?Dint\n"
	                 "Max amount of arguments accepted by @this ?. (excluding a varargs or varkwds argument)"),
	TYPE_GETTER_AB_F("isyielding", &function_get_code_isyielding,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if calls to @this ?. produce yield-functions"),
	TYPE_GETTER_AB_F("iscopyable", &function_get_code_iscopyable,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if yield-function iterators of @this ?. are copyable (as per ${@[copyable]})"),
	TYPE_GETTER_AB_F("hasvarargs", &function_get_code_hasvarargs,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if @this ?. accepts variable arguments as overflow"),
	TYPE_GETTER_AB_F("hasvarkwds", &function_get_code_hasvarkwds,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if @this ?. accepts variable keyword arguments as overflow"),
	TYPE_GETTER_AB_F("__isthiscall__", &function_get_code_isthiscall,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if @this ?. takes an extra leading $this-argument"),
	TYPE_GETTER_AB_F("__hasassembly__", &function_get_code_hasassembly,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if assembly of @this ?. is executed in safe-mode"),
	TYPE_GETTER_AB_F("__islenient__", &function_get_code_islenient,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if the runtime stack allocation allows for leniency"),
	TYPE_GETTER_AB_F("__hasheapframe__", &function_get_code_hasheapframe,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "Check if the runtime stack-frame must be allocated on the heap"),
	TYPE_GETTER_AB_F("__hasfinally__", &function_get_code_hasfinally,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "True if execution will jump to the nearest finally-block when a return instruction is encountered\n"
	                 "Note that this does not necessarily guaranty, or deny the presence of a try...finally statement in the "
	                 /**/ "user's source code, as the compiler may try to optimize this flag away to speed up runtime execution"),
	TYPE_GETTER_AB_F("__isconstructor__", &function_get_code_isconstructor,
	                 METHOD_FNOREFESCAPE | METHOD_FNOTHROW | METHOD_FCONSTCALL,
	                 "->?Dbool\n"
	                 "True for class constructor ?. objects. - When set, don't include the this-argument in "
	                 /**/ "tracebacks, thus preventing incomplete instances from being leaked when the constructor "
	                 /**/ "causes some sort of exception to be thrown"),
	TYPE_GETTER_AB_F("__nlocal__", &function_get_code_nlocal,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Number of available local variables during execution"),
	TYPE_GETTER_AB_F("__nconst__", &function_get_code_nconst,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Number of constant objects during execution"),
	TYPE_GETTER_AB_F("__nref__", &function_get_code_nref,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Number of referenced objects during execution"),
	TYPE_GETTER_AB_F("__nexcept__", &function_get_code_nexcept,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Number of exception handlers"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst function_members[] = {
	TYPE_MEMBER_FIELD_DOC("__code__", STRUCT_OBJECT_AB, offsetof(Function, fo_code), "->?Ert:Code"),
	TYPE_MEMBER_END
};


PRIVATE NONNULL((1)) void DCALL
function_fini(Function *__restrict self) {
	/* TODO: There should be a sub-class of ref-objects
	 *       that we don't actually hold a proper DREF to.
	 * We could use those slots to reference the declaring
	 * class in the case of thiscall functions, as used by
	 * the `getmember' instruction set:
	 *   - `DeeObject_InstanceOf(thisarg, MyClass)' fails safely,
	 *     even when `MyClass' has already been destroyed.
	 *     XXX: But what if `MyClass' was free'd, and alloc'd again?
	 *          In that case, `thisarg' could be an instance of the
	 *          new `MyClass', and we'd be none the wiser...
	 *          To solve this, we'd need a proper `weakref'...
	 *   - Assuming that `DeeObject_InstanceOf(thisarg, MyClass)'
	 *     succeeds, we know that `MyClass' will remain alive as long
	 *     as `thisarg' remains alive, too. With this knowledge, we
	 *     can keep on accessing the `MyClass' type object without
	 *     holding our own, dedicated reference to it!
	 *
	 * Why all this? - Because right now we rely on GC to kill the
	 * reference loop within `MyClass -> Member-function -> refs -> MyClass',
	 * and while that works, it's really inefficient, and most importantly:
	 * doesn't automatically free the class as soon as possible, but instead
	 * waits until GC is invoked. - And this in turn results in some really
	 * noticeable effects if static class members were to be used, as those
	 * only get decref'd once the class itself gets killed, too...
	 *
	 * If this was implemented, the only spot where the user can create a
	 * function with a dead type-reference would be:
	 * >> function makeUnboundClassFunction() {
	 * >>     class MyClass {
	 * >>         final member bar = 42;
	 * >>         function foo() {
	 * >>             return bar;
	 * >>         }
	 * >>     }
	 * >>     return MyClass.foo;
	 * >> }
	 * >> local x = makeUnboundClassFunction();
	 * >> local y = x(Object());
	 *
	 * FIXME: Currently, the above code crashes in `DeeInstance_GetMember()'! (very bad)
	 *        All of this could be fixed if DeeFunctionObject had a weakref
	 *        member that is a pointer to the required thisarg type.
	 * This field could then be accessible via a new instruction set:
	 *   - Added instructions:
	 *     - `push this_class'
	 *     - `push super this, super_class'                        (uses DeeType_Base(this_class))
	 *     - `push getcmember this_class, $<imm8>'
	 *     - `push getcmember this_class, $<imm16>'
	 *     - `push callcmember this, this_class, $<imm8>, #<imm8>'
	 *     - `push callcmember this, this_class, $<imm16>, #<imm8>'
	 *     - `push getattr this, super_class, const <imm8>'
	 *     - `push getattr this, super_class, const <imm16>'
	 *     - `push callattr this, super_class, const <imm8>, #<imm8>'
	 *     - `push callattr this, super_class, const <imm16>, #<imm8>'
	 *     - `push boundmember this, this_class, $<imm8>'
	 *     - `push boundmember this, this_class, $<imm16>'
	 *     - `push getmember this, this_class, $<imm8>'
	 *     - `push getmember this, this_class, $<imm16>'
	 *     - `delmember this, this_class, $<imm8>'
	 *     - `delmember this, this_class, $<imm16>'
	 *     - `setmember this, this_class, $<imm8>, pop'
	 *     - `setmember this, this_class, $<imm16>, pop'
	 *   - Removed instructions:
	 *     - `push super this, ref <imm8/16>'
	 *     - `push getcmember ref <imm8/16>, $<imm8>'
	 *     - `push getcmember ref <imm8/16>, $<imm16>'
	 *     - `push callcmember this, ref <imm8/16>, $<imm8>, #<imm8>'
	 *     - `push callcmember this, ref <imm8/16>, $<imm16>, #<imm8>'
	 *     - `push getattr this, ref <imm8>, const <imm8>'
	 *     - `push getattr this, ref <imm16>, const <imm16>'
	 *     - `push callattr this, ref <imm8>, const <imm8>, #<imm8>'
	 *     - `push callattr this, ref <imm16>, const <imm16>, #<imm8>'
	 *     - `push boundmember this, ref <imm8/16>, $<imm8>'
	 *     - `push boundmember this, ref <imm8/16>, $<imm16>'
	 *     - `push getmember this, ref <imm8/16>, $<imm8>'
	 *     - `push getmember this, ref <imm8/16>, $<imm16>'
	 *     - `delmember this, ref <imm8/16>, $<imm8>'
	 *     - `delmember this, ref <imm8/16>, $<imm16>'
	 *     - `setmember this, ref <imm8/16>, $<imm8>, pop'
	 *     - `setmember this, ref <imm8/16>, $<imm16>, pop'
	 * Additionally, when performing a thiscall, check that the
	 * this-argument is actually an instance of `this_class'!
	 * However, this check may be skipped in the case of an attribute
	 * call (as in `foo.fun()', as opposed to `type(foo).fun(foo)')
	 */
	DeeCodeObject *code;
	DREF DeeObject **refv_iter;
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	if (self->fo_hostasm.hafu_data)
		Dee_hostasm_function_data_destroy(self->fo_hostasm.hafu_data);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	code = self->fo_code;
	refv_iter = Dee_Decprefv(self->fo_refv, code->co_refc);
	ASSERT(code->co_refstaticc >= code->co_refc);
	if unlikely(code->co_refstaticc > code->co_refc) {
		uint16_t i, n = (uint16_t)(code->co_refstaticc - code->co_refc);
		for (i = 0; i < n; ++i) {
			DREF DeeObject *ob = refv_iter[i];
			if (ITER_ISOK(ob))
				Dee_Decref(ob);
		}
	}
	Dee_Decref(code);
}

PRIVATE NONNULL((1, 2)) void DCALL
function_visit(Function *__restrict self,
               Dee_visit_t proc, void *arg) {
	size_t i;
	for (i = 0; i < self->fo_code->co_refc; ++i)
		Dee_Visit(self->fo_refv[i]);
	DeeFunction_RefLockRead(self);
	for (; i < self->fo_code->co_refstaticc; ++i) {
		DeeObject *ob = self->fo_refv[i];
		if (ITER_ISOK(ob))
			Dee_Visit(ob);
	}
	DeeFunction_RefLockEndRead(self);
	Dee_Visit(self->fo_code);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
function_serialize(Function *__restrict self, DeeSerial *__restrict writer) {
	Function *out;
	size_t i, refc = self->fo_code->co_refstaticc;
	size_t sizeof_function = offsetof(Function, fo_refv) + refc * sizeof(DREF DeeObject *);
	Dee_seraddr_t addr = DeeSerial_GCObject_Malloc(writer, sizeof_function, self);
	if (!Dee_SERADDR_ISOK(addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, Function);
	(void)out;
	Dee_atomic_rwlock_init(&out->fo_reflock);
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	bzero(&out->fo_hostasm, sizeof(out->fo_hostasm)); /* Don't serialize hostasm */
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
	if (DeeSerial_PutObject(writer, addr + offsetof(Function, fo_code), self->fo_code))
		goto err;
#define addrof_out_refv(i) (addr + offsetof(Function, fo_refv) + (i) * sizeof(DREF DeeObject *))
	/* Constant references... */
	for (i = 0; i < self->fo_code->co_refc; ++i) {
		if (DeeSerial_PutObject(writer, addrof_out_refv(i), self->fo_refv[i]))
			goto err;
	}

	/* Static variables... */
	for (; i < refc; ++i) {
		DREF DeeObject *value;
		DeeFunction_RefLockRead(self);
		value = self->fo_refv[i];
		Dee_XIncref(value);
		DeeFunction_RefLockEndRead(self);
		if (DeeSerial_XPutObjectInherited(writer, addrof_out_refv(i), value))
			goto err;
	}
#undef addrof_out_refv
	return addr;
err:
	return Dee_SERADDR_INVALID;
}


#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
function_print(Function *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	char const *name = DeeCode_NAME(self->fo_code);
	Dee_ssize_t temp, result;
	uint16_t i;
	if (name != NULL)
		return DeeFormat_Printf(printer, arg, "<Function %s>", name);
	result = DeeFormat_PRINT(printer, arg, "<Function(");
	if unlikely(result < 0)
		goto done;
	DO(err, DeeFormat_PrintObjectRepr(printer, arg, Dee_AsObject(self->fo_code)));
	if (self->fo_code->co_refc > 0) {
		DO(err, DeeFormat_PRINT(printer, arg, ", { "));
		for (i = 0; i < self->fo_code->co_refc; ++i) {
			if (i != 0)
				DO(err, DeeFormat_PRINT(printer, arg, ", "));
			DO(err, DeeFormat_PrintObjectRepr(printer, arg, self->fo_refv[i]));
		}
		DO(err, DeeFormat_PRINT(printer, arg, " }"));
	}
	DO(err, DeeFormat_PRINT(printer, arg, ">"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
function_printrepr(Function *__restrict self,
                   Dee_formatprinter_t printer, void *arg) {
	char const *name = DeeCode_NAME(self->fo_code);
	if (name != NULL)
		return DeeFormat_PrintStr(printer, arg, name);
	/* TODO: Print representation as java-style lambda: "(a, b, c) -> ..."
	 * NOTE: For this purpose, also print default arguments (where present) */
	/* TODO: Print the current values of static variables? */
	return function_print(self, printer, arg);
}

PRIVATE NONNULL((1)) void DCALL
function_clear(DeeFunctionObject *__restrict self) {
	DeeCodeObject *code = self->fo_code;
	DREF DeeObject *buffer[16];
	size_t i, bufi = 0;
	/* Clear out static variables. */
restart:
	DeeFunction_RefLockWrite(self);
	for (i = code->co_refc; i < code->co_refstaticc; ++i) {
		DREF DeeObject *ob = self->fo_refv[i];
		if (!ITER_ISOK(ob))
			continue;

		/* Don't clear out simple primitives */
		if (DeeNone_Check(ob))
			continue;
		if (DeeString_Check(ob))
			continue;
		if (DeeInt_Check(ob))
			continue;
		if (DeeBool_Check(ob))
			continue;

		/* Clear out this static variable */
		self->fo_refv[i] = NULL;
		DeeFutex_WakeAll(&self->fo_refv[i]);
		if (!Dee_DecrefIfNotOne(ob)) {
			buffer[bufi] = ob; /* Inherit reference */
			++bufi;
			if (bufi >= COMPILER_LENOF(buffer)) {
				DeeFunction_RefLockEndWrite(self);
				Dee_Decrefv(buffer, bufi);
				bufi = 0;
				goto restart;
			}
		}
	}
	DeeFunction_RefLockEndWrite(self);
	Dee_Decrefv(buffer, bufi);
}

PRIVATE struct type_gc tpconst function_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&function_clear
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
function_hash(Function *__restrict self) {
	DeeCodeObject *code = self->fo_code;
	Dee_hash_t result;
	result = DeeObject_Hash(code);
	result = Dee_HashCombine(result, DeeObject_Hashv(self->fo_refv, code->co_refc));
	if unlikely(code->co_refstaticc > code->co_refc) {
		uint16_t i;
		for (i = code->co_refc; i < code->co_refstaticc; ++i) {
			DREF DeeObject *ob;
			DeeFunction_RefLockRead(self);
			ob = self->fo_refv[i];
			if (!ITER_ISOK(ob)) {
				DeeFunction_RefLockEndRead(self);
				result = Dee_HashCombine(result, Dee_HASHOF_UNBOUND_ITEM);
			} else {
				Dee_Incref(ob);
				DeeFunction_RefLockEndRead(self);
				result = Dee_HashCombine(result, DeeObject_HashInherited(ob));
			}
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
function_compare_eq(Function *self, Function *other) {
	DeeCodeObject *code = self->fo_code;
	uint16_t i;
	int result;
	if (DeeObject_AssertTypeExact(other, &DeeFunction_Type))
		goto err;
	result = DeeObject_CompareEq(Dee_AsObject(code),
	                             Dee_AsObject(other->fo_code));
	if (Dee_COMPARE_ISNE_OR_ERR(result))
		goto done;
	ASSERT(code->co_refc == other->fo_code->co_refc);
	for (i = 0; i < code->co_refc; ++i) {
		result = DeeObject_TryCompareEq(self->fo_refv[i],
		                                other->fo_refv[i]);
		if (Dee_COMPARE_ISNE_OR_ERR(result))
			goto done;
	}
	ASSERT(code->co_refstaticc == other->fo_code->co_refstaticc);
	for (; i < code->co_refstaticc; ++i) {
		DREF DeeObject *lhs, *rhs;
		DeeFunction_RefLockRead(self);
		lhs = self->fo_refv[i];
		if (ITER_ISOK(lhs))
			Dee_Incref(lhs);
		DeeFunction_RefLockEndRead(self);
		DeeFunction_RefLockRead(other);
		rhs = other->fo_refv[i];
		if (ITER_ISOK(rhs))
			Dee_Incref(rhs);
		DeeFunction_RefLockEndRead(other);
		if (lhs == rhs) {
			if (ITER_ISOK(lhs))
				Dee_Decref_n(lhs, 2);
		} else if (!ITER_ISOK(lhs) || !ITER_ISOK(rhs)) {
			if (ITER_ISOK(lhs))
				Dee_Decref_unlikely(lhs);
			if (ITER_ISOK(rhs))
				Dee_Decref_unlikely(rhs);
			goto not_equal;
		} else {
			result = DeeObject_TryCompareEq(lhs, rhs);
			Dee_Decref_unlikely(lhs);
			Dee_Decref_unlikely(rhs);
			if (Dee_COMPARE_ISNE_OR_ERR(result))
				goto done;
		}
	}
done:
	return result;
not_equal:
	return Dee_COMPARE_NE;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp function_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&function_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&function_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE struct type_callable function_callable = {
	/* .tp_call_kw     = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&DeeFunction_CallKw,
	/* .tp_thiscall    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&DeeFunction_ThisCall,
	/* .tp_thiscall_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&DeeFunction_ThisCallKw,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&DeeFunction_CallTuple,
	/* .tp_call_tuple_kw     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&DeeFunction_CallTupleKw,
	/* .tp_thiscall_tuple    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&DeeFunction_ThisCallTuple,
	/* .tp_thiscall_tuple_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&DeeFunction_ThisCallTupleKw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeFunction_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Function),
	/* .tp_doc      = */ DOC("(" function_Function_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FNAMEOBJECT | TP_FVARIABLE | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL, /* TODO */
			/* tp_copy_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ &function_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &function_serialize,
			/* tp_free:        */ NULL /* XXX: Use the tuple-allocator? (if somehow still possible with "CONFIG_EXPERIMENTAL_MMAP_DEC") */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&function_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&function_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&function_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&function_visit,
	/* .tp_gc            = */ &function_gc,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &function_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ function_methods,
	/* .tp_getsets       = */ function_getsets,
	/* .tp_members       = */ function_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ function_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&DeeFunction_Call,
	/* .tp_callable      = */ &function_callable,
};

PRIVATE NONNULL((1)) void DCALL
yf_fini(YFunction *__restrict self) {
	if (self->yf_kw) {
		Dee_Decref(self->yf_kw->fk_kw);
		if (self->yf_func->fo_code->co_flags & Dee_CODE_FVARKWDS) {
			if (self->yf_kw->fk_varkwds)
				DeeKwBlackList_Decref(self->yf_kw->fk_varkwds);
		}
		Dee_Free(self->yf_kw);
	}
	Dee_Decref(self->yf_func);
	Dee_Decrefv(self->yf_argv, self->yf_argc);
	Dee_XDecref(self->yf_this);
}

PRIVATE NONNULL((1, 2)) void DCALL
yf_visit(YFunction *__restrict self, Dee_visit_t proc, void *arg) {
	if (self->yf_kw) {
		Dee_Visit(self->yf_kw->fk_kw);
		if (self->yf_func->fo_code->co_flags & Dee_CODE_FVARKWDS)
			Dee_XVisit(self->yf_kw->fk_varkwds);
	}
	Dee_Visit(self->yf_func);
	Dee_Visitv(self->yf_argv, self->yf_argc);
	Dee_XVisit(self->yf_this);
}

PRIVATE WUNUSED DREF YFunction *DCALL yf_ctor(void) {
	DREF YFunction *result;
	result = (DREF YFunction *)DeeObject_Malloc(DeeYieldFunction_Sizeof(0));
	if unlikely(!result->yf_func)
		goto err;
	result->yf_func = function_init(0, NULL);
	if unlikely(!result->yf_func)
		goto err_r;
	result->yf_pargc = 0;
	result->yf_argc  = 0;
	result->yf_kw    = NULL;
	result->yf_this  = NULL;
	DeeObject_InitStatic(result, &DeeYieldFunction_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF YFunction *DCALL
yf_copy(YFunction *__restrict self) {
	DREF YFunction *result;
	struct Dee_code_frame_kwds *kw;
	result = (DREF YFunction *)DeeObject_Malloc(DeeYieldFunction_Sizeof(self->yf_argc));
	if unlikely(!result)
		goto err;
	result->yf_kw = NULL;
	if (self->yf_kw) {
		size_t count;
		ASSERT(self->yf_func->fo_code->co_argc_max >= self->yf_pargc);
		count = (self->yf_func->fo_code->co_argc_max - self->yf_pargc);
		kw = (struct Dee_code_frame_kwds *)Dee_Mallococ(offsetof(struct Dee_code_frame_kwds, fk_kargv),
		                                                count, sizeof(DeeObject *));
		if unlikely(!kw)
			goto err_r;
		result->yf_kw = kw;
		memcpyc(kw->fk_kargv, self->yf_kw->fk_kargv, count, sizeof(DeeObject *));
		kw->fk_kw = self->yf_kw->fk_kw;
		Dee_Incref(kw->fk_kw);
		if (self->yf_func->fo_code->co_flags & Dee_CODE_FVARKWDS)
			kw->fk_varkwds = NULL; /* Don't copy this one... */
	}
	result->yf_func  = self->yf_func;
	result->yf_pargc = self->yf_pargc;
	result->yf_argc  = self->yf_argc;
	result->yf_this  = self->yf_this;
	Dee_Incref(result->yf_func);
	Dee_Movrefv(result->yf_argv, self->yf_argv, self->yf_argc);
	Dee_XIncref(result->yf_this);
	DeeObject_InitStatic(result, &DeeYieldFunction_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfi_init(YFIterator *__restrict self,
         YFunction *__restrict yield_function) {
	DeeCodeObject *code;
	DBG_memset(&self->yi_frame, 0xcc, sizeof(struct Dee_code_frame));
	/* Setup the frame for the iterator. */
	Dee_Incref(yield_function); /* Reference stored in `self->yi_func' */
	self->yi_func          = yield_function;
	self->yi_frame.cf_func = yield_function->yf_func;
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	Dee_Incref(self->yi_frame.cf_func); /* Reference stored in here. */
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	code = self->yi_frame.cf_func->fo_code;
	/* Allocate memory for frame data. */
	self->yi_frame.cf_prev  = Dee_CODE_FRAME_NOT_EXECUTING;
	self->yi_frame.cf_frame = (DREF DeeObject **)Dee_Calloc(code->co_framesize);
	if unlikely(!self->yi_frame.cf_frame)
		goto err_r_base;
	self->yi_frame.cf_stack = self->yi_frame.cf_frame + code->co_localc;
	self->yi_frame.cf_sp    = self->yi_frame.cf_stack;
	self->yi_frame.cf_ip    = code->co_code;
	self->yi_frame.cf_kw    = yield_function->yf_kw;
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	self->yi_frame.cf_flags = code->co_flags;
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	self->yi_frame.cf_vargs = NULL;
	self->yi_frame.cf_argc  = yield_function->yf_pargc;
	self->yi_frame.cf_argv  = yield_function->yf_argv;
	self->yi_frame.cf_this  = yield_function->yf_this;
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	Dee_XIncref(self->yi_frame.cf_this);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	self->yi_frame.cf_stacksz = 0;
	Dee_rshared_rwlock_init(&self->yi_lock);
	return 0;
err_r_base:
	Dee_Decref(self->yi_frame.cf_func);
	Dee_Decref(yield_function);
/*err:*/
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF YFIterator *DCALL
yf_iter(YFunction *__restrict self) {
	DREF YFIterator *result;
	result = DeeGCObject_MALLOC(YFIterator);
	if unlikely(!result)
		goto err;
	if unlikely(yfi_init(result, self))
		goto err_r;
	DeeObject_InitStatic(result, &DeeYieldFunctionIterator_Type);
	return DeeGC_TRACK(YFIterator, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
yf_serialize(YFunction *__restrict self, DeeSerial *__restrict writer) {
	YFunction *out;
	size_t sizeof_self = _Dee_MallococBufsize(offsetof(YFunction, yf_argv),
	                                          self->yf_argc,
	                                          sizeof(DREF DeeObject *));
	Dee_seraddr_t out_addr = DeeSerial_Object_Malloc(writer, sizeof_self, self);
#define ADDROF(field) (out_addr + offsetof(YFunction, field))
	if unlikely(!Dee_SERADDR_ISOK(out_addr))
		goto err;

	/* Copy static fields */
	out = DeeSerial_Addr2Mem(writer, out_addr, YFunction);
	out->yf_pargc = self->yf_pargc;
	out->yf_argc  = self->yf_argc;
	out->yf_kw    = NULL;

	/* Copy object references */
	if (DeeSerial_PutObject(writer, ADDROF(yf_func), self->yf_func))
		goto err;
	if (DeeSerial_XPutObject(writer, ADDROF(yf_this), self->yf_this))
		goto err;
	if (DeeSerial_PutObjectv(writer, ADDROF(yf_argv), self->yf_argv, self->yf_argc))
		goto err;

	/* Copy keyword arguments (must happen last since may contain
	 * weak references to objects actually owned by `yf_argv') */
	if (self->yf_kw) {
		struct Dee_code_frame_kwds *kw = self->yf_kw;
		size_t i, count;
		Dee_seraddr_t out_kw_addr;
		size_t sizeof_out_kw;
		ASSERT(self->yf_func->fo_code->co_argc_max >= self->yf_pargc);
		count = (self->yf_func->fo_code->co_argc_max - self->yf_pargc);
		sizeof_out_kw = _Dee_MallococBufsize(offsetof(struct Dee_code_frame_kwds, fk_kargv),
		                                     count, sizeof(DeeObject *));
		out_kw_addr = DeeSerial_Malloc(writer, sizeof_out_kw, kw);
		if unlikely(!Dee_SERADDR_ISOK(out_kw_addr))
			goto err;
#define KW_ADDROF(field) (out_kw_addr + offsetof(struct Dee_code_frame_kwds, field))
		if (self->yf_func->fo_code->co_flags & Dee_CODE_FVARKWDS) {
			struct Dee_code_frame_kwds *out_kw;
			out_kw = DeeSerial_Addr2Mem(writer, out_kw_addr, struct Dee_code_frame_kwds);
			out_kw->fk_varkwds = NULL; /* Don't copy this one... */
		}
		if (DeeSerial_PutObject(writer, KW_ADDROF(fk_kw), kw->fk_kw))
			goto err;

		/* Copy `fk_kargv' */
		for (i = 0; i < count; ++i) {
			DeeObject *ob = kw->fk_kargv[i];
			if (DeeSerial_XPutPointer(writer, KW_ADDROF(fk_kargv) + (i * sizeof(DeeObject *)), ob))
				goto err;
		}
#undef KW_ADDROF
		if (DeeSerial_PutAddr(writer, ADDROF(yf_kw), out_kw_addr))
			goto err;
	}
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
}

PRIVATE struct type_seq yf_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yf_iter
	/* TODO: tp_foreach */,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst yf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeYieldFunctionIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst yf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT_AB, offsetof(YFunction, yf_func), "->?Dfunction"),
	TYPE_MEMBER_FIELD_DOC("__this__", STRUCT_OBJECT, offsetof(YFunction, yf_this),
	                      "#tUnboundAttribute{The associated code doesn't take a this-argument}"
	                      "Returns the special $this argument linked to @this yield-function"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_args(YFunction *__restrict self) {
	return DeeRefVector_NewReadonly(self, self->yf_pargc, self->yf_argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
yf_get_code(YFunction *__restrict self) {
	return_reference_(self->yf_func->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_name(YFunction *__restrict self) {
	return function_get_name(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yf_bound_name(YFunction *__restrict self) {
	return function_bound_name(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_doc(YFunction *__restrict self) {
	return function_get_doc(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
yf_get_type(YFunction *__restrict self) {
	return function_get_type(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yf_bound_type(YFunction *__restrict self) {
	return function_bound_type(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_module(YFunction *__restrict self) {
	return function_get_module(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yf_bound_module(YFunction *__restrict self) {
	return function_bound_module(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_operator(YFunction *__restrict self) {
	return function_get_operator(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yf_bound_operator(YFunction *__restrict self) {
	return function_bound_operator(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_operatorname(YFunction *__restrict self) {
	return function_get_operatorname(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_property(YFunction *__restrict self) {
	return function_get_property(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yf_bound_property(YFunction *__restrict self) {
	return function_bound_property(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_refs(YFunction *__restrict self) {
	return function_get_refs(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_statics(YFunction *__restrict self) {
	return DeeFunction_GetStaticsWrapper(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_refsbyname(YFunction *__restrict self) {
	return DeeFunction_GetRefsByNameWrapper(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_staticsbyname(YFunction *__restrict self) {
	return DeeFunction_GetStaticsByNameWrapper(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_kwds(YFunction *__restrict self) {
	return function_get_kwds(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yf_bound_kwds(YFunction *__restrict self) {
	return function_bound_kwds(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_sizeof(YFunction *__restrict self) {
	size_t size = DeeYieldFunction_Sizeof(self->yf_argc);
	return DeeInt_NewSize(size);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_code_defaults(YFunction *__restrict self) {
	return code_getdefaults(self->yf_func->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_code_constants(YFunction *__restrict self) {
	return code_getconstants(self->yf_func->fo_code);
}

PRIVATE struct type_getset tpconst yf_getsets[] = {
	TYPE_GETTER_AB_F("__args__", &yf_get_args,
	                 METHOD_FCONSTCALL,
	                 "->?S?O"),
	TYPE_GETTER_AB_F("__code__", &yf_get_code,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Ert:Code\n"
	                 "Alias for ?A__code__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___name__, &yf_get_name, &yf_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__name__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F(STR___doc__, &yf_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Alias for ?A__doc__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___type__, &yf_get_type, &yf_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__type__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___module__, &yf_get_module, &yf_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__module__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__operator__", &yf_get_operator, &yf_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__operator__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__operatorname__", &yf_get_operatorname, &yf_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?X2?Dstring?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__operatorname__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__property__", &yf_get_property, &yf_bound_property,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__property__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__refs__", &yf_get_refs,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Alias for ?A__refs__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__statics__", &yf_get_statics,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Alias for ?A__statics__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__refsbyname__", &yf_get_refsbyname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__refsbyname__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__staticsbyname__", &yf_get_staticsbyname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__staticsbyname__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__argsbyname__", &DeeYieldFunction_GetArgsByNameWrapper,
	                 METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Combine ?#__kwds__ with ?#__args__ to access the values of arguments"),
	TYPE_GETTER_AB_F("__symbols__", &DeeYieldFunction_GetSymbolsByNameWrapper,
	                 METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "The combination of ?#__refsbyname__, ?#__staticsbyname__ and ?#__argsbyname__, "
	                 /**/ "allowing access to all named symbol that need to maintain their value across "
	                 /**/ "different calls to ?. (requires debug information to be present in the case "
	                 /**/ "of ?#__refsbyname__, ?#__staticsbyname__, and keyword argument support in the "
	                 /**/ "case of ?#__argsbyname__)"),
	TYPE_GETTER_BOUND_F(STR___kwds__, &yf_get_kwds, &yf_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__kwds__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__defaults__", &yf_get_code_defaults,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Alias for ?A__defaults__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__constants__", &yf_get_code_constants,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Alias for ?A__constants__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__sizeof__", &yf_get_sizeof,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint"),
	TYPE_GETSET_END
};


PUBLIC DeeTypeObject DeeYieldFunction_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunction",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &yf_ctor,
			/* tp_copy_ctor:   */ &yf_copy,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &yf_serialize,
			/* tp_free:        */ NULL /* XXX: Use the tuple-allocator? (if somehow still possible with "CONFIG_EXPERIMENTAL_MMAP_DEC") */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&yf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__96C7A3207FAE93E2),
	/* .tp_seq           = */ &yf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yf_getsets,
	/* .tp_members       = */ yf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ yf_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

PRIVATE NONNULL((1)) void DCALL
yfi_run_finally(YFIterator *__restrict self) {
	DeeCodeObject *code;
	code_addr_t ip_addr;
	struct Dee_except_handler *iter, *begin;
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	if unlikely(!self->yi_func)
		return;
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

	/* Recursively execute all finally-handlers that
	 * protect the current PC until none are left. */
	code = self->yi_frame.cf_func->fo_code;
	if unlikely(!code)
		return;
	ASSERT_OBJECT_TYPE(code, &DeeCode_Type);

	/* Simple case: without any finally handlers, we've got nothing to do. */
	if (!(code->co_flags & Dee_CODE_FFINALLY))
		return;
exec_finally:
	begin = code->co_exceptv;
	iter  = begin + code->co_exceptc;

	/* NOTE: The frame-PC is allowed to equal the end of the
	 *       associated code object, because it contains the
	 *       address of the next instruction to-be executed.
	 * Similarly, range checks of handlers are adjusted, too. */
	ASSERT(self->yi_frame.cf_ip >= code->co_code &&
	       self->yi_frame.cf_ip <= code->co_code + code->co_codebytes);
	ip_addr = (code_addr_t)(self->yi_frame.cf_ip - code->co_code);
	while (iter > begin) {
		DREF DeeObject *result, **req_sp;
		--iter;
		if (!(iter->eh_flags & Dee_EXCEPTION_HANDLER_FFINALLY))
			continue;
		if (!(ip_addr > iter->eh_start && ip_addr <= iter->eh_end))
			continue;

		/* Execute this finally-handler. */
		self->yi_frame.cf_ip = code->co_code + iter->eh_addr;

		/* Must adjust the stack to match the requirements of the handler. */
		req_sp = self->yi_frame.cf_stack + iter->eh_stack;
		if (self->yi_frame.cf_sp != req_sp) {
			while (self->yi_frame.cf_sp > req_sp) {
				--self->yi_frame.cf_sp;
				Dee_Decref(*self->yi_frame.cf_sp);
			}
			while (self->yi_frame.cf_sp < req_sp) {
				*self->yi_frame.cf_sp = DeeNone_NewRef();
				++self->yi_frame.cf_sp;
			}
		}

		/* We must somehow indicate to code-exec to stop when an
		 * `ASM_ENDFINALLY' instruction is hit.
		 *
		 * Normally, this is done when the return value has been
		 * assigned, so we simply fake that by pre-assigning `none'. */
		self->yi_frame.cf_result = DeeNone_NewRef();
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
		if unlikely(self->yi_frame.cf_func->fo_code->co_flags & Dee_CODE_FASSEMBLY)
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		if unlikely(self->yi_frame.cf_flags & Dee_CODE_FASSEMBLY)
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		{
			/* Special case: Execute the code using the safe runtime, rather than the fast. */
			result = DeeCode_ExecFrameSafe(&self->yi_frame);
		} else {
			/* Default case: Execute from a fast yield-function-iterator frame. */
			result = DeeCode_ExecFrameFast(&self->yi_frame);
		}
		if likely(result) {
			/* Most likely, this is still the `none' from above */
			Dee_Decref(result);
		} else {
			DeeError_Print("Unhandled exception in YieldFunction.Iterator destructor",
			               ERROR_PRINT_DOHANDLE);
		}
		goto exec_finally;
	}
}

PRIVATE NONNULL((1)) void DCALL
yfi_dtor(YFIterator *__restrict self) {
	size_t numlocals = 0;

	/* Execute established finally handlers. */
	yfi_run_finally(self);
	ASSERT(self->yi_frame.cf_prev == Dee_CODE_FRAME_NOT_EXECUTING);
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	ASSERT_OBJECT_TYPE(self->yi_func, &DeeYieldFunction_Type);
	ASSERT(self->yi_frame.cf_func == self->yi_func->yf_func);
	ASSERT(self->yi_frame.cf_this == self->yi_func->yf_this);
	ASSERT(self->yi_frame.cf_argc == self->yi_func->yf_pargc);
	ASSERT(self->yi_frame.cf_argv == self->yi_func->yf_argv);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	ASSERT_OBJECT_TYPE_OPT(self->yi_func, &DeeYieldFunction_Type);
	ASSERT_OBJECT_TYPE_OPT(self->yi_frame.cf_func, &DeeFunction_Type);
	ASSERT_OBJECT_OPT(self->yi_frame.cf_this);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	ASSERT_OBJECT_TYPE_OPT(self->yi_frame.cf_vargs, &DeeTuple_Type);
	if (self->yi_frame.cf_func)
		numlocals = self->yi_frame.cf_func->fo_code->co_localc;
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	Dee_Decref(self->yi_func);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	Dee_XDecref(self->yi_func);
	Dee_XDecref(self->yi_frame.cf_func);
	Dee_XDecref(self->yi_frame.cf_this);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	Dee_XDecref(self->yi_frame.cf_vargs);

	/* Clear local objects. */
	Dee_XDecrefv(self->yi_frame.cf_frame, numlocals);

	/* Clear objects from the stack. */
	Dee_Decrefv(self->yi_frame.cf_stack,
	            (size_t)(self->yi_frame.cf_sp -
	                     self->yi_frame.cf_stack));
	if (self->yi_frame.cf_stacksz)
		Dee_Free(self->yi_frame.cf_stack);
	Dee_Free(self->yi_frame.cf_frame);
}


PRIVATE NONNULL((1, 2)) void DCALL
yfi_visit(YFIterator *__restrict self,
          Dee_visit_t proc, void *arg) {
	if (self->yi_frame.cf_prev != Dee_CODE_FRAME_NOT_EXECUTING)
		return; /* Can't visit a frame that is current executing. */

	/* NOTE: This won't dead-lock if the caller is inside
	 *       the function because the lock is recursive! */
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	Dee_Visit(self->yi_func);
	DeeYieldFunctionIterator_LockReadNoInt(self);
#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->yi_frame.cf_prev != Dee_CODE_FRAME_NOT_EXECUTING) {
		DeeYieldFunctionIterator_LockEndRead(self);
		return; /* See above... */
	}
#endif /* !CONFIG_NO_THREADS */
	Dee_XVisit(self->yi_frame.cf_vargs);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	DeeYieldFunctionIterator_LockReadNoInt(self);
#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->yi_frame.cf_prev != Dee_CODE_FRAME_NOT_EXECUTING) {
		DeeYieldFunctionIterator_LockEndRead(self);
		return; /* See above... */
	}
#endif /* !CONFIG_NO_THREADS */
	Dee_XVisit(self->yi_func);
	Dee_XVisit(self->yi_frame.cf_this);
	Dee_XVisit(self->yi_frame.cf_vargs);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

	/* Visit local variables. */
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	Dee_XVisitv(self->yi_frame.cf_frame,
	            self->yi_frame.cf_func->fo_code->co_localc);
#else /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	if (self->yi_frame.cf_func) {
		Dee_Visit(self->yi_frame.cf_func);
		Dee_XVisitv(self->yi_frame.cf_frame,
		            self->yi_frame.cf_func->fo_code->co_localc);
	}
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

	/* Visit stack objects. */
	Dee_Visitv(self->yi_frame.cf_stack,
	           (size_t)(self->yi_frame.cf_sp -
	                    self->yi_frame.cf_stack));
	DeeYieldFunctionIterator_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
yfi_clear(YFIterator *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	uint16_t i, nlid;
	DeeCodeObject *code;
	DREF DeeObject **fram;

	/* NOTE: This won't dead-lock if the caller is inside
	 *       the function because the lock is recursive! */
	DeeYieldFunctionIterator_LockWriteNoInt(self);
	ASSERT(self->yi_frame.cf_prev == Dee_CODE_FRAME_NOT_EXECUTING);

	/* Execute established finally handlers. */
	yfi_run_finally(self);

	/* Clear the stack */
	while (self->yi_frame.cf_sp > self->yi_frame.cf_stack) {
		--self->yi_frame.cf_sp;
		Dee_Decref(*self->yi_frame.cf_sp);
	}

	/* Clear local variables */
	code = self->yi_frame.cf_func->fo_code;
	nlid = code->co_localc;
	fram = self->yi_frame.cf_frame;
	for (i = 0; i < nlid; ++i) {
		DREF DeeObject *ob = fram[i];
		fram[i] = NULL;
		Dee_XDecref(ob);
	}

	/* Reset the program counter to the start of code, since that
	 * position is known to *always* accept an empty stack/locals
	 *
	 * Generally, the iterator should not be re-used after it was
	 * cleared, but for the sake of consistency, we have to leave
	 * it in a well-defined state here */
	self->yi_frame.cf_ip = code->co_code;
	ASSERT(self->yi_frame.cf_sp == self->yi_frame.cf_stack);
	DeeYieldFunctionIterator_LockEndWrite(self);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	DeeObject *obj[4], **stack;
	size_t stacksize;
	DeeObject **locals;
	size_t numlocals = 0;
	bool heap_stack  = false;

	/* NOTE: This won't dead-lock if the caller is inside
	 *       the function because the lock is recursive! */
	DeeYieldFunctionIterator_LockWriteNoInt(self);

	/* Execute established finally handlers. */
	yfi_run_finally(self);
	if unlikely(self->yi_frame.cf_prev != Dee_CODE_FRAME_NOT_EXECUTING) {
		/* Can't clear a frame currently being executed. */
		DeeYieldFunctionIterator_LockEndWrite(self);
		return;
	}
	obj[0] = Dee_AsObject(self->yi_func);
	obj[1] = Dee_AsObject(self->yi_frame.cf_func);
	obj[2] = Dee_AsObject(self->yi_frame.cf_this);
	obj[3] = Dee_AsObject(self->yi_frame.cf_vargs);
	if (self->yi_frame.cf_func)
		numlocals = self->yi_frame.cf_func->fo_code->co_localc;
	stack     = self->yi_frame.cf_stack;
	stacksize = self->yi_frame.cf_sp - stack;
	if (self->yi_frame.cf_stacksz) {
		self->yi_frame.cf_stacksz = 0;
		heap_stack                = true;
	}
	locals        = self->yi_frame.cf_frame;
	self->yi_func = NULL;
	/*self->yi_frame.cf_kw  = NULL;*/ /* No necessary */
	self->yi_frame.cf_func  = NULL;
	self->yi_frame.cf_argc  = 0;
	self->yi_frame.cf_argv  = NULL;
	self->yi_frame.cf_this  = NULL;
	self->yi_frame.cf_vargs = NULL;
	self->yi_frame.cf_frame = NULL;
	self->yi_frame.cf_stack = NULL;
	self->yi_frame.cf_sp    = NULL;
	self->yi_frame.cf_ip    = NULL;
	DeeYieldFunctionIterator_LockEndWrite(self);
	Dee_XDecref(obj[0]);
	Dee_XDecref(obj[1]);
	Dee_XDecref(obj[2]);
	Dee_XDecref(obj[3]);

	/* Clear local objects. */
	Dee_XDecrefv(locals, numlocals);

	/* Clear objects from the stack. */
	Dee_Decrefv(stack, stacksize);

	/* Free a heap-allocated stack, and local variable memory. */
	if (heap_stack)
		Dee_Free(stack);
	Dee_Free(locals);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
yfi_iter_next(YFIterator *__restrict self) {
	DREF DeeObject *result;
	if unlikely(DeeYieldFunctionIterator_LockWrite(self))
		goto err;
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	if unlikely(!self->yi_func) {
		/* Special case: Always be indicative of an exhausted iterator
		 * when default-constructed, or after being cleared. */
		result = ITER_DONE;
	} else
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	{
		ASSERT_OBJECT_TYPE(self->yi_func, &DeeYieldFunction_Type);
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
		ASSERT(self->yi_frame.cf_func == self->yi_func->yf_func);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		ASSERT_OBJECT_TYPE(self->yi_frame.cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(self->yi_frame.cf_func->fo_code, &DeeCode_Type);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		ASSERTF(self->yi_frame.cf_func->fo_code->co_flags & Dee_CODE_FYIELDING,
		        "Code is not assembled as a yield-function");
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
		ASSERTF(self->yi_frame.cf_ip >= self->yi_frame.cf_func->fo_code->co_code &&
		        self->yi_frame.cf_ip < (self->yi_frame.cf_func->fo_code->co_code +
		                                self->yi_frame.cf_func->fo_code->co_codebytes),
		        "Illegal PC: %p is not in [%p,%p)",
		        self->yi_frame.cf_ip,
		        self->yi_frame.cf_func->fo_code->co_code,
		        self->yi_frame.cf_func->fo_code->co_code +
		        self->yi_frame.cf_func->fo_code->co_codebytes);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		ASSERTF(self->yi_frame.cf_ip >= self->yi_frame.cf_func->fo_code->co_code &&
		        self->yi_frame.cf_ip <= self->yi_frame.cf_func->fo_code->co_code +
		                                self->yi_frame.cf_func->fo_code->co_codebytes,
		        "Illegal PC: %p is not in %p...%p",
		        self->yi_frame.cf_ip,
		        self->yi_frame.cf_func->fo_code->co_code,
		        self->yi_frame.cf_func->fo_code->co_code +
		        self->yi_frame.cf_func->fo_code->co_codebytes);
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		if unlikely(self->yi_frame.cf_prev != Dee_CODE_FRAME_NOT_EXECUTING) {
			DeeError_Throwf(&DeeError_SegFault, "Stack frame is already being executed");
			result = NULL;
			goto done;
		}
		self->yi_frame.cf_result = NULL;
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
		if unlikely(self->yi_frame.cf_func->fo_code->co_flags & Dee_CODE_FASSEMBLY)
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		if unlikely(self->yi_frame.cf_flags & Dee_CODE_FASSEMBLY)
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
		{
			/* Special case: Execute the code using the safe runtime, rather than the fast. */
			result = DeeCode_ExecFrameSafe(&self->yi_frame);
		} else {
			/* Default case: Execute from a fast yield-function-iterator frame. */
			result = DeeCode_ExecFrameFast(&self->yi_frame);
		}
	}
done:
	DeeYieldFunctionIterator_LockEndWrite(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_ctor(YFIterator *__restrict self) {
	int result;
	DREF YFunction *func;
	func = (DREF YFunction *)DeeObject_NewDefault(&DeeYieldFunction_Type);
	if unlikely(!func)
		goto err;
	result = yfi_init(self, func);
	Dee_Decref(func);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_new(YFIterator *__restrict self,
        size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_YieldFunctionIterator", params: """
	YFunction *func:?Ert:YieldFunction;
""", docStringPrefix: "yfi");]]]*/
#define yfi__YieldFunctionIterator_params "func:?Ert:YieldFunction"
	struct {
		YFunction *func;
	} args;
	DeeArg_Unpack1(err, argc, argv, "_YieldFunctionIterator", &args.func);
/*[[[end]]]*/
	if (DeeObject_AssertType(args.func, &DeeYieldFunction_Type))
		goto err;
	return yfi_init(self, args.func);
err:
	return -1;
}


#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
struct yfi_deepcopy_hook {
	struct Dee_deepcopy_hook        ydch_hook; /* Underlying hook */
	DeeYieldFunctionIteratorObject *ydch_iter; /* [1..1] Target iterator */
};

PRIVATE NONNULL((1, 2)) void DFCALL
yfi_deepcopy_hook_cb(struct Dee_deepcopy_hook *__restrict self,
                     struct Dee_deepcopy_vars *__restrict vars) {
	size_t i;
	struct yfi_deepcopy_hook *me = container_of(self, struct yfi_deepcopy_hook, ydch_hook);
	DeeObject *ob = vars->dcv_obj;
	DeeYieldFunctionIteratorObject *iter = me->ydch_iter;
	DeeFunctionObject *func;

	/* Objects found in the original argv/refv vectors must be copied by-reference! */
	if (ob == iter->yi_frame.cf_this)
		goto copy_by_ref;
	if (ob == Dee_AsObject(iter->yi_frame.cf_vargs))
		goto copy_by_ref;
	for (i = 0; i < iter->yi_frame.cf_argc; ++i) {
		if (ob == iter->yi_frame.cf_argv[i])
			goto copy_by_ref;
	}

	func = iter->yi_frame.cf_func;
	for (i = 0; i < func->fo_code->co_refc; ++i) {
		if (ob == func->fo_refv[i])
			goto copy_by_ref;
	}
	return;
copy_by_ref:
	vars->dcv_mode = Dee_DEEPCOPY_V_MODE_REF;
}


#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
PRIVATE WUNUSED NONNULL((1)) int DCALL
inplace_deepcopy_noarg(DREF DeeObject **__restrict p_ob,
                       size_t argc1, DeeObject *const *argv1,
                       size_t argc2, DeeObject *const *argv2) {
	size_t i;
	DREF DeeObject *ob = *p_ob;
	/* Check if `*p_ob' is apart of the argument
	 * tuple, and don't copy it if it is.
	 * We take special care not to copy objects that were loaded
	 * from arguments/references, as those are intended to be shared.
	 * WARNING: This system isn't, and can never really be perfect.
	 *          For example: should we copy an object loaded from
	 *          the item of a List accessed through a reference/argument?
	 *          The current implementation does, but the user
	 *          may expect it not to do so.
	 *          As far as logic goes, the sane thing would be to
	 *          only copy objects when they are ever written to.
	 *          But the again: how do we know when something will be written?
	 *          Since everything can be dynamically altered, we have no
	 *          way of predicting, or determining when _anything_ is going
	 *          to change in the most dramatic way imaginable.
	 *      ... Anyways. This is the best we can make out of a bad
	 *          situation. - And luckily enough, the old deemon already
	 *          knew that this could lead to troubles and established
	 *          copyable stackframe as a whitelist-based system that
	 *          a user must opt-in if they wish to use it.
	 *          So in that sense: we've always got the user to blame if
	 *                            they manage to break something or get
	 *                            undefined behavior when using [[copyable]]!
	 * NOTE: Don't get me wrong, through. I very much believe that copyable
	 *       stack frames open up the door for _a_ _lot_ of awesome programming
	 *       possibilities, while leaving any undefined behavior as pure-weak,
	 *       in the sense that unless for some bug, the design is able to
	 *       never crash no matter what the user might do.
	 */
	for (i = 0; i < argc1; ++i) {
		if (argv1[i] == ob)
			return 0;
	}
	for (i = 0; i < argc2; ++i) {
		if (argv2[i] == ob)
			return 0;
	}
	/* Create an inplace deep-copy of this object. */
	return DeeObject_InplaceDeepCopy(p_ob);
}
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfi_copy(YFIterator *__restrict self,
         YFIterator *__restrict other) {
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	DeeCodeObject *code;
	size_t i, stacksize;
	DeeDeepCopyContext deep_ctx;
	DREF DeeObject **shallow_objv, **iter;
	size_t shallow_objc;
	struct yfi_deepcopy_hook hook;

	/* Make sure that the function is actually copyable. */
	code = other->yi_frame.cf_func->fo_code;
	if unlikely(!(code->co_flags & Dee_CODE_FCOPYABLE)) {
		char const *function_name = DeeCode_NAME(code);
		if (function_name == NULL)
			function_name = "?";
		return DeeError_Throwf(&DeeError_ValueError, "Function `%s' is not copyable", function_name);
	}

	/* Copy over [const] frame data. */
	self->yi_func          = other->yi_func; /* This gets incref'd at the very end... */
	self->yi_frame.cf_prev = Dee_CODE_FRAME_NOT_EXECUTING;
	self->yi_frame.cf_func = other->yi_frame.cf_func;
	ASSERT(self->yi_frame.cf_func == other->yi_func->yf_func);
	self->yi_frame.cf_argc = other->yi_frame.cf_argc;
	ASSERT(self->yi_frame.cf_argc == other->yi_func->yf_pargc);
	self->yi_frame.cf_argv = other->yi_frame.cf_argv;
	ASSERT(self->yi_frame.cf_argv == other->yi_func->yf_argv);
	self->yi_frame.cf_kw = other->yi_frame.cf_kw;
	ASSERT(self->yi_frame.cf_kw == other->yi_func->yf_kw);
	self->yi_frame.cf_this = other->yi_frame.cf_this;
	ASSERT(self->yi_frame.cf_this == other->yi_func->yf_this);
	self->yi_frame.cf_result = NULL;
	self->yi_frame.cf_frame = (DREF DeeObject **)Dee_Calloc(code->co_framesize);
	if unlikely(!self->yi_frame.cf_frame)
		goto err;
again_lock_other:
	if unlikely(DeeYieldFunctionIterator_LockRead(other))
		goto err_frame;

	/* Mirror heap-allocated stack (if used by "other") */
	self->yi_frame.cf_stacksz = other->yi_frame.cf_stacksz;
	if (self->yi_frame.cf_stacksz) {
		self->yi_frame.cf_stack = (DREF DeeObject **)Dee_TryMallocc(self->yi_frame.cf_stacksz,
		                                                            sizeof(DREF DeeObject *));
		if unlikely(!self->yi_frame.cf_stack) {
			DeeYieldFunctionIterator_LockEndRead(other);
			self->yi_frame.cf_stack = (DREF DeeObject **)Dee_Mallocc(self->yi_frame.cf_stacksz,
			                                                         sizeof(DREF DeeObject *));
			if unlikely(!self->yi_frame.cf_stack)
				goto err_frame;
			if unlikely(DeeYieldFunctionIterator_LockRead(other))
				goto err_frame_stack;
			if unlikely(self->yi_frame.cf_stacksz != other->yi_frame.cf_stacksz) {
				DeeYieldFunctionIterator_LockEndRead(other);
				Dee_Free(self->yi_frame.cf_stack);
				goto again_lock_other;
			}
		}
	} else {
		ASSERT(other->yi_frame.cf_stack ==
		       other->yi_frame.cf_frame + code->co_localc);
		self->yi_frame.cf_stack = self->yi_frame.cf_frame + code->co_localc;
	}

	/* Figure out how much stack space is currently in use */
	stacksize = (size_t)(other->yi_frame.cf_sp - other->yi_frame.cf_stack);

	/* Create by-reference copies of locals and stack-variables.
	 * These must be stored in a secondary storage, since we must
	 * create deep copies of these objects (with special rules)
	 * before they can be written to the output iterator. */
	shallow_objc = code->co_localc + stacksize;
	shallow_objv = (DREF DeeObject **)Dee_TryMallocac(shallow_objc, sizeof(DREF DeeObject *));
	if unlikely(!shallow_objv) {
		DeeYieldFunctionIterator_LockEndRead(other);
		shallow_objv = (DREF DeeObject **)Dee_MallocaHeap(shallow_objc * sizeof(DREF DeeObject *));
		if unlikely(!shallow_objv)
			goto err_frame_stack;
		if unlikely(DeeYieldFunctionIterator_LockRead(other))
			goto err_frame_stack_shallow_objv;
		if unlikely(stacksize != (size_t)(other->yi_frame.cf_sp - other->yi_frame.cf_stack)) {
/*free_shallow_objv_and_stack_and_again_lock_other:*/
			Dee_Freea(shallow_objv);
			if (self->yi_frame.cf_stacksz)
				Dee_Free(self->yi_frame.cf_stack);
			goto again_lock_other;
		}
	}

	/* Copy current (relative) stack and code positions. */
	self->yi_frame.cf_sp = self->yi_frame.cf_stack + stacksize;
	self->yi_frame.cf_ip = other->yi_frame.cf_ip;

	/* Copy references... */
	iter = Dee_XMovprefv(shallow_objv, other->yi_frame.cf_frame, code->co_localc);
	Dee_Movrefv(iter, other->yi_frame.cf_stack, stacksize);

	/* Re-use pre-generated varargs buffer */
	self->yi_frame.cf_vargs = other->yi_frame.cf_vargs;
	Dee_XIncref(self->yi_frame.cf_vargs);

	/* Release lock from "other" */
	DeeYieldFunctionIterator_LockEndRead(other);

	/* Setup deepcopy context */
	DeeDeepCopy_Init(&deep_ctx);
	hook.ydch_hook.dch_hook = &yfi_deepcopy_hook_cb;
	hook.ydch_iter          = self;
	DeeDeepCopyContext_AddHook(&deep_ctx, &hook.ydch_hook);

	/* Initialize locals/stack of the new iterator by creating deep copies */
	for (i = 0; i < code->co_localc; ++i) {
		DREF DeeObject *dst;
		DREF DeeObject *src = shallow_objv[i];
		if (src) {
			dst = DeeDeepCopy_CopyObject(&deep_ctx, src);
			if unlikely(!dst)
				goto err_frame_stack_shallow_objv_refs_deep;
		} else {
			dst = NULL;
		}
		self->yi_frame.cf_frame[i] = dst;
	}
	for (i = 0; i < stacksize; ++i) {
		DREF DeeObject *src = shallow_objv[code->co_localc + i];
		DREF DeeObject *dst = DeeDeepCopy_CopyObject(&deep_ctx, src);
		if unlikely(!dst)
			goto err_frame_stack_shallow_objv_refs_deep;
		self->yi_frame.cf_stack[i] = dst;
	}

	/* Realize deep copies that were made of local/stack variables */
	DeeDeepCopy_Pack(&deep_ctx);

	/* Cleanup shadow object buffer */
	Dee_XDecrefv(shallow_objv, code->co_localc);
	Dee_Decrefv(shallow_objv + code->co_localc, stacksize);
	Dee_Freea(shallow_objv);

	/* Create the (thus-far omitted) reference to the linked yield-function */
	Dee_rshared_rwlock_init(&self->yi_lock);
	Dee_Incref(self->yi_func);
	return 0;
err_frame_stack_shallow_objv_refs_deep:
	DeeDeepCopy_Fini(&deep_ctx);
/*err_frame_stack_shallow_objv_refs:*/
	Dee_XDecrefv(shallow_objv, code->co_localc);
	Dee_Decrefv(shallow_objv + code->co_localc, stacksize);
err_frame_stack_shallow_objv:
	Dee_Freea(shallow_objv);
err_frame_stack:
	if (self->yi_frame.cf_stacksz)
		Dee_Free(self->yi_frame.cf_stack);
err_frame:
	Dee_Free(self->yi_frame.cf_frame);
err:
	return -1;
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	DeeCodeObject *code;
	size_t stack_size;
again:
	if unlikely(DeeYieldFunctionIterator_LockRead(other))
		goto err;

	/* Make sure that the function is actually copyable. */
	code = NULL;
	if (other->yi_frame.cf_func) {
		code = other->yi_frame.cf_func->fo_code;
		if (!(code->co_flags & Dee_CODE_FCOPYABLE)) {
			char const *function_name;
			Dee_Incref(code);
			function_name = DeeCode_NAME(code);
			DeeYieldFunctionIterator_LockEndRead(other);
			if (!function_name)
				function_name = "?";
			DeeError_Throwf(&DeeError_ValueError, "Function `%s' is not copyable", function_name);
			Dee_XDecref(code);
			return -1;
		}
	}
	self->yi_func = other->yi_func;

	/* Copy over frame data. */
	memcpy(&self->yi_frame, &other->yi_frame, sizeof(struct Dee_code_frame));

	/* In case the other frame is currently executing, mark ours as not. */
	self->yi_frame.cf_prev = Dee_CODE_FRAME_NOT_EXECUTING;
	if (code) {
		PTR_isub(DeeObject *, self->yi_frame.cf_sp, (uintptr_t)self->yi_frame.cf_stack);
		if (self->yi_frame.cf_stacksz) {
			/* Copy a heap-allocated, extended stack. */
			self->yi_frame.cf_stack = (DREF DeeObject **)Dee_TryMallocc(self->yi_frame.cf_stacksz,
			                                                            sizeof(DREF DeeObject *));
			if unlikely(!self->yi_frame.cf_stack)
				goto nomem;
			Dee_Movrefv(self->yi_frame.cf_stack,
			            other->yi_frame.cf_stack,
			            self->yi_frame.cf_stacksz);
		}
		self->yi_frame.cf_frame = (DREF DeeObject **)Dee_TryMalloc(code->co_framesize);
		if unlikely(!self->yi_frame.cf_frame)
			goto nomem_stack;

		/* Copy local variables. */
		Dee_Movrefv(self->yi_frame.cf_frame,
		            other->yi_frame.cf_frame,
		            code->co_localc);
		if (!self->yi_frame.cf_stacksz) {
			/* Relocate + copy a frame-shared stack. */
			self->yi_frame.cf_stack = self->yi_frame.cf_frame + code->co_localc;
			PTR_iadd(DeeObject *, self->yi_frame.cf_sp, (uintptr_t)self->yi_frame.cf_stack);
			stack_size = (self->yi_frame.cf_sp - self->yi_frame.cf_stack);
			ASSERTF(stack_size * sizeof(DeeObject *) <=
			        (code->co_framesize - code->co_localc * sizeof(DeeObject *)),
			        "The stack is too large");
			/* Copy the stack. */
			Dee_Movrefv(self->yi_frame.cf_stack,
			            other->yi_frame.cf_stack,
			            stack_size);
		} else {
			PTR_iadd(DeeObject *, self->yi_frame.cf_sp, (uintptr_t)self->yi_frame.cf_stack);
			stack_size = self->yi_frame.cf_stacksz;
		}
	} else {
		self->yi_frame.cf_frame   = NULL;
		self->yi_frame.cf_stack   = NULL;
		self->yi_frame.cf_sp      = NULL;
		self->yi_frame.cf_stacksz = 0;
		stack_size                = 0;
	}

	/* Create references. */
	Dee_XIncref(self->yi_func);
	Dee_XIncref(self->yi_frame.cf_func);
	Dee_XIncref(self->yi_frame.cf_this);
	Dee_XIncref(self->yi_frame.cf_vargs);

	DeeYieldFunctionIterator_LockEndRead(other);
	Dee_rshared_rwlock_init(&self->yi_lock);
	if (code) {
		DeeDeepCopyContext deep_ctx;
		DeeObject *this_arg;
		DeeObject *varargs;
		size_t i, argc, refc;
		DeeObject *const *argv;
		DeeObject **refv;
		this_arg = self->yi_frame.cf_this;
		varargs  = Dee_AsObject(self->yi_frame.cf_vargs);
		argc     = self->yi_frame.cf_argc;
		argv     = self->yi_frame.cf_argv;
		refc     = self->yi_func->yf_func->fo_code->co_refc;
		refv     = self->yi_func->yf_func->fo_refv;

		DeeDeepCopy_Init(&deep_ctx);

		/* TO-DO: Must also (somehow) register "argv" with "deep_ctx" such that it will treat
		 *       those objects as though they were "DeeObject_IsDeepImmutable" (meaning that
		 *       no matter which way they're nested, they won't get copied, such that the
		 *       copy of the iterator will continue to reference the same argument objects
		 *       as were given during the original invocation)  */

		/* With all objects now referenced, we still have to replace
		 * all locals and the stack with deep copies of themself. */
		for (i = 0; i < stack_size; ++i) {
			DeeObject *elem;
			elem = self->yi_frame.cf_stack[i];
			if (elem != this_arg && elem != varargs) {
				/* TO-DO: Must create deep copies of stack/locals using the same deepcopy context! */
				if (inplace_deepcopy_noarg(&self->yi_frame.cf_stack[i],
				                           argc, argv, refc, refv)) {
err_self_deep_ctx:
					DeeDeepCopy_Fini(&deep_ctx);
					goto err_self;
				}
			}
		}
		for (i = 0; i < code->co_localc; ++i) {
			DeeObject *elem;
			elem = self->yi_frame.cf_frame[i];
			if (elem != this_arg && elem != varargs) {
				if (inplace_deepcopy_noarg(&self->yi_frame.cf_frame[i],
				                           argc, argv, refc, refv))
					goto err_self_deep_ctx;
			}
		}

		/* Realize deep copies that were made of local/stack variables */
		DeeDeepCopy_Pack(&deep_ctx);

		/* WARNING: There are some thing that we don't copy, such as the this-argument.
		 *          Similarly, we also don't copy function input arguments! */
	} else {
		ASSERT(!stack_size);
	}
	return 0;
err_self:
	yfi_dtor(self);
err:
	return -1;
nomem_stack:
	if (self->yi_frame.cf_stacksz) {
		DeeObject **vector;
		vector = Dee_Decrefv(self->yi_frame.cf_stack,
		                     self->yi_frame.cf_stacksz);
		Dee_Free(vector);
	}
nomem:
	DeeYieldFunctionIterator_LockEndRead(other);
	if (Dee_CollectMemory(1))
		goto again;
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
}


#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfi_serialize(YFIterator *__restrict self,
              DeeSerial *__restrict writer,
              Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(YFIterator, field))
	YFIterator *out;
	DeeCodeObject *code = self->yi_frame.cf_func->fo_code;
	size_t stacksize;
	Dee_instruction_t *pc;
	Dee_seraddr_t out__cf_frame_addr;
	Dee_seraddr_t out__cf_stack_addr;
	DREF DeeObject **out_objv;

	/* Copy [const] fields... */
	if (DeeSerial_PutObject(writer, ADDROF(yi_func), self->yi_func))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(yi_frame.cf_func), self->yi_frame.cf_func))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(yi_frame.cf_argv), self->yi_frame.cf_argv))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(yi_frame.cf_kw), self->yi_frame.cf_kw))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(yi_frame.cf_this), self->yi_frame.cf_this))
		goto err;
	/* Allocate frame buffer */
	out__cf_frame_addr = DeeSerial_Malloc(writer, code->co_framesize, self->yi_frame.cf_frame);
	if (!Dee_SERADDR_ISOK(out__cf_frame_addr))
		goto err;

	/* Fill in basic fields... */
again_lock_self:
	out = DeeSerial_Addr2Mem(writer, addr, YFIterator);
	Dee_rshared_rwlock_init(&out->yi_lock);
	out->yi_frame.cf_prev   = Dee_CODE_FRAME_NOT_EXECUTING;
	out->yi_frame.cf_argc   = self->yi_frame.cf_argc;
	out->yi_frame.cf_result = NULL;

	/* Lock iterator */
	if (DeeYieldFunctionIterator_LockRead(self))
		goto err;

	/* Copy locals/stack (as a shallow reference copies that can
	 * then be serialized via `DeeSerial_*InplacePutObjectv') */
	stacksize = self->yi_frame.cf_stacksz;
	out->yi_frame.cf_stacksz = (uint16_t)stacksize;
	if (stacksize) {
		size_t sizeof_stack = stacksize * sizeof(DREF DeeObject *);
		out__cf_stack_addr = DeeSerial_TryMalloc(writer, sizeof_stack, self->yi_frame.cf_stack);
		if (!Dee_SERADDR_ISOK(out__cf_stack_addr)) {
			DeeYieldFunctionIterator_LockEndRead(self);
			out__cf_stack_addr = DeeSerial_Malloc(writer, sizeof_stack, self->yi_frame.cf_stack);
			if (!Dee_SERADDR_ISOK(out__cf_stack_addr))
				goto err;
			if unlikely(DeeYieldFunctionIterator_LockRead(self))
				goto err;
			if unlikely(self->yi_frame.cf_stacksz != stacksize) {
				DeeYieldFunctionIterator_LockEndRead(self);
				DeeSerial_Free(writer, out__cf_stack_addr, self->yi_frame.cf_stack);
				goto again_lock_self;
			}
		}
	} else {
		ASSERT(self->yi_frame.cf_stack ==
		       self->yi_frame.cf_frame + code->co_localc);
		out__cf_stack_addr = out__cf_frame_addr + (code->co_localc * sizeof(DREF DeeObject *));
	}

	/* Figure out how much stack space is currently in use */
	stacksize = (size_t)(self->yi_frame.cf_sp - self->yi_frame.cf_stack);

	/* Copy object references */
	out_objv = DeeSerial_Addr2Mem(writer, out__cf_frame_addr, DREF DeeObject *);
	Dee_XMovrefv(out_objv, self->yi_frame.cf_frame, code->co_localc);
	out_objv = DeeSerial_Addr2Mem(writer, out__cf_stack_addr, DREF DeeObject *);
	Dee_Movrefv(out_objv, self->yi_frame.cf_stack, stacksize);

	/* Copy PC + varargs */
	pc  = self->yi_frame.cf_ip;
	out = DeeSerial_Addr2Mem(writer, addr, YFIterator);
	out->yi_frame.cf_vargs = self->yi_frame.cf_vargs;
	Dee_XIncref(out->yi_frame.cf_vargs);
	DeeYieldFunctionIterator_LockEndRead(self);

	/* Inplace-serialize varargs cache */
	if (DeeSerial_XInplacePutObject(writer, ADDROF(yi_frame.cf_vargs)))
		goto err_frame_copy_stack_copy;

	/* Inplace-serialize objects from stack-copy */
	if (DeeSerial_InplacePutObjectv(writer, out__cf_stack_addr, stacksize))
		goto err_frame_copy;

	/* Inplace-serialize objects from frame-copy */
	if (DeeSerial_XInplacePutObjectv(writer, out__cf_frame_addr, code->co_localc))
		goto err;

	/* Store address of program counter. */
	if (DeeSerial_PutPointer(writer, ADDROF(yi_frame.cf_ip), pc))
		goto err;

	/* Store address of frame-copy in ADDROF(yi_frame.cf_frame) */
	if (DeeSerial_PutAddr(writer, ADDROF(yi_frame.cf_frame), out__cf_frame_addr))
		goto err;

	/* Store address of stack-copy in ADDROF(yi_frame.cf_stack) */
	if (DeeSerial_PutAddr(writer, ADDROF(yi_frame.cf_stack), out__cf_stack_addr))
		goto err;

	/* Store relative SP-address in ADDROF(yi_frame.cf_sp) */
	return DeeSerial_PutAddr(writer, ADDROF(yi_frame.cf_sp),
	                         out__cf_stack_addr + (stacksize * sizeof(DREF DeeObject *)));
err_frame_copy_stack_copy:
	Dee_Decrefv(DeeSerial_Addr2Mem(writer, out__cf_stack_addr, DREF DeeObject *), stacksize);
err_frame_copy:
	Dee_XDecrefv(DeeSerial_Addr2Mem(writer, out__cf_frame_addr, DREF DeeObject *), code->co_localc);
err:
	return -1;
#undef ADDROF
}
#endif /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
#define PTR_yfi_serialize &yfi_serialize /* TODO: Inline once "CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS" becomes mandatory */
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
#define PTR_yfi_serialize NULL
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
#ifndef CONFIG_NO_THREADS
PRIVATE WUNUSED NONNULL((1)) DREF YFunction *DCALL
yfi_get_yfunc(YFIterator *__restrict self) {
	DREF YFunction *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	result = self->yi_func;
	Dee_XIncref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!result)
		DeeRT_ErrUnboundAttr(self, &str_seq);
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_yfunc(YFIterator *__restrict self) {
	YFunction *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	result = self->yi_func;
	DeeYieldFunctionIterator_LockEndRead(self);
	return Dee_BOUND_FROMBOOL(result);
err:
	return Dee_BOUND_ERR;
}
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_this(YFIterator *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	DREF DeeObject *thisarg;
	if unlikely(!(self->yi_frame.cf_func->fo_code->co_flags & Dee_CODE_FTHISCALL))
		goto err_unbound;
	thisarg = self->yi_frame.cf_this;
	Dee_Incref(thisarg);
	return thisarg;
err_unbound:
	return DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__this__");
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	DREF DeeObject *thisarg;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	thisarg = self->yi_frame.cf_this;
	if (!(self->yi_frame.cf_flags & Dee_CODE_FTHISCALL))
		thisarg = NULL;
	Dee_XIncref(thisarg);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!thisarg)
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__this__");
	return thisarg;
err:
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_this(YFIterator *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	bool result = (self->yi_frame.cf_func->fo_code->co_flags & Dee_CODE_FTHISCALL) != 0;
	return Dee_BOUND_FROMBOOL(result);
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	bool result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	result = self->yi_frame.cf_this != NULL &&
	         (self->yi_frame.cf_flags & Dee_CODE_FTHISCALL) != 0;
	DeeYieldFunctionIterator_LockEndRead(self);
	return Dee_BOUND_FROMBOOL(result);
err:
	return Dee_BOUND_ERR;
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeFrameObject *DCALL
yfi_get_frame(YFIterator *__restrict self) {
	enum {
		FLAGS = 0 |

		        /* Yield-function-iterator frames are writable, but only while the user is
		         * holding a lock to the respective iterator's lock. Since that lock is also
		         * held while the frame is executing, it becomes impossible for the frame to
		         * be read while executing.
		         * FIXME: This doesn't work under `#define CONFIG_NO_THREADS' */
		        Dee_FRAME_FWRITABLE |
		
		        /* Yield-function-iterator use recursive, shared locks */
		        Dee_FRAME_FSHRLOCK | Dee_FRAME_FRECLOCK |

		        /* The "cf_result" field is undefined here (see `yfi_iter_next()' which
		         * only assigns a proper value *before* yielding the next value). As such,
		         * the frame needs some sort of flag to tell it that the frame's result
		         * field should not be touched and be considered as unbound. */
		        Dee_FRAME_FNORESULT
	};
	return (DREF DeeFrameObject *)DeeFrame_NewReferenceWithLock(Dee_AsObject(self), &self->yi_frame,
	                                                            FLAGS, &self->yi_lock);
}

#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
yfi_get_code(YFIterator *__restrict self) {
	return_reference_(self->yi_frame.cf_func->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_refs(YFIterator *__restrict self) {
	return function_get_refs(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_statics(YFIterator *__restrict self) {
	return DeeFunction_GetStaticsWrapper(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_refsbyname(YFIterator *__restrict self) {
	return DeeFunction_GetRefsByNameWrapper(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_staticsbyname(YFIterator *__restrict self) {
	return DeeFunction_GetStaticsByNameWrapper(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_argsbyname(YFIterator *__restrict self) {
	return DeeYieldFunction_GetArgsByNameWrapper(self->yi_func);
}
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

PRIVATE WUNUSED NONNULL((1)) DREF Function *DCALL
yfi_get_func(YFIterator *__restrict self) {
	DREF Function *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__func__");
		goto err;
	}
	result = self->yi_func->yf_func;
	Dee_Incref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	return result;
err:
	return NULL;
}
#define yfi_bound_func yfi_bound_yfunc
PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
yfi_get_code(YFIterator *__restrict self) {
	DREF DeeCodeObject *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__code__");
		goto err;
	}
	result = self->yi_func->yf_func->fo_code;
	Dee_Incref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	return result;
err:
	return NULL;
}
#define yfi_bound_code yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_refs(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__refs__");
		goto err;
	}
	func = self->yi_func->yf_func;
	Dee_Incref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = function_get_refs(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_refs yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_statics(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__statics__");
		goto err;
	}
	func = self->yi_func->yf_func;
	Dee_Incref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = DeeFunction_GetStaticsWrapper(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_statics yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_refsbyname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__refsbyname__");
		goto err;
	}
	func = self->yi_func->yf_func;
	Dee_Incref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = DeeFunction_GetRefsByNameWrapper(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_refsbyname yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_staticsbyname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__staticsbyname__");
		goto err;
	}
	func = self->yi_func->yf_func;
	Dee_Incref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = DeeFunction_GetStaticsByNameWrapper(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_staticsbyname yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_argsbyname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeYieldFunctionObject *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__argsbyname__");
		goto err;
	}
	func = self->yi_func;
	Dee_Incref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = DeeYieldFunction_GetArgsByNameWrapper(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_argsbyname yfi_bound_yfunc
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_frame_locals(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFrameObject *frame = yfi_get_frame(self);
	if unlikely(!frame)
		goto err;
	result = DeeFrame_GetLocalsWrapper(frame); /* XXX: Inherited */
	Dee_Decref_unlikely(frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_frame_stack(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFrameObject *frame = yfi_get_frame(self);
	if unlikely(!frame)
		goto err;
	result = DeeFrame_GetStackWrapper(frame); /* XXX: Inherited */
	Dee_Decref_unlikely(frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_frame_localsbyname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFrameObject *frame = yfi_get_frame(self);
	if unlikely(!frame)
		goto err;
	result = DeeFrame_GetLocalsByNameWrapper(frame); /* XXX: Inherited */
	Dee_Decref_unlikely(frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_frame_stackbyname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFrameObject *frame = yfi_get_frame(self);
	if unlikely(!frame)
		goto err;
	result = DeeFrame_GetStackByNameWrapper(frame); /* XXX: Inherited */
	Dee_Decref_unlikely(frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_frame_variablesbyname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFrameObject *frame = yfi_get_frame(self);
	if unlikely(!frame)
		goto err;
	result = DeeFrame_GetVariablesByNameWrapper(frame); /* XXX: Inherited */
	Dee_Decref_unlikely(frame);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_frame_symbols(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFrameObject *frame = yfi_get_frame(self);
	if unlikely(!frame)
		goto err;
	result = DeeFrame_GetSymbolsByNameWrapper(frame); /* XXX: Inherited */
	Dee_Decref_unlikely(frame);
	return result;
err:
	return NULL;
}

#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_kwds(YFIterator *__restrict self) {
	return function_get_kwds(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_kwds(YFIterator *__restrict self) {
	return function_bound_kwds(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_args(YFIterator *__restrict self) {
	return yf_get_args(self->yi_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_name(YFIterator *__restrict self) {
	return function_get_name(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_name(YFIterator *__restrict self) {
	return function_bound_name(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_doc(YFIterator *__restrict self) {
	return function_get_doc(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
yfi_get_type(YFIterator *__restrict self) {
	return function_get_type(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_type(YFIterator *__restrict self) {
	return function_bound_type(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_module(YFIterator *__restrict self) {
	return function_get_module(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_module(YFIterator *__restrict self) {
	return function_bound_module(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_operator(YFIterator *__restrict self) {
	return function_get_operator(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_operator(YFIterator *__restrict self) {
	return function_bound_operator(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_operatorname(YFIterator *__restrict self) {
	return function_get_operatorname(self->yi_frame.cf_func);
}
#define yfi_bound_operatorname yfi_bound_operator

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_property(YFIterator *__restrict self) {
	return function_get_property(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_property(YFIterator *__restrict self) {
	return function_bound_property(self->yi_frame.cf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_code_defaults(YFIterator *__restrict self) {
	return code_getdefaults(self->yi_frame.cf_func->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_code_constants(YFIterator *__restrict self) {
	return code_getconstants(self->yi_frame.cf_func->fo_code);
}
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_kwds(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttr(self, &str___kwds__);
		goto err;
	}
	func = self->yi_func->yf_func;
	Dee_Incref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = function_get_kwds(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_kwds(YFIterator *__restrict self) {
	int result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	result = function_bound_kwds(self->yi_func->yf_func);
	DeeYieldFunctionIterator_LockEndRead(self);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_args(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *yfunction;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), "__args__");
		goto err;
	}
	yfunction = self->yi_func;
	Dee_Incref(yfunction);
	DeeYieldFunctionIterator_LockEndRead(self);
	result = yf_get_args(yfunction);
	Dee_Decref(yfunction);
	return result;
err:
	return NULL;
}
#define yfi_bound_args yfi_bound_yfunc


PRIVATE WUNUSED NONNULL((1, 2)) DREF YFunction *DCALL
yfi_get_func_reference(YFIterator *__restrict self,
                       char const *__restrict attr_name) {
	DREF YFunction *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		DeeRT_ErrUnboundAttrCStr(Dee_AsObject(self), attr_name);
		goto err;
	}
	result = self->yi_func;
	Dee_Incref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_name(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, STR___name__);
	if unlikely(!func)
		goto err;
	result = yf_get_name(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_name(YFIterator *__restrict self) {
	int result;
	DREF YFunction *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	func = self->yi_func;
	Dee_XIncref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!func)
		return Dee_BOUND_NO;
	result = yf_bound_name(func);
	Dee_Decref(func);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_doc(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, STR___doc__);
	if unlikely(!func)
		goto err;
	result = yf_get_doc(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_doc yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
yfi_get_type(YFIterator *__restrict self) {
	DREF DeeTypeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, STR___type__);
	if unlikely(!func)
		goto err;
	result = yf_get_type(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_type(YFIterator *__restrict self) {
	int result;
	DREF YFunction *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	func = self->yi_func;
	Dee_XIncref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!func)
		return Dee_BOUND_NO;
	result = yf_bound_type(func);
	Dee_Decref(func);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_module(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, STR___module__);
	if unlikely(!func)
		goto err;
	result = yf_get_module(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_module(YFIterator *__restrict self) {
	bool bound;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	bound = self->yi_func && self->yi_func->yf_func->fo_code->co_module;
	DeeYieldFunctionIterator_LockEndRead(self);
	return Dee_BOUND_FROMBOOL(bound);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_operator(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, "__operator__");
	if unlikely(!func)
		goto err;
	result = yf_get_operator(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_operator(YFIterator *__restrict self) {
	int result;
	DREF YFunction *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	func = self->yi_func;
	Dee_XIncref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!func)
		return Dee_BOUND_NO;
	result = yf_bound_operator(func);
	Dee_Decref(func);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_operatorname(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, "__operatorname__");
	if unlikely(!func)
		goto err;
	result = yf_get_operatorname(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_operatorname yfi_bound_operator

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_property(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, "__property__");
	if unlikely(!func)
		goto err;
	result = yf_get_property(func);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
yfi_bound_property(YFIterator *__restrict self) {
	int result;
	DREF YFunction *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	func = self->yi_func;
	Dee_XIncref(func);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!func)
		return Dee_BOUND_NO;
	result = yf_bound_property(func);
	Dee_Decref(func);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_code_defaults(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, "__defaults__");
	if unlikely(!func)
		goto err;
	result = code_getdefaults(func->yf_func->fo_code);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_code_defaults yfi_bound_yfunc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_get_code_constants(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *func;
	func = yfi_get_func_reference(self, "__constants__");
	if unlikely(!func)
		goto err;
	result = code_getconstants(func->yf_func->fo_code);
	Dee_Decref(func);
	return result;
err:
	return NULL;
}
#define yfi_bound_code_constants yfi_bound_yfunc
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */


PRIVATE struct type_getset tpconst yfi_getsets[] = {
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
#ifndef CONFIG_NO_THREADS
	TYPE_GETTER_BOUND_F(STR_seq, &yfi_get_yfunc, &yfi_bound_yfunc,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    "Alias for ?#__yfunc__"),
#endif /* !CONFIG_NO_THREADS */
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_GETTER_AB_F("__frame__", &yfi_get_frame,
	                 METHOD_FCONSTCALL,
	                 "->?Dframe\n"
	                 "The execution stack-frame representing the current state of the iterator"),
	TYPE_GETTER_BOUND_F("__this__", &yfi_get_this, &yfi_bound_this,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "#tUnboundAttribute{No $this-argument available}"
	                    "The $this-argument used during execution"),
#ifndef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
#ifndef CONFIG_NO_THREADS
	TYPE_GETTER_BOUND_F("__yfunc__", &yfi_get_yfunc, &yfi_bound_yfunc,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Ert:YieldFunction\n"
	                    "The underlying yield-function, describing the ?DFunction "
	                    /**/ "and arguments that are being executed"),
#endif /* !CONFIG_NO_THREADS */
	TYPE_GETTER_BOUND_F("__func__", &yfi_get_func, &yfi_bound_func,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dfunction\n"
	                    "The function that is being executed"),
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	TYPE_GETTER_AB_F("__code__", &yfi_get_code,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Ert:Code\n"
	                 "The code object that is being executed"),
	TYPE_GETTER_AB_F("__refs__", &yfi_get_refs,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Returns a sequence of all of the references used by the function"),
	TYPE_GETTER_AB_F("__statics__", &yfi_get_statics,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Returns a writable sequence of all of the static variables that appear in the function"),
	TYPE_GETTER_AB_F("__refsbyname__", &yfi_get_refsbyname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__refsbyname__?Ert:YieldFunction though ?#__yfunc__"),
	TYPE_GETTER_AB_F("__staticsbyname__", &yfi_get_staticsbyname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__staticsbyname__?Ert:YieldFunction though ?#__yfunc__"),
	TYPE_GETTER_AB_F("__argsbyname__", &yfi_get_argsbyname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__argsbyname__?Ert:YieldFunction though ?#__yfunc__"),
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_GETTER_BOUND_F("__code__", &yfi_get_code, &yfi_bound_code,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Ert:Code\n"
	                    "The code object that is being executed"),
	TYPE_GETTER_BOUND_F("__refs__", &yfi_get_refs, &yfi_bound_refs,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    "Returns a sequence of all of the references used by the function"),
	TYPE_GETTER_BOUND_F("__statics__", &yfi_get_statics, &yfi_bound_statics,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    "Returns a writable sequence of all of the static variables that appear in the function"),
	TYPE_GETTER_BOUND_F("__refsbyname__", &yfi_get_refsbyname, &yfi_bound_refsbyname,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?M?X2?Dstring?Dint?O\n"
	                    "Alias for ?A__refsbyname__?Ert:YieldFunction though ?#__yfunc__"),
	TYPE_GETTER_BOUND_F("__staticsbyname__", &yfi_get_staticsbyname, &yfi_bound_staticsbyname,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?M?X2?Dstring?Dint?O\n"
	                    "Alias for ?A__staticsbyname__?Ert:YieldFunction though ?#__yfunc__"),
	TYPE_GETTER_BOUND_F("__argsbyname__", &yfi_get_argsbyname, &yfi_bound_argsbyname,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?M?X2?Dstring?Dint?O\n"
	                    "Alias for ?A__argsbyname__?Ert:YieldFunction though ?#__yfunc__"),
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

	TYPE_GETTER_AB_F("__locals__", &yfi_get_frame_locals,
	                 METHOD_FCONSTCALL,
	                 "->?S?O\n"
	                 "Alias for ?A__locals__?Ert:Frame through ?#__frame__"),
	TYPE_GETTER_AB_F("__stack__", &yfi_get_frame_stack,
	                 METHOD_FCONSTCALL,
	                 "->?S?O\n"
	                 "Alias for ?A__stack__?Ert:Frame through ?#__frame__"),
	TYPE_GETTER_AB_F("__localsbyname__", &yfi_get_frame_localsbyname,
	                 METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__localsbyname__?Ert:Frame through ?#__frame__"),
	TYPE_GETTER_AB_F("__stackbyname__", &yfi_get_frame_stackbyname,
	                 METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__stackbyname__?Ert:Frame through ?#__frame__"),
	TYPE_GETTER_AB_F("__variablesbyname__", &yfi_get_frame_variablesbyname,
	                 METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__variablesbyname__?Ert:Frame through ?#__frame__"),
	TYPE_GETTER_AB_F("__symbols__", &yfi_get_frame_symbols,
	                 METHOD_FCONSTCALL,
	                 "->?M?X2?Dstring?Dint?O\n"
	                 "Alias for ?A__symbols__?Ert:Frame through ?#__frame__"),

#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	TYPE_GETTER_AB_F("__args__", &yfi_get_args,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "Returns a sequence representing the positional arguments passed to the function"),
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_GETTER_BOUND_F("__args__", &yfi_get_args, &yfi_bound_args,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    "Returns a sequence representing the positional arguments passed to the function"),
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_GETTER_BOUND_F(STR___name__, &yfi_get_name, &yfi_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__name__?DFunction though ?#__func__"),
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	TYPE_GETTER_AB_F(STR___doc__, &yfi_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Alias for ?A__doc__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___kwds__, &yfi_get_kwds, &yfi_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__kwds__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___type__, &yfi_get_type, &yfi_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__type__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___module__, &yfi_get_module, &yfi_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__module__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__operator__", &yfi_get_operator, &yfi_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__operator__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__operatorname__", &yfi_get_operatorname, &yfi_bound_operatorname,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?X2?Dstring?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__operatorname__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__property__", &yfi_get_property, &yfi_bound_property,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__property__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__defaults__", &yfi_get_code_defaults,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "#t{UnboundAttribute}"
	                 "Alias for ?A__defaults__?DFunction though ?#__func__"),
	TYPE_GETTER_AB_F("__constants__", &yfi_get_code_constants,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?S?O\n"
	                 "#t{UnboundAttribute}"
	                 "Alias for ?A__constants__?DFunction though ?#__func__"),
#else /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_GETTER_BOUND_F(STR___doc__, &yfi_get_doc, &yfi_bound_doc,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?X2?Dstring?N\n"
	                    "Alias for ?A__doc__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___kwds__, &yfi_get_kwds, &yfi_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__kwds__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___type__, &yfi_get_type, &yfi_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__type__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F(STR___module__, &yfi_get_module, &yfi_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__module__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__operator__", &yfi_get_operator, &yfi_bound_operator,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__operator__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__operatorname__", &yfi_get_operatorname, &yfi_bound_operatorname,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?X2?Dstring?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__operatorname__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__property__", &yfi_get_property, &yfi_bound_property,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__property__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__defaults__", &yfi_get_code_defaults, &yfi_bound_code_defaults,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__defaults__?DFunction though ?#__func__"),
	TYPE_GETTER_BOUND_F("__constants__", &yfi_get_code_constants, &yfi_bound_code_constants,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    "#t{UnboundAttribute}"
	                    "Alias for ?A__constants__?DFunction though ?#__func__"),
#endif /* !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_GETSET_END
};

#if defined(CONFIG_NO_THREADS) || defined(CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS)
PRIVATE struct type_member tpconst yfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(YFIterator, yi_func),
	                      "->?Ert:YieldFunction\n"
	                      "Alias for ?#__yfunc__"),
	TYPE_MEMBER_FIELD_DOC("__yfunc__", STRUCT_OBJECT_AB, offsetof(YFIterator, yi_func),
	                      "->?Ert:YieldFunction\n"
	                      "The underlying yield-function, describing the ?DFunction "
	                      /**/ "and arguments that are being executed"),
#ifdef CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT_AB, offsetof(YFIterator, yi_frame.cf_func),
	                      "->?Dfunction\n"
	                      "The function that is being executed"),
#endif /* CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
	TYPE_MEMBER_END
};
#else /* CONFIG_NO_THREADS || CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */
#define yfi_members NULL
#endif /* !CONFIG_NO_THREADS && !CONFIG_EXPERIMENTAL_SIMPLIFIED_YIELD_FUNCTION_ITERATORS */

PRIVATE struct type_gc tpconst yfi_gc = {
	/* .tp_gc = */ (void (DCALL *)(DeeObject *__restrict))&yfi_clear
};


PUBLIC DeeTypeObject DeeYieldFunctionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionIterator",
	/* .tp_doc      = */ DOC("(" yfi__YieldFunctionIterator_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ YFIterator,
			/* tp_ctor:        */ &yfi_ctor,
			/* tp_copy_ctor:   */ &yfi_copy,
			/* tp_any_ctor:    */ &yfi_new,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ PTR_yfi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfi_dtor,
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&yfi_visit,
	/* .tp_gc            = */ &yfi_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__27F47A9BEBC0B992),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yfi_getsets,
	/* .tp_members       = */ yfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_C */
