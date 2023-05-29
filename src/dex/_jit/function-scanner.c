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
#ifndef GUARD_DEX_JIT_FUNCTION_SCANNER_C
#define GUARD_DEX_JIT_FUNCTION_SCANNER_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/stringutils.h>

#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

DECL_BEGIN

INTDEF NONNULL((1, 2)) void DCALL
JITLexer_ReferenceKeyword(JITLexer *__restrict self,
                          char const *__restrict name,
                          size_t size) {
	JITObjectTable *iter;
	dhash_t hash;
	if (self->jl_scandata.jl_flags & JIT_SCANDATA_FERROR)
		return;
	hash = Dee_HashUtf8(name, size);
	if (JITObjectTable_Lookup(&self->jl_scandata.jl_function->jf_args, name, size, hash) ||
	    JITObjectTable_Lookup(&self->jl_scandata.jl_function->jf_refs, name, size, hash))
		return; /* Keyword is an argument, or has already been referenced. */
	iter = self->jl_scandata.jl_parobtab;
	for (; iter; iter = iter->ot_prev.otp_tab) {
		struct jit_object_entry *ent, *destent;
		ent = JITObjectTable_Lookup(iter, name, size, hash);
		if (!ent || !ent->oe_value)
			continue;
		switch (ent->oe_type) {

		case JIT_OBJECT_ENTRY_TYPE_ATTR:
			if (iter == self->jl_scandata.jl_parobtab) {
				/* Make sure that we're referencing our caller's this-argument! */
				if likely(size != COMPILER_STRLEN(JIT_RTSYM_THIS) ||
				          bcmpc(name, JIT_RTSYM_THIS, COMPILER_STRLEN(JIT_RTSYM_THIS), sizeof(char)) != 0)
					JITLexer_ReferenceKeyword(self, JIT_RTSYM_THIS, COMPILER_STRLEN(JIT_RTSYM_THIS));
				if unlikely(self->jl_scandata.jl_flags & JIT_SCANDATA_FERROR)
					return;
do_alias_thisattr:
				destent = JITObjectTable_Create(&self->jl_scandata.jl_function->jf_refs,
				                                name, size, hash);
				if unlikely(!destent)
					goto err;
				destent->oe_attr.a_attr  = ent->oe_attr.a_attr;
				destent->oe_attr.a_class = ent->oe_attr.a_class;
				Dee_Incref(destent->oe_attr.a_class);
			} else {
				/* Reference the member of a recursive class:
				 * >> class Foo {
				 * >>     public member fooMember = 42;
				 * >>     public function getLambda() {
				 * >>         return () -> {
				 * >>             return fooMember; // << We're here right now!
				 * >>         };
				 * >>     }
				 * >> }
				 * In this case, we must lookup the this-argument from the scope
				 * one before `iter' and re-use its value for our own purposes! */
				JITObjectTable *before_iter;
				struct jit_object_entry *effective_this;
				before_iter = self->jl_scandata.jl_parobtab;
				while (before_iter->ot_prev.otp_tab != iter)
					before_iter = before_iter->ot_prev.otp_tab;
				effective_this = JITObjectTable_Lookup(before_iter, JIT_RTSYM_THIS, COMPILER_STRLEN(JIT_RTSYM_THIS),
				                                       Dee_HashUtf8(JIT_RTSYM_THIS, COMPILER_STRLEN(JIT_RTSYM_THIS)));
				if unlikely(!effective_this)
					goto do_alias_thisattr; /* Shouldn't happen... */
				if unlikely(effective_this->oe_type != JIT_OBJECT_ENTRY_TYPE_LOCAL)
					goto do_alias_thisattr; /* Shouldn't happen... */
				if unlikely(!effective_this->oe_value)
					goto do_alias_thisattr; /* Shouldn't happen... */
				if (DeeObject_AssertType(effective_this->oe_value, ent->oe_attr.a_class))
					goto err; /* Shouldn't happen... */
				destent = JITObjectTable_Create(&self->jl_scandata.jl_function->jf_refs,
				                                name, size, hash);
				if unlikely(!destent)
					goto err;

				/* Load the symbols as a fixed attribute into the object table. */
				destent->oe_attr_fixed.af_obj  = effective_this->oe_value;
				destent->oe_attr_fixed.af_attr = ent->oe_attr.a_attr;
				destent->oe_attr_fixed.af_desc = DeeInstance_DESC(ent->oe_attr.a_class->tp_class,
				                                                  effective_this->oe_value);
				Dee_Incref(destent->oe_attr_fixed.af_obj);
			}
			break;

		case JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED:
			destent = JITObjectTable_Create(&self->jl_scandata.jl_function->jf_refs,
			                                name, size, hash);
			if unlikely(!destent)
				goto err;
			destent->oe_type = JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED;
			memcpy(&destent->oe_attr_fixed, &ent->oe_attr_fixed,
			       sizeof(destent->oe_attr_fixed));
			Dee_Incref(destent->oe_attr_fixed.af_obj);
			break;

		default:
			destent = JITObjectTable_Create(&self->jl_scandata.jl_function->jf_refs,
			                                name, size, hash);
			if unlikely(!destent)
				goto err;
			destent->oe_type  = JIT_OBJECT_ENTRY_TYPE_LOCAL;
			destent->oe_value = ent->oe_value;
			Dee_XIncref(ent->oe_value);
			break;
		}
		break;
	}
	return;
err:
	self->jl_scandata.jl_flags |= JIT_SCANDATA_FERROR;
	self->jl_tokstart = self->jl_end;
	self->jl_tokend   = self->jl_end;
	self->jl_tok      = 0;
}


INTERN void FCALL
JITLexer_ScanForHead(JITLexer *__restrict self) {
	bool has_paren;
	if (JITLexer_ISKWD(self, "pack"))
		JITLexer_Yield(self);
	has_paren = self->jl_tok == '(';
	if (has_paren)
		JITLexer_Yield(self);
	JITLexer_ScanExpression(self, true);
	if (self->jl_tok == ':') {
		JITLexer_Yield(self);
		JITLexer_ScanExpression(self, true);
	} else {
		while (self->jl_tok == ';') {
			JITLexer_Yield(self);
			JITLexer_ScanExpression(self, true);
		}
	}
	if (has_paren && self->jl_tok == ')')
		JITLexer_Yield(self);
}

INTERN void FCALL
JITLexer_ScanCatchMask(JITLexer *__restrict self) {
	bool hasparen;
	if (JITLexer_ISKWD(self, "pack"))
		JITLexer_Yield(self);
	hasparen = self->jl_tok == '(';
	if (hasparen)
		JITLexer_Yield(self);
	if (self->jl_tok == TOK_DOTS) {
		JITLexer_Yield(self);
		if (self->jl_tok == JIT_KEYWORD)
			JITLexer_Yield(self);
	} else {
		JITLexer_ScanExpression(self, true);
	}
	if (hasparen && self->jl_tok == ')')
		JITLexer_Yield(self);
}

INTERN void FCALL
JITLexer_QuickSkipModuleName(JITLexer *__restrict self) {
	if (self->jl_tok == '.' || self->jl_tok == JIT_KEYWORD) {
		unsigned char *start, *end;
		start = self->jl_tokstart;
		end   = self->jl_tokend;
		while (end < self->jl_end) {
			unsigned char ch = *end++;
			if (ch == '.')
				continue;
			if (ch > 0x7f) {
				uint32_t ch32;
				unsigned char *temp;
				--end;
				temp = end;
				ch32 = unicode_readutf8_n(&end, self->jl_end);
				if (!DeeUni_IsSymCont(ch32)) {
					end = temp;
					break;
				}
			} else {
				if (!DeeUni_IsSymCont(ch)) {
					--end;
					break;
				}
			}
		}
		/* Check if the keyword following the simple module
		 * name is something that would belong to our name. */
		JITLexer_YieldAt(self, end);
		if (self->jl_tok == '.' ||
		    self->jl_tok == JIT_KEYWORD ||
		    self->jl_tok == JIT_STRING ||
		    self->jl_tok == JIT_RAWSTRING) {
			JITLexer_YieldAt(self, start);
			goto do_print;
		}
		return;
	}
do_print:
	for (;;) {
		if (self->jl_tok == '.' || self->jl_tok == TOK_DOTS) {
			JITLexer_Yield(self);
			if (self->jl_tok != JIT_KEYWORD &&
			    self->jl_tok != JIT_STRING &&
			    self->jl_tok != JIT_RAWSTRING &&
			    self->jl_tok != '.' &&
			    self->jl_tok != TOK_DOTS)
				break; /* Special case: `.' is a valid name for the current module. */
		} else if (self->jl_tok == JIT_KEYWORD) {
			JITLexer_Yield(self);
			if (self->jl_tok != '.' && self->jl_tok != TOK_DOTS)
				break;
		} else if (self->jl_tok == JIT_STRING ||
		           self->jl_tok == JIT_RAWSTRING) {
			JITLexer_Yield(self);
			if (self->jl_tok != '.' &&
			    self->jl_tok != TOK_DOTS &&
			    self->jl_tok != JIT_STRING &&
			    self->jl_tok != JIT_RAWSTRING)
				break;
		} else {
			break;
		}
	}
}


INTERN void FCALL
JITLexer_QuickSkipOperatorName(JITLexer *__restrict self) {
	switch (self->jl_tok) {

	case TOK_INT:
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '&':
	case '|':
	case '^':
	case '~':
	case TOK_SHL:
	case TOK_SHR:
	case TOK_POW:
	case TOK_ADD_EQUAL:
	case TOK_SUB_EQUAL:
	case TOK_MUL_EQUAL:
	case TOK_DIV_EQUAL:
	case TOK_MOD_EQUAL:
	case TOK_SHL_EQUAL:
	case TOK_SHR_EQUAL:
	case TOK_AND_EQUAL:
	case TOK_OR_EQUAL:
	case TOK_XOR_EQUAL:
	case TOK_POW_EQUAL:
	case TOK_INC:
	case TOK_DEC:
	case TOK_EQUAL:
	case TOK_NOT_EQUAL:
	case TOK_LOWER:
	case TOK_LOWER_EQUAL:
	case TOK_GREATER:
	case TOK_GREATER_EQUAL:
	case JIT_RAWSTRING:
	case JIT_STRING:
	case '#':
	case JIT_KEYWORD:
done_y1:
		JITLexer_Yield(self);
		break;

	case '=':
	case TOK_COLON_EQUAL:
		JITLexer_Yield(self);
		if (JITLexer_ISKWD(self, "move"))
			goto done_y1;
		break;

	case '(':
		JITLexer_Yield(self);
		if (self->jl_tok == ')')
			goto done_y1;
		JITLexer_QuickSkipOperatorName(self);
		if (self->jl_tok == ')')
			goto done_y1;
		break;

	case '[':
		JITLexer_Yield(self);
		if (self->jl_tok == ':')
			JITLexer_Yield(self);
		if (self->jl_tok == ']')
			JITLexer_Yield(self);
		if (self->jl_tok == '=')
			goto done_y1;
		break;

	case '.':
		JITLexer_Yield(self);
		if (self->jl_tok == '=')
			goto done_y1;
		break;

	default: break;
	}
}

/* Starting _AFTER_ the leading '(', skip ahead until _AFTER_ the closing ')' */
PRIVATE void FCALL
JITLexer_SkipParamList(JITLexer *__restrict self) {
	unsigned int recursion;
	recursion = 1;
	for (;;) {
		if (!self->jl_tok)
			break;
		if (self->jl_tok == '(') {
			++recursion;
		} else if (self->jl_tok == ')') {
			--recursion;
			if (!recursion) {
				JITLexer_Yield(self);
				break;
			}
		}
		JITLexer_Yield(self);
	}
}


INTERN void FCALL
JITLexer_ScanExpression(JITLexer *__restrict self, bool allow_casts) {
again:
	switch (self->jl_tok) {

	case JIT_STRING:
	case JIT_RAWSTRING:
		do {
			JITLexer_Yield(self);
		} while (self->jl_tok == JIT_STRING ||
		         self->jl_tok == JIT_RAWSTRING);
		break;

	case TOK_INT:
	case TOK_FLOAT:
do_yield_suffix:
		JITLexer_Yield(self);
		break;

	case '+':
	case '-':
	case '~':
	case '!':
	case '#':
	case TOK_INC:
	case TOK_DEC:
		goto do_yield_again_docast;

	case '(': {
		bool allow_cast;
		JITLexer_Yield(self);
		allow_cast = self->jl_tok != '(' && allow_casts;
		if (self->jl_tok == '{') {
			JITLexer_ScanStatement(self);
			if (self->jl_tok == ',') {
				/* FIXME: This fails to handle the trailing ')'
				 * -> write a test that triggers this, and then fix this! */
				goto do_yield_again_docast;
			}
			if (self->jl_tok == ')')
				JITLexer_Yield(self);
			break;
		}

		/* Check for java-style lambdas */
		{
			unsigned char *saved;
			saved = self->jl_tokstart;
			JITLexer_SkipParamList(self);
			if (self->jl_tok == TOK_ARROW) {
				unsigned int old_flags;
do_handle_java_lambda:
				JITLexer_Yield(self); /* Skip over `->' */
				old_flags = self->jl_scandata.jl_flags;
				self->jl_scandata.jl_flags |= JIT_SCANDATA_FINCHILD;
				if (self->jl_tok == '{') {
					JITLexer_ScanStatement(self); /* Always handling a statement here is good enough! */
				} else {
					JITLexer_ScanExpression(self, true);
				}
				self->jl_scandata.jl_flags &= ~JIT_SCANDATA_FINCHILD;
				self->jl_scandata.jl_flags |= old_flags & JIT_SCANDATA_FINCHILD;
				break;
			}
			if (self->jl_tok == ':') {
				JITLexer_Yield(self);
				if (JITLexer_SkipTypeAnnotation(self, false) == 0 &&
				    self->jl_tok == TOK_ARROW)
					goto do_handle_java_lambda;
			}
			JITLexer_YieldAt(self, saved);
		}

		/* Check for special case: empty tuple */
		if (self->jl_tok == ')')
			goto do_yield_suffix;

		/* Normal paren-expression. */
		for (;;) {
			unsigned char *start = self->jl_tokstart;
			JITLexer_ScanExpression(self, true);
			if (!self->jl_tok)
				break;
			if (self->jl_tok == ')') {
				JITLexer_Yield(self);
				break;
			}
			if unlikely(start == self->jl_tokstart)
				JITLexer_Yield(self);
		}
		if (allow_cast)
			goto again;
	}	break;

	case '[':
		JITLexer_Yield(self);
		if (self->jl_tok == ']') {
			/* Lambda function. */
			JITLexer_Yield(self);
			if (self->jl_tok == '(') {
				unsigned int old_flags;
				/* Skip the parameter list. */
do_parse_function_arglist:
				JITLexer_Yield(self);
				JITLexer_SkipParamList(self);
do_parse_function:
				old_flags = self->jl_scandata.jl_flags;
				self->jl_scandata.jl_flags |= JIT_SCANDATA_FINCHILD;
				if (self->jl_tok == TOK_ARROW) {
					JITLexer_Yield(self);
					JITLexer_ScanExpression(self, true);
				} else if (self->jl_tok == '{') {
					JITLexer_ScanStatement(self);
				}
				self->jl_scandata.jl_flags &= ~JIT_SCANDATA_FINCHILD;
				self->jl_scandata.jl_flags |= old_flags & JIT_SCANDATA_FINCHILD;
				break;
			}
			if (self->jl_tok == TOK_ARROW || self->jl_tok == '{') {
				unsigned int old_flags;
				old_flags = self->jl_scandata.jl_flags;
				self->jl_scandata.jl_flags |= JIT_SCANDATA_FINCHILD;
				if (self->jl_tok == TOK_ARROW) {
					JITLexer_Yield(self);
					JITLexer_ScanExpression(self, true);
				} else {
					JITLexer_ScanStatement(self);
				}
				self->jl_scandata.jl_flags &= ~JIT_SCANDATA_FINCHILD;
				self->jl_scandata.jl_flags |= old_flags & JIT_SCANDATA_FINCHILD;
				break;
			}
			break; /* Empty list */
		}
		for (;;) {
			unsigned char *start;
			start = self->jl_tokstart;
			JITLexer_ScanExpression(self, true);
			if (self->jl_tok == ']') {
				JITLexer_Yield(self);
				break;
			}
			if (!self->jl_tok)
				break;
			if (self->jl_tok == ':') {
				/* Range expressions. */
				JITLexer_Yield(self);
				continue;
			}
			if unlikely(start == self->jl_tokstart)
				JITLexer_Yield(self);
		}
		break;

	case '{':
		/* Sequence expression. */
		JITLexer_Yield(self);
		for (;;) {
			unsigned char *start;
			start = self->jl_tokstart;
			JITLexer_ScanExpression(self, true);
			if (self->jl_tok == '}') {
				JITLexer_Yield(self);
				break;
			}
			if (!self->jl_tok)
				break;
			if unlikely(start == self->jl_tokstart)
				JITLexer_Yield(self);
		}
		break;

	case JIT_KEYWORD: {
		char const *tok_begin;
		size_t tok_length;
		uint32_t name;
		tok_begin  = (char *)self->jl_tokstart;
		tok_length = (size_t)((char *)self->jl_tokend - tok_begin);
		switch (tok_length) {

		case 2:
			if (tok_begin[0] == 'i' && tok_begin[1] == 'f') {
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, false);
				JITLexer_ScanExpression(self, true);
				if (JITLexer_ISKWD(self, "else")) {
					JITLexer_Yield(self);
					goto do_again_docast;
				}
				goto do_suffix;
			}
			if (tok_begin[0] == 'd' && tok_begin[1] == 'o') {
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, true);
				if (JITLexer_ISKWD(self, "while")) {
do_yield_again_nocast:
					allow_casts = false;
do_yield_again:
					JITLexer_Yield(self);
					goto again;
				}
				goto do_suffix;
			}
			break;

		case 3:
			if (tok_begin[0] == 's' && tok_begin[1] == 't' &&
			    tok_begin[2] == 'r')
				goto do_yield_again_nocast;
			if (tok_begin[0] == 't' && tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y')
				goto do_yield_again_docast;
			if (tok_begin[0] == 'f' && tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
do_handle_for:
				JITLexer_Yield(self);
				JITLexer_ScanForHead(self);
				goto do_again_docast;
			}
			break;

		case 4:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('n', 'o', 'n', 'e'))
				goto do_yield_suffix;
			if (name == ENCODE_INT32('t', 'r', 'u', 'e'))
				goto do_yield_suffix;
			if (name == ENCODE_INT32('r', 'e', 'p', 'r'))
				goto do_yield_again_nocast;
			if (name == ENCODE_INT32('c', 'o', 'p', 'y'))
				goto do_yield_again_nocast;
			if (name == ENCODE_INT32('t', 'y', 'p', 'e'))
				goto do_yield_again_nocast;
			if (name == ENCODE_INT32('p', 'a', 'c', 'k')) {
				int has_paren = 0;
				JITLexer_Yield(self);
				if (self->jl_tok == '(') {
					JITLexer_Yield(self);
					has_paren = self->jl_tok == '(' ? 2 : 1;
				}
#if 0
				if (self->jl_tok == '{') {
					/* Statements in expressions. */
					JITLexer_ScanStatement(self);
				} else
#endif
				if (JITLexer_MaybeExpressionBegin(self)) {
					/* Parse the packed expression. */
					JITLexer_ScanExpression(self, true);
				}
				if (has_paren && self->jl_tok == ')')
					JITLexer_Yield(self);
				goto do_suffix;
			}
			if (name == ENCODE_INT32('w', 'i', 't', 'h'))
				goto do_yield_again_docast;
			if (name == ENCODE_INT32('t', 'h', 'i', 's'))
				goto do_reference_this_and_class;
			break;

		case 5:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('c', 'a', 't', 'c') ||
			    UNALIGNED_GET8(tok_begin + 4) == 'h') {
				JITLexer_Yield(self);
				JITLexer_ScanCatchMask(self);
				goto again;
			}
			if (name == ENCODE_INT32('f', 'a', 'l', 's') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'e')
				goto do_yield_suffix;
			if (name == ENCODE_INT32('l', 'o', 'c', 'a') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'l')
				goto do_yield_again;
			if (name == ENCODE_INT32('b', 'o', 'u', 'n') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'd')
				goto do_yield_again_nocast;
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'e') {
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, false);
				goto do_again_docast;
			}
			if (name == ENCODE_INT32('s', 'u', 'p', 'e') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'r') {
do_reference_this_and_class:
				JITLexer_Yield(self);
				JITLexer_ReferenceKeyword(self, JIT_RTSYM_THIS, COMPILER_STRLEN(JIT_RTSYM_THIS));
				JITLexer_ReferenceKeyword(self, JIT_RTSYM_CLASS, COMPILER_STRLEN(JIT_RTSYM_CLASS));
				goto do_suffix;
			}
			/* TODO: Scan class (but must make sure not to reference symbols found
			 *       in member bodies that are the names of other members!) */
			break;

		case 6:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't'))
				goto do_yield_suffix;
			if (name == ENCODE_INT32('a', 's', 's', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't'))
				goto do_yield_again_nocast;
			if (name == ENCODE_INT32('g', 'l', 'o', 'b') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'l'))
				goto do_yield_again;
			if (name == ENCODE_INT32('s', 't', 'a', 't') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('i', 'c'))
				goto do_yield_again;
			break;

		case 7:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('f', 'i', 'n', 'a') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('l', 'l') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'y')
				goto do_yield_again_docast;
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'c') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'h')
				goto do_handle_for;
			if (name == ENCODE_INT32('_', '_', 's', 't') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'c') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'k')
				goto do_yield_again;
			break;

		case 8:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('d', 'e', 'e', 'p') &&
			    UNALIGNED_GET32(tok_begin + 4) == ENCODE_INT32('c', 'o', 'p', 'y'))
				goto do_yield_again_nocast;
			if (name == ENCODE_INT32('f', 'u', 'n', 'c') &&
			    UNALIGNED_GET32(tok_begin + 4) == ENCODE_INT32('t', 'i', 'o', 'n')) {
				/* Local function definition. */
				JITLexer_Yield(self);
				if (self->jl_tok == JIT_KEYWORD)
					JITLexer_Yield(self);
				if (self->jl_tok == '(')
					goto do_parse_function_arglist;
				goto do_parse_function;
			}
			if (name == ENCODE_INT32('o', 'p', 'e', 'r') &&
			    UNALIGNED_GET32(tok_begin + 4) == ENCODE_INT32('a', 't', 'o', 'r')) {
				/* Local function definition. */
				JITLexer_Yield(self);
				JITLexer_QuickSkipOperatorName(self);
				goto do_suffix;
			}
			break;

		default: break;
		}
		{
			char *sname;
			size_t ssize;
			sname = JITLexer_TokPtr(self);
			ssize = JITLexer_TokLen(self);
			JITLexer_Yield(self);
			if (JITLexer_ISKWD(self, "from")) {
				/* `foo from bar' */
				JITLexer_Yield(self);
				JITLexer_QuickSkipModuleName(self);
			} else if (self->jl_tok == TOK_ARROW) {
				/* `x -> x.lower()' */
				goto do_handle_java_lambda;
			} else {
				JITLexer_ReferenceKeyword(self, sname, ssize);
			}
		}
	}	break;

	default: return;
	}

do_suffix:
	/* Suffix */
	switch (self->jl_tok) {

	case '.':
		/* Attribute suffix */
		JITLexer_Yield(self);
		if (self->jl_tok != JIT_KEYWORD)
			break;
		if (JITLexer_ISTOK(self, "operator")) {
			JITLexer_Yield(self);
			JITLexer_QuickSkipOperatorName(self);
			goto do_suffix;
		}
		JITLexer_Yield(self);
		goto do_suffix;

	case '(':
		/* Call suffix */
		JITLexer_Yield(self);
do_scan_call_args:
		JITLexer_ScanExpression(self, true);
		if (self->jl_tok == ':' ||
		    self->jl_tok == TOK_POW) {
			JITLexer_Yield(self);
			goto do_scan_call_args;
		}
		if (self->jl_tok == ')')
			JITLexer_Yield(self);
		goto do_suffix;

	case '{':
		/* Sequence call suffix */
		JITLexer_Yield(self);
		JITLexer_ScanExpression(self, true);
		if (self->jl_tok == '}')
			JITLexer_Yield(self);
		goto do_suffix;

	case '[':
		/* Getitem suffix */
		JITLexer_Yield(self);
		JITLexer_ScanExpression(self, true);
		if (self->jl_tok == ':') {
			JITLexer_Yield(self);
			JITLexer_ScanExpression(self, true);
		}
		if (self->jl_tok == ']')
			JITLexer_Yield(self);
		goto do_suffix;

	case '?':
		/* Ifelse suffix */
		JITLexer_Yield(self);
		JITLexer_ScanExpression(self, true);
		if (self->jl_tok == ':') {
			JITLexer_Yield(self);
			JITLexer_ScanExpression(self, true);
		}
		goto do_suffix;

	case TOK_DOTS:
	case TOK_INC:
	case TOK_DEC:
	case '!':
		JITLexer_Yield(self);
		goto do_suffix;

	case '+':
	case '&':
	case '=':
	case ',':
	case '/':
	case '<':
	case '%':
	case '*':
	case '|':
	case '>':
	case '-':
	case '^':
	case TOK_SHL:
	case TOK_SHR:
	case TOK_EQUAL:
	case TOK_NOT_EQUAL:
	case TOK_GREATER_EQUAL:
	case TOK_LOWER_EQUAL:
	case TOK_ADD_EQUAL:
	case TOK_SUB_EQUAL:
	case TOK_MUL_EQUAL:
	case TOK_DIV_EQUAL:
	case TOK_MOD_EQUAL:
	case TOK_SHL_EQUAL:
	case TOK_SHR_EQUAL:
	case TOK_AND_EQUAL:
	case TOK_OR_EQUAL:
	case TOK_XOR_EQUAL:
	case TOK_POW_EQUAL:
	case TOK_LAND:
	case TOK_LOR:
	case TOK_POW:
	case TOK_COLON_EQUAL:
	case TOK_EQUAL3:
	case TOK_NOT_EQUAL3:
do_yield_again_docast:
		JITLexer_Yield(self);
do_again_docast:
		allow_casts = true;
		goto again;

	case JIT_KEYWORD:
		if (JITLexer_ISTOK(self, "is") ||
		    JITLexer_ISTOK(self, "as"))
			goto do_yield_again_docast;
		break;

	default: break;
	}
}

INTERN void FCALL
JITLexer_ScanStatement(JITLexer *__restrict self) {
again:
	switch (self->jl_tok) {

	case '{':
		JITLexer_Yield(self);
		for (;;) {
			unsigned char *start;
			start = self->jl_tokstart;
			JITLexer_ScanStatement(self);
			if (!self->jl_tok)
				break;
			if (self->jl_tok == '}') {
				JITLexer_Yield(self);
				break;
			}
			if unlikely(start == self->jl_tokstart)
				JITLexer_Yield(self);
		}
		break;

	case '}':
		break;

	case JIT_KEYWORD: {
		char const *tok_begin;
		size_t tok_length;
		uint32_t name;
		tok_begin  = (char *)self->jl_tokstart;
		tok_length = (size_t)((char *)self->jl_tokend - tok_begin);
		switch (tok_length) {

		case 2:
			if (tok_begin[0] == 'i' &&
			    tok_begin[1] == 'f') {
do_yield_expression_again:
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, false);
				goto again;
			}
			if (tok_begin[0] == 'd' &&
			    tok_begin[1] == 'o') {
do_yield_again:
				JITLexer_Yield(self);
				goto again;
			}
			break;

		case 3:
			if (tok_begin[0] == 't' &&
			    tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y')
				goto do_yield_again;
			if (tok_begin[0] == 'f' &&
			    tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
do_scan_for_statement:
				JITLexer_Yield(self);
				JITLexer_ScanForHead(self);
				goto again;
			}
			if (tok_begin[0] == 'd' &&
			    tok_begin[1] == 'e' &&
			    tok_begin[2] == 'l') {
do_yield_expression_semi:
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, true);
				goto done_skip_semi;
			}
			break;

		case 4:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('w', 'i', 't', 'h') ||
			    name == ENCODE_INT32('e', 'l', 'i', 'f')) {
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, false);
				goto again;
			}
			if (name == ENCODE_INT32('e', 'l', 's', 'e'))
				goto do_yield_again;
			if (name == ENCODE_INT32('f', 'r', 'o', 'm')) {
do_skip_until_semi:
				do {
					JITLexer_Yield(self);
				} while (self->jl_tok && self->jl_tok != ';');
				goto done_skip_semi;
			}
			break;

		case 5:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('t', 'h', 'r', 'o') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'w')
				goto do_yield_expression_semi;
			if (name == ENCODE_INT32('c', 'a', 't', 'c') ||
			    UNALIGNED_GET8(tok_begin + 4) == 'h') {
				JITLexer_Yield(self);
				JITLexer_ScanCatchMask(self);
				goto again;
			}
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'd') {
				/* Set the YIELDING flag if we're still within the core function! */
				if (!(self->jl_scandata.jl_flags & JIT_SCANDATA_FINCHILD))
					self->jl_scandata.jl_function->jf_flags |= JIT_FUNCTION_FYIELDING;
				goto do_yield_expression_semi;
			}
			if (name == ENCODE_INT32('p', 'r', 'i', 'n') &&
			    UNALIGNED_GET8(tok_begin + 4) == 't') {
				JITLexer_Yield(self);
				JITLexer_ScanExpression(self, true);
				if (self->jl_tok == ':')
					goto do_yield_expression_semi;
				goto done_skip_semi;
			}
			if (name == ENCODE_INT32('b', 'r', 'e', 'a') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'k') {
do_yield_semi:
				JITLexer_Yield(self);
				goto done_skip_semi;
			}
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'm')
				goto do_asm;
			break;

		case 6:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('r', 'e', 't', 'u') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 'n'))
				goto do_yield_expression_semi;
			if (name == ENCODE_INT32('a', 's', 's', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't'))
				goto do_yield_expression_semi;
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't')) {
				JITLexer_Yield(self);
				if (self->jl_tok == '(' || JITLexer_ISKWD(self, "pack"))
					goto do_yield_expression_semi;
				goto do_skip_until_semi;
			}
			if (name == ENCODE_INT32('s', 'w', 'i', 't') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('c', 'h'))
				goto do_yield_expression_again;
			break;

		case 7:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'c') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'h')
				goto do_scan_for_statement;
			if (name == ENCODE_INT32('f', 'i', 'n', 'a') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('l', 'l') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'y')
				goto do_yield_again;
			if (name == ENCODE_INT32('_', '_', 'a', 's') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('m', '_') &&
			    UNALIGNED_GET8(tok_begin + 6) == '_') {
				bool has_lparen;
do_asm:
				JITLexer_Yield(self);
				if (JITLexer_ISKWD(self, "pack"))
					JITLexer_Yield(self);
				has_lparen = self->jl_tok == '(';
				if (has_lparen)
					JITLexer_Yield(self);
				if (self->jl_tok == '{') {
					unsigned int recursion;
					JITLexer_Yield(self);
					recursion = 1;
					for (;;) {
						if (!self->jl_tok)
							break;
						if (self->jl_tok == '{') {
							++recursion;
						} else if (self->jl_tok == '}') {
							--recursion;
							if (!recursion) {
								JITLexer_Yield(self);
								break;
							}
						}
						JITLexer_Yield(self);
					}
				} else {
					while (self->jl_tok == JIT_STRING ||
					       self->jl_tok == JIT_RAWSTRING) {
						JITLexer_Yield(self);
					}
				}
				while (self->jl_tok == ':' || self->jl_tok == TOK_COLON_EQUAL) {
					if (self->jl_tok == TOK_COLON_EQUAL) {
						++self->jl_tokstart;
						self->jl_tok = '=';
					} else {
						JITLexer_Yield(self);
					}
continue_skip_asm_operand:
					if (self->jl_tok == '[') {
						do {
							JITLexer_Yield(self);
						} while (self->jl_tok && self->jl_tok != ']');
						if (self->jl_tok == ']')
							JITLexer_Yield(self);
					}
					while (self->jl_tok == JIT_STRING ||
					       self->jl_tok == JIT_RAWSTRING)
						JITLexer_Yield(self);
					if (self->jl_tok == '(') {
						JITLexer_ScanExpression(self, false);
					} else if (JITLexer_ISKWD(self, "pack")) {
						JITLexer_Yield(self);
						JITLexer_ScanExpression(self, true);
					}
					if (self->jl_tok == ',') {
						JITLexer_Yield(self);
						goto continue_skip_asm_operand;
					}
				}
				if (has_lparen && self->jl_tok == ')')
					JITLexer_Yield(self);
			}
			break;

		case 8: {
			uint32_t nam2;
			name = UNALIGNED_GET32(tok_begin);
			nam2 = UNALIGNED_GET32(tok_begin + 4);
			if (name == ENCODE_INT32('c', 'o', 'n', 't') &&
			    nam2 == ENCODE_INT32('i', 'n', 'u', 'e'))
				goto do_yield_semi;
		}	break;

		default: break;
		}
	}	ATTR_FALLTHROUGH

	default:
		JITLexer_ScanExpression(self, true);
done_skip_semi:
		if (self->jl_tok == ';') {
	case ';':
			JITLexer_Yield(self);
		}
/*done:*/
		break;
	}
}


/* Assume that the given source text start/ends with `{' and `}'.
 * This function trims those characters, before also trimming any
 * additional whitespace next to them. */
INTERN NONNULL((1, 2)) void FCALL
JITFunction_TrimSurroundingBraces(/*utf-8*/ char const **__restrict p_source_start,
                                  /*utf-8*/ char const **__restrict p_source_end) {
	/*utf-8*/ char const *source_start = *p_source_start;
	/*utf-8*/ char const *source_end   = *p_source_end;

	/* Skip outer-most braces. */
	++source_start;
	--source_end;

	/* Skip additional whitespace */
	for (;;) {
		uint32_t ch;
		char const *next;
		next = source_start;
		ch   = unicode_readutf8_n(&next, source_end);
		if (!DeeUni_IsSpace(ch))
			break;
		source_start = next;
	}
	for (;;) {
		uint32_t ch;
		char const *next;
		next = source_end;
		ch   = unicode_readutf8_rev_n(&next, source_start);
		if (!DeeUni_IsSpace(ch))
			break;
		source_end = next;
	}

	/* Write-back trimmed pointers. */
	*p_source_start = source_start;
	*p_source_end   = source_end;
}



DECL_END

#endif /* !GUARD_DEX_JIT_FUNCTION_SCANNER_C */
