/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_GENPRINT_C
#define GUARD_DEEMON_COMPILER_ASM_GENPRINT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/bytes.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/error.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memset, ... */
#include <deemon/tuple.h>

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

/* Ensure that we can directly encode print modes in instructions. */
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NORMAL) == ASM_PRINT);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_SP) == ASM_PRINT_SP);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NL) == ASM_PRINT_NL);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NORMAL | PRINT_MODE_FILE) == ASM_FPRINT);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_SP | PRINT_MODE_FILE) == ASM_FPRINT_SP);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NL | PRINT_MODE_FILE) == ASM_FPRINT_NL);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NORMAL | PRINT_MODE_ALL) == ASM_PRINTALL);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_SP | PRINT_MODE_ALL) == ASM_PRINTALL_SP);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NL | PRINT_MODE_ALL) == ASM_PRINTALL_NL);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NORMAL | PRINT_MODE_FILE | PRINT_MODE_ALL) == ASM_FPRINTALL);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_SP | PRINT_MODE_FILE | PRINT_MODE_ALL) == ASM_FPRINTALL_SP);
STATIC_ASSERT(ASM_PRINT + (PRINT_MODE_NL | PRINT_MODE_FILE | PRINT_MODE_ALL) == ASM_FPRINTALL_NL);
STATIC_ASSERT(ASM_PRINTNL + (PRINT_MODE_FILE) == ASM_FPRINTNL);
STATIC_ASSERT(ASM_PRINT_C + (PRINT_MODE_NORMAL) == ASM_PRINT_C);
STATIC_ASSERT(ASM_PRINT_C + (PRINT_MODE_SP) == ASM_PRINT_C_SP);
STATIC_ASSERT(ASM_PRINT_C + (PRINT_MODE_NL) == ASM_PRINT_C_NL);
STATIC_ASSERT(ASM_PRINT_C + (PRINT_MODE_NORMAL | PRINT_MODE_FILE) == ASM_FPRINT_C);
STATIC_ASSERT(ASM_PRINT_C + (PRINT_MODE_SP | PRINT_MODE_FILE) == ASM_FPRINT_C_SP);
STATIC_ASSERT(ASM_PRINT_C + (PRINT_MODE_NL | PRINT_MODE_FILE) == ASM_FPRINT_C_NL);
STATIC_ASSERT(ASM16_PRINT_C + (PRINT_MODE_NORMAL) == ASM16_PRINT_C);
STATIC_ASSERT(ASM16_PRINT_C + (PRINT_MODE_SP) == ASM16_PRINT_C_SP);
STATIC_ASSERT(ASM16_PRINT_C + (PRINT_MODE_NL) == ASM16_PRINT_C_NL);
STATIC_ASSERT(ASM16_PRINT_C + (PRINT_MODE_NORMAL | PRINT_MODE_FILE) == ASM16_FPRINT_C);
STATIC_ASSERT(ASM16_PRINT_C + (PRINT_MODE_SP | PRINT_MODE_FILE) == ASM16_FPRINT_C_SP);
STATIC_ASSERT(ASM16_PRINT_C + (PRINT_MODE_NL | PRINT_MODE_FILE) == ASM16_FPRINT_C_NL);

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
constexpr_is_empty_string(DeeObject *__restrict self) {
	if (DeeString_Check(self) && DeeString_IsEmpty(self))
		goto yes;
	if (DeeBytes_Check(self) && DeeBytes_IsEmpty(self))
		goto yes;
	if (self == Dee_EmptyTuple)
		goto yes;
	return false;
yes:
	return true;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
ast_is_empty_string(struct ast *__restrict self) {
	while (self->a_type == AST_MULTIPLE &&
	       self->a_flag == AST_FMULTIPLE_KEEPLAST) {
		if (!self->a_multiple.m_astc)
			return false; /* This would be `none' */
		self = self->a_multiple.m_astv[self->a_multiple.m_astc - 1];
	}
	if (self->a_type == AST_CONSTEXPR)
		return constexpr_is_empty_string(self->a_constexpr);
	if (self->a_type == AST_MULTIPLE) {
		if (self->a_flag == AST_FMULTIPLE_TUPLE) {
			size_t i;
			for (i = 0; i < self->a_multiple.m_astc; ++i) {
				if (!ast_is_empty_string(self->a_multiple.m_astv[i]))
					return false;
			}
			return true;
		}
	}
	return false;
}

PRIVATE int DCALL
ast_genprint_emptystring(instruction_t mode,
                         struct ast *__restrict ddi_ast) {
	int32_t empty_cid;
	/* Just print an empty line. */
	if ((mode & ~(PRINT_MODE_FILE | PRINT_MODE_ALL)) == PRINT_MODE_NL) {
		if (asm_putddi(ddi_ast))
			goto err;
		return asm_put((instruction_t)(ASM_PRINTNL + (mode & PRINT_MODE_FILE)));
	}
	/* Nothing needs to be printed _at_ _all_. */
	if ((mode & ~(PRINT_MODE_FILE | PRINT_MODE_ALL)) == PRINT_MODE_NORMAL)
		return 0;
	/* Print whitespace. */
	empty_cid = asm_newconst(Dee_EmptyString);
	if unlikely(empty_cid < 0)
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	return asm_gprint_const((uint16_t)empty_cid);
err:
	return -1;
}

/* Given an AST_MULTIPLE:AST_FMULTIPLE_KEEPLAST, recursively unwrap
 * it and generate assembly for all unused operands before returning
 * the actually effective AST (without generating assembly for _it_) */
PRIVATE WUNUSED NONNULL((1)) struct ast *DCALL
ast_unwrap_effective(struct ast *__restrict self) {
	while (self->a_type == AST_MULTIPLE &&
	       self->a_flag == AST_FMULTIPLE_KEEPLAST &&
	       self->a_multiple.m_astc != 0 &&
	       self->a_scope == current_assembler.a_scope) {
		size_t i, count;
		struct ast **vector;
		vector = self->a_multiple.m_astv;
		count  = self->a_multiple.m_astc - 1;
		for (i = 0; i < count; ++i) {
			if (ast_genasm(vector[i], ASM_G_FNORMAL))
				goto err;
		}
		ASSERT(i == count);
		self = vector[i];
	}
	return self;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((2, 4)) int DCALL
ast_genprint_utf8_string(instruction_t mode, char const *str,
                         size_t num_bytes, struct ast *ddi_ast) {
	int32_t cid;
	DREF DeeObject *ob;
	ASSERTF(num_bytes != 0, "Only call this function for non-empty strings!");
	ob = DeeString_NewUtf8(str, num_bytes, STRING_ERROR_FSTRICT);
	if unlikely(!ob)
		goto err;
	cid = asm_newconst(ob);
	Dee_Decref(ob);
	if unlikely(cid < 0)
		goto err;
	if (asm_putddi(ddi_ast))
		goto err;
	return asm_put816(ASM_PRINT_C + mode, (uint16_t)cid);
err:
	return -1;
}

/* Generate a print expression for a format-string */
PRIVATE WUNUSED NONNULL((2, 5)) int DCALL
ast_genprint_string_format(instruction_t mode,
                           DeeObject *format_str,
                           size_t format_argc,
                           struct ast **format_argv,
                           struct ast *ddi_ast) {
	char const *str, *end, *flush_start;
	char *flush_buffer;
	size_t flush_buffer_size;
	flush_buffer      = NULL;
	flush_buffer_size = 0;

	/* Load input string. */
	str = DeeString_AsUtf8(format_str);
	if unlikely(!str)
		goto err;
	end = str + WSTR_LENGTH(str);
	flush_start = str;
	if unlikely(str >= end) {
		/* Special case for when the format string is empty.
		 * In this case, we still need to produce a space or line-feed. */
		ASSERT(format_argc == 0);
		return ast_genprint_emptystring(mode, ddi_ast);
	}

	while (str < end) {
		char ch = *str++;
		if (ch == '{') {
			char const *insert_start;
			size_t insert_size;
			instruction_t argument_mode;
			if (*str == '{')
				goto append_single_ch_to_flush_buffer;
			--str; /* Go back before the initial '{' */

			/* Flush text prior to the insert-start */
			if (flush_buffer_size || flush_start < str) {
				int error;
				instruction_t flush_mode;
				flush_mode = mode & ~(PRINT_MODE_SP | PRINT_MODE_NL);
				if (flush_buffer_size) {
					if (flush_start < str) {
						/* Must append the remainder to the flush buffer. */
						char *new_flush_buffer;
						size_t extra;
						extra            = (size_t)(str - flush_start);
						new_flush_buffer = (char *)Dee_Reallocc(flush_buffer,
						                                        flush_buffer_size + extra,
						                                        sizeof(char));
						if unlikely(!new_flush_buffer)
							goto err;
						flush_buffer = new_flush_buffer;
						memcpyc(flush_buffer + flush_buffer_size, flush_start, extra, sizeof(char));
						flush_buffer_size += extra;
					}
					error = ast_genprint_utf8_string(flush_mode, flush_buffer, flush_buffer_size, ddi_ast);
				} else {
					error = ast_genprint_utf8_string(flush_mode, flush_start, (size_t)(str - flush_start), ddi_ast);
				}
				if unlikely(error)
					goto err;
			}
			++str; /* Go after the  */

			/* Search for the end of the insert-string. */
			insert_start = str;
			while (*str != '}') {
				ASSERT(str < end);
				++str;
			}
			ASSERT(*str == '}');
			insert_size = (size_t)(str - insert_start);
			ASSERT(format_argc != 0);
			/* Skip closing '}' */
			++str;

			/* Figure out how we want to print the element. */
			argument_mode = mode;
			if (format_argc >= 2 || str < end)
				argument_mode &= ~(PRINT_MODE_SP | PRINT_MODE_NL); /* Not the last argument. */

			/* Print this argument. */
			if (insert_size == 0) {
				if unlikely(ast_genprint(argument_mode, format_argv[0], ddi_ast))
					goto err;
			} else {
				ASSERTF(insert_size == 2 && insert_start[0] == '!' && insert_start[1] == 'r',
				        "This should have been asserted by `ast_genprint_string_format_check_simple()'");
				if unlikely(ast_genprint_repr(argument_mode, format_argv[0], ddi_ast))
					goto err;
			}

			/* Consume argument */
			--format_argc;
			++format_argv;

			/* Start a new flush area. */
			flush_start       = str;
			flush_buffer_size = 0;
		} else if (ch == '}') {
			char *new_flush_buffer;
			size_t extra;
			ASSERT(*str == '}');
append_single_ch_to_flush_buffer:
			extra = (size_t)(str - flush_start);
			new_flush_buffer = (char *)Dee_Reallocc(flush_buffer,
			                                        flush_buffer_size + extra,
			                                        sizeof(char));
			if unlikely(!new_flush_buffer)
				goto err;
			flush_buffer = new_flush_buffer;
			memcpyc(flush_buffer + flush_buffer_size, flush_start, extra, sizeof(char));
			flush_buffer_size += extra;
			++str; /* Skip 1 character */
			flush_start = str;
			continue;
		}
	}
	ASSERTF(format_argc == 0, "This should have been asserted by `ast_genprint_string_format_check_simple()'");

	/* Flush any remaining text from the flush buffer and/or the input text. */
	if (flush_buffer_size || flush_start < end) {
		int error;
		if (flush_buffer_size) {
			if (flush_start < end) {
				/* Must append the remainder to the flush buffer. */
				char *new_flush_buffer;
				size_t extra;
				extra            = (size_t)(end - flush_start);
				new_flush_buffer = (char *)Dee_Reallocc(flush_buffer,
				                                        flush_buffer_size + extra,
				                                        sizeof(char));
				if unlikely(!new_flush_buffer)
					goto err;
				flush_buffer = new_flush_buffer;
				memcpyc(flush_buffer + flush_buffer_size, flush_start, extra, sizeof(char));
				flush_buffer_size += extra;
			}
			error = ast_genprint_utf8_string(mode, flush_buffer, flush_buffer_size, ddi_ast);
		} else {
			error = ast_genprint_utf8_string(mode, flush_start, (size_t)(end - flush_start), ddi_ast);
		}
		if unlikely(error)
			goto err;
	}
	Dee_Free(flush_buffer);
	return 0;
err:
	Dee_Free(flush_buffer);
	return -1;
}

/* Check if `format_str' is simple enough to be used for `ast_genprint_string_format'.
 * @return: 1 : Yes, it's simple enough
 * @return: 0 : No, it's not simple enough
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
ast_genprint_string_format_check_simple(DeeObject *format_str,
                                        size_t format_argc) {
	char const *str, *end;
	str = DeeString_AsUtf8(format_str);
	if unlikely(!str)
		goto err;
	end = str + WSTR_LENGTH(str);
	while (str < end) {
		char ch = *str++;
		if (ch == '{') {
			char const *insert_start;
			size_t insert_size;
			if (*str == '{') {
				++str;
				continue;
			}

			/* Search for the end of the insert-string. */
			insert_start = str;
			while (*str != '}' && str < end)
				++str;
			insert_size = (size_t)(str - insert_start);

			/* Make sure that the insert-string is "simple" */
			switch (insert_size) {

			case 0:
				break;

			case 2:
				if (insert_start[0] == '!' &&
				    insert_start[1] == 'r')
					break;
				goto nope;

			default: goto nope;
			}

			if (*str != '}')
				goto nope; /* Unterminated template insert request. */
			if (format_argc == 0)
				goto nope; /* Not enough arguments. */
			--format_argc; /* Consume argument */
			++str;         /* Skip closing '}' */
		} else if (ch == '}') {
			if (*str != '}')
				goto nope; /* Unescaped '}' */
			++str;
		}
	}
	if (format_argc != 0)
		goto nope; /* Too many arguments (handle at runtime) */
	return 1;
nope:
	return 0;
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
string_format(DeeStringObject *self, size_t argc, DeeObject *const *argv);


/* Generate code for the expression `print print_expression...;'
 * @param: mode: The print mode. NOTE: When `PRINT_MODE_FILE' is set,
 *               then the caller must first push the file to print to. */
INTERN WUNUSED NONNULL((2, 3)) int DCALL
ast_genprint(instruction_t mode,
             struct ast *print_expression,
             struct ast *ddi_ast) {
	print_expression = ast_unwrap_effective(print_expression);
	if unlikely(!print_expression)
		goto err;
	if (mode & PRINT_MODE_ALL) {
		if (print_expression->a_type == AST_MULTIPLE &&
		    print_expression->a_flag != AST_FMULTIPLE_KEEPLAST) {
			struct ast **iter, **end;

			/* Special optimization for printing an expanded, multi-branch.
			 * This is actually the most likely case, because something like
			 * `print "Hello World";' is actually encoded as `print pack("Hello World")...;' */
			end = (iter = print_expression->a_multiple.m_astv) +
			      print_expression->a_multiple.m_astc;
			if (iter == end) {
empty_operand:
				return ast_genprint_emptystring(mode, ddi_ast);
			}

			/* Print each expression individually. */
			for (; iter < end; ++iter) {
				instruction_t item_mode;
				item_mode = PRINT_MODE_SP | (mode & PRINT_MODE_FILE);
				if (iter == end - 1)
					item_mode = (mode & ~PRINT_MODE_ALL);
				if unlikely(ast_genprint(item_mode, *iter, ddi_ast))
					goto err;
			}
			return 0;
		}
		if (print_expression->a_type == AST_CONSTEXPR) {
			DREF DeeObject *items, **iter, **end;
			items = DeeTuple_FromSequence(print_expression->a_constexpr);
			if unlikely(!items) {
				DeeError_Handled(ERROR_HANDLED_RESTORE);
				goto fallback;
			}
			if (DeeTuple_IsEmpty(items)) {
				Dee_Decref(items);
				goto empty_operand;
			}

			/* Print each expression individually. */
			end = (iter = DeeTuple_ELEM(items)) + DeeTuple_SIZE(items);
			for (; iter < end; ++iter) {
				instruction_t item_mode;
				int32_t const_cid;
				DeeObject *elem;
				item_mode = PRINT_MODE_SP | (mode & PRINT_MODE_FILE);
				elem = *iter;
				if (iter == end - 1)
					item_mode = (mode & ~PRINT_MODE_ALL);

				/* Check if the operand is allowed to appear in constants. */
				if ((current_assembler.a_flag & ASM_FOPTIMIZE) &&
				    constexpr_is_empty_string(elem)) {
					if (ast_genprint_emptystring(item_mode, ddi_ast))
						goto err_items;
				} else if (!asm_allowconst(elem)) {
					if (asm_gpush_constexpr(elem))
						goto err_items;
					if (asm_putddi(ddi_ast))
						goto err;
					if unlikely(asm_put(ASM_PRINT + item_mode))
						goto err_items;
					asm_decsp();
				} else {
					const_cid = asm_newconst(elem);
					if unlikely(const_cid < 0)
						goto err_items;
					if (asm_putddi(ddi_ast))
						goto err;
					if (asm_put816(ASM_PRINT_C + item_mode, (uint16_t)const_cid))
						goto err_items;
				}
			}
			Dee_Decref(items);
			return 0;
err_items:
			Dee_Decref(items);
			goto err;
		}
	} else if (print_expression->a_type == AST_CONSTEXPR) {
		/* Special case: Print what essentially boils down to being an empty string. */
		if (current_assembler.a_flag & ASM_FOPTIMIZE) {
			if (constexpr_is_empty_string(print_expression->a_constexpr))
				goto empty_operand;
		}
		/* Special instructions exist for direct printing of constants. */
		if (asm_allowconst(print_expression->a_constexpr)) {
			int32_t const_cid;
			const_cid = asm_newconst(print_expression->a_constexpr);
			if unlikely(const_cid < 0)
				goto err;
			if (asm_putddi(ddi_ast))
				goto err;
			return asm_put816(ASM_PRINT_C + mode, (uint16_t)const_cid);
		}
	}

fallback:
	/* Fallback: Compile the print expression, then print it as an expanded sequence. */
	if (print_expression->a_type == AST_EXPAND && !(mode & PRINT_MODE_ALL)) {
		if (ast_genasm(print_expression->a_expand, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_put(ASM_PRINTALL + mode))
			goto err;
	} else {

		/* Perform some special optimizations for certain cases. */
		if (current_assembler.a_flag & ASM_FOPTIMIZE) {

			/* Printing a Tuple expression is the same as printing
			 * each if its elements without any separators.
			 * >> print("foo", "bar", 42); // Prints "foobar42\n"
			 * >> print "foo", "bar", 42;  // Prints "foo bar 42\n"
			 * Instead of generating a Tuple object here, we can
			 * instead directly go through the tuple elements and
			 * print them individually. */
			if (print_expression->a_type == AST_MULTIPLE &&
			    print_expression->a_flag == AST_FMULTIPLE_TUPLE) {
				size_t i, cnt;
				cnt = print_expression->a_multiple.m_astc;
				/* Trim trailing empty-string-expressions now so that we're
				 * properly optimizing assembly generated by code such as:
				 * >> print(foo, "");
				 * ASM:
				 * >> push  @foo
				 * >> print pop, nl
				 * Without this optimization, this would generate as:
				 * ASM:
				 * >> push  @foo
				 * >> print pop
				 * >> print nl  // Because of the empty string (""), this wasn't merged
				 * NOTE: Non-trailing empty strings don't need to be optimized away here! */
				while (cnt && ast_is_empty_string(print_expression->a_multiple.m_astv[cnt - 1]))
					--cnt;
				if (!cnt)
					goto empty_operand; /* Special case: `print()'; */
				for (i = 0; i < cnt; ++i) {
					instruction_t elem_mode = mode;
					if (i < cnt - 1)
						elem_mode &= ~(PRINT_MODE_SP | PRINT_MODE_NL);
					if unlikely(ast_genprint(elem_mode,
					                         print_expression->a_multiple.m_astv[i],
					                         ddi_ast))
						goto err;
				}
				return 0;
			}

			/* Special optimizations for certain constructs. */
			if ((print_expression->a_type == AST_OPERATOR) &&
			    !(print_expression->a_operator.o_exflag & AST_OPERATOR_FVARARGS)) {
				switch (print_expression->a_flag) {

				case OPERATOR_STR:
					/* `print str(x);' is the same as `print x;'. */
					if likely(print_expression->a_operator.o_op0 != NULL)
						return ast_genprint(mode, print_expression->a_operator.o_op0, ddi_ast);
					break;

				case OPERATOR_CALL: {
					DeeObject *format_str;
					size_t format_argc;
					struct ast **format_argv;
					struct ast *format_str_ast;
					struct ast *format_args_ast;
					int error;
					/* Optimize:
					 * >> print("foo = {}".format({ foo }));
					 *
					 * Into:
					 * >> print("foo = ", foo);
					 *
					 * Reminder:
					 * >> print f"foo = {bar}";
					 * Is compiled as:
					 * >> print "foo = {}".format({ bar });
					 * So really, this optimization is for template-strings. */
					if (!print_expression->a_operator.o_op1)
						break;
					format_str_ast = print_expression->a_operator.o_op0;
					if (format_str_ast->a_type == AST_OPERATOR) {
						if (format_str_ast->a_flag != OPERATOR_GETATTR)
							break;
						if (format_str_ast->a_operator.o_exflag & AST_OPERATOR_FVARARGS)
							break;
						if (!format_str_ast->a_operator.o_op1)
							break;
						if (format_str_ast->a_operator.o_op1->a_type != AST_CONSTEXPR)
							break;
						if (!DeeString_Check(format_str_ast->a_operator.o_op1->a_constexpr))
							break;
						if (!DeeString_EQUALS_ASCII(format_str_ast->a_operator.o_op1->a_constexpr, "format"))
							break;
						format_str_ast = format_str_ast->a_operator.o_op0;
						if (format_str_ast->a_type != AST_CONSTEXPR)
							break;
						format_str = format_str_ast->a_constexpr;
						if (!DeeString_Check(format_str))
							break;
					} else if (format_str_ast->a_type == AST_CONSTEXPR) {
						/* Check for a constant expression `"pattern".format' */
						if (!DeeObjMethod_Check(format_str_ast->a_constexpr))
							break;
						if (DeeObjMethod_FUNC(format_str_ast->a_constexpr) != (dobjmethod_t)&string_format)
							break;
						format_str = DeeObjMethod_SELF(format_str_ast->a_constexpr);
						ASSERT(DeeString_Check(format_str));
					} else {
						break;
					}
					format_args_ast = print_expression->a_operator.o_op1;
					if (format_args_ast->a_type != AST_MULTIPLE)
						break;
					if (format_args_ast->a_flag != AST_FMULTIPLE_TUPLE)
						break; /* Argument tuple */
					if (format_args_ast->a_multiple.m_astc != 1)
						break;
					format_args_ast = format_args_ast->a_multiple.m_astv[0];
					if (format_args_ast->a_type != AST_MULTIPLE)
						break;
					if (!AST_FMULTIPLE_ISSEQUENCE(format_args_ast->a_flag))
						break;
					format_argc = format_args_ast->a_multiple.m_astc;
					format_argv = format_args_ast->a_multiple.m_astv;

					/* Make sure that the format string is simple enough. */
					error = ast_genprint_string_format_check_simple(format_str, format_argc);
					if (error <= 0) {
						if unlikely(error < 0)
							goto err;
						break;
					}

					/* Yes: go ahead and generate optimized code */
					return ast_genprint_string_format(mode, format_str, format_argc, format_argv, ddi_ast);
				}	break;

				/* TODO: Optimize `print "foo" + bar' into `print("foo", bar)' */

				default: break;
				}
			}


		}

		/* Fallback: just generate the expected assembly */
		if (ast_genasm(print_expression, ASM_G_FPUSHRES))
			goto err;
		if (asm_putddi(ddi_ast))
			goto err;
		if (asm_put(ASM_PRINT + mode))
			goto err;
	}
	asm_decsp(); /* Consume the print expression. */
	return 0;
err:
	return -1;
}

/* Same as `ast_genprint()', but print the repr of `print_expression' */
INTERN WUNUSED NONNULL((2, 3)) int DCALL
ast_genprint_repr(instruction_t mode,
                  struct ast *print_expression,
                  struct ast *ddi_ast) {
	/* Super hacky (don't look)
	 *
	 * However, this *does* work, because `ast_genprint()' doesn't take
	 * references to AST objects, nor does it modify them in any way. */
	struct ast repr_ast;
	DBG_memset(&repr_ast, 0xcc, sizeof(repr_ast));
	repr_ast.a_type              = AST_OPERATOR;
	repr_ast.a_flag              = OPERATOR_REPR;
	repr_ast.a_operator.o_op0    = print_expression;
	repr_ast.a_operator.o_op1    = NULL;
	repr_ast.a_operator.o_op2    = NULL;
	repr_ast.a_operator.o_op3    = NULL;
	repr_ast.a_operator.o_exflag = AST_OPERATOR_FNORMAL;
	return ast_genprint(mode, &repr_ast, ddi_ast);
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENPRINT_C */
