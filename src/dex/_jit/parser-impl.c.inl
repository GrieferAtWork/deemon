/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "parser.c"
//#define JIT_SKIP 1
#define JIT_EVAL 1
#endif /* __INTELLISENSE__ */

#include <deemon/bool.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

#if !defined(JIT_SKIP) && !defined(JIT_EVAL)
#error "Must either #define JIT_SKIP or JIT_EVAL before #including this file"
#endif /* !JIT_SKIP && !JIT_EVAL */

#undef ERROR
#undef F
#undef ISOK
#undef ISERR

#if 1
#define JITLEXER_EVAL_FSECONDARY   JITLEXER_EVAL_FNORMAL
#else
#define JITLEXER_EVAL_FSECONDARY  (flags & ~(JITLEXER_EVAL_FPRIMARY))
#endif

#ifdef JIT_SKIP
#define LOAD_LVALUE(obj, err)            (void)0
#define LOAD_LVALUE_OR_RETURN_ERROR(obj) (void)0
#define IFELSE(if_eval, if_skip)         if_skip
#define IF_SKIP(...)                     __VA_ARGS__
#define IF_EVAL(...)                     /* nothing */
#define SYNTAXERROR(...)                                  \
	(DeeError_Throwf(&DeeError_SyntaxError, __VA_ARGS__), \
	 JITLexer_ErrorTrace(self, self->jl_tokstart),        \
	 self->jl_context->jc_flags |= JITCONTEXT_FSYNERR, -1)
#define DECREF(x)                    (void)0
#define DECREF_MAYBE_LVALUE(x)       (void)0
#define ERROR                        -1
#define RETURN_TYPE                  int
#define LHS_OR_OK                    0
#define ISOK(x)                      likely(!(x))
#define ISERR(x)                     unlikely(x)
#define FUNC(x)                      JITLexer_Skip##x
#define CALL_PRIMARY(name)           JITLexer_Skip##name(self, flags)
#define CALL_PRIMARYF(name, flags)   JITLexer_Skip##name(self, flags)
#define CALL_SECONDARY(name, result) JITLexer_Skip##name(self, JITLEXER_EVAL_FSECONDARY)
#define DEFINE_PRIMARY(name) \
	INTERN WUNUSED NONNULL((1)) int DFCALL JITLexer_Skip##name(JITLexer *__restrict self, unsigned int flags)
#define DEFINE_SECONDARY(name) \
	INTERN WUNUSED NONNULL((1)) int DFCALL JITLexer_Skip##name(JITLexer *__restrict self, unsigned int flags)
#else /* JIT_SKIP */
#define LOAD_LVALUE(obj, err)                                                   \
	do {                                                                        \
		if ((obj) == JIT_LVALUE && ((obj) = JITLexer_PackLValue(self)) == NULL) \
			goto err;                                                           \
	}	__WHILE0
#define LOAD_LVALUE_OR_RETURN_ERROR(obj)                                        \
	do {                                                                        \
		if ((obj) == JIT_LVALUE && ((obj) = JITLexer_PackLValue(self)) == NULL) \
			return NULL;                                                        \
	}	__WHILE0

#define IFELSE(if_eval, if_skip) if_eval
#define IF_SKIP(...)             /* nothing */
#define IF_EVAL(...)             __VA_ARGS__
#define SYNTAXERROR(...)                                  \
	(DeeError_Throwf(&DeeError_SyntaxError, __VA_ARGS__), \
	 JITLexer_ErrorTrace(self, self->jl_tokstart),        \
	 self->jl_context->jc_flags |= JITCONTEXT_FSYNERR,    \
	 NULL)

#define ERROR                        NULL
#define RETURN_TYPE                  DREF DeeObject *
#define DECREF(x)                    Dee_Decref(x)
#define DECREF_MAYBE_LVALUE(x)       ((x) == JIT_LVALUE || (Dee_Decref(x), 0))
#define LHS_OR_OK                    lhs
#define ISOK(x)                      likely(x)
#define ISERR(x)                     unlikely(!(x))
#define FUNC(x)                      JITLexer_Eval##x
#define CALL_PRIMARY(name)           JITLexer_Eval##name(self, flags)
#define CALL_PRIMARYF(name, flags)   JITLexer_Eval##name(self, flags)
#define CALL_SECONDARY(name, result) JITLexer_Eval##name(self, result, JITLEXER_EVAL_FSECONDARY)
#define DEFINE_PRIMARY(name)                          \
	INTERN WUNUSED NONNULL((1)) DREF DeeObject *DFCALL \
	JITLexer_Eval##name(JITLexer *__restrict self, unsigned int flags)
#define DEFINE_SECONDARY(name)                                              \
	INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL                    \
	JITLexer_Eval##name(JITLexer *__restrict self,                          \
	                    /*inherit(always)*/ DREF DeeObject *__restrict lhs, \
	                    unsigned int flags)
#endif /* !JIT_SKIP */




DECL_BEGIN

#ifndef ERROR_CANNOT_TEST_BINDING_DEFINED
#define ERROR_CANNOT_TEST_BINDING_DEFINED 1
PRIVATE ATTR_COLD int DCALL
err_cannot_invoke_inplace(DeeObject *base, Dee_operator_t opname) {
	DeeTypeObject *typetype = &DeeType_Type;
	struct opinfo const *info;
	if (base) {
		typetype = Dee_TYPE(base);
		if (typetype == &DeeSuper_Type)
			typetype = DeeSuper_TYPE(base);
		typetype = Dee_TYPE(typetype);
	}
	info = DeeTypeType_GetOperatorById(typetype, opname);
	if likely(info) {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Cannot invoke inplace `operator %s' (`__%s__') without l-value",
		                       info->oi_uname, info->oi_sname);
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Cannot invoke inplace operator #%X without l-value",
		                       opname);
	}
}
#endif /* !ERROR_CANNOT_TEST_BINDING_DEFINED */

#ifdef CONFIG_HAVE_FPU
#ifndef TPPLIKE_STRING_TO_FLOAT_DEFINED
#define TPPLIKE_STRING_TO_FLOAT_DEFINED 1
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
JIT_atof(JITLexer *__restrict self,
         char const *__restrict start, size_t length) {
	double result;
	char const *endptr;
	result = Dee_Strtod(start, (char **)&endptr);
	if (endptr != start + length)
		goto err_badnum;
	return DeeFloat_New(result);
err_badnum:
	(void)SYNTAXERROR("Invalid floating point number %$q",
	                  length, start);
	return NULL;
}
#endif /* !TPPLIKE_STRING_TO_FLOAT_DEFINED */
#endif /* CONFIG_HAVE_FPU */


INTERN RETURN_TYPE DFCALL
#ifdef JIT_EVAL
JITLexer_EvalKeywordLabelList(JITLexer *__restrict self,
                              char const *__restrict first_label_name,
                              size_t first_label_size)
#else /* JIT_EVAL */
JITLexer_SkipKeywordLabelList(JITLexer *__restrict self)
#endif /* !JIT_EVAL */
{
	RETURN_TYPE result;
#ifdef JIT_EVAL
	DREF DeeObject *value;
	int error;
	result = DeeDict_New();
	if unlikely(!result)
		goto err;
again_eval_expression:
	value = JITLexer_EvalRValue(self);
	if unlikely(!value)
		goto err_r;
	error = DeeDict_SetItemStringLen(result,
	                                 first_label_name,
	                                 first_label_size,
	                                 value);
	Dee_Decref(value);
	if unlikely(error)
		goto err_r;
	if (self->jl_tok == ',') {
		JITLexer_Yield(self);
		if (self->jl_tok == JIT_KEYWORD) {
			first_label_name = (char const *)self->jl_tokstart;
			first_label_size = (size_t)(self->jl_tokend -
			                            (unsigned char const *)first_label_name);
			JITLexer_Yield(self);
			if (self->jl_tok == ':') {
				JITLexer_Yield(self);
				goto again_eval_expression;
			}
			JITLexer_YieldAt(self, (unsigned char const *)first_label_name);
		}
	}
#else /* JIT_EVAL */
again_skip_expression:
	if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
		goto err;
	if (self->jl_tok == ',') {
		JITLexer_Yield(self);
		if (self->jl_tok == JIT_KEYWORD) {
			unsigned char const *start = self->jl_tokstart;
			JITLexer_Yield(self);
			if (self->jl_tok == ':') {
				JITLexer_Yield(self);
				goto again_skip_expression;
			}
			JITLexer_YieldAt(self, start);
		}
	}
	result = 0;
#endif /* !JIT_EVAL */
	return result;
#ifdef JIT_EVAL
err_r:
	Dee_Decref(result);
#endif /* JIT_EVAL */
err:
	return ERROR;
}


INTERN RETURN_TYPE DFCALL
#ifdef JIT_EVAL
JITLexer_EvalArgumentList(JITLexer *__restrict self, DREF DeeObject **__restrict p_kwds)
#else /* JIT_EVAL */
JITLexer_SkipArgumentList(JITLexer *__restrict self)
#endif /* !JIT_EVAL */
{
	RETURN_TYPE result;
#ifdef JIT_EVAL
	*p_kwds = NULL;
	result = JITLexer_EvalComma(self,
	                            AST_COMMA_FORCEMULTIPLE |
	                            AST_COMMA_ALLOWKWDLIST,
	                            &DeeTuple_Type,
	                            NULL);
	LOAD_LVALUE(result, err);
	if (self->jl_tok == TOK_POW) {
		JITLexer_Yield(self);
		/* Parse the keyword invocation AST. */
		*p_kwds = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
		if unlikely(!*p_kwds)
			goto err_r;
	} else if (self->jl_tok == JIT_KEYWORD) {
		unsigned char const *name = self->jl_tokstart;
		size_t size               = (size_t)(self->jl_tokend - name);
		JITLexer_Yield(self);
		if unlikely(self->jl_tok != ':') {
			JITLexer_YieldAt(self, name);
			goto done;
		}
		JITLexer_Yield(self);
		*p_kwds = JITLexer_EvalKeywordLabelList(self, (char const *)name, size);
		if unlikely(!*p_kwds)
			goto err_r;
	}
#else /* JIT_EVAL */
	result = JITLexer_SkipComma(self,
	                            AST_COMMA_FORCEMULTIPLE |
	                            AST_COMMA_ALLOWKWDLIST,
	                            NULL);
	if (ISERR(result))
		goto err;
	if (self->jl_tok == TOK_POW) {
		JITLexer_Yield(self);
		if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
			goto err_r;
	} else {
skip_keyword_label:
		if (self->jl_tok == JIT_KEYWORD) {
			unsigned char const *start = self->jl_tokstart;
			JITLexer_Yield(self);
			if unlikely(self->jl_tok != ':') {
				JITLexer_YieldAt(self, start);
				goto done;
			}
			JITLexer_Yield(self);
			if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
				goto err_r;
			if (self->jl_tok == ',') {
				JITLexer_Yield(self);
				goto skip_keyword_label;
			}
		}
	}
#endif /* !JIT_EVAL */
done:
	return result;
err_r:
	DECREF(result);
err:
	return ERROR;
}



DEFINE_PRIMARY(YieldAndParseUnaryKeywordOperand) {
	RETURN_TYPE result;
	(void)flags;
	JITLexer_Yield(self);
	if (self->jl_tok == '(') {
		result = CALL_PRIMARYF(UnaryHead,
		                       JITLEXER_EVAL_FSECONDARY |
		                       JITLEXER_EVAL_FDISALLOWCAST);
	} else {
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
	}
	return result;
}

DEFINE_SECONDARY(CastOperand) {
	RETURN_TYPE result;
	RETURN_TYPE merge;
	IF_EVAL(unsigned char const *pos;)
	(void)flags;
	IF_EVAL(pos = self->jl_tokstart;)
	switch (self->jl_tok) {

	case '!': {
		JITSmallLexer smlex;
		/* Special handling required:
		 * >> (int)!!!42;         // This...
		 * >> (int)!!!in my_list; // ... vs. this
		 * After parsing any number of additional `!' tokens, if the token
		 * thereafter is the keyword `is' or `in', then this isn't a cast
		 * expression. However if it isn't, then it is a cast expression. */
		memcpy(&smlex, self, sizeof(JITSmallLexer));
		for (;;) {
			JITLexer_Yield((JITLexer *)&smlex);
			if (smlex.jl_tok != '!')
				break;
		}
		if (!JITLexer_ISKWD(&smlex, "is") &&
		    !JITLexer_ISKWD(&smlex, "in"))
			goto do_a_cast;
		/* This isn't a cast expression. */
		goto not_a_cast;
	}

	case '+': /* `(typexpr).operator add(castexpr)' vs. `(typexpr)castexpr.operator pos()' */
	case '-': /* `(typexpr).operator sub(castexpr)' vs. `(typexpr)castexpr.operator neg()' */
/*	case '<': /* `(typexpr).operator lo(castexpr)' vs. `(typexpr)(Cell(castexpr))' */
	case '[': /* `(typexpr).operator [](castexpr)' vs. `(typexpr)(List(castexpr))' */
not_a_cast:
		/* Not a cast expression. */
		result = LHS_OR_OK; /* Inherit reference */
		break;

#if 1
	case '(': {
		bool second_paren;
		uint16_t out_mode;
		/* Special handling for the following cases:
		 * >> (float)();                // Call with 0 arguments
		 * >> (float)(42);              // Call with 1 argument `42'
		 * >> (float)((42),);           // Call with 1 argument `42'
		 * >> (float)(10, 20, 30);      // Call with 3 arguments `10, 20, 30'
		 * >> (float)(pack 10, 20, 30); // Call with 1 argument `(10, 20, 30)'
		 * Without this handling, the 4th line would be compiled as
		 * `float(pack(10, 20, 30))', when we want it to be `float(10, 20, 30)' */
		LOAD_LVALUE(lhs, err);
		JITLexer_Yield(self);
		if (self->jl_tok == ')') {
			/* Handle case #0 */
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			result = DeeObject_Call(lhs, 0, NULL);
			if unlikely(!result)
				JITLexer_ErrorTrace(self, pos);
			Dee_Decref(lhs);
#else /* JIT_EVAL */
			result = 0;
#endif /* JIT_EVAL */
			goto done;
		}
		second_paren = self->jl_tok == '(';
		/* Parse the cast-expression / argument list. */
		merge = FUNC(Comma)(self,
		                    AST_COMMA_FORCEMULTIPLE |
		                    AST_COMMA_ALLOWKWDLIST,
		                    IF_EVAL(&DeeTuple_Type, ) & out_mode);
		if (ISERR(merge))
			goto err_lhs;
		LOAD_LVALUE(merge, err_lhs);
		IF_EVAL(ASSERT(DeeTuple_Check(merge));)
		if likely(self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else if (self->jl_tok == TOK_POW) {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			{
				DREF DeeObject *kwds;
				/* Parse the keyword invocation AST. */
				kwds = JITLexer_EvalExpression(self, JITLEXER_EVAL_FNORMAL);
				if unlikely(!kwds)
					goto err_merge;
				result = DeeObject_CallTupleKw(lhs, merge, kwds);
				Dee_Decref(kwds);
				Dee_Decref(merge);
				Dee_Decref(lhs);
			}
#else /* JIT_EVAL */
			if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FNORMAL))
				goto err_merge;
			result = 0;
#endif /* !JIT_EVAL */
			break;
		} else if (self->jl_tok == JIT_KEYWORD) {
			unsigned char const *label_name = self->jl_tokstart;
#ifdef JIT_EVAL
			size_t label_size = (size_t)(self->jl_tokend - label_name);
#endif /* JIT_EVAL */
			JITLexer_Yield(self);
			if unlikely(self->jl_tok != ':') {
				JITLexer_YieldAt(self, label_name);
				goto err_missing_rparen;
			}
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			{
				DREF DeeObject *kwds;
				/* Parse the keyword invocation AST. */
				kwds = JITLexer_EvalKeywordLabelList(self,
				                                     (char const *)label_name,
				                                     label_size);
				if unlikely(!kwds)
					goto err_merge;
				result = DeeObject_CallTupleKw(lhs, merge, kwds);
				Dee_Decref(kwds);
				Dee_Decref(merge);
				Dee_Decref(lhs);
			}
#else /* JIT_EVAL */
#if 1
			if (JITLexer_SkipPair(self, '(', ')'))
				goto err_merge;
#else
			if (JITLexer_SkipKeywordLabelList(self))
				goto err_merge;
#endif
			result = 0;
#endif /* !JIT_EVAL */
			break;
		} else {
err_missing_rparen:
			syn_paren_expected_rparen_after_lparen(self);
			goto err_merge;
		}
		if (!second_paren && !(out_mode & AST_COMMA_OUT_FMULTIPLE)) {
			/* Recursively parse cast suffix expressions:
			 * >> (int)(float)get_value(); 
			 * Parse as:
			 * >> int(float(get_value()));
			 * Without this, it would be parsed as:
			 * >> int(float)(get_value());
			 */
#ifdef JIT_EVAL
			unsigned char const *cast_start = self->jl_tokstart;
			Dee_Incref(DeeTuple_GET(merge, 0));
			result = JITLexer_EvalCastOperand(self,
			                                  DeeTuple_GET(merge, 0),
			                                  JITLEXER_EVAL_FSECONDARY);
#else /* JIT_EVAL */
			result = JITLexer_SkipCastOperand(self,
			                                  JITLEXER_EVAL_FSECONDARY);
#endif /* JIT_EVAL */
			if (ISERR(result))
				goto err_merge;
			LOAD_LVALUE(result, err_merge);
#ifdef JIT_EVAL
			if (cast_start == self->jl_tokstart) {
				ASSERT(result == DeeTuple_GET(merge, 0));
				Dee_Decref(result);
				/* Same argument expression. */
			} else {
				/* New argument expression. */
				Dee_Decref(merge);
				merge = DeeTuple_PackSymbolic(1, result);
				if unlikely(!merge) {
					Dee_Decref(result);
					goto err_lhs;
				}
			}
#endif /* JIT_EVAL */
		}
#ifdef JIT_EVAL
		result = DeeObject_CallTupleInherited(lhs, merge);
		Dee_Decref(merge);
#else /* JIT_EVAL */
		result = 0;
#endif /* JIT_EVAL */
	}	break;
#endif


	default:
		/* If what follows still isn't the start of
		 * an expression, then this isn't a cast. */
		if (!JITLexer_MaybeExpressionBegin(self))
			goto not_a_cast;
do_a_cast:
		/* Actually do a cast. */
		LOAD_LVALUE(lhs, err);
		merge = CALL_PRIMARY(Unary);
		LOAD_LVALUE(merge, err_lhs);
#ifdef JIT_EVAL
		if (ISERR(merge))
			goto err_lhs;
		result = DeeObject_Call(lhs, 1, (DeeObject **)&merge);
		Dee_Decref(merge);
		Dee_Decref(lhs);
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		break;
	}
done:
	return result;
err_merge:
	DECREF(merge);
err_lhs:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}








INTERN RETURN_TYPE DFCALL
FUNC(BraceItems)(JITLexer *__restrict self) {
	RETURN_TYPE result;
	if (self->jl_tok == '}') {
		JITLexer_Yield(self);
		/* Empty sequence. */
#ifdef JIT_EVAL
		result = DeeSeq_NewEmpty();
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		goto done;
	}
	if (self->jl_tok == '.') {
		IF_EVAL(DREF DeeObject * first_key;)
		JITLexer_Yield(self);
		if (self->jl_tok != JIT_KEYWORD) {
			syn_brace_expected_keyword_after_dot(self);
			goto err;
		}
#ifdef JIT_EVAL
		first_key = DeeString_NewUtf8(JITLexer_TokPtr(self),
		                              JITLexer_TokLen(self),
		                              STRING_ERROR_FSTRICT);
		if unlikely(!first_key)
			goto err;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
		if (self->jl_tok != '=') {
			syn_brace_expected_equals_after_dot(self);
			goto err;
		}
		result = CALL_SECONDARY(CommaDictOperand, first_key);
	} else {
		/* Parse the initial key / sequence item. */
		result = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
		if (ISERR(result))
			goto err;
		if (self->jl_tok == ':') {
			/* Mapping-like Dict expression. */
			result = CALL_SECONDARY(CommaDictOperand, result);
		} else {
			result = CALL_SECONDARY(CommaListOperand, result);
		}
	}
done:
	return result;
err:
	return ERROR;
}








DEFINE_PRIMARY(UnaryHead) {
	RETURN_TYPE result;
	IF_EVAL(unsigned char const *pos;)
	uint16_t out_mode;
	(void)flags;
	IF_EVAL(pos = self->jl_tokstart;)
#ifdef JIT_EVAL
	ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
#endif /* JIT_EVAL */
	switch (self->jl_tok) {

	case TOK_INT:
		/* Integer constant. */
#ifdef JIT_EVAL
		result = DeeInt_FromString(JITLexer_TokPtr(self),
		                           JITLexer_TokLen(self),
		                           DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
done_y1:
		JITLexer_Yield(self);
		goto done;

	case JIT_STRING:
		/* String literal */
#ifdef JIT_EVAL
		result = DeeString_FromBackslashEscaped(JITLexer_TokPtr(self) + 1,
		                                        JITLexer_TokLen(self) - 2,
		                                        STRING_ERROR_FSTRICT);
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		goto done_y1;

	case JIT_RAWSTRING:
		/* Raw string literal */
#ifdef JIT_EVAL
		result = DeeString_NewUtf8(JITLexer_TokPtr(self) + 2,
		                           JITLexer_TokLen(self) - 3,
		                           STRING_ERROR_FSTRICT);
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		goto done_y1;

#ifdef CONFIG_HAVE_FPU
	case TOK_FLOAT:
		/* Floating point constant */
#ifdef JIT_EVAL
		result = JIT_atof(self,
		                  JITLexer_TokPtr(self),
		                  JITLexer_TokLen(self));
#else /* JIT_EVAL */
		result = 0;
#endif /* !JIT_EVAL */
		goto done_y1;
#endif /* CONFIG_HAVE_FPU */

	case '#':
		JITLexer_Yield(self);
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
		if (ISERR(result))
			goto done;
		LOAD_LVALUE(result, err);
		{
			DREF DeeObject *new_result;
			new_result = DeeObject_SizeOb(result);
			if unlikely(!new_result)
				goto err_r_invoke;
			Dee_Decref(result);
			result = new_result;
		}
#endif /* JIT_EVAL */
		goto done;

	case '~':
		JITLexer_Yield(self);
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
		if (ISERR(result))
			goto done;
		LOAD_LVALUE(result, err);
		{
			DREF DeeObject *new_result;
			new_result = DeeObject_Inv(result);
			if unlikely(!new_result)
				goto err_r_invoke;
			Dee_Decref(result);
			result = new_result;
		}
#endif /* JIT_EVAL */
		goto done;

	case '+':
		JITLexer_Yield(self);
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
		if (ISERR(result))
			goto done;
		LOAD_LVALUE(result, err);
		{
			DREF DeeObject *new_result;
			new_result = DeeObject_Pos(result);
			if unlikely(!new_result)
				goto err_r_invoke;
			Dee_Decref(result);
			result = new_result;
		}
#endif /* JIT_EVAL */
		goto done;

	case '-':
		JITLexer_Yield(self);
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
		if (ISERR(result))
			goto done;
		LOAD_LVALUE(result, err);
		{
			DREF DeeObject *new_result;
			new_result = DeeObject_Neg(result);
			if unlikely(!new_result)
				goto err_r_invoke;
			Dee_Decref(result);
			result = new_result;
		}
#endif /* JIT_EVAL */
		goto done;

	case TOK_INC:
	case TOK_DEC: {
#ifdef JIT_EVAL
		unsigned int cmd = self->jl_tok;
		int error;
#endif /* JIT_EVAL */
		/* Need a custom parser protocol! */
		JITLexer_Yield(self);
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
		if (ISERR(result))
			goto done;
#ifdef JIT_EVAL
		if (result != JIT_LVALUE) {
			err_cannot_invoke_inplace(NULL,
			                          cmd == TOK_INC
			                          ? OPERATOR_INC
			                          : OPERATOR_DEC);
			goto err;
		}
		result = JITLValue_GetValue(&self->jl_lvalue,
		                            self->jl_context);
		if unlikely(!result)
			goto err;
		error = cmd == TOK_INC
		        ? DeeObject_Inc(&result)
		        : DeeObject_Dec(&result);
		if unlikely(error)
			goto err_r;
		error = JITLValue_SetValue(&self->jl_lvalue,
		                           self->jl_context,
		                           result);
		if unlikely(error)
			goto err_r;
		JITLValue_Fini(&self->jl_lvalue);
		JITLValue_Init(&self->jl_lvalue);
#endif /* JIT_EVAL */
		goto done;
	}


	case '!': /* not */
		JITLexer_Yield(self);
		result = CALL_PRIMARYF(Unary, JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
		if (ISERR(result))
			goto done;
		LOAD_LVALUE(result, err);
		{
			int new_result;
			new_result = DeeObject_Bool(result);
			if unlikely(new_result < 0)
				goto err_r_invoke;
			Dee_Decref(result);
			result = DeeBool_For(!new_result);
			Dee_Incref(result);
		}
#endif /* JIT_EVAL */
		goto done;

	case '(': {
		bool allow_cast;
		/* Parenthesis. */
		JITLexer_Yield(self);
		allow_cast = self->jl_tok != '(' && !(flags & JITLEXER_EVAL_FDISALLOWCAST);
#if 1
		if (self->jl_tok == '{') {
			unsigned int was_expression;
			/* Statements in expressions. */
			result = FUNC(StatementOrBraces)(self, &was_expression);
			if (ISERR(result))
				goto err;
			LOAD_LVALUE(result, err);
			allow_cast = false; /* Don't allow braces, or statements as cast expressions. */
			if (self->jl_tok == ',' && was_expression != AST_PARSE_WASEXPR_NO) {
				JITLexer_Yield(self);
				if (self->jl_tok == ')') {
					/* single-element tuple expression, where the single element is a sequence. */
#ifdef JIT_EVAL
					DREF DeeObject *tuple;
					tuple = DeeTuple_PackSymbolic(1, result);
					if unlikely(!tuple)
						goto err_r;
					result = tuple; /* Inherit reference. */
#endif /* JIT_EVAL */
				} else {
					/* There are more items! */
					RETURN_TYPE merge;
					merge = FUNC(Comma)(self, AST_COMMA_FORCEMULTIPLE, IF_EVAL(&DeeTuple_Type, ) NULL);
					if (ISERR(merge))
						goto err_r;
#ifdef JIT_EVAL
					ASSERT(DeeTuple_Check(merge));
					if (merge == Dee_EmptyTuple) {
						DREF DeeObject *tuple;
						Dee_DecrefNokill(merge);
						tuple = DeeTuple_PackSymbolic(1, result);
						if unlikely(!tuple)
							goto err_r;
						result = tuple; /* Inherit reference. */
					} else {
						DREF DeeTupleObject *tuple;
						tuple = DeeTuple_NewUninitialized(1 + DeeTuple_SIZE(merge));
						if unlikely(!tuple) {
							Dee_Decref(merge);
							goto err_r;
						}
						DeeTuple_SET(tuple, 0, result);   /* Inherit reference. */
						memcpyc(DeeTuple_ELEM(tuple) + 1, /* Inherit references. */
						        DeeTuple_ELEM(merge),
						        DeeTuple_SIZE(merge),
						        sizeof(DREF DeeObject *));
						DeeTuple_DecrefSymbolic(merge);
						result = (DREF DeeObject *)tuple; /* Inherit references. */
					}
#endif /* JIT_EVAL */
				}
			}
			if (self->jl_tok != ')' && was_expression != AST_PARSE_WASEXPR_NO) {
				result = CALL_SECONDARY(Operand, result);
				if (ISERR(result))
					goto err;
				/*was_expression = AST_PARSE_WASEXPR_YES;*/
			}
		} else
#endif
		{
			/* Check for java-style lambda. */
			unsigned int             saved_jl_tok;
			/*utf-8*/ unsigned char const *saved_jl_tokstart;
			/*utf-8*/ unsigned char const *saved_jl_tokend;
			IF_EVAL(unsigned char const *param_end);
			IF_EVAL(unsigned int recursion);
			saved_jl_tok      = self->jl_tok;
			saved_jl_tokstart = self->jl_tokstart;
			saved_jl_tokend   = self->jl_tokend;

#ifdef JIT_EVAL
			/* FIXME: We can't just blindly skip over what (we think) is an argument list:
			 * >> true ? ([]) : int -> 42
			 * This example has to be parsed as `true ? List({}) : [](int) { return 42; }'
			 * But because we're skipping over everything inside of arguments, we end up
			 * trying to parse it as: `true ? []([]): int { return 42; } : none', which
			 * later causes an error:            ^
			 *                         Expected a keyword as argument name, but got `['
			 */
			param_end = self->jl_tokstart;
			recursion = 1;
			for (;;) {
				if (!self->jl_tok)
					goto not_a_java_lambda;
				if (self->jl_tok == '(') {
					++recursion;
				} else if (self->jl_tok == ')') {
					--recursion;
					if (!recursion) {
						JITLexer_Yield(self);
						break;
					}
				}
				param_end = self->jl_tokend;
				JITLexer_Yield(self);
			}
#else /* JIT_EVAL */
			if (JITLexer_SkipPair(self, '(', ')'))
				goto err;
#endif /* !JIT_EVAL */
			if (self->jl_tok == ':') {
				JITLexer_Yield(self);
				if (JITLexer_SkipTypeAnnotation(self, false) != 0)
					goto not_a_java_lambda;
			}
			if (self->jl_tok == TOK_ARROW) {
				IF_EVAL(unsigned char const *source_start);
				IF_EVAL(unsigned char const *source_end);
				IF_EVAL(unsigned int is_expression);

				/* Yup! it's a java-lambda alright! */
				JITLexer_Yield(self);
				IF_EVAL(source_start = self->jl_tokstart);
				if (self->jl_tok == '{') {
					if (JITLexer_SkipHybridAtBrace(self, IFELSE(&is_expression, NULL)))
						goto err;
				} else {
					IF_EVAL(is_expression = AST_PARSE_WASEXPR_YES);
					if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY))
						goto err;
				}

#ifdef JIT_EVAL
				source_end = self->jl_tokstart;
				/* Trim trailing whitespace. */
				while (source_end > source_start) {
					uint32_t ch;
					char const *next;
					next = (char const *)source_end;
					ch   = unicode_readutf8_rev_n(&next, source_start);
					if (!DeeUni_IsSpace(ch))
						break;
					source_end = (unsigned char const *)next;
				}
				if (is_expression == AST_PARSE_WASEXPR_NO) {
					/* Get rid of the surrounding '{' and '}'
					 * Execution would work the same even with them there, however
					 * their presence looks weird in `repr', and might make debugging
					 * harder (because one might not immediately understand where
					 * they're from) */
					JITFunction_TrimSurroundingBraces((char const **)&source_start,
					                                  (char const **)&source_end);
				}
				result = JITFunction_New(NULL,
				                         NULL,
				                         (char const *)saved_jl_tokstart,
				                         (char const *)param_end,
				                         (char const *)source_start,
				                         (char const *)source_end,
				                         self->jl_context,
				                         self->jl_text,
				                         is_expression == AST_PARSE_WASEXPR_NO
				                         ? JIT_FUNCTION_FNORMAL  /* w/ statements */
				                         : JIT_FUNCTION_FRETEXPR /* w/ return-expression */);
				if (!result && DeeError_CurrentIs(&DeeError_SyntaxError))
					self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			}
not_a_java_lambda:
			/* Restore lexer state */
			self->jl_tok      = saved_jl_tok;
			self->jl_tokstart = saved_jl_tokstart;
			self->jl_tokend   = saved_jl_tokend;

			if (self->jl_tok == ')') {
				/* Empty tuple. */
#ifdef JIT_EVAL
				result = DeeTuple_NewEmpty();
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				allow_cast = false; /* Don't allow empty tuples for cast expressions. */
			} else {
				/* Parenthesis / tuple expression. */
				out_mode = 0;
				result   = FUNC(Comma)(self, AST_COMMA_NORMAL, IF_EVAL(&DeeTuple_Type, ) &out_mode);
				if (ISERR(result))
					goto err;
				if (out_mode & AST_COMMA_OUT_FMULTIPLE)
					allow_cast = false; /* Don't allow comma-lists for cast expressions. */
			}
		}
		if likely(self->jl_tok == ')') {
			JITLexer_Yield(self);
		} else {
			syn_paren_expected_rparen_after_lparen(self);
			goto err_r;
		}
		if (allow_cast) {
			/* C-style cast expression (only for single-parenthesis expressions) */
			result = CALL_SECONDARY(CastOperand, result);
		}
	}	break;

	case '[':
		JITLexer_Yield(self);
		if (self->jl_tok == ']') {
			JITLexer_Yield(self);
			if (self->jl_tok == '(') {
#ifdef JIT_EVAL
				unsigned char const *param_start;
				unsigned char const *param_end;
				unsigned int recursion;
#endif /* JIT_EVAL */
				JITLexer_Yield(self);
#ifdef JIT_EVAL
				param_start = self->jl_tokstart;
				param_end   = param_start;
				recursion   = 1;
				while (self->jl_tok) {
					if (self->jl_tok == '(') {
						++recursion;
					} else if (self->jl_tok == ')') {
						--recursion;
						if (!recursion) {
							JITLexer_Yield(self);
							break;
						}
					}
					param_end = self->jl_tokend;
					JITLexer_Yield(self);
				}
#else /* JIT_EVAL */
				if (JITLexer_SkipPair(self, '(', ')'))
					goto err;
#endif /* !JIT_EVAL */
				if (self->jl_tok == TOK_ARROW) {
#ifdef JIT_EVAL
					/* Lambda function. */
					unsigned char const *source_start;
					unsigned char const *source_end;
					JITLexer_Yield(self);
					source_start = self->jl_tokstart;
					if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY))
						goto err;
					source_end = self->jl_tokstart;
					/* Trim trailing whitespace. */
					while (source_end > source_start) {
						uint32_t ch;
						char const *next;
						next = (char const *)source_end;
						ch   = unicode_readutf8_rev_n(&next, source_start);
						if (!DeeUni_IsSpace(ch))
							break;
						source_end = (unsigned char const *)next;
					}
					result = JITFunction_New(NULL,
					                         NULL,
					                         (char const *)param_start,
					                         (char const *)param_end,
					                         (char const *)source_start,
					                         (char const *)source_end,
					                         self->jl_context,
					                         self->jl_text,
					                         JIT_FUNCTION_FRETEXPR);
					if (!result && DeeError_CurrentIs(&DeeError_SyntaxError))
						self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
					goto done;
#else /* JIT_EVAL */
					goto skip_arrow_lambda;
#endif /* !JIT_EVAL */
				} else if (self->jl_tok == '{') {
#ifdef JIT_EVAL
					/* Lambda function. */
					unsigned char const *source_start;
					unsigned char const *source_end;
					unsigned int brace_recursion = 1;
					JITLexer_Yield(self);
					source_end = source_start = self->jl_tokstart;
					/* Scan the body of the function. */
					while (self->jl_tok) {
						if (self->jl_tok == '{') {
							++brace_recursion;
						} else if (self->jl_tok == '}') {
							--brace_recursion;
							if (!brace_recursion) {
								JITLexer_Yield(self);
								break;
							}
						}
						source_end = self->jl_tokend;
						JITLexer_Yield(self);
					}
					result = JITFunction_New(NULL,
					                         NULL,
					                         (char const *)param_start,
					                         (char const *)param_end,
					                         (char const *)source_start,
					                         (char const *)source_end,
					                         self->jl_context,
					                         self->jl_text,
					                         JIT_FUNCTION_FNORMAL);
					if (!result && DeeError_CurrentIs(&DeeError_SyntaxError))
						self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
					goto done;
#else /* JIT_EVAL */
					goto skip_brace_lambda;
#endif /* !JIT_EVAL */
				} else {
					syn_function_expected_arrow_or_lbrace_after_function(self);
					goto err;
				}
			}

			if (self->jl_tok == ':') {
				/* If the current token is ':', try to skip over the return type
				 * annotation and check if the next token thereafter is '->' or '{'. */
				/*utf-8*/ unsigned char const *saved_jl_tokstart;
				/*utf-8*/ unsigned char const *saved_jl_tokend;
				saved_jl_tokstart = self->jl_tokstart;
				saved_jl_tokend   = self->jl_tokend;
				JITLexer_Yield(self);
				if (JITLexer_SkipTypeAnnotation(self, false) == 0 &&
				    (self->jl_tok == TOK_ARROW || self->jl_tok == '{')) {
					/* Jup: this must be a lambda! */

					/* fallthru to the proper handler below. */
				} else {
					/* Rewind... */
					self->jl_tok      = ':';
					self->jl_tokstart = saved_jl_tokstart;
					self->jl_tokend   = saved_jl_tokend;
				}
			}

			if (self->jl_tok == TOK_ARROW) {
#ifdef JIT_EVAL
				/* Lambda function. */
				unsigned char const *source_start;
				unsigned char const *source_end;
				JITLexer_Yield(self);
				source_start = self->jl_tokstart;
				if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY))
					goto err;
				source_end = self->jl_tokstart;
				/* Trim trailing whitespace. */
				while (source_end > source_start) {
					uint32_t ch;
					char const *next;
					next = (char const *)source_end;
					ch   = unicode_readutf8_rev_n(&next, source_start);
					if (!DeeUni_IsSpace(ch))
						break;
					source_end = (unsigned char const *)next;
				}
				result = JITFunction_New(NULL,
				                         NULL,
				                         NULL,
				                         NULL,
				                         (char const *)source_start,
				                         (char const *)source_end,
				                         self->jl_context,
				                         self->jl_text,
				                         JIT_FUNCTION_FRETEXPR);
				if (!result && DeeError_CurrentIs(&DeeError_SyntaxError))
					self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
				goto done;
#else /* JIT_EVAL */
skip_arrow_lambda:
				JITLexer_Yield(self);
				result = JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY);
				goto done;
#endif /* !JIT_EVAL */
			}
			if (self->jl_tok == '{') {
#ifdef JIT_EVAL
				/* Lambda function. */
				unsigned char const *source_start;
				unsigned char const *source_end;
				unsigned int recursion = 1;
				JITLexer_Yield(self);
				source_end = source_start = self->jl_tokstart;
				/* Scan the body of the function. */
				while (self->jl_tok) {
					if (self->jl_tok == '{') {
						++recursion;
					} else if (self->jl_tok == '}') {
						--recursion;
						if (!recursion) {
							JITLexer_Yield(self);
							break;
						}
					}
					source_end = self->jl_tokend;
					JITLexer_Yield(self);
				}
				result = JITFunction_New(NULL,
				                         NULL,
				                         NULL,
				                         NULL,
				                         (char const *)source_start,
				                         (char const *)source_end,
				                         self->jl_context,
				                         self->jl_text,
				                         JIT_FUNCTION_FNORMAL);
				if (!result && DeeError_CurrentIs(&DeeError_SyntaxError))
					self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
				goto done;
#else /* JIT_EVAL */
skip_brace_lambda:
				JITLexer_Yield(self);
				result = JITLexer_SkipPair(self, '{', '}');
				goto done;
#endif /* !JIT_EVAL */
			}

			/* Empty list. */
#ifdef JIT_EVAL
			result = DeeList_New();
#else /* JIT_EVAL */
			result = 0;
#endif /* !JIT_EVAL */
			goto done;
		}
#ifndef JIT_EVAL
		result = JITLexer_SkipPair(self, '[', ']');
#else /* !JIT_EVAL */
		if (self->jl_tok == ':') {
			/* Range generator with zero starting value. */
			JITLexer_Yield(self);
			result = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
			if (ISERR(result))
				goto err;
			LOAD_LVALUE(result, err);
			if (self->jl_tok == ',') {
				/* Step expression. */
				RETURN_TYPE step;
				JITLexer_Yield(self);
				step = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
				if (ISERR(step))
					goto err_r;
				LOAD_LVALUE(step, err_r);
#ifdef JIT_EVAL
				{
					DREF DeeObject *start;
					DREF DeeObject *range = NULL;
					start                 = DeeObject_NewDefault(Dee_TYPE(result));
					if likely(start) {
						range = DeeRange_New(start, result, step);
						Dee_Decref(start);
					}
					Dee_Decref(step);
					if unlikely(!range)
						goto err_r;
					Dee_Decref(result);
					result = range;
				}
#endif /* JIT_EVAL */
			} else {
#ifdef JIT_EVAL
				DREF DeeObject *start;
				DREF DeeObject *range;
				start = DeeObject_NewDefault(Dee_TYPE(result));
				if unlikely(!start)
					goto err_r;
				range = DeeRange_New(start, result, NULL);
				Dee_Decref(start);
				if unlikely(!range)
					goto err_r;
				Dee_Decref(result);
				result = range;
#endif /* JIT_EVAL */
			}
			goto skip_rbrck_and_done;
		}
		out_mode = 0;
#ifdef JIT_EVAL
		result   = JITLexer_EvalComma(self, AST_COMMA_NORMAL, &DeeList_Type, &out_mode);
#else /* JIT_EVAL */
		result = JITLexer_SkipComma(self, AST_COMMA_NORMAL, &out_mode);
#endif /* !JIT_EVAL */
		if (ISERR(result))
			goto err;
		LOAD_LVALUE(result, err);
		if (!(out_mode & AST_COMMA_OUT_FMULTIPLE)) {
			if (self->jl_tok == ':') {
				/* Range generate with non-zero starting value. */
				RETURN_TYPE range_end;
				JITLexer_Yield(self);
				range_end = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
				if (ISERR(range_end))
					goto err_r;
				LOAD_LVALUE(range_end, err_r);
				if (self->jl_tok == ',') {
					RETURN_TYPE step;
					JITLexer_Yield(self);
					step = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
					if (ISERR(step)) {
						IF_EVAL(err_r_range_end:)
						DECREF(range_end);
						goto err_r;
					}
					LOAD_LVALUE(step, err_r_range_end);
#ifdef JIT_EVAL
					{
						DREF DeeObject *range;
						range = DeeRange_New(result, range_end, step);
						Dee_Decref(step);
						if unlikely(!range)
							goto err_r_range_end;
						Dee_Decref(range_end);
						Dee_Decref(result);
						result = range; /* Inherit reference. */
					}
#endif /* JIT_EVAL */
				} else {
#ifdef JIT_EVAL
					DREF DeeObject *range;
					range = DeeRange_New(result, range_end, NULL);
					if unlikely(!range)
						goto err_r_range_end;
					Dee_Decref(range_end);
					Dee_Decref(result);
					result = range; /* Inherit reference. */
#endif /* JIT_EVAL */
				}
				goto skip_rbrck_and_done;
			}
#ifdef JIT_EVAL
			{
				DREF DeeObject *merge;
				/* Single-element list. */
				merge = DeeList_NewVectorInherited(1, (DeeObject *const *)&result);
				if unlikely(!merge)
					goto err_r;
				result = merge; /* Inherit reference */
			}
#endif /* JIT_EVAL */
		}
skip_rbrck_and_done:
		if likely(self->jl_tok == ']') {
			JITLexer_Yield(self);
		} else {
			syn_bracket_expected_rbracket_after_lbracket(self);
			goto err_r;
		}
#endif /* JIT_EVAL */
		goto done;

	case '{':
		JITLexer_Yield(self);
#ifndef JIT_EVAL
		result = JITLexer_SkipPair(self, '{', '}');
#else /* !JIT_EVAL */
		result = FUNC(BraceItems)(self);
		if (ISERR(result))
			goto err;
		if likely(self->jl_tok == '}') {
			JITLexer_Yield(self);
		} else {
			syn_brace_expected_rbrace(self);
			goto err_r;
		}
#endif /* JIT_EVAL */
		break;

	case TOK_DOTS:
		/* Anonymous varargs with automatic expansion.
		 * -> We handle this kind-of cheaty by simply returning the varargs symbol,
		 *    but not consuming the "..."-token. Thus, our caller will see that same
		 *    token and finally do the actual dots-expansion! */
#ifdef JIT_EVAL
		if (JITContext_Lookup(self->jl_context,
		                      (JITSymbol *)&self->jl_lvalue,
		                      "...",
		                      3,
		                      flags & ~LOOKUP_SYM_ALLOWDECL))
			goto err;
#endif /* JIT_EVAL */
		/* !!!NO YIELD HERE!!! -- That is intentional (see comment above!) */
		result = IFELSE(JIT_LVALUE, 0);
		break;

	case JIT_KEYWORD: {
		char const *tok_begin;
		size_t tok_length;
		uint32_t name;
		tok_begin  = (char const *)self->jl_tokstart;
		tok_length = (size_t)((char const *)self->jl_tokend - tok_begin);
		switch (tok_length) {

		case 1:
			/* Support for template strings! */
			if ((tok_begin[0] == 'f' || tok_begin[0] == 'F') &&
			    (tok_begin[1] == '\"' || tok_begin[1] == '\'')) {
				result = FUNC(TemplateString)(self);
				goto done;
			}
			break;

		case 2:
			if (tok_begin[0] == 'i' &&
			    tok_begin[1] == 'f') {
				result = FUNC(If)(self, false);
				goto done;
			}
			if (tok_begin[0] == 'd' &&
			    tok_begin[1] == 'o') {
				result = FUNC(Do)(self, false);
				goto done;
			}
			break;

		case 3:
			if (tok_begin[0] == 's' &&
			    tok_begin[1] == 't' &&
			    tok_begin[2] == 'r') {
				result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
				                       JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
				if (ISERR(result))
					goto done;
				LOAD_LVALUE(result, err);
				{
					DREF DeeObject *new_result;
					new_result = DeeObject_Str(result);
					if unlikely(!new_result)
						goto err_r_invoke;
					Dee_Decref(result);
					result = new_result;
				}
#endif /* JIT_EVAL */
				goto done;
			}
			if (tok_begin[0] == 't' &&
			    tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y') {
				result = FUNC(Try)(self, false);
				goto done;
			}
			if (tok_begin[0] == 'f' &&
			    tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
				result = FUNC(For)(self, false);
				goto done;
			}
			break;

		case 4:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('n', 'o', 'n', 'e')) {
#ifdef JIT_EVAL
				result = DeeNone_NewRef();
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done_y1;
			}
			if (name == ENCODE_INT32('t', 'r', 'u', 'e')) {
#ifdef JIT_EVAL
				result = DeeBool_NewTrue();
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done_y1;
			}
			if (name == ENCODE_INT32('r', 'e', 'p', 'r')) {
				result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
				                       JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
				if (ISERR(result))
					goto done;
				LOAD_LVALUE(result, err);
				{
					DREF DeeObject *new_result;
					new_result = DeeObject_Repr(result);
					if unlikely(!new_result)
						goto err_r_invoke;
					Dee_Decref(result);
					result = new_result;
				}
#endif /* JIT_EVAL */
				goto done;
			}
			if (name == ENCODE_INT32('c', 'o', 'p', 'y')) {
				result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
				                       JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
				if (ISERR(result))
					goto done;
				LOAD_LVALUE(result, err);
				{
					DREF DeeObject *new_result;
					new_result = DeeObject_Copy(result);
					if unlikely(!new_result)
						goto err_r_invoke;
					Dee_Decref(result);
					result = new_result;
				}
#endif /* JIT_EVAL */
				goto done;
			}
			if (name == ENCODE_INT32('t', 'y', 'p', 'e')) {
				result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
				                       JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
				if (ISERR(result))
					goto done;
				LOAD_LVALUE(result, err);
				{
					DREF DeeTypeObject *new_result;
					new_result = Dee_TYPE(result);
					Dee_Incref(new_result);
					Dee_Decref(result);
					result = (DREF DeeObject *)new_result;
				}
#endif /* JIT_EVAL */
				goto done;
			}
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
					result = ast_parse_statement_or_braces(NULL);
				} else
#endif
				if (JITLexer_MaybeExpressionBegin(self)) {
					/* Parse the packed expression. */
					result = FUNC(Comma)(self,
					                     has_paren
					                     ? AST_COMMA_FORCEMULTIPLE
					                     : AST_COMMA_FORCEMULTIPLE | AST_COMMA_STRICTCOMMA,
					                     IF_EVAL(&DeeTuple_Type, )
					                     NULL);
					if (ISERR(result))
						goto err;
				} else {
#ifdef JIT_EVAL
					result = DeeTuple_NewEmpty();
#else /* JIT_EVAL */
					result = 0;
#endif /* !JIT_EVAL */
				}
				if (has_paren) {
					if likely(self->jl_tok == ')') {
						JITLexer_Yield(self);
					} else {
						syn_pack_expected_rparen_after_lparen(self);
						goto err_r;
					}
				}
				goto done;
			}
			if (name == ENCODE_INT32('w', 'i', 't', 'h')) {
				result = FUNC(With)(self, false);
				goto done;
			}
			if (name == ENCODE_INT32('t', 'h', 'i', 's')) {
#ifdef JIT_EVAL
				if (JITContext_Lookup(self->jl_context,
				                      (JITSymbol *)&self->jl_lvalue,
				                      JITLexer_TokPtr(self),
				                      JITLexer_TokLen(self),
				                      LOOKUP_SYM_NORMAL))
					goto err;
				/* Convert an object reference into  */
				if (self->jl_lvalue.lv_kind != JIT_LVALUE_RVALUE) {
					result = JITLValue_GetValue(&self->jl_lvalue, self->jl_context);
					if unlikely(!result)
						goto err;
					JITLValue_Fini(&self->jl_lvalue);
					self->jl_lvalue.lv_rvalue = result;
				}
				self->jl_lvalue.lv_kind = JIT_LVALUE_THIS;
				result                  = JIT_LVALUE;
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				JITLexer_Yield(self);
				goto done;
			}
			break;

		case 5:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('f', 'a', 'l', 's') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'e') {
#ifdef JIT_EVAL
				result = DeeBool_NewFalse();
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done_y1;
			}
			if (name == ENCODE_INT32('b', 'o', 'u', 'n') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'd') {
				result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
				                       JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
				if (ISERR(result))
					goto done;
				if (result != JIT_LVALUE) {
					syn_bound_cannot_test(self);
					goto err_r;
				}
				/* Test the lvalue expression for being bound. */
				{
					int error;
					error = JITLValue_IsBound(&self->jl_lvalue,
					                          self->jl_context);
					if unlikely(error < 0)
						goto err_r;
					JITLValue_Fini(&self->jl_lvalue);
					JITLValue_Init(&self->jl_lvalue);
					result = DeeBool_For(error);
					Dee_Incref(result);
				}
#endif /* JIT_EVAL */
				goto done;
			}
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'e') {
				result = FUNC(While)(self, false);
				goto done;
			}
			if ((name == ENCODE_INT32('c', 'l', 'a', 's') && UNALIGNED_GET8(tok_begin + 4) == 's') ||
			    (name == ENCODE_INT32('f', 'i', 'n', 'a') && UNALIGNED_GET8(tok_begin + 4) == 'l')) {
#ifdef JIT_EVAL
				uint16_t tp_flags = TP_FNORMAL;
#endif /* JIT_EVAL */
				if (self->jl_tokstart[0] == 'f') {
#ifdef JIT_EVAL
					tp_flags = TP_FFINAL;
#endif /* JIT_EVAL */
					JITLexer_Yield(self);
					if (!JITLexer_ISKWD(self, "class")) {
						syn_class_expected_class_after_final(self);
						goto err;
					}
					JITLexer_Yield(self);
				} else {
					JITLexer_Yield(self);
					if (JITLexer_ISKWD(self, "final")) {
#ifdef JIT_EVAL
						tp_flags = TP_FFINAL;
#endif /* JIT_EVAL */
						JITLexer_Yield(self);
					}
				}
#ifdef JIT_EVAL
				result = (DREF DeeObject *)JITLexer_EvalClass(self, tp_flags);
#else /* JIT_EVAL */
				result = JITLexer_SkipClass(self);
#endif /* !JIT_EVAL */
				goto done;
			}
			if (name == ENCODE_INT32('s', 'u', 'p', 'e') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'r') {
				/* In jit, `super' is essentially compiled as `this as __identifier(class).base'
				 * s.a. `JIT_RTSYM_THIS' and `JIT_RTSYM_CLASS' */
#ifdef JIT_EVAL
				DeeTypeObject *oo_super;
				DREF DeeTypeObject *oo_class;
				DREF DeeObject *oo_this;
				if (JITContext_Lookup(self->jl_context,
				                      (JITSymbol *)&self->jl_lvalue,
				                      JIT_RTSYM_CLASS,
				                      COMPILER_STRLEN(JIT_RTSYM_CLASS),
				                      LOOKUP_SYM_NORMAL))
					goto err;
				oo_class = (DREF DeeTypeObject *)JITLValue_GetValue(&self->jl_lvalue, self->jl_context);
				JITLValue_Fini(&self->jl_lvalue);
				if unlikely(!oo_class) {
err_reinit_lvalue:
					JITLValue_Init(&self->jl_lvalue);
					goto err;
				}
				if (DeeObject_AssertType(oo_class, &DeeType_Type)) {
err_oo_class_reinit_lvalue:
					Dee_Decref(oo_class);
					goto err_reinit_lvalue;
				}
				if (JITContext_Lookup(self->jl_context,
				                      (JITSymbol *)&self->jl_lvalue,
				                      JIT_RTSYM_THIS,
				                      COMPILER_STRLEN(JIT_RTSYM_THIS),
				                      LOOKUP_SYM_NORMAL))
					goto err_oo_class_reinit_lvalue;
				oo_this = JITLValue_GetValue(&self->jl_lvalue, self->jl_context);
				if unlikely(!oo_this)
					goto err_oo_class_reinit_lvalue;
				JITLValue_Init(&self->jl_lvalue);
				oo_super = DeeType_Base(oo_class);
				if unlikely(!oo_super)
					oo_super = &DeeObject_Type;
				result = DeeSuper_New(oo_super, oo_this);
				Dee_Decref(oo_class);
				Dee_Decref(oo_this);
				if unlikely(!result)
					goto err;
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				JITLexer_Yield(self);
				goto done;
			}
			if (name == ENCODE_INT32('_', '_', 'n', 't') &&
			    UNALIGNED_GET8(tok_begin + 4) == 'h') {
				/* TODO */
			}
			break;

		case 6:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('i', 'm', 'p', 'o') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't')) {
#ifdef JIT_EVAL
				JITLexer_Yield(self);
				result = JITLexer_EvalImportExpression(self);
				goto done;
#else /* JIT_EVAL */
				result = 0;
				goto done_y1;
#endif /* !JIT_EVAL */
			}
			if (name == ENCODE_INT32('a', 's', 's', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('r', 't')) {
				result = FUNC(Assert)(self, false);
				goto done;
			}
			break;

		case 7:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'c') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'h') {
				result = FUNC(Foreach)(self, false);
				goto done;
			}
			break;

		case 8:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('d', 'e', 'e', 'p') &&
			    UNALIGNED_GET32(tok_begin + 4) == ENCODE_INT32('c', 'o', 'p', 'y')) {
				result = CALL_PRIMARYF(YieldAndParseUnaryKeywordOperand,
				                       JITLEXER_EVAL_FSECONDARY);
#ifdef JIT_EVAL
				if (ISERR(result))
					goto done;
				LOAD_LVALUE(result, err);
				{
					DREF DeeObject *new_result;
					new_result = DeeObject_DeepCopy(result);
					if unlikely(!new_result)
						goto err_r_invoke;
					Dee_Decref(result);
					result = new_result;
				}
#endif /* JIT_EVAL */
				goto done;
			}
			if (name == ENCODE_INT32('o', 'p', 'e', 'r') &&
			    UNALIGNED_GET32(tok_begin + 4) == ENCODE_INT32('a', 't', 'o', 'r')) {
				/* Operator function access. */
#ifdef JIT_EVAL
				int32_t opno;
				JITLexer_Yield(self);
				/* TODO: Don't hard-code "DeeType_Type" here! Instead, lookup the operator
				 *       via its name when the operator function actually gets called. */
				opno = JITLexer_ParseOperatorName(self, &DeeType_Type, P_OPERATOR_FNORMAL);
				if unlikely(opno < 0)
					goto err;
				result = JIT_GetOperatorFunction(&DeeType_Type, (Dee_operator_t)opno);
#else /* JIT_EVAL */
				JITLexer_Yield(self);
				if unlikely(JITLexer_SkipOperatorName(self))
					goto err;
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			}
			break;
		default: break;
		}
		/* Fallback: identifier lookup / <x from y> expression. */
		{
			IF_EVAL(char const *symbol_name  = JITLexer_TokPtr(self));
			IF_EVAL(size_t symbol_size = JITLexer_TokLen(self));
			JITLexer_Yield(self);
			if (self->jl_tok == TOK_ARROW) {
				/* Single-argument java-style lambda. */
				IF_EVAL(unsigned char const *source_start);
				IF_EVAL(unsigned char const *source_end);
				IF_EVAL(unsigned int is_expression);
				JITLexer_Yield(self);
				IF_EVAL(source_start = self->jl_tokstart);
				if (self->jl_tok == '{') {
					if (JITLexer_SkipHybridAtBrace(self, IFELSE(&is_expression, NULL)))
						goto err;
				} else {
					IF_EVAL(is_expression = AST_PARSE_WASEXPR_YES);
					if (JITLexer_SkipExpression(self, JITLEXER_EVAL_FSECONDARY))
						goto err;
				}

#ifdef JIT_EVAL
				source_end = self->jl_tokstart;
				/* Trim trailing whitespace. */
				while (source_end > source_start) {
					uint32_t ch;
					char const *next;
					next = (char const *)source_end;
					ch   = unicode_readutf8_rev_n(&next, source_start);
					if (!DeeUni_IsSpace(ch))
						break;
					source_end = (unsigned char const *)next;
				}
				if (is_expression == AST_PARSE_WASEXPR_NO) {
					/* Get rid of the surrounding '{' and '}'
					 * Execution would work the same even with them there, however
					 * their presence looks weird in `repr', and might make debugging
					 * harder (because one might not immediately understand where
					 * they're from) */
					JITFunction_TrimSurroundingBraces((char const **)&source_start,
					                                  (char const **)&source_end);
				}
				result = JITFunction_New(NULL,
				                         NULL,
				                         (char const *)symbol_name,
				                         (char const *)symbol_name + symbol_size,
				                         (char const *)source_start,
				                         (char const *)source_end,
				                         self->jl_context,
				                         self->jl_text,
				                         is_expression == AST_PARSE_WASEXPR_NO
				                         ? JIT_FUNCTION_FNORMAL  /* w/ statements */
				                         : JIT_FUNCTION_FRETEXPR /* w/ return-expression */);
				if (!result && DeeError_CurrentIs(&DeeError_SyntaxError))
					self->jl_context->jc_flags |= JITCONTEXT_FSYNERR;
#else /* JIT_EVAL */
				result = 0;
#endif /* !JIT_EVAL */
				goto done;
			} else if (JITLexer_ISKWD(self, "from")) {
#ifdef JIT_EVAL
				DREF DeeObject *mod;
				dhash_t symbol_hash;
				JITLexer_Yield(self);
				mod = JITLexer_EvalModule(self);
				if unlikely(!mod)
					goto err;
				symbol_hash = Dee_HashUtf8(symbol_name, symbol_size);
				if (DeeModule_Check(mod)) {
					struct module_symbol *symbol;
					symbol = DeeModule_GetSymbolStringLenHash((DREF DeeModuleObject *)mod,
					                                      symbol_name, symbol_size,
					                                      symbol_hash);
					if unlikely(!symbol) {
						DeeError_Throwf(&DeeError_SymbolError,
						                "Symbol `%$s' could not be found in mod `%k'",
						                symbol_size, symbol_name, mod);
						Dee_Decref(mod);
						goto err;
					}
					self->jl_lvalue.lv_kind          = JIT_LVALUE_EXTERN;
					self->jl_lvalue.lv_extern.lx_mod = (DREF DeeModuleObject *)mod; /* Inherit reference. */
					self->jl_lvalue.lv_extern.lx_sym = symbol;
				} else {
					/* When the import function didn't return */
					self->jl_lvalue.lv_kind = JIT_LVALUE_ATTRSTR;
					self->jl_lvalue.lv_attrstr.la_base = mod; /* Inherit reference. */
					self->jl_lvalue.lv_attrstr.la_name = symbol_name;
					self->jl_lvalue.lv_attrstr.la_size = symbol_size;
					self->jl_lvalue.lv_attrstr.la_hash = symbol_hash;
				}
#else /* JIT_EVAL */
				JITLexer_Yield(self);
				if unlikely(JITLexer_SkipModule(self))
					goto err;
#endif /* !JIT_EVAL */
			} else {
#ifdef JIT_EVAL
				if (JITContext_Lookup(self->jl_context,
				                      (JITSymbol *)&self->jl_lvalue,
				                      symbol_name,
				                      symbol_size,
				                      flags))
					goto err;
#endif /* JIT_EVAL */
			}
			result = IFELSE(JIT_LVALUE, 0);
		}
	}	break;

	default:
		syn_expr_unexpected_token(self);
		result = ERROR;
		break;
	}
done:
	return result;
#ifdef JIT_EVAL
err_r_invoke:
	JITLexer_ErrorTrace(self, pos);
#endif /* JIT_EVAL */
err_r:
	DECREF_MAYBE_LVALUE(result);
err:
	return ERROR;
}



DEFINE_SECONDARY(UnaryOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_UNARY(self));
	(void)flags;
#ifdef JIT_EVAL
again:
#endif /* JIT_EVAL */
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		switch (self->jl_tok) {

		case TOK_INC:
		case TOK_DEC: {
#ifdef JIT_EVAL
			DREF DeeObject *result_copy;
			unsigned int cmd = self->jl_tok;
			int error;
#endif /* JIT_EVAL */
			/* Need a custom parser protocol! */
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			if (lhs != JIT_LVALUE) {
				err_cannot_invoke_inplace(NULL,
				                          cmd == TOK_INC
				                          ? OPERATOR_INC
				                          : OPERATOR_DEC);
				goto err;
			}
			lhs = JITLValue_GetValue(&self->jl_lvalue,
			                         self->jl_context);
			if unlikely(!lhs)
				goto err;
			result_copy = DeeObject_Copy(lhs);
			if unlikely(!result_copy)
				goto err_r;
			error = cmd == TOK_INC
			        ? DeeObject_Inc((DeeObject **)&lhs)
			        : DeeObject_Dec((DeeObject **)&lhs);
			if unlikely(error) {
err_result_copy:
				Dee_Decref(result_copy);
				goto err_r;
			}
			error = JITLValue_SetValue(&self->jl_lvalue,
			                           self->jl_context,
			                           lhs);
			if unlikely(error)
				goto err_result_copy;
			JITLValue_Fini(&self->jl_lvalue);
			JITLValue_Init(&self->jl_lvalue);
			Dee_Decref(lhs);
			lhs = result_copy; /* Inherit reference. */
#endif /* JIT_EVAL */
		}	break;


		case '.':
			IF_EVAL(pos = self->jl_tokstart;)
			/* Attribute lookup. */
			JITLexer_Yield(self);
			if (self->jl_tok != JIT_KEYWORD) {
				syn_attr_expected_keyword(self);
				goto err_r;
			}
			if (JITLexer_ISTOK(self, "operator")) {
#ifdef JIT_EVAL
				int32_t opno;
				DREF DeeObject *opfun;
				DeeTypeObject *typetype;
				JITLexer_Yield(self);
				LOAD_LVALUE(lhs, err);
				typetype = Dee_TYPE(Dee_TYPE(lhs));
				opno = JITLexer_ParseOperatorName(self, typetype, P_OPERATOR_FNORMAL);
				if unlikely(opno < 0)
					goto err_r;
				opfun = JIT_GetOperatorFunction(typetype, (Dee_operator_t)opno);
				if unlikely(!opfun)
					goto err_r;
				Dee_Incref(lhs);
				rhs = DeeInstanceMethod_NewInherited(opfun, lhs);
				if unlikely(!rhs)
					goto err_r_invoke;
				Dee_DecrefNokill(lhs); /* Nokill because referenced by "rhs" */
				lhs = rhs;
#else /* JIT_EVAL */
				if unlikely(JITLexer_SkipOperatorName(self))
					goto err_r;
#endif /* !JIT_EVAL */
			} else if (JITLexer_ISTOK(self, "this")) {
				JITLexer_Yield(self);
			} else if (JITLexer_ISTOK(self, "class")) {
				JITLexer_Yield(self);
#ifdef JIT_EVAL
				LOAD_LVALUE(lhs, err);
				rhs = (DeeObject *)DeeObject_Class(lhs);
				Dee_Incref(rhs);
				Dee_Decref(lhs);
				lhs = rhs;
#endif /* JIT_EVAL */
			} else if (JITLexer_ISTOK(self, "super")) {
				JITLexer_Yield(self);
#ifdef JIT_EVAL
				LOAD_LVALUE(lhs, err);
				rhs = DeeSuper_Of(lhs);
				if unlikely(!rhs)
					goto err_r_invoke;
				Dee_Decref(lhs);
				lhs = rhs;
#endif /* JIT_EVAL */
			} else {
#ifdef JIT_EVAL
				char const *attr_name;
				size_t attr_size;
				/* Generic attribute lookup. */
				attr_name = JITLexer_TokPtr(self);
				attr_size = JITLexer_TokLen(self);
				if (lhs == JIT_LVALUE) {
					if (self->jl_lvalue.lv_kind == JIT_LVALUE_THIS) {
						/* The deemon specs require that `this.attrib' be evaluated
						 * as static-member-access, so-long as the named attribute
						 * actually exists.
						 *
						 * For this purpose, the specs only require that `this.foo' be
						 * static,  while `this.operator . ("foo")' is allowed to be
						 * dynamic. Confirming with this requirement, we only check
						 * for this-attribute-access in the case of `this.foo'! */
						DREF DeeTypeObject *oo_class;
						lhs = self->jl_lvalue.lv_rvalue;
						JITLValue_Init(&self->jl_lvalue);

						/* Lookup the class descriptor. */
						if (JITContext_Lookup(self->jl_context,
						                      (JITSymbol *)&self->jl_lvalue,
						                      JIT_RTSYM_CLASS,
						                      COMPILER_STRLEN(JIT_RTSYM_CLASS),
						                      LOOKUP_SYM_NORMAL))
							goto err;
						oo_class = (DREF DeeTypeObject *)JITLValue_GetValue(&self->jl_lvalue, self->jl_context);
						JITLValue_Fini(&self->jl_lvalue);
						JITLValue_Init(&self->jl_lvalue);
						if unlikely(!oo_class)
							goto err_r;
						if (DeeType_Check(oo_class) && DeeType_IsClass(oo_class)) {
							/* Check if `oo_class' contains an instance-member `attr_name'
							 * that had been declared as `private' or `final'. If it does,
							 * then we _must_ (as per the specs) access that attribute
							 * statically, rather than dynamically! */
							struct class_attribute *attrib;
							attrib = DeeClass_QueryClassAttributeStringLen(oo_class, attr_name, attr_size);
							if (attrib != NULL && (attrib->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
								/* Yes, it does! -> Construct an l-value reference to the accessed instance member. */
								JITLexer_Yield(self);
								self->jl_lvalue.lv_kind = JIT_LVALUE_CLSATTRIB;
								self->jl_lvalue.lv_clsattrib.lc_obj  = lhs; /* Inherit reference */
								self->jl_lvalue.lv_clsattrib.lc_attr = attrib;
								self->jl_lvalue.lv_clsattrib.lc_desc = DeeInstance_DESC(oo_class->tp_class, lhs);
								lhs = JIT_LVALUE;
								goto again;
							}
						}
						Dee_Decref(oo_class);
					} else {
						lhs = JITLexer_PackLValue(self);
						if unlikely(!lhs)
							goto err;
					}
				}
#endif /* JIT_EVAL */
				JITLexer_Yield(self);
#ifdef JIT_EVAL
				/* Generic attribute lookup */
				ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
				self->jl_lvalue.lv_kind            = JIT_LVALUE_ATTRSTR;
				self->jl_lvalue.lv_attrstr.la_base = lhs; /* Inherit reference. */
				self->jl_lvalue.lv_attrstr.la_name = attr_name;
				self->jl_lvalue.lv_attrstr.la_size = attr_size;
				self->jl_lvalue.lv_attrstr.la_hash = Dee_HashUtf8(attr_name, attr_size);
				lhs = JIT_LVALUE;
#endif /* JIT_EVAL */
			}
			break;

		case '[': {
			/* sequence operator */
			RETURN_TYPE temp;
			IF_EVAL(pos = self->jl_tokstart;)
			LOAD_LVALUE(lhs, err);
			JITLexer_Yield(self);
			if (self->jl_tok == ':') {
				JITLexer_Yield(self);
				if (self->jl_tok == ']') {
					JITLexer_Yield(self);
#ifdef JIT_EVAL
					ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
					self->jl_lvalue.lv_kind           = JIT_LVALUE_RANGE;
					self->jl_lvalue.lv_range.lr_base  = lhs; /* Inherit reference. */
					self->jl_lvalue.lv_range.lr_start = (DeeObject *)&DeeNone_Singleton;
					self->jl_lvalue.lv_range.lr_end   = (DeeObject *)&DeeNone_Singleton;
					Dee_Incref_n(&DeeNone_Singleton, 2);
					lhs = JIT_LVALUE;
#endif /* JIT_EVAL */
				} else {
					temp = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
					if (ISERR(temp))
						goto err_r;
					LOAD_LVALUE(temp, err_r);
					if (self->jl_tok == ']') {
						JITLexer_Yield(self);
					} else {
err_r_temp_expected_rbrck:
						DECREF(temp);
						syn_item_expected_rbracket_after_lbracket(self);
						goto err_r;
					}
#ifdef JIT_EVAL
					ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
					self->jl_lvalue.lv_kind           = JIT_LVALUE_RANGE;
					self->jl_lvalue.lv_range.lr_base  = lhs; /* Inherit reference. */
					self->jl_lvalue.lv_range.lr_start = DeeNone_NewRef();
					self->jl_lvalue.lv_range.lr_end   = temp; /* Inherit reference. */
					lhs = JIT_LVALUE;
#endif /* JIT_EVAL */
				}
			} else {
				temp = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
				if (ISERR(temp))
					goto err_r;
				LOAD_LVALUE(temp, err_r);
				if (self->jl_tok == ':') {
					JITLexer_Yield(self);
					if (self->jl_tok == ']') {
						JITLexer_Yield(self);
#ifdef JIT_EVAL
						ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
						self->jl_lvalue.lv_kind           = JIT_LVALUE_RANGE;
						self->jl_lvalue.lv_range.lr_base  = lhs;  /* Inherit reference. */
						self->jl_lvalue.lv_range.lr_start = temp; /* Inherit reference. */
						self->jl_lvalue.lv_range.lr_end   = DeeNone_NewRef();
						lhs = JIT_LVALUE;
#endif /* JIT_EVAL */
					} else {
#ifdef JIT_EVAL
						DREF DeeObject *start_expr;
						start_expr = temp;
#endif /* JIT_EVAL */
						temp = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
						if (self->jl_tok == ']') {
							JITLexer_Yield(self);
						} else {
							DECREF(start_expr);
							goto err_r_temp_expected_rbrck;
						}
#ifndef JIT_EVAL
						if (ISERR(temp))
							goto err_r;
#else /* !JIT_EVAL */
						if (ISERR(temp)) {
err_start_expr:
							Dee_Decref(start_expr);
							goto err_r;
						}
						LOAD_LVALUE(temp, err_start_expr);
						ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
						self->jl_lvalue.lv_kind           = JIT_LVALUE_RANGE;
						self->jl_lvalue.lv_range.lr_base  = lhs;        /* Inherit reference. */
						self->jl_lvalue.lv_range.lr_start = start_expr; /* Inherit reference. */
						self->jl_lvalue.lv_range.lr_end   = temp;       /* Inherit reference. */
						lhs = JIT_LVALUE;
#endif /* JIT_EVAL */
					}
				} else {
					if (self->jl_tok == ']') {
						JITLexer_Yield(self);
					} else {
						goto err_r_temp_expected_rbrck;
					}
#ifdef JIT_EVAL
					ASSERT(self->jl_lvalue.lv_kind == JIT_LVALUE_NONE);
					self->jl_lvalue.lv_kind          = JIT_LVALUE_ITEM;
					self->jl_lvalue.lv_item.li_base  = lhs;  /* Inherit reference. */
					self->jl_lvalue.lv_item.li_index = temp; /* Inherit reference. */
					lhs = JIT_LVALUE;
#endif /* JIT_EVAL */
				}
			}
		}	break;

		case '{':
			/* Brace initializer expressions. */
			LOAD_LVALUE(lhs, err);
			rhs = CALL_PRIMARYF(UnaryHead, JITLEXER_EVAL_FSECONDARY);
			if (ISERR(rhs))
				goto err_r;
#ifdef JIT_EVAL
			LOAD_LVALUE(rhs, err_r);
			{
				DREF DeeObject *merge;
				merge = DeeObject_Call(lhs, 1, &rhs);
				Dee_Decref(rhs);
				if unlikely(!merge)
					goto err_r;
				Dee_Decref(lhs);
				lhs = merge;
			}
#endif /* JIT_EVAL */
			break;

		case '(': {
			/* Call operator */
#ifdef JIT_EVAL
			DREF DeeObject *kwds;
#endif /* !JIT_EVAL */
			IF_EVAL(pos = self->jl_tokstart;)
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			if (lhs == JIT_LVALUE) {
				JITLValue function_lvalue;
				function_lvalue = self->jl_lvalue;
				self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
				if (self->jl_tok == ')') {
					JITLexer_Yield(self);
					rhs  = DeeTuple_NewEmpty();
					kwds = NULL;
				} else {
					rhs = JITLexer_EvalArgumentList(self, &kwds);
					if (ISERR(rhs)) {
err_function_lvalue:
						JITLValue_Fini(&function_lvalue);
						goto err;
					}
					ASSERT(rhs != JIT_LVALUE);
					if (self->jl_tok != ')') {
						syn_call_expected_rparen_after_call(self);
						Dee_XDecref(kwds);
						DECREF(rhs);
						goto err_function_lvalue;
					}
					JITLexer_Yield(self);
				}
				lhs = JITLValue_CallValue(&function_lvalue, self->jl_context, rhs, kwds);
				Dee_XDecref(kwds);
				Dee_Decref(rhs);
				JITLValue_Fini(&function_lvalue);
				if unlikely(!lhs) {
					JITLexer_ErrorTrace(self, pos);
					goto err;
				}
			} else
#endif /* !JIT_EVAL */
			{
				if (self->jl_tok == ')') {
					JITLexer_Yield(self);
#ifdef JIT_EVAL
					rhs  = DeeTuple_NewEmpty();
					kwds = NULL;
#endif /* JIT_EVAL */
				} else {
#ifdef JIT_EVAL
					rhs = JITLexer_EvalArgumentList(self, &kwds);
					if (ISERR(rhs))
						goto err_r;
					ASSERT(rhs != JIT_LVALUE);
					if (self->jl_tok == ')') {
						JITLexer_Yield(self);
					} else {
						syn_call_expected_rparen_after_call(self);
						Dee_XDecref(kwds);
						DECREF(rhs);
						goto err_r;
					}
#else /* JIT_EVAL */
					if (JITLexer_SkipPair(self, '(', ')'))
						goto err_r;
#endif /* !JIT_EVAL */
				}
#ifdef JIT_EVAL
				{
					DREF DeeObject *call_result;
					call_result = DeeObject_CallTupleKw(lhs, rhs, kwds);
					Dee_XDecref(kwds);
					Dee_Decref(rhs);
					if unlikely(!call_result)
						goto err_r_invoke;
					Dee_Decref(lhs);
					lhs = call_result;
				}
#endif /* JIT_EVAL */
			}
		}	break;

		case JIT_KEYWORD:
			if (JITLexer_ISTOK(self, "pack")) {
				/* TODO */
				DeeError_NOTIMPLEMENTED();
				goto err_r;
			}
			goto done;

		default:
			goto done;
		}
	}
done:
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_r_invoke:
	JITLexer_ErrorTrace(self, pos);
#endif /* JIT_EVAL */
err_r:
	DECREF_MAYBE_LVALUE(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}




DEFINE_PRIMARY(Unary) {
	RETURN_TYPE result = CALL_PRIMARY(UnaryHead);
	if (ISOK(result) && TOKEN_IS_UNARY(self))
		result = CALL_SECONDARY(UnaryOperand, result);
	return result;
}

DEFINE_PRIMARY(Prod) {
	RETURN_TYPE result = CALL_PRIMARY(Unary);
	if (ISOK(result) && TOKEN_IS_PROD(self))
		result = CALL_SECONDARY(ProdOperand, result);
	return result;
}

DEFINE_PRIMARY(Sum) {
	RETURN_TYPE result = CALL_PRIMARY(Prod);
	if (ISOK(result) && TOKEN_IS_SUM(self))
		result = CALL_SECONDARY(SumOperand, result);
	return result;
}

DEFINE_PRIMARY(Shift) {
	RETURN_TYPE result = CALL_PRIMARY(Sum);
	if (ISOK(result) && TOKEN_IS_SHIFT(self))
		result = CALL_SECONDARY(ShiftOperand, result);
	return result;
}

DEFINE_PRIMARY(Cmp) {
	RETURN_TYPE result = CALL_PRIMARY(Shift);
	if (ISOK(result) && TOKEN_IS_CMP(self))
		result = CALL_SECONDARY(CmpOperand, result);
	return result;
}

DEFINE_PRIMARY(CmpEQ) {
	RETURN_TYPE result = CALL_PRIMARYF(Cmp, flags | JITLEXER_EVAL_FALLOWISBOUND);
	if (ISOK(result) && TOKEN_IS_CMPEQ(self))
		result = CALL_SECONDARY(CmpEQOperand, result);
	return result;
}

DEFINE_PRIMARY(And) {
	RETURN_TYPE result = CALL_PRIMARY(CmpEQ);
	if (ISOK(result) && TOKEN_IS_AND(self))
		result = CALL_SECONDARY(AndOperand, result);
	return result;
}

DEFINE_PRIMARY(Xor) {
	RETURN_TYPE result = CALL_PRIMARY(And);
	if (ISOK(result) && TOKEN_IS_XOR(self))
		result = CALL_SECONDARY(XorOperand, result);
	return result;
}

DEFINE_PRIMARY(Or) {
	RETURN_TYPE result = CALL_PRIMARY(Xor);
	if (ISOK(result) && TOKEN_IS_OR(self))
		result = CALL_SECONDARY(OrOperand, result);
	return result;
}

DEFINE_PRIMARY(As) {
	RETURN_TYPE result = CALL_PRIMARY(Or);
	if (ISOK(result) && TOKEN_IS_AS(self))
		result = CALL_SECONDARY(AsOperand, result);
	return result;
}

DEFINE_PRIMARY(Land) {
	RETURN_TYPE result = CALL_PRIMARY(As);
	if (ISOK(result) && TOKEN_IS_LAND(self))
		result = CALL_SECONDARY(LandOperand, result);
	return result;
}

DEFINE_PRIMARY(Lor) {
	RETURN_TYPE result = CALL_PRIMARY(Land);
	if (ISOK(result) && TOKEN_IS_LOR(self))
		result = CALL_SECONDARY(LorOperand, result);
	return result;
}

DEFINE_PRIMARY(Cond) {
	RETURN_TYPE result = CALL_PRIMARY(Lor);
	if (ISOK(result) && TOKEN_IS_COND(self))
		result = CALL_SECONDARY(CondOperand, result);
	return result;
}

DEFINE_PRIMARY(Assign) {
#ifdef __OPTIMIZE_SIZE__
	RETURN_TYPE result;
	result = CALL_PRIMARYF(Cond, flags | JITLEXER_EVAL_FALLOWINPLACE);
	if (ISOK(result) && TOKEN_IS_ASSIGN(self))
		result = CALL_SECONDARY(AssignOperand, result);
	return result;
#elif 1
	RETURN_TYPE result;
	result = CALL_PRIMARYF(UnaryHead,
	                       flags |
	                       JITLEXER_EVAL_FALLOWINPLACE |
	                       JITLEXER_EVAL_FALLOWISBOUND);
	if (ISERR(result))
		goto done;
	switch (self->jl_tok) {

	case JIT_KEYWORD:
		if (JITLexer_ISTOK(self, "is"))
			goto case_cmpeq;
		if (JITLexer_ISTOK(self, "in"))
			goto case_cmpeq;
		if (JITLexer_ISTOK(self, "as"))
			goto case_as;
		if (JITLexer_ISTOK(self, "pack"))
			goto case_unary;
		break;

	CASE_TOKEN_IS_UNARY:
case_unary:
		result = CALL_SECONDARY(UnaryOperand, result);
		if (ISERR(result))
			goto done;
		if (TOKEN_IS_PROD(self)) {
	CASE_TOKEN_IS_PROD:
			result = CALL_SECONDARY(ProdOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_SUM(self)) {
	CASE_TOKEN_IS_SUM:
			result = CALL_SECONDARY(SumOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_SHIFT(self)) {
	CASE_TOKEN_IS_SHIFT:
			result = CALL_SECONDARY(ShiftOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_CMP(self)) {
	CASE_TOKEN_IS_CMP:
			result = CALL_SECONDARY(CmpOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_CMPEQ(self)) {
	CASE_TOKEN_IS_CMPEQ:
case_cmpeq:
			result = CALL_SECONDARY(CmpEQOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_AND(self)) {
	CASE_TOKEN_IS_AND:
			result = CALL_SECONDARY(AndOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_XOR(self)) {
	CASE_TOKEN_IS_XOR:
			result = CALL_SECONDARY(XorOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_OR(self)) {
	CASE_TOKEN_IS_OR:
			result = CALL_SECONDARY(OrOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_AS(self)) {
/*CASE_TOKEN_IS_AS:*/
case_as:
			result = CALL_SECONDARY(AsOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_LAND(self)) {
	CASE_TOKEN_IS_LAND:
			result = CALL_SECONDARY(LandOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_LOR(self)) {
	CASE_TOKEN_IS_LOR:
			result = CALL_SECONDARY(LorOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_COND(self)) {
	CASE_TOKEN_IS_COND:
			result = CALL_SECONDARY(CondOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_ASSIGN(self)) {
	CASE_TOKEN_IS_ASSIGN:
			result = CALL_SECONDARY(AssignOperand, result);
			if (ISERR(result))
				goto done;
		}
		break;

	default: break;
	}
done:
	return result;
#endif
}

/* With the current token one of the unary operator symbols, consume
* it and parse the second operand before returning the combination */
DEFINE_SECONDARY(ProdOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	IF_EVAL(unsigned int cmd = self->jl_tok;)
	ASSERT(TOKEN_IS_PROD(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		rhs = CALL_PRIMARY(Unary);
		if (ISERR(rhs))
			goto err_r;
		LOAD_LVALUE(rhs, err_r);
#ifdef JIT_EVAL
		switch (cmd) {

		case '*':
			merge = DeeObject_Mul(lhs, rhs);
			break;

		case '/':
			merge = DeeObject_Div(lhs, rhs);
			break;

		case '%':
			merge = DeeObject_Mod(lhs, rhs);
			break;

		case TOK_POW:
			merge = DeeObject_Pow(lhs, rhs);
			break;

		default: __builtin_unreachable();
		}
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
		if (!TOKEN_IS_PROD(self))
			break;
		IF_EVAL(cmd = self->jl_tok;)
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(SumOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	unsigned int cmd = self->jl_tok;
	ASSERT(TOKEN_IS_SUM(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		if (self->jl_tok == TOK_DOTS && cmd == '+') {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			merge = DeeSeq_Sum(lhs);
			if unlikely(!merge)
				goto err_invoke_norhs;
			Dee_Decref(lhs);
			lhs = merge;
#endif /* JIT_EVAL */
		} else {
			rhs = CALL_PRIMARY(Prod);
			if (ISERR(rhs))
				goto err_r;
			LOAD_LVALUE(rhs, err_r);
#ifdef JIT_EVAL
			switch (cmd) {
			case '+':
				merge = DeeObject_Add(lhs, rhs);
				break;
			case '-':
				merge = DeeObject_Sub(lhs, rhs);
				break;
			default: __builtin_unreachable();
			}
			if unlikely(!merge)
				goto err_invoke;
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = merge;
#endif /* JIT_EVAL */
		}
		cmd = self->jl_tok;
		if (!TOKEN_IS_SUM(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	DECREF(rhs);
err_invoke_norhs:
	JITLexer_ErrorTrace(self, pos);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(ShiftOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	IF_EVAL(unsigned int cmd = self->jl_tok;)
	ASSERT(TOKEN_IS_SHIFT(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		rhs = CALL_PRIMARY(Sum);
		if (ISERR(rhs))
			goto err_r;
		LOAD_LVALUE(rhs, err_r);
#ifdef JIT_EVAL
		switch (cmd) {
		case TOK_SHL:
			merge = DeeObject_Shl(lhs, rhs);
			break;
		case TOK_SHR:
			merge = DeeObject_Shr(lhs, rhs);
			break;
		default: __builtin_unreachable();
		}
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
		if (!TOKEN_IS_SHIFT(self))
			break;
		IF_EVAL(cmd = self->jl_tok;)
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(CmpOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	unsigned int cmd = self->jl_tok;
	ASSERT(TOKEN_IS_CMP(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		if (self->jl_tok == TOK_DOTS && (cmd == '<' || cmd == '>')) {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			merge = cmd == '<'
			        ? DeeSeq_Min(lhs)
			        : DeeSeq_Max(lhs);
			if unlikely(!merge)
				goto err_invoke_norhs;
			Dee_Decref(lhs);
			lhs = merge;
#endif /* JIT_EVAL */
		} else {
			rhs = CALL_PRIMARY(Shift);
			if (ISERR(rhs))
				goto err_r;
			LOAD_LVALUE(rhs, err_r);
#ifdef JIT_EVAL
			switch (cmd) {
			case TOK_LOWER:
				merge = DeeObject_CmpLo(lhs, rhs);
				break;
			case TOK_LOWER_EQUAL:
				merge = DeeObject_CmpLe(lhs, rhs);
				break;
			case TOK_GREATER:
				merge = DeeObject_CmpGr(lhs, rhs);
				break;
			case TOK_GREATER_EQUAL:
				merge = DeeObject_CmpGe(lhs, rhs);
				break;
			default: __builtin_unreachable();
			}
			if unlikely(!merge)
				goto err_invoke;
			Dee_Decref(rhs);
			Dee_Decref(lhs);
			lhs = merge;
#endif /* JIT_EVAL */
		}
		cmd = self->jl_tok;
		if (!TOKEN_IS_CMP(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	DECREF(rhs);
err_invoke_norhs:
	JITLexer_ErrorTrace(self, pos);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(CmpEQOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	IF_EVAL(unsigned int cmd = self->jl_tok;)
	ASSERT(TOKEN_IS_CMPEQ(self));
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		if (self->jl_tok == '!') {
			bool inverted = false;
			do {
				JITLexer_Yield(self);
				inverted = !inverted;
			} while (self->jl_tok == '!');
			if (JITLexer_ISKWD(self, "is")) {
				JITLexer_Yield(self);
				if (JITLexer_ISKWD(self, "bound")) {
					/* Suffix expression */
#ifdef JIT_EVAL
					int error;
					if (lhs != JIT_LVALUE) {
						syn_bound_cannot_test(self);
						goto err_r;
					}
					error = JITLValue_IsBound(&self->jl_lvalue, self->jl_context);
					JITLValue_Fini(&self->jl_lvalue);
					JITLValue_Init(&self->jl_lvalue);
					if unlikely(error < 0)
						goto err;
					lhs = DeeBool_For((error != 0) ^ inverted);
					Dee_Incref(lhs);
#endif /* JIT_EVAL */
					JITLexer_Yield(self);
					goto continue_expr;
				}
				LOAD_LVALUE(lhs, err);
				rhs = CALL_PRIMARYF(Cmp, flags | JITLEXER_EVAL_FALLOWISBOUND);
				if (ISERR(rhs))
					goto err_r;
#ifdef JIT_EVAL
				LOAD_LVALUE(rhs, err_r);
				{
					bool is_instance;
					if (DeeNone_Check(rhs)) {
						is_instance = DeeNone_Check(lhs);
					} else if (DeeSuper_Check(lhs)) {
						is_instance = DeeType_Implements(DeeSuper_TYPE(lhs), (DeeTypeObject *)rhs);
					} else {
						is_instance = DeeObject_Implements(lhs, (DeeTypeObject *)rhs);
					}
					Dee_Decref(rhs);
					Dee_Decref(lhs);
					lhs = DeeBool_For(inverted ^ is_instance);
					Dee_Incref(lhs);
				}
#endif /* JIT_EVAL */
				goto continue_expr;
			}
			LOAD_LVALUE(lhs, err);
			if (JITLexer_ISKWD(self, "in")) {
				JITLexer_Yield(self);
				rhs = CALL_PRIMARYF(Cmp, flags | JITLEXER_EVAL_FALLOWISBOUND);
				if (ISERR(rhs))
					goto err_r;
#ifdef JIT_EVAL
				LOAD_LVALUE(rhs, err_r);
				{
					merge = DeeObject_Contains(rhs, lhs);
					if unlikely(!merge)
						goto err_rhs;
					Dee_Decref(rhs);
					Dee_Decref(lhs);
					lhs = merge;
					if (inverted) {
						int b = DeeObject_Bool(lhs);
						if unlikely(b < 0)
							goto err_r;
						Dee_Decref(lhs);
						lhs = DeeBool_For(!b);
						Dee_Incref(lhs);
					}
				}
#endif /* JIT_EVAL */
				goto continue_expr;
			}
			syn_isin_expected_is_or_in_after_exclaim(self);
			goto err_r;
		}
		if (JITLexer_ISKWD(self, "is")) {
			JITLexer_Yield(self);
			if (JITLexer_ISKWD(self, "bound")) {
				JITLexer_Yield(self);
#ifdef JIT_EVAL
				{
					int error;
					if (lhs != JIT_LVALUE) {
						syn_bound_cannot_test(self);
						goto err_r;
					}
					error = JITLValue_IsBound(&self->jl_lvalue, self->jl_context);
					JITLValue_Fini(&self->jl_lvalue);
					JITLValue_Init(&self->jl_lvalue);
					if unlikely(error < 0)
						goto err;
					lhs = DeeBool_For(error);
					Dee_Incref(lhs);
				}
#endif /* JIT_EVAL */
			} else {
				LOAD_LVALUE(lhs, err);
				rhs = CALL_PRIMARYF(Cmp, flags | JITLEXER_EVAL_FALLOWISBOUND);
				if (ISERR(rhs))
					goto err_r;
#ifdef JIT_EVAL
				LOAD_LVALUE(rhs, err_r);
				{
					bool is_instance;
					if (DeeNone_Check(rhs)) {
						is_instance = DeeNone_Check(lhs);
					} else if (DeeSuper_Check(lhs)) {
						is_instance = DeeType_Implements(DeeSuper_TYPE(lhs), (DeeTypeObject *)rhs);
					} else {
						is_instance = DeeObject_Implements(lhs, (DeeTypeObject *)rhs);
					}
					Dee_Decref(rhs);
					Dee_Decref(lhs);
					lhs = DeeBool_For(is_instance);
					Dee_Incref(lhs);
				}
#endif /* JIT_EVAL */
			}
			goto continue_expr;
		} else {
			JITLexer_Yield(self);
		}
		LOAD_LVALUE(lhs, err);
#ifdef JIT_EVAL
		if (cmd == TOK_QMARK_QMARK && !DeeNone_Check(lhs)) {
			/* Skip operand expression. */
			if unlikely(JITLexer_SkipCmp(self, flags | JITLEXER_EVAL_FALLOWISBOUND))
				goto err_r;
			goto continue_expr;
		}
#endif /* JIT_EVAL */
		rhs = CALL_PRIMARYF(Cmp, flags | JITLEXER_EVAL_FALLOWISBOUND);
		if (ISERR(rhs))
			goto err_r;
#ifdef JIT_EVAL
		LOAD_LVALUE(rhs, err_r);
		switch (cmd) {
		case TOK_EQUAL:
			merge = DeeObject_CmpEq(lhs, rhs);
			break;
		case TOK_NOT_EQUAL:
			merge = DeeObject_CmpNe(lhs, rhs);
			break;
		case TOK_EQUAL3:
			merge = DeeBool_For(lhs == rhs);
			Dee_Incref(merge);
			break;
		case TOK_NOT_EQUAL3:
			merge = DeeBool_For(lhs != rhs);
			Dee_Incref(merge);
			break;
		case TOK_QMARK_QMARK:
			ASSERT(DeeNone_Check(lhs));
			Dee_DecrefNokill(Dee_None);
			lhs = rhs; /* Inherit reference */
			goto continue_expr;
		case JIT_KEYWORD:
			merge = DeeObject_Contains(rhs, lhs);
			break;
		default: __builtin_unreachable();
		}
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
continue_expr:
		if (!TOKEN_IS_CMPEQ(self))
			break;
		IF_EVAL(cmd = self->jl_tok;)
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
err_rhs:
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(AndOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_AND(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		rhs = CALL_PRIMARY(CmpEQ);
		if (ISERR(rhs))
			goto err_r;
#ifdef JIT_EVAL
		LOAD_LVALUE(rhs, err_r);
		merge = DeeObject_And(lhs, rhs);
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
		if (!TOKEN_IS_AND(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(XorOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_XOR(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		rhs = CALL_PRIMARY(And);
		if (ISERR(rhs))
			goto err_r;
#ifdef JIT_EVAL
		LOAD_LVALUE(rhs, err_r);
		merge = DeeObject_Xor(lhs, rhs);
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
		if (!TOKEN_IS_XOR(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(OrOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_OR(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		rhs = CALL_PRIMARY(Xor);
		if (ISERR(rhs))
			goto err_r;
#ifdef JIT_EVAL
		LOAD_LVALUE(rhs, err_r);
		merge = DeeObject_Or(lhs, rhs);
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
		if (!TOKEN_IS_OR(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(AsOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(DREF DeeObject * merge;)
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_AS(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		rhs = CALL_PRIMARY(Or);
		if (ISERR(rhs))
			goto err_r;
#ifdef JIT_EVAL
		LOAD_LVALUE(rhs, err_r);
		merge = DeeSuper_New((DeeTypeObject *)rhs, lhs);
		if unlikely(!merge)
			goto err_invoke;
		Dee_Decref(rhs);
		Dee_Decref(lhs);
		lhs = merge;
#endif /* JIT_EVAL */
		if (!TOKEN_IS_AS(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
	DECREF(rhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(LandOperand) {
#ifndef JIT_EVAL
	RETURN_TYPE rhs;
#endif /* !JIT_EVAL */
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_LAND(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		if (self->jl_tok == TOK_DOTS) {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			{
				int b = DeeSeq_All(lhs);
				if (b < 0)
					goto err_invoke_norhs;
				Dee_Decref(lhs);
				lhs = DeeBool_For(b);
				Dee_Incref(lhs);
			}
#endif /* JIT_EVAL */
			goto continue_expr;
		}
#ifdef JIT_EVAL
		{
			int b = DeeObject_Bool(lhs);
			if (b < 0)
				goto err_invoke_norhs;
			Dee_Decref(lhs);
			if (b) {
				lhs = CALL_PRIMARY(As);
				if (ISERR(lhs))
					goto err;
				LOAD_LVALUE(lhs, err);
				b = DeeObject_Bool(lhs);
				if (b < 0)
					goto err_invoke_norhs;
				Dee_Decref(lhs);
				lhs = DeeBool_For(b);
				Dee_Incref(lhs);
			} else {
				if unlikely(JITLexer_SkipAs(self, flags))
					goto err;
				lhs = DeeBool_NewFalse();
			}
		}
#else /* JIT_EVAL */
		rhs = CALL_PRIMARY(As);
		if (ISERR(rhs))
			goto err_r;
#endif /* !JIT_EVAL */
continue_expr:
		if (!TOKEN_IS_LAND(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
/*err_invoke:*/
	/*DECREF(rhs);*/
err_invoke_norhs:
	JITLexer_ErrorTrace(self, pos);
#else /* JIT_EVAL */
err_r:
#endif /* !JIT_EVAL */
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(LorOperand) {
#ifndef JIT_EVAL
	RETURN_TYPE rhs;
#endif /* !JIT_EVAL */
	IF_EVAL(unsigned char const *pos;)
	ASSERT(TOKEN_IS_LOR(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
		if (self->jl_tok == TOK_DOTS) {
			JITLexer_Yield(self);
#ifdef JIT_EVAL
			{
				int b = DeeSeq_Any(lhs);
				if (b < 0)
					goto err_invoke_norhs;
				Dee_Decref(lhs);
				lhs = DeeBool_For(b);
				Dee_Incref(lhs);
			}
#endif /* JIT_EVAL */
			goto continue_expr;
		}
#ifdef JIT_EVAL
		{
			int b = DeeObject_Bool(lhs);
			if (b < 0)
				goto err_invoke_norhs;
			Dee_Decref(lhs);
			if (b) {
				if unlikely(JITLexer_SkipLand(self, flags))
					goto err;
				lhs = DeeBool_NewTrue();
			} else {
				lhs = CALL_PRIMARY(As);
				if (ISERR(lhs))
					goto err;
				LOAD_LVALUE(lhs, err);
				b = DeeObject_Bool(lhs);
				if (b < 0)
					goto err_invoke_norhs;
				Dee_Decref(lhs);
				lhs = DeeBool_For(b);
				Dee_Incref(lhs);
			}
		}
#else /* JIT_EVAL */
		rhs = CALL_PRIMARY(As);
		if (ISERR(rhs))
			goto err_r;
#endif /* !JIT_EVAL */
continue_expr:
		if (!TOKEN_IS_LOR(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
/*err_invoke:*/
	/*DECREF(rhs);*/
err_invoke_norhs:
	JITLexer_ErrorTrace(self, pos);
#else /* JIT_EVAL */
err_r:
#endif /* !JIT_EVAL */
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(CondOperand) {
	IF_EVAL(unsigned char const *pos;)
	/* >>  x ? y : z // >> x ? y : z
	 * >>  x ?: z    // >> x ? x : z
	 * >> (x ? y : ) // >> x ? y : x
	 * >> (x ? y)    // >> x ? y : none
	 */
	ASSERT(TOKEN_IS_COND(self));
	LOAD_LVALUE(lhs, err);
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		{
			int b = DeeObject_Bool(lhs);
			if unlikely(b < 0)
				goto err_invoke;
			if (self->jl_tok == ':') {
				/* Missing true-branch. */
				JITLexer_Yield(self);
				if (b) {
					/* true?:<SKIP> */
					if unlikely(JITLexer_SkipCond(self, flags))
						goto err_r;
				} else {
					/* false?:<EVAL> */
					Dee_Decref(lhs);
					lhs = JITLexer_EvalCond(self, flags);
					if unlikely(!lhs)
						goto err;
					LOAD_LVALUE(lhs, err);
				}
				goto continue_expr;
			}
			if (b) {
				/* true?<EXPR>[:[<SKIP>]] */
				Dee_Decref(lhs);
				lhs = JITLexer_EvalCond(self, flags);
				if unlikely(!lhs)
					goto err;
				LOAD_LVALUE(lhs, err);
				if (self->jl_tok == ':') {
					JITLexer_Yield(self);
					if (JITLexer_MaybeExpressionBegin(self)) {
						if unlikely(JITLexer_SkipCond(self, flags))
							goto err_r;
					}
				}
			} else {
				/* false?<SKIP>[:[<EXPR>]] */
				if unlikely(JITLexer_SkipCond(self, flags))
					goto err_r;
				if (self->jl_tok != ':') {
					Dee_Decref(lhs);
					lhs = DeeNone_NewRef();
				} else {
					JITLexer_Yield(self);
					if (JITLexer_MaybeExpressionBegin(self)) {
						Dee_Decref(lhs);
						lhs = JITLexer_EvalCond(self, flags);
						if unlikely(!lhs)
							goto err;
						LOAD_LVALUE(lhs, err);
					}
				}
			}
		}
#else /* JIT_EVAL */
		if (self->jl_tok == ':') {
			/* Missing true-branch. */
			JITLexer_Yield(self);
			if unlikely(JITLexer_SkipCond(self, flags))
				goto err_r;
			goto continue_expr;
		}
		if unlikely(JITLexer_SkipCond(self, flags))
			goto err_r;
		if (self->jl_tok == ':') {
			/* Parse the false-branch. */
			JITLexer_Yield(self);
			if (JITLexer_MaybeExpressionBegin(self)) {
				if unlikely(JITLexer_SkipCond(self, flags))
					goto err_r;
			} else {
				/* Missing false-branch. */
			}
		}
#endif /* !JIT_EVAL */
continue_expr:
		if (!TOKEN_IS_COND(self))
			break;
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	JITLexer_ErrorTrace(self, pos);
#endif /* JIT_EVAL */
err_r:
	DECREF(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}


#ifndef INPLACE_FOPS_DEFINED
#define INPLACE_FOPS_DEFINED 1
#define TOK_INPLACE_MIN TOK_ADD_EQUAL
#define OPERATOR_INPLACE_MIN OPERATOR_INPLACE_ADD
STATIC_ASSERT((TOK_ADD_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_ADD - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SUB_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SUB - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_MUL_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MUL - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_DIV_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_DIV - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_MOD_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_MOD - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SHL_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHL - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_SHR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_SHR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_AND_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_AND - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_OR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_OR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_XOR_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_XOR - OPERATOR_INPLACE_MIN));
STATIC_ASSERT((TOK_POW_EQUAL - TOK_INPLACE_MIN) == (OPERATOR_INPLACE_POW - OPERATOR_INPLACE_MIN));
PRIVATE Dee_operator_t const inplace_fops[] = {
	/* [TOK_ADD_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_ADD,
	/* [TOK_SUB_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SUB,
	/* [TOK_MUL_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_MUL,
	/* [TOK_DIV_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_DIV,
	/* [TOK_MOD_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_MOD,
	/* [TOK_SHL_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SHL,
	/* [TOK_SHR_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_SHR,
	/* [TOK_AND_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_AND,
	/* [TOK_OR_EQUAL  - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_OR,
	/* [TOK_XOR_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_XOR,
	/* [TOK_POW_EQUAL - TOK_INPLACE_MIN] = */ OPERATOR_INPLACE_POW
};
#endif /* !INPLACE_FOPS_DEFINED */


DEFINE_SECONDARY(AssignOperand) {
	RETURN_TYPE rhs;
	IF_EVAL(int error;)
	IF_EVAL(unsigned char const *pos;)
	IF_EVAL(unsigned int cmd = self->jl_tok;)
	ASSERT(TOKEN_IS_ASSIGN(self));
	for (;;) {
		IF_EVAL(pos = self->jl_tokstart;)
		if (self->jl_tok == TOK_COLON_EQUAL) {
			LOAD_LVALUE(lhs, err);
			JITLexer_Yield(self);
			rhs = CALL_PRIMARYF(Cond, flags | JITLEXER_EVAL_FALLOWINPLACE);
			if (ISERR(rhs))
				goto err_r;
#ifdef JIT_EVAL
			LOAD_LVALUE(rhs, err_r);
			error = DeeObject_Assign(lhs, rhs);
			Dee_Decref(rhs);
			if unlikely(error)
				goto err_invoke;
#endif /* JIT_EVAL */
		} else {
#ifdef JIT_EVAL
			JITLValue lhs_lvalue;
			if (lhs != JIT_LVALUE) {
				err_cannot_invoke_inplace(lhs, inplace_fops[self->jl_tok - TOK_INPLACE_MIN]);
				goto err_invoke_norhs;
			}
			/* Store the L-value expression for LHS locally, so we can load another expression for RHS*/
			memcpy(&lhs_lvalue, &self->jl_lvalue, sizeof(JITLValue));
			JITLValue_Init(&self->jl_lvalue);
			JITLexer_Yield(self);
			rhs = CALL_PRIMARYF(Cond, flags | JITLEXER_EVAL_FALLOWINPLACE);
			if (ISERR(rhs)) {
err_lvalue:
				JITLValue_Fini(&lhs_lvalue);
				goto err;
			}
			LOAD_LVALUE(rhs, err_lvalue);
			/* Load the L-value expression target object. */
			lhs = JITLValue_GetValue(&lhs_lvalue, self->jl_context);
			if unlikely(!lhs) {
				Dee_Decref(rhs);
				self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
				goto err_lvalue;
			}
			switch (cmd) {

			case TOK_ADD_EQUAL:
				error = DeeObject_InplaceAdd((DeeObject **)&lhs, rhs);
				break;

			case TOK_SUB_EQUAL:
				error = DeeObject_InplaceSub((DeeObject **)&lhs, rhs);
				break;

			case TOK_MUL_EQUAL:
				error = DeeObject_InplaceMul((DeeObject **)&lhs, rhs);
				break;

			case TOK_DIV_EQUAL:
				error = DeeObject_InplaceDiv((DeeObject **)&lhs, rhs);
				break;

			case TOK_MOD_EQUAL:
				error = DeeObject_InplaceMod((DeeObject **)&lhs, rhs);
				break;

			case TOK_SHL_EQUAL:
				error = DeeObject_InplaceShl((DeeObject **)&lhs, rhs);
				break;

			case TOK_SHR_EQUAL:
				error = DeeObject_InplaceShr((DeeObject **)&lhs, rhs);
				break;

			case TOK_AND_EQUAL:
				error = DeeObject_InplaceAnd((DeeObject **)&lhs, rhs);
				break;

			case TOK_OR_EQUAL:
				error = DeeObject_InplaceOr((DeeObject **)&lhs, rhs);
				break;

			case TOK_XOR_EQUAL:
				error = DeeObject_InplaceXor((DeeObject **)&lhs, rhs);
				break;

			case TOK_POW_EQUAL:
				error = DeeObject_InplacePow((DeeObject **)&lhs, rhs);
				break;

			default: __builtin_unreachable();
			}
			if unlikely(error) {
				JITLValue_Fini(&lhs_lvalue);
				goto err_invoke;
			}
			Dee_Decref(rhs);
			/* Save back the l-value expression result object. */
			error = JITLValue_SetValue(&lhs_lvalue, self->jl_context, lhs);
			JITLValue_Fini(&lhs_lvalue);
			if unlikely(error)
				goto err_r;
#else /* JIT_EVAL */
			JITLexer_Yield(self);
			rhs = CALL_PRIMARYF(Cond, flags | JITLEXER_EVAL_FALLOWINPLACE);
			if (ISERR(rhs))
				goto err_r;
#endif /* !JIT_EVAL */
		}
		if (!TOKEN_IS_ASSIGN(self))
			break;
		IF_EVAL(cmd = self->jl_tok;)
	}
	return LHS_OR_OK;
#ifdef JIT_EVAL
err_invoke:
	DECREF(rhs);
err_invoke_norhs:
	JITLexer_ErrorTrace(self, pos);
#endif /* JIT_EVAL */
err_r:
	DECREF_MAYBE_LVALUE(lhs);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

#ifndef DeeTuple_Append_DEFINED
#define DeeTuple_Append_DEFINED
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
DeeTuple_Append(/*inherit(on_success)*/ DREF DeeTupleObject *__restrict self,
                DeeObject *__restrict item) {
	DREF DeeTupleObject *result;
	size_t index;
	/* Must increase the tuple's size. */
	index  = DeeTuple_SIZE(self);
	result = DeeTuple_ResizeUninitialized(self, index + 1);
	if likely(result) {
		DeeTuple_SET(result, index, item);
		Dee_Incref(item);
	}
	return result;
}
#endif /* !DeeTuple_Append_DEFINED */


DEFINE_SECONDARY(CommaTupleOperand) {
#ifdef JIT_EVAL
	DREF DeeTupleObject *result;
	DREF DeeTupleObject *new_result;
	result = (DREF DeeTupleObject *)DeeTuple_NewEmpty();
#else /* JIT_EVAL */
	RETURN_TYPE result;
	int lhs;
	result = 0;
#endif /* !JIT_EVAL */
	(void)flags;
	LOAD_LVALUE(lhs, err_r);
again:
	switch (self->jl_tok) {

	case TOK_DOTS:
		/* Expand expression */
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		result = (DREF DeeTupleObject *)DeeTuple_ConcatInherited((DREF DeeObject *)result, lhs);
		if unlikely(!result)
			goto err;
#endif /* JIT_EVAL */
		if (self->jl_tok != ',')
			break;
#ifdef JIT_EVAL
		__IF0 {
		case ',':
			new_result = DeeTuple_Append(result, lhs);
			if unlikely(!new_result)
				goto err_r_lhs;
			Dee_Decref(lhs);
			result = new_result;
		}
#else /* JIT_EVAL */
		ATTR_FALLTHROUGH
	case ',':
#endif /* !JIT_EVAL */
		JITLexer_Yield(self);
		/* Allow  */
		if (!JITLexer_MaybeExpressionBegin(self))
			break;
		/* Load the next expression. */
		lhs = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
		if (ISERR(lhs))
			goto err_r;
		LOAD_LVALUE(lhs, err_r);
		goto again;

	default:
#ifdef JIT_EVAL
		new_result = DeeTuple_Append(result, lhs);
		if unlikely(!new_result)
			goto err_r_lhs;
		Dee_Decref(lhs);
		result = new_result;
#endif /* JIT_EVAL */
		break;
	}
	return (RETURN_TYPE)result;
#ifdef JIT_EVAL
err_r_lhs:
	DECREF(lhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(result);
#ifdef JIT_EVAL
err:
#endif /* JIT_EVAL */
	return ERROR;
}

DEFINE_SECONDARY(CommaListOperand) {
	RETURN_TYPE result;
#ifdef JIT_EVAL
	result = DeeList_New();
	if unlikely(!result) {
		Dee_Decref(lhs);
		return NULL;
	}
#else /* JIT_EVAL */
	int lhs;
	result = 0;
#endif /* !JIT_EVAL */
	(void)flags;
	LOAD_LVALUE(lhs, err_r);
again:
	switch (self->jl_tok) {

	case TOK_DOTS:
		/* Expand expression */
		JITLexer_Yield(self);
#ifdef JIT_EVAL
		if (DeeList_AppendSequence(result, lhs))
			goto err_lhs;
		Dee_Decref(lhs);
#endif /* JIT_EVAL */
		if (self->jl_tok != ',')
			break;
#ifdef JIT_EVAL
		__IF0 {
		case ',':
			if (DeeList_Append(result, lhs))
				goto err_lhs;
			Dee_Decref(lhs);
		}
#else /* JIT_EVAL */
		ATTR_FALLTHROUGH
	case ',':
#endif /* !JIT_EVAL */
		JITLexer_Yield(self);
		/* Allow  */
		if (!JITLexer_MaybeExpressionBegin(self))
			break;
		/* Load the next expression. */
		lhs = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
		if (ISERR(lhs))
			goto err_r;
		LOAD_LVALUE(lhs, err_r);
		goto again;

	default:
#ifdef JIT_EVAL
		if (DeeList_Append(result, lhs))
			goto err_lhs;
		Dee_Decref(lhs);
#endif /* JIT_EVAL */
		break;
	}
	return result;
#ifdef JIT_EVAL
err_lhs:
	DECREF(lhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(result);
	return ERROR;
}


DEFINE_SECONDARY(CommaDictOperand) {
	RETURN_TYPE result;
	RETURN_TYPE value;
#ifdef JIT_EVAL
	result = DeeDict_New();
	if unlikely(!result) {
		Dee_Decref(lhs);
		return NULL;
	}
#else /* JIT_EVAL */
	int lhs;
	result = 0;
#endif /* !JIT_EVAL */
	(void)flags;
	LOAD_LVALUE(lhs, err_r);
again:
	JITLexer_Yield(self);
	value = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
	if (ISERR(value))
		goto err_lhs;
	LOAD_LVALUE(value, err_lhs);
#ifdef JIT_EVAL
	{
		int error = DeeObject_SetItem(result, lhs, value);
		Dee_Decref(value);
		if unlikely(error)
			goto err_lhs;
		Dee_Decref(lhs);
	}
#endif /* JIT_EVAL */
	if (self->jl_tok != ',')
		goto done;
	JITLexer_Yield(self);
	/* Parse the next key. */
	if (self->jl_tok == '.') {
		JITLexer_Yield(self);
		if (self->jl_tok != JIT_KEYWORD) {
			syn_brace_expected_keyword_after_dot(self);
			goto err_r;
		}
#ifdef JIT_EVAL
		lhs = DeeString_NewUtf8((char const *)self->jl_tokstart,
		                        JITLexer_TokLen(self),
		                        STRING_ERROR_FSTRICT);
		if unlikely(!lhs)
			goto err_r;
#endif /* JIT_EVAL */
		JITLexer_Yield(self);
		if (self->jl_tok != '=') {
			syn_brace_expected_equals_after_dot(self);
			goto err_lhs;
		}
	} else {
		if (!JITLexer_MaybeExpressionBegin(self))
			goto done;
		lhs = CALL_PRIMARYF(Expression, JITLEXER_EVAL_FSECONDARY);
		if (ISERR(lhs))
			goto err_r;
		LOAD_LVALUE(lhs, err_r);
		if (self->jl_tok != ':') {
			syn_brace_expected_colon_after_key(self);
			goto err_lhs;
		}
	}
	goto again;
done:
	return result;
err_lhs:
#ifdef JIT_EVAL
	DECREF(lhs);
#endif /* JIT_EVAL */
err_r:
	DECREF(result);
	return ERROR;
}

INTERN RETURN_TYPE DFCALL
FUNC(Operand)(JITLexer *__restrict self,
              IF_EVAL(/*inherit(always)*/ DREF DeeObject *__restrict lhs, ) unsigned int flags) {
	RETURN_TYPE result = LHS_OR_OK;
	(void)flags;
	switch (self->jl_tok) {

	case JIT_KEYWORD:
		if (JITLexer_ISTOK(self, "is"))
			goto case_cmpeq;
		if (JITLexer_ISTOK(self, "in"))
			goto case_cmpeq;
		if (JITLexer_ISTOK(self, "as"))
			goto case_as;
		if (JITLexer_ISTOK(self, "pack"))
			goto case_unary;
		break;

	CASE_TOKEN_IS_UNARY:
case_unary:
		result = CALL_SECONDARY(UnaryOperand, result);
		if (ISERR(result))
			goto done;
		if (TOKEN_IS_PROD(self)) {
	CASE_TOKEN_IS_PROD:
			result = CALL_SECONDARY(ProdOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_SUM(self)) {
	CASE_TOKEN_IS_SUM:
			result = CALL_SECONDARY(SumOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_SHIFT(self)) {
	CASE_TOKEN_IS_SHIFT:
			result = CALL_SECONDARY(ShiftOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_CMP(self)) {
	CASE_TOKEN_IS_CMP:
			result = CALL_SECONDARY(CmpOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_CMPEQ(self)) {
	CASE_TOKEN_IS_CMPEQ:
case_cmpeq:
			result = CALL_SECONDARY(CmpEQOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_AND(self)) {
	CASE_TOKEN_IS_AND:
			result = CALL_SECONDARY(AndOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_XOR(self)) {
	CASE_TOKEN_IS_XOR:
			result = CALL_SECONDARY(XorOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_OR(self)) {
	CASE_TOKEN_IS_OR:
			result = CALL_SECONDARY(OrOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_AS(self)) {
/*CASE_TOKEN_IS_AS:*/
case_as:
			result = CALL_SECONDARY(AsOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_LAND(self)) {
	CASE_TOKEN_IS_LAND:
			result = CALL_SECONDARY(LandOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_LOR(self)) {
	CASE_TOKEN_IS_LOR:
			result = CALL_SECONDARY(LorOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_COND(self)) {
	CASE_TOKEN_IS_COND:
			result = CALL_SECONDARY(CondOperand, result);
			if (ISERR(result))
				goto done;
		}
		if (TOKEN_IS_ASSIGN(self)) {
	CASE_TOKEN_IS_ASSIGN:
			result = CALL_SECONDARY(AssignOperand, result);
			if (ISERR(result))
				goto done;
		}
		break;
	default: break;
	}
done:
	return result;
}



DECL_END

#ifndef __INTELLISENSE__

#include "parser-impl-template-string.c.inl"
/**/

#include "parser-impl-class.c.inl"
/**/

#include "parser-impl-comma.c.inl"
/**/

#include "parser-impl-statement.c.inl"
/**/

#include "parser-impl-hybrid.c.inl"
/**/

#undef LOAD_LVALUE
#undef LOAD_LVALUE_OR_RETURN_ERROR
#undef IF_SKIP
#undef IF_EVAL
#undef DECREF
#undef DECREF_MAYBE_LVALUE
#undef SYNTAXERROR
#undef ERROR
#undef RETURN_TYPE
#undef LHS_OR_OK
#undef ISOK
#undef ISERR
#undef DEFINE_PRIMARY
#undef DEFINE_SECONDARY
#undef CALL_PRIMARY
#undef CALL_PRIMARYF
#undef CALL_SECONDARY
#undef JITLEXER_EVAL_FSECONDARY

#undef FUNC
#undef IFELSE
#undef JIT_EVAL
#undef JIT_SKIP
#endif /* !__INTELLISENSE__ */
