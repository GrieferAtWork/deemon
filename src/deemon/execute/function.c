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
#ifndef GUARD_DEEMON_EXECUTE_FUNCTION_C
#define GUARD_DEEMON_EXECUTE_FUNCTION_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>

#include "../objects/seq/varkwds.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

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
                          DeeFunctionObject *function,
                          struct function_info *__restrict info) {
	struct class_desc *my_class;
	DeeClassDescriptorObject *desc;
	uint16_t addr;
	size_t i;
	my_class = DeeClass_DESC(type);
	desc     = my_class->cd_desc;
	Dee_class_desc_lock_read(my_class);
	for (addr = 0; addr < desc->cd_cmemb_size; ++addr) {
		if (my_class->cd_members[addr] == (DeeObject *)function ||
		    (my_class->cd_members[addr] &&
		     DeeFunction_Check(my_class->cd_members[addr]) &&
		     DeeFunction_CODE(my_class->cd_members[addr]) == code)) {
			Dee_class_desc_lock_endread(my_class);
			for (i = 0; i <= desc->cd_iattr_mask; ++i) {
				struct class_attribute *attr;
				attr = &desc->cd_iattr_list[i];
				if (!attr->ca_name)
					continue;
				if (addr < attr->ca_addr)
					continue;
				if ((attr->ca_flag & (CLASS_ATTRIBUTE_FCLASSMEM | CLASS_ATTRIBUTE_FMETHOD)) !=
				    (CLASS_ATTRIBUTE_FCLASSMEM | CLASS_ATTRIBUTE_FMETHOD))
					continue;
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					if (addr > ((attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) ? attr->ca_addr : attr->ca_addr + 2))
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
				Dee_Incref(type);
				Dee_Incref(attr->ca_name);
				Dee_XIncref(attr->ca_doc);
				return 0;
			}
			for (i = 0; i <= desc->cd_cattr_mask; ++i) {
				struct class_attribute *attr;
				attr = &desc->cd_cattr_list[i];
				if (!attr->ca_name)
					continue;
				if (addr < attr->ca_addr)
					continue;
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					if (addr > ((attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) ? attr->ca_addr : attr->ca_addr + 2))
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
				Dee_Incref(type);
				Dee_Incref(attr->ca_name);
				Dee_XIncref(attr->ca_doc);
				return 0;
			}
			/* Check if we can find the address as an operator binding. */
			for (i = 0; i <= desc->cd_clsop_mask; ++i) {
				struct class_operator *op;
				op = &desc->cd_clsop_list[i];
				if (op->co_name == (uint16_t)-1)
					continue;
				if (op->co_addr != addr)
					continue;
				/* Found it! */
				info->fi_type   = type;
				info->fi_opname = op->co_name;
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
                 /*[in]*/ DeeFunctionObject *function,
                 /*[out]*/ struct function_info *__restrict info) {
	DeeModuleObject *mod;
	uint16_t addr;
	int result;
	info->fi_type   = NULL;
	info->fi_name   = NULL;
	info->fi_doc    = NULL;
	info->fi_opname = (uint16_t)-1;
	info->fi_getset = (uint16_t)-1;

	/* Step #1: Search the code object's module for the given `function' */
	mod = code->co_module;
	if unlikely(!mod)
		goto without_module;
	if unlikely(DeeInteractiveModule_Check(mod))
		goto without_module;
	DeeModule_LockRead(mod);
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		if (!mod->mo_globalv[addr])
			continue;
		if (mod->mo_globalv[addr] == (DeeObject *)function ||
		    (DeeFunction_Check(mod->mo_globalv[addr]) &&
		     DeeFunction_CODE(mod->mo_globalv[addr]) == code)) {
			struct module_symbol *function_symbol;
			DeeModule_LockEndRead(mod);
			function_symbol = DeeModule_GetSymbolID(mod, addr);
			if (function_symbol) {
				/* Found it! (it's a global) */
				info->fi_name = module_symbol_getnameobj(function_symbol);
				if unlikely(!info->fi_name)
					goto err;
				if (function_symbol->ss_doc) {
					info->fi_doc = module_symbol_getdocobj(function_symbol);
					if unlikely(!info->fi_doc)
						goto err_name;
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
		if (result <= 0)
			return result;
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
			if (result <= 0)
				return result;
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
		char *name = DeeCode_NAME(code);
		if (name) {
			/* Well... At least we got the name. - That's something. */
			info->fi_name = (DREF DeeStringObject *)DeeString_New(name);
			if unlikely(!info->fi_name)
				goto err;
			return 0;
		}
	}
	return 1;
err_name:
	Dee_Decref(info->fi_name);
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeCode_GetInfo(/*Code*/ DeeObject *__restrict self,
                /*[out]*/ struct function_info *__restrict info) {
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeCode_Type);
	return lookup_code_info((DeeCodeObject *)self, NULL, info);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeFunction_GetInfo(/*Function*/ DeeObject *__restrict self,
                    /*[out]*/ struct function_info *__restrict info) {
	DeeFunctionObject *me = (DeeFunctionObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeFunction_Type);
	return lookup_code_info(me->fo_code, me, info);
}





PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_New(DeeObject *code_, size_t refc,
                DeeObject *const *refv) {
	DREF Function *result;
	DeeCodeObject *code = (DeeCodeObject *)code_;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERTF(code->co_refc == refc,
	        "code->co_refc = %" PRFu16 "\n"
	        "refc          = %" PRFu16 "\n"
	        "name          = %s\n",
	        code->co_refc, refc,
	        DeeCode_NAME(code));
	result = (DREF Function *)DeeObject_Malloc(offsetof(Function, fo_refv) +
	                                           (refc * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto done;
	result->fo_code = code;
	Dee_Incref(code);
	Dee_Movrefv(result->fo_refv, refv, refc);
	DeeObject_Init(result, &DeeFunction_Type);
done:
	return (DREF DeeObject *)result;
}


INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeFunction_NewInherited(DeeObject *code_, size_t refc,
                         /*inherit(on_success)*/ DREF DeeObject *const *__restrict refv) {
	DeeCodeObject *code = (DeeCodeObject *)code_;
	DREF Function *result;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERTF(code->co_refc == refc,
	        "code->co_refc = %" PRFu16 "\n"
	        "refc          = %" PRFu16 "\n"
	        "name          = %s\n",
	        code->co_refc, refc,
	        DeeCode_NAME(code));
	result = (DREF Function *)DeeObject_Malloc(offsetof(Function, fo_refv) +
	                                           (refc * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto done;
	result->fo_code = code;
	Dee_Incref(code);
	memcpyc(result->fo_refv, refv, refc,
	        sizeof(DREF DeeObject *));
	DeeObject_Init(result, &DeeFunction_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_NewNoRefs(DeeObject *__restrict code_) {
	DeeCodeObject *code = (DeeCodeObject *)code_;
	DREF Function *result;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(code->co_refc == 0);
	result = (DREF Function *)DeeObject_Malloc(offsetof(Function, fo_refv));
	if unlikely(!result)
		goto done;
	result->fo_code = code;
	Dee_Incref(code);
	DeeObject_Init(result, &DeeFunction_Type);
done:
	return (DREF DeeObject *)result;
}


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_Call(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFunction_CallKw(DeeObject *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw);


PRIVATE WUNUSED DREF Function *DCALL
function_init(size_t argc, DeeObject *const *argv) {
	DREF Function *result;
	DeeCodeObject *code = &empty_code;
	DeeObject *refs     = Dee_EmptyTuple;
	if (DeeArg_Unpack(argc, argv, "|oo:Function", &code, &refs))
		goto err;
	if (DeeObject_AssertTypeExact(code, &DeeCode_Type))
		goto err;
	result = (DREF Function *)DeeObject_Malloc(offsetof(Function, fo_refv) +
	                                           (code->co_refc * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto err;
	if (DeeObject_Unpack(refs, code->co_refc, result->fo_refv))
		goto err_r;
	result->fo_code = code;
	Dee_Incref(code);
	DeeObject_Init(result, &DeeFunction_Type);
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
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                self->fo_code->co_refc,
	                                self->fo_refv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_kwds(Function *__restrict self) {
	DeeCodeObject *code = self->fo_code;
	if (!code->co_keywords)
		return_empty_seq;
	return DeeRefVector_NewReadonly((DeeObject *)code,
	                                (size_t)code->co_argc_max,
	                                (DeeObject *const *)code->co_keywords);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_name(Function *__restrict self) {
	struct function_info info;
	if (DeeFunction_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_doc);
	if (!info.fi_name)
		return_none;
	return (DREF DeeObject *)info.fi_name;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_doc(Function *__restrict self) {
	struct function_info info;
	if (DeeFunction_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	if (!info.fi_doc)
		return_none;
	return (DREF DeeObject *)info.fi_doc;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
function_get_type(Function *__restrict self) {
	struct function_info info;
	if (DeeFunction_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (!info.fi_type) {
		info.fi_type = (DREF DeeTypeObject *)Dee_None;
		Dee_Incref(Dee_None);
	}
	return info.fi_type;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_module(Function *__restrict self) {
	if unlikely(!self->fo_code->co_module)
		goto err_unbound; /* Shouldn't happen... */
	return_reference_((DREF DeeObject *)self->fo_code->co_module);
err_unbound:
	err_unbound_attribute_string(&DeeFunction_Type, STR___module__);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_operator(Function *__restrict self) {
	struct function_info info;
	if (DeeFunction_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_opname == (uint16_t)-1)
		return_none;
	return DeeInt_NewUInt16(info.fi_opname);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_operatorname(Function *__restrict self) {
	struct function_info info;
	struct opinfo const *op;
	if (DeeFunction_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_opname == (uint16_t)-1) {
		Dee_XDecref(info.fi_type);
		return_none;
	}
	op = Dee_OperatorInfo(Dee_TYPE(info.fi_type), info.fi_opname);
	Dee_XDecref(info.fi_type);
	if (!op)
		return DeeInt_NewUInt16(info.fi_opname);
	return DeeString_New(op->oi_sname);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
function_get_property(Function *__restrict self) {
	struct function_info info;
	if (DeeFunction_GetInfo((DeeObject *)self, &info) < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	if (info.fi_getset == (uint16_t)-1)
		return_none;
	return DeeInt_NewUInt16(info.fi_getset);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst function_getsets[] = {
	TYPE_GETTER(STR___name__, &function_get_name,
	            "->?X2?Dstring?N\n"
	            "Returns the name of @this function, or ?N if unknown"),
	TYPE_GETTER(STR___doc__, &function_get_doc,
	            "->?X2?Dstring?N\n"
	            "Returns the documentation string of @this function, or ?N if unknown"),
	TYPE_GETTER(STR___type__, &function_get_type,
	            "->?X2?DType?N\n"
	            "Try to determine if @this function is defined as part of a user-defined class, "
	            /**/ "and if it is, return that class type, or ?N if that class couldn't be found, "
	            /**/ "or if @this function is defined as stand-alone"),
	TYPE_GETTER(STR___module__, &function_get_module,
	            "->?DModule\n"
	            "Return the module as part of which @this function's code was originally written"),
	TYPE_GETTER("__operator__", &function_get_operator,
	            "->?X2?Dint?N\n"
	            "Try to determine if @this function is defined as part of a user-defined class, "
	            /**/ "and if so, if it is used to define an operator callback. If that is the case, "
	            /**/ "return the internal ID of the operator that @this function provides, or ?N "
	            /**/ "if that class couldn't be found, @this function is defined as stand-alone, or "
	            /**/ "defined as a class- or instance-method"),
	TYPE_GETTER("__operatorname__", &function_get_operatorname,
	            "->?X3?Dstring?Dint?N\n"
	            "Same as ?#__operator__, but instead try to return the unambiguous name of the "
	            /**/ "operator, though still return its ID if the operator isn't recognized as being "
	            /**/ "part of the standard"),
	TYPE_GETTER("__property__", &function_get_property,
	            "->?X2?Dint?N\n"
	            "Returns an integer describing the kind if @this function is part of a property or getset, "
	            /**/ "or returns ?N if the function's property could not be found, or if the function isn't "
	            /**/ "declared as a property callback\n"
	            "#T{Id|Callback|Compatible prototype~"
	            /**/ "$" PP_STR(CLASS_GETSET_GET) "|Getter callback|${get(): Object}&"
	            /**/ "$" PP_STR(CLASS_GETSET_DEL) "|Delete callback|${del(): none}&"
	            /**/ "$" PP_STR(CLASS_GETSET_SET) "|Setter callback|${set(value: Object): none}"
	            "}"),
	TYPE_GETTER("__refs__", &function_get_refs,
	            "->?S?O\n"
	            "Returns a sequence of all of the references used by @this function"),
	TYPE_GETTER(STR___kwds__, &function_get_kwds,
	            "->?S?Dstring\n"
	            "Returns a sequence of keyword argument names accepted by @this function\n"
	            "If @this function doesn't accept keyword arguments, an empty sequence is returned"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst function_members[] = {
	TYPE_MEMBER_FIELD_DOC("__code__", STRUCT_OBJECT, offsetof(Function, fo_code), "->?Ert:Code"),
	TYPE_MEMBER_END
};


PRIVATE NONNULL((1)) void DCALL
function_fini(Function *__restrict self) {
	size_t i;
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
	 * only get decref'd once the class itself gets killed...
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
	for (i = 0; i < self->fo_code->co_refc; ++i)
		Dee_Decref(self->fo_refv[i]);
	Dee_Decref(self->fo_code);
}

PRIVATE NONNULL((1, 2)) void DCALL
function_visit(Function *__restrict self,
               dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i < self->fo_code->co_refc; ++i)
		Dee_Visit(self->fo_refv[i]);
	Dee_Visit(self->fo_code);
}

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
function_print(Function *__restrict self,
               dformatprinter printer, void *arg) {
	char *name = DeeCode_NAME(self->fo_code);
	dssize_t temp, result;
	uint16_t i;
	if (name != NULL)
		return DeeFormat_Printf(printer, arg, "<Function %s>", name);
	result = DeeFormat_PRINT(printer, arg, "<Function(");
	if unlikely(result < 0)
		goto done;
	DO(err, DeeFormat_PrintObjectRepr(printer, arg, (DeeObject *)self->fo_code));
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

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
function_printrepr(Function *__restrict self,
                   dformatprinter printer, void *arg) {
	char *name = DeeCode_NAME(self->fo_code);
	if (name != NULL)
		return DeeFormat_PrintStr(printer, arg, name);
	/* TODO: Print representation as java-style lambda: "(a, b, c) -> ..."
	 * NOTE: For this purpose, also print default arguments (where present) */
	return function_print(self, printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
function_hash(Function *__restrict self) {
	dhash_t result;
	result = DeeObject_Hash((DeeObject *)self->fo_code);
	result = Dee_HashCombine(result, DeeObject_Hashv(self->fo_refv, self->fo_code->co_refc));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
function_eq(Function *self, Function *other) {
	uint16_t i;
	int result;
	if (DeeObject_AssertTypeExact(other, &DeeFunction_Type))
		goto err;
	result = DeeObject_CompareEq((DeeObject *)self->fo_code,
	                             (DeeObject *)other->fo_code);
	if unlikely(result <= 0)
		goto err_or_false;
	ASSERT(self->fo_code->co_refc == other->fo_code->co_refc);
	for (i = 0; i < self->fo_code->co_refc; ++i) {
		result = DeeObject_CompareEq(self->fo_refv[i],
		                             other->fo_refv[i]);
		if (result <= 0)
			goto err_or_false;
	}
	return_true;
err_or_false:
	if (!result)
		return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
function_ne(Function *self, Function *other) {
	uint16_t i;
	int result;
	if (DeeObject_AssertTypeExact(other, &DeeFunction_Type))
		goto err;
	result = DeeObject_CompareNe((DeeObject *)self->fo_code,
	                             (DeeObject *)other->fo_code);
	if unlikely(result != 0)
		goto err_or_true;
	ASSERT(self->fo_code->co_refc == other->fo_code->co_refc);
	for (i = 0; i < self->fo_code->co_refc; ++i) {
		result = DeeObject_CompareNe(self->fo_refv[i],
		                             other->fo_refv[i]);
		if (result != 0)
			goto err_or_true;
	}
	return_false;
err_or_true:
	if (result > 0)
		return_true;
err:
	return NULL;
}

PRIVATE struct type_cmp function_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&function_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&function_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&function_ne
};


PUBLIC DeeTypeObject DeeFunction_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Function),
	/* .tp_doc      = */ DOC("(code=!Ert:Code_empty,refs=!T0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FNAMEOBJECT | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&function_init,
				/* .tp_free      = */ (dfunptr_t)NULL, /* XXX: Use the tuple-allocator? */
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&function_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&function_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&function_printrepr
	},
	/* .tp_call          = */ &DeeFunction_Call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&function_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &function_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ function_getsets,
	/* .tp_members       = */ function_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ function_class_members,
	/* .tp_call_kw       = */ &DeeFunction_CallKw,
};

PRIVATE NONNULL((1)) void DCALL
yf_fini(YFunction *__restrict self) {
	if (self->yf_kw) {
		size_t i, count;
		count = (self->yf_func->fo_code->co_argc_max - self->yf_argc);
		for (i = 0; i < count; ++i)
			Dee_XDecref(self->yf_kw->fk_kargv[i]);
		if (self->yf_func->fo_code->co_flags & CODE_FVARKWDS) {
			Dee_Decref(self->yf_kw->fk_kw);
			/* NOTE: When `BlackListVarkwds_Type' or `DeeKwdsMapping_Type' are used,
			 *       the argument vector loaded by the object had been implicitly
			 *       referenced via `self->yf_args' until this point. */
			if (self->yf_kw->fk_varkwds)
				VARKWDS_DECREF(self->yf_kw->fk_varkwds);
		}
		Dee_Free(self->yf_kw);
	}
	Dee_Decref(self->yf_func);
	Dee_Decrefv(self->yf_argv, self->yf_argc);
	Dee_XDecref(self->yf_this);
}

PRIVATE NONNULL((1, 2)) void DCALL
yf_visit(YFunction *__restrict self, dvisit_t proc, void *arg) {
	if (self->yf_kw) {
		size_t i, count;
		count = (self->yf_func->fo_code->co_argc_max - self->yf_argc);
		for (i = 0; i < count; ++i)
			Dee_XVisit(self->yf_kw->fk_kargv[i]);
		if (self->yf_func->fo_code->co_flags & CODE_FVARKWDS) {
			Dee_Visit(self->yf_kw->fk_kw);
			Dee_XVisit(self->yf_kw->fk_varkwds);
		}
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
	result->yf_argc = 0;
	result->yf_kw   = NULL;
	result->yf_this = NULL;
	DeeObject_Init(result, &DeeYieldFunction_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF YFunction *DCALL
yf_copy(YFunction *__restrict self) {
	DREF YFunction *result;
	struct code_frame_kwds *kw;
	result = (DREF YFunction *)DeeObject_Malloc(DeeYieldFunction_Sizeof(self->yf_argc));
	if unlikely(!result->yf_func)
		goto err;
	result->yf_kw = NULL;
	if (self->yf_kw) {
		size_t count;
		count = (self->yf_func->fo_code->co_argc_max - self->yf_argc);
		kw = (struct code_frame_kwds *)Dee_Malloc(offsetof(struct code_frame_kwds, fk_kargv) +
		                                          (count * sizeof(DREF DeeObject *)));
		if unlikely(!kw)
			goto err_r;
		result->yf_kw = kw;
		Dee_XMovrefv(kw->fk_kargv, self->yf_kw->fk_kargv, count);
		if (self->yf_func->fo_code->co_flags & CODE_FVARKWDS) {
			result->yf_kw->fk_kw      = self->yf_kw->fk_kw;
			result->yf_kw->fk_varkwds = NULL; /* Don't copy this one... */
			Dee_Incref(result->yf_kw->fk_kw);
		}
	}
	result->yf_func = result->yf_func;
	result->yf_argc = result->yf_argc;
	result->yf_this = self->yf_this;
	Dee_Incref(result->yf_func);
	Dee_Movrefv(result->yf_argv, self->yf_argv, self->yf_argc);
	Dee_XIncref(result->yf_this);
	DeeObject_Init(result, &DeeYieldFunction_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF YFunction *DCALL
yf_deepcopy(YFunction *__restrict self) {
	size_t i, count;
	struct code_frame_kwds *kw;
	DREF YFunction *result;
	result = (DREF YFunction *)DeeObject_Malloc(DeeYieldFunction_Sizeof(self->yf_argc));
	if unlikely(!result->yf_func)
		goto err;
	result->yf_argc = self->yf_argc;
	Dee_Movrefv(result->yf_argv, self->yf_argv, self->yf_argc);
	if (DeeObject_InplaceDeepCopyv(result->yf_argv, self->yf_argc))
		goto err_args_r;
	result->yf_this = NULL;
	if (self->yf_this) {
		result->yf_this = DeeObject_DeepCopy(self->yf_this);
		if unlikely(!result->yf_this)
			goto err_args_r;
	}
	result->yf_kw = NULL;
	if (self->yf_kw) {
		DREF DeeObject *temp;
		count = (self->yf_func->fo_code->co_argc_max - self->yf_argc);
		kw = (struct code_frame_kwds *)Dee_Malloc(offsetof(struct code_frame_kwds, fk_kargv) +
		                                          (count * sizeof(DREF DeeObject *)));
		if unlikely(!kw)
			goto err_this_args_r;
		result->yf_kw = kw;
		if (self->yf_func->fo_code->co_flags & CODE_FVARKWDS) {
			temp = DeeObject_DeepCopy(self->yf_kw->fk_kw);
			if unlikely(!temp)
				goto err_kw_this_args_r;
			result->yf_kw->fk_kw      = temp; /* Inherit reference. */
			result->yf_kw->fk_varkwds = NULL; /* Don't copy this one... */
		}
		for (i = 0; i < count; ++i) {
			temp = self->yf_kw->fk_kargv[i];
			if (temp) {
				temp = DeeObject_DeepCopy(temp);
				if unlikely(!temp)
					goto err_kw_kw_i;
			}
			kw->fk_kargv[i] = temp; /* Inherit reference. */
		}
	}
	result->yf_func = self->yf_func;
	Dee_Incref(result->yf_func);
	DeeObject_Init(result, &DeeYieldFunction_Type);
	return result;
err_kw_kw_i:
	Dee_XDecrefv(kw->fk_kargv, i);
/*err_kw_kw:*/
	if (self->yf_func->fo_code->co_flags & CODE_FVARKWDS)
		Dee_Decref(result->yf_kw->fk_kw);
err_kw_this_args_r:
	Dee_Free(kw);
err_this_args_r:
	Dee_XDecref(result->yf_this);
err_args_r:
	Dee_Decrefv(result->yf_argv, result->yf_argc);
/*err_r:*/
	DeeObject_Free(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfi_init(YFIterator *__restrict self,
         YFunction *__restrict yield_function) {
	DeeCodeObject *code;
	DBG_memset(&self->yi_frame, 0xcc, sizeof(struct code_frame));
	/* Setup the frame for the iterator. */
	self->yi_func          = yield_function;
	self->yi_frame.cf_func = yield_function->yf_func;
	Dee_Incref(yield_function);         /* Reference stored in `self->yi_func' */
	Dee_Incref(self->yi_frame.cf_func); /* Reference stored in here. */
	code = self->yi_frame.cf_func->fo_code;
	/* Allocate memory for frame data. */
	self->yi_frame.cf_prev  = CODE_FRAME_NOT_EXECUTING;
	self->yi_frame.cf_frame = (DREF DeeObject **)Dee_Calloc(code->co_framesize);
	if unlikely(!self->yi_frame.cf_frame)
		goto err_r_base;
	self->yi_frame.cf_stack = self->yi_frame.cf_frame + code->co_localc;
	self->yi_frame.cf_sp    = self->yi_frame.cf_stack;
	self->yi_frame.cf_ip    = code->co_code;
	self->yi_frame.cf_kw    = yield_function->yf_kw;
	self->yi_frame.cf_flags = code->co_flags;
	self->yi_frame.cf_vargs = NULL;
	self->yi_frame.cf_argc  = yield_function->yf_argc;
	self->yi_frame.cf_argv  = yield_function->yf_argv;
	self->yi_frame.cf_this  = yield_function->yf_this;
	Dee_XIncref(self->yi_frame.cf_this);
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
yf_iter_self(YFunction *__restrict self) {
	DREF YFIterator *result;
	result = DeeGCObject_MALLOC(YFIterator);
	if unlikely(!result)
		goto err;
	if unlikely(yfi_init(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeYieldFunctionIterator_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_seq yf_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yf_iter_self
};

PRIVATE struct type_member tpconst yf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeYieldFunctionIterator_Type),
	TYPE_MEMBER_END
};

/* Since YieldFunction objects are bound to a specific function, comparing
 * them won't compare the bound function, but rather that function's pointer! */
PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
yf_hash(DeeYieldFunctionObject *__restrict self) {
	dhash_t result;
	result = DeeObject_HashGeneric(self->yf_func);
	result = Dee_HashCombine(result, DeeObject_Hashv(self->yf_argv, self->yf_argc));
	if (self->yf_this)
		result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->yf_this));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yf_eq_impl(DeeYieldFunctionObject *__restrict self,
           DeeYieldFunctionObject *__restrict other) {
	int error;
	if (DeeObject_AssertTypeExact(other, &DeeYieldFunction_Type))
		goto err;
	if (self == other)
		goto yes;
	if (self->yf_func != other->yf_func)
		goto nope;
	if (self->yf_argc != other->yf_argc)
		goto nope;
	error = DeeSeq_EqVV(self->yf_argv, other->yf_argv, self->yf_argc);
	if unlikely(error <= 0)
		goto do_return_error;
	ASSERTF((self->yf_this != NULL) == (other->yf_this != NULL),
	        "If the functions are identical, they must also have "
	        "identical requirements for the presence of a this-argument!");
	if (self->yf_this)
		return DeeObject_CompareEq(self->yf_this, other->yf_this);
yes:
	return 1;
do_return_error:
	return error;
nope:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yf_eq(DeeYieldFunctionObject *self, DeeYieldFunctionObject *other) {
	int result = yf_eq_impl(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
yf_ne(DeeYieldFunctionObject *self, DeeYieldFunctionObject *other) {
	int result = yf_eq_impl(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE struct type_cmp yf_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&yf_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yf_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&yf_ne
};


PRIVATE struct type_member tpconst yf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(YFunction, yf_func), "->?Dfunction"),
	TYPE_MEMBER_FIELD("__this__", STRUCT_OBJECT, offsetof(YFunction, yf_this)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_args(YFunction *__restrict self) {
	return DeeRefVector_NewReadonly((DeeObject *)self, self->yf_argc, self->yf_argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
yf_get_code(YFunction *__restrict self) {
	return_reference_(self->yf_func->fo_code);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_name(YFunction *__restrict self) {
	return function_get_name(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_doc(YFunction *__restrict self) {
	return function_get_doc(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
yf_get_type(YFunction *__restrict self) {
	return function_get_type(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_module(YFunction *__restrict self) {
	return function_get_module(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_operator(YFunction *__restrict self) {
	return function_get_operator(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_operatorname(YFunction *__restrict self) {
	return function_get_operatorname(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_property(YFunction *__restrict self) {
	return function_get_property(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_refs(YFunction *__restrict self) {
	return function_get_refs(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_kwds(YFunction *__restrict self) {
	return function_get_kwds(self->yf_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yf_get_sizeof(YFunction *__restrict self) {
	size_t size = DeeYieldFunction_Sizeof(self->yf_argc);
	return DeeInt_NewSize(size);
}

PRIVATE struct type_getset tpconst yf_getsets[] = {
	TYPE_GETTER("__args__", &yf_get_args, "->?S?O"),
	TYPE_GETTER("__code__", &yf_get_code,
	            "->?Ert:Code\n"
	            "Alias for :Function.__code__ though ?#__func__"),
	TYPE_GETTER(STR___name__, &yf_get_name,
	            "->?X2?Dstring?N\n"
	            "Alias for :Function.__name__ though ?#__func__"),
	TYPE_GETTER(STR___doc__, &yf_get_doc,
	            "->?X2?Dstring?N\n"
	            "Alias for :Function.__doc__ though ?#__func__"),
	TYPE_GETTER(STR___type__, &yf_get_type,
	            "->?X2?DType?N\n"
	            "Alias for :Function.__type__ though ?#__func__"),
	TYPE_GETTER(STR___module__, &yf_get_module,
	            "->?DModule\n"
	            "Alias for :Function.__module__ though ?#__func__"),
	TYPE_GETTER("__operator__", &yf_get_operator,
	            "->?X2?Dint?N\n"
	            "Alias for :Function.__operator__ though ?#__func__"),
	TYPE_GETTER("__operatorname__", &yf_get_operatorname,
	            "->?X3?Dstring?Dint?N\n"
	            "Alias for :Function.__operatorname__ though ?#__func__"),
	TYPE_GETTER("__property__", &yf_get_property,
	            "->?X2?Dint?N\n"
	            "Alias for :Function.__property__ though ?#__func__"),
	TYPE_GETTER("__refs__", &yf_get_refs,
	            "->?S?O\n"
	            "Alias for :Function.__refs__ though ?#__func__"),
	TYPE_GETTER(STR___kwds__, &yf_get_kwds,
	            "->?S?Dstring\n"
	            "Alias for :Function.__kwds__ though ?#__func__"),
	TYPE_GETTER("__sizeof__", &yf_get_sizeof, "->?Dint"),
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
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&yf_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&yf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&yf_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_free      = */ (dfunptr_t)NULL, /* XXX: Use the tuple-allocator? */
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&yf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &yf_cmp,
	/* .tp_seq           = */ &yf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yf_getsets,
	/* .tp_members       = */ yf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ yf_class_members
};

PRIVATE NONNULL((1)) void DCALL
yfi_run_finally(YFIterator *__restrict self) {
	DeeCodeObject *code;
	code_addr_t ip_addr;
	struct except_handler *iter, *begin;
	if unlikely(!self->yi_func)
		return;

	/* Recursively execute all finally-handlers that
	 * protect the current PC until none are left. */
	code = self->yi_frame.cf_func->fo_code;
	if unlikely(!code)
		return;
	ASSERT_OBJECT_TYPE(code, &DeeCode_Type);

	/* Simple case: without any finally handlers, we've got nothing to do. */
	if (!(code->co_flags & CODE_FFINALLY))
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
		if (!(iter->eh_flags & EXCEPTION_HANDLER_FFINALLY))
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
				*self->yi_frame.cf_sp = Dee_None;
				Dee_Incref(Dee_None);
				++self->yi_frame.cf_sp;
			}
		}

		/* We must somehow indicate to code-exec to stop when an
		 * `ASM_ENDFINALLY' instruction is hit.
		 *
		 * Normally, this is done when the return value has been
		 * assigned, so we simply fake that by pre-assigning `none'. */
		self->yi_frame.cf_result = Dee_None;
		Dee_Incref(Dee_None);
		if unlikely(self->yi_frame.cf_flags & CODE_FASSEMBLY) {
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
			DeeError_Print("Unhandled exception in YieldFunction.Iterator destructor\n",
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
	ASSERT(self->yi_frame.cf_prev == CODE_FRAME_NOT_EXECUTING);
	ASSERT_OBJECT_TYPE_OPT(self->yi_func, &DeeYieldFunction_Type);
	ASSERT_OBJECT_TYPE_OPT(self->yi_frame.cf_func, &DeeFunction_Type);
	ASSERT_OBJECT_TYPE_OPT(self->yi_frame.cf_vargs, &DeeTuple_Type);
	ASSERT_OBJECT_OPT(self->yi_frame.cf_this);
	if (self->yi_frame.cf_func)
		numlocals = self->yi_frame.cf_func->fo_code->co_localc;
	Dee_XDecref(self->yi_func);
	Dee_XDecref(self->yi_frame.cf_func);
	Dee_XDecref(self->yi_frame.cf_this);
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
          dvisit_t proc, void *arg) {
	if (self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING)
		return; /* Can't visit a frame that is current executing. */

	/* NOTE: This won't dead-lock if the caller is inside
	 *       the function because the lock is recursive! */
	DeeYieldFunctionIterator_LockReadNoInt(self);
#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING) {
		DeeYieldFunctionIterator_LockEndRead(self);
		return; /* See above... */
	}
#endif /* !CONFIG_NO_THREADS */
	Dee_XVisit(self->yi_func);
	Dee_XVisit(self->yi_frame.cf_this);
	Dee_XVisit(self->yi_frame.cf_vargs);

	/* Visit local variables. */
	if (self->yi_frame.cf_func) {
		Dee_Visit(self->yi_frame.cf_func);
		Dee_XVisitv(self->yi_frame.cf_frame,
		            self->yi_frame.cf_func->fo_code->co_localc);
	}

	/* Visit stack objects. */
	Dee_Visitv(self->yi_frame.cf_stack,
	           (size_t)(self->yi_frame.cf_sp -
	                    self->yi_frame.cf_stack));
	DeeYieldFunctionIterator_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
yfi_clear(YFIterator *__restrict self) {
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
	if unlikely(self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING) {
		/* Can't clear a frame currently being executed. */
		DeeYieldFunctionIterator_LockEndWrite(self);
		return;
	}
	obj[0] = (DeeObject *)self->yi_func;
	obj[1] = (DeeObject *)self->yi_frame.cf_func;
	obj[2] = (DeeObject *)self->yi_frame.cf_this;
	obj[3] = (DeeObject *)self->yi_frame.cf_vargs;
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
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
yfi_iter_next(YFIterator *__restrict self) {
	DREF DeeObject *result;
	if unlikely(DeeYieldFunctionIterator_LockWrite(self))
		goto err;
	if unlikely(!self->yi_func) {
		/* Special case: Always be indicative of an exhausted iterator
		 * when default-constructed, or after being cleared. */
		result = ITER_DONE;
	} else {
		ASSERT_OBJECT_TYPE(self->yi_func, &DeeYieldFunction_Type);
		ASSERT_OBJECT_TYPE(self->yi_frame.cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(self->yi_frame.cf_func->fo_code, &DeeCode_Type);
		ASSERTF(self->yi_frame.cf_func->fo_code->co_flags & CODE_FYIELDING,
		        "Code is not assembled as a yield-function");
		ASSERTF(self->yi_frame.cf_ip >= self->yi_frame.cf_func->fo_code->co_code &&
		        self->yi_frame.cf_ip <= self->yi_frame.cf_func->fo_code->co_code +
		                                self->yi_frame.cf_func->fo_code->co_codebytes,
		        "Illegal PC: %p is not in %p...%p",
		        self->yi_frame.cf_ip,
		        self->yi_frame.cf_func->fo_code->co_code,
		        self->yi_frame.cf_func->fo_code->co_code +
		        self->yi_frame.cf_func->fo_code->co_codebytes);
		if unlikely(self->yi_frame.cf_prev != CODE_FRAME_NOT_EXECUTING) {
			DeeError_Throwf(&DeeError_SegFault, "Stack frame is already being executed");
			result = NULL;
			goto done;
		}
		self->yi_frame.cf_result = NULL;
		if unlikely(self->yi_frame.cf_flags & CODE_FASSEMBLY) {
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
	YFunction *func;
	if (DeeArg_Unpack(argc, argv, "o:_YieldFunctionIterator", &func))
		goto err;
	if (DeeObject_AssertType(func, &DeeYieldFunction_Type))
		goto err;
	return yfi_init(self, func);
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1)) int DCALL
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
yfi_copy(YFIterator *__restrict self,
         YFIterator *__restrict other) {
	DeeCodeObject *code;
	size_t stack_size;
again:
	if unlikely(DeeYieldFunctionIterator_LockRead(other))
		goto err;

	/* Make sure that the function is actually copyable. */
	code = NULL;
	if (other->yi_frame.cf_func) {
		code = other->yi_frame.cf_func->fo_code;
		if (!(code->co_flags & CODE_FCOPYABLE)) {
			char *function_name;
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
	memcpy(&self->yi_frame, &other->yi_frame, sizeof(struct code_frame));

	/* In case the other frame is currently executing, mark ours as not. */
	self->yi_frame.cf_prev = CODE_FRAME_NOT_EXECUTING;
	if (code) {
		*(uintptr_t *)&self->yi_frame.cf_sp -= (uintptr_t)self->yi_frame.cf_stack;
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
			*(uintptr_t *)&self->yi_frame.cf_sp += (uintptr_t)self->yi_frame.cf_stack;
			stack_size = (self->yi_frame.cf_sp - self->yi_frame.cf_stack);
			ASSERTF(stack_size * sizeof(DeeObject *) <=
			        (code->co_framesize - code->co_localc * sizeof(DeeObject *)),
			        "The stack is too large");
			/* Copy the stack. */
			Dee_Movrefv(self->yi_frame.cf_stack,
			            other->yi_frame.cf_stack,
			            stack_size);
		} else {
			*(uintptr_t *)&self->yi_frame.cf_sp += (uintptr_t)self->yi_frame.cf_stack;
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
		DeeObject *this_arg;
		DeeObject *varargs;
		size_t i, argc, refc;
		DeeObject *const *argv;
		DeeObject **refv;
		this_arg = self->yi_frame.cf_this;
		varargs  = (DeeObject *)self->yi_frame.cf_vargs;
		argc     = self->yi_frame.cf_argc;
		argv     = self->yi_frame.cf_argv;
		refc     = self->yi_func->yf_func->fo_code->co_refc;
		refv     = self->yi_func->yf_func->fo_refv;

		/* With all objects now referenced, we still have to replace
		 * all locals and the stack with deep copies of themself. */
		for (i = 0; i < stack_size; ++i) {
			DeeObject *elem;
			elem = self->yi_frame.cf_stack[i];
			if (elem != this_arg && elem != varargs) {
				if (inplace_deepcopy_noarg(&self->yi_frame.cf_stack[i],
				                           argc, argv, refc, refv))
					goto err_self;
			}
		}
		for (i = 0; i < code->co_localc; ++i) {
			DeeObject *elem;
			elem = self->yi_frame.cf_frame[i];
			if (elem != this_arg && elem != varargs) {
				if (inplace_deepcopy_noarg(&self->yi_frame.cf_frame[i],
				                           argc, argv, refc, refv))
					goto err_self;
			}
		}
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
}

#ifndef CONFIG_NO_THREADS
PRIVATE WUNUSED NONNULL((1)) DREF YFunction *DCALL
yfi_getyfunc(YFIterator *__restrict self) {
	DREF YFunction *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	result = self->yi_func;
	Dee_XIncref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!result)
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, STR_seq);
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getthis(YFIterator *__restrict self) {
	DREF DeeObject *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	result = self->yi_frame.cf_this;
	if (!(self->yi_frame.cf_flags & CODE_FTHISCALL))
		result = NULL;
	Dee_XIncref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	if unlikely(!result)
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, "__this__");
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getframe(YFIterator *__restrict self) {
	return DeeFrame_NewReferenceWithLock((DeeObject *)self,
	                                     &self->yi_frame,
	                                     DEEFRAME_FREADONLY |
	                                     DEEFRAME_FUNDEFSP |
	                                     DEEFRAME_FSHRLOCK |
	                                     DEEFRAME_FRECLOCK,
	                                     &self->yi_lock);
}

PRIVATE WUNUSED NONNULL((1)) DREF Function *DCALL
yfi_getfunc(YFIterator *__restrict self) {
	DREF Function *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, "__func__");
		goto err;
	}
	result = self->yi_func->yf_func;
	Dee_Incref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeCodeObject *DCALL
yfi_getcode(YFIterator *__restrict self) {
	DREF DeeCodeObject *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, "__code__");
		goto err;
	}
	result = self->yi_func->yf_func->fo_code;
	Dee_Incref(result);
	DeeYieldFunctionIterator_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getrefs(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, "__refs__");
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getkwds(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF Function *func;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, STR___kwds__);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getargs(YFIterator *__restrict self) {
	DREF DeeObject *result;
	DREF YFunction *yfunction;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, "__args__");
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF YFunction *DCALL
yfi_get_func_reference(YFIterator *__restrict self,
                       char const *__restrict attr_name) {
	DREF YFunction *result;
	if unlikely(DeeYieldFunctionIterator_LockRead(self))
		goto err;
	if unlikely(!self->yi_func) {
		DeeYieldFunctionIterator_LockEndRead(self);
		err_unbound_attribute_string(&DeeYieldFunctionIterator_Type, attr_name);
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
yfi_getname(YFIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getdoc(YFIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
yfi_gettype(YFIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getmodule(YFIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getoperator(YFIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getoperatorname(YFIterator *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
yfi_getproperty(YFIterator *__restrict self) {
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


PRIVATE struct type_getset tpconst yfi_getsets[] = {
#ifndef CONFIG_NO_THREADS
	TYPE_GETTER(STR_seq, &yfi_getyfunc,
	            "->?S?O\n"
	            "Alias for ?#__yfunc__"),
#endif /* !CONFIG_NO_THREADS */
	TYPE_GETTER("__frame__", &yfi_getframe,
	            "->?Dframe\n"
	            "The execution stack-frame representing the current state of the iterator"),
	TYPE_GETTER("__this__", &yfi_getthis,
	            "#tUnboundAttribute{No $this-argument available}"
	            "The $this-argument used during execution"),
#ifndef CONFIG_NO_THREADS
	TYPE_GETTER("__yfunc__", &yfi_getyfunc,
	            "->?Ert:YieldFunction\n"
	            "The underlying yield-function, describing the ?DFunction "
	            /**/ "and arguments that are being executed"),
#endif /* !CONFIG_NO_THREADS */
	TYPE_GETTER("__func__", &yfi_getfunc,
	            "->?Dfunction\n"
	            "The function that is being executed"),
	TYPE_GETTER("__code__", &yfi_getcode,
	            "->?Ert:Code\n"
	            "The code object that is being executed"),
	TYPE_GETTER("__refs__", &yfi_getrefs,
	            "->?S?O\n"
	            "Returns a sequence of all of the references used by the function"),
	TYPE_GETTER("__args__", &yfi_getargs,
	            "->?S?O\n"
	            "Returns a sequence representing the positional arguments passed to the function"),
	TYPE_GETTER(STR___name__, &yfi_getname,
	            "->?X2?Dstring?N\n"
	            "Alias for ?A__name__?DFunction though ?#__func__"),
	TYPE_GETTER(STR___doc__, &yfi_getdoc,
	            "->?X2?Dstring?N\n"
	            "Alias for ?A__doc__?DFunction though ?#__func__"),
	TYPE_GETTER(STR___kwds__, &yfi_getkwds,
	            "->?S?Dstring\n"
	            "Alias for ?A__kwds__?DFunction though ?#__func__"),
	TYPE_GETTER(STR___type__, &yfi_gettype,
	            "->?X2?DType?N\n"
	            "Alias for ?A__type__?DFunction though ?#__func__"),
	TYPE_GETTER(STR___module__, &yfi_getmodule,
	            "->?DModule\n"
	            "Alias for ?A__module__?DFunction though ?#__func__"),
	TYPE_GETTER("__operator__", &yfi_getoperator,
	            "->?X2?Dint?N\n"
	            "Alias for ?A__operator__?DFunction though ?#__func__"),
	TYPE_GETTER("__operatorname__", &yfi_getoperatorname,
	            "->?X3?Dstring?Dint?N\n"
	            "Alias for ?A__operatorname__?DFunction though ?#__func__"),
	TYPE_GETTER("__property__", &yfi_getproperty,
	            "->?X2?Dint?N\n"
	            "Alias for ?A__property__?DFunction though ?#__func__"),
	TYPE_GETSET_END
};

#ifdef CONFIG_NO_THREADS
PRIVATE struct type_member tpconst yfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(YFIterator, yi_func),
	                      "->?Ert:YieldFunction\n"
	                      "Alias for ?#__yfunc__"),
	TYPE_MEMBER_FIELD_DOC("__yfunc__", STRUCT_OBJECT, offsetof(YFIterator, yi_func),
	                      "->?Ert:YieldFunction\n"
	                      "The underlying yield-function, describing the ?DFunction "
	                      /**/ "and arguments that are being executed"),
	TYPE_MEMBER_END
};
#endif /* CONFIG_NO_THREADS */

PRIVATE struct type_gc tpconst yfi_gc = {
	/* .tp_gc = */ (void (DCALL *)(DeeObject *__restrict))&yfi_clear
};


PUBLIC DeeTypeObject DeeYieldFunctionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_YieldFunctionIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&yfi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&yfi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&yfi_new,
				TYPE_FIXED_ALLOCATOR_GC(YFIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&yfi_dtor,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&yfi_visit,
	/* .tp_gc            = */ &yfi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&yfi_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ yfi_getsets,
#ifdef CONFIG_NO_THREADS
	/* .tp_members       = */ yfi_members,
#else /* CONFIG_NO_THREADS */
	/* .tp_members       = */ NULL,
#endif /* !CONFIG_NO_THREADS */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_FUNCTION_C */
