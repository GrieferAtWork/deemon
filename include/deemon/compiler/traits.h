/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_TRAITS_H
#define GUARD_DEEMON_COMPILER_TRAITS_H 1

#include "../api.h"
#include "../object.h"
#include "ast.h"

#ifdef CONFIG_BUILDING_DEEMON
DECL_BEGIN

#define AST_ENUMERATE_CHILD_BRANCHES(self,callback)


/* Return true if a given `AST_MULTIPLE' contains expand ASTs. */
INTDEF WUNUSED NONNULL((1)) bool DCALL ast_chk_multiple_hasexpand(struct ast *__restrict self);

/* Check if a given AST may cause an instance of `exception_type' to be
 * thrown, or when `exception_type' is `NULL', any kind of exception at all. */
INTDEF bool DCALL ast_chk_maythrow(struct ast *__restrict self,
                                   bool result_used,
                                   DeeTypeObject *exception_type);


DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_TRAITS_H */
