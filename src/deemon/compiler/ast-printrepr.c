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
#ifndef GUARD_DEEMON_COMPILER_AST_PRINTREPR_C
#define GUARD_DEEMON_COMPILER_AST_PRINTREPR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/code.h>              /* Dee_EXCEPTION_HANDLER_FFINALLY */
#include <deemon/compiler/ast.h>      /* ASM_OPERAND_IS_INOUT, ASSERT_AST, ASSERT_AST_OPT, AST_*, CONFIG_NO_AST_DEBUG, PRIVATE_AST_GENERATOR_UNPACK_ARGS, asm_operand, ast, ast_*, catch_expr, class_member */
#include <deemon/compiler/compiler.h> /* DeeCompiler_DelItem, DeeCompiler_LockReading */
#include <deemon/compiler/lexer.h>    /* DeeLexer, DeeLexer_GetLoc, ast_loc, ast_loc_init_empty, ast_loc_isempty, loc_here, tpp_* */
#include <deemon/compiler/parser.h>   /* current_tags */
#include <deemon/compiler/symbol.h>   /* DeeBaseScopeObject, DeeBaseScope_Type, SYMBOL_DEC_N*, SYMBOL_INC_N*, current_scope, lbl_free, symbol, text_label */
#include <deemon/map.h>               /* Dee_EmptyMap */
#include <deemon/none.h>              /* Dee_None */
#include <deemon/format.h>              /* Dee_None */
#include <deemon/object.h>            /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, DREF, DeeObject, DeeTypeObject, Dee_Decref, Dee_DecrefNokill, Dee_Incref, OBJECT_HEAD_INIT */
#include <deemon/seq.h>               /* Dee_EmptySeq */
#include <deemon/set.h>               /* Dee_EmptySet */
#include <deemon/tuple.h>             /* Dee_EmptyTuple */
#include <deemon/type.h>              /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, Dee_operator_t, Dee_visit_t, OPERATOR_ISINPLACE, TF_NONE, TP_FNORMAL */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER
#ifdef DeeAst_HAVE_PRINT_REPR

#define EDO(err_temp, expr)      \
	do {                         \
		if ((temp = (expr)) < 0) \
			goto err_temp;       \
		result += temp;          \
	}	__WHILE0

#define tpp_keyword_print(self, printer, arg) \
	DeeFormat_Print(printer, arg, tpp_keyword_getcstr(self), tpp_keyword_getlen(self))

/* Print the doc-string stored in `self` (if any) */
PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_ast_typedef_print_docstring(struct Dee_ast_typedef const *__restrict self,
                                size_t indent_level,
                                Dee_formatprinter_t printer, void *arg) {
	(void)self;
	(void)indent_level;
	(void)printer;
	(void)arg;
	/* TODO */
	return 0;
}

/* Print `: `, followed by a representation of the typedef-expression `self`
 * When `self` is empty, don't print anything and return `0` */
PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_ast_typedef_print_colon_and_repr(struct Dee_ast_typedef const *__restrict self,
                                     size_t indent_level,
                                     Dee_formatprinter_t printer, void *arg) {
	(void)self;
	(void)indent_level;
	(void)printer;
	(void)arg;
	/* TODO */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_link_symbol_printrepr(struct Dee_link_symbol const *__restrict self,
                          Dee_ast_printrepr_flag flag,
                          Dee_formatprinter_t printer, void *arg) {
	(void)self;
	(void)flag;
	/* TODO */
	return DeeFormat_PRINT(printer, arg, "<link-symbol>");
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_ast_symref_byname_printrepr_without_typedef(struct Dee_ast_symref_byname const *__restrict self,
                                                Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char const *prefix;
	switch (self->asrbn_kind) {
	case Dee_AST_SYMREF_BYNAME_KIND_AUTO:
		return tpp_keyword_print(self->asrbn_name, printer, arg);
	case Dee_AST_SYMREF_BYNAME_KIND_LOCAL:
		prefix = "local ";
		break;
	case Dee_AST_SYMREF_BYNAME_KIND_STACK:
		prefix = "__stack local ";
		break;
	case Dee_AST_SYMREF_BYNAME_KIND_GLOBAL:
		prefix = "global ";
		break;
	case Dee_AST_SYMREF_BYNAME_KIND_STATIC:
		prefix = "static local ";
		break;
	default: __builtin_unreachable();
	}
	result = 0;
	if (self->asrbn_flag & Dee_AST_SYMREF_BYNAME_FLAG_FINAL)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "final "));
	EDO(err_temp, DeeFormat_PrintStr(printer, arg, prefix));
	EDO(err_temp, tpp_keyword_print(self->asrbn_name, printer, arg));
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_ast_symref_byname_printrepr(struct Dee_ast_symref_byname const *__restrict self,
                                size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = Dee_ast_symref_byname_printrepr_without_typedef(self, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, Dee_ast_typedef_print_colon_and_repr(&self->asrbn_typedef, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
Dee_ast_symref_printrepr(struct Dee_ast_symref const *__restrict self,
                         Dee_ast_printrepr_flag flag, size_t indent_level,
                         Dee_formatprinter_t printer, void *arg) {
	switch (self->asr_kind) {
	case Dee_AST_SYMREF_KIND_BYNAME:
		return Dee_ast_symref_byname_printrepr(&self->asr_data.asr_byname, indent_level, printer, arg);
	case Dee_AST_SYMREF_KIND_NTH:
		return DeeFormat_Printf(printer, arg, "%s__nth(%u) %$s%s",
		                        (flag & Dee_AST_PRINTREPR_FLAG_PAREN) ? "(" : "",
		                        (unsigned int)self->asr_data.asr_nth.asrnt_nth,
		                        tpp_keyword_getlen(self->asr_data.asr_nth.asrnt_name),
		                        tpp_keyword_getcstr(self->asr_data.asr_nth.asrnt_name),
		                        (flag & Dee_AST_PRINTREPR_FLAG_PAREN) ? ")" : "");
	case Dee_AST_SYMREF_KIND_EXTERN:
		return DeeFormat_Printf(printer, arg, "%s%s from %q%s",
		                        (flag & Dee_AST_PRINTREPR_FLAG_PAREN) ? "(" : "",
		                        self->asr_data.asr_extern.asrex_sym->ss_name,
		                        DeeModule_GetShortName(self->asr_data.asr_extern.asrex_mod),
		                        (flag & Dee_AST_PRINTREPR_FLAG_PAREN) ? ")" : "");
	case Dee_AST_SYMREF_KIND_THIS:
		return DeeFormat_PRINT(printer, arg, "this");
	case Dee_AST_SYMREF_KIND_TP_THIS:
		return DeeFormat_PRINT(printer, arg, "<tp-this>");
	case Dee_AST_SYMREF_KIND_THISMOD:
		return DeeFormat_PRINT(printer, arg, "import(\".\")");
	case Dee_AST_SYMREF_KIND_VARARGS:
		return DeeFormat_PRINT(printer, arg, "(...)");
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_ast_label_printrepr(struct Dee_ast_label const *__restrict self, size_t indent_level,
                        Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	switch (self->al_kind) {

	case Dee_AST_LABEL_KIND_GOTO: {
		tpp_keyword const *name = self->al_data.ald_goto.alg_name;
		result = tpp_keyword_print(name, printer, arg);
	}	break;

	case Dee_AST_LABEL_KIND_CASE: {
		temp = DeeFormat_PRINT(printer, arg, "case ");
		if unlikely(temp < 0)
			goto err_temp;
		result = DeeAst_PrintRepr(self->al_data.ald_case.alc_case,
		                          Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level,
		                          printer, arg);
		if likely(result >= 0)
			result += temp;
	}	break;

	default: __builtin_unreachable();
	}
	if unlikely(result < 0)
		return result;
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ":\n"));
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_ast_labels_printrepr(struct Dee_ast_labels const *__restrict self, size_t indent_level,
                         Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->al_count; ++i) {
		EDO(err_temp, Dee_ast_label_printrepr(&self->al_labels[i],
		                                      indent_level, printer, arg));
	}
	return result;
err_temp:
	return temp;
}

/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeAstRValue_PrintRepr(DeeAstRValue const *__restrict self,
                       Dee_ast_printrepr_flag flag, size_t indent_level,
                       Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	if (self->arv_labels)
		EDO(err_temp, Dee_ast_labels_printrepr(self->arv_labels, indent_level, printer, arg));
	switch (self->arv_kind) {
	case Dee_AST_RVALUE_KIND_NONE:
		temp = DeeFormat_PRINT(printer, arg, "none");
		break;
	case Dee_AST_RVALUE_KIND_SYMBOL:
		temp = Dee_link_symbol_printrepr(self->arv_data.arvd_symbol,
		                                 Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                                 printer, arg);
		break;
	case Dee_AST_RVALUE_KIND_SYMREF:
		temp = Dee_ast_symref_printrepr(&self->arv_data.arvd_symref,
		                                flag, indent_level, printer, arg);
		break;
	case Dee_AST_RVALUE_KIND_SETATTR: {
		DeeAst const *attr = self->arv_data.arvd_setattr.arvsa_attr;
		EDO(err_temp, DeeAst_PrintRepr(self->arv_data.arvd_setattr.arvsa_obj,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                               Dee_AST_PRINTREPR_FLAG_PAREN,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "."));
		if (attr->a_kind == Dee_AST_CONST && DeeString_Check(attr->a_data.ad_const.ace_value)) {
			temp = DeeString_PrintUtf8(attr->a_data.ad_const.ace_value, printer, arg);
		} else {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "operator . ("));
			EDO(err_temp, DeeAst_PrintRepr(attr, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
			temp = DeeFormat_PRINT(printer, arg, ")");
		}
	}	break;
	case Dee_AST_RVALUE_KIND_SETITEM:
		EDO(err_temp, DeeAst_PrintRepr(self->arv_data.arvd_setitem.arvsi_obj,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                               Dee_AST_PRINTREPR_FLAG_PAREN,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
		EDO(err_temp, DeeAst_PrintRepr(self->arv_data.arvd_setitem.arvsi_item,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
		temp = DeeFormat_PRINT(printer, arg, "]");
		break;
	case Dee_AST_RVALUE_KIND_SETRANGE: {
		DeeAst const *start = self->arv_data.arvd_setrange.arvsr_start;
		DeeAst const *end = self->arv_data.arvd_setrange.arvsr_end;
		EDO(err_temp, DeeAst_PrintRepr(self->arv_data.arvd_setrange.arvsr_obj,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                               Dee_AST_PRINTREPR_FLAG_PAREN,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
		if (start->a_kind != Dee_AST_CONST || !DeeNone_Check(start->a_data.ad_const.ace_value))
			EDO(err_temp, DeeAst_PrintRepr(start, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ":"));
		if (end->a_kind != Dee_AST_CONST || !DeeNone_Check(end->a_data.ad_const.ace_value))
			EDO(err_temp, DeeAst_PrintRepr(end, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
		temp = DeeFormat_PRINT(printer, arg, "]");
	}	break;
	case Dee_AST_RVALUE_KIND_ASSIGN:
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "<assign:=<"));
		EDO(err_temp, DeeAst_PrintRepr(self->arv_data.arvd_assign.arva_obj,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
		temp = DeeFormat_PRINT(printer, arg, ">>");
		break;
	default: __builtin_unreachable();
	}
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	return result;
err_temp:
	return temp;
}



/* Wrap something that can only be printed as a statement, inside of an expression... */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_stmt_as_expr_start(size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "({ // NOSCOPE\n");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
done:
	return result;
err_temp:
	return temp;
}

/* ... and also print the suffix after `DeeAst_PrintRepr_stmt_as_expr_start()` */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_stmt_as_expr_end(size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "\n");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "})"));
done:
	return result;
err_temp:
	return temp;
}



PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_stmt(size_t count, DeeAst const *const *elem,
                                    size_t indent_level,
                                    Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < count; ++i) {
		if (i > 0) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
			EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
		}
		EDO(err_temp, DeeAst_PrintRepr(elem[i], Dee_AST_PRINTREPR_FLAG_ASSTMT,
		                               indent_level, printer, arg));
	}
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_seq(size_t count, DeeAst const *const *elem,
                                   size_t indent_level,
                                   Dee_formatprinter_t printer, void *arg,
                                   char open_ch, char close_ch) {
	size_t i;
	Dee_ssize_t temp, result = DeeFormat_Print(printer, arg, &open_ch, 1);
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < count; ++i) {
		if (i > 0)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, DeeAst_PrintRepr(elem[i], Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
	}
	if (count == 1 && close_ch == ')')
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ","));
	EDO(err_temp, DeeFormat_Print(printer, arg, &close_ch, 1));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_map(size_t count, DeeAst const *const *elem,
                                   size_t indent_level,
                                   Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "{");
	if unlikely(result < 0)
		goto done;
	for (i = 0; (i + 1) < count; i += 2) {
		if (i > 0)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, DeeAst_PrintRepr(elem[i + 0], Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ": "));
		EDO(err_temp, DeeAst_PrintRepr(elem[i + 1], Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
	}
	if (count & 1) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, DeeAst_PrintRepr(elem[count - 1], Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_seq_set(size_t count, DeeAst const *const *elem,
                                       size_t indent_level,
                                       Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "<SET>");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_multi_as_seq(count, elem, indent_level, printer, arg, '{', '}'));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_seq_hashset(size_t count, DeeAst const *const *elem,
                                           size_t indent_level,
                                           Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "import.deemon.HashSet");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_multi_as_seq(count, elem, indent_level, printer, arg, '{', '}'));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_map_dict(size_t count, DeeAst const *const *elem,
                                        size_t indent_level,
                                        Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "import.deemon.Dict");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_multi_as_map(count, elem, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_stmt_brace(size_t count, DeeAst const *const *elem,
                                          size_t indent_level, bool has_scope,
                                          Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	result = has_scope ? DeeFormat_PRINT(printer, arg, "{")
	                   : DeeFormat_PRINT(printer, arg, "{ // NOSCOPE\n");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	EDO(err_temp, DeeAst_PrintRepr_impl_multi_as_stmt(count, elem, indent_level + 1, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_multi_as_stmt_in_expr(size_t count, DeeAst const *const *elem,
                                            size_t indent_level, bool has_scope,
                                            Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_multi_as_stmt_brace(count, elem, indent_level, has_scope, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}


PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_foreach(DeeAst const *elem, DeeAst const *iter,
                              DeeAst const *__restrict body, size_t indent_level,
                              Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "foreach (");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, elem ? DeeAst_PrintRepr(elem, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg)
	                   : DeeFormat_PRINT(printer, arg, "<no-elem>"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ": "));
	EDO(err_temp, iter ? DeeAst_PrintRepr(iter, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg)
	                   : DeeFormat_PRINT(printer, arg, "<no-iter>"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ") "));
	EDO(err_temp, DeeAst_PrintRepr(body, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_foreach_asexpr(DeeAst const *elem, DeeAst const *iter,
                                     DeeAst const *__restrict body, size_t indent_level,
                                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_foreach(elem, iter, body, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_do_while(DeeAst const *__restrict cond, DeeAst const *next,
                               DeeAst const *__restrict body, size_t indent_level,
                               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "do ");
	if unlikely(result < 0)
		goto done;
	if (next) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "{\n"));
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
		EDO(err_temp, DeeAst_PrintRepr(body, Dee_AST_PRINTREPR_FLAG_RAW, indent_level + 1, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
		EDO(err_temp, DeeAst_PrintRepr(next, Dee_AST_PRINTREPR_FLAG_RAW, indent_level + 1, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
	} else {
		EDO(err_temp, DeeAst_PrintRepr(body, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, " while ("));
	EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ");"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_do_while_asexpr(DeeAst const *__restrict cond, DeeAst const *next,
                                      DeeAst const *__restrict body, size_t indent_level,
                                      Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_do_while(cond, next, body, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_for(DeeAst const *cond, DeeAst const *next,
                          DeeAst const *__restrict body, size_t indent_level,
                          Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "for (;");
	if unlikely(result < 0)
		goto done;
	if (cond) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
		EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
	if (next) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
		EDO(err_temp, DeeAst_PrintRepr(next, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ") "));
	EDO(err_temp, DeeAst_PrintRepr(body, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_for_asexpr(DeeAst const *cond, DeeAst const *next,
                                 DeeAst const *__restrict body, size_t indent_level,
                                 Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_for(cond, next, body, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_while(DeeAst const *__restrict cond,
                            DeeAst const *__restrict body, size_t indent_level,
                            Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "while (");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ") "));
	EDO(err_temp, DeeAst_PrintRepr(body, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_while_asexpr(DeeAst const *__restrict cond,
                                   DeeAst const *__restrict body, size_t indent_level,
                                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_while(cond, body, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_qmark_qmark(DeeAst const *cond, DeeAst const *tt,
                                  Dee_ast_printrepr_flag flag, size_t indent_level,
                                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR | Dee_AST_PRINTREPR_FLAG_PAREN,
	                               indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, " ?? "));
	EDO(err_temp, tt ? DeeAst_PrintRepr(tt, Dee_AST_PRINTREPR_FLAG_ASEXPR | Dee_AST_PRINTREPR_FLAG_PAREN,
	                                    indent_level, printer, arg)
	                 : DeeFormat_PRINT(printer, arg, "none"));
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
	return result;
err_temp:
	return temp;
}

/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 7)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_qmark(DeeAst const *cond, DeeAst const *tt, DeeAst const *ff,
                            bool cond_is_none, Dee_ast_printrepr_flag flag,
                            size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR | Dee_AST_PRINTREPR_FLAG_PAREN,
	                               indent_level, printer, arg));
	if (cond_is_none)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " is none"));
	if (cond == tt && !cond_is_none) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " ?: "));
	} else {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " ? "));
		EDO(err_temp, tt ? DeeAst_PrintRepr(tt, Dee_AST_PRINTREPR_FLAG_ASEXPR | Dee_AST_PRINTREPR_FLAG_PAREN,
		                                    indent_level, printer, arg)
		                 : DeeFormat_PRINT(printer, arg, "none"));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " : "));
	}
	if (cond != ff) {
		EDO(err_temp, ff ? DeeAst_PrintRepr(ff, Dee_AST_PRINTREPR_FLAG_ASEXPR | Dee_AST_PRINTREPR_FLAG_PAREN,
		                                    indent_level, printer, arg)
		                 : DeeFormat_PRINT(printer, arg, "none"));
	}
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_if(DeeAst const *cond, DeeAst const *tt, DeeAst const *ff, size_t indent_level,
                         Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "if (");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ") "));
	EDO(err_temp, tt ? DeeAst_PrintRepr(tt, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg)
	                 : DeeFormat_PRINT(printer, arg, ";"));
	if (ff) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " else "));
		EDO(err_temp, DeeAst_PrintRepr(ff, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg));
	}
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_switch_stmt(DeeAst const *cond, DeeAst const *body, size_t indent_level,
                                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "switch (");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(cond, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ") "));
	EDO(err_temp, DeeAst_PrintRepr(body, Dee_AST_PRINTREPR_FLAG_ASSTMT, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_switch_stmt_asexpr(DeeAst const *cond, DeeAst const *body, size_t indent_level,
                                         Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_switch_stmt(cond, body, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_switch_expr(struct Dee_ast_switch_expr const *__restrict self,
                                  size_t indent_level,
                                  Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "switch (");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(self->aswe_expr, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ") {\n"));
	for (i = 0; i < self->aswe_casec; ++i) {
		struct Dee_ast_switch_expr_case const *this_case = self->aswe_casev[i];
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
		if (this_case->asec_condc == 0) {
			if (self->aswe_defcase == i) {
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "default"));
			} else {
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "<empty-case>"));
			}
		} else {
			size_t j;
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "case "));
			for (j = 0; j < this_case->asec_condc; ++j) {
				DeeAst const *this_cond = this_case->asec_condv[j];
				if (j > 0)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
				EDO(err_temp, DeeAst_PrintRepr(this_cond, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
			}
			if (self->aswe_defcase == i)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ", default"));
		}
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " -> "));
		EDO(err_temp, DeeAst_PrintRepr(this_case->asec_body, Dee_AST_PRINTREPR_FLAG_ASBOTH, indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ";\n"));
	}
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_goto(char const *label_name, Dee_formatprinter_t printer, void *arg) {
	if (label_name == NULL)
		label_name = "<anonymous>";
	return DeeFormat_Printf(printer, arg, "goto %s;", label_name);
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_goto_asexpr(char const *label_name, size_t indent_level,
                                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_goto(label_name, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_break(char const *break_mode, char const *label_name,
                            Dee_formatprinter_t printer, void *arg) {
	return label_name ? DeeFormat_Printf(printer, arg, "%s %s;", break_mode, label_name)
	                  : DeeFormat_Printf(printer, arg, "%s;", break_mode);
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_break_asexpr(char const *break_mode, char const *label_name, size_t indent_level,
                                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_break(break_mode, label_name, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 6)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_catch(Dee_ast_printrepr_flag flag, size_t indent_level,
                            DeeAst const *__restrict body, size_t handler_c,
                            struct Dee_ast_catch_handler const *handler_v,
                            Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "try ");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(body, flag, indent_level, printer, arg));
	if (handler_c == 0)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " <no-catch-handlers>"));
	for (i = 0; i < handler_c; ++i) {
		struct Dee_ast_catch_handler const *this_handler = &handler_v[i];
		if (this_handler->ach_flag & Dee_AST_CATCH_HANDLER_FLAG_INTERRUPT)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " @[interrupt]"));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " catch ("));
		EDO(err_temp, this_handler->ach_mask
		              ? DeeAst_PrintRepr(this_handler->ach_mask,
		                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                                 Dee_AST_PRINTREPR_FLAG_PAREN,
		                                 indent_level, printer, arg)
		              : DeeFormat_PRINT(printer, arg, "..."));
		if (this_handler->ach_into) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " as "));
			EDO(err_temp, DeeAst_PrintRepr(this_handler->ach_into, Dee_AST_PRINTREPR_FLAG_ASEXPR,
			                               indent_level, printer, arg));
		}
		EDO(err_temp, DeeAst_PrintRepr(this_handler->ach_body, flag,
		                               indent_level, printer, arg));
	}
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3, 4, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_finally(Dee_ast_printrepr_flag flag, size_t indent_level,
                              DeeAst const *__restrict body, DeeAst const *__restrict finally_,
                              Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "try ");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(body, flag, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, " finally "));
	EDO(err_temp, DeeAst_PrintRepr(finally_, flag, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_yield(DeeAst const *__restrict expr, size_t indent_level,
                            Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "yield ");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(expr, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_yield_asexpr(DeeAst const *__restrict expr, size_t indent_level,
                                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_yield(expr, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_return(DeeAst const *expr, size_t indent_level,
                             Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (!expr)
		return DeeFormat_PRINT(printer, arg, "return;");
	result = DeeFormat_PRINT(printer, arg, "return ");
	if (result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(expr, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_return_asexpr(DeeAst const *expr, size_t indent_level,
                                    Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_return(expr, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}


/* Print function parameter list (including opening "(" and closing ")") */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_function_params(struct Dee_ast_function const *__restrict self,
                                      size_t indent_level,
                                      Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < self->af_argc_max; ++i) {
		struct Dee_ast_function_param const *this_param;
		this_param = &self->af_params[i];
		if (i != 0)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, Dee_ast_typedef_print_docstring(&this_param->afp_base.afpb_typedef, indent_level, printer, arg));
		if (this_param->afp_base.afpb_flag & Dee_AST_SYMREF_BYNAME_FLAG_FINAL)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "final "));
		EDO(err_temp, tpp_keyword_print(this_param->afp_base.afpb_name, printer, arg));
		if (this_param->afp_default == NULL && i > self->af_argc_min)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "?"));
		EDO(err_temp, Dee_ast_typedef_print_colon_and_repr(&this_param->afp_base.afpb_typedef, indent_level, printer, arg));
		if (this_param->afp_default) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
			if (i < self->af_argc_min)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "<ILLEGAL:default>"));
			EDO(err_temp, DeeAst_PrintRepr(this_param->afp_default, Dee_AST_PRINTREPR_FLAG_ASEXPR,
			                               indent_level, printer, arg));
		}
	}
	if (self->af_varargs != NULL) {
		struct Dee_ast_function_param_base const *varargs = self->af_varargs;
		if (self->af_argc_max != 0)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, Dee_ast_typedef_print_docstring(&varargs->afpb_typedef, indent_level, printer, arg));
		EDO(err_temp, tpp_keyword_print(varargs->afpb_name, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "..."));
		EDO(err_temp, Dee_ast_typedef_print_colon_and_repr(&varargs->afpb_typedef, indent_level, printer, arg));
	}
	if (self->af_varkwds != NULL) {
		struct Dee_ast_function_param_base const *varkwds = self->af_varkwds;
		if (self->af_argc_max != 0 || self->af_varargs != NULL)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, Dee_ast_typedef_print_docstring(&varkwds->afpb_typedef, indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "**"));
		EDO(err_temp, tpp_keyword_print(varkwds->afpb_name, printer, arg));
		EDO(err_temp, Dee_ast_typedef_print_colon_and_repr(&varkwds->afpb_typedef, indent_level, printer, arg));
	}

	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

/* Print function, starting **after** the name (iow: at the parameter list) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_function_after_name(struct Dee_ast_function const *__restrict self,
                                          size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_impl_function_params(self, indent_level + 1, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_PRINT(printer, arg, " {\n"));
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
	EDO(err_temp, DeeAst_PrintRepr(self->af_body, Dee_AST_PRINTREPR_FLAG_RAW,
	                               indent_level + 1, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
done:
	return result;
err_temp:
	return temp;
}

/* Print function as a statement/expression */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_function(struct Dee_ast_function const *__restrict self,
                               size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "function");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_function_after_name(self, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}


PRIVATE WUNUSED NONNULL((1)) DeeAst const *DCALL
Dee_ast_class_findmember(struct Dee_ast_class const *__restrict self, uint16_t addr) {
	size_t i;
	for (i = 0; i < self->ac_memberc; ++i) {
		struct Dee_ast_class_member const *this_member;
		this_member = &self->ac_memberv[i];
		if (this_member->cm_addr == addr)
			return this_member->cm_init;
	}
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_class_attribute_uses_class_addr(struct Dee_class_attribute const *__restrict self,
                                    bool is_cattr, uint16_t addr) {
	if (self->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM)
		is_cattr = true;
	if (!is_cattr)
		return false;
	if ((self->ca_flag & (Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY)) ==
	    /*            */ (Dee_CLASS_ATTRIBUTE_FGETSET)) {
		return addr >= self->ca_addr &&
		       addr < (self->ca_addr + Dee_CLASS_GETSET_COUNT);
	}
	return addr == self->ca_addr;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeClassDescriptor_UsesClassAddr(DeeClassDescriptorObject const *__restrict self, uint16_t addr) {
	size_t attr_i;
	Dee_operator_t operator_i;
	for (operator_i = 0; operator_i <= self->cd_clsop_mask; ++operator_i) {
		struct Dee_class_operator const *op = &self->cd_clsop_list[operator_i];
		if (op->co_name != (Dee_operator_t)-1 && op->co_addr == addr)
			return true;
	}
	for (attr_i = 0; attr_i <= self->cd_cattr_mask; ++attr_i) {
		struct Dee_class_attribute const *attr = &self->cd_cattr_list[attr_i];
		if (attr->ca_name && Dee_class_attribute_uses_class_addr(attr, true, addr))
			return true;
	}
	for (attr_i = 0; attr_i <= self->cd_iattr_mask; ++attr_i) {
		struct Dee_class_attribute const *attr = &self->cd_iattr_list[attr_i];
		if (attr->ca_name && Dee_class_attribute_uses_class_addr(attr, false, addr))
			return true;
	}
	return false;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_class_attr(struct Dee_ast_class const *__restrict self,
                                 struct Dee_class_attribute const *__restrict attr, bool is_cattr,
                                 size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;

	/* Print doc attribute (if provided) */
	if (attr->ca_doc) {
		EDO(err_temp, DeeFormat_Printf(printer, arg, "@[gatw.doc(%r)]\n", attr->ca_doc));
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	}

	if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FPRIVATE)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "private "));
	if (is_cattr)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "static "));

	/* Deal with properties (getsets) */
	if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FGETSET) {
		uint16_t cb_index, cb_count;
		if (is_cattr) {
			if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FFINAL)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "<static-final> "));
			if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "@[thiscall] "));
		} else {
			if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FFINAL)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "final "));
			if (!(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD))
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "<not-thiscall> "));
			is_cattr = (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM) != 0;
		}
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "property "));
		EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " = {\n"));
		cb_count = 3;
		if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY)
			cb_count = 1;
		if (!is_cattr)
			cb_count = 0; /* Cannot lookup at compile-time */
		for (cb_index = 0; cb_index < cb_count; ++cb_index) {
			uint16_t cb_addr = attr->ca_addr + cb_index;
			DeeAst const *member_init = Dee_ast_class_findmember(self, cb_addr);
			if (member_init) {
				static char const cb_names[][4] = { "get", "del", "set" };
				char const *cb_name = cb_names[cb_index];
				EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
				EDO(err_temp, DeeFormat_Print(printer, arg, cb_name, 3));
				if (member_init->a_kind == Dee_AST_FUNCTION &&
				    (((member_init->a_data.ad_function.af_flag & Dee_AST_FUNCTION_FLAG_THISCALL) != 0) ==
				     ((attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD) != 0))) {
					EDO(err_temp, DeeAst_PrintRepr_impl_function_after_name(&member_init->a_data.ad_function,
					                                                        indent_level + 1, printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
				} else {
					EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
					EDO(err_temp, DeeAst_PrintRepr(member_init, Dee_AST_PRINTREPR_FLAG_ASEXPR,
					                               indent_level + 1, printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ";\n"));
				}
			}
		}
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
		return result;
	}

	/* Print modifier prefixes... */
	if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FFINAL) {
		EDO(err_temp, is_cattr ? DeeFormat_PRINT(printer, arg, "<static-final> ")
		                       : DeeFormat_PRINT(printer, arg, "final "));
		if (!(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY))
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "varying "));
	} else if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY) {
		if (is_cattr) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "final "));
		} else {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "<read-only> "));
		}
	}

	/* Deal with attributes wrapped as InstanceMethod objects */
	if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD) {
		if (is_cattr) {
			/* Shouldn't happen: class (static) attribute has `Dee_CLASS_ATTRIBUTE_FMETHOD` flag.
			 * This will case the method to be called with the (possibly sub-)class as this-argument. */
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "<static-instance-method> "));
		}

		/* Deal with (expected) case of instance-method attribute being stored in class-memory. */
		if (is_cattr || (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
			DeeAst const *member_init = Dee_ast_class_findmember(self, attr->ca_addr);
			if (member_init && member_init->a_kind == Dee_AST_FUNCTION &&
			    !(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD)) {
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "function "));
				EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
				EDO(err_temp, DeeAst_PrintRepr_impl_function_after_name(&member_init->a_data.ad_function,
				                                                        indent_level, printer, arg));
				return result;
			}
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "<instance-method> "));
			EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
			EDO(err_temp, member_init ? DeeFormat_PRINT(printer, arg, "del")
			                          : DeeAst_PrintRepr(member_init, Dee_AST_PRINTREPR_FLAG_ASEXPR,
			                                             indent_level, printer, arg));
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
			return result;
		}

		/* Shouldn't happen: instance-method attribute is stored in instance-memory. */
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "<instance-memory-method> "));
		EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
		return result;
	}



	/* Deal with members where we have an initializer */
	if (is_cattr || (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
		DeeAst const *member_init = Dee_ast_class_findmember(self, attr->ca_addr);
		if (member_init == NULL) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "member "));
			EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
			EDO(err_temp, DeeFormat_PRINT(printer, arg, " = del;"));
			return result;
		}

		/* Check for special case when this is a function */
		if (is_cattr && member_init->a_kind == Dee_AST_FUNCTION &&
		    !(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD)) {
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "function "));
			EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
			EDO(err_temp, DeeAst_PrintRepr_impl_function_after_name(&member_init->a_data.ad_function,
			                                                        indent_level, printer, arg));
			return result;
		}

		/* Only static attributes should get here (instance-attributes with
		 * the `Dee_CLASS_ATTRIBUTE_FCLASSMEM` flag set shouldn't exist) */
		if (!is_cattr)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "<class-memory> "));

		/* Generic attribute with static initializer */
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "member "));
		EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
		EDO(err_temp, DeeAst_PrintRepr(member_init, Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
		return result;
	}

	/* Not a GETSET / METHOD, nor is it located in CLASSMEM
	 * -> This must be a simple instance attribute (which may
	 *    have an initializer in the class's constructor) */
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "member "));
	EDO(err_temp, DeeString_PrintUtf8(Dee_AsObject(attr->ca_name), printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
	return result;
err_temp:
	return temp;
}

/* Print class, starting **after** the name (iow: at the base-expression) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_class_after_name(struct Dee_ast_class const *__restrict self,
                                       size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	DeeAst const *desc_ast = self->ac_desc;
	Dee_ssize_t temp, result = 0;
	if (self->ac_base) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ": "));
		EDO(err_temp, DeeAst_PrintRepr(self->ac_base, Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "{\n"));
	if (desc_ast->a_kind == Dee_AST_CONST &&
	    DeeClassDescriptor_Check(desc_ast->a_data.ad_const.ace_value)) {
		Dee_operator_t operator_i;
		size_t attr_i, member_i;
		DeeClassDescriptorObject const *desc;
		desc = (DeeClassDescriptorObject const *)desc_ast->a_data.ad_const.ace_value;
		if (desc->cd_flags & Dee_TP_FCLASS_AUTOINIT) {
			EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "this = default;\n"));
		}
		for (operator_i = 0; operator_i <= desc->cd_clsop_mask; ++operator_i) {
			struct Dee_class_operator const *op = &desc->cd_clsop_list[operator_i];
			if (op->co_name != (Dee_operator_t)-1) {
				DeeAst const *member_init = Dee_ast_class_findmember(self, op->co_addr);
				struct Dee_opinfo const *opinfo = DeeTypeType_GetOperatorById(&DeeType_Type, op->co_name);
				EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
				EDO(err_temp, opinfo ? DeeFormat_Printf(printer, arg, "operator %s", opinfo->oi_sname)
				                     : DeeFormat_Printf(printer, arg, "operator %" PRFu16, op->co_name));
				if (member_init == NULL) {
					EDO(err_temp, DeeFormat_PRINT(printer, arg, " = del"));
				} else if (member_init->a_kind == Dee_AST_FUNCTION &&
				           (member_init->a_data.ad_function.af_flag & Dee_AST_FUNCTION_FLAG_THISCALL)) {
					EDO(err_temp, DeeAst_PrintRepr_impl_function_after_name(&member_init->a_data.ad_function,
					                                                        indent_level + 1, printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
				} else {
					EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
					EDO(err_temp, DeeAst_PrintRepr(member_init, Dee_AST_PRINTREPR_FLAG_ASEXPR,
					                               indent_level + 1, printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ";\n"));
				}
			}
		}
		for (attr_i = 0; attr_i <= desc->cd_cattr_mask; ++attr_i) {
			struct Dee_class_attribute const *attr = &desc->cd_cattr_list[attr_i];
			if (attr->ca_name) {
				EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
				EDO(err_temp, DeeAst_PrintRepr_impl_class_attr(self, attr, true, indent_level + 1, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
			}
		}
		for (attr_i = 0; attr_i <= desc->cd_iattr_mask; ++attr_i) {
			struct Dee_class_attribute const *attr = &desc->cd_iattr_list[attr_i];
			if (attr->ca_name) {
				EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
				EDO(err_temp, DeeAst_PrintRepr_impl_class_attr(self, attr, false, indent_level + 1, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
			}
		}

		/* Print info about unreferenced members (there shouldn't be any of these) */
		for (member_i = 0; member_i < self->ac_memberc; ++member_i) {
			struct Dee_ast_class_member const *this_member = &self->ac_memberv[member_i];
			if (!DeeClassDescriptor_UsesClassAddr(desc, this_member->cm_addr)) {
				EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
				EDO(err_temp, DeeFormat_Printf(printer, arg, "<unused member#%" PRFuSIZ "> = ", member_i));
				EDO(err_temp, DeeAst_PrintRepr(this_member->cm_init, Dee_AST_PRINTREPR_FLAG_ASEXPR,
				                               indent_level + 1, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "\n"));
			}
		}
	} else {
		size_t i;
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "<unconventional descriptor>\n"));
		EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "<desc> = "));
		EDO(err_temp, DeeAst_PrintRepr(desc_ast, Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level + 1, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ";\n"));
		for (i = 0; i < self->ac_memberc; ++i) {
			struct Dee_ast_class_member const *this_member = &self->ac_memberv[i];
			EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
			EDO(err_temp, DeeFormat_Printf(printer, arg, "<member:%" PRFu16 "> = ",
			                               this_member->cm_addr));
			EDO(err_temp, DeeAst_PrintRepr(this_member->cm_init,
			                               Dee_AST_PRINTREPR_FLAG_ASEXPR,
			                               indent_level + 1, printer, arg));
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ";\n"));
		}
	}
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "}"));
	return result;
err_temp:
	return temp;
}

/* Print class as a statement/expression */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_class(struct Dee_ast_class const *__restrict self,
                            size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "class");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_class_after_name(self, indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}


/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_unpack_wr(struct Dee_ast_unpack_wr const *__restrict self,
                                Dee_ast_printrepr_flag flag, size_t indent_level,
                                Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	if (self->aupwr_targetc == 0) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "() = "));
		EDO(err_temp, DeeAst_PrintRepr(self->aupwr_expr, Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
	} else {
		for (i = 0; i < self->aupwr_targetc; ++i) {
			if (i > 0)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
			EDO(err_temp, DeeAstRValue_PrintRepr(&self->aupwr_targetv[i],
			                                     Dee_AST_PRINTREPR_FLAG_ASEXPR,
			                                     indent_level, printer, arg));
		}
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
		EDO(err_temp, DeeAst_PrintRepr(self->aupwr_expr,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                               Dee_AST_PRINTREPR_FLAG_PAREN,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "..."));
	}
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
	return result;
err_temp:
	return temp;
}



PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
Dee_ast_iasm_operand_printrepr(struct Dee_ast_iasm_operand const *__restrict self,
                               size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	if (self->aio_name) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
		EDO(err_temp, tpp_keyword_print(self->aio_name, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "] "));
	}
	if (self->aio_type) {
		EDO(err_temp, DeeFormat_Printf(printer, arg, "%$q ",
		                               self->aio_type_size,
		                               (char const *)self->aio_type));
	}
	switch (self->aio_kind) {
	case Dee_AST_IASM_OPERAND_KIND_AST:
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
		EDO(err_temp, DeeAst_PrintRepr(self->aio_data.aiod_ast,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
		break;
	case Dee_AST_IASM_OPERAND_KIND_RVALUE:
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
		EDO(err_temp, DeeAstRValue_PrintRepr(&self->aio_data.aiod_rvalue,
		                                     Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                                     indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
		break;
	case Dee_AST_IASM_OPERAND_KIND_LABEL:
		EDO(err_temp, tpp_keyword_print(self->aio_data.aiod_label, printer, arg));
		break;
	case Dee_AST_IASM_OPERAND_KIND_SYMBOL: {
		char const *label_name = Dee_asm_symbol_getname(&self->a_data.ad_asm_goto.aag_symbol);
		if (label_name == NULL)
			label_name = "<anonymous>";
		EDO(err_temp, DeeFormat_PrintStr(printer, arg, label_name));
	}	break;
	default: __builtin_unreachable();
	}
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((2, 4)) Dee_ssize_t DCALL
Dee_ast_iasm_operand_printrepr_vec(size_t count, struct Dee_ast_iasm_operand const *__restrict base,
                                   size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < count; ++i) {
		if (i != 0)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, Dee_ast_iasm_operand_printrepr(&base[i], indent_level, printer, arg));
	}
	return result;
err_temp:
	return temp;
}

struct iasm_clobber_flag {
	Dee_ast_iasm_flag iacf_flag;    /* Flag value */
	char              iacf_name[9]; /* Flag name */
};

PRIVATE struct iasm_clobber_flag const iasm_clobber_flags[] = {
	{ Dee_AST_IASM_FLAG_C_MEMORY, "memory" },
	{ Dee_AST_IASM_FLAG_C_STACK, "stack" },
	{ Dee_AST_IASM_FLAG_C_REACH, "reach" },
	{ Dee_AST_IASM_FLAG_C_NORETURN, "noreturn" },
};

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_iasm(struct Dee_ast_iasm const *__restrict self,
                           size_t indent_level, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "__asm__");
	ASSERT(self->aia_opc == (self->aia_opc_o + self->aia_opc_i + self->aia_opc_l));
	if unlikely(result < 0)
		goto done;
	if (self->aia_flag & Dee_AST_IASM_FLAG_VOLATILE)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " volatile"));
	if (self->aia_opc_l)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " goto"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	if (!(self->aia_flag & Dee_AST_IASM_FLAG_FORMAT)) {
		if (self->aia_opc != 0 || (self->aia_flag & Dee_AST_IASM_FLAG_C_ALL))
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "<no-format> "));
	}
	EDO(err_temp, DeeFormat_Printf(printer, arg, "%$q",
	                               (size_t)self->aia_text_size,
	                               (char const *)self->aia_text));
	if (self->aia_opc != 0 || (self->aia_flag & Dee_AST_IASM_FLAG_C_ALL))
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " :"));
	if (self->aia_opc_o) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
		EDO(err_temp, Dee_ast_iasm_operand_printrepr_vec(self->aia_opc_o, self->aia_opv,
		                                                 indent_level, printer, arg));
	}
	if ((self->aia_opc_i != 0 || self->aia_opc_l != 0) || (self->aia_flag & Dee_AST_IASM_FLAG_C_ALL))
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " :"));
	if (self->aia_opc_i) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " "));
		EDO(err_temp, Dee_ast_iasm_operand_printrepr_vec(self->aia_opc_i,
		                                                 self->aia_opv + self->aia_opc_o,
		                                                 indent_level, printer, arg));
	}
	if (self->aia_opc_l != 0) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " : "));
		EDO(err_temp, Dee_ast_iasm_operand_printrepr_vec(self->aia_opc_l,
		                                                 self->aia_opv + self->aia_opc_o + self->aia_opc_i,
		                                                 indent_level, printer, arg));
	}
	if (self->aia_flag & Dee_AST_IASM_FLAG_C_ALL) {
		size_t i;
		bool is_first = true;
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " : "));
		for (i = 0; i < COMPILER_LENOF(iasm_clobber_flags); ++i) {
			if (self->aia_flag & iasm_clobber_flags[i].iacf_flag) {
				if (!is_first)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
				EDO(err_temp, DeeFormat_PrintStr(printer, arg, iasm_clobber_flags[i].iacf_name));
				is_first = false;
			}
		}
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ");"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_iasm_asexpr(struct Dee_ast_iasm const *__restrict self, size_t indent_level,
                                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_iasm(self, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_operator_nopostop(struct Dee_ast_operator const *__restrict self,
                                        Dee_ast_printrepr_flag flag,
                                        size_t indent_level, char const *argv0_repr_override,
                                        Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	struct Dee_opinfo const *opinfo;
	ASSERT(Dee_ast_operator_id_hasid(&self->ao_id)); /* Other types not implemented, yet */
	ASSERT(self->ao_argc > 1);
	opinfo = DeeTypeType_GetOperatorById(&DeeType_Type, Dee_ast_operator_id_getid(&self->ao_id));
	if (!(self->ao_flag & Dee_AST_OPERATOR_FLAG_VARARG)) {
		char const *unary_repr = NULL;
		char const *binary_repr = NULL;
		switch (Dee_ast_operator_id_getid(&self->ao_id)) {

			/* Special cases... */
		case OPERATOR_CALL:
			if (self->ao_argc == 2) {
				DeeAst const *unpack_args;
				result = argv0_repr_override
				         ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				         : DeeAst_PrintRepr(self->ao_argv[0],
				                            Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                            Dee_AST_PRINTREPR_FLAG_PAREN,
				                            indent_level, printer, arg);
				if unlikely(result < 0)
					goto done;
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
				unpack_args = self->ao_argv[1];
				if (unpack_args->a_kind == Dee_AST_ACT1_UNPACK) {
					DeeAst const *args = unpack_args->a_data.ad_act1.aa_act0;
					if (args->a_kind == Dee_AST_MULTI &&
					    args->a_data.ad_multi.am_kind == Dee_AST_MULTI_KIND_TUPLE) {
						size_t i;
						for (i = 0; i < args->a_data.ad_multi.am_count; ++i) {
							DeeAst const *this_arg = args->a_data.ad_multi.am_elem[i];
							if (i != 0)
								EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
							EDO(err_temp, DeeAst_PrintRepr(this_arg, Dee_AST_PRINTREPR_FLAG_ASEXPR,
							                               indent_level, printer, arg));
						}
						EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
						goto done;
					} else if (args->a_kind == Dee_AST_CONST &&
					           DeeTuple_Check(args->a_data.ad_const.ace_value)) {
						DeeTupleObject *args_tuple = (DeeTupleObject *)args->a_data.ad_const.ace_value;
						size_t i;
						for (i = 0; i < DeeTuple_SIZE(args_tuple); ++i) {
							DeeObject *this_arg = DeeTuple_GET(args_tuple, i);
							if (i != 0)
								EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
							EDO(err_temp, DeeObject_PrintRepr(this_arg, printer, arg));
						}
						EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
						goto done;
					}
				}
				EDO(err_temp, DeeAst_PrintRepr(unpack_args,
				                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                               Dee_AST_PRINTREPR_FLAG_PAREN,
				                               indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "...)"));
				goto done;
			}
			break;

		case OPERATOR_GETITEM:
			if (self->ao_argc == 2) {
				result = argv0_repr_override
				         ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				         : DeeAst_PrintRepr(self->ao_argv[0],
				                            Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                            Dee_AST_PRINTREPR_FLAG_PAREN,
				                            indent_level, printer, arg);
				if unlikely(result < 0)
					goto done;
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
				EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[1], Dee_AST_PRINTREPR_FLAG_ASEXPR,
				                               indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "]"));
				goto done;
			}
			break;

		case OPERATOR_DELITEM:
			if (self->ao_argc == 2) {
				result = DeeFormat_PRINT(printer, arg, "del(");
				if unlikely(result < 0)
					goto done;
				EDO(err_temp, argv0_repr_override
				              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				              : DeeAst_PrintRepr(self->ao_argv[0],
				                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                                 Dee_AST_PRINTREPR_FLAG_PAREN,
				                                 indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
				EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[1], Dee_AST_PRINTREPR_FLAG_ASEXPR,
				                               indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "])"));
				goto done;
			}
			break;

		case OPERATOR_SETITEM:
			if (self->ao_argc == 3) {
				result = 0;
				if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
				EDO(err_temp, argv0_repr_override
				              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				              : DeeAst_PrintRepr(self->ao_argv[0],
				                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                                 Dee_AST_PRINTREPR_FLAG_PAREN,
				                                 indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
				EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[1], Dee_AST_PRINTREPR_FLAG_ASEXPR,
				                               indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "] = "));
				EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[2], Dee_AST_PRINTREPR_FLAG_ASEXPR,
				                               indent_level, printer, arg));
				if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
				goto done;
			}
			break;

		case OPERATOR_GETRANGE:
			if (self->ao_argc == 3) {
				DeeAst const *start = self->ao_argv[1];
				DeeAst const *end = self->ao_argv[2];
				result = argv0_repr_override
				         ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				         : DeeAst_PrintRepr(self->ao_argv[0],
				                            Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                            Dee_AST_PRINTREPR_FLAG_PAREN,
				                            indent_level, printer, arg);
				if unlikely(result < 0)
					goto done;
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
				if (start->a_kind != Dee_AST_CONST || !DeeNone_Check(start->a_data.ad_const.ace_value))
					EDO(err_temp, DeeAst_PrintRepr(start, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ":"));
				if (end->a_kind != Dee_AST_CONST || !DeeNone_Check(end->a_data.ad_const.ace_value))
					EDO(err_temp, DeeAst_PrintRepr(end, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "]"));
				goto done;
			}
			break;

		case OPERATOR_DELRANGE:
			if (self->ao_argc == 3) {
				DeeAst const *start = self->ao_argv[1];
				DeeAst const *end = self->ao_argv[2];
				result = DeeFormat_PRINT(printer, arg, "del(");
				if unlikely(result < 0)
					goto done;
				EDO(err_temp, argv0_repr_override
				              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				              : DeeAst_PrintRepr(self->ao_argv[0],
				                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                                 Dee_AST_PRINTREPR_FLAG_PAREN,
				                                 indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
				if (start->a_kind != Dee_AST_CONST || !DeeNone_Check(start->a_data.ad_const.ace_value))
					EDO(err_temp, DeeAst_PrintRepr(start, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ":"));
				if (end->a_kind != Dee_AST_CONST || !DeeNone_Check(end->a_data.ad_const.ace_value))
					EDO(err_temp, DeeAst_PrintRepr(end, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "])"));
				goto done;
			}
			break;

		case OPERATOR_SETRANGE:
			if (self->ao_argc == 4) {
				DeeAst const *start = self->ao_argv[1];
				DeeAst const *end = self->ao_argv[2];
				result = 0;
				if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
				EDO(err_temp, argv0_repr_override
				              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
				              : DeeAst_PrintRepr(self->ao_argv[0],
				                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
				                                 Dee_AST_PRINTREPR_FLAG_PAREN,
				                                 indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
				if (start->a_kind != Dee_AST_CONST || !DeeNone_Check(start->a_data.ad_const.ace_value))
					EDO(err_temp, DeeAst_PrintRepr(start, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ":"));
				if (end->a_kind != Dee_AST_CONST || !DeeNone_Check(end->a_data.ad_const.ace_value))
					EDO(err_temp, DeeAst_PrintRepr(end, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "] = "));
				EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[3], Dee_AST_PRINTREPR_FLAG_ASEXPR,
				                               indent_level, printer, arg));
				if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
				goto done;
			}
			break;

		case OPERATOR_GETATTR:
			if (self->ao_argc == 2) {
				DeeAst const *attr_ast = self->ao_argv[1];
				if (attr_ast->a_kind == Dee_AST_CONST &&
				    DeeString_Check(attr_ast->a_data.ad_const.ace_value)) {
					result = argv0_repr_override
					         ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
					         : DeeAst_PrintRepr(self->ao_argv[0],
					                            Dee_AST_PRINTREPR_FLAG_ASEXPR |
					                            Dee_AST_PRINTREPR_FLAG_PAREN,
					                            indent_level, printer, arg);
					if unlikely (result < 0)
						goto done;
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "."));
					EDO(err_temp, DeeString_PrintUtf8(attr_ast->a_data.ad_const.ace_value,
					                                  printer, arg));
					goto done;
				}
			}
			break;

		case OPERATOR_DELATTR:
			if (self->ao_argc == 2) {
				DeeAst const *attr_ast = self->ao_argv[1];
				if (attr_ast->a_kind == Dee_AST_CONST &&
				    DeeString_Check(attr_ast->a_data.ad_const.ace_value)) {
					result = DeeFormat_PRINT(printer, arg, "del(");
					if unlikely (result < 0)
						goto done;
					EDO(err_temp, argv0_repr_override
					              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
					              : DeeAst_PrintRepr(self->ao_argv[0],
					                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
					                                 Dee_AST_PRINTREPR_FLAG_PAREN,
					                                 indent_level, printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "."));
					EDO(err_temp, DeeString_PrintUtf8(attr_ast->a_data.ad_const.ace_value,
					                                  printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
					goto done;
				}
			}
			break;

		case OPERATOR_SETATTR:
			if (self->ao_argc == 3) {
				DeeAst const *attr_ast = self->ao_argv[1];
				if (attr_ast->a_kind == Dee_AST_CONST &&
				    DeeString_Check(attr_ast->a_data.ad_const.ace_value)) {
					result = 0;
					if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
						EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
					EDO(err_temp, argv0_repr_override
					              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
					              : DeeAst_PrintRepr(self->ao_argv[0],
					                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
					                                 Dee_AST_PRINTREPR_FLAG_PAREN,
					                                 indent_level, printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, "."));
					EDO(err_temp, DeeString_PrintUtf8(attr_ast->a_data.ad_const.ace_value,
					                                  printer, arg));
					EDO(err_temp, DeeFormat_PRINT(printer, arg, " = "));
					EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[2], Dee_AST_PRINTREPR_FLAG_ASEXPR,
					                               indent_level, printer, arg));
					if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
						EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
					goto done;
				}
			}
			break;

		case OPERATOR_COPY: unary_repr = "copy("; break;
		case OPERATOR_ASSIGN: binary_repr = " := "; break;
		case OPERATOR_STR: unary_repr = "str("; break;
		case OPERATOR_REPR: unary_repr = "repr("; break;
		case OPERATOR_BOOL: unary_repr = "(!!"; break;
		case OPERATOR_INV: unary_repr = "(~"; break;
		case OPERATOR_POS: unary_repr = "(+"; break;
		case OPERATOR_NEG: unary_repr = "(-"; break;
		case OPERATOR_ADD: binary_repr = " + "; break;
		case OPERATOR_SUB: binary_repr = " - "; break;
		case OPERATOR_MUL: binary_repr = " * "; break;
		case OPERATOR_DIV: binary_repr = " / "; break;
		case OPERATOR_MOD: binary_repr = " % "; break;
		case OPERATOR_SHL: binary_repr = " << "; break;
		case OPERATOR_SHR: binary_repr = " >> "; break;
		case OPERATOR_AND: binary_repr = " & "; break;
		case OPERATOR_OR: binary_repr = " | "; break;
		case OPERATOR_XOR: binary_repr = " ^ "; break;
		case OPERATOR_POW: binary_repr = " * "; break;
		case OPERATOR_INC: unary_repr = "(++"; break;
		case OPERATOR_DEC: unary_repr = "(--"; break;
		case OPERATOR_INPLACE_ADD: binary_repr = " += "; break;
		case OPERATOR_INPLACE_SUB: binary_repr = " -= "; break;
		case OPERATOR_INPLACE_MUL: binary_repr = " *= "; break;
		case OPERATOR_INPLACE_DIV: binary_repr = " /= "; break;
		case OPERATOR_INPLACE_MOD: binary_repr = " %= "; break;
		case OPERATOR_INPLACE_SHL: binary_repr = " <<= "; break;
		case OPERATOR_INPLACE_SHR: binary_repr = " >>= "; break;
		case OPERATOR_INPLACE_AND: binary_repr = " &= "; break;
		case OPERATOR_INPLACE_OR:  binary_repr = " |= "; break;
		case OPERATOR_INPLACE_XOR: binary_repr = " ^ "; break;
		case OPERATOR_INPLACE_POW: binary_repr = " **= "; break;
		case OPERATOR_EQ: binary_repr = " == "; break;
		case OPERATOR_NE: binary_repr = " != "; break;
		case OPERATOR_LO: binary_repr = " < "; break;
		case OPERATOR_LE: binary_repr = " <= "; break;
		case OPERATOR_GR: binary_repr = " > "; break;
		case OPERATOR_GE: binary_repr = " >= "; break;
		case OPERATOR_SIZE: unary_repr = "(#"; break;
		default: break;
		}

		/* Common path for unary operators */
		if (unary_repr && self->ao_argc == 1) {
			size_t unary_repr_len = strlen(unary_repr);
			if (!(flag & Dee_AST_PRINTREPR_FLAG_PAREN)) {
				/* Remove leading parenthesis */
				if (*unary_repr == '(') {
					--unary_repr_len;
					++unary_repr;
				}
			}
			result = DeeFormat_Print(printer, arg, unary_repr, unary_repr_len);
			if unlikely(result < 0)
				goto done;
			EDO(err_temp, argv0_repr_override
			              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
			              : DeeAst_PrintRepr(self->ao_argv[0],
			                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
			                                 Dee_AST_PRINTREPR_FLAG_PAREN,
			                                 indent_level, printer, arg));
			if (unary_repr_len && (unary_repr[0] == '(' ||
			                       unary_repr[unary_repr_len - 1] == '('))
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
			goto done;
		}

		/* Common path for binary operators... */
		if (binary_repr && self->ao_argc == 2) {
			result = 0;
			if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
			EDO(err_temp, argv0_repr_override
			              ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
			              : DeeAst_PrintRepr(self->ao_argv[0],
			                                 Dee_AST_PRINTREPR_FLAG_ASEXPR |
			                                 Dee_AST_PRINTREPR_FLAG_PAREN,
			                                 indent_level, printer, arg));
			EDO(err_temp, DeeFormat_PrintStr(printer, arg, binary_repr));
			EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[1],
			                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
			                               Dee_AST_PRINTREPR_FLAG_PAREN,
			                               indent_level, printer, arg));
			if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
			goto done;
		}
	}


	/* Fallback: print as a generic operator expression */
	result = argv0_repr_override
	         ? DeeFormat_PrintStr(printer, arg, argv0_repr_override)
	         : DeeAst_PrintRepr(self->ao_argv[0],
	                            Dee_AST_PRINTREPR_FLAG_ASEXPR |
	                            Dee_AST_PRINTREPR_FLAG_PAREN,
	                            indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ".operator "));
	EDO(err_temp, opinfo ? DeeFormat_PrintStr(printer, arg, opinfo->oi_sname)
	                     : DeeFormat_Printf(printer, arg, "%" PRFu16,
	                                        Dee_ast_operator_id_getid(&self->ao_id)));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, " ("));
	{
		size_t i;
		Dee_ast_printrepr_flag repr_flags = Dee_AST_PRINTREPR_FLAG_ASEXPR;
		if (self->ao_flag & Dee_AST_OPERATOR_FLAG_VARARG)
			repr_flags |= Dee_AST_PRINTREPR_FLAG_PAREN;
		for (i = 1; i < self->ao_argc; ++i) {
			if (i > 1)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
			EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[i], repr_flags,
			                               indent_level, printer, arg));
		}
	}
	if (self->ao_flag & Dee_AST_OPERATOR_FLAG_VARARG)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "..."));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

/* @param: flag: for `Dee_AST_PRINTREPR_FLAG_PAREN` only */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_operator(struct Dee_ast_operator const *__restrict self,
                               Dee_ast_printrepr_flag flag, size_t indent_level,
                               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	ASSERT(Dee_ast_operator_id_hasid(&self->ao_id)); /* Other types not implemented, yet */
	if (!(self->ao_flag & Dee_AST_OPERATOR_FLAG_POSTOP))
		return DeeAst_PrintRepr_impl_operator_nopostop(self, flag, indent_level, NULL, printer, arg);
	ASSERT(self->ao_argc >= 1);
	if (!(self->ao_flag & Dee_AST_OPERATOR_FLAG_VARARG) && self->ao_argc == 1 &&
	    (Dee_ast_operator_id_getid(&self->ao_id) == OPERATOR_INC ||
	     Dee_ast_operator_id_getid(&self->ao_id) == OPERATOR_DEC)) {
		result = 0;
		if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
		EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[0],
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                               Dee_AST_PRINTREPR_FLAG_PAREN,
		                               indent_level, printer, arg));
		EDO(err_temp, (Dee_ast_operator_id_getid(&self->ao_id) == OPERATOR_INC)
		              ? DeeFormat_PRINT(printer, arg, "++")
		              : DeeFormat_PRINT(printer, arg, "--"));
		if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
			EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
		goto done;
	}

	result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	result = DeeFormat_PRINT(printer, arg, "local <postop-temp> = copy(");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(self->ao_argv[0], Dee_AST_PRINTREPR_FLAG_ASEXPR,
	                               indent_level + 1, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ");\n"));
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
	EDO(err_temp, DeeAst_PrintRepr_impl_operator_nopostop(self, Dee_AST_PRINTREPR_FLAG_ASEXPR,
	                                                      indent_level + 1, "<postop-temp>",
	                                                      printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";\n"));
	EDO(err_temp, DeeFormat_Repeat(printer, arg, '\t', indent_level + 1));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "<postop-temp>;"));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

/* Print the representation of a print-statement. */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((2, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_printact_impl(DeeAst const *file_arg, DeeAst const *print_args,
                                    bool is_print_ln, size_t indent_level,
                                    Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "print ");
	if unlikely(result < 0)
		goto done;
	if (file_arg) {
		EDO(err_temp, DeeAst_PrintRepr(file_arg, Dee_AST_PRINTREPR_FLAG_ASEXPR,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ": "));
	}
	if (print_args->a_kind == Dee_AST_MULTI &&
	    Dee_AST_MULTI_KIND_ISSEQ(print_args->a_data.ad_multi.am_kind)) {
		size_t i;
		for (i = 0; i < print_args->a_data.ad_multi.am_count; ++i) {
			if (i > 0)
				EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
			EDO(err_temp, DeeAst_PrintRepr(print_args->a_data.ad_multi.am_elem[i],
			                               Dee_AST_PRINTREPR_FLAG_ASEXPR,
			                               indent_level, printer, arg));
		}
	} else {
		EDO(err_temp, DeeAst_PrintRepr(print_args,
		                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
		                               Dee_AST_PRINTREPR_FLAG_PAREN,
		                               indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "..."));
	}
	if (!is_print_ln)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ","));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((2, 6)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_printact(DeeAst const *file_arg, DeeAst const *print_args,
                               bool is_print_ln, Dee_ast_printrepr_flag flag, size_t indent_level,
                               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
		return DeeAst_PrintRepr_impl_printact_impl(file_arg, print_args, is_print_ln, indent_level, printer, arg);
	result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_printact_impl(file_arg, print_args, is_print_ln, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_del_stmt(DeeAst const *__restrict expr, size_t indent_level,
                               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "del ");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(expr, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_del_stmt_asexpr(DeeAst const *__restrict expr, size_t indent_level,
                                      Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr_stmt_as_expr_start(indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr_impl_del_stmt(expr, indent_level + 1, printer, arg));
	EDO(err_temp, DeeAst_PrintRepr_stmt_as_expr_end(indent_level, printer, arg));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((2, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_act1(enum Dee_ast_kind act1_kind,
                           DeeAst const *__restrict argv0,
                           Dee_ast_printrepr_flag flag, size_t indent_level,
                           Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char const *unary_prefix;
	char const *unary_suffix;
	size_t unary_prefix_len;
	switch (act1_kind) {
	case Dee_AST_ACT1_SCOPE: {
		if (argv0->a_kind == Dee_AST_MULTI) {
			size_t count = argv0->a_data.ad_multi.am_count;
			DeeAst const *const *elem = argv0->a_data.ad_multi.am_elem;
			if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) {
				return DeeAst_PrintRepr_impl_multi_as_stmt_in_expr(count, elem, indent_level,
				                                                   true, printer, arg);
			}
			return DeeAst_PrintRepr_impl_multi_as_stmt_brace(count, elem, indent_level,
			                                                 true, printer, arg);
		}
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) {
			return DeeAst_PrintRepr_impl_multi_as_stmt_in_expr(1, (DeeAst const *const *)&argv0,
			                                                   indent_level, true, printer, arg);
		}
		return DeeAst_PrintRepr_impl_multi_as_stmt_brace(1, (DeeAst const *const *)&argv0,
		                                                 indent_level, true, printer, arg);
	}	break;
	case Dee_AST_ACT1_PRINT:
		return DeeAst_PrintRepr_impl_printact(NULL, argv0, false, flag, indent_level, printer, arg);
	case Dee_AST_ACT1_PRINTLN:
		return DeeAst_PrintRepr_impl_printact(NULL, argv0, true, flag, indent_level, printer, arg);
	case Dee_AST_ACT1_NOT: unary_prefix = "(!"; goto do_unary_prefix;
	case Dee_AST_ACT1_TYPEOF: unary_prefix = "type("; goto do_unary_prefix;
	case Dee_AST_ACT1_CLASSOF: unary_suffix = ".class"; goto do_unary_suffix;
	case Dee_AST_ACT1_SUPEROF: unary_suffix = ".super"; goto do_unary_suffix;
	case Dee_AST_ACT1_ASSERT: unary_prefix = "assert("; goto do_unary_prefix;
	case Dee_AST_ACT1_DEEPCOPY: unary_prefix = "deepcopy("; goto do_unary_prefix;
	case Dee_AST_ACT1_UNPACK: unary_suffix = "..."; goto do_unary_suffix;
	case Dee_AST_ACT1_DEL:
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_del_stmt_asexpr(argv0, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_del_stmt(argv0, indent_level, printer, arg);
	case Dee_AST_ACT1_UNBIND: unary_prefix = "del("; goto do_unary_prefix;
	case Dee_AST_ACT1_BOUND: unary_prefix = "bound("; goto do_unary_prefix;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
do_unary_suffix:
	result = DeeAst_PrintRepr(argv0,
	                          Dee_AST_PRINTREPR_FLAG_ASEXPR |
	                          Dee_AST_PRINTREPR_FLAG_PAREN,
	                          indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_PrintStr(printer, arg, unary_suffix));
	goto done_maybe_print_semicolon;
do_unary_prefix:
	unary_prefix_len = strlen(unary_prefix);
	if (!(flag & Dee_AST_PRINTREPR_FLAG_PAREN)) {
		/* Remove leading parenthesis */
		if (*unary_prefix == '(') {
			--unary_prefix_len;
			++unary_prefix;
		}
	}
	result = DeeFormat_Print(printer, arg, unary_prefix, unary_prefix_len);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(argv0,
	                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
	                               Dee_AST_PRINTREPR_FLAG_PAREN,
	                               indent_level, printer, arg));
	if (unary_prefix_len && (unary_prefix[0] == '(' ||
	                         unary_prefix[unary_prefix_len - 1] == '('))
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done_maybe_print_semicolon:
	if (!(flag & Dee_AST_PRINTREPR_FLAG_ASEXPR))
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_assert2(DeeAst const *__restrict argv0,
                              DeeAst const *__restrict argv1,
                              size_t indent_level,
                              Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "assert(");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeAst_PrintRepr(argv0, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ","));
	EDO(err_temp, DeeAst_PrintRepr(argv1, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_boundattr_impl(DeeAst const *__restrict obj,
                                     DeeAst const *__restrict attr,
                                     size_t indent_level,
                                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr(obj, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	if (attr->a_kind == Dee_AST_CONST && DeeString_Check(attr->a_data.ad_const.ace_value)) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "."));
		EDO(err_temp, DeeString_PrintUtf8(attr->a_data.ad_const.ace_value, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, " is bound"));
	} else {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ".operator . ("));
		EDO(err_temp, DeeAst_PrintRepr(attr, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ") is bound"));
	}
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_boundattr(DeeAst const *__restrict obj,
                                DeeAst const *__restrict attr,
                                Dee_ast_printrepr_flag flag, size_t indent_level,
                                Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	EDO(err_temp, DeeAst_PrintRepr_impl_boundattr_impl(obj, attr, indent_level, printer, arg));
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_bounditem_impl(DeeAst const *__restrict obj,
                                     DeeAst const *__restrict item,
                                     size_t indent_level,
                                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeAst_PrintRepr(obj, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg);
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "["));
	EDO(err_temp, DeeAst_PrintRepr(item, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "] is bound"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_bounditem(DeeAst const *__restrict obj,
                                DeeAst const *__restrict item,
                                Dee_ast_printrepr_flag flag, size_t indent_level,
                                Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	EDO(err_temp, DeeAst_PrintRepr_impl_bounditem_impl(obj, item, indent_level, printer, arg));
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((2, 3, 6)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_act2(enum Dee_ast_kind act2_kind,
                           DeeAst const *__restrict argv0,
                           DeeAst const *__restrict argv1,
                           Dee_ast_printrepr_flag flag, size_t indent_level,
                           Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char const *binary_repr;
	switch (act2_kind) {
	case Dee_AST_ACT2_FPRINT:
		return DeeAst_PrintRepr_impl_printact(argv0, argv1, false, flag, indent_level, printer, arg);
	case Dee_AST_ACT2_FPRINTLN:
		return DeeAst_PrintRepr_impl_printact(argv0, argv1, true, flag, indent_level, printer, arg);
	case Dee_AST_ACT2_ASSERT:
		result = DeeAst_PrintRepr_impl_assert2(argv0, argv1, indent_level, printer, arg);
		if unlikely(result < 0)
			goto done;
		goto done_maybe_print_semicolon;
	case Dee_AST_ACT2_BOUNDATTR:
		result = DeeAst_PrintRepr_impl_boundattr(argv0, argv1, flag, indent_level, printer, arg);
		if unlikely(result < 0)
			goto done;
		goto done_maybe_print_semicolon;
	case Dee_AST_ACT2_BOUNDITEM:
		result = DeeAst_PrintRepr_impl_bounditem(argv0, argv1, flag, indent_level, printer, arg);
		if unlikely(result < 0)
			goto done;
		goto done_maybe_print_semicolon;
	case Dee_AST_ACT2_IS: binary_repr = " is "; goto do_binary_repr;
	case Dee_AST_ACT2_IN: binary_repr = " in "; goto do_binary_repr;
	case Dee_AST_ACT2_AS: binary_repr = " as "; goto do_binary_repr;
	case Dee_AST_ACT2_SAMEOBJ: binary_repr = " === "; goto do_binary_repr;
	case Dee_AST_ACT2_DIFFOBJ: binary_repr = " !== "; goto do_binary_repr;
	case Dee_AST_ACT2_STORE: binary_repr = " = "; goto do_binary_repr;
	case Dee_AST_ACT2_INIT: binary_repr = " <INIT>= "; goto do_binary_repr;
	case Dee_AST_ACT2_XCH: binary_repr = " <XCH>= "; goto do_binary_repr;
	default: __builtin_unreachable();
	}
do_binary_repr:
	result = 0;
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "("));
	EDO(err_temp, DeeAst_PrintRepr(argv0,
	                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
	                               Dee_AST_PRINTREPR_FLAG_PAREN,
	                               indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PrintStr(printer, arg, binary_repr));
	EDO(err_temp, DeeAst_PrintRepr(argv1,
	                               Dee_AST_PRINTREPR_FLAG_ASEXPR |
	                               Dee_AST_PRINTREPR_FLAG_PAREN,
	                               indent_level, printer, arg));
	if (flag & Dee_AST_PRINTREPR_FLAG_PAREN)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done_maybe_print_semicolon:
	if (!(flag & Dee_AST_PRINTREPR_FLAG_ASEXPR))
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ";"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 3, 5)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl_range(DeeAst const *__restrict start,
                            DeeAst const *__restrict end,
                            DeeAst const *__restrict step,
                            size_t indent_level,
                            Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = DeeFormat_PRINT(printer, arg, "[");
	if unlikely(result < 0)
		goto done;
	if (start->a_kind != Dee_AST_CONST || !DeeNone_Check(start->a_data.ad_const.ace_value))
		EDO(err_temp, DeeAst_PrintRepr(start, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ":"));
	if (end->a_kind != Dee_AST_CONST || !DeeNone_Check(end->a_data.ad_const.ace_value))
		EDO(err_temp, DeeAst_PrintRepr(end, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	if (step->a_kind != Dee_AST_CONST || !DeeNone_Check(step->a_data.ad_const.ace_value)) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", "));
		EDO(err_temp, DeeAst_PrintRepr(step, Dee_AST_PRINTREPR_FLAG_ASEXPR, indent_level, printer, arg));
	}
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "]"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr_impl(DeeAst const *__restrict self,
                      Dee_ast_printrepr_flag flag, size_t indent_level,
                      Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	switch (self->a_kind) {

	case Dee_AST_CONST: {
		DeeObject *value = self->a_data.ad_const.ace_value;
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeObject_PrintRepr(value, printer, arg);
		return DeeFormat_Printf(printer, arg, "%r;", value);
	}	break;

	case Dee_AST_MULTI: {
		size_t count = self->a_data.ad_multi.am_count;
		DeeAst const *const *elem = self->a_data.ad_multi.am_elem;
		switch (self->a_data.ad_multi.am_kind) {
		case Dee_AST_MULTI_KIND_LAST:
			if (!(flag & (Dee_AST_PRINTREPR_FLAG_ASEXPR | Dee_AST_PRINTREPR_FLAG_ASSTMT))) {
				/* Special case: brace-less multi-statements are allowed */
				return DeeAst_PrintRepr_impl_multi_as_stmt(count, elem, indent_level, printer, arg);
			}
			if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
				return DeeAst_PrintRepr_impl_multi_as_stmt_in_expr(count, elem, indent_level, false, printer, arg);
			return DeeAst_PrintRepr_impl_multi_as_stmt_brace(count, elem, indent_level, false, printer, arg);

		case Dee_AST_MULTI_KIND_GENERIC_SEQ:
			result = DeeAst_PrintRepr_impl_multi_as_seq(count, elem, indent_level, printer, arg, '{', '}');
			goto maybe_print_semicolon;
		case Dee_AST_MULTI_KIND_GENERIC_SET:
			result = DeeAst_PrintRepr_impl_multi_as_seq_set(count, elem, indent_level, printer, arg);
			goto maybe_print_semicolon;
		case Dee_AST_MULTI_KIND_TUPLE:
			result = DeeAst_PrintRepr_impl_multi_as_seq(count, elem, indent_level, printer, arg, '(', ')');
			goto maybe_print_semicolon;
		case Dee_AST_MULTI_KIND_LIST:
			result = DeeAst_PrintRepr_impl_multi_as_seq(count, elem, indent_level, printer, arg, '[', ']');
			goto maybe_print_semicolon;
		case Dee_AST_MULTI_KIND_HASHSET:
			result = DeeAst_PrintRepr_impl_multi_as_seq_hashset(count, elem, indent_level, printer, arg);
			goto maybe_print_semicolon;
		case Dee_AST_MULTI_KIND_GENERIC_MAP:
			result = DeeAst_PrintRepr_impl_multi_as_map(count, elem, indent_level, printer, arg);
			goto maybe_print_semicolon;
		case Dee_AST_MULTI_KIND_DICT:
			result = DeeAst_PrintRepr_impl_multi_as_map_dict(count, elem, indent_level, printer, arg);
			goto maybe_print_semicolon;
		default: __builtin_unreachable();
		}
		__builtin_unreachable();
	}	break;

	case Dee_AST_LOOP: {
		Dee_ssize_t prefix_status = 0;
		DeeAst const *body = self->a_data.ad_loop.al_body;
		if (self->a_data.ad_loop.al_flag & Dee_AST_LOOP_FLAG_UNLIKELY) {
			prefix_status = DeeFormat_PRINT(printer, arg, "@[unlikely] ");
			if unlikely(prefix_status < 0)
				return prefix_status;
		}
		if (self->a_data.ad_loop.al_flag & Dee_AST_LOOP_FLAG_FOREACH) {
			DeeAst const *elem = self->a_data.ad_loop.al_data.ald_foreach.alf_elem;
			DeeAst const *iter = self->a_data.ad_loop.al_data.ald_foreach.alf_iter;
			if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) {
				result = DeeAst_PrintRepr_impl_foreach_asexpr(elem, iter, body, indent_level, printer, arg);
			} else {
				result = DeeAst_PrintRepr_impl_foreach(elem, iter, body, indent_level, printer, arg);
			}
		} else {
			DeeAst const *cond = self->a_data.ad_loop.al_data.ald_normal.aln_cond;
			DeeAst const *next = self->a_data.ad_loop.al_data.ald_normal.aln_next;
			if (cond && (self->a_data.ad_loop.al_flag & Dee_AST_LOOP_FLAG_POSTCOND)) {
				if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) {
					result = DeeAst_PrintRepr_impl_do_while_asexpr(cond, next, body, indent_level, printer, arg);
				} else {
					result = DeeAst_PrintRepr_impl_do_while(cond, next, body, indent_level, printer, arg);
				}
			} else if (cond && !next) {
				if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) {
					result = DeeAst_PrintRepr_impl_while_asexpr(cond, body, indent_level, printer, arg);
				} else {
					result = DeeAst_PrintRepr_impl_while(cond, body, indent_level, printer, arg);
				}
			} else {
				if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) {
					result = DeeAst_PrintRepr_impl_for_asexpr(cond, next, body, indent_level, printer, arg);
				} else {
					result = DeeAst_PrintRepr_impl_for(cond, next, body, indent_level, printer, arg);
				}
			}
		}
		if (result >= 0)
			result += prefix_status;
		return result;
	}	break;

	case Dee_AST_IF: {
		Dee_ssize_t prefix_status = 0;
		DeeAst const *cond = self->a_data.ad_if.ai_cond;
		DeeAst const *tt   = self->a_data.ad_if.ai_tt;
		DeeAst const *ff   = self->a_data.ad_if.ai_ff;
		bool cond_is_none = false;
		switch (self->a_data.ad_if.ai_flag & (Dee_AST_IF_FLAG_LIKELY | Dee_AST_IF_FLAG_UNLIKELY)) {
		case 0: break;
		case Dee_AST_IF_FLAG_LIKELY:
			prefix_status = DeeFormat_PRINT(printer, arg, "@[likely] ");
			break;
		case Dee_AST_IF_FLAG_UNLIKELY:
			prefix_status = DeeFormat_PRINT(printer, arg, "@[unlikely] ");
			break;
		case Dee_AST_IF_FLAG_LIKELY | Dee_AST_IF_FLAG_UNLIKELY:
			prefix_status = DeeFormat_PRINT(printer, arg, "@[likely, unlikely] ");
			break;
		default: __builtin_unreachable();
		}
		if unlikely(prefix_status < 0)
			return prefix_status;
		if ((self->a_data.ad_if.ai_flag & Dee_AST_IF_FLAG_COND) == Dee_AST_IF_FLAG_COND_ISNONE) {
			if (cond == ff) {
				result = DeeAst_PrintRepr_impl_qmark_qmark(cond, tt, flag, indent_level, printer, arg);
				if (result >= 0)
					result += prefix_status;
				goto maybe_print_semicolon;
			}
			cond_is_none = true;
		}
		if ((flag & Dee_AST_PRINTREPR_FLAG_ASEXPR) ||
		    (cond == tt || cond == ff)) {
			result = DeeAst_PrintRepr_impl_qmark(cond, tt, ff, cond_is_none, flag, indent_level, printer, arg);
			if (result >= 0)
				result += prefix_status;
			goto maybe_print_semicolon;
		}
		result = DeeAst_PrintRepr_impl_if(cond, tt, ff, indent_level, printer, arg);
		if (result >= 0)
			result += prefix_status;
		return result;
	}	break;

	case Dee_AST_SWITCH_STMT: {
		DeeAst const *expr = self->a_data.ad_switch_stmt.asws_expr;
		DeeAst const *body = self->a_data.ad_switch_stmt.asws_body;
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_switch_stmt_asexpr(expr, body, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_switch_stmt(expr, body, indent_level, printer, arg);
	}	break;

	case Dee_AST_SWITCH_EXPR:
		return DeeAst_PrintRepr_impl_switch_expr(&self->a_data.ad_switch_expr, indent_level, printer, arg);

	case Dee_AST_ASM_GOTO: {
		char const *name = Dee_asm_symbol_getname(&self->a_data.ad_asm_goto.aag_symbol);
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_goto_asexpr(name, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_goto(name, printer, arg);
	}	break;

	case Dee_AST_CATCH:
		return DeeAst_PrintRepr_impl_catch(flag, indent_level,
		                                   self->a_data.ad_catch.ac_body,
		                                   self->a_data.ad_catch.ac_handlerc,
		                                   self->a_data.ad_catch.ac_handlerv,
		                                   printer, arg);

	case Dee_AST_FINALLY:
		return DeeAst_PrintRepr_impl_finally(flag, indent_level,
		                                     self->a_data.ad_finally.af_body,
		                                     self->a_data.ad_finally.af_finally,
		                                     printer, arg);

	case Dee_AST_YIELD:
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_yield_asexpr(self->a_data.ad_yield.ay_expr, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_yield(self->a_data.ad_yield.ay_expr, indent_level, printer, arg);

	case Dee_AST_RETURN:
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_return_asexpr(self->a_data.ad_return.ar_expr, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_return(self->a_data.ad_return.ar_expr, indent_level, printer, arg);

	case Dee_AST_FUNCTION:
		return DeeAst_PrintRepr_impl_function(&self->a_data.ad_function, indent_level, printer, arg);

	case Dee_AST_CLASS:
		return DeeAst_PrintRepr_impl_class(&self->a_data.ad_class, indent_level, printer, arg);

	case Dee_AST_UNPACK_WR:
		result = DeeAst_PrintRepr_impl_unpack_wr(&self->a_data.ad_unpack_wr, flag, indent_level, printer, arg);
		goto maybe_print_semicolon;

	case Dee_AST_IASM:
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_iasm_asexpr(&self->a_data.ad_iasm, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_iasm(&self->a_data.ad_iasm, indent_level, printer, arg);

	case Dee_AST_OPERATOR:
		result = DeeAst_PrintRepr_impl_operator(&self->a_data.ad_operator, flag, indent_level, printer, arg);
		goto maybe_print_semicolon;

	Dee_AST_CASE_ACT1
		return DeeAst_PrintRepr_impl_act1(self->a_kind,
		                                  self->a_data.ad_act1.aa_act0,
		                                  flag, indent_level, printer, arg);
	Dee_AST_CASE_ACT2
		return DeeAst_PrintRepr_impl_act2(self->a_kind,
		                                  self->a_data.ad_act2.aa_act0,
		                                  self->a_data.ad_act2.aa_act1,
		                                  flag, indent_level, printer, arg);

	case Dee_AST_ACT3_RANGE:
		return DeeAst_PrintRepr_impl_range(self->a_data.ad_act3.aa_act0,
		                                   self->a_data.ad_act3.aa_act1,
		                                   self->a_data.ad_act3.aa_act2,
		                                   indent_level, printer, arg);

	case Dee_AST_SYMBOL_RD:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMBOL_BND:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMBOL_DEL:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMBOL_WR:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMBOL_XCH:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMBOL_INIT:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMREF:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_SYMREF_ALIAS:
		return DeeFormat_PRINT(printer, arg, "TODO");
	case Dee_AST_IMPORT_STAR:
		return DeeFormat_PRINT(printer, arg, "TODO");

	case Dee_AST_GOTO: {
		char const *name = tpp_keyword_getcstr(self->a_data.ad_goto.ach_symname);
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_goto_asexpr(name, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_goto(name, printer, arg);
	}	break;

	case Dee_AST_BREAK: {
		char const *name = self->a_data.ad_break.ab_symname
		                   ? tpp_keyword_getcstr(self->a_data.ad_break.ab_symname)
		                   : NULL;
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_break_asexpr("break", name, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_break("break", name, printer, arg);
	}	break;

	case Dee_AST_CONTINUE: {
		char const *name = self->a_data.ad_break.ab_symname
		                   ? tpp_keyword_getcstr(self->a_data.ad_break.ab_symname)
		                   : NULL;
		if (flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)
			return DeeAst_PrintRepr_impl_break_asexpr("continue", name, indent_level, printer, arg);
		return DeeAst_PrintRepr_impl_break("continue", name, printer, arg);
	}	break;

	default: __builtin_unreachable();
	}
	__builtin_unreachable();
maybe_print_semicolon:
	if (result >= 0 && !(flag & Dee_AST_PRINTREPR_FLAG_ASEXPR)) {
		Dee_ssize_t temp = DeeFormat_PRINT(printer, arg, ";");
		if (temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

/* Print a user-code representation of `self` */
INTERN WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeAst_PrintRepr(DeeAst const *__restrict self,
                 Dee_ast_printrepr_flag flag, size_t indent_level,
                 Dee_formatprinter_t printer, void *arg) {
	/* Deal with label prefixes */
	Dee_ssize_t result;
	struct Dee_ast_labels *labels = self->a_labels;
	if (labels == NULL)
		return DeeAst_PrintRepr_impl(self, flag, indent_level, printer, arg);
	result = Dee_ast_labels_printrepr(labels, indent_level, printer, arg);
	if likely(result >= 0) {
		Dee_ssize_t temp = DeeAst_PrintRepr_impl(self, flag, indent_level, printer, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}
#endif /* DeeAst_HAVE_PRINT_REPR */
#endif /* CONFIG_EXPERIMENTAL_USE_STATELESS_PARSER */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_AST_PRINTREPR_C */
