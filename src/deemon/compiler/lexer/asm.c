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
#ifndef GUARD_DEEMON_COMPILER_LEXER_ASM_C
#define GUARD_DEEMON_COMPILER_LEXER_ASM_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/format.h>

#include "../../runtime/strings.h"

DECL_BEGIN

#define OPERAND_TYPE_OUTPUT 0
#define OPERAND_TYPE_INPUT  1
#define OPERAND_TYPE_LABEL  2
#define OPERAND_TYPE_COUNT  3
struct operand_list {
	size_t              ol_a;  /* Allocated amount of operands. */
	size_t              ol_c;  /* Amount of operands in use. */
	struct asm_operand *ol_v;  /* [0..ol_c|ALLOC(ol_a)][owned] Vector of operands. */
	size_t              ol_count[OPERAND_TYPE_COUNT]; /* Different operand counts. */
};

PRIVATE void DCALL
operand_list_fini(struct operand_list *__restrict self) {
	struct asm_operand *iter, *end;
	ASSERT(self->ol_c <= self->ol_a);
	ASSERT(self->ol_count[OPERAND_TYPE_OUTPUT] +
	       self->ol_count[OPERAND_TYPE_INPUT] +
	       self->ol_count[OPERAND_TYPE_LABEL] ==
	       self->ol_c);
	end = (iter = self->ol_v) +
	      (self->ol_count[OPERAND_TYPE_OUTPUT] +
	       self->ol_count[OPERAND_TYPE_INPUT]);
	for (; iter != end; ++iter) {
		ASSERT(iter->ao_type);
		ASSERT(iter->ao_expr);
		TPPString_Decref(iter->ao_type);
		ast_decref(iter->ao_expr);
	}
	end = self->ol_v + self->ol_c;
	for (; iter != end; ++iter) {
		ASSERT(!iter->ao_type);
		ASSERT(iter->ao_label);
		ASSERT(iter->ao_label->tl_goto);
		--iter->ao_label->tl_goto;
	}
	Dee_Free(self->ol_v);
}

/* Allocate and return a new operand.
 * NOTE: The caller is responsible for safely initializing this operand. */
PRIVATE struct asm_operand *DCALL
operand_list_add(struct operand_list *__restrict self,
                 unsigned int type) {
	struct asm_operand *result = self->ol_v;
	ASSERT(self->ol_c <= self->ol_a);
	ASSERT(type < OPERAND_TYPE_COUNT);
	if (self->ol_c == self->ol_a) {
		size_t new_alloc = self->ol_a * 2;
		if (!new_alloc)
			new_alloc = 2;
do_realloc:
		result = (struct asm_operand *)Dee_TryRealloc(self->ol_v, new_alloc *
		                                                          sizeof(struct asm_operand));
		if unlikely(!result) {
			if (new_alloc != self->ol_c + 1) {
				new_alloc = self->ol_c + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemory(new_alloc * sizeof(struct asm_operand)))
				goto do_realloc;
			return NULL;
		}
		self->ol_v = result;
		self->ol_a = new_alloc;
	}
	/* Reserve the next operand number. */
	result += self->ol_c++;
	++self->ol_count[type];
	return result;
}

PRIVATE int DCALL
asm_parse_operands(struct operand_list *__restrict list,
                   unsigned int type) {
	/*ref*/ struct TPPString *operand_type;
	DREF struct ast *operand_value;
	struct asm_operand *operand;
	while ((type == OPERAND_TYPE_LABEL
	        ? TPP_ISKEYWORD(tok)
	        : (tok == TOK_STRING ||
	           (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS)))) ||
	       (tok == '[')) {
#ifndef CONFIG_LANGUAGE_NO_ASM
		struct TPPKeyword *name = NULL;
#endif /* CONFIG_LANGUAGE_NO_ASM */
		if (tok == '[') {
			if unlikely(yield() < 0)
				goto err;
			if (TPP_ISKEYWORD(tok)) {
#ifndef CONFIG_LANGUAGE_NO_ASM
				name = token.t_kwd;
#endif /* CONFIG_LANGUAGE_NO_ASM */
				if unlikely(yield() < 0)
					goto err;
			} else {
				if (WARN(W_EXPECTED_KEYWORD_FOR_OPERAND_NAME))
					goto err;
			}
			if unlikely(likely(tok == ']') ? (yield() < 0) : WARN(W_EXPECTED_RBRACKET_AFTER_OPERAND_NAME))
				goto err;
		}
		if (type == OPERAND_TYPE_LABEL) {
			struct text_label *label_value;
			/* Label operand. */
			if (TPP_ISKEYWORD(tok)) {
				label_value = lookup_label(token.t_kwd);
				if unlikely(!label_value)
					goto err;
				if unlikely(yield() < 0)
					goto err;
			} else {
				if (WARN(W_EXPECTED_KEYWORD_FOR_LABEL_OPERAND))
					goto err;
				label_value = lookup_label(&TPPKeyword_Empty);
				if unlikely(!label_value)
					goto err;
			}
			operand = operand_list_add(list, type);
			if unlikely(!operand)
				goto err;
			/* Add the usage-reference to the label. */
			++label_value->tl_goto;
			operand->ao_label = label_value;
			operand->ao_type  = NULL;
		} else {
			if (tok == TOK_STRING ||
			    (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
				operand_type = TPPLexer_ParseString();
				if unlikely(!operand_type)
					goto err;
			} else {
				if (WARN(W_EXPECTED_STRING_BEFORE_OPERAND_VALUE))
					goto err;
				operand_type = TPPString_NewEmpty();
			}
			if (tok == KWD_pack) {
				if unlikely(yield() < 0)
					goto err_type;
				if (tok == '(')
					goto with_paren;
				operand_value = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!operand_value)
					goto err_type;
			} else {
with_paren:
				if unlikely(likely(tok == '(') ? (yield() < 0) : WARN(W_EXPECTED_LPAREN_BEFORE_OPERAND_VALUE))
					goto err_type;
				operand_value = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!operand_value)
					goto err_type;
				if unlikely(likely(tok == ')') ? (yield() < 0) : WARN(W_EXPECTED_RPAREN_AFTER_OPERAND_VALUE))
					goto err_value;
			}
			operand = operand_list_add(list, type);
			if unlikely(!operand)
				goto err_value;
			operand->ao_type = operand_type;  /* Inherit */
			operand->ao_expr = operand_value; /* Inherit */
		}
#ifndef CONFIG_LANGUAGE_NO_ASM
		operand->ao_name = name;
#endif /* CONFIG_LANGUAGE_NO_ASM */
		/* Yield the trailing comma. */
		if (tok != ',')
			break;
		if unlikely(yield() < 0)
			goto err;
	}
	return 0;
err_value:
	ast_decref(operand_value);
err_type:
	TPPString_Decref(operand_type);
err:
	return -1;
}




struct clobber_desc {
	char     cd_name[12]; /* Name of the clobber-operand. */
	uint16_t cd_flags;    /* Set of `AST_FASSEMBLY_*' defined for this descriptor. */
};

PRIVATE struct clobber_desc const clobber_descs[] = {
	{ "memory",   AST_FASSEMBLY_MEMORY },   /* Memory barrier. */
	{ "reach",    AST_FASSEMBLY_REACH },    /* User-assembly can be reached through non-conventional means. */
	{ "noreturn", AST_FASSEMBLY_NORETURN }, /* User-assembly doesn't return normally (i.e. returns using `ret', or `throw').
	                                         * NOTE: This flag can also be used to hint to the optimizer
	                                         *       that a branch is not reachable:
	                                         * >> if (never_true()) {
	                                         * >>     __asm__("" : : : "noreturn");
	                                         * >>     print "This print statement can be optimized away";
	                                         * >> }
	                                         */
	{ "sp",       AST_FASSEMBLY_CLOBSP },   /* Don't warn about miss-aligned stack pointers. */
	{ "cc", 0 },                            /* Ignored... */
};

/* Parse the clobber list and return a set of `AST_FASSEMBLY_*' */
PRIVATE int32_t DCALL asm_parse_clobber(void) {
	struct TPPString *name;
	uint16_t result = 0;
	while (tok == TOK_STRING ||
	       (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
		name = TPPLexer_ParseString();
		if unlikely(!name)
			goto err;
		if (name->s_size < COMPILER_LENOF(clobber_descs[0].cd_name)) {
			struct clobber_desc const *iter = clobber_descs;
			for (; iter != COMPILER_ENDOF(clobber_descs); ++iter) {
				if (name->s_size == strlen(iter->cd_name) &&
				    memcmp(name->s_text, iter->cd_name, name->s_size * sizeof(char)) == 0) {
					/* Found it! Set the proper flags and continue. */
					result |= iter->cd_flags;
					goto got_clobber;
				}
			}
		}
		if (WARN(W_UNKNOWN_CLOBBER_NAME, name->s_text))
			goto err_name;
got_clobber:
		TPPString_Decref(name);
		/* Yield the trailing comma. */
		if (tok != ',')
			break;
		if unlikely(yield() < 0)
			goto err;
	}
	return result;
err_name:
	TPPString_Decref(name);
err:
	return -1;
}

LOCAL bool DCALL is_collon(void) {
	if (tok == ':')
		return true;
	if (tok == TOK_COLLON_COLLON ||
	    tok == TOK_COLLON_EQUAL) {
		/* Convert to a `:'-token and setup the lexer to re-parse
		 * the remainder of the current token as part of the next. */
		token.t_id          = ':';
		token.t_end         = token.t_begin + 1;
		token.t_file->f_pos = token.t_end;
		return true;
	}
	return false;
}


struct tpp_string_printer {
	struct TPPString *sp_string; /* [0..1][owned] String buffer. */
	size_t            sp_length; /* Used string length. */
};

PRIVATE int
(TPPCALL tpp_string_printer_append)(char const *__restrict buf, size_t bufsize,
                                    struct tpp_string_printer *__restrict self) {
	struct TPPString *string;
	size_t alloc_size;
	ASSERT(self);
	ASSERT(buf || !bufsize);
	if ((string = self->sp_string) == NULL) {
		/* Make sure not to allocate a string when the used length remains ZERO.
		 * >> Must be done to assure the expectation of `if(sp_length == 0) sp_string == NULL' */
		if unlikely(!bufsize)
			return 0;
		/* Allocate the initial string. */
		alloc_size = 8;
		while (alloc_size < bufsize)
			alloc_size *= 2;
alloc_again:
		string = (struct TPPString *)Dee_TryMalloc(offsetof(struct TPPString, s_text) +
		                                           (alloc_size + 1) * sizeof(char));
		if unlikely(!string) {
			if (alloc_size != bufsize) {
				alloc_size = bufsize;
				goto alloc_again;
			}
			if (Dee_CollectMemory(offsetof(struct TPPString, s_text) +
			                      (alloc_size + 1) * sizeof(char)))
				goto alloc_again;
			return -1;
		}
		self->sp_string = string;
		string->s_size  = alloc_size;
		memcpy(string->s_text, buf, bufsize * sizeof(char));
		self->sp_length = bufsize;
		goto done;
	}
	alloc_size = string->s_size;
	ASSERT(alloc_size >= self->sp_length);
	alloc_size -= self->sp_length;
	if unlikely(alloc_size < bufsize) {
		size_t min_alloc = self->sp_length + bufsize;
		alloc_size       = (min_alloc + 63) & ~63;
realloc_again:
		string = (struct TPPString *)Dee_TryRealloc(string, offsetof(struct TPPString, s_text) +
		                                                    (alloc_size + 1) * sizeof(char));
		if unlikely(!string) {
			string = self->sp_string;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemory(offsetof(struct TPPString, s_text) +
			                      (alloc_size + 1) * sizeof(char)))
				goto realloc_again;
			return -1;
		}
		self->sp_string = string;
		string->s_size  = alloc_size;
	}
	/* Copy text into the dynamic string. */
	memcpy(string->s_text + self->sp_length,
	       buf, bufsize * sizeof(char));
	self->sp_length += bufsize;
done:
	return 0;
}

PRIVATE dssize_t DCALL
tpp_string_printer_print(void *arg, char const *__restrict buf, size_t bufsize) {
	if unlikely(tpp_string_printer_append(buf, bufsize, (struct tpp_string_printer *)arg))
		return -1;
	return (dssize_t)bufsize;
}

PRIVATE /*REF*/ struct TPPString *
(TPPCALL tpp_string_printer_pack)(struct tpp_string_printer *__restrict self) {
	/*REF*/ struct TPPString *result = (struct TPPString *)self->sp_string;
	if unlikely(!result)
		return TPPString_NewEmpty();
	/* Deallocate unused memory. */
	if likely(self->sp_length != result->s_size) {
		DREF struct TPPString *reloc;
		reloc = (DREF struct TPPString *)Dee_TryRealloc(result, offsetof(struct TPPString, s_text) +
		                                                        (self->sp_length + 1) * sizeof(char));
		if likely(reloc)
			result         = reloc;
		result->s_size = self->sp_length;
	}
	/* Make sure to terminate the c-string representation. */
	result->s_text[self->sp_length] = '\0';
	/* Do final object initialization. */
	result->s_refcnt = 1;
#ifndef NDEBUG
	memset(self, 0xcc, sizeof(*self));
#endif /* !NDEBUG */
	return result;
}



PRIVATE /*REF*/ struct TPPString *DCALL parse_brace_text(void) {
	struct tpp_string_printer printer;
	unsigned int brace_recursion   = 0;
	unsigned int paren_recursion   = 0;
	unsigned int bracket_recursion = 0;
	uint32_t old_flags;
	bool is_after_linefeed    = true;
	struct TPPFile *last_file = NULL;
	printer.sp_string         = NULL;
	printer.sp_length         = 0;
	old_flags                 = TPPLexer_Current->l_flags;
	TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_WANTCOMMENTS |
	                              TPPLEXER_FLAG_WANTSPACE |
	                              TPPLEXER_FLAG_WANTLF |
	                              TPPLEXER_FLAG_TERMINATE_STRING_LF |
	                              TPPLEXER_FLAG_DIRECTIVE_NOOWN_LF |
	                              TPPLEXER_FLAG_COMMENT_NOOWN_LF |
	                              TPPLEXER_FLAG_NO_DIRECTIVES |
	                              TPPLEXER_FLAG_NO_MACROS |
	                              TPPLEXER_FLAG_NO_BUILTIN_MACROS);
	ASSERT(tok == '{');
	for (;;) {
		if unlikely(yield() < 0)
			goto err_printer;
		switch (tok) {
		case 0: goto done;
		case '(': ++paren_recursion; goto default_case;
		case ')': --paren_recursion; goto default_case;
		case '[':
			if (paren_recursion == 0)
				++bracket_recursion;
			goto default_case;
		case ']':
			if (paren_recursion == 0)
				--bracket_recursion;
			goto default_case;
		case '{':
			if (paren_recursion == 0 && bracket_recursion == 0)
				++brace_recursion;
			goto default_case;
		case '}':
			if (paren_recursion == 0 && bracket_recursion == 0) {
				if (!brace_recursion)
					goto done;
				--brace_recursion;
			}
			goto default_case;
		case ' ':
			break;
		case '\n':
		case ';':
			/* Insert DDI directives after line-feeds and `;' tokens. */
			is_after_linefeed = true;
			break;
		default:
			if (is_after_linefeed && TPP_ISKEYWORD(tok)) {
				struct ast_loc loc;
				dssize_t error;
				loc_here(&loc);
				/* Insert an automatic DDI directive, describing
				 * the location of this instruction token. */
				if (loc.l_file == last_file) {
					error = DeeFormat_Printf(&tpp_string_printer_print, &printer,
					                         ".ddi %d,%d;\t",
					                         loc.l_line + 1,
					                         loc.l_col + 1);
				} else {
					last_file = loc.l_file;
					error = DeeFormat_Printf(&tpp_string_printer_print, &printer,
					                         ".ddi %$q,%d,%d;\t",
					                         loc.l_file->f_namesize,
					                         loc.l_file->f_name,
					                         loc.l_line + 1,
					                         loc.l_col + 1);
				}
				if unlikely(error < 0)
					goto err_printer;
			}
		default_case:
			is_after_linefeed = false;
			break;
		}
		if unlikely(TPP_PrintToken((printer_t)&tpp_string_printer_append, &printer))
			goto err_printer;
	}
done:
	TPPLexer_Current->l_flags &= TPPLEXER_FLAG_MERGEMASK;
	TPPLexer_Current->l_flags |= old_flags;
	/* Yield the final `}'-token. */
	if unlikely(yield() < 0)
		goto err_printer;
	return tpp_string_printer_pack(&printer);
err_printer:
	Dee_Free(printer.sp_string);
	TPPLexer_Current->l_flags &= TPPLEXER_FLAG_MERGEMASK;
	TPPLexer_Current->l_flags |= old_flags;
	return NULL;
}


#ifdef CONFIG_LANGUAGE_NO_ASM
PRIVATE int DCALL
check_empty_assembly_text(struct TPPString *__restrict text,
                          struct ast_loc *loc) {
	char const *iter, *end;
	end = (iter = text->s_text) + text->s_size;
	/* Make sure that the string contains only whitespace. */
	for (;;) {
		if (iter >= end)
			goto ok;
		if (!DeeUni_IsSpace(*iter))
			break;
		++iter;
	}
	for (;;) {
		if (iter >= end)
			goto ok;
		if (!DeeUni_IsSpace(end[-1]))
			break;
		--end;
	}
	return WARNAT(loc, W_UASM_NOT_SUPPORTED);
ok:
	return 0;
}
#endif



INTERN DREF struct ast *DCALL ast_parse_asm(void) {
	struct ast_loc loc;
	bool is_asm_goto   = false;
	uint16_t ast_flags = AST_FASSEMBLY_NORMAL;
	/*REF*/ struct TPPString *text;
	struct operand_list operands;
	DREF struct ast *result;
	uint32_t old_flags;
	bool has_paren;
	memset(&operands, 0, sizeof(struct operand_list));
	/*ASSERT(tok == KWD___asm__);*/
	if unlikely(yield() < 0)
		goto err;
	while (TPP_ISKEYWORD(tok)) {
		char const *name = token.t_kwd->k_name;
		size_t size      = token.t_kwd->k_size;
		while (size && *name == '_')
			++name, --size;
		while (size && name[size - 1] == '_')
			--size;
		if (size == COMPILER_STRLEN("volatile") &&
		    memcmp(name, "volatile", size * sizeof(char)) == 0) {
			ast_flags |= AST_FASSEMBLY_VOLATILE;
			goto yield_prefix;
		}
		if (size == 4 && memcmp(name, DeeString_STR(&str_goto), size * sizeof(char)) == 0) {
			is_asm_goto = true;
			goto yield_prefix;
		}
		break;
yield_prefix:
		if unlikely(yield() < 0)
			goto err;
	}
	old_flags = TPPLexer_Current->l_flags;
	TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
	if (tok == KWD_pack) {
		if unlikely(yield() < 0)
			goto err_flags;
		if (tok == '(')
			goto with_paren;
		has_paren = false;
	} else {
with_paren:
		if unlikely(likely(tok == '(') ? (yield() < 0) : WARN(W_EXPECTED_LPAREN_AFTER_ASM))
			goto err_flags;
		has_paren = true;
	}
	loc_here(&loc); /* Use the assembly text for DDI information. */
	if (tok == TOK_STRING ||
	    (tok == TOK_CHAR && !HAS(EXT_CHARACTER_LITERALS))) {
		text = TPPLexer_ParseString();
		if unlikely(!text)
			goto err_flags;
	} else if (tok == '{') {
		/* Auto-format token-based source code:
		 *    >> __asm__({
		 *    >>    print @"Now throwing", sp
		 *    >>    push  %0
		 *    >>    print pop, nl
		 *    >>    throw %0
		 *    >> }  :
		 *    >>    : "P" (42)
		 *    >> );
		 * Same as:
		 *    >> __asm__(
		 *    >>    ".ddi ...\nprint @\"Now throwing\", sp\n"
		 *    >>    ".ddi ...\npush  %0\n"
		 *    >>    ".ddi ...\nprint pop, nl\n"
		 *    >>    ".ddi ...\nthrow %0\n"
		 *    >>    :
		 *    >>    : "P" (42));
		 * A DDI directive is automatically inserted at the start, as
		 * well as after every linefeed or `;' character, excluding
		 * multiple consecutive line-feeds or `;', as well as some that
		 * may be trailing. - In other words: before every actual
		 * directive or instruction.
		 * Additionally, no DDI directives are inserted before instructions
		 * that start with a `.' token (i.e. pseudo instructions; aka.
		 * assembler directives). That way, the user may override DDI
		 * information for a single instruction at a time.
		 * Using this, debug information is automatically generated
		 * in associated with the locations of instructions, rather
		 * than leaving it up to the programmer to include DDI directives
		 * manually.
		 * Further special handling is done to prevent the generation
		 * of DDI directions inside of recursive () or [] pairs. However,
		 * recursive {} pairs will still produce DDi directives in order
		 * to allow for brace-like inner code object definitions.
		 * Also note that the generated DDI information will even contain
		 * column information for the exact positions of found instructions.
		 * Assembly is terminated once a `}' token matching the initial `{'
		 * is found. */
		text = parse_brace_text();
		if unlikely(!text)
			goto err_flags;
	} else {
		if (WARN(W_EXPECTED_STRING_AFTER_ASM))
			goto err_flags;
		text = TPPString_NewEmpty();
	}

#ifdef CONFIG_LANGUAGE_NO_ASM
	/* When user-assembly is disabled, only empty (or
	 * fully whitespace) strings are allowed as text. */
	if unlikely(check_empty_assembly_text(text, &loc))
		goto err_ops;
#endif /* CONFIG_LANGUAGE_NO_ASM */

	if (is_collon()) {
		if unlikely(yield() < 0)
			goto err_ops;
		/* Enable assembly formatting. */
		ast_flags |= AST_FASSEMBLY_FORMAT;
		/* Parse operands. */
		if unlikely(asm_parse_operands(&operands, OPERAND_TYPE_OUTPUT))
			goto err_ops;
		if (is_collon()) {
			if unlikely(yield() < 0)
				goto err_ops;
			if unlikely(asm_parse_operands(&operands, OPERAND_TYPE_INPUT))
				goto err_ops;
			if (is_collon()) {
				int32_t clobber;
				if unlikely(yield() < 0)
					goto err_ops;
				clobber = asm_parse_clobber();
				if unlikely(clobber < 0)
					goto err_ops;
				ast_flags |= (uint16_t)clobber;
				if (is_asm_goto && is_collon()) {
					if unlikely(yield() < 0)
						goto err_ops;
					if unlikely(asm_parse_operands(&operands, OPERAND_TYPE_LABEL))
						goto err_ops;
				}
			}
		}
	}
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	if (has_paren) {
		if unlikely(likely(tok == ')') ? (yield() < 0) : WARN(W_EXPECTED_RPAREN_AFTER_ASM))
			goto err_text;
	}
	ASSERT(operands.ol_c ==
	       operands.ol_count[OPERAND_TYPE_OUTPUT] +
	       operands.ol_count[OPERAND_TYPE_INPUT] +
	       operands.ol_count[OPERAND_TYPE_LABEL]);
#ifdef CONFIG_LANGUAGE_NO_ASM
	result = ast_assembly(ast_flags,
	                      operands.ol_count[OPERAND_TYPE_OUTPUT],
	                      operands.ol_count[OPERAND_TYPE_INPUT],
	                      operands.ol_count[OPERAND_TYPE_LABEL],
	                      operands.ol_v);
#else  /* CONFIG_LANGUAGE_NO_ASM */
	result = ast_assembly(ast_flags, text,
	                      operands.ol_count[OPERAND_TYPE_OUTPUT],
	                      operands.ol_count[OPERAND_TYPE_INPUT],
	                      operands.ol_count[OPERAND_TYPE_LABEL],
	                      operands.ol_v);
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	if unlikely(!result)
		goto err_ops;
	/* NOTE: `a_assembly' has inherited the operand vector upon success. */
	TPPString_Decref(text);
	return ast_setddi(result, &loc);
err_ops:
	operand_list_fini(&operands);
err_text:
	TPPString_Decref(text);
err_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_ASM_C */
