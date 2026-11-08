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
#ifndef GUARD_DEEMON_COMPILER_AST_LINKER_C
#define GUARD_DEEMON_COMPILER_AST_LINKER_C 1

#include <deemon/api.h>

#include "../runtime/builtin.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER

/* TODO: Linker name-table must be able to point to:
 * - `struct Dee_link_symbol`       (regular linker symbol -- for "normal" globals/locals/statics)
 * - `struct Dee_class_attribute`   (class attribute -- to properly encode access to class attributes)
 * - `struct Dee_module_symbol`     (imported module symbol -- to link against individual get/del/set callbacks in property-globals)
 *
 * Also: it must have an array of modules whose symbols were *-imported
 *
 * During linking, the linker will then resolve AST references and:
 * - Replace a write to a property-like `struct Dee_module_symbol` symbol
 *   with a call to the relevant setter of that module symbol (meaning that
 *   the assembler will only ever see basic symbol accesses, and there is
 *   no need for `struct Dee_link_symbol` to know of the complexity of
 *   properties or anything like that.
 *
 * TODO: Maybe even go further than that: if we have distinct AST types for
 *       interacting with global/extern/static symbols on an ID-level, since
 *       anything that falls under those categories is externally visible
 *       and thus cannot be transformed later anyways, then `struct Dee_link_symbol`
 *       can be further simplified by ONLY describing local/stack symbols,
 *       and the burden of allocating global/extern/static variables is
 *       lifted off the assembler!
 *
 * TODO: The linker should also be responsible for allocating the `Dee_module_symbol`
 *       table of the module that is currently being compiled, since said table can
 *       never change anymore once AST linking is done. Or more specifically:
 *       - DeeModuleObject::mo_globalc   (because globals are all defined during linking)
 *       - DeeModuleObject::mo_bucketm   (because globals are all defined during linking)
 *       - DeeModuleObject::mo_bucketv   (because globals are all defined during linking)
 */


#endif /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_LINKER_C */
