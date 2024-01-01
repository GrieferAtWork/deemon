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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/arg.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/string.h>

DECL_BEGIN

#define Q3 "??" "?"

/************************************************************************/
/* RUNTIME API FUNCTIONS CALLED BY GENERATED CODE                       */
/************************************************************************/

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_unbound_global(DeeModuleObject *__restrict mod,
                                 uint16_t global_index) {
	char const *name;
	ASSERT_OBJECT(mod);
	ASSERT(DeeModule_Check(mod));
	ASSERT(global_index < mod->mo_globalc);
	name = DeeModule_GlobalName((DeeObject *)mod, global_index);
	return DeeError_Throwf(&DeeError_UnboundLocal, /* XXX: UnboundGlobal? */
	                       "Unbound global variable `%s' from `%s'",
	                       name ? name : Q3,
	                       DeeString_STR(mod->mo_name));
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
libhostasm_rt_err_unbound_local(struct code_object *code, void *ip, uint16_t local_index) {
	char const *code_name = NULL;
	uint8_t *error;
	struct ddi_state state;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(local_index < code->co_localc);
	error = DeeCode_FindDDI((DeeObject *)code, &state, NULL,
	                        (code_addr_t)((instruction_t *)ip - code->co_code),
	                        DDI_STATE_FNOTHROW);
	if (DDI_ISOK(error)) {
		struct ddi_xregs *iter;
		DDI_STATE_DO(iter, &state) {
			if (local_index < iter->dx_lcnamc) {
				char *local_name;
				if (!code_name)
					code_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_name);
				if ((local_name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_lcnamv[local_index])) != NULL) {
					if (!code_name)
						code_name = DeeCode_NAME(code);
					DeeError_Throwf(&DeeError_UnboundLocal,
					                "Unbound local variable `%s' %s%s",
					                local_name,
					                code_name ? "in function " : "",
					                code_name ? code_name : "");
					Dee_ddi_state_fini(&state);
					return -1;
				}
			}
		}
		DDI_STATE_WHILE(iter, &state);
		Dee_ddi_state_fini(&state);
	}
	if (!code_name)
		code_name = DeeCode_NAME(code);
	return DeeError_Throwf(&DeeError_UnboundLocal,
	                       "Unbound local variable %" PRFu16 "%s%s",
	                       local_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
libhostasm_rt_err_unbound_arg(struct code_object *code, void *ip, uint16_t arg_index) {
	char const *code_name;
	(void)ip;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ASSERT(arg_index < code->co_argc_max);
	code_name = DeeCode_NAME(code);
	if (code->co_keywords) {
		struct string_object *kwname;
		kwname = code->co_keywords[arg_index];
		if (!DeeString_IsEmpty(kwname)) {
			return DeeError_Throwf(&DeeError_UnboundLocal,
			                       "Unbound argument %k%s%s",
			                       kwname,
			                       code_name ? " in function " : "",
			                       code_name ? code_name : "");
		}
	}
	return DeeError_Throwf(&DeeError_UnboundLocal,
	                       "Unbound argument %" PRFu16 "%s%s",
	                       arg_index,
	                       code_name ? " in function " : "",
	                       code_name ? code_name : "");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
libhostasm_rt_err_illegal_instruction(DeeCodeObject *code, void *ip) {
	uint32_t offset;
	char const *code_name = DeeCode_NAME(code);
	if (!code_name)
		code_name = "<anonymous>";
	offset = (uint32_t)((instruction_t *)ip - code->co_code);
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Illegal instruction at %s+%.4" PRFX32,
	                       code_name, offset);
}

INTERN ATTR_COLD int DCALL libhostasm_rt_err_no_active_exception(void) {
	return DeeError_Throwf(&DeeError_RuntimeError, "No active exception");
}

INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
libhostasm_rt_err_unbound_attribute_string(DeeTypeObject *__restrict tp,
                                           char const *__restrict name) {
	ASSERT_OBJECT(tp);
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%r.%s'",
	                       tp, name);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_unbound_class_member(DeeTypeObject *__restrict class_type, uint16_t addr) {
	/* Check if we can find the proper member so we can pass its name. */
	size_t i;
	char const *name = "??" "?";
	struct class_desc *desc;
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	desc = DeeClass_DESC(class_type);
	for (i = 0; i <= desc->cd_desc->cd_cattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_cattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		name = DeeString_STR(attr->ca_name);
		goto got_it;
	}
	for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
			continue;
		name = DeeString_STR(attr->ca_name);
		goto got_it;
	}
	/* Throw the error. */
got_it:
	return libhostasm_rt_err_unbound_attribute_string(class_type, name);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_unbound_instance_member(DeeTypeObject *__restrict class_type, uint16_t addr) {
	/* Check if we can find the proper member so we can pass its name. */
	size_t i;
	char const *name = "??" "?";
	struct class_desc *desc;
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	desc = DeeClass_DESC(class_type);
	for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			continue;
		name = DeeString_STR(attr->ca_name);
		break;
	}

	/* Throw the error. */
	return libhostasm_rt_err_unbound_attribute_string(class_type, name);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_requires_class(DeeTypeObject *__restrict tp_self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Needed a class when %k is only a regular type",
	                       tp_self);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_invalid_class_addr(DeeTypeObject *__restrict tp_self,
                                     uint16_t addr) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid class address %" PRFu16 " for %k",
	                       addr, tp_self);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_invalid_instance_addr(DeeTypeObject *__restrict tp_self,
                                        uint16_t addr) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Invalid class instance address %" PRFu16 " for %k",
	                       addr, tp_self);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
libhostasm_rt_err_nonempty_kw(DeeObject *__restrict kw) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Keyword arguments %r are not accepted here",
	                       kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
libhostasm_rt_assert_empty_kw(DeeObject *__restrict kw) {
	size_t kw_length;
	if (DeeKwds_Check(kw)) {
		kw_length = DeeKwds_SIZE(kw);
	} else {
		kw_length = DeeObject_Size(kw);
		if unlikely(kw_length == (size_t)-1)
			goto err;
	}
	if unlikely(kw_length != 0)
		return libhostasm_rt_err_nonempty_kw(kw);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
libhostasm_rt_DeeObject_ShlRepr(DeeObject *lhs, DeeObject *rhs) {
	DeeTypeMRO mro;
	DREF DeeObject *result;
	DeeTypeObject *tp_temp;
	tp_temp = DeeTypeMRO_Init(&mro, Dee_TYPE(rhs));
	for (;;) {
		DREF DeeObject *(DCALL *tp_shl)(DeeObject *, DeeObject *);
		if (!tp_temp->tp_math ||
		    (tp_shl = tp_temp->tp_math->tp_shl) == NULL) {
			tp_temp = DeeTypeMRO_Next(&mro, tp_temp);
			if (!tp_temp)
				break;
			continue;
		}
		if (tp_shl == DeeFile_Type.ft_base.tp_math->tp_shl) {
			/* Special case: `fp << repr foo'
			 * In this case, we can do a special optimization
			 * to directly print the repr to the file. */
			if (DeeObject_PrintRepr(rhs, (dformatprinter)&DeeFile_WriteAll, lhs) < 0)
				return NULL;
			return_reference_(lhs);
		}
		rhs = DeeObject_Repr(rhs);
		if unlikely(!rhs)
			return NULL;
		result = (*tp_shl)(lhs, rhs);
		Dee_Decref(rhs);
		return result;
	}
	rhs = DeeObject_Repr(rhs);
	if unlikely(!rhs)
		return NULL;
	result = DeeObject_Shl(lhs, rhs);
	Dee_Decref(rhs);
	return result;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C */
