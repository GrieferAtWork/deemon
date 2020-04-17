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
#ifndef GUARD_DEEMON_COMPILER_LEXER_DOCTEXT_C
#define GUARD_DEEMON_COMPILER_LEXER_DOCTEXT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/doctext.h>

#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <string.h>

DECL_BEGIN

/* Returns a new pointer for `start' */
PRIVATE NONNULL((1, 2)) /*utf-8*/ char const *DCALL
lstrip_whitespace(/*utf-8*/ char const *start,
                  /*utf-8*/ char const *end) {
	for (;;) {
		uint32_t ch;
		char const *ch_start;
		ch_start = start;
		ch = utf8_readchar((char const **)&start, end);
		if (!DeeUni_IsSpace(ch)) {
			start = ch_start;
			break;
		}
	}
	return start;
}

/* Returns a new pointer for `end' */
PRIVATE NONNULL((1, 2)) /*utf-8*/ char const *DCALL
rstrip_whitespace(/*utf-8*/ char const *start,
                  /*utf-8*/ char const *end) {
	for (;;) {
		uint32_t ch;
		char const *ch_end;
		ch_end = end;
		ch = utf8_readchar_rev((char const **)&end, start);
		if (!DeeUni_IsSpace(ch)) {
			end = ch_end;
			break;
		}
	}
	return end;
}

/* Check if the given start/end pointers contain a symbol-string */
PRIVATE NONNULL((1, 2)) bool DCALL
is_symbol(/*utf-8*/ char const *start,
          /*utf-8*/ char const *end) {
	uint32_t ch;
	ch = utf8_readchar((char const **)&start, end);
	if (!DeeUni_IsSymStrt(ch))
		goto no;
	for (;;) {
		ch = utf8_readchar((char const **)&start, end);
		if (!ch && (start >= end))
			break; /* Done */
		if (!DeeUni_IsSymCont(ch)) {
			/* Special case: \_ will be encoded as _, which is a symbol character,
			 *               so if a \ is followed by _, then we can still consider
			 *               it as a symbol character here! */
			if (ch == '\\') {
				ch = utf8_readchar((char const **)&start, end);
				if (DeeUni_IsSymCont(ch))
					continue;
			}
			goto no;
		}
	}
	return true;
no:
	return false;
}


/* Strip trailing characters from `printer' until its length <= stop_at,
 * or until its last character no longer is a whitespace character. */
PRIVATE NONNULL((1)) void DCALL
strip_trailing_whitespace_until(struct unicode_printer *__restrict printer,
                                size_t stop_at) {
	size_t len = UNICODE_PRINTER_LENGTH(printer);
	while (len > stop_at) {
		uint32_t ch;
		--len;
		ch = UNICODE_PRINTER_GETCHAR(printer, len);
		if (!DeeUni_IsSpace(ch))
			break;
		printer->up_length = len;
	}
}

PRIVATE NONNULL((1, 2)) bool DCALL
contains_only_decimals_dot_collon_or_backslash(/*utf-8*/ char const *text,
                                               /*utf-8*/ char const *end) {
	uint32_t ch;
	for (;;) {
		ch = utf8_readchar((char const **)&text, end);
		if (!ch && (text >= end))
			break;
		if (DeeUni_IsDecimal(ch))
			continue;
		if (ch == '.')
			continue;
		if (ch == ':')
			continue;
		if (ch == '\\')
			continue;
		goto no;
	}
	return true;
no:
	return false;
}


/* Find the first non-matching, non-escaped `find_close' character that
 * isn't balanced by another non-escaped `find_open'. If no such character
 * exists between text...end, return NULL instead. */
PRIVATE NONNULL((1, 2)) char const *DCALL
find_nonescaped_match(/*utf-8*/ char const *text,
                      /*utf-8*/ char const *end,
                      char find_open, char find_close) {
	unsigned int recursion = 0;
	for (;;) {
		char const *ch_start;
		uint32_t ch;
		ch_start = text;
		ch = utf8_readchar((char const **)&text, end);
		if (!ch && (text >= end))
			break;
		if (ch == '\\') {
			utf8_readchar((char const **)&text, end);
			continue;
		}
		if (ch == (unsigned char)find_open) {
			++recursion;
			continue;
		}
		if (ch == (unsigned char)find_close) {
			if (!recursion)
				return ch_start;
			--recursion;
			continue;
		}
	}
	return NULL;
}


/* Print the given text...end whilst escaping the
 * following characters by prefixing a # to each of them:
 *      # $ % & ~ ^ { } [ ] | ? * @ - +
 */
PRIVATE NONNULL((1, 2, 3)) int DCALL
print_escaped(struct unicode_printer *__restrict printer,
              /*utf-8*/ char const *text,
              /*utf-8*/ char const *end) {
	char const *iter, *flush_start;
	iter = flush_start = text;
	for (;;) {
		uint32_t ch;
		char const *ch_start;
		ch_start = iter;
		ch = utf8_readchar((char const **)&text, end);
		if (!ch && (text >= end))
			break;
		if (ch == '#' || ch == '$' || ch == '%' || ch == '&' || ch == '~' ||
		    ch == '^' || ch == '{' || ch == '}' || ch == '[' || ch == ']' ||
		    ch == '|' || ch == '?' || ch == '*' || ch == '@' || ch == '-' ||
		    ch == '+') {
			if unlikely(unicode_printer_print(printer, flush_start,
			                                  (size_t)(ch_start - flush_start)) < 0)
				goto err;
			if unlikely(unicode_printer_putascii(printer, '#'))
				goto err;
			flush_start = ch_start;
		}
	}
	if unlikely(unicode_printer_print(printer, flush_start,
	                                  (size_t)(end - flush_start)) < 0)
		goto err;
	return 0;
err:
	return -1;
}


PRIVATE NONNULL((1, 2, 3)) int DCALL
do_compile(/*utf-8*/ char const *text,
           /*utf-8*/ char const *end,
           struct unicode_printer *__restrict result_printer,
           /*nullable*/ struct unicode_printer *source_printer) {
	uint32_t ch;
	char const *ch_start;
	char const *iter, *flush_start;
	char const *current_line_start;
	char const *current_line_start_after_whitespace;
	size_t result_printer_origlen;
	size_t current_line_leading_spaces; /* # of leading spaces within the current line */
	size_t min_line_leading_spaces;     /* Lowest # of leading spaces in any line */
#define DO(expr)                \
	do {                        \
		if unlikely((expr) < 0) \
			goto err;           \
	} __WHILE0
#define FLUSHTO(flushto)                                                        \
	do {                                                                        \
		ASSERT((flushto) >= flush_start);                                       \
		if unlikely(unicode_printer_print(result_printer, flush_start,          \
		                                  (size_t)((flushto)-flush_start)) < 0) \
			goto err;                                                           \
	} __WHILE0
#define PUTUNI(ch)                                       \
	do {                                                 \
		if (unicode_printer_put32(result_printer, (ch))) \
			goto err;                                    \
	} __WHILE0
#define PUTASCII(ch)                                        \
	do {                                                    \
		if (unicode_printer_putascii(result_printer, (ch))) \
			goto err;                                       \
	} __WHILE0
#define PRINT(ptr, len)      DO(unicode_printer_print(result_printer, ptr, len))
#define PRINTASCII(ptr, len) DO(unicode_printer_printascii(result_printer, ptr, len))

	/* Strip trailing whitespace. */
	for (;;) {
		char const *old_end;
		uint32_t ch;
		old_end = end;
		ch = utf8_readchar_rev((char const **)&end, text);
		if (DeeUni_IsSpace(ch) || DeeUni_IsLF(ch))
			continue;
		end = old_end;
		break;
	}
	iter                        = text;
	current_line_start          = text;
	current_line_leading_spaces = 0;
	/* Count the number of leading whitespace characters, and
	 * skip all leading lines that are empty, or contain only
	 * white-space characters. */
	for (;;) {
		uint32_t ch;
		char const *ch_start;
		ch_start = iter;
		ch = utf8_readchar((char const **)&iter, end);
		if (DeeUni_IsLF(ch)) {
			current_line_start          = iter;
			current_line_leading_spaces = 0;
			continue;
		}
		if (DeeUni_IsSpace(ch)) {
			++current_line_leading_spaces;
			continue;
		}
		iter = ch_start;
		break;
	}
	/* Set the start of the first non-empty line at starting point for data being flushed. */
	current_line_start_after_whitespace = iter;
	flush_start             = current_line_start;
	min_line_leading_spaces = current_line_leading_spaces;
	result_printer_origlen  = result_printer->up_length;

	ch_start = iter;
	ch       = utf8_readchar((char const **)&iter, end);
	goto do_switch_ch_at_start_of_line;
	for (;;) {
do_read_and_switch_ch:
		ch_start = iter;
		ch = utf8_readchar((char const **)&iter, end);
do_switch_ch:
		ASSERT(ch_start >= flush_start);
		ASSERT(iter >= flush_start);
		switch (ch) {

		case 0:
			if (iter >= end)
				goto done;
			break;

		/* Escape special characters when they are used in user-defined doc strings.
		 * Escaping is done by inserting an additional `#' character */
		case '$': case '%': case '&': case '#':
		case '^': case '{': case '}': case '+':
		case ']': case '|': case '?': case '-':
			/* NOTE: '*', '~' and '@' are markdown format characters and handled separately. */
escape_current_character:
			FLUSHTO(ch_start);
			PUTASCII('#');
			flush_start = ch_start;
			break;

		case '\r':
		case '\n': {
			uint32_t nextchar;
			char const *next_line_start;
			size_t next_line_leading_spaces;
			bool has_explicit_linefeed;
do_set_current_line:
			FLUSHTO(ch_start);
do_set_current_line_noflush:
			ch_start = iter;
			nextchar = utf8_readchar((char const **)&iter, end);
			if (ch == '\r' && nextchar == '\n') {
				ch_start = iter;
				nextchar = utf8_readchar((char const **)&iter, end);
			}
			ch = nextchar;
			/* Count the # of leading spaces within the next line.
			 * Also check if the next is empty (except for space characters),
			 * in which case we must print a single line-feed for the 2 line,
			 * followed by possibly more line-feeds for any additional lines
			 * followed there-after. */
			has_explicit_linefeed = false;
			__IF0 {
scan_newline_with_first_ch_and_explicit_linefeed:
				has_explicit_linefeed = true;
			}
			next_line_start = ch_start;
			next_line_leading_spaces = 0;
			goto check_ch_after_lf;
			for (;;) {
				ch_start = iter;
				ch = utf8_readchar((char const **)&iter, end);
check_ch_after_lf:
				if (DeeUni_IsLF(ch)) {
					next_line_start          = iter;
					next_line_leading_spaces = 0;
					PUTASCII('\n'); /* Force a line-feed here. */
					has_explicit_linefeed = true;
					continue;
				}
				if (DeeUni_IsSpace(ch)) {
					++next_line_leading_spaces;
					continue;
				}
				if (!ch && (iter >= end))
					goto done_dontflush; /* Shouldn't happen (trailing space/line-feeds are stripped above) */
				/* Don't re-wind, so below code can simply `do_switch_ch' */
				break;
			}
			/* Compare the # of leading spaces to determine how we
			 * should join this line with the one that follows. */
			flush_start = next_line_start;
			if (!has_explicit_linefeed) {
				if (next_line_leading_spaces < current_line_leading_spaces) {
					/* The next line is indented less -> Join via line-feed */
					PUTASCII('\n'); /* Force a line-feed here. */
				} else {
					/* The next line is the same or more -> Join via space */
					PUTASCII(' '); /* Force a space character */
					flush_start = ch_start;
				}
			}
			current_line_start_after_whitespace = ch_start;
			current_line_start          = next_line_start;
			current_line_leading_spaces = next_line_leading_spaces;
			/* Track the min # of leading spaces for all lines. */
			if (min_line_leading_spaces > current_line_leading_spaces)
				min_line_leading_spaces = current_line_leading_spaces;
do_switch_ch_at_start_of_line:
			switch (ch) {

			case '#':
				/* TODO: Header */
				break;

				/* TODO: When starting a List or Table, `result_printer' must be modified
				 *       such that all trailing white-space characters are first stripped,
				 *       before an additional line-feed is inserted (if no line-feed was
				 *       present already). */

			case '|':
			case '+':
				; /* TODO: Table */
				ATTR_FALLTHROUGH
			case '-':
			case '*':
				/* TODO: Unordered list */
				/* NOTE: Be careful, as `*' can also be used for italics! */
				break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				/* TODO: Ordered list */
				break;

			default:
				break;
			}
			/*goto do_switch_ch_after_whitespace_or_lf;*/
			goto do_switch_ch;
		}	break;

		case '\\': {
			/* Construct escaping */
			uint32_t escaped_ch;
			char const *escaped_ch_start;
			escaped_ch_start = iter;
			escaped_ch = utf8_readchar((char const **)&iter, end);
			switch (escaped_ch) {

			case '\r': {
				/* When escaping a line-feed, also check for windows-style linefeeds */
				char const *temp = iter;
				uint32_t second_lf_char;
				second_lf_char = utf8_readchar((char const **)&iter, end);
				if (second_lf_char != '\n')
					iter = temp; /* Only include the second character in the escape if it's a \n */
			}	ATTR_FALLTHROUGH
			case '\n': /* Escape line-feeds to force them to appear in the output */
				/* Strip trailing whitespace. */
				for (;;) {
					uint32_t trailing_space_character;
					char const *orig_ch_start;
					orig_ch_start = ch_start;
					trailing_space_character = utf8_readchar_rev((char const **)&ch_start, flush_start);
					if (trailing_space_character == 0 && (ch_start <= flush_start)) {
						strip_trailing_whitespace_until(result_printer, result_printer_origlen);
						break;
					}
					if (DeeUni_IsSpace(trailing_space_character))
						continue;
					/* Don't strip this character as well! */
					ch_start = orig_ch_start;
					break;
				}
				FLUSHTO(ch_start);
				PUTASCII('\n');
				/* Due to how user-defined documentation strings are written, and how deemon interacts
				 * with TPP, an escaped line-feed has some special quirks by also causing the @@ to be
				 * included within the documentation string that we've been given:
				 * >> @@First line \               <Put some text here to prevent C compiler warnings>
				 * >> @@Second line
				 *    ^
				 *    iter
				 * In this case, the raw string would look like:
				 *    "First line \\\n@@Second line"
				 * Try to work around this quirk, and hide this fact by scanning ahead to check for the
				 * double-@@, setting the `flush_start' pointer after it to act as through that's where
				 * the line actually began. */
				{
					char const *orig_line_start;
					orig_line_start = iter;
					for (;;) {
						ch = utf8_readchar((char const **)&iter, end);
						if (DeeUni_IsLF(ch) || !ch)
							break;
						if (DeeUni_IsSpace(ch))
							continue;
						if (ch == '/') {
							/* Another special case: C-style comments could appear before
							 * the start of the documentation string on this line. - Since
							 * we're trying to act like TPP hadn't escaped the linefeed, we
							 * must also remove such a comment. */
							ch = utf8_readchar((char const **)&iter, end);
							if (ch != '*')
								break;
							do {
								ch = utf8_readchar((char const **)&iter, end);
again_search_for_c_comment_end_after_escaped_linefeed:
								;
							} while (ch && ch != '*');
							if (ch != '*')
								break;
							ch = utf8_readchar((char const **)&iter, end);
							if (ch != '/')
								goto again_search_for_c_comment_end_after_escaped_linefeed;
							continue;
						}
						if (ch == '@') {
							ch = utf8_readchar((char const **)&iter, end);
							if (ch != '@')
								break;
							/* The double-@ has been found! */
							goto set_iter_as_start_of_line_after_explicit_linefeed;
						}
					}
					iter = orig_line_start;
				}
set_iter_as_start_of_line_after_explicit_linefeed:
				flush_start = iter;
				ch_start    = iter;
				ch = utf8_readchar((char const **)&iter, end);
				goto scan_newline_with_first_ch_and_explicit_linefeed;

			case ' ':  /* Escape space characters to force them to be kept */
			case '\\': /* Escape the escape character itself to force it to be inserted */
			case '_':  /* _Bold_ */
			case '@':  /* Reference */
			case '`':  /* Code */
			case '[': case ']': /* Link */
			case '(': case ')': /* Link */
			case '-': case '+': /* List / Table */
			case '|': case '=': /* Table */
escape_remove_backslash_and_keep_next_char:
				FLUSHTO(ch_start);
				flush_start = escaped_ch_start;
				break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '.': case ':': /* List */
check_ordered_list_digit:
				/* Ordered list (only at the start of a line) */
				if (ch_start == current_line_start_after_whitespace)
					goto escape_remove_backslash_and_keep_next_char;
				/* Also allow the backslash to appear somewhere where
				 * it is only preceded by digits or . or : characters */
				if (contains_only_decimals_dot_collon_or_backslash(current_line_start_after_whitespace,
				                                                   ch_start))
					goto escape_remove_backslash_and_keep_next_char;
				break;

			case '~': /* ~Strikethrough~ */
			case '*': /* *Italic* or List */
			case '#': /* Header */
				FLUSHTO(ch_start);
				PUTASCII('#');
				flush_start = escaped_ch_start;
				/* Check if this is escaping an extended block. */
				ch_start = iter;
				ch = utf8_readchar((char const **)&iter, end);
				if (ch == escaped_ch)
					goto do_switch_ch; /* Yes -> Also escape the second character */
				goto do_read_and_switch_ch;

			default:
				if (DeeUni_IsLF(escaped_ch))
					goto escape_remove_backslash_and_keep_next_char;
				if (DeeUni_IsSpace(escaped_ch))
					goto escape_remove_backslash_and_keep_next_char;
				if (DeeUni_IsDecimal(escaped_ch))
					goto check_ordered_list_digit;
				/* Fallback: Don't remove anything (including the backslash) */
				break;
			}
		}	break;

		case '_': { /* _bold_ */
			/* Make sure that _ isn't considered a construct
			 * character when it appears inside of a word! */
			uint32_t prev_ch;
			char const *orig_iter;
			orig_iter = iter;
			iter      = ch_start;
			do {
				prev_ch = utf8_readchar_rev((char const **)&iter, text);
			} while (prev_ch == '_');
			iter = orig_iter;
			if (DeeUni_IsSymCont(prev_ch) || prev_ch == '\\')
				break; /* Not a construct! */
		}	ATTR_FALLTHROUGH
		case '*': /* *italic*         (NOTE: Must be escaped) */
		case '~': /* ~strikethrough~  (NOTE: Must be escaped) */
		{
			uint32_t nextch;
			char const *after_first_ch, *construct_start;
			char const *body_start, *body_end;
			construct_start = ch_start;
			after_first_ch = iter;
			nextch = utf8_readchar((char const **)&iter, end);
			if (nextch == ch) {
				/* Extended attribute block.
				 * -> Search for the end of the block and re-parse the body */
				bool did_find_single = false;
				body_start = iter;
				for (;;) {
					body_end = iter;
					nextch = utf8_readchar((char const **)&iter, end);
					if (nextch == ch) {
						did_find_single = true;
						nextch = utf8_readchar((char const **)&iter, end);
						if (nextch != ch)
							continue;
						break;
					}
					if (DeeUni_IsLF(nextch) || (!nextch && (iter >= end))) {
						/* Unterminated attribute block...
						 * Raw-forward the header and continue parsing what
						 * would have been the body. */
						char buf[4];
						size_t count = 2;
						buf[0] = buf[1] = (char)ch;
						if (ch == '*' || ch == '~') {
							buf[0] = buf[2] = '#';
							buf[1] = buf[3] = (char)ch;
							count  = 4;
						}
						FLUSHTO(construct_start);
						if (did_find_single) {
							PRINTASCII(buf, count / 2);
							iter = construct_start;
							utf8_readchar((char const **)&iter, end);
							flush_start = iter;
						} else {
							PRINTASCII(buf, count);
							iter        = body_start;
							flush_start = body_start;
						}
						goto do_read_and_switch_ch;
					}
					if (nextch == '\\') /* Escaped the next character. */
						utf8_readchar((char const **)&iter, end);
				}
			} else {
				/* Search until a space/line-feed character is found, or the construct is ended correctly */
				body_start = after_first_ch;
				for (;;) {
					if (DeeUni_IsSpace(nextch) ||
					    DeeUni_IsLF(nextch) || (!nextch && iter >= end)) {
						iter = body_start;
						if (ch == '*' || ch == '~')
							goto escape_current_character;
						goto do_read_and_switch_ch;
					}
					if (nextch == ch) {
						if (ch == '_') {
							/* Special case: `_' must be followed by a non-symcont character to count here!
							 *   We don't want to get an early exit for something like _this_and_that_,
							 *   which should be encoded as #Bthis_and_that and not #B{this}and_that_ */
							char const *followup;
							followup = iter;
							do {
								nextch = utf8_readchar((char const **)&iter, end);
							} while (nextch == '_');
							if (!DeeUni_IsSymCont(nextch)) {
								iter = followup;
								break; /* Yup! this really is the end! */
							}
							body_end = followup;
							continue; /* Continue still! */
						}
						break; /* Found the end! */
					}
					if (nextch == '\\') /* Escaped the next character. */
						utf8_readchar((char const **)&iter, end);
					body_end = iter;
					nextch   = utf8_readchar((char const **)&iter, end);
				}
			}
			/* Got the body:
			 *  construct_start
			 *                v
			 *          First **Bold Bold** Second
			 *                 ^^        ^ ^
			 *                 ||        | |
			 *    after_first_ch| body_end |
			 *         body_start       iter
			 *
			 *  construct_start
			 *                v
			 *          First *Bold* Second
			 *                 ^   ^^
			 *                 |   ||
			 *                 |   |iter
			 *        body_start   body_end
			 */
			FLUSHTO(construct_start);
			{
				char buf[2];
				buf[0] = '#';
				if (ch == '_')
					buf[1] = 'B'; /* Bold */
				else if (ch == '*')
					buf[1] = 'I'; /* Italic */
				else {
					buf[1] = 'S'; /* Strikethrough */
				}
				PRINTASCII(buf, 2);
			}
			/* Check if the body qualifies for being a symbol */
			flush_start = iter;
			ch_start = iter;
			ch = utf8_readchar((char const **)&iter, end);
			{
				bool need_brace;
				/* Special case: The component is followed by more symbol-continue characters.
				 * In this case, we must still enclose our portion with braces, so-as to prevent
				 * the follow-up text also being considered apart of our construct! */
				need_brace = DeeUni_IsSymCont(ch) || !is_symbol(body_start, body_end);
				if (need_brace)
					PUTASCII('{');
				/* Recursively compile the contained body. */
				if unlikely(do_compile(body_start, body_end, result_printer, NULL))
					goto err;
				if (need_brace)
					PUTASCII('}');
			}
			goto do_switch_ch;
		}	break;


		case '[': { /* Link */
			char const *before_lbracket, *after_lbracket;
			char const *before_rbracket;
			char const *after_lparen;
			char const *before_rparen, *after_rparen;
			bool has_leading_space;
			bool has_trailing_space;
			before_lbracket = ch_start;
			after_lbracket  = iter;
			/* Search for the matching end-character */
			before_rbracket = find_nonescaped_match(after_lbracket, end, '[', ']');
			if (!before_rbracket) {
not_a_link:
				ch_start = before_lbracket;
				iter     = after_lbracket;
				goto escape_current_character;
			}
			after_lparen = before_rbracket;
			utf8_readchar_u((char const **)&after_lparen);
			ch = utf8_readchar((char const **)&after_lparen, end);
			if (ch != '(')
				goto not_a_link;
			before_rparen = find_nonescaped_match(after_lparen, end, '(', ')');
			if (!before_rparen)
				goto not_a_link;
			after_rparen = before_rparen;
			utf8_readchar_u((char const **)&after_rparen);
			/* Flush up until the leading [-character */
			FLUSHTO(before_lbracket);
			/* Check if first part has leading/trailing space, and strip that space. */
			has_leading_space = has_trailing_space = false;
			for (;;) {
				char const *temp;
				temp = after_lbracket;
				ch = utf8_readchar((char const **)&after_lbracket, before_rbracket);
				if (DeeUni_IsSpace(ch)) {
					has_leading_space = true;
					continue;
				}
				after_lbracket = temp;
				break;
			}
			for (;;) {
				char const *temp;
				temp = before_rbracket;
				ch = utf8_readchar_rev((char const **)&before_rbracket, after_lbracket);
				if (DeeUni_IsSpace(ch)) {
					has_trailing_space = true;
					continue;
				}
				before_rbracket = temp;
				break;
			}
			/* If there is leading space, and the printer doesn't already end with a space
			 * character, then print one additional space character before the link. */
			if (has_leading_space && UNICODE_PRINTER_LENGTH(result_printer) > result_printer_origlen) {
				uint32_t lastch;
				lastch = UNICODE_PRINTER_GETCHAR(result_printer, UNICODE_PRINTER_LENGTH(result_printer) - 1);
				if (!DeeUni_IsSpace(lastch))
					PUTASCII(' ');
			}
			/* Strip whitespace from the link portion. */
			after_lparen  = lstrip_whitespace(after_lparen, before_rparen);
			before_rparen = rstrip_whitespace(after_lparen, before_rparen);
			PRINT("#A{", 3);
			/* Print the link text. */
			if unlikely(print_escaped(result_printer, after_lparen, before_rparen))
				goto err;
			PUTASCII('|');
			/* Print the link body */
			if unlikely(do_compile(after_lbracket, before_rbracket, result_printer, NULL))
				goto err;
			PUTASCII('}');
			/* If there was trailing space within the link text, re-insert that space after the link. */
			iter = after_rparen;
			if (has_trailing_space) {
				PUTASCII(' ');
				/* Skip all additional space characters following the link */
				for (;;) {
					ch_start = iter;
					ch = utf8_readchar((char const **)&iter, end);
					if (!DeeUni_IsSpace(ch) || (!ch && iter >= end))
						break;
				}
				flush_start = ch_start;
				goto do_switch_ch;
			}
			flush_start = iter;
			goto do_read_and_switch_ch;
		}

		case '`':
			/* TODO: Code */
			break;

		case '@':
			/* TODO: Reference */
			goto escape_current_character;


		default:
			if (DeeUni_IsLF(ch))
				goto do_set_current_line;
			if (DeeUni_IsSpace(ch)) {
				/* Remove multiple consecutive space characters */
				char const *before_first_space;
				char const *after_first_space;
		case ' ':
				before_first_space = ch_start;
				after_first_space  = iter;
				for (;;) {
					uint32_t second_space;
					ch_start     = iter;
					second_space = utf8_readchar((char const **)&iter, end);
					if (DeeUni_IsLF(second_space)) {
						FLUSHTO(before_first_space);
						goto do_set_current_line_noflush;
					}
					if (!DeeUni_IsSpace(second_space)) {
						/* Convert all space characters to ` ' */
						if (ch == ' ') {
							FLUSHTO(after_first_space);
						} else {
							FLUSHTO(before_first_space);
							PUTASCII(' ');
						}
						flush_start = ch_start;
						ch = second_space;
/*do_switch_ch_after_whitespace_or_lf:*/
						goto do_switch_ch;
					}
					if (!second_space && (iter >= end))
						goto done_dontflush; /* Shouldn't happen (trailing space/line-feeds are stripped above) */
				}
			}
			break;
		}
	}
done:
	if (flush_start == text && UNICODE_PRINTER_ISEMPTY(result_printer) && source_printer) {
		/* Steal all data from the source printer (that way we don't have to copy anything!). */
		memcpy(result_printer, source_printer, sizeof(struct unicode_printer));
		unicode_printer_init(source_printer);
	} else {
		if unlikely(unicode_printer_print(result_printer, flush_start,
		                                  (size_t)(end - flush_start)) < 0)
			goto err;
	}
done_dontflush:
	if (min_line_leading_spaces != 0) {
		/* Take everything after `result_printer_origlen' and
		 * delete the first `min_line_leading_spaces' characters
		 * at the start, and after every line-feed found thereafter. */
		size_t i = result_printer_origlen;
		unicode_printer_erase(result_printer, i, min_line_leading_spaces);
		while (i < UNICODE_PRINTER_LENGTH(result_printer)) {
			uint32_t ch;
			ch = UNICODE_PRINTER_GETCHAR(result_printer, i);
			++i;
			if (!DeeUni_IsLF(ch))
				continue;
			if (ch == '\r') {
				/* Check if the next character is a \n, in which case: erase after that one */
				ch = UNICODE_PRINTER_GETCHAR(result_printer, i);
				if (ch == '\n')
					++i;
			}
			unicode_printer_erase(result_printer, i, min_line_leading_spaces);
		}
	}
	return 0;
err:
	return -1;
#undef FLUSHTO
}

/* Compile documentation text in `doctext' into itself.
 * This function scans `doctext' according to `FORMAT',
 * then re-writes `doctext' to contain the equivalent
 * as described by `ENCODING'.
 * NOTE: This function should be called by the compiler
 *       in the context of the declaration being annotated,
 *       such that in the case of a function being annotated,
 *       argument variables are visible. */
INTERN int DCALL
doctext_compile(struct unicode_printer *__restrict doctext) {
	/* Fast-pass: no documentation text defined. */
	if (UNICODE_PRINTER_ISEMPTY(doctext))
		goto done;
	if likely((doctext->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		struct unicode_printer result = UNICODE_PRINTER_INIT;
		/* Directly compile the original documentation text. */
		if unlikely(do_compile((/*utf-8*/ char const *)doctext->up_buffer,
		                       (/*utf-8*/ char const *)doctext->up_buffer + doctext->up_length, &result, doctext)) {
			unicode_printer_fini(&result);
			goto err;
		}
		unicode_printer_fini(doctext);
		memcpy(doctext, &result, sizeof(struct unicode_printer));
	} else {
		/* Re-package as a 1-byte, utf-8 string. */
		DREF DeeStringObject *rawtext;
		/*utf-8*/ char const *rawutf8;
		rawtext = (DREF DeeStringObject *)unicode_printer_pack(doctext);
		unicode_printer_init(doctext);
		if unlikely(!rawtext)
			goto err;
		/* Convert to utf-8 */
		rawutf8 = DeeString_AsUtf8((DeeObject *)rawtext);
		if unlikely(!rawutf8) {
err_rawtext:
			Dee_Decref_likely(rawtext);
			goto err;
		}
		/* Compile the utf-8 variant of the documentation string. */
		if unlikely(do_compile(rawutf8, rawutf8 + WSTR_LENGTH(rawutf8), doctext, NULL))
			goto err_rawtext;
		Dee_Decref_likely(rawtext);
	}
done:
	return 0;
err:
	return -1;
}

DECL_END

#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */

#endif /* !GUARD_DEEMON_COMPILER_LEXER_DOCTEXT_C */
