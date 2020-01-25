/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C
#define GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memrchr() */

#include <string.h>

DECL_BEGIN

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr 1
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */


struct match_count {
	size_t mc_min;    /* Min number of matches */
	size_t mc_max;    /* [!0][>= mc_min] Max number of matches */
	bool   mc_greedy; /* Operate in greedy mode. */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) char *DCALL
parse_match_count(char *piter, char *pend,
                  struct match_count *__restrict result) {
	result->mc_min    = 1;
	result->mc_max    = 1;
	result->mc_greedy = false;
	if (piter < pend) {
		char ch = *piter;
		switch (ch) {

		case '*':
			result->mc_min    = 0;
			result->mc_max    = (size_t)-1;
			result->mc_greedy = true;
check_greedy:
			++piter;
check_greedy2:
			if (*piter == '?') {
				result->mc_greedy = false;
				++piter;
			}
			break;

		case '+':
			result->mc_max    = (size_t)-1;
			result->mc_greedy = true;
			goto check_greedy;

		case '?':
			result->mc_min    = 0;
			result->mc_greedy = true;
			goto check_greedy;

		case '{': {
			uint32_t ch32;
			struct unitraits *trt;
			result->mc_greedy = true;
			++piter;
			ch32 = utf8_readchar((char const **)&piter, pend);
			if (ch32 == ',') {
				result->mc_min = 0;
			} else {
				trt = DeeUni_Descriptor(ch32);
				if (trt->ut_flags & UNICODE_FDECIMAL) {
					/* Digit. */
					result->mc_min = trt->ut_digit;
					for (;;) {
						if (piter >= pend) {
err_eof_in_repeat:
							DeeError_Throwf(&DeeError_ValueError,
							                "Unexpected end of pattern "
							                "following `{' in regular expression");
							goto err;
						}
						ch32 = utf8_readchar((char const **)&piter, pend);
						trt  = DeeUni_Descriptor(ch32);
						if (!(trt->ut_flags & UNICODE_FDECIMAL))
							break;
						result->mc_min *= 10;
						result->mc_min += trt->ut_digit;
					}
				} else {
					DeeError_Throwf(&DeeError_ValueError,
					                "Expected `,' or a decimal after `{' in "
					                "regular expression, but got %I32c",
					                ch32);
					goto err;
				}
			}
			if (ch32 == '}') {
				result->mc_max = result->mc_min;
			} else {
				if (ch32 != ',') {
					DeeError_Throwf(&DeeError_ValueError,
					                "Expected `,' after `{%Iu' in "
					                "regular expression, but got %I32c",
					                result->mc_min, ch32);
					goto err;
				}
				if (piter == pend)
					goto err_eof_in_repeat;
				ch32 = utf8_readchar((char const **)&piter, pend);
				if (ch32 == '}') {
					result->mc_max = (size_t)-1;
				} else {
					trt = DeeUni_Descriptor(ch32);
					if (trt->ut_flags & UNICODE_FDECIMAL) {
						result->mc_max = trt->ut_digit;
						for (;;) {
							if (piter >= pend)
								goto err_eof_in_repeat;
							ch32 = utf8_readchar((char const **)&piter, pend);
							trt  = DeeUni_Descriptor(ch32);
							if (!(trt->ut_flags & UNICODE_FDECIMAL))
								break;
							result->mc_max *= 10;
							result->mc_max += trt->ut_digit;
						}
					}
					if (ch32 != '}') {
						DeeError_Throwf(&DeeError_ValueError,
						                "Expected `}' after `{%Iu,%Iu' in "
						                "regular expression, but got %I32c",
						                result->mc_min, result->mc_max, ch32);
						goto err;
					}
					if (result->mc_max < result->mc_min) {
						DeeError_Throwf(&DeeError_ValueError,
						                "Upper repetition bound %Iu isn't greater or equal to lower bound %Iu",
						                result->mc_max, result->mc_min);
						goto err;
					}
					if (!result->mc_max) {
						/* Don't allow an upper bound of ZERO, which would be pointless
						 * -> Instead, force the user to not write non-sensical regex
						 *    that would only slow down the matching algorithm. */
						DeeError_Throwf(&DeeError_ValueError,
						                "An upper repetition bound of 0 is pointless and not allowed");
						goto err;
					}
				}
			}
			goto check_greedy2;
		}

		default: break;
		}
	}
	return piter;
err:
	return NULL;
}


struct regex_data {
	size_t matlen;                          /* Match length (success-output-only). */
	char *data_start;                       /* [const] Data start pointer. */
	uint16_t flags;                         /* [const] Processing flags. */
	uint16_t pad[(sizeof(void *) - 2) / 2]; /* ... */
	/* XXX: Group support? */
};





LOCAL WUNUSED NONNULL((1, 2, 4)) bool DCALL
is_in_range(char *range_start, char *range_end,
            uint32_t data_ch, struct regex_data *__restrict data) {
	char *iter = range_start;
	if (data->flags & Dee_REGEX_FNOCASE) {
		data_ch = DeeUni_ToLower(data_ch);
		while (iter < range_end) {
			uint32_t ch = utf8_readchar((char const **)&iter, range_end);
			if (ch == '\\') {
				/* Escaped character, or special group */
				ch = utf8_readchar((char const **)&iter, range_end);
				switch (ch) {

				case 'd':
					if (DeeUni_IsDigit(data_ch))
						goto ok;
					break;

				case 'D':
					if (!DeeUni_IsDigit(data_ch))
						goto ok;
					break;

				case 's':
					if (DeeUni_IsSpace(data_ch))
						goto ok;
					break;

				case 'S':
					if (!DeeUni_IsSpace(data_ch))
						goto ok;
					break;

				case 'w':
					if (DeeUni_Flags(data_ch) & (UNICODE_FSYMSTRT | UNICODE_FSYMCONT))
						goto ok;
					break;

				case 'W':
					if (!(DeeUni_Flags(data_ch) & (UNICODE_FSYMSTRT | UNICODE_FSYMCONT)))
						goto ok;
					break;

				case 'n':
					if (DeeUni_IsLF(data_ch))
						goto ok;
					break;

				case 'N':
					if (!DeeUni_IsLF(data_ch))
						goto ok;
					break;

				case 'a': ch = 0x07; goto match_single_nocase2;
				case 'b': ch = 0x08; goto match_single_nocase2;
				case 'f': ch = 0x0c; goto match_single_nocase2;
				case 'r': ch = 0x0d; goto match_single_nocase2;
				case 't': ch = 0x09; goto match_single_nocase2;
				case 'v': ch = 0x0b; goto match_single_nocase2;
				case 'e': ch = 0x1b; goto match_single_nocase2;

				default:
					goto match_single_nocase;
				}
			} else if (iter[0] == '-') {
				/* Ranged match. */
				uint32_t high;
				++iter;
				high = utf8_readchar((char const **)&iter, range_end);
				if (data_ch >= DeeUni_ToLower(ch) &&
				    data_ch <= DeeUni_ToLower(high))
					return true;
			} else {
				/* Fallback: match single */
match_single_nocase:
				ch = DeeUni_ToLower(ch);
match_single_nocase2:
				if (data_ch == ch)
					goto ok;
			}
		}
	} else {
		while (iter < range_end) {
			uint32_t ch = utf8_readchar((char const **)&iter, range_end);
			if (ch == '\\') {
				/* Escaped character, or special group */
				ch = utf8_readchar((char const **)&iter, range_end);
				switch (ch) {

				case 'd':
					if (DeeUni_IsDigit(data_ch))
						goto ok;
					break;

				case 'D':
					if (!DeeUni_IsDigit(data_ch))
						goto ok;
					break;

				case 's':
					if (DeeUni_IsSpace(data_ch))
						goto ok;
					break;

				case 'S':
					if (!DeeUni_IsSpace(data_ch))
						goto ok;
					break;

				case 'w':
					if (DeeUni_Flags(data_ch) & (UNICODE_FSYMSTRT | UNICODE_FSYMCONT))
						goto ok;
					break;

				case 'W':
					if (!(DeeUni_Flags(data_ch) & (UNICODE_FSYMSTRT | UNICODE_FSYMCONT)))
						goto ok;
					break;

				case 'n':
					if (DeeUni_IsLF(data_ch))
						goto ok;
					break;

				case 'N':
					if (!DeeUni_IsLF(data_ch))
						goto ok;
					break;

				case 'a':
					ch = 0x07;
					goto match_single;

				case 'b':
					ch = 0x08;
					goto match_single;

				case 'f':
					ch = 0x0c;
					goto match_single;

				case 'r':
					ch = 0x0d;
					goto match_single;

				case 't':
					ch = 0x09;
					goto match_single;

				case 'v':
					ch = 0x0b;
					goto match_single;

				case 'e':
					ch = 0x1b;
					goto match_single;

					/* TODO: `x' (hex-digit) */

				default:
					goto match_single;
				}
			} else if (iter[0] == '-') {
				/* Ranged match. */
				uint32_t high;
				++iter;
				high = utf8_readchar((char const **)&iter, range_end);
				if (data_ch >= ch && data_ch <= high)
					return true;
			} else {
				/* Fallback: match single */
match_single:
				if (data_ch == ch)
					goto ok;
			}
		}
	}
	return false;
ok:
	return true;
}


PRIVATE WUNUSED NONNULL((1, 2)) char *DCALL
find_rparen(char *piter, char *pend) {
	unsigned int paren_recursion = 0;
	while (piter < pend) {
		char ch = *piter++;
		if (ch == '\\' && piter < pend) {
			++piter;
		} else if (ch == '(') {
			++paren_recursion;
		} else if (ch == ')') {
			if (!paren_recursion)
				break;
			--paren_recursion;
		} else if (ch == '[') {
			while (piter < pend) {
				ch = *piter++;
				if (ch == '\\' && piter < pend) {
					++piter;
				} else if (ch == ']') {
					break;
				}
			}
		}
	}
	return piter;
}

PRIVATE WUNUSED NONNULL((1, 2)) char *DCALL
find_pipe(char *piter, char *pend) {
	unsigned int paren_recursion = 0;
	while (piter < pend) {
		char ch = *piter++;
		if (ch == '\\' && piter < pend) {
			++piter;
		} else if (ch == '|') {
			if (!paren_recursion)
				return piter;
		} else if (ch == '(') {
			++paren_recursion;
		} else if (ch == ')') {
			--paren_recursion;
		} else if (ch == '[') {
			while (piter < pend) {
				ch = *piter++;
				if (ch == '\\' && piter < pend) {
					++piter;
				} else if (ch == ']') {
					break;
				}
			}
		}
	}
	return NULL;
}


#define REGEX_CONTEXT_FNORMAL   0x0000
#define REGEX_CONTEXT_FINPAREN  0x0001
#define REGEX_CONTEXT_FEMPTYOK  0x0002 /* Empty matches are OK */
#define regex_match(pditer, dend, ppiter, pend, data) \
	regex_match_impl(pditer, dend, ppiter, pend, data, REGEX_CONTEXT_FNORMAL)
PRIVATE WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
regex_match_impl(char **pditer, char *dend,
                 char **ppiter, char *pend,
                 struct regex_data *__restrict data,
                 unsigned int context);



PRIVATE WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
regex_match_or(char **pditer, char *dend,
               char *content_start,
               char *content_end,
               struct regex_data *__restrict data,
               unsigned int context) {
	int error;
	ASSERT(context & REGEX_CONTEXT_FINPAREN);
	for (;;) {
		error = regex_match_impl(pditer, dend,
		                         &content_start, content_end,
		                         data, context);
		if (error != 0)
			break; /* Error, or success. */
		/* Search for the next variant. */
		content_start = find_pipe(content_start, content_end);
		if (!content_start)
			break;
	}
	return error;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
parse_u8(char **ppiter, char *pend,
         uint8_t *__restrict result) {
	bool is_first = true;
	*result       = 0;
	while (*ppiter < pend) {
		char *prev_piter      = *ppiter;
		uint32_t ch           = utf8_readchar((char const **)ppiter, pend);
		struct unitraits *trt = DeeUni_Descriptor(ch);
		if (!(trt->ut_flags & UNICODE_FDECIMAL)) {
			*ppiter = prev_piter;
			break;
		}
		*result *= 10;
		*result += trt->ut_digit;
		is_first = false;
	}
	if unlikely(is_first) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Expected a decimal for digit range");
		goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
parse_digit_range(char *piter, char *pend,
                  uint8_t *__restrict plow,
                  uint8_t *__restrict phigh) {
	bool has_low = false, has_high = false;
	while (piter < pend) {
		if (!has_high && *piter == '<') {
			++piter;
			if (*piter == '=') {
				++piter;
				if (parse_u8(&piter, pend, phigh))
					goto err;
			} else {
				if (parse_u8(&piter, pend, phigh))
					goto err;
				if unlikely(!*phigh) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Invalid digit matching rule: `<0'");
					goto err;
				}
				--*phigh;
			}
			has_high = true;
			continue;
		}
		if (!has_low && *piter == '>') {
			++piter;
			if (*piter == '=') {
				++piter;
				if (parse_u8(&piter, pend, plow))
					goto err;
			} else {
				if (parse_u8(&piter, pend, plow))
					goto err;
				if unlikely(*plow == 0xff) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Invalid digit matching rule: `>255'");
					goto err;
				}
				++*plow;
			}
			has_low = true;
			continue;
		}
		if (!has_low && !has_high && *piter == '=') {
			++piter;
			if (parse_u8(&piter, pend, plow))
				goto err;
			*phigh = *plow;
			break;
		}
		break;
	}
	ASSERT(piter <= pend);
	if unlikely(piter < pend) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Bad digit matching range string %$q",
		                (size_t)(pend - piter), piter);
		goto err;
	}
	return 0;
err:
	return -1;
}


LOCAL WUNUSED uniflag_t DCALL
get_regex_trait(/*utf-32*/ uint32_t name) {
	uniflag_t result;
	switch (name) {
#define REGEX_TRAIT_NAMES "pasnlutcdD01"
	case 'p': result = UNICODE_FPRINT; break;   /* The character is printable, or SPC (` '). */
	case 'a': result = UNICODE_FALPHA; break;   /* The character is alphabetic. */
	case 's': result = UNICODE_FSPACE; break;   /* The character is a space-character. */
	case 'n': result = UNICODE_FLF; break;      /* Line-feed/line-break character. */
	case 'l': result = UNICODE_FLOWER; break;   /* Lower-case. */
	case 'u': result = UNICODE_FUPPER; break;   /* Upper-case. */
	case 't': result = UNICODE_FTITLE; break;   /* Title-case. */
	case 'c': result = UNICODE_FCNTRL; break;   /* Control character. */
	case 'D': result = UNICODE_FDIGIT; break;   /* The character is a digit. e.g.: `²' (sqare; `ut_digit' is `2') */
	case 'd': result = UNICODE_FDECIMAL; break; /* The character is a decimal. e.g: `5' (ascii; `ut_digit' is `5') */
	case '0': result = UNICODE_FSYMSTRT; break; /* The character can be used as the start of an identifier. */
	case '1': result = UNICODE_FSYMCONT; break; /* The character can be used to continue an identifier. */
	default: result = 0; break;                 /* Unrecognized. */
	}
	return result;
}


/* @param: pditer: [IN]  A pointer to the starting address of input data
 *                 [OUT][ON_SUCCESS] Updated to point to the end of matched data.
 *                 [OUT][ON_FAILURE] Undefined
 * @param: ppiter: [IN]  A pointer to the starting address of pattern data
 *                 [OUT] Updated to point to the end of matched data.
 * @return:  1: A match was found (success; `data->matlen' is filled in)
 * @return:  0: No match was made (failure)
 * @return: -1: Error.
 */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
regex_match_impl(char **pditer, char *dend,
                 char **ppiter, char *pend,
                 struct regex_data *__restrict data,
                 unsigned int context) {
#define CHECK_NONGREEDY_SUFFIX()                                        \
	if (!match.mc_greedy && match_count >= match.mc_min) {              \
		/* Check if the remainder of input data can not be matched. */  \
		new_diter = diter;                                              \
		new_piter = piter;                                              \
		error = regex_match_impl(&new_diter, dend,                      \
		                         &new_piter, pend, data,                \
		                         context | REGEX_CONTEXT_FEMPTYOK);     \
		if (error != 0)                                                 \
			goto forward_error; /* Suffix match found! */               \
	}

	/* Match character if `expr_using_data_ch' is true */
#define DO_CHARACTERWISE_MATCH(expr_using_data_ch)               \
	while (match_count < match.mc_max) {                         \
		char *prev_diter;                                        \
		CHECK_NONGREEDY_SUFFIX();                                \
		if (diter >= dend)                                       \
			break;                                               \
		prev_diter = diter;                                      \
		data_ch    = utf8_readchar((char const **)&diter, dend); \
		if (!(expr_using_data_ch)) {                             \
			diter = prev_diter;                                  \
			break;                                               \
		}                                                        \
		++match_count;                                           \
		++match_length;                                          \
	}

 char *new_diter;
	char *new_piter;
	char *diter = *pditer;
	char *piter = *ppiter;
	uint32_t ch, data_ch;
	struct match_count match;
	int error;
	size_t match_count;
	size_t match_length = 0;
next:
	ASSERT(piter <= pend);
	if (piter >= pend)
		goto done;
	ch          = utf8_readchar((char const **)&piter, pend);
	match_count = 0;
	switch (ch) {

	case '(': {
		char *rparen;
		char *content_start;
		char *content_end;
		char *variant_diter;
		char *variant_piter;
		char *suffix_piter;
		size_t variant_match_length;
		rparen        = find_rparen(piter, pend);
		content_start = piter;
		content_end   = rparen - 1;
		if (rparen[-1] != ')')
			++content_end;
		piter = parse_match_count(rparen, pend, &match);
		if unlikely(!piter)
			goto err;
		ASSERT(match.mc_min <= match.mc_max);
		if (content_start[0] == '?') {
			/* Regex extensions. */
			switch (content_start[1]) {

			case '\\':
				/* Match special character trait sub-classes (currently only digit) */
				if (content_start[2] == 'd') {
					uint8_t COMPILER_IGNORE_UNINITIALIZED(low);
					uint8_t COMPILER_IGNORE_UNINITIALIZED(high);
					if (parse_digit_range(content_start + 3, content_end, &low, &high))
						goto err;
					/* Match digit characters with a numerical value >= low && <= high */
					while (match_count < match.mc_max) {
						char *prev_diter;
						struct unitraits *trt;
						CHECK_NONGREEDY_SUFFIX();
						if (diter >= dend)
							break;
						prev_diter = diter;
						data_ch    = utf8_readchar((char const **)&diter, dend);
						trt        = DeeUni_Descriptor(data_ch);
						if (!(trt->ut_flags & UNICODE_FDIGIT) ||
						    trt->ut_digit < low ||
						    trt->ut_digit > high) {
							diter = prev_diter;
							break;
						}
						++match_count;
						++match_length;
					}
					goto check_match;
				}
				break;

			case '<':
				if (content_start[2] != '=' && content_start[2] != '!')
					break;
				/* Positive look-behind assertion.
				 * -> Check for if parenthesis contents match before the current position. */
				content_start += 3; /* ?<= */
				if (diter == data->data_start) {
					/* Special case: Check if the content string can match an empty string. */
					variant_diter = diter;
					error = regex_match_or(&variant_diter, diter,
					                       content_start, content_end,
					                       data,
					                       REGEX_CONTEXT_FINPAREN |
					                       REGEX_CONTEXT_FEMPTYOK);
					if (error != 0) { /* Match or error. */
						if unlikely(error < 0)
							goto err;
						if (content_start[-1] == '!')
							goto nope;
						goto next; /* It was able to match an empty string! */
					}
				} else {
					variant_diter = diter - 1;
					while (variant_diter > data->data_start) {
						error = regex_match_or(&variant_diter, diter, /* dend = current_data_position */
						                       content_start, content_end,
						                       data,
						                       REGEX_CONTEXT_FINPAREN |
						                       REGEX_CONTEXT_FEMPTYOK);
						if (error != 0) { /* Match or error. */
							if unlikely(error < 0)
								goto err;
							if unlikely((variant_diter != diter) ^ (content_start[-1] == '!'))
								goto nope; /* The match doesn't end where our datastring begins. */
							goto next;
						}
						--variant_diter;
					}
				}
				if (content_start[-1] == '!')
					goto next;
				goto nope;

			case '=':
				if unlikely(!match.mc_min) {
					/* Don't allow optional matches, which would mean that the entire lookahead
					 * was optional in itself. And considering that it's not meant to consume any
					 * data, it would become a no-op altogether! */
err_lookahead_must_be_nonzero:
					DeeError_Throwf(&DeeError_ValueError,
					                "Positive lookahead assertions require a non-zero lower matching-bound. "
					                "I.e. You can't use a `?', `*' or `{0,x}' / `{,x}' suffix");
					goto err;
				}
				/* Lookahead to assert that at the very leas `match.mc_min'  */
				variant_diter = diter;
				match_count   = 0;
				while (match_count < match.mc_min) {
					error = regex_match_or(&variant_diter, dend,
					                       content_start + 2,
					                       content_end,
					                       data,
					                       REGEX_CONTEXT_FEMPTYOK |
					                       REGEX_CONTEXT_FINPAREN);
					if unlikely(error < 0)
						goto err;
					if (!error)
						goto nope; /* Insufficient number of matches */
					++match_count;
				}
				goto next;

			case '!':
				/* The inverse of `?=': Lookahead for a non-matching assertion
				 * Useful if you don't want to match anything x followed by y:
				 * >> print repr "foo foobar".refindall(r"foo");        // { (0, 3), (4, 7) }
				 * >> print repr "foo foobar".refindall(r"foo(?!bar)"); // { (0, 3) } */
				if unlikely(!match.mc_min)
					goto err_lookahead_must_be_nonzero;
				variant_diter = diter;
				match_count   = 0;
				/* Lookahead to assert that at the very leas `match.mc_min'  */
				while (match_count < match.mc_min) {
					error = regex_match_or(&variant_diter, dend,
					                       content_start + 2,
					                       content_end,
					                       data,
					                       REGEX_CONTEXT_FEMPTYOK |
					                       REGEX_CONTEXT_FINPAREN);
					if unlikely(error < 0)
						goto err;
					if (!error)
						goto next; /* Miss-match found before it would have been too late. */
					++match_count;
				}
				goto nope; /* Too many matches... */

			case '#':
				/* Comment. */
				goto next;

			case '~': /* Invert flag meaning. */
			case 'i': /* Case-insensitive */
			case 'm': /* Multi-line */
			case 's': /* Dot matches all */
				content_start += 1;
				variant_piter = content_start;
				while (content_start < content_end) {
					bool should_invert = false;
					char flag          = *content_start++;
					uint16_t flag_value;
					if (flag == '~') {
						should_invert = true;
						flag          = *content_start++;
					}
					switch (flag) {
					case 'i': flag_value = Dee_REGEX_FNOCASE; break;
					case 'm': flag_value = Dee_REGEX_FMULTILINE; break;
					case 's': flag_value = Dee_REGEX_FDOTALL; break;
					default:
						DeeError_Throwf(&DeeError_ValueError,
						                "Unknown regex flag `%c'",
						                flag);
						goto err;
					}
					if (should_invert)
						data->flags &= ~flag_value;
					else {
						data->flags |= flag_value;
					}
				}
				goto next;


			default:
				break;
			}
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown regex extension `(?%c...)'",
			                content_start[1]);
			goto err;
		}

		variant_piter = content_start;
		while (variant_piter < content_end) {
			struct match_backup {
				char *mb_diter;
				size_t mb_dcount;
			};
			/* [0..match_count|ALLOC(backup_a)][owned] */
			struct match_backup *backup_v;
			size_t backup_a;
			char *variant_pstart;
			CHECK_NONGREEDY_SUFFIX();
			variant_diter  = diter;
			variant_pstart = variant_piter;
			error = regex_match_impl(&variant_diter, dend,
			                         &variant_piter, content_end,
			                         data, REGEX_CONTEXT_FINPAREN);
			if unlikely(error < 0)
				goto err;
			if (error == 0) {
				/* Variant doesn't result in a match. */
				variant_piter = find_pipe(variant_piter, content_end);
				if (!variant_piter)
					break;
				continue;
			}
			variant_match_length = match_length;
			variant_match_length += data->matlen; /* Include the matched length from the inner match. */
			if (match.mc_max == 1) {
				/* Only a single match is allowed (Skip allocating the backup-vector) */
				suffix_piter = piter;
				error = regex_match_impl(&variant_diter, dend,
				                         &suffix_piter, pend,
				                         data, context | REGEX_CONTEXT_FEMPTYOK);
				if unlikely(error < 0)
					goto err;
				if (error) {
					data->matlen += variant_match_length;
					*ppiter = suffix_piter;
					if unlikely(!data->matlen && !(context & REGEX_CONTEXT_FEMPTYOK))
						return 0;
					*pditer = variant_diter;
					return 1;
				}
				if (match_count <= match.mc_min)
					break;
				continue; /* Try the next variant. */
			}

			/* Found a matching variant. - Do a greedy match
			 * with the remainder of the pattern string. */
			ASSERT(match.mc_max >= 2);
			match_count = 1;

			if (!match.mc_greedy) { /* No need to keep a backup when not being greedy */
				backup_a = 0;
				backup_v = NULL;
			} else {
				backup_a = 4;
				backup_v = (struct match_backup *)Dee_TryMalloc(backup_a * sizeof(struct match_backup));
				if unlikely(!backup_v) {
					backup_a = 1;
					backup_v = (struct match_backup *)Dee_Malloc(backup_a * sizeof(struct match_backup));
					if unlikely(!backup_v)
						goto err;
				}
				backup_v[0].mb_diter  = variant_diter;
				backup_v[0].mb_dcount = variant_match_length;
			}
			while (match_count < match.mc_max) {
				char *post_variant_diter;
				if (!match.mc_greedy && match_count >= match.mc_min) {
					ASSERT(!backup_a);
					ASSERT(!backup_v);
					/* Check if the remainder of input data can not be matched.
					 * -> This is done as early as possible in non-greedy mode. */
					new_diter = diter;
					new_piter = piter;
					error = regex_match_impl(&new_diter, dend,
					                         &new_piter, pend, data,
					                         context | REGEX_CONTEXT_FEMPTYOK);
					if (error != 0)
						goto forward_error; /* Suffix match found! */
				}

				/* Repeat the match however often is possible. */
				variant_piter      = variant_pstart;
				post_variant_diter = variant_diter;
				error = regex_match_impl(&post_variant_diter, dend,
				                         &variant_piter, content_end,
				                         data,
				                         REGEX_CONTEXT_FINPAREN |
				                         /* Pass EMPTYOK, because we check for that case
				                          * explicitly by jumping to `has_infinite_submatch'. */
				                         REGEX_CONTEXT_FEMPTYOK);
				if unlikely(error < 0) {
err_backup_v:
					Dee_Free(backup_v);
					goto err;
				}
				if (!error) {
#if 1 /* Allow the primary variant used for secondary matches to change. */
					/* Check if another variant could be used to match the remainder. */
					char *alt_variant_piter = content_start;
					while (alt_variant_piter < content_end) {
						char *alt_variant_diter;
						char *alt_variant_pstart;
						if (alt_variant_piter == variant_pstart) {
							/* Skip the current primary variant in this suffix-check. */
							alt_variant_piter = find_pipe(alt_variant_piter, content_end);
							if (!alt_variant_piter)
								break;
							continue;
						}
						alt_variant_diter = variant_diter;
						/* Check if we can use this alternative variant. */
						alt_variant_pstart = alt_variant_piter;
						error = regex_match_impl(&alt_variant_diter, dend,
						                         &alt_variant_piter, content_end,
						                         data,
						                         REGEX_CONTEXT_FINPAREN |
						                         /* Pass EMPTYOK, because we check for that case
						                          * explicitly by jumping to `has_infinite_submatch'. */
						                         REGEX_CONTEXT_FEMPTYOK);
						if unlikely(error < 0)
							goto err_backup_v;
						if (error) {
							if (variant_diter == alt_variant_diter)
								goto has_infinite_submatch; /* No data was parsed -> no data was matched (in this case x/0 == INF) */
							/* One of the secondary variant has just started matching.
							 * -> Set it as the new primary variant and remember this match. */
							variant_pstart     = alt_variant_pstart;
							post_variant_diter = alt_variant_diter;
							goto save_variant_match;
						}
						/* Now try the next variant. */
						alt_variant_piter = find_pipe(alt_variant_piter, content_end);
						if (!alt_variant_piter)
							break;
					}
#endif
					break; /* Done matching variant repetitions. */
				}
				if (variant_diter == post_variant_diter)
					goto has_infinite_submatch; /* No data was parsed -> no data was matched (in this case x/0 == INF) */
save_variant_match:
				variant_diter = post_variant_diter;
				variant_match_length += data->matlen;
				if (match.mc_greedy) {
					/* Save the new match entry, so we can undo our greediness if necessary. */
					ASSERT(match_count <= backup_a);
					if unlikely(match_count >= backup_a) {
						size_t new_alloc = backup_a * 2;
						struct match_backup *new_vec;
						new_vec = (struct match_backup *)Dee_TryRealloc(backup_v, new_alloc *
						                                                          sizeof(struct match_backup));
						if unlikely(!new_vec) {
							new_alloc = match_count + 1;
							new_vec = (struct match_backup *)Dee_Realloc(backup_v, new_alloc *
							                                                       sizeof(struct match_backup));
							if unlikely(!new_vec)
								goto err_backup_v;
						}
						backup_a = new_alloc;
						backup_v = new_vec;
					}
					backup_v[match_count].mb_diter  = variant_diter;
					backup_v[match_count].mb_dcount = variant_match_length;
				}
				++match_count;
			}
			/* Check if we managed to parse the minimum requirement. */
			if (match_count < match.mc_min) {
				Dee_Free(backup_v);
				continue; /* Try the next variant. */
			}
has_infinite_submatch:
			if (backup_v) { /* NULL when in non-greedy mode. */
				for (;;) {
					ASSERT(match_count);
					--match_count;
					suffix_piter  = piter;
					variant_diter = backup_v[match_count].mb_diter;
					error = regex_match_impl(&variant_diter, dend,
					                         &suffix_piter, pend,
					                         data, context | REGEX_CONTEXT_FEMPTYOK);
					if unlikely(error < 0)
						goto err_backup_v;
					if (error) {
						data->matlen += backup_v[match_count].mb_dcount;
						*ppiter = suffix_piter;
						Dee_Free(backup_v);
						if unlikely(!data->matlen && !(context & REGEX_CONTEXT_FEMPTYOK))
							return 0;
						*pditer = variant_diter;
						return 1;
					}
					/* Don't go under the minimum number of matches. */
					if (match_count <= match.mc_min)
						break;
					/* Try to be a little less greedy */
				}
				Dee_Free(backup_v);
			}
		}
		/* No matches found */
		if (!match.mc_min)
			goto next; /* No match is ok. */
		goto nope;     /* Miss-matching expression. */
	}	break;

	case '|':
		if (context & REGEX_CONTEXT_FINPAREN)
			goto done;
		DeeError_Throwf(&DeeError_ValueError,
		                "`|' in regular expressions can "
		                "only be used within parenthesis");
		goto err;

	case ')':
	case '*':
	case '+':
	case '?':
	case '{':
	case '}':
	case ']':
		DeeError_Throwf(&DeeError_ValueError,
		                "Encountered unescaped control character "
		                "`%c' in regular expression",
		                (char)ch);
		goto err;

		/* Character trait matching */
	case '\\':
		if unlikely(piter == pend) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Missing character after `\\' in regular expression");
			goto err;
		}
		ch = utf8_readchar((char const **)&piter, pend);
		switch (ch) {
			uniflag_t mask;
			uniflag_t flag;

			case '[': {
			bool negate;
			uniflag_t temp;
			/* USAGE: r"\[TRAITS]"          Match characters implementing all of TRAITS
			 * USAGE: r"\[^TRAITS]"         Match characters implementing none of TRAITS
			 * USAGE: r"\[+TRAITS]"         Match characters implementing any of TRAITS
			 * USAGE: r"\[TRAITS1-TRAITS2]" Match characters implementing all of TRAITS1, but none of TRAITS2 */

			/* Trait-based matching rules. */
			ch = utf8_readchar((char const **)&piter, pend);
			if (ch == '+') {
				flag = 0;
next_anyflag_ch:
				temp = get_regex_trait(ch);
				if (!temp)
					goto err_unknown_trait_char;
				flag |= temp;
				ch = utf8_readchar((char const **)&piter, pend);
				if (!ch)
					goto err_missing_rbracket;
				if (ch != ']')
					goto next_anyflag_ch;
				piter = parse_match_count(piter, pend, &match);
				if unlikely(!piter)
					goto err;
				/* Match any. */
				while (match_count < match.mc_max) {
					char *prev_diter;
					CHECK_NONGREEDY_SUFFIX();
					if (diter >= dend)
						break;
					prev_diter = diter;
					data_ch    = utf8_readchar((char const **)&diter, dend);
					if ((DeeUni_Flags(data_ch) & flag) == 0) {
						diter = prev_diter;
						break;
					}
					++match_count;
					++match_length;
				}
				goto check_match;
			}
			negate = ch == '^';
			if (negate)
				ch = utf8_readchar((char const **)&piter, pend);
			mask = flag = 0;
next_flag_ch:
			temp = get_regex_trait(ch);
			if (!temp) {
err_unknown_trait_char:
				DeeError_Throwf(&DeeError_ValueError,
				                "Unknown character trait character `%I32c' "
				                "(Must be one of \"" REGEX_TRAIT_NAMES "\")");
				goto err;
			}
			flag |= temp;
			ch = utf8_readchar((char const **)&piter, pend);
			if (ch && ch != '-' && ch != ']')
				goto next_flag_ch;
			if (negate)
				flag = ~flag;
			mask = flag;
			if (ch == '-') {
next_mask_ch:
				ch   = utf8_readchar((char const **)&piter, pend);
				temp = get_regex_trait(ch);
				if (!temp)
					goto err_unknown_trait_char;
				flag &= ~temp; /* Disallow in flags. */
				mask |= temp;  /* Include in mask. */
				if (ch && ch != ']')
					goto next_mask_ch;
			}
			if (ch != ']') {
err_missing_rbracket:
				DeeError_Throwf(&DeeError_ValueError,
				                "Missing `]' after `\\[' in regular expression");
				goto err;
			}
			goto match_unicode_trait;
		}

		case 'd':
			mask = UNICODE_FDIGIT;
			flag = UNICODE_FDIGIT;
match_unicode_trait:
			piter = parse_match_count(piter, pend, &match);
			if unlikely(!piter)
				goto err;
			DO_CHARACTERWISE_MATCH((DeeUni_Flags(data_ch) & mask) == flag);
			goto check_match;

		case 'D':
			mask = UNICODE_FDIGIT;
			flag = 0;
			goto match_unicode_trait;

		case 's':
			mask = UNICODE_FSPACE;
			flag = UNICODE_FSPACE;
			goto match_unicode_trait;

		case 'S':
			mask = UNICODE_FSPACE;
			flag = 0;
			goto match_unicode_trait;

		case 'w':
			piter = parse_match_count(piter, pend, &match);
			if unlikely(!piter)
				goto err;
			DO_CHARACTERWISE_MATCH(DeeUni_Flags(data_ch) &
			                       (UNICODE_FALPHA | UNICODE_FDECIMAL |
			                        UNICODE_FSYMSTRT | UNICODE_FSYMCONT));
			goto check_match;
		case 'W':
			mask = UNICODE_FALPHA | UNICODE_FDECIMAL | UNICODE_FSYMSTRT | UNICODE_FSYMCONT;
			flag = 0;
			goto match_unicode_trait;

		case 'n':
			piter = parse_match_count(piter, pend, &match);
			if unlikely(!piter)
				goto err;
			while (match_count < match.mc_max) {
				char *prev_diter;
				CHECK_NONGREEDY_SUFFIX();
				if (diter >= dend)
					break;
				prev_diter = diter;
				data_ch    = utf8_readchar((char const **)&diter, dend);
				if (!DeeUni_IsLF(data_ch)) {
					diter = prev_diter;
					break;
				}
				if (data_ch == '\r' && diter < dend && *diter == '\n')
					++diter, ++match_length; /* CRLF */
				++match_count;
				++match_length;
			}
			goto check_match;

		case 'N':
			mask = UNICODE_FLF;
			flag = 0;
			goto match_unicode_trait;


		case 'A':
			/* Matches only at the start of input data. */
			if (diter <= data->data_start)
				goto next; /* Input data start. */
			goto nope;

		case 'Z':
			/* Matches only at the end of input data. */
			if (diter >= dend)
				goto next; /* Input data end. */
			goto nope;


#define IS_ALNUM_EXTENDED(ch)                                                                     \
	(DeeUni_Flags(data_ch) & (UNICODE_FALPHA | UNICODE_FLOWER | UNICODE_FUPPER | UNICODE_FTITLE | \
	                          UNICODE_FDECIMAL | UNICODE_FSYMSTRT | UNICODE_FSYMCONT))

		case 'b':
		case 'B': {
			char *temp_diter;
			/* Matches only at the start or end of a word */
			if (diter <= data->data_start)
				goto next; /* Input data start -> start of a word */
			/* Check what the previous character was */
			temp_diter = diter;
			data_ch    = utf8_readchar_rev((char const **)&temp_diter, data->data_start);
			if (IS_ALNUM_EXTENDED(data_ch) ^ (ch == 'B')) { /* Allow an upper-case `B' to invert the logic. */
				/* Previous character is apart of a word.
				 * -> Check if the next data character isn't, or is at the end of input data. */
				if (diter >= dend)
					goto next; /* End of input data */
				temp_diter = diter;
				data_ch    = utf8_readchar((char const **)&temp_diter, dend);
				if (!IS_ALNUM_EXTENDED(data_ch))
					goto next; /* The next character isn't an ALNUM */
			} else {
				/* Previous character isn't apart of a word.
				 * -> Check that the next one is! */
				if (diter >= dend)
					goto nope; /* End of input data */
				temp_diter = diter;
				data_ch    = utf8_readchar((char const **)&temp_diter, dend);
				if (IS_ALNUM_EXTENDED(data_ch))
					goto next; /* The next character is an ALNUM */
			}
			goto nope;
		}	break;

		case 'a':
			ch = 0x07;
			goto match_raw_character;
			/*case 'b': ch = 0x08; goto match_raw_character;*/
		case 'f': ch = 0x0c; goto match_raw_character;
		case 'r': ch = 0x0d; goto match_raw_character;
		case 't': ch = 0x09; goto match_raw_character;
		case 'v': ch = 0x0b; goto match_raw_character;
		case 'e':
			ch = 0x1b;
			goto match_raw_character;

			/* TODO: `x' (hex-digit) */

		default: break;
		}
		goto match_raw_character;


		case '[': {
		char *range_start;
		char *range_end;
		bool invert_range;
		/* Character range. */
		range_start = piter;
#if 1
		if (piter < pend && *piter != '\\') /* Same effect, but a bit more efficient. */
#else
		if (piter < pend && *piter == ']')
#endif
		{
			++piter; /* Special case: regex requires that a ']' at the first
			          * position not be used to mark the end of a range, thus
			          * creating an empty one. The following 2 sets are the same:
			          *  - []()[{}]  // RBRACKET at the first position
			          *  - [()[\]{}] // Escaped r-bracket */
		}
		for (;;) {
			if (piter >= pend) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Unexpected end of pattern "
				                "following `[' in regular expression");
				goto err;
			}
			if (*piter == ']')
				break;
			if (*piter == '\\' && piter + 1 < pend)
				++piter;
			++piter;
		}
		range_end = piter;
		++piter; /* Skip the `]' */
		piter = parse_match_count(piter, pend, &match);
		if unlikely(!piter)
			goto err;

		/* Match characters apart of the range. */
		invert_range = false;
		/* Check for match inversion. */
		while (*range_start == '^') {
			invert_range = !invert_range;
			++range_start;
		}
		DO_CHARACTERWISE_MATCH(is_in_range(range_start, range_end, data_ch, data) ^
		                       invert_range);
	}	break;

	case '.':
		piter = parse_match_count(piter, pend, &match);
		if unlikely(!piter)
			goto err;
		if (data->flags & Dee_REGEX_FDOTALL) {
			/* Match anything (must still use a regular iteration
			 * because we must match by character, which can have
			 * different lengths in UTF-8). */
			while (match_count < match.mc_max) {
				CHECK_NONGREEDY_SUFFIX();
				if (diter >= dend)
					break;
				utf8_readchar((char const **)&diter, dend);
				++match_length;
				++match_count;
			}
		} else {
			/* Search for the nearest line-feed, and return its offset */
			DO_CHARACTERWISE_MATCH(!DeeUni_IsLF(data_ch));
		}
		break;

	case '^': {
		char *temp_diter;
		if (diter <= data->data_start)
			goto next; /* Input data start. */
		if (!(data->flags & Dee_REGEX_FMULTILINE))
			goto nope; /* Only accept line-feeds in multi-line mode. */
		/* Check if we're at the start of a line (i.e. the previous data-character was a line-feed) */
		temp_diter = diter;
		data_ch = utf8_readchar_rev((char const **)&temp_diter,
		                            data->data_start);
		if (DeeUni_IsLF(data_ch))
			goto next; /* Yes, it was a line-feed */
	}	goto nope;

	case '$': {
		char *temp_diter;
		if (diter >= dend)
			goto next; /* Input data end. */
		if (!(data->flags & Dee_REGEX_FMULTILINE))
			goto nope; /* Only accept line-feeds in multi-line mode. */
		/* Check if we're at the end of a line (i.e. the next data-character is a line-feed) */
		temp_diter = diter;
		data_ch    = utf8_readchar((char const **)&temp_diter, dend);
		if (DeeUni_IsLF(data_ch))
			goto next; /* Yes, it was a line-feed */
	}	goto nope;

		/* Default: match raw characters. */
	default:
match_raw_character:
		piter = parse_match_count(piter, pend, &match);
		if unlikely(!piter)
			goto err;
		if (data->flags & Dee_REGEX_FNOCASE) {
			ch = DeeUni_ToLower(ch);
			DO_CHARACTERWISE_MATCH(DeeUni_ToLower(data_ch) == ch);
		} else {
			DO_CHARACTERWISE_MATCH(data_ch == ch);
		}
		break;
	}
check_match:
	/* Check what was actually matched. */
	if (match_count < match.mc_min)
		goto nope; /* Too few matches */
	if (match.mc_greedy) {
		while (match_count > match.mc_min) {
			/* Allow matched characters to be undone. */
			new_diter = diter;
			new_piter = piter;
			ASSERT(match_length);
			ASSERT(match_count);
			error = regex_match_impl(&new_diter, dend,
			                         &new_piter, pend,
			                         data,
			                         context |
			                         /* Suffix expressions are always allowed to be empty. */
			                         REGEX_CONTEXT_FEMPTYOK);
			if (error != 0)
				goto forward_error; /* Suffix match found! */
			/* Try to be a little bit less greedy */
			utf8_readchar_rev((char const **)&diter, *pditer);
			--match_length;
			--match_count;
		}
	}
	goto next;
done:
	if unlikely(!match_length && !(context & REGEX_CONTEXT_FEMPTYOK))
		goto nope; /* Don't allow empty matches. */
	*ppiter      = piter;
	*pditer      = diter;
	data->matlen = match_length;
	return 1;
nope:
	*ppiter = piter;
	return 0;
err:
	return -1;
forward_error:
	*ppiter = new_piter;
	data->matlen += match_length;
	if unlikely(!data->matlen && !(context & REGEX_CONTEXT_FEMPTYOK))
		return 0;
	*pditer = new_diter;
	return error;
}


PUBLIC WUNUSED NONNULL((1, 3)) size_t DCALL
DeeRegex_Matches(/*utf-8*/ char const *__restrict data, size_t datalen,
                 /*utf-8*/ char const *__restrict pattern, size_t patternlen,
                 uint16_t flags) {
	struct regex_data rdat;
	int error;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	error = regex_match((char **)&data, (char *)data + datalen,
	                    (char **)&pattern, (char *)pattern + patternlen,
	                    &rdat);
	if unlikely(error < 0)
		return (size_t)-1;
	if unlikely(error == 0)
		return 0;
	return rdat.matlen;
}


PUBLIC WUNUSED NONNULL((1, 3, 5)) size_t DCALL
DeeRegex_MatchesPtr(/*utf-8*/ char const *__restrict data, size_t datalen,
                    /*utf-8*/ char const *__restrict pattern, size_t patternlen,
                    /*utf-8*/ char const **__restrict pdataend,
                    uint16_t flags) {
	struct regex_data rdat;
	int error;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	error = regex_match((char **)&data, (char *)data + datalen,
	                    (char **)&pattern, (char *)pattern + patternlen,
	                    &rdat);
	if unlikely(error < 0)
		return (size_t)-1;
	if unlikely(error == 0)
		return 0;
	*pdataend = data;
	return rdat.matlen;
}



LOCAL WUNUSED bool DCALL is_regex_special(char ch) {
	switch (ch) {

	case '.':
	case '^':
	case '$':
	case '*':
	case '+':
	case '?':
	case '{':
	case '}':
	case '[':
	case ']':
	case '\\':
	case '|':
	case '(':
	case ')':
		return true;

	default: break;
	}
	return false;
}

LOCAL WUNUSED bool DCALL is_regex_suffix(char ch) {
	switch (ch) {

	case '*':
	case '+':
	case '?':
	case '{':
		return true;

	default: break;
	}
	return false;
}


PUBLIC WUNUSED NONNULL((1, 3, 5)) int DCALL
DeeRegex_Find(/*utf-8*/ char const *__restrict data, size_t datalen,
              /*utf-8*/ char const *__restrict pattern, size_t patternlen,
              struct regex_range *__restrict presult, uint16_t flags) {
	uint32_t candidate;
	int error;
	char *piter, *pend, *diter, *dend;
	size_t data_index = 0;
	struct regex_data rdat;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	if unlikely(!patternlen)
		goto nope;
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	dend            = (diter = (char *)data) + datalen;
	pend            = (piter = (char *)pattern) + patternlen;
	candidate       = utf8_readchar((char const **)&piter, pend);
	if ((candidate > 0x7f || !is_regex_special((char)candidate)) &&
	    (piter >= pend || !is_regex_suffix((char)*piter))) {
		/* Search for the first instance of `candidate', simplifying the entire search. */
		while (diter < dend) {
			uint32_t data_ch;
			char *temp_piter;
			data_ch = utf8_readchar((char const **)&diter, dend);
			++data_index;
			if (data_ch != candidate)
				continue;
			if (piter >= pend) {
				/* Got a match! */
				presult->rr_start = data_index - 1;
				presult->rr_end   = data_index;
				return 1;
			}
			temp_piter = piter;
			error      = regex_match(&diter, dend,
			                    &temp_piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start = data_index - 1;
				presult->rr_end   = data_index + rdat.matlen;
				return 1;
			}
		}
	} else {
		/* Fallback: to a manual search on each character individually. */
		while (diter < dend) {
			piter = (char *)pattern;
			error = regex_match(&diter, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start = data_index;
				presult->rr_end   = data_index + rdat.matlen;
				return 1;
			}
			/* Try again with the next data-character. */
			utf8_readchar((char const **)&diter, dend);
			++data_index;
		}
	}
nope:
	return 0;
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1, 2)) size_t DCALL
count_utf8_characters(char *start,
                      char *end) {
	size_t result = 0;
	while (start < end) {
		utf8_readchar((char const **)&start, end);
		++result;
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 3, 5)) int DCALL
DeeRegex_RFind(/*utf-8*/ char const *__restrict data, size_t datalen,
               /*utf-8*/ char const *__restrict pattern, size_t patternlen,
               struct regex_range *__restrict presult, uint16_t flags) {
	uint32_t candidate;
	int error;
	char *piter, *pend, *diter, *dend;
	struct regex_data rdat;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	if unlikely(!patternlen)
		goto nope;
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	dend = diter = (char *)data + datalen;
	pend         = (piter = (char *)pattern) + patternlen;
	candidate    = utf8_readchar((char const **)&piter, pend);
	if ((candidate > 0x7f || !is_regex_special((char)candidate)) &&
	    (piter >= pend || !is_regex_suffix((char)*piter))) {
		/* Search for the first instance of `candidate', simplifying the entire search. */
		while (diter > (char *)data) {
			uint32_t data_ch;
			char *match_begin, *temp_piter;
			data_ch = utf8_readchar_rev((char const **)&diter, data);
			if (data_ch != candidate)
				continue;
			if (piter >= pend) {
				/* Got a match! */
				presult->rr_end   = count_utf8_characters((char *)data, diter);
				presult->rr_start = presult->rr_end - 1;
				return 1;
			}
			match_begin = diter;
			temp_piter  = piter;
			error = regex_match(&diter, dend,
			                    &temp_piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start = count_utf8_characters((char *)data, match_begin);
				presult->rr_end   = presult->rr_start + rdat.matlen;
				--presult->rr_start;
				return 1;
			}
		}
	} else {
		/* Fallback: to a manual search on each character individually. */
		/* Search for the first instance of `candidate', simplifying the entire search. */
		while (diter > (char *)data) {
			char *match_begin;
			utf8_readchar_rev((char const **)&diter, data);
			match_begin = diter;
			piter       = (char *)pattern;
			error = regex_match(&diter, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start = count_utf8_characters((char *)data, match_begin);
				presult->rr_end   = presult->rr_start + rdat.matlen;
				return 1;
			}
		}
	}
nope:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3, 5)) int DCALL
DeeRegex_FindEx(/*utf-8*/ char const *__restrict data, size_t datalen,
                /*utf-8*/ char const *__restrict pattern, size_t patternlen,
                struct regex_range_ex *__restrict presult, uint16_t flags) {
	uint32_t candidate;
	int error;
	char *piter, *pend, *diter, *dend;
	size_t data_index = 0;
	struct regex_data rdat;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	if unlikely(!patternlen)
		goto nope;
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	dend            = (diter = (char *)data) + datalen;
	pend            = (piter = (char *)pattern) + patternlen;
	candidate       = utf8_readchar((char const **)&piter, pend);
	if ((candidate > 0x7f || !is_regex_special((char)candidate)) &&
	    (piter >= pend || !is_regex_suffix((char)*piter))) {
		/* Search for the first instance of `candidate', simplifying the entire search. */
		while (diter < dend) {
			uint32_t data_ch;
			char *temp_piter, *data_start;
			data_start = diter;
			data_ch    = utf8_readchar((char const **)&diter, dend);
			++data_index;
			if (data_ch != candidate)
				continue;
			if (piter >= pend) {
				/* Got a match! */
				presult->rr_start     = data_index - 1;
				presult->rr_end       = data_index;
				presult->rr_start_ptr = data_start;
				presult->rr_end_ptr   = diter;
				return 1;
			}
			temp_piter = piter;
			error = regex_match(&diter, dend,
			                    &temp_piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start     = data_index - 1;
				presult->rr_end       = data_index + rdat.matlen;
				presult->rr_start_ptr = data_start;
				presult->rr_end_ptr   = diter;
				return 1;
			}
		}
	} else {
		/* Fallback: to a manual search on each character individually. */
		while (diter < dend) {
			char *data_start = diter;
			piter            = (char *)pattern;
			error = regex_match(&diter, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start     = data_index;
				presult->rr_end       = data_index + rdat.matlen;
				presult->rr_start_ptr = data_start;
				presult->rr_end_ptr   = diter;
				return 1;
			}
			/* Try again with the next data-character. */
			utf8_readchar((char const **)&diter, dend);
			++data_index;
		}
	}
nope:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3, 5)) int DCALL
DeeRegex_RFindEx(/*utf-8*/ char const *__restrict data, size_t datalen,
                 /*utf-8*/ char const *__restrict pattern, size_t patternlen,
                 struct regex_range_ex *__restrict presult, uint16_t flags) {
	uint32_t candidate;
	int error;
	char *piter, *pend, *diter, *dend;
	struct regex_data rdat;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	if unlikely(!patternlen)
		goto nope;
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	dend = diter = (char *)data + datalen;
	pend         = (piter = (char *)pattern) + patternlen;
	candidate    = utf8_readchar((char const **)&piter, pend);
	if ((candidate > 0x7f || !is_regex_special((char)candidate)) &&
	    (piter >= pend || !is_regex_suffix((char)*piter))) {
		/* Search for the first instance of `candidate', simplifying the entire search. */
		while (diter > (char *)data) {
			uint32_t data_ch;
			char *match_begin, *temp_piter;
			match_begin = diter;
			data_ch     = utf8_readchar_rev((char const **)&diter, data);
			if (data_ch != candidate)
				continue;
			if (piter >= pend) {
				/* Got a match! */
				presult->rr_start     = count_utf8_characters((char *)data, match_begin);
				presult->rr_end       = presult->rr_start + 1;
				presult->rr_start_ptr = match_begin;
				presult->rr_end_ptr   = diter;
				return 1;
			}
			temp_piter = piter;
			error = regex_match(&diter, dend,
			                    &temp_piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start     = count_utf8_characters((char *)data, match_begin);
				presult->rr_end       = presult->rr_start + 1 + rdat.matlen;
				presult->rr_start_ptr = match_begin;
				presult->rr_end_ptr   = diter;
				return 1;
			}
		}
	} else {
		/* Fallback: to a manual search on each character individually. */
		/* Search for the first instance of `candidate', simplifying the entire search. */
		while (diter > (char *)data) {
			char *match_begin;
			utf8_readchar_rev((char const **)&diter, data);
			match_begin = diter;
			piter       = (char *)pattern;
			error = regex_match(&diter, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				presult->rr_start     = count_utf8_characters((char *)data, match_begin);
				presult->rr_end       = presult->rr_start + rdat.matlen;
				presult->rr_start_ptr = match_begin;
				presult->rr_end_ptr   = diter;
				return 1;
			}
		}
	}
nope:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3, 5)) int DCALL
DeeRegex_FindPtr(/*utf-8*/ char const *__restrict data, size_t datalen,
                 /*utf-8*/ char const *__restrict pattern, size_t patternlen,
                 struct regex_range_ptr *__restrict presult, uint16_t flags) {
	int error;
	struct regex_data rdat;
	uint8_t candidate;
	char *piter, *pend, *dend;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	if unlikely(!patternlen)
		goto nope;
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	dend            = (char *)data + datalen;
	pend            = (char *)pattern + patternlen;
	candidate       = (uint8_t)*pattern;
	if (candidate <= 0x7f &&
	    (!is_regex_special((char)candidate)) &&
	    (patternlen < 2 || !is_regex_suffix((char)pattern[1]))) {
		for (;;) {
			char *data_start, *diter;
			data_start = (char *)memchr(data, candidate, datalen);
			if (!data_start)
				goto nope;
			piter = (char *)pattern + 1;
			if (piter >= pend) {
				/* Got a match! */
				presult->rr_start = data_start;
				presult->rr_end   = data_start + 1;
				return 1;
			}
			diter = data_start + 1;
			error = regex_match(&diter, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				/* Got a match! */
				presult->rr_start = data_start;
				presult->rr_end   = diter;
				return 1;
			}
			data    = data_start + 1;
			datalen = (size_t)(dend - (char *)data);
		}
	} else {
		char *diter = (char *)data;
		while (diter < dend) {
			char *dend_ptr = diter;
			piter          = (char *)pattern;
			error = regex_match(&dend_ptr, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				/* Got a match! */
				presult->rr_start = diter;
				presult->rr_end   = dend_ptr;
				return 1;
			}
			++diter;
		}
	}
nope:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3, 5)) int DCALL
DeeRegex_RFindPtr(/*utf-8*/ char const *__restrict data, size_t datalen,
                  /*utf-8*/ char const *__restrict pattern, size_t patternlen,
                  struct regex_range_ptr *__restrict presult, uint16_t flags) {
	int error;
	struct regex_data rdat;
	uint8_t candidate;
	char *piter, *pend, *dend;
	ASSERT(datalen != (size_t)-1);
	ASSERT(patternlen != (size_t)-1);
	if unlikely(!patternlen)
		goto nope;
	rdat.data_start = (char *)data;
	rdat.flags      = flags;
	dend            = (char *)data + datalen;
	pend            = (char *)pattern + patternlen;
	candidate       = (uint8_t)*pattern;
	if (candidate <= 0x7f &&
	    (!is_regex_special((char)candidate)) &&
	    (patternlen < 2 || !is_regex_suffix((char)pattern[1]))) {
		for (;;) {
			char *data_start, *diter;
			data_start = (char *)memrchr(data, candidate, datalen);
			if (!data_start)
				goto nope;
			piter = (char *)pattern + 1;
			if (piter >= pend) {
				/* Got a match! */
				presult->rr_start = data_start;
				presult->rr_end   = data_start + 1;
				return 1;
			}
			diter = data_start + 1;
			error = regex_match(&diter, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				/* Got a match! */
				presult->rr_start = data_start;
				presult->rr_end   = diter;
				return 1;
			}
			datalen = (size_t)(data_start - (char *)data);
		}
	} else {
		char *diter = dend;
		while (diter > (char *)data) {
			char *dend_ptr;
			--diter;
			dend_ptr = diter;
			piter    = (char *)pattern;
			error = regex_match(&dend_ptr, dend,
			                    &piter, pend,
			                    &rdat);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				/* Got a match! */
				presult->rr_start = diter;
				presult->rr_end   = dend_ptr;
				return 1;
			}
		}
	}
nope:
	return 0;
err:
	return -1;
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGEX_C */
