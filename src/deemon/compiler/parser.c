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
#ifndef GUARD_DEEMON_COMPILER_PARSER_C
#define GUARD_DEEMON_COMPILER_PARSER_C 1

#include <deemon/api.h>

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER
#include <deemon/compiler/parser.h>
#include <deemon/string.h>
#include <deemon/module.h>
#include <deemon/thread.h>

DECL_BEGIN

/* Helper function for opening (sub-)modules, whilst
 * dealing with file-not-found and recursion warnings,
 * as well as context flags.
 * WARNING: The caller still needs to deal with case
 *          where `.` is used as module name!
 *
 * @return: * :        The module in question, or something to substitute it
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: Cannot load module (caller should load module at runtime) */
INTERN WUNUSED NONNULL((1, 3)) DREF struct Dee_module_object *DCALL
DeeLexer_OpenModule(DeeLexer *__restrict self, /*0..1*/ DeeAstLoc const *loc,
                    /*utf-8*/ char const *__restrict module_name) {
	DREF struct Dee_module_object *result;
	char const *filename = loc ? DeeAstLoc_GetName(loc) : NULL;
	if (filename == NULL)
		filename = tpp_file_getrealfilename(DeeLexer_GetFile(self));
	result = DeeModule_OpenEx(module_name, strlen(module_name),
	                          filename, filename ? strlen(filename) : 0,
	                          DeeModule_IMPORT_F_ENOENT |
	                          DeeModule_IMPORT_F_ERECUR,
	                          NULL);
	if (DeeModule_IMPORT_ISOK(result))
		return result;
	if (result == DeeModule_IMPORT_ENOENT) {
		if (DeeLexer_WarnfLoc(self, loc, TPP_W_MODULE_NOT_FOUND, module_name))
			goto err;
	} else if (result == DeeModule_IMPORT_ERECUR) {
		struct Dee_import_frame *current = DeeThread_Self()->t_import_curr;
		char const *current_name = current ? current->if_absfile : "?";
		if (DeeLexer_WarnfLoc(self, loc, TPP_W_RECURSIVE_MODULE_DEPENDENCY, module_name, current_name))
			goto err;
	} else {
		goto err;
	}

	/* Fallback: return deemon module */
	Dee_Incref(&DeeModule_Deemon);
	return &DeeModule_Deemon;
err:
	return NULL;
}

/* @return: * :        Module symbol in question
 * @return: NULL:      An error was thrown
 * @return: ITER_DONE: No such symbol (a warning `TPP_W_MODULE_IMPORT_NOT_FOUND` was already emit) */
INTERN WUNUSED NONNULL((1, 3)) struct Dee_module_symbol *DCALL
DeeLexer_GetModuleSymbol(DeeLexer *__restrict self,
                         /*0..1*/ DeeAstLoc const *loc,
                         struct Dee_module_object *__restrict mod,
                         /*utf-8*/ char const *__restrict symbol_name) {
	struct Dee_module_symbol *result;
	result = DeeModule_GetSymbolString(mod, symbol_name);
	if (result)
		return result;
	if (DeeLexer_WarnfLoc(self, loc, TPP_W_MODULE_IMPORT_NOT_FOUND,
	                      symbol_name, DeeModule_GetShortName(mod)))
		goto err;
	return (struct Dee_module_symbol *)ITER_DONE;
err:
	return NULL;
}

DECL_END
#endif /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

#endif /* !GUARD_DEEMON_COMPILER_PARSER_C */
