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
#ifndef GUARD_DEX_STREXEC_TYPE_ANNOTATIONS_C
#define GUARD_DEX_STREXEC_TYPE_ANNOTATIONS_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/api.h>

#include <deemon/error.h>

#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint32_t */


/************************************************************************/
/* Handling of type annotations.                                        */
/************************************************************************/

DECL_BEGIN

#define THROW_ERROR(expr) \
	(throw_errors ? (expr) : -1)

PRIVATE WUNUSED NONNULL((1)) int DFCALL
parse_unary_head(JITLexer *__restrict self, bool throw_errors) {
	switch (self->jl_tok) {

	case '(':
		/* >> (string)      -- Parenthesis
		 * >> (string, int) -- Tuple */
		JITLexer_Yield(self);
		while (self->jl_tok != ')') {
			if unlikely(JITLexer_SkipTypeAnnotation(self, throw_errors))
				goto err;
			if (self->jl_tok != ',')
				break;
			JITLexer_Yield(self);
		}
		if (self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else {
			return THROW_ERROR(syn_type_annotation_expected_rparen(self));
		}
		break;

	case '{':
		/* >> {string...}
		 * >> {string: int} */
do_handle_lparen:
		JITLexer_Yield(self);
		if unlikely(JITLexer_SkipTypeAnnotation(self, throw_errors))
			goto err;
		if (self->jl_tok == ':') {
			JITLexer_Yield(self);
			if unlikely(JITLexer_SkipTypeAnnotation(self, throw_errors))
				goto err;
		} else if (self->jl_tok == TOK_DOTS) {
			JITLexer_Yield(self);
		} else {
			return THROW_ERROR(syn_type_annotation_expected_dots_or_colon(self));
		}
		if (self->jl_tok == '}') {
			JITLexer_Yield(self);
		} else {
			return THROW_ERROR(syn_type_annotation_expected_rbrace(self));
		}
		break;

	case JIT_KEYWORD: {
		char const *tok_begin;
		size_t tok_length;
		tok_begin  = (char *)self->jl_tokstart;
		tok_length = (size_t)((char *)self->jl_tokend - tok_begin);

		/* Special keywords:
		 * >> __asm("...")
		 * >> __asm__("...")
		 * >> __nth(42) foo
		 * >> none
		 * >> type 10+20
		 * >> type(10+20) */
		switch (tok_length) {

		case 4: {
			uint32_t name;
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('n', 'o', 'n', 'e')) {
				/* Don't consume a trailing "from" after "none" */
				JITLexer_Yield(self);
				goto done;
			}
			if (name == ENCODE_INT32('t', 'y', 'p', 'e')) {
				int error;
				JITLexer_Yield(self);
				if (self->jl_tok == '(') {
					error = JITLexer_SkipUnaryHead(self, LOOKUP_SYM_NORMAL | JITLEXER_EVAL_FDISALLOWCAST);
				} else {
					error = JITLexer_SkipUnary(self, LOOKUP_SYM_NORMAL);
				}
				if unlikely(error != 0)
					goto maybe_handle_error;
				goto done;
			}
			if (name == ENCODE_INT32('p', 'a', 'c', 'k')) {
				JITLexer_Yield(self);
				if (self->jl_tok == '(')
					goto do_handle_lparen;
				for (;;) {
					unsigned int saved_jl_tok;
					/*utf-8*/ unsigned char const *saved_jl_tokstart;
					/*utf-8*/ unsigned char const *saved_jl_tokend;
					saved_jl_tok      = self->jl_tok;
					saved_jl_tokstart = self->jl_tokstart;
					saved_jl_tokend   = self->jl_tokend;
					if (JITLexer_SkipTypeAnnotation(self, false) != 0) {
						self->jl_tok      = saved_jl_tok;
						self->jl_tokstart = saved_jl_tokstart;
						self->jl_tokend   = saved_jl_tokend;
						break;
					}
					if (self->jl_tok != ',')
						break;
					JITLexer_Yield(self);
				}
				goto done;
			}
		}	break;

		case 5: {
			uint32_t name;
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('_', '_', 'a', 's') && UNALIGNED_GET8(tok_begin + 4) == 'm')
				goto do_handle_asm_in_typedecl;
			if (name == ENCODE_INT32('_', '_', 'n', 't') && UNALIGNED_GET8(tok_begin + 4) == 'h') {
				bool hasparen;
				JITLexer_Yield(self);
				if (self->jl_tok == '(') {
					hasparen = true;
					JITLexer_Yield(self);
				} else if (JITLexer_ISKWD(self, "pack")) {
					JITLexer_Yield(self);
					hasparen = self->jl_tok == '(';
					if (hasparen)
						JITLexer_Yield(self);
				} else {
					return THROW_ERROR(syn_nth_expected_lparen(self));
				}
				if (JITLexer_SkipExpression(self, LOOKUP_SYM_NORMAL))
					goto maybe_handle_error;
				if (hasparen) {
					if (self->jl_tok == ')') {
						JITLexer_Yield(self);
					} else {
						return THROW_ERROR(syn_nth_expected_rparen(self));
					}
				}
				goto done;
			}
		}	break;

		case 7:
			if (UNALIGNED_GET32(tok_begin + 0) == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('m', '_') &&
			    UNALIGNED_GET8(tok_begin + 6) == '_') {
				bool hasparen;
do_handle_asm_in_typedecl:
				JITLexer_Yield(self);
				if (self->jl_tok == '(') {
					JITLexer_Yield(self);
					hasparen = true;
				} else if (JITLexer_ISKWD(self, "pack")) {
					JITLexer_Yield(self);
					hasparen = self->jl_tok == '(';
					if (hasparen)
						JITLexer_Yield(self);
				} else {
					return THROW_ERROR(syn_asm_expected_lparen_after_asm(self));
				}
				for (;;) {
					if (self->jl_tok != JIT_STRING) {
						if (!hasparen)
							break; /* Stop once there are no more strings */
						return THROW_ERROR(syn_type_annotation_expected_string_after_asm(self));
					}
					JITLexer_Yield(self);
					if (self->jl_tok == ')' && hasparen) {
						JITLexer_Yield(self); /* Skip trailing ')' */
						break;
					}
				}
				goto done;
			}
			break;


		default:
			break;
		}

		/* Normal keyword processing (symbol lookup, though
		 * we don't actually verify the lookup itself) */
		JITLexer_Yield(self);
		if (JITLexer_ISKWD(self, "from")) {
			JITLexer_Yield(self);
			if unlikely(JITLexer_SkipModule(self)) {
maybe_handle_error:
				if (!throw_errors)
					DeeError_Handled(ERROR_HANDLED_RESTORE);
				goto err;
			}
		}
	}	break;

	default:
		return THROW_ERROR(syn_type_annotation_unexpected_token(self));
	}
done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DFCALL
parse_unary(JITLexer *__restrict self, bool throw_errors) {
	int result;
	result = parse_unary_head(self, throw_errors);
	if (result == 0) {
		while (self->jl_tok == '.') {
			JITLexer_Yield(self);
			if (self->jl_tok != JIT_KEYWORD)
				return THROW_ERROR(syn_attr_expected_keyword(self));
			JITLexer_Yield(self);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DFCALL
parse_alt(JITLexer *__restrict self, bool throw_errors) {
	int result;
again:
	result = parse_unary(self, throw_errors);
	if (result == 0 && self->jl_tok == '|') {
		JITLexer_Yield(self);
		goto again;
	}
	return result;
}


/* Skip type annotations:
 * >> local x: int from deemon | string from deemon = 42;
 *             ^                                    ^
 * @return: 0 : Success
 * @return: -1: Compiler error (only thrown when `throw_errors != false') */
INTERN WUNUSED NONNULL((1)) int DFCALL
JITLexer_SkipTypeAnnotation(JITLexer *__restrict self, bool throw_errors) {
	int result;
again:
	result = parse_alt(self, throw_errors);
	if (result == 0 && JITLexer_ISKWD(self, "with")) {
		JITLexer_Yield(self);
		goto again;
	}
	return result;
}

DECL_END

#endif /* !GUARD_DEX_STREXEC_TYPE_ANNOTATIONS_C */
