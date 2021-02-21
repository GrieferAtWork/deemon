/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_TRAITS_C
#define GUARD_DEEMON_COMPILER_TRAITS_C 1

#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/traits.h>

DECL_BEGIN

INTERN WUNUSED NONNULL((1)) bool DCALL
ast_chk_multiple_hasexpand(struct ast *__restrict self) {
	size_t i, count;
	ASSERT_AST(self);
	ASSERT(self->a_type == AST_MULTIPLE);
	count = self->a_multiple.m_astc;
	for (i = 0; i < count; ++i) {
		struct ast *elem;
		elem = self->a_multiple.m_astv[i];
		if (elem->a_type == AST_EXPAND)
			return true;
	}
	return false;
}

// INTERN WUNUSED NONNULL((1)) bool DCALL
// ast_chk_maythrow(struct ast *__restrict self,
//                  bool result_used,
//                  DeeTypeObject *exception_type) {
//
// }


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_TRAITS_C */
