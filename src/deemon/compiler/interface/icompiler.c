/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C
#define GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/object.h>
#include <deemon/none.h>
#include <deemon/module.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN


INTERN int DCALL
compiler_init(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject **__restrict argv,
              DeeObject *kw) {
 DeeObject *module = Dee_None;
 struct keyword kwlist[] = { K(module), KEND };
 /* TODO: All those other arguments, like compiler options, etc. */
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"|o",&module))
     goto err;
 if (DeeNone_Check(module)) {
  module = DeeModule_New(Dee_EmptyString);
  if unlikely(!module) goto err;
 } else if (DeeString_Check(module)) {
  module = DeeModule_New(module);
  if unlikely(!module) goto err;
 } else {
  Dee_Incref(module);
 }
 /* Create the new root scope object. */
 self->cp_scope = (DREF DeeScopeObject *)DeeObject_New(&DeeRootScope_Type,1,
                                                      (DeeObject **)&module);
 Dee_Decref(module);
 if unlikely(!self->cp_scope) goto err;
 weakref_support_init(self);
 memset(&self->cp_tags,0,sizeof(self->cp_tags));
 memset(&self->cp_items,0,sizeof(self->cp_items));
 self->cp_flags           = COMPILER_FNORMAL;
 self->cp_prev            = NULL;
 self->cp_recursion       = 0;
 self->cp_options         = NULL;
 self->cp_parser_flags    = PARSE_FNORMAL;
 self->cp_optimizer_flags = OPTIMIZE_FNORMAL;
 self->cp_unwind_limit    = 0;
#ifndef CONFIG_LANGUAGE_NO_ASM
 self->cp_uasm_unique     = 0;
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 if unlikely(!TPPLexer_Init(&self->cp_lexer))
    goto err_scope;
#ifdef _MSC_VER
 /* Mirror MSVC's file-and-line syntax. */
 self->cp_lexer.l_flags |= TPPLEXER_FLAG_MSVC_MESSAGEFORMAT;
#endif
 self->cp_lexer.l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
 parser_errors_init(&self->cp_errors);
 return 0;
err_scope:
 Dee_Decref(self->cp_scope);
err:
 return -1;
}


INTERN DREF DeeObject *DCALL
compiler_get_lexer(DeeCompilerObject *__restrict self) {
 return DeeCompiler_GetLexer(self);
}


INTERN struct type_getset compiler_getsets[] = {
    { "lexer", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&compiler_get_lexer, NULL, NULL,
      DOC("->lexer") },
    { NULL }
};
INTERN struct type_method compiler_methods[] = {
    { NULL }
};
INTERN struct type_member compiler_class_members[] = {
    TYPE_MEMBER_CONST("lexer",&DeeCompilerLexer_Type),
//  TYPE_MEMBER_CONST("ast",&DeeAst_Type),              /* TODO: Rework using the new managed interface */
//  TYPE_MEMBER_CONST("scope",&DeeScope_Type),          /* TODO: Rework using the new managed interface */
//  TYPE_MEMBER_CONST("base_scope",&DeeBaseScope_Type), /* TODO: Rework using the new managed interface */
//  TYPE_MEMBER_CONST("root_scope",&DeeRootScope_Type), /* TODO: Rework using the new managed interface */
    TYPE_MEMBER_END
};


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_ICOMPILER_C */
