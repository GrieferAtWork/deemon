/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_ISORMAT_C
#define GUARD_DEEMON_OBJECTS_UNICODE_ISORMAT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/objectlist.h>

#include <stdbool.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

/* TODO: Re-write this file such that "string.format" uses `DeeObject_Foreach()'.
 *       For this purpose, scan ahead until the first '{'. If it is followed by
 *       '}' or '!', assume that the format string is "simple", meaning that
 *       every input parameter is only ever used once.
 *
 * TODO: Get rid of ':' and "DeeObject_PrintFormat()" (or at least re-work them)
 */



struct formatter {
	/* TODO: Unicode support */
	char           *f_iter;        /* [1..1] Current format string position. */
	char           *f_end;         /* [1..1] Format string end. */
	char           *f_flush_start; /* [1..1] Address where string flushing should start. */
	DREF DeeObject *f_seqiter;     /* [null_if(f_seqsize != DEE_FASTSEQ_NOTFAST_DEPRECATED)][0..1] The iterator used to get sequence-elements from `f_args' */
	DeeObject      *f_args;        /* [1..1][const] A user-given sequence object used to index format arguments. */
	size_t          f_seqsize;     /* [const] Fast sequence size of `f_args'. */
	size_t          f_seqindex;    /* [valid_if(f_seqsize != DEE_FASTSEQ_NOTFAST_DEPRECATED)] Fast sequence  */
};

PRIVATE NONNULL((1, 2)) int DCALL
error_unused_format_string(char *start, char *end) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unused/unrecognized format string %$q in string.format",
	                       (size_t)(end - start), start);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetUnaryArg(struct formatter *__restrict self,
                      char **__restrict p_fmt_start,
                      bool do_eval) {
	/* TODO: Unicode support */
	char *fmt_start = *p_fmt_start;
	char ch         = *fmt_start;
	DREF DeeObject *result;
	if (DeeUni_IsSymStrt(ch)) {
		/* Dict-style key-lookup */
		char *key_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		while (DeeUni_IsSymCont(*key_end))
			++key_end;
		if (!do_eval) {
			result = Dee_None;
			Dee_Incref(result);
		} else {
			size_t len = (size_t)(key_end - fmt_start);
			result = DeeObject_GetItemStringLenHash(self->f_args,
			                                    fmt_start,
			                                    len,
			                                    Dee_HashPtr(fmt_start, len));
		}
		fmt_start = key_end;
	} else if (DeeUni_IsDigit(ch)) {
		/* list-style index lookup */
		DREF DeeObject *key;
		char *index_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		if (!do_eval) {
			while (DeeUni_IsSymCont(*index_end))
				++index_end;
			result = Dee_None;
			Dee_Incref(result);
		} else if (ch >= '0' && ch <= '9') {
			/* Optimization for ~normal~ indices. */
			unsigned int value, radix = 10;
			dssize_t new_index, index = ch - '0';
			ch = *index_end;
			if (ch == '0') {
				if (ch == 'b' || ch == 'B') {
					radix = 2;
					++index_end;
				} else if (ch == 'x' || ch == 'X') {
					radix = 16;
					++index_end;
				} else {
					radix = 8;
				}
			}
			for (;;) {
				ch = *index_end;
				if (!DeeUni_AsDigit(ch, 16, &value)) {
					/* Check for symbol characters not recognized in numbers. */
					if unlikely(DeeUni_IsSymCont(ch))
						goto do_variable_length_index;
					break;
				}
				/* Check for values illegal for the selected radix. */
				if unlikely(value >= radix)
					goto do_variable_length_index;
				/* Add values to the new index. */
				new_index = index * radix;
				new_index += value;
				/* Check for overflow (including the sign bit). */
				if unlikely(new_index <= index)
					goto do_variable_length_index;
				/* Use the new index from now on. */
				index = new_index;
				++index_end;
			}
			/* Do an integer-index lookup. */
			result = DeeObject_GetItemIndex(self->f_args, index);
		} else {
do_variable_length_index:
			index_end = fmt_start + 1;
			while (DeeUni_IsSymCont(*index_end))
				++index_end;
			key = DeeInt_FromString(fmt_start, index_end - fmt_start,
			                        DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
			if unlikely(!key)
				goto err;
			result = DeeObject_GetItem(self->f_args, key);
			Dee_Decref(key);
		}
		fmt_start = index_end;
#if 0 /* TODO: Quoted strings? */
	} else if (ch == '\'') {
#endif
	} else if (!do_eval) {
		result = Dee_None;
		Dee_Incref(result);
	} else {
		/* Support for fast-sequence index access (primarily for Tuple and SharedVector,
		 * which are the most likely types of argument sequences used in format strings,
		 * including the case of template strings) */
		if (self->f_seqsize != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
			ASSERT(!self->f_seqiter);
			if unlikely(self->f_seqindex >= self->f_seqsize)
				goto err_not_enough_args;
			result = DeeFastSeq_GetItem_deprecated(self->f_args, self->f_seqindex);
			++self->f_seqindex;
		} else {
			/* Fallback: No key or index -> Yield the next iterator-item. */
			if (!self->f_seqiter &&
			    (self->f_seqiter = DeeObject_Iter(self->f_args)) == NULL)
				goto err;
			result = DeeObject_IterNext(self->f_seqiter);
			/* Check for iter-done */
			if unlikely(result == ITER_DONE)
				goto err_not_enough_args;
		}
	}
	*p_fmt_start = fmt_start;
	return result;
err_not_enough_args:
	DeeError_Throwf(&DeeError_UnpackError,
	                "Insufficient number of arguments");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Formatter_GetUnaryIndex(char **__restrict p_fmt_start) {
	char *fmt_start = *p_fmt_start;
	char ch         = *fmt_start;
	DREF DeeObject *result;
	if (DeeUni_IsSymStrt(ch)) {
		/* Dict-style key-lookup */
		char *key_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		while (DeeUni_IsSymCont(*key_end))
			++key_end;
		result = DeeString_NewSized(fmt_start,
		                            (size_t)(key_end - fmt_start));
		fmt_start = key_end;
	} else if (DeeUni_IsDigit(ch)) {
		/* list-style index lookup */
		char *index_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		index_end = fmt_start + 1;
		while (DeeUni_IsSymCont(*index_end))
			++index_end;
		result = DeeInt_FromString(fmt_start, index_end - fmt_start,
		                           DEEINT_STRING(0, DEEINT_STRING_FNORMAL));
		fmt_start = index_end;
	} else {
		char *end = fmt_start;
		while (*end && *end != '}')
			++end;
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid expression: %$q",
		                (size_t)(end - fmt_start), fmt_start);
		goto err;
	}
	*p_fmt_start = fmt_start;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Formatter_GetUnaryKey(char **__restrict p_fmt_start) {
	char *fmt_start = *p_fmt_start;
	char ch         = *fmt_start;
	DREF DeeObject *result;
	if (DeeUni_IsSymStrt(ch)) {
		/* Dict-style key-lookup */
		char *key_end = fmt_start + 1;
		ASSERT(!DeeUni_IsSymCont('}'));
		while (DeeUni_IsSymCont(*key_end))
			++key_end;
		result = DeeString_NewSized(fmt_start,
		                            (size_t)(key_end - fmt_start));
		fmt_start = key_end;
#if 0 /* TODO: Quoted strings? */
	} else if (ch == '\'') {
#endif
	} else {
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid key expression: %:1q",
		                fmt_start);
		goto err;
	}
	*p_fmt_start = fmt_start;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetOne(struct formatter *__restrict self,
                 char **__restrict p_fmt_start,
                 bool do_eval);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetValue(struct formatter *__restrict self,
                   char **__restrict p_fmt_start,
                   bool do_eval) {
	char *fmt_end, *fmt_start = *p_fmt_start;
	DREF DeeObject *result;
	unsigned int recursion;
	ASSERT(*fmt_start == '{');
	/* Figure out where the format ends (the next, recursive `}') */
	++fmt_start;
	fmt_end   = fmt_start;
	recursion = 1;
	for (; fmt_end < self->f_end; ++fmt_end) {
		if (*fmt_end == '{') {
			++recursion;
		} else {
			if (*fmt_end == '}' && !--recursion)
				break;
		}
	}
	/* Load the format expression. */
	result = Formatter_GetOne(self, &fmt_start, do_eval);
	if unlikely(!result)
		goto err;
	if (fmt_start < fmt_end) {
		error_unused_format_string(fmt_start, fmt_end);
		goto err;
	}

	if (*fmt_end == '}')
		++fmt_end;
	*p_fmt_start = fmt_end;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetExpr(struct formatter *__restrict self,
                  char **__restrict p_fmt_start,
                  bool do_eval) {
	return **p_fmt_start == '{'
	       ? Formatter_GetValue(self, p_fmt_start, do_eval)
	       : Formatter_GetUnaryIndex(p_fmt_start);
}

#undef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
#ifndef CONFIG_NO_ALLOW_SPACE_IN_FORMAT_EXPRESSION
#define CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
#endif /* !CONFIG_NO_ALLOW_SPACE_IN_FORMAT_EXPRESSION */


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Formatter_GetOne(struct formatter *__restrict self,
                 char **__restrict p_fmt_start,
                 bool do_eval) {
	char *fmt_start;
	DREF DeeObject *result, *new_result;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
	ASSERT(!DeeUni_IsSpace('\0'));
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
	if (**p_fmt_start == '(') {
		/* Parenthesis around main argument. */
		fmt_start = *p_fmt_start;
		++fmt_start; /* Skip `(' */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		result = Formatter_GetOne(self, &fmt_start, do_eval);
		if unlikely(!result)
			goto err;
		if (*fmt_start != ')') {
			DeeError_Throwf(&DeeError_ValueError,
			                "Expected `)' after `(' in format expression, but got %:1q",
			                fmt_start);
			goto err_r;
		}
		++fmt_start; /* Skip `)' */
	} else {
		result = Formatter_GetUnaryArg(self, p_fmt_start, do_eval);
		if unlikely(!result)
			goto err;
		fmt_start = *p_fmt_start;
	}
next_suffix:
	/* Deal with item suffix modifiers (`foo[bar]', `foo(bar)', `foo.bar') */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
	while (DeeUni_IsSpace(*fmt_start))
		++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
	switch (*fmt_start) {

		case '[': {
		DREF DeeObject *index;
		/* Key / Index lookup. */
		++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		index = Formatter_GetExpr(self, &fmt_start, do_eval);
		if unlikely(!index)
			goto err_r;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == ':') {
			DREF DeeObject *index2;
			++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			index2 = Formatter_GetExpr(self, &fmt_start, do_eval);
			if unlikely(!index2) {
				Dee_Decref(index);
				goto err_r;
			}
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			if unlikely(*fmt_start != ']') {
				Dee_Decref(index2);
err_bad_index_expression:
				DeeError_Throwf(&DeeError_ValueError,
				                "Expected `]' after `[' in format expression, but got %:1q",
				                fmt_start);
				Dee_Decref(index);
				goto err_r;
			}
			new_result = DeeObject_GetRange(result, index, index2);
			Dee_Decref(index2);
		} else {
			if unlikely(*fmt_start != ']')
				goto err_bad_index_expression;
			new_result = DeeObject_GetItem(result, index);
		}
		Dee_Decref(index);
		if unlikely(!new_result)
			goto err_r;
		Dee_Decref(result);
		result = new_result;
		++fmt_start; /* Skip the trailing `]' character. */
		goto next_suffix;
	}	break;

	case '.': {
		DREF DeeObject *attr;
		/* Attribute lookup */
		++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == '{') {
			attr = Formatter_GetValue(self, &fmt_start, do_eval);
			if (attr && DeeObject_AssertTypeExact(attr, &DeeString_Type))
				Dee_Clear(attr);
		} else {
			attr = Formatter_GetUnaryKey(&fmt_start);
		}
		if unlikely(!attr)
			goto err_r;
		/* Do the attribute lookup. */
		new_result = DeeObject_GetAttr(result, attr);
		Dee_Decref(attr);
		if unlikely(!new_result)
			goto err_r;
		Dee_Decref(result);
		result = new_result;
		goto next_suffix;
	}	break;

	case '(': {
		DREF DeeObject *arg;
		struct objectlist args;
		/* Call the currently set result expression. */
		++fmt_start;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == ')') {
			/* Simple case: 0-argument call */
			new_result = DeeObject_Call(result, 0, NULL);
			if unlikely(!new_result)
				goto err_r;
			Dee_Decref(result);
			result = new_result;
			goto next_suffix;
		}
		arg = Formatter_GetExpr(self, &fmt_start, do_eval);
		if unlikely(!arg)
			goto err_r;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		objectlist_init(&args);
		if (*fmt_start != ',') {
			/* Another simple case: 1-argument call */
			if (fmt_start[0] == '.' &&
			    fmt_start[1] == '.' &&
			    fmt_start[2] == '.') {
				/* Varargs call with single argument. */
				DREF DeeObject *new_arg;
				char *after_dots = fmt_start + 3;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
				while (DeeUni_IsSpace(*after_dots))
					++after_dots;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
				if (after_dots[0] == ',') {
					size_t error;
					/* There are more arguments after this first one. */
					error = objectlist_extendseq(&args, arg);
					Dee_Decref(arg);
					if unlikely(error == (size_t)-1)
						goto err_r;
					fmt_start = after_dots + 1; /* Skip `,' */
					goto parse_second_argument;
				}
				if unlikely(after_dots[0] != ')') {
					fmt_start = after_dots;
					goto err_expected_rparen_arg;
				}
				new_arg = DeeTuple_FromSequence(arg);
				Dee_Decref(arg);
				if unlikely(!new_arg)
					goto err_call_argv;
				new_result = DeeObject_Call(result,
				                            DeeTuple_SIZE(new_arg),
				                            DeeTuple_ELEM(new_arg));
				Dee_Decref(new_arg);
				fmt_start = after_dots; /* Skip `...' */
			} else if (fmt_start[0] != ')') {
err_expected_rparen_arg:
				Dee_Decref(arg);
err_expected_rparen:
				DeeError_Throwf(&DeeError_ValueError,
				                "Expected `)' after `(' to complete call expression, but got %:1q",
				                fmt_start);
err_call_argv:
				objectlist_fini(&args);
				goto err_r;
			} else {
				new_result = DeeObject_Call(result, 1, &arg);
			}
			++fmt_start; /* Skip `)' */
			if unlikely(!new_result)
				goto err_r;
			Dee_Decref(result);
			result = new_result;
			goto next_suffix;
		}
		if (objectlist_append(&args, arg))
			goto err_call_argv;
		++fmt_start; /* Skip `,' */
parse_second_argument:
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		/* Parse additional arguments. */
		for (;;) {
			arg = Formatter_GetExpr(self, &fmt_start, do_eval);
			if unlikely(!arg)
				goto err_call_argv;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			if (fmt_start[0] == '.' &&
			    fmt_start[1] == '.' &&
			    fmt_start[2] == '.') {
				size_t extend_error;
				/* Expand argument list. */
				extend_error = objectlist_extendseq(&args, arg);
				Dee_Decref(arg);
				if unlikely(extend_error == (size_t)-1)
					goto err_call_argv;
				fmt_start += 3;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
				while (DeeUni_IsSpace(*fmt_start))
					++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			} else {
				/* Append to the argument list. */
				if (objectlist_append(&args, arg))
					goto err_call_argv;
			}
			if (*fmt_start != ',')
				break;
			++fmt_start; /* Skip `,' */
		}
		if (*fmt_start != ')')
			goto err_expected_rparen;
		++fmt_start; /* Skip `)' */
		/* Actually do the call. */
		new_result = DeeObject_Call(result, args.ol_elemc, args.ol_elemv);
		objectlist_fini(&args);
		/* Set the returned value as new result. */
		if unlikely(!new_result)
			goto err_r;
		Dee_Decref(result);
		result = new_result;
		goto next_suffix;
	}	break;

		case '?': {
		DREF DeeObject *tt, *ff;
		int is_true;
		/* Conditional expression:
		 * >> print "Hello {cond ? world : universe}".format({ .cond = true });  // Hello world
		 * >> print "Hello {cond ? world : universe}".format({ .cond = false }); // Hello universe
		 * NOTE: Also supports the `?:' extension that re-uses
		 *       the condition expression as the tt-, or ff-value
		 * NOTE: Also supports the missing-: extension that uses `none' for `ff' */
		/* Evaluate the condition. */
		is_true = DeeObject_Bool(result);
		if unlikely(is_true < 0)
			goto err_r;
		++fmt_start; /* Skip `?' */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
		while (DeeUni_IsSpace(*fmt_start))
			++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		if (*fmt_start == ':') {
			tt = result;
			Dee_Incref(tt);
		} else {
			tt = Formatter_GetExpr(self, &fmt_start, do_eval && is_true != 0);
			if unlikely(!tt)
				goto err_r;
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
		}
		if (*fmt_start == ':') {
			++fmt_start; /* Skip `?' */
#ifdef CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION
			while (DeeUni_IsSpace(*fmt_start))
				++fmt_start;
#endif /* CONFIG_ALLOW_SPACE_IN_FORMAT_EXPRESSION */
			if (!*fmt_start || *fmt_start == '}') {
				/* Re-use the condition for `ff' */
				ff = result;
				Dee_Incref(ff);
			} else {
				ff = Formatter_GetExpr(self, &fmt_start, do_eval && is_true == 0);
				if unlikely(!ff) {
					Dee_Decref(tt);
					goto err_r;
				}
			}
		} else {
			/* No ff-branch given. */
			ff = Dee_None;
			Dee_Incref(ff);
		}
		/* Set the new result value. */
		Dee_Decref(result);
		if (is_true) {
			result = tt; /* Inherit reference */
			Dee_Decref(ff);
		} else {
			result = ff; /* Inherit reference */
			Dee_Decref(tt);
		}
		goto next_suffix;
	}	break;

	default: break;
	}

	*p_fmt_start = fmt_start;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}




PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
format_impl(struct formatter *__restrict self,
            dformatprinter printer, void *arg) {
	DREF DeeObject *in_arg;
#define print(p, s)                            \
	do {                                       \
		if unlikely((*printer)(arg, p, s) < 0) \
			goto err;                          \
	}	__WHILE0
	ASSERT(!*self->f_end);
	while (self->f_iter < self->f_end) {
		char *format_start;
		char *format_end;
		char ch = *self->f_iter++;
		unsigned int recursion;
		dssize_t print_error;
		if (ch == '}' && *self->f_iter == '}') {
			/* Replace `}}' with `}' */
			ASSERT(self->f_iter != self->f_end);
			print(self->f_flush_start, (size_t)((self->f_iter - 1) - self->f_flush_start));
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		if (ch != '{')
			continue;
		/* Flush everything up to this point. */
		print(self->f_flush_start, (size_t)((self->f_iter - 1) - self->f_flush_start));
		if (*self->f_iter == '{') {
			/* Replace `{{' with `{' */
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		format_start = self->f_iter;
		/* Figure out where the format ends (the next, recursive `}') */
		format_end = self->f_iter;
		recursion  = 1;
		for (; format_end < self->f_end; ++format_end) {
			if (*format_end == '{') {
				++recursion;
			} else {
				if (*format_end == '}' && !--recursion)
					break;
			}
		}
		/* Process the format string to extract an argument. */
		in_arg = Formatter_GetOne(self, &format_start, true);
		if unlikely(!in_arg)
			goto err;
		if (*format_start == '!') {
			/* Explicit format mode. */
			char mode = *++format_start;
			if (mode == 'a' || mode == 's')
				goto print_normal;
			if (mode == 'r') {
				print_error = DeeObject_PrintRepr(in_arg, printer, arg);
			} else {
				/* TODO: This error message doesn't handle unicode! */
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid character %.1q following `!' in string.format",
				                format_start);
				goto err_arg;
			}
			++format_start;
		} else if (*format_start == ':') {
			/* Format according to the format string. */
			char *content_start = format_start + 1;
			char *content_end   = format_end;
			char *content_iter;
			/* Check if the format string contains additional format commands. */
			content_iter = content_start;
			for (; content_iter < content_end; ++content_iter) {
				if (*content_iter == '{' || *content_iter == '}') {
					/* Special format-string pre-processing is required. */
					struct formatter inner_formatter;
					struct unicode_printer format_string_printer = UNICODE_PRINTER_INIT;
					DREF DeeObject *format_string;
					inner_formatter.f_iter        = content_iter;
					inner_formatter.f_end         = content_end;
					inner_formatter.f_flush_start = content_start;
					inner_formatter.f_seqiter     = self->f_seqiter;
					inner_formatter.f_args        = self->f_args;
					inner_formatter.f_seqsize     = self->f_seqsize;
					inner_formatter.f_seqindex    = self->f_seqindex;

					/* Format the format string, thus allowing it
					 * to contain text from input arguments. */
					print_error = format_impl(&inner_formatter,
					                          &unicode_printer_print,
					                          &format_string_printer);
					self->f_seqiter  = inner_formatter.f_seqiter;
					self->f_seqindex = inner_formatter.f_seqindex;
					if unlikely(print_error < 0) {
						unicode_printer_fini(&format_string_printer);
						goto err_arg;
					}
					format_string = unicode_printer_pack(&format_string_printer);
					if unlikely(!format_string)
						goto err_arg;
					/* Now use the generated format-string to format the input argument. */
					print_error = DeeObject_PrintFormat(in_arg, printer, arg, format_string);
					Dee_Decref(format_string);
					goto check_print_error;
				}
			}
			/* No pre-processing required -> Just format the object as it is right now! */
			print_error = DeeObject_PrintFormatString(in_arg, printer, arg, content_start,
			                                          (size_t)(content_end - content_start));
		} else {
print_normal:
			print_error = DeeObject_Print(in_arg, printer, arg);
		}
check_print_error:
		Dee_Decref(in_arg);
		if unlikely(print_error < 0)
			goto err;
		ASSERT(format_start <= format_end);
		if (format_start < format_end) {
			error_unused_format_string(format_start, format_end);
			goto err;
		}
		if (format_end >= self->f_end)
			break; /* The format string ends here. */
		self->f_iter        = format_end + 1;
		self->f_flush_start = self->f_iter;
	}

	/* Flush the remainder. */
	return (int)(*printer)(arg, self->f_flush_start,
	                       (size_t)(self->f_iter -
	                                self->f_flush_start));
#undef print
err_arg:
	Dee_Decref(in_arg);
err:
	return -1;
}


/* Format a given `format' string subject to {}-style formatting rules.
 * NOTE: This is the function called by `.format' for strings.
 * @param: args: A sequence (usually a Dict, or a List) used for
 *               providing input values to the format string. */
INTERN WUNUSED NONNULL((1, 3, 5)) dssize_t DCALL
DeeString_Format(dformatprinter printer, void *arg,
                 /*utf-8*/ char const *__restrict format,
                 size_t format_len, DeeObject *__restrict args) {
	struct formatter self;
	dssize_t result;
	self.f_flush_start = (char *)format;
	self.f_iter        = (char *)format;
	self.f_end         = (char *)format + format_len;
	self.f_seqiter     = NULL;
	self.f_args        = args;
	self.f_seqsize     = DeeFastSeq_GetSize_deprecated(args);
	self.f_seqindex    = 0;
	result             = format_impl(&self, printer, arg);
	Dee_XDecref(self.f_seqiter);
	return result;
}



PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
format_bytes_impl(struct formatter *__restrict self,
                  dformatprinter printer,
                  dformatprinter format_printer,
                  void *arg) {
	dssize_t temp, result = 0;
	DREF DeeObject *in_arg;
#define print_format(p, s)                                     \
	do {                                                       \
		if unlikely((temp = (*format_printer)(arg, p, s)) < 0) \
			goto err;                                          \
		result += temp;                                        \
	}	__WHILE0
	while (self->f_iter < self->f_end) {
		char *format_start;
		char *format_end;
		char ch = *self->f_iter++;
		unsigned int recursion;
		dssize_t print_error;
		if (ch == '}' &&
		    self->f_iter < self->f_end &&
		    *self->f_iter == '}') {
			/* Replace `}}' with `}' */
			print_format(self->f_flush_start,
			             (size_t)((self->f_iter - 1) - self->f_flush_start));
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		if (ch != '{')
			continue;
		if (self->f_iter == self->f_end)
			break;
		/* Flush everything up to this point. */
		print_format(self->f_flush_start,
		             (size_t)((self->f_iter - 1) - self->f_flush_start));
		if (*self->f_iter == '{') {
			/* Replace `{{' with `{' */
			self->f_flush_start = self->f_iter;
			++self->f_iter;
			continue;
		}
		format_start = self->f_iter;
		/* Figure out where the format ends (the next, recursive `}') */
		format_end = self->f_iter;
		recursion  = 1;
		for (; format_end < self->f_end; ++format_end) {
			if (*format_end == '{') {
				++recursion;
			} else {
				if (*format_end == '}' && !--recursion)
					break;
			}
		}
		/* Process the format string to extract an argument. */
		in_arg = Formatter_GetOne(self, &format_start, true);
		if unlikely(!in_arg)
			goto err;
		if (*format_start == '!') {
			/* Explicit format mode. */
			char mode = *++format_start;
			if (mode == 'a' || mode == 's')
				goto print_normal;
			if (mode == 'r') {
				print_error = DeeObject_PrintRepr(in_arg, printer, arg);
			} else {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid character %.1q following `!' in string.format",
				                format_start);
				goto err_arg;
			}
			++format_start;
		} else if (*format_start == ':') {
			/* Format according to the format string. */
			char *content_start = format_start + 1;
			char *content_end   = format_end;
			char *content_iter;
			/* Check if the format string contains additional format commands. */
			content_iter = content_start;
			for (; content_iter < content_end; ++content_iter) {
				if (*content_iter == '{' || *content_iter == '}') {
					/* Special format-string pre-processing is required. */
					struct formatter inner_formatter;
					struct unicode_printer format_string_printer = UNICODE_PRINTER_INIT;
					DREF DeeObject *format_string;
					inner_formatter.f_iter        = content_iter;
					inner_formatter.f_end         = content_end;
					inner_formatter.f_flush_start = content_start;
					inner_formatter.f_seqiter     = self->f_seqiter;
					inner_formatter.f_args        = self->f_args;
					inner_formatter.f_seqindex    = self->f_seqindex;
					inner_formatter.f_seqsize     = self->f_seqsize;

					/* Format the format string, thus allowing it
					 * to contain text from input arguments. */
					print_error = format_bytes_impl(&inner_formatter,
					                                &unicode_printer_print,
					                                &unicode_printer_print,
					                                &format_string_printer);
					self->f_seqiter  = inner_formatter.f_seqiter;
					self->f_seqindex = inner_formatter.f_seqindex;
					if unlikely(print_error < 0) {
						unicode_printer_fini(&format_string_printer);
						goto err_arg;
					}
					format_string = unicode_printer_pack(&format_string_printer);
					if unlikely(!format_string)
						goto err_arg;
					/* Now use the generated format-string to format the input argument. */
					print_error = DeeObject_PrintFormat(in_arg, printer, arg, format_string);
					Dee_Decref(format_string);
					goto check_print_error;
				}
			}
			/* No pre-processing required -> Just format the object as it is right now! */
			print_error = DeeObject_PrintFormatString(in_arg, printer, arg, content_start,
			                                          (size_t)(content_end - content_start));
		} else {
print_normal:
			print_error = DeeObject_Print(in_arg, printer, arg);
		}
check_print_error:
		Dee_Decref(in_arg);
		if unlikely(print_error < 0)
			goto err;
		ASSERT(format_start <= format_end);
		if (format_start < format_end) {
			error_unused_format_string(format_start, format_end);
			goto err;
		}
		if (format_end >= self->f_end)
			break; /* The format string ends here. */
		self->f_iter        = format_end + 1;
		self->f_flush_start = self->f_iter;
	}
	/* Flush the remainder. */
	print_format(self->f_flush_start,
	             (size_t)(self->f_iter -
	                      self->f_flush_start));
	return result;
#undef print_format
err_arg:
	Dee_Decref(in_arg);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 4, 6)) dssize_t DCALL
DeeBytes_Format(dformatprinter printer,
                dformatprinter format_printer, void *arg,
                char const *__restrict format,
                size_t format_len, DeeObject *__restrict args) {
	struct formatter self;
	dssize_t result;
	self.f_flush_start = (char *)format;
	self.f_iter        = (char *)format;
	self.f_end         = (char *)format + format_len;
	self.f_seqiter     = NULL;
	self.f_args        = args;
	self.f_seqsize     = DeeFastSeq_GetSize_deprecated(args);
	self.f_seqindex    = 0;
	result             = format_bytes_impl(&self, printer, format_printer, arg);
	Dee_XDecref(self.f_seqiter);
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_ISORMAT_C */
