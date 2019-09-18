/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENCLASS_C
#define GUARD_DEEMON_COMPILER_ASM_GENCLASS_C 1

#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/object.h>

DECL_BEGIN

INTERN int(DCALL asm_genclass)(struct ast *__restrict class_ast,
                               unsigned int gflags) {
	size_t i;
	ASSERT(class_ast->a_type == AST_CLASS);
	if (class_ast->a_class.c_base) {
		if (class_ast->a_class.c_base->a_type == AST_SYM) {
			struct symbol *base = class_ast->a_class.c_base->a_sym;
			SYMBOL_INPLACE_UNWIND_ALIAS(base);
			/* TODO: Optimizations for global/extern base types. */
			//if (base->s_type == SYMBOL_TYPE_GLOBAL) {
			//} else if (base->s_type == SYMBOL_TYPE_EXTERN) {
			//}
		}
		if (ast_genasm(class_ast->a_class.c_base, ASM_G_FPUSHRES))
			goto err;
	} else {
		if (asm_putddi(class_ast))
			goto err;
		if (asm_gpush_none())
			goto err;
	}
	if (class_ast->a_class.c_supersym &&
	    !(class_ast->a_flag & AST_FCLASS_NOWRITESUPER) &&
	    (!class_ast->a_class.c_base ||
	     class_ast->a_class.c_base->a_type != AST_SYM ||
	     class_ast->a_class.c_base->a_sym != class_ast->a_class.c_supersym)) {
		/* Save the super-class in its symbol. */
		if (asm_can_prefix_symbol(class_ast->a_class.c_supersym)) {
			/* mov <c_supersym>, top */
			if (asm_gprefix_symbol(class_ast->a_class.c_supersym, class_ast))
				goto err;
			if (asm_gdup_p())
				goto err;
		} else {
			if (asm_gdup())
				goto err;
			if (asm_gpop_symbol(class_ast->a_class.c_supersym, class_ast))
				goto err;
		}
	}
	if (class_ast->a_class.c_desc->a_type == AST_CONSTEXPR &&
	    DeeClassDescriptor_Check(class_ast->a_class.c_desc->a_constexpr)) {
		int32_t cid = asm_newconst(class_ast->a_class.c_desc->a_constexpr);
		if unlikely(cid < 0)
			goto err;
		if (asm_gclass_c((uint16_t)cid))
			goto err;
	} else {
		if (ast_genasm_one(class_ast->a_class.c_desc, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(class_ast))
			goto err;
		if (asm_gclass())
			goto err;
	}
	/* At this point, the new class type has already been created.
	 * Now to store it in its own symbol, before moving on to
	 * initialize all of the members saved within the class
	 * member table. */
	if (class_ast->a_class.c_classsym) {
		if (asm_can_prefix_symbol(class_ast->a_class.c_classsym)) {
			/* mov <c_classsym>, top */
			if (asm_gprefix_symbol(class_ast->a_class.c_classsym, class_ast))
				goto err;
			if (asm_gdup_p())
				goto err;
		} else {
			if (asm_gdup())
				goto err;
			if (asm_gpop_symbol(class_ast->a_class.c_classsym, class_ast))
				goto err;
		}
	}
	/* Now move on to initialize all of the members. */
	for (i = 0; i < class_ast->a_class.c_memberc; ++i) {
		struct class_member *member;
		member = &class_ast->a_class.c_memberv[i];
		if likely(member->cm_index != (uint16_t)-1)
			{
			if (ast_genasm_one(member->cm_ast, ASM_G_FPUSHRES))
				goto err;
			if (asm_putddi(class_ast))
				goto err;
			if (asm_gdefcmember(member->cm_index))
				goto err;
		} else {
			if (ast_genasm_one(member->cm_ast, ASM_G_FNORMAL))
				goto err;
		}
	}
	/* And that's already it! - The new class is complete. */
	if (!(gflags & ASM_G_FPUSHRES) && asm_gpop())
		goto err;
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENCLASS_C */
