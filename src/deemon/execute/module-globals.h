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
#ifndef GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_H
#define GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_H 1

#include <deemon/api.h>

#include <deemon/module.h> /* DeeModuleObject */
#include <deemon/object.h> /* DREF, DeeTypeObject, Dee_AsObject */

#include "../objects/generic-proxy.h"

#include <stdint.h> /* uint16_t */

DECL_BEGIN

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
INTDEF WUNUSED NONNULL((1)) DREF ModuleExports *DCALL
DeeModule_ViewExports(DeeModuleObject *__restrict self);





typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeModuleObject, mg_module); /* [1..1] The module who's exports are being viewed. */
} ModuleGlobals;

#define ModuleGlobals_New(mod) \
	((DREF ModuleGlobals *)ProxyObject_New(&ModuleGlobals_Type, Dee_AsObject(mod)))

INTDEF DeeTypeObject ModuleGlobals_Type;

INTDEF WUNUSED NONNULL((1)) DREF ModuleGlobals *DCALL
DeeModule_ViewGlobals(DeeModuleObject *__restrict self);




#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeModuleObject, mln_module); /* [1..1] The module who's libnames are being viewed. */
} ModuleLibNames;

#define ModuleLibNames_New(mod) \
	((DREF ModuleLibNames *)ProxyObject_New(&ModuleLibNames_Type, Dee_AsObject(mod)))

INTDEF DeeTypeObject ModuleLibNames_Type;

INTDEF WUNUSED NONNULL((1)) DREF ModuleLibNames *DCALL
DeeModule_LibNames(DeeModuleObject *__restrict self);
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODULE_GLOBALS_H */
