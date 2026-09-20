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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TAG_C
#define GUARD_DEEMON_COMPILER_LEXER_TAG_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_*alloc*, Dee_Free */
#include <deemon/code.h>            /* Dee_CODE_F* */
#include <deemon/compiler/ast.h>    /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>  /* AST_ANNOTATION_FNOFUNC, AST_ANNOTATION_FNORMAL, ast_* */
#include <deemon/compiler/symbol.h> /* LOOKUP_SYM_NORMAL */
#include <deemon/compiler/tpp.h>
#include <deemon/object.h>          /* DREF */
#include <deemon/string.h>          /* DeeUni_IsLF, Dee_UNICODE_PRINTER_ISEMPTY, Dee_unicode_printer* */
#include <deemon/system-features.h> /* bcmpc, bzero, memcpy, memmoveupc */
#include <deemon/type.h>            /* DeeObject_IsShared, OPERATOR_CALL, TP_F* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

INTERN struct ast_tags current_tags;

/* Apply & free annotations to the given `input` ast. */
INTERN WUNUSED NONNULL((1, 2)) DREF struct ast *DCALL
ast_annotations_apply(struct ast_annotations *__restrict self,
                      /*inherit(always)*/ DREF struct ast *__restrict input) {
	DREF struct ast *merge, **expr_v, *args;
	while (self->an_annoc) {
		struct ast *func = self->an_annov[self->an_annoc - 1].aa_func;
		if ((self->an_annov[self->an_annoc - 1].aa_flag & AST_ANNOTATION_FNOFUNC) ||
		    (func->a_type != AST_OPERATOR) || (func->a_flag != OPERATOR_CALL) ||
		    (func->a_operator.o_op1 == NULL)) {
			/* Invoke the annotation function using the current input:
			 * >> input = aa_func(input); */
			expr_v = (struct ast **)Dee_Mallocc(1, sizeof(struct ast *));
			if unlikely(!expr_v)
				goto err_input;
			expr_v[0] = input; /* Inherit reference. */
			args      = ast_multiple(AST_FMULTIPLE_TUPLE, 1, expr_v);
			args      = ast_setddi(args, &func->a_ddi);
			if unlikely(!args) {
				Dee_Free(expr_v);
				goto err_input;
			}
			merge = ast_operator2(OPERATOR_CALL,
			                      AST_OPERATOR_FNORMAL,
			                      func, args);
		} else {
			/* Invoke the annotation function using the current input:
			 * >> input = aa_func.op0(input, aa_func.op1...); */
			struct ast *base;
			base = func->a_operator.o_op0;
			args = func->a_operator.o_op1;
#ifdef CONFIG_AST_IS_STRUCT
			if (args->a_refcnt == 1)
#else /* CONFIG_AST_IS_STRUCT */
			if (!DeeObject_IsShared(args))
#endif /* !CONFIG_AST_IS_STRUCT */
			{
				if (args->a_type == AST_MULTIPLE &&
				    args->a_flag == AST_FMULTIPLE_TUPLE) {
					expr_v = (DREF struct ast **)Dee_Reallocc(args->a_multiple.m_astv,
					                                          args->a_multiple.m_astc + 1,
					                                          sizeof(DREF struct ast *));
					if unlikely(!expr_v)
						goto err_input;
					memmoveupc(expr_v + 1,
					           expr_v,
					           args->a_multiple.m_astc,
					           sizeof(DREF struct ast *));
					expr_v[0]               = input; /* inherit reference. */
					args->a_multiple.m_astv = expr_v;
					++args->a_multiple.m_astc;
					ast_incref(args);
					goto set_merge_from_inherit_args;
				}
			}
			merge = ast_expand(args);
			merge = ast_setddi(merge, &func->a_ddi);
			if unlikely(!merge)
				goto err_input;
			expr_v = (struct ast **)Dee_Mallocc(2, sizeof(struct ast *));
			if unlikely(!expr_v)
				goto err_input_merge;
			expr_v[0] = input; /* Inherit reference. */
			expr_v[1] = merge; /* Inherit reference. */
			args      = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE, 2, expr_v), &func->a_ddi);
			if unlikely(!args) {
				Dee_Free(expr_v);
				goto err_input_merge;
			}
set_merge_from_inherit_args:
			merge = ast_operator2(OPERATOR_CALL,
			                      AST_OPERATOR_FNORMAL,
			                      base, args);
		}
		ast_decref_unlikely(args);
		if unlikely(!merge)
			goto err;
		input = ast_setddi(merge, &func->a_ddi);
		--self->an_annoc;
		ast_decref(self->an_annov[self->an_annoc].aa_func);
	}
	ast_annotations_free(self);
	return input;
err_input_merge:
	ast_decref(merge);
err_input:
	ast_decref(input);
err:
	ast_annotations_free(self);
	return NULL;
}


/* Capture all currently saved annotations. */
INTERN NONNULL((1)) void DCALL
ast_annotations_get(struct ast_annotations *__restrict result) {
	memcpy(result, &current_tags.at_anno, sizeof(struct ast_annotations));
	bzero(&current_tags.at_anno, sizeof(struct ast_annotations));
}

INTERN NONNULL((1)) void DCALL
ast_annotations_free(struct ast_annotations *__restrict self) {
	if (!self->an_annov)
		return;
	while (self->an_annoc) {
		--self->an_annoc;
		ast_decref(self->an_annov[self->an_annoc].aa_func);
	}
	if (!current_tags.at_anno.an_annov) {
		memcpy(&current_tags.at_anno, self, sizeof(struct ast_annotations));
	} else if (!current_tags.at_anno.an_annoc &&
	           current_tags.at_anno.an_annoa < self->an_annoa) {
		Dee_Free(current_tags.at_anno.an_annov);
		memcpy(&current_tags.at_anno, self, sizeof(struct ast_annotations));
	} else {
		Dee_Free(self->an_annov);
	}
}

/* Clear annotations, and warn if some were given. */
INTERN WUNUSED NONNULL((1, 2)) int DFCALL
ast_annotations_clear(DeeLexer *lexer, struct ast_annotations *__restrict self) {
	if (!self->an_annov)
		goto done;
	while (self->an_annoc) {
		if (DeeLexer_WarnfAst(lexer, self->an_annov[self->an_annoc].aa_func, TPP_W_UNUSED_ANNOTATION))
			goto err;
		--self->an_annoc;
		ast_decref(self->an_annov[self->an_annoc].aa_func);
	}
	if (!current_tags.at_anno.an_annov) {
		memcpy(&current_tags.at_anno, self, sizeof(struct ast_annotations));
	} else if (!current_tags.at_anno.an_annoc &&
	           current_tags.at_anno.an_annoa < self->an_annoa) {
		Dee_Free(current_tags.at_anno.an_annov);
		memcpy(&current_tags.at_anno, self, sizeof(struct ast_annotations));
	} else {
		Dee_Free(self->an_annov);
	}
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
ast_annotations_add(struct ast *__restrict func, uint16_t flag) {
	ASSERT(current_tags.at_anno.an_annoc <= current_tags.at_anno.an_annoa);
	if (current_tags.at_anno.an_annoc >= current_tags.at_anno.an_annoa) {
		struct ast_annotation *new_anno;
		size_t new_alloc = current_tags.at_anno.an_annoa * 2;
		if (!new_alloc)
			new_alloc = 2;
		new_anno = (struct ast_annotation *)Dee_TryReallocc(current_tags.at_anno.an_annov, new_alloc,
		                                                    sizeof(struct ast_annotation));
		if unlikely(!new_anno) {
			new_alloc = current_tags.at_anno.an_annoc + 1;
			new_anno = (struct ast_annotation *)Dee_Reallocc(current_tags.at_anno.an_annov, new_alloc,
			                                                 sizeof(struct ast_annotation));
			if unlikely(!new_anno)
				goto err;
		}
		current_tags.at_anno.an_annoa = new_alloc;
		current_tags.at_anno.an_annov = new_anno;
	}
	ast_incref(func);
	current_tags.at_anno.an_annov[current_tags.at_anno.an_annoc].aa_flag = flag;
	current_tags.at_anno.an_annov[current_tags.at_anno.an_annoc].aa_func = func;
	++current_tags.at_anno.an_annoc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DFCALL ast_tags_clear(DeeLexer *self) {
	while (current_tags.at_anno.an_annoc) {
		struct ast_annotation *anno;
		anno = &current_tags.at_anno.an_annov[current_tags.at_anno.an_annoc - 1];
		if (DeeLexer_WarnfAst(self, anno->aa_func, TPP_W_UNUSED_ANNOTATION))
			goto err;
		ast_decref(anno->aa_func);
		--current_tags.at_anno.an_annoc;
	}
	if (!Dee_UNICODE_PRINTER_ISEMPTY(&current_tags.at_decl)) {
		Dee_unicode_printer_fini(&current_tags.at_decl);
		Dee_unicode_printer_init(&current_tags.at_decl);
	}
	if (!Dee_UNICODE_PRINTER_ISEMPTY(&current_tags.at_doc)) {
		Dee_unicode_printer_fini(&current_tags.at_doc);
		Dee_unicode_printer_init(&current_tags.at_doc);
	}
	current_tags.at_expect      = 0;
	current_tags.at_class_flags = 0;
	return 0;
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) int DFCALL
append_decl_string(DeeLexer *self) {
	ASSERT(TPP_TOK_ISSTRING(DeeLexer_GetTok(self)));
	do {
		if unlikely(ast_parse_string_const_printer(self, &current_tags.at_decl))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
	} while (DeeLexer_IsStringToken(self));

	/* Append a line-feed at the end. */
	return Dee_unicode_printer_putascii(&current_tags.at_decl, '\n');
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
convert_dot_tag_namespace(DeeLexer *self, char const *__restrict tag_name_str) {
	if unlikely(DeeLexer_GetTok(self) == ':' ||
	            DeeLexer_GetTok(self) == TPP_TOK_COLON_COLON) {
		if (DeeLexer_Warnf(self, TPP_W_COMPILER_TAG_EXPECTED_DOT_AFTER_KEYWORD,
		                   tag_name_str))
			goto err;
		DeeLexer_SetTokenId(self, TPP_TOK_OFCHAR('.'));
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DFCALL
parse_tags(DeeLexer *self) {
	if (DeeLexer_GetTok(self) == '@') {
		/* Line-style documentation string (terminated by a line-feed) */
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		tpp_unichar uc;
		tpp_errno error;
		tpp_char const *doc_end = DeeLexer_GetTokenEnd(self);
		tpp_file *const file = DeeLexer_GetFile(self);
		for (;;) {
			/* NOTE: `tpp_lexer_readunichar()` already handles BSE and-the-like,
			 *       so we don't have to do anything other than watching out for
			 *       line-feed characters, and writing unicode characters as we
			 *       read them. */
			error = tpp_lexer_readunichar(&self->dl_lexer, &doc_end, &uc);
			if (TPP_ISERR(error))
				goto err;
			if (tpp_unicode_islf(uc) || (uc == 0 && doc_end == tpp_file_getend(file)))
				break;
			if unlikely(Dee_unicode_printer_putc(&current_tags.at_doc, uc))
				goto err;
		}
		/* Set file pointer to parse the next token after
		 * the terminating line-feed (see the yield below) */
		tpp_file_setpos(file, doc_end);
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		char const *doc_start = (char const *)DeeLexer_GetTokenEnd(self);
		char const *doc_end   = doc_start;
		char const *file_end  = TPPLexer_Current->l_token.t_file->f_end;
		while (doc_end < file_end) {
			char ch = *doc_end;
			if (ch == '\\') {
				if (++doc_end >= file_end)
					break;
				ch = *doc_end;
				if (ch == '\r') {
					ch = *++doc_end;
					if (doc_end >= file_end)
						break;
					if (ch == '\n') {
						ch = *++doc_end;
						if (doc_end >= file_end)
							break;
					}
				} else if (ch == '\n') {
					++doc_end;
				}
			} else if (DeeUni_IsLF(ch)) {
				break;
			} else {
				++doc_end;
			}
		}
		TPPLexer_Current->l_token.t_file->f_pos = (char *)doc_end;
		if unlikely(Dee_unicode_printer_print(&current_tags.at_doc, doc_start,
		                                      (size_t)(doc_end - doc_start)) < 0)
			goto err;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		if unlikely(Dee_unicode_printer_putascii(&current_tags.at_doc, '\n'))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
	} else if (DeeLexer_GetTok(self) == '[') {
		/* Implementation-specific / compile-time tags */
		char const *tag_name_str;
		size_t tag_name_len;
		bool is_optional;
#define IS_TAG(x)                              \
		(tag_name_len == COMPILER_STRLEN(x) && \
		 bcmpc(tag_name_str, x, COMPILER_STRLEN(x), sizeof(char)) == 0)
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
again_compiler_tag:
		is_optional = false;
again_compiler_subtag:
		if (!DeeLexer_HasTokenKwd(self)) {
			if (DeeLexer_Warnf(self, TPP_W_COMPILER_TAG_EXPECTED_KEYWORD))
				goto err;
		} else {
			tag_name_str = DeeLexer_GetTokenKwdCStr(self);
			tag_name_len = DeeLexer_GetTokenKwdLen(self);

			/* Trim leading/trailing underscores from tag names (prevent ambiguity when macros are used).
			 * NOTE: Doing this is an extension implemented by the GATW implementation */
			while (tag_name_len && *tag_name_str == '_')
				++tag_name_str, --tag_name_len;
			while (tag_name_len && tag_name_str[tag_name_len - 1] == '_')
				--tag_name_len;

			/* Compiler annotation required by the standard. */
			if (IS_TAG("interrupt")) {
				current_tags.at_class_flags |= TP_FINTERRUPT;
			} else if (IS_TAG("likely")) {
				current_tags.at_expect |= AST_FCOND_LIKELY;
			} else if (IS_TAG("unlikely")) {
				current_tags.at_expect |= AST_FCOND_UNLIKELY;
			} else if (IS_TAG("copyable")) {
				current_tags.at_code_flags |= Dee_CODE_FCOPYABLE;
			} else if (IS_TAG("inline")) {
				/* Optional tag to try to inline an annotated function.
				 * The tag is ignored if inlining could alter the behavior
				 * of the program. - only function that are local+final, or
				 * local+implicit-final (write-once), or global+final can
				 * be inlined.
				 * Even then, this tag can be ignored, and even without
				 * this tag, the compiler is allowed to inline functions
				 * when inlining them is possible, and doing so is deemed
				 * to be advantageous.
				 * Compilers are allowed to simply ignore this tag. */
			} else if (IS_TAG("optional")) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if unlikely(convert_dot_tag_namespace(self, tag_name_str))
					goto err;
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('.'), W_COMPILER_TAG_EXPECTED_DOT_AFTER_OPTIONAL))
					goto err;
				is_optional = true;
				goto again_compiler_subtag;
			} else if (IS_TAG("gatw")) {
				/* The annotation namespace used by our implementation (GATW). */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if unlikely(convert_dot_tag_namespace(self, tag_name_str))
					goto err;
				if (DeeLexer_GetTok(self) != '.')
					goto warn_unknown_tag;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if (!DeeLexer_HasTokenKwd(self))
					goto err_no_keyword_after_dot;
				tag_name_str = DeeLexer_GetTokenKwdCStr(self);
				tag_name_len = DeeLexer_GetTokenKwdLen(self);
				while (tag_name_len && *tag_name_str == '_')
					++tag_name_str, --tag_name_len;
				while (tag_name_len && tag_name_str[tag_name_len - 1] == '_')
					--tag_name_len;
				if (IS_TAG("truncate")) {
					current_tags.at_class_flags |= TP_FTRUNCATE;
				} else if (IS_TAG("moveany")) {
					current_tags.at_class_flags |= TP_FMOVEANY;
				} else if (IS_TAG("final")) {
					current_tags.at_class_flags |= TP_FFINAL;
				} else if (IS_TAG("interrupt")) {
					current_tags.at_class_flags |= TP_FINTERRUPT;
				} else if (IS_TAG("likely")) {
					current_tags.at_expect |= AST_FCOND_LIKELY;
				} else if (IS_TAG("unlikely")) {
					current_tags.at_expect |= AST_FCOND_UNLIKELY;
				} else if (IS_TAG("copyable")) {
					current_tags.at_code_flags |= Dee_CODE_FCOPYABLE;
				} else if (IS_TAG("assembly")) {
					current_tags.at_code_flags |= Dee_CODE_FASSEMBLY;
				} else if (IS_TAG("lenient")) {
					current_tags.at_code_flags |= Dee_CODE_FLENIENT;
				} else if (IS_TAG("thiscall")) {
					current_tags.at_code_flags |= Dee_CODE_FTHISCALL;
				} else if (IS_TAG("heapframe")) {
					current_tags.at_code_flags |= Dee_CODE_FHEAPFRAME;
				} else if (IS_TAG("finally")) {
					current_tags.at_code_flags |= Dee_CODE_FFINALLY;
				} else if (IS_TAG("constructor")) {
					current_tags.at_code_flags |= Dee_CODE_FCONSTRUCTOR;
				} else if (IS_TAG("doc")) {
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
					if (DeeLexer_GetTok(self) != '(' && is_optional)
						goto do_next_compiler_tag;
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
					{
						tpp_token_id tok = DeeLexer_Require(self, TPP_TOK_OFCHAR('('));
						if (TPP_TOK_ISERR(tok))
							goto err;
						if (tok == ',' || tok == ']')
							goto do_next_compiler_tag;
					}
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
					if (DeeLexer_GetTok(self) != '(') {
						if (DeeLexer_Warnf(self, TPP_W_COMPILER_TAG_EXPECTED_LPAREN_AFTER_DOC))
							goto err;
						if (DeeLexer_GetTok(self) == ',' || DeeLexer_GetTok(self) == ']')
							goto do_next_compiler_tag;
					} else {
						if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
							goto err;
					}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
					if likely(DeeLexer_IsStringToken(self)) {
						if unlikely(append_decl_string(self))
							goto err;
					} else {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
						if (DeeLexer_Warnf(self, TPP_W_EXPECTED_STRING))
							goto err;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
						if (DeeLexer_Warnf(self, TPP_W_COMPILER_TAG_EXPECTED_STRING_AFTER_DOC))
							goto err;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
					}
					if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(')'), W_COMPILER_TAG_EXPECTED_RPAREN_AFTER_DOC))
						goto err;
					goto do_next_compiler_tag;
				} else {
					goto warn_unknown_tag_yield;
				}
			} else {
warn_unknown_tag_yield:
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
warn_unknown_tag:
				if unlikely(convert_dot_tag_namespace(self, tag_name_str))
					goto err;
				if (!is_optional) {
					if (DeeLexer_Warnf(self,
					                   DeeLexer_GetTok(self) == '.'
					                   ? TPP_W_COMPILER_TAG_UNKNOWN_NS
					                   : TPP_W_COMPILER_TAG_UNKNOWN,
					                   tag_name_str))
						goto err;
				}
again_check_tag_namespace:
				if (DeeLexer_GetTok(self) == '.') {
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err;
					if (!DeeLexer_HasTokenKwd(self)) {
err_no_keyword_after_dot:
						if (DeeLexer_Warnf(self, TPP_W_COMPILER_TAG_EXPECTED_KEYWORD_AFTER_DOT,
						                   tag_name_str))
							goto err;
					} else {
						tag_name_str = DeeLexer_GetTokenKwdCStr(self);
						tag_name_len = DeeLexer_GetTokenKwdLen(self);
						if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
							goto err;
					}
					if unlikely(convert_dot_tag_namespace(self, tag_name_str))
						goto err;
					goto again_check_tag_namespace;
				}
				if (DeeLexer_GetTok(self) == '(') {
					/* Skip tag argument list. */
					unsigned int recursion = 1;
					while (DeeLexer_GetTok(self) != TPP_TOK_EOF) {
						if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
							goto err;
						if (DeeLexer_GetTok(self) == '(') {
							++recursion;
						} else if (DeeLexer_GetTok(self) == ')') {
							if (recursion == 1) {
								if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
									goto err;
								break;
							}
							--recursion;
						}
					}
				}
				goto do_next_compiler_tag;
			}
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if unlikely(convert_dot_tag_namespace(self, tag_name_str))
				goto err;
			if unlikely(DeeLexer_GetTok(self) == '.')
				goto warn_unknown_tag;
		}
do_next_compiler_tag:
		if (DeeLexer_GetTok(self) == ',') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			if (DeeLexer_GetTok(self) != ']')
				goto again_compiler_tag;
		}
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(']'), W_COMPILER_TAG_EXPECTED_RBRACKET))
			goto err;
#undef IS_TAG
	} else {
		uint16_t flags;
		int error;
		DREF struct ast *annotation;
		flags = AST_ANNOTATION_FNORMAL;
		if (DeeLexer_GetTok(self) == '(')
			flags |= AST_ANNOTATION_FNOFUNC;
		annotation = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!annotation)
			goto err;
		error = ast_annotations_add(annotation, flags);
		ast_decref_unlikely(annotation);
		if unlikely(error)
			goto err;
		goto done;
	}
done:
	return 0;
err:
	return -1;
}

/* Parse tags at the current lexer position, starting
 * immediately at the `@` token.
 * >> @doc("foo"), doc("bar")
 *    ^                      ^
 *    entry                  exit */
INTERN WUNUSED NONNULL((1)) int DFCALL
parse_tags_block(DeeLexer *self) {
	while (DeeLexer_GetTok(self) == '@') {
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if unlikely(parse_tags(self))
			goto err;
	}
	return 0;
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_TAG_C */
