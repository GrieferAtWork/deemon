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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/format.h>
#include <deemon/string.h>
#include <deemon/error.h>

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


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C */
