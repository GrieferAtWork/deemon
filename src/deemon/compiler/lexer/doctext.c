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
	size_t len = printer->up_length;
	for (;;) {
		uint32_t ch;
		if (len <= stop_at)
			break;
		ch = UNICODE_PRINTER_GETCHAR(printer, len - 1);
		if (DeeUni_IsLF(ch))
			break;
		if (!DeeUni_IsSpace(ch))
			break;
		--len;
	}
	printer->up_length = len;
}

PRIVATE NONNULL((1)) void DCALL
strip_trailing_whitespace(struct unicode_printer *__restrict printer,
                          size_t stop_at) {
	size_t len = printer->up_length;
	for (;;) {
		uint32_t ch;
		if (len <= stop_at)
			break;
		ch = UNICODE_PRINTER_GETCHAR(printer, len - 1);
		if (!DeeUni_IsSpace(ch))
			break;
		--len;
	}
	printer->up_length = len;
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


/* Print the given string `text ... end' to `printer', but unescape
 * all \|-sequences into |-characters. - This function is used by the
 * line-printer functions used by tables. */
PRIVATE NONNULL((1, 2, 3)) int DCALL
reprint_but_unescape_backslash_pipe(struct unicode_printer *__restrict printer,
                                    /*utf-8*/ char const *text,
                                    /*utf-8*/ char const *end) {
	char const *flush_start;
	flush_start = text;
	for (;;) {
		uint32_t ch;
		char const *ch_start, *escape_start;
		ch_start = text;
		ch = utf8_readchar((char const **)&text, end);
		if (!ch && (text >= end))
			break;
		if (ch != '\\')
			continue;
		escape_start = text;
		ch = utf8_readchar((char const **)&text, end);
		if (!ch && (text >= end))
			break;
		if (ch == '|' || ch == '\\') {
			/* Escape */
			if unlikely(unicode_printer_print(printer, flush_start,
			                                  (size_t)(ch_start - flush_start)) < 0)
				goto err;
			flush_start = escape_start;
		}
	}
	if unlikely(unicode_printer_print(printer, flush_start,
	                                  (size_t)(end - flush_start)) < 0)
		goto err;
	return 0;
err:
	return -1;
}


/* Print the given text...end whilst escaping the
 * following characters by prefixing a # to each of them:
 *      # $ % & ~ ^ { } [ ] | ? * @ - + :
 * Additionally, skip over \-characters that are followed by one of:
 *      \ _ @ ` [ ] ( ) - + | = ~ * # :
 */
PRIVATE NONNULL((1, 2, 3)) int DCALL
print_escaped(struct unicode_printer *__restrict printer,
              /*utf-8*/ char const *text,
              /*utf-8*/ char const *end) {
	char const *flush_start;
	flush_start = text;
	for (;;) {
		uint32_t ch;
		char const *ch_start;
		ch_start = text;
		ch = utf8_readchar((char const **)&text, end);
		if (!ch && (text >= end))
			break;
		if (ch == '#' || ch == '$' || ch == '%' || ch == '&' || ch == '~' ||
		    ch == '^' || ch == '{' || ch == '}' || ch == '[' || ch == ']' ||
		    ch == '|' || ch == '?' || ch == '*' || ch == '@' || ch == '-' ||
		    ch == '+' || ch == ':') {
			if unlikely(unicode_printer_print(printer, flush_start,
			                                  (size_t)(ch_start - flush_start)) < 0)
				goto err;
do_escape_ch:
			if unlikely(unicode_printer_putascii(printer, '#'))
				goto err;
			flush_start = ch_start;
		}
		if (ch == '\\') {
			if unlikely(unicode_printer_print(printer, flush_start,
			                                  (size_t)(ch_start - flush_start)) < 0)
				goto err;
			flush_start = ch_start;
			ch_start    = text;
			ch = utf8_readchar((char const **)&text, end);
			if (ch == '[' || ch == ']' || ch == '|' || ch == '~' ||
			    ch == '*' || ch == '@' || ch == '-' || ch == '+' ||
			    ch == '#' || ch == ':')
				goto do_escape_ch;
			if (ch == '\\' || ch == '_' || ch == '`' ||
			    ch == '(' || ch == ')' || ch == '=')
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

/* Same as `print_escaped()', but skip the first `num_characters'
 * at the start, and after every line-feed not followed by another
 * linefeed after less than that many characters.
 * All lines are also right-stripped of trailing space characters.
 * This function is used to dedent ```-style multi-line code blocks. */
PRIVATE NONNULL((1, 2, 3)) int DCALL
print_escaped_dedent(struct unicode_printer *__restrict printer,
                     /*utf-8*/ char const *text,
                     /*utf-8*/ char const *end,
                     size_t num_characters) {
	if (!num_characters)
		return print_escaped(printer, text, end);
	else {
		size_t nskip;
		nskip = num_characters;
		for (;;) {
			uint32_t ch;
			char const *ch_start;
			ch_start = text;
			ch = utf8_readchar((char const **)&text, end);
			if (nskip) {
				if (DeeUni_IsLF(ch)) {
					/* Empty line (reset the counter) */
do_reset_on_newline:
					if unlikely(unicode_printer_put32(printer, ch))
						goto err;
					nskip = num_characters;
				} else {
					/* Must skip additional leading characters. */
					--nskip;
				}
				continue;
			}
			if (DeeUni_IsLF(ch))
				goto do_reset_on_newline;
			/* Print the current line starting with `text' and until the trailing line-feed.
			 * Do so only if there is more than only whitespace within the actual line. */
			{
				char const *current_line_start;
				char const *current_line_end;
				current_line_start = ch_start;
				for (;;) {
					ch_start = text;
					ch = utf8_readchar((char const **)&text, end);
					if (DeeUni_IsLF(ch))
						break;
					if (!ch && (text >= end)) {
						ch_start = text;
						break;
					}
				}
				/* Strip trailing whitespace. */
				current_line_end = ch_start;
				current_line_end = rstrip_whitespace(current_line_start, current_line_end);
				if unlikely(print_escaped(printer, current_line_start, current_line_end))
					goto err;
			}
			if (!ch)
				break;
			goto do_reset_on_newline;
		}
	}
	return 0;
err:
	return -1;
}

/* Find # of leading whitespace characters of the line containing the
 * least of them, and return that number while also updating `*ptext'
 * to point to the start (before potential leading whitespace) of the
 * first line that contains anything other than only whitespace. */
PRIVATE NONNULL((1, 2)) size_t DCALL
find_least_indentation_and_strip_empty_leading_lines(/*utf-8*/ char const **__restrict ptext,
                                                     /*utf-8*/ char const *end) {
	char const *text = *ptext;
	size_t min_line_leading_spaces = (size_t)-1;
	size_t current_line_leading_spaces = 0;
	for (;;) {
		uint32_t ch;
		ch = utf8_readchar((char const **)&text, end);
		if (DeeUni_IsLF(ch)) {
			if (min_line_leading_spaces == (size_t)-1)
				*ptext = text; /* Skip an empty leading line */
			current_line_leading_spaces = 0;
			continue;
		}
		if (DeeUni_IsSpace(ch)) {
			++current_line_leading_spaces;
			continue;
		}
		/* Non-space, non-line-feed character found.
		 * NOTE: Also do this when the end of the block is found! */
		if (min_line_leading_spaces > current_line_leading_spaces) {
			min_line_leading_spaces = current_line_leading_spaces;
			if (!min_line_leading_spaces)
				goto done;
		}
		/* Scan ahead until the end of the block, or until a
		 * line-feed character is found. */
		for (;;) {
			if (!ch && (text >= end))
				goto done;
			ch = utf8_readchar((char const **)&text, end);
			if (DeeUni_IsLF(ch))
				break;
		}
		/* Continue on with scanning this line. */
		current_line_leading_spaces = 0;
	}
done:
	ASSERT(min_line_leading_spaces != (size_t)-1);
	return min_line_leading_spaces;
}


PRIVATE NONNULL((1, 2, 3)) int DCALL
do_compile(/*utf-8*/ char const *text,
           /*utf-8*/ char const *end,
           struct unicode_printer *__restrict result_printer,
           /*nullable*/ struct unicode_printer *source_printer);

/* Compile the string from `source_printer' and write the result to `result_printer'
 * NOTE: This function is allowed to modify `source_printer' */
PRIVATE NONNULL((1, 2)) int DCALL
do_compile_printer_to_printer(struct unicode_printer *__restrict result_printer,
                              struct unicode_printer *__restrict source_printer) {
	if likely((source_printer->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		/* Directly compile the original documentation text. */
		return do_compile((/*utf-8*/ char const *)source_printer->up_buffer,
		                  (/*utf-8*/ char const *)source_printer->up_buffer + source_printer->up_length,
		                  result_printer, source_printer);
	} else {
		/* Re-package as a 1-byte, utf-8 string. */
		DREF DeeStringObject *rawtext;
		/*utf-8*/ char const *rawutf8;
		rawtext = (DREF DeeStringObject *)unicode_printer_pack(source_printer);
		unicode_printer_init(source_printer);
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
		if unlikely(do_compile(rawutf8, rawutf8 + WSTR_LENGTH(rawutf8),
		                       result_printer, NULL))
			goto err_rawtext;
		Dee_Decref_likely(rawtext);
	}
	return 0;
err:
	return -1;
}

#define table_iscorner(ch) ((ch) == '+')
#define table_ishori(ch)   ((ch) == '-' || (ch) == '=')
#define table_isvert(ch)   ((ch) == '|')
#define table_casecorner case '+'
#define table_casehori   case '-': case '='
#define table_casevert   case '|'



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
	end = rstrip_whitespace(text, end);
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
		case ']': case '|': case '?': case '-': case ':':
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
				} else if (next_line_leading_spaces > current_line_leading_spaces){
					/* The next line is indented more -> Join via line-feed,
					 * and keep the next line's extra indentation */

					/* Special handling for this case:
					 * >> Foobar barfoo fuz fuz fuz fuz
					 * >>
					 * >> NOTE: Very important note that
					 * >>       you should remember
					 * With the current rules, this is encoded as:
					 * "Foobar barfoo fuz fuz fuz fuz\nNOTE: Very important note that\n      you should remember"
					 * When that was intended was actually:
					 * "Foobar barfoo fuz fuz fuz fuz\nNOTE: Very important note that you should remember"
					 * Check for this case by looking at the character posistion at:
					 *   current_line_start_after_whitespace +[utf8] (next_line_leading_spaces -
					 *                                                current_line_leading_spaces)
					 * If that character position descbides a non-whitespace character,
					 * and is preceded by the following sequence:
					 *     (`.', <space> or <issymcont>) : <optional_space>
					 * Then we mustn't insert a line-feed here, but join the two lines with a space. */
					char const *prev_line_start;
					char const *prev_line_end, *temp;
					uint32_t temp_ch;
					size_t count = next_line_leading_spaces - current_line_leading_spaces;
					prev_line_start = current_line_start_after_whitespace;
					prev_line_end   = current_line_start_after_whitespace;
					do {
						temp_ch = utf8_readchar((char const **)&prev_line_end, next_line_start);
						if (!temp_ch && (prev_line_end <= prev_line_start))
							goto do_join_with_linefeed;
						if (DeeUni_IsLF(temp_ch))
							goto do_join_with_linefeed;
					} while (--count);
					/* Check if `prev_line_end' points to a non-whitespace character. */
					temp = prev_line_end;
					temp_ch = utf8_readchar((char const **)&prev_line_end, next_line_start);
					if (DeeUni_IsSpace(temp_ch))
						goto do_join_with_linefeed;
					prev_line_end = temp;
					/* Skip whitespace found prior to `prev_line_end' */
					prev_line_end = rstrip_whitespace(prev_line_start, prev_line_end);
					/* Check if there's a :-character before `prev_line_end' */
					temp_ch = utf8_readchar_rev((char const **)&prev_line_end, prev_line_start);
					if (temp_ch != ':')
						goto do_join_with_linefeed;
					/* Check if the :-character is preceded by `.', <space> or <issymcont>
					 * If it is, then we must join the two lines via space */
					temp_ch = utf8_readchar_rev((char const **)&prev_line_end, prev_line_start);
					if (DeeUni_IsSymCont(temp_ch) || DeeUni_IsSpace(temp_ch) || temp_ch == '.')
						goto do_join_with_space;
do_join_with_linefeed:
					PUTASCII('\n'); /* Force a line-feed here. */
				} else {
					/* The next line is the same or more -> Join via space */
do_join_with_space:
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

			table_casevert:
			table_casecorner: {
				/* Table */
				uint32_t corner_ch;
				uint32_t horizontal_ch;
				uint32_t vertical_ch;
				size_t column_count;
				char const *table_top_left; /* Pointer to the top-left corner of the table */
				bool corner_ch_is_known;
				/* Tables must appear at the start of lines. */
				ASSERT(ch_start == current_line_start_after_whitespace);
				corner_ch      = ch;
				vertical_ch    = 0;
				horizontal_ch  = 0;
				table_top_left = ch_start;
				/* Seek ahead to the start of the first cell. */
				for (;;) {
					ch_start = iter;
					ch       = utf8_readchar((char const **)&iter, end);
					if (!ch && (iter >= end))
						goto not_a_table;
					if (DeeUni_IsLF(ch))
						goto not_a_table;
					if (DeeUni_IsSpace(ch))
						continue;
					if (table_ishori(ch)) {
						/* With a top-border present, the corner
						 * character used is known to be valid. */
						corner_ch_is_known = true;
						horizontal_ch = (char)(unsigned char)ch;
						/* A top-border is present.
						 * Ensure that the current line contains only `corner_ch',
						 * `-', `=' and space characters, as well as count the #
						 * of remaining `corner_ch'-characters as `column_count' */
						column_count = 0;
						for (;;) {
							ch = utf8_readchar((char const **)&iter, end);
							if (DeeUni_IsLF(ch)) {
								/* Deal with windows-style line-feeds */
								if (ch == '\r') {
									char const *temp = iter;
									ch = utf8_readchar((char const **)&iter, end);
									if (ch != '\n')
										iter = temp;
								}
								break; /* End of the top-border. */
							}
							if (DeeUni_IsSpace(ch) || ch == horizontal_ch)
								continue;
							if (ch != corner_ch)
								goto not_a_table;
							++column_count;
						}
						if (!column_count)
							goto not_a_table; /* Need at least 1 column! */
						/* A top border was present, and we did parse it.
						 * Now we must move on to the start of the first cell. */
						for (;;) {
							ch = utf8_readchar((char const **)&iter, end);
							if (DeeUni_IsLF(ch) || (!ch && iter >= end))
								goto not_a_table;
							if (DeeUni_IsSpace(ch))
								continue;
							if (table_isvert(ch))
								break;
							goto not_a_table;
						}
						vertical_ch = ch;
						/* Skip additional whitespace at the start of the first cell. */
						for (;;) {
							ch_start = iter;
							ch = utf8_readchar((char const **)&iter, end);
							if (DeeUni_IsLF(ch) || (!ch && iter >= end))
								goto not_a_table;
							if (!DeeUni_IsSpace(ch))
								break;
						}
						break;
					} else {
						uint32_t scan_ch;
						char const *scan_iter;
						/* No top-border present. - This must (presumably) be start of the top-left! */
						if (table_iscorner(corner_ch))
							goto not_a_table; /* +-corners require the top-border be present */
						/* Without a top-border, and | as (premature) corner character, the actual
						 * corner character of the table may still change to `+' once a thick line
						 * is encountered. */
						corner_ch_is_known = false;
						column_count = 0;
						/* Figure out the correct # of cells by counting the number
						 * of non-escaped |-characters on the current line. */
						scan_ch   = ch;
						scan_iter = iter;
						for (;;) {
							if (DeeUni_IsLF(scan_ch) || (!scan_ch && scan_iter >= end))
								break;
							if (vertical_ch) {
								if (scan_ch == vertical_ch)
									++column_count;
							} else if (table_isvert(scan_ch)) {
								vertical_ch = scan_ch;
								++column_count;
							}
							if (scan_ch == '\\') {
								scan_ch = utf8_readchar((char const **)&scan_iter, end);
								if (DeeUni_IsLF(scan_ch) || (!scan_ch && scan_iter >= end))
									break;
							}
							/* TODO: Try to recognize special control blocks and skip over them:
							 *       A |-character inside of @(foo | bar) should automatically
							 *       be escaped, and similar should happen for `foo | bar' (i.e.
							 *       inlined code) */
							scan_ch = utf8_readchar((char const **)&scan_iter, end);
						}
						if (!column_count)
							goto not_a_table;
						/*if (ch == vertical_ch)
							break;*/ /* No top-border is present, and the top-left cell is empty. */
						break;
					}
				}
				/* NOTE: ch/ch_start point at the first non-whitespace character of the top-left cell;
				 *       iter points after that same character! */
				ASSERT(vertical_ch);
				iter = ch_start;
				for (;;) {
					uint32_t prev_ch;
					char const *prev_iter = iter;
					/* Rewind whitespace characters. */
					prev_ch = utf8_readchar_rev((char const **)&iter, table_top_left);
					if (DeeUni_IsSpace(prev_ch))
						continue;
					iter     = prev_iter;
					ch_start = prev_iter;
					ch       = utf8_readchar((char const **)&iter, table_top_left);
					break;
				}
				/* NOTE: ch/ch_start point at the first character of the top-left cell;
				 *       iter points after that same character! */
				{
					struct table_column {
						struct unicode_printer tc_body; /* The inner body of the cell. */
					};
					/* The buffer for the compiled contents of the table. */
					struct unicode_printer table_output = UNICODE_PRINTER_INIT;
					struct table_column *columns; /* [1..column_count][owned] Vector of column buffers. */
					size_t column_index;
					bool need_thin_seperator = false;
					char const *nextline_start = NULL;

					/* Allocate & initialize buffers for all of the columns. */
					columns = (struct table_column *)Dee_Malloc(column_count *
					                                            sizeof(struct table_column));
					if unlikely(!columns)
						goto err_table_output;
					/* Initialize printers for the cells. */
					for (column_index = 0; column_index < column_count; ++column_index)
						unicode_printer_init(&columns[column_index].tc_body);
					for (;;) {
						bool has_nonempty_cell;
scan_table_line:
						/* NOTE: ch/ch_start point at the first character of the left-most
						 *       cell of the current row; iter points after that same character! */
						column_index = 0;
						has_nonempty_cell = false;
						for (;;) {
							bool has_escaped_vert;
							char const *cell_start;
							char const *cell_end;
							cell_start = ch_start;
							has_escaped_vert = false;
							/* Parse cell lines */
							while (ch != vertical_ch) {
								if (DeeUni_IsLF(ch) || (!ch && (iter >= end))) {
do_handle_end_of_line_in_table_line:
									cell_start = lstrip_whitespace(cell_start, iter);
									if (cell_start < iter)
										goto not_a_table2_or_done_table; /* Garbage after the table? */
									/* End-of-line */
									if (column_index == column_count) {
										/* Deal with windows-style line-feeds */
										if (ch == '\r') {
											char const *temp = iter;
											ch = utf8_readchar((char const **)&iter, end);
											if (ch != '\n') {
												iter = temp;
												/*ch = '\r';*/
											}
										}
										goto got_table_line;
									}
									/* Wrong # of cells */
not_a_table2_or_done_table:
									if (UNICODE_PRINTER_ISEMPTY(&table_output))
										goto not_a_table2;
									if (!nextline_start)
										goto not_a_table2;
									/* Finalize partially written cells. */
									for (column_index = 0; column_index < column_count; ++column_index)
										unicode_printer_fini(&columns[column_index].tc_body);
									iter = nextline_start;
									goto done_table_nochk_columns;
								}
								/* Ignore |-characters that are pre-fixed by \-characters */
								if (ch == '\\') {
									ch_start = iter;
									ch = utf8_readchar((char const **)&iter, end);
									if (DeeUni_IsLF(ch) || (!ch && (iter >= end)))
										goto do_handle_end_of_line_in_table_line;
									if (ch == vertical_ch)
										has_escaped_vert = true;
								}
								/* TODO: Try to recognize special control blocks and skip over them:
								 *       A |-character inside of @(foo | bar) should automatically
								 *       be escaped, and similar should happen for `foo | bar' (i.e.
								 *       inlined code) */
								ch_start = iter;
								ch = utf8_readchar((char const **)&iter, end);
							}
							/* Figure out where the contents of the current cell end. */
							cell_end = ch_start;
							cell_end = rstrip_whitespace(cell_start, cell_end);
							if (cell_start >= cell_end) {
								/* Empty cell. */
								++column_index;
								ch_start = iter;
								ch = utf8_readchar((char const **)&iter, end);
								continue;
							}
							/* Make sure there aren't more columns all-of-the-sudden */
							if unlikely(column_index >= column_count)
								goto not_a_table2_or_done_table;
							if (!has_nonempty_cell) {
								/* Append empty lines to all cells already written as empty. */
								size_t i;
								for (i = 0; i < column_index; ++i) {
									if unlikely(unicode_printer_putascii(&columns[i].tc_body, '\n'))
										goto err_table;
								}
								has_nonempty_cell = true;
							}
							/* Append the current line to our cell */
							if (has_escaped_vert) {
								/* Unescape \|-sequences to single |-characters */
								if unlikely(reprint_but_unescape_backslash_pipe(&columns[column_index].tc_body,
								                                                cell_start, cell_end))
									goto err_table;
							} else {
								if unlikely(unicode_printer_print(&columns[column_index].tc_body,
								                                  cell_start, (size_t)(cell_end - cell_start)) < 0)
									goto err_table;
							}
							/* Append trailing new-line characters to this column */
							if unlikely(unicode_printer_putascii(&columns[column_index].tc_body, '\n'))
								goto err_table;
							/* Move on to the next cell */
							++column_index;
							ch_start = iter;
							ch = utf8_readchar((char const **)&iter, end);
						} /* for (;;) */
						{
							/* Table row separator:
							 *   '~'  Thick separator
							 *   '&'  Thin separator
							 *   0    Table end */
							char row_seperator;
got_table_line:
							/* At this point, ch/ch_start point at the line-feed character that followed
							 * the end of the table line; iter points after that same character */
							nextline_start = iter;
							/* Skip leading spaces at the start of the next line. */
							for (;;) {
								ch_start = iter;
								ch = utf8_readchar((char const **)&iter, end);
								if (DeeUni_IsSpace(ch) && !DeeUni_IsLF(ch))
									continue;
								break;
							}
							/* When an empty line appears in all columns, switch to a new line */
							if (!has_nonempty_cell)
								need_thin_seperator = true;
							/* ch/ch_start now point at the first non-whitespace character of the next line. */
							if (ch == vertical_ch) {
								char const *nextline_firstcell_start;
								bool did_encounter_horizontal_character;
								ch_start = iter;
								ch = utf8_readchar((char const **)&iter, end);
								if (corner_ch_is_known && corner_ch != vertical_ch) {
extend_table_row_or_insert_thin_seperator:
									if (need_thin_seperator) {
										row_seperator = '&';
										goto end_table_row;
									}
									goto scan_table_line; /* This definitely is still part of the same row! */
								}
								/* iter now pointer at the first character after the |-character at the start
								 * of a row that can either be a thick separator, or a continuation of the
								 * current row.
								 * We can differentiate the two by checking if the first cell contains only
								 * whitespace, and at least 1 instance of `horizontal_ch'. If this is the case,
								 * then it's a thick separator. - Otherwise, it is a normal row that might possibly
								 * be entirely empty (if the later is the case, then the next time `got_table_line'
								 * is jumped to, `has_nonempty_cell' will be `false', and the line will be checked
								 * accordingly) */
								nextline_firstcell_start = ch_start;
								did_encounter_horizontal_character = false;
								for (;;) {
									if (DeeUni_IsLF(ch) || (!ch && iter >= end))
										goto set_end_of_table;
									if (DeeUni_IsSpace(ch))
										/* continue */;
									else if (ch == vertical_ch)
										break; /* end-of-cell */
									else if (ch == horizontal_ch)
										did_encounter_horizontal_character = true;
									else if (table_ishori(ch) && !horizontal_ch) {
										horizontal_ch                      = ch;
										did_encounter_horizontal_character = true;
									} else {
										/* Non-empty cell. */
continue_row_at_nextline_firstcell_start:
										ch_start = nextline_firstcell_start;
										iter     = nextline_firstcell_start;
										ch       = utf8_readchar((char const **)&iter, end);
										goto extend_table_row_or_insert_thin_seperator;
									}
									ch = utf8_readchar((char const **)&iter, end);
								}
								/* If we didn't encounter any horizontal characters, then the
								 * cell is entirely empty, which we must handle as a row that
								 * may potentially contain more text in later cells. */
								if (!did_encounter_horizontal_character)
									goto continue_row_at_nextline_firstcell_start;
								/* This row contains a thick separator. */
								row_seperator = '~';
								/* Scan until the end of this row. */
								column_index = 1;
								for (;;) {
									ch = utf8_readchar((char const **)&iter, end);
									if (DeeUni_IsLF(ch) || (!ch && iter >= end))
										break;
									if (DeeUni_IsSpace(ch))
										continue;
									if (ch == vertical_ch) {
										if (column_index >= column_count)
											goto set_end_of_table;
										++column_index;
										continue;
									}
									if (ch == horizontal_ch)
										continue;
									if (table_ishori(ch) && !horizontal_ch) {
										horizontal_ch = ch;
										continue;
									}
									/* Some other character (unexpected / not allowed) */
									goto set_end_of_table;
								}
								if (column_index != column_count)
									goto set_end_of_table;
								/* Deal with windows-line-feeds */
								if (ch == '\r') {
									char const *temp = iter;
									ch = utf8_readchar((char const **)&iter, end);
									if (ch != '\n')
										iter = temp;
								}
								ch_start = iter;
								ch       = utf8_readchar((char const **)&iter, end);
								nextline_start = iter;
								/* NOTE: ch/ch_start point at the first character of the line that followed
								 *       after the thick row. Next, skip some optional whitespace and update
								 *       ch/ch_start to point at the first character of the left-most cell.
								 *       If the next line doesn't continue the table, set `iter' to `nextline_start'
								 *       and `row_seperator' to 0 */
								for (;;) {
									if (DeeUni_IsLF(ch)) {
thick_border_is_actually_table_end:
										/* Table end */
										iter = nextline_start;
										row_seperator = 0;
										goto end_table_row;
									}
									if (DeeUni_IsSpace(ch)) {
										ch_start = iter;
										ch = utf8_readchar((char const **)&iter, end);
										continue;
									}
									if (ch != vertical_ch)
										goto thick_border_is_actually_table_end;
									/* First character after the initial <vertical_ch> */
									ch_start = iter;
									ch = utf8_readchar((char const **)&iter, end);
									break;
								}
							} else if (ch == corner_ch) {
								/* Thick line! */
do_handle_corner_ch_as_thick_row:
								row_seperator = '~';
								/* Scan until the end of this line. */
								/* NOTE: Must be able to handle this case:
								 * >> +-----+-----+
								 * >> | Foo | Bar |
								 * >> | Foo | Bar |
								 * >> + Foo
								 * >> + Bar
								 * This is a table without a footer, that is immediately
								 * followed by a +-style list. This is a valid construct! */
								/* ch/ch_start now point at the first non-whitespace character of the next line. */
								column_index = 0;
								for (;;) {
									ch = utf8_readchar((char const **)&iter, end);
									if (DeeUni_IsLF(ch) || (!ch && iter >= end))
										break;
									if (DeeUni_IsSpace(ch))
										continue;
									if (ch == horizontal_ch)
										continue;
									if (!horizontal_ch && (ch == '=' || ch == '-')) {
										horizontal_ch = ch;
										continue;
									}
									if (ch == corner_ch) {
										if (column_index >= column_count)
											goto set_end_of_table;
										++column_index;
										continue;
									}
									/* Anything else? -> Mark the end of the table. */
									goto set_end_of_table;
								}
								if (column_index != column_count)
									goto set_end_of_table;
								/* All right! the thick row was parsed. - Now check if the table
								 * continues, or if the thick row was actually the table's footer */
								nextline_start = iter;
								if (ch == '\r') {
									ch = utf8_readchar((char const **)&iter, end);
									if (ch != '\n')
										iter = nextline_start;
								}
								/* Skip leading whitespace. */
								for (;;) {
									ch = utf8_readchar((char const **)&iter, end);
									if (DeeUni_IsLF(ch) || (!ch && iter >= end))
										goto set_end_of_table;
									if (DeeUni_IsSpace(ch))
										continue;
									if (ch == vertical_ch)
										break; /* Next row start here! */
									/* Something else? -> End the table */
									goto set_end_of_table;
								}
								/* Read the first character for the first cell after the thick separator. */
								ch_start = iter;
								ch = utf8_readchar((char const **)&iter, end);
							} else if (table_iscorner(ch) && !corner_ch_is_known) {
								corner_ch_is_known = true;
								corner_ch          = ch;
								goto do_handle_corner_ch_as_thick_row;
							} else {
set_end_of_table:
								row_seperator = 0; /* End-of-table */
								iter = nextline_start;
							}
end_table_row:
							/* NOTE: ch/ch_start point at the first character of the left-most
							 *       cell of the current row; iter points after that same character! */
							/* Compile the current row */
							for (column_index = 0; column_index < column_count; ++column_index) {
								if (column_index != 0) {
									/* Write the cell separator. */
									if unlikely(unicode_printer_putascii(&table_output, '|'))
										goto err_table;
								}
								/* compile and write output to `table_output' */
								if unlikely(do_compile_printer_to_printer(&table_output,
								                                          &columns[column_index].tc_body))
									goto err_table;
								unicode_printer_fini(&columns[column_index].tc_body);
								unicode_printer_init(&columns[column_index].tc_body);
							}
							if (row_seperator == 0)
								goto done_table;
							/* Terminate the row with either a thin, or a thick separator. */
							if unlikely(unicode_printer_putascii(&table_output, row_seperator))
								goto err_table;
							need_thin_seperator = false;
						}
						/* NOTE: ch/ch_start point at the first character of the left-most
						 *       cell of the current row; iter points after that same character! */
						goto scan_table_line;
					} /* for (;;) */
done_table:
					/* NOTE: At this point, `iter' points at the start of
					 *       line immediately following the table. */
#ifndef NDEBUG
					/* Make sure that all column buffers are empty when the table ends. */
					for (column_index = 0; column_index < column_count; ++column_index)
						ASSERT(UNICODE_PRINTER_ISEMPTY(&columns[column_index].tc_body));
#endif /* !NDEBUG */
done_table_nochk_columns:
					Dee_Free(columns);
					/* Flush to the start of the table. */
					if unlikely(unicode_printer_print(result_printer, flush_start,
					                                  (size_t)(table_top_left - flush_start)) < 0)
						goto err_table_output;
					/* Strip trailing whitespace from the result printer. */
					strip_trailing_whitespace_until(result_printer,
					                                result_printer_origlen);
					/* Insert a linefeed before the table (if not there already) */
					if (result_printer->up_length > result_printer_origlen &&
					    !DeeUni_IsLF(UNICODE_PRINTER_GETCHAR(result_printer, result_printer->up_length - 1))) {
						if unlikely(unicode_printer_putascii(result_printer, '\n'))
							goto err_table_output;
					}
					if (current_line_start_after_whitespace > current_line_start) {
						/* Insert leading whitespace found prior to the table.
						 * This is the indentation of the table itself.
						 * This may be de-dented once again in the final output. */
						if unlikely(unicode_printer_print(result_printer, current_line_start,
						                                  (size_t)(current_line_start_after_whitespace - current_line_start)) < 0)
							goto err_table_output;
					}
					/* Write the table header */
					if unlikely(unicode_printer_printascii(result_printer, "#T{", 3) < 0)
						goto err_table_output;
					/* Print the compiled table contents into the result printer. */
					if unlikely(unicode_printer_printinto(&table_output,
					                                      &unicode_printer_print,
					                                      result_printer) < 0)
						goto err_table_output;
					/* Write the table footer */
					if unlikely(unicode_printer_printascii(result_printer, "}\n", 2) < 0)
						goto err_table_output;
					/* Destroy the table output buffer. */
					unicode_printer_fini(&table_output);
					/* Read the first character that is no longer apart of the table. */
					flush_start = iter;
					ch_start    = iter;
					ch = utf8_readchar((char const **)&iter, end);
					goto scan_newline_with_first_ch_and_explicit_linefeed;
err_table:
					for (column_index = 0; column_index < column_count; ++column_index)
						unicode_printer_fini(&columns[column_index].tc_body);
					Dee_Free(columns);
err_table_output:
					unicode_printer_fini(&table_output);
					goto err;
not_a_table2:
					unicode_printer_fini(&table_output);
					for (column_index = 0; column_index < column_count; ++column_index)
						unicode_printer_fini(&columns[column_index].tc_body);
					Dee_Free(columns);
				}
not_a_table:
				ch_start = table_top_left;
				iter     = table_top_left;
				ch = utf8_readchar((char const **)&iter, end);
#if table_iscorner('+') || table_isvert('+')
				if (ch == '+')
					goto check_for_list;
#endif /* table_iscorner('+') || table_isvert('+') */
			}	break;

#if !table_iscorner('+') && table_isvert('+')
			case '+':
#endif /* !table_iscorner('+') && !table_isvert('+') */
			case '-':
			case '*':
check_for_list:
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
						strip_trailing_whitespace_until(result_printer,
						                                result_printer_origlen);
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

			/* ESCAPED INPUT:  \ _ @ ` [ ] ( ) - + | = ~ * # : */
			/* ESCAPED OUTPUT: # $ % & ~ ^ { } [ ] | ? * @ - + : */


				/* Characters that need escaping in output only. */
			case '$': case '%': case '&': case '^':
			case '{': case '}': case '?':
				FLUSHTO(escaped_ch_start);
				PUTASCII('#');
				flush_start = escaped_ch_start;
				break;

				/* Characters that need escaping in input & output. */
			case '@': /* tags */
			case '[': /* Hyper-links */
			case ']': /* Hyper-links */
			case ':': /* Lists */
			case '+': /* Lists */
			case '-': /* Lists */
			case '|': /* Tables */
			case '#': /* Headers */
				FLUSHTO(ch_start);
				PUTASCII('#');
				flush_start = escaped_ch_start;
				break;

				/* INPUT-ONLY-ESCAPE and INPUT-OUTPUT-ESCAPE. (with extended range) */
			case '_':  /* _Bold_ */
			case '`':  /* `Code` */
			case '*':  /* Lists and *Italics* */
			case '~':  /* ~Strikethrough~ */
				FLUSHTO(ch_start);
				if (ch == '~' || ch == '*') /* INPUT-OUTPUT-ESCAPE */
					PUTASCII('#');
				flush_start = escaped_ch_start;
				/* Check if this is escaping an extended block. */
				ch_start = iter;
				ch = utf8_readchar((char const **)&iter, end);
				if (ch == escaped_ch) {
					if (ch == '`') {
						/* Code-blocks can appear with 3 backticks, in
						 * which case we must escape all 3 of them here! */
						char const *temp;
						temp     = ch_start;
						ch_start = iter;
						ch = utf8_readchar((char const **)&iter, end);
						if (ch != '`') {
							ch_start = temp;
							iter     = temp;
							ch = utf8_readchar((char const **)&iter, end);
						}
					}
					goto do_switch_ch; /* Yes -> Also escape the second character */
				}
				goto do_read_and_switch_ch;


				/* INPUT-ONLY-ESCAPE. */
			case ' ':  /* Escape space characters to force them to be kept */
			case '\\': /* Escape the escape character itself to force it to be inserted */
			case '(':  /* Hyper-links */
			case ')':  /* Hyper-links */
			case '=':  /* Lists */
escape_inputonly:
				FLUSHTO(ch_start);
				flush_start = escaped_ch_start;
				break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '.':
check_ordered_list_digit:
				/* Ordered list (only at the start of a line) */
				if (ch_start == current_line_start_after_whitespace)
					goto escape_inputonly;
				/* Also allow the backslash to appear somewhere where
				 * it is only preceded by digits or . or : characters */
				if (contains_only_decimals_dot_collon_or_backslash(current_line_start_after_whitespace,
				                                                   ch_start))
					goto escape_inputonly;
				break;

			default:
				if (DeeUni_IsLF(escaped_ch))
					goto escape_inputonly;
				if (DeeUni_IsSpace(escaped_ch))
					goto escape_inputonly;
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
				bool need_braces;
				/* Special case: The component is followed by more symbol-continue characters.
				 * In this case, we must still enclose our portion with braces, so-as to prevent
				 * the follow-up text also being considered apart of our construct! */
				need_braces = DeeUni_IsSymCont(ch) || !is_symbol(body_start, body_end);
				if (need_braces)
					PUTASCII('{');
				/* Recursively compile the contained body. */
				if unlikely(do_compile(body_start, body_end, result_printer, NULL))
					goto err;
				if (need_braces)
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
			PRINTASCII("#A{", 3);
			/* Print the link body */
			if unlikely(do_compile(after_lbracket, before_rbracket, result_printer, NULL))
				goto err;
			PUTASCII('|');
			/* Print the link text. */
			if unlikely(print_escaped(result_printer, after_lparen, before_rparen))
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

		case '`': {
			/* Code */
			char const *before_left_backtick, *after_left_backtick;
			char const *before_right_backtick;
			before_left_backtick = ch_start;
			after_left_backtick  = iter;
			ch = utf8_readchar((char const **)&iter, end);
			if (ch == '`') {
				char const *temp = iter;
				ch = utf8_readchar((char const **)&iter, end);
				if (ch == '`') {
					char const *end_of_first_line;
					after_left_backtick = iter;
					/* Extended code block. (with optional language-specific highlighting)
					 * This type of block can span multiple lines. */
					end_of_first_line = NULL;
					for (;;) {
						before_right_backtick = iter;
						ch = utf8_readchar((char const **)&iter, end);
						if (!ch && (iter >= end))
							goto not_a_code;
						if (DeeUni_IsLF(ch) && !end_of_first_line)
							end_of_first_line = before_right_backtick;
						if (ch != '`')
							continue;
						ch = utf8_readchar((char const **)&iter, end);
						if (ch != '`')
							continue;
						ch = utf8_readchar((char const **)&iter, end);
						if (ch == '`')
							break;
					}
					/* Check for a valid syntax name, as well as how many
					 * leading space characters should be removed from each line.
					 * The syntax here is that anything that appears on the first
					 * line (immediately after the initial ```) is considered to
					 * be the name of the language for which highlighting is to be
					 * provided. */
					if (end_of_first_line) {
						char const *before_syntax_name;
						char const *after_syntax_name;
						char const *start_of_second_line;
						size_t smallest_indentation;
						before_syntax_name   = after_left_backtick;
						after_syntax_name    = end_of_first_line;
						/* Strip leading/trailing whitespace from the syntax name. */
						before_syntax_name = lstrip_whitespace(before_syntax_name, after_syntax_name);
						after_syntax_name  = rstrip_whitespace(before_syntax_name, after_syntax_name);
						/* Skip the line-feed between the first and second line in
						 * order to determine the true start of the second line. */
						start_of_second_line = end_of_first_line;
						ch = utf8_readchar((char const **)&start_of_second_line, before_right_backtick);
						if (ch == '\r') {
							char const *temp = start_of_second_line;
							ch = utf8_readchar((char const **)&start_of_second_line, before_right_backtick);
							if (ch != '\n')
								start_of_second_line = temp;
						}
						/* Find the smallest indentation between:
						 *    start_of_second_line ... before_right_backtick
						 * During this search, include the indentation of the last line as well,
						 * in that the indentation of the trailing ``` will also be considered! */
						smallest_indentation = find_least_indentation_and_strip_empty_leading_lines(&start_of_second_line,
						                                                                            before_right_backtick);
						if (before_left_backtick == current_line_start_after_whitespace) {
							/* If the first line's ```-sequence appears at the start of that
							 * line, then also consider the indentation on that line for how
							 * to figure out the actual # of leading spaces to strip from every
							 * line. */
							if (smallest_indentation > current_line_leading_spaces)
								smallest_indentation = current_line_leading_spaces;
						}
						/* Strip trailing whitespace from the code block. */
						before_right_backtick = rstrip_whitespace(start_of_second_line, before_right_backtick);
						FLUSHTO(before_left_backtick);
						/* If we're not already at the start of our own line, then insert a
						 * manual line-feed such that the code-block appears on its own line. */
						if (before_left_backtick != current_line_start_after_whitespace) {
							strip_trailing_whitespace_until(result_printer,
							                                result_printer_origlen);
							PUTASCII('\n');
						}
						flush_start = iter;
						ch_start    = iter;
						ch          = utf8_readchar((char const **)&iter, end);
						{
							bool need_braces;
							need_braces = DeeUni_IsSymCont(ch) ||
							              !is_symbol(start_of_second_line,
							                         before_right_backtick);
							/* Select encoding based on syntax name. */
							if (before_syntax_name >= after_syntax_name) {
								PRINTASCII("#C", 2); /* No special syntax name */
							} else if (before_syntax_name + 6 == after_syntax_name &&
							           memcmp(before_syntax_name, "deemon", 6 * sizeof(char)) == 0) {
								PUTASCII('$'); /* Deemon syntax */
							} else {
								PRINTASCII("#C[", 3); /* Custom syntax */
								if unlikely(print_escaped(result_printer,
								                          before_syntax_name,
								                          after_syntax_name))
									goto err;
								PUTASCII(']');
							}
							if (need_braces)
								PUTASCII('{');
							if unlikely(print_escaped_dedent(result_printer,
							                                 start_of_second_line,
							                                 before_right_backtick,
							                                 smallest_indentation))
								goto err;
							if (need_braces)
								PUTASCII('}');
						}
						/* Use the common indentation of the code-block to affect how
						 * further text is arranged in concern to block breaks. */
						current_line_leading_spaces = smallest_indentation;
						/* If the character following the code-block is a line-feed, force
						 * that line-feed to appear in the output documentation text. */
						if (DeeUni_IsLF(ch)) {
do_handle_autoescaped_linefeed:
							if (ch == '\r') {
								char const *temp = iter;
								ch = utf8_readchar((char const **)&iter, end);
								if (ch != '\n')
									iter = temp;
							}
							strip_trailing_whitespace_until(result_printer,
							                                result_printer_origlen);
							PUTASCII('\n');
							/* Read the first character from the next line. */
							flush_start = iter;
							ch_start    = iter;
							ch = utf8_readchar((char const **)&iter, end);
							goto scan_newline_with_first_ch_and_explicit_linefeed;
						} else if (DeeUni_IsSpace(ch)) {
							/* Allow for trailing space before an eventual line-feed */
							char const *temp = ch_start;
							for (;;) {
								ch = utf8_readchar((char const **)&iter, end);
								if (DeeUni_IsLF(ch))
									goto do_handle_autoescaped_linefeed;
								if (DeeUni_IsSpace(ch))
									continue;
								break;
							}
							ch_start = temp;
							iter     = temp;
							ch = utf8_readchar((char const **)&iter, end);
						}
						goto do_switch_ch;
					}
				} else {
					/* 2 backticks behave the same as a single backtick, except that this
					 * type of block must also be terminated by 2 backticks, rather than one. */
					after_left_backtick = temp;
					for (;;) {
						if (!ch && (iter >= end))
							goto not_a_code;
						if (DeeUni_IsLF(ch))
							goto not_a_code;
						before_right_backtick = iter;
						ch = utf8_readchar((char const **)&iter, end);
						if (ch != '`')
							continue;
						ch = utf8_readchar((char const **)&iter, end);
						if (ch == '`')
							break;
					}
				}
			} else {
				/* Single-backtick code block. */
				for (;;) {
					if (!ch && (iter >= end))
						goto not_a_code;
					if (DeeUni_IsLF(ch))
						goto not_a_code;
					before_right_backtick = iter;
					ch = utf8_readchar((char const **)&iter, end);
					if (ch == '`' || ch == '\'')
						break;
				}
			}
			after_left_backtick   = lstrip_whitespace(after_left_backtick, before_right_backtick);
			before_right_backtick = rstrip_whitespace(after_left_backtick, before_right_backtick);
			FLUSHTO(before_left_backtick);
			/* Update flush pointers and read the next character (needs to be done to see
			 * if we have to force braces because the next character would continue a symbol) */
			flush_start = iter;
			ch_start    = iter;
			ch          = utf8_readchar((char const **)&iter, end);
			/* Print the code contents as an escaped string (but don't re-parse the contents). */
			{
				bool need_braces;
				need_braces = DeeUni_IsSymCont(ch) ||
				              !is_symbol(after_left_backtick,
				                         before_right_backtick);
				PRINTASCII("#C", 2);
				if (need_braces)
					PUTASCII('{');
				if unlikely(print_escaped(result_printer, after_left_backtick, before_right_backtick))
					goto err;
				if (need_braces)
					PUTASCII('}');
			}
			goto do_switch_ch;
not_a_code:
			iter = after_left_backtick;
			break;
		}

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
						ch = second_space;
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
		result_printer->up_length = (size_t)(end - text);
		unicode_printer_init(source_printer);
	} else {
		if unlikely(unicode_printer_print(result_printer, flush_start,
		                                  (size_t)(end - flush_start)) < 0)
			goto err;
	}
done_dontflush:
	/* Strip all trailing whitespace from the printer. */
	strip_trailing_whitespace(result_printer, result_printer_origlen);
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
			} else if (ch == '{') {
				/* Skip non-escaped {}-pairs here! - Those belong to inner constructs,
				 * and may contain line-feed characters which we must ignore! */
				size_t recursion = 0;
				for (;;) {
					if (i >= UNICODE_PRINTER_LENGTH(result_printer))
						goto done_return_now;
					ch = UNICODE_PRINTER_GETCHAR(result_printer, i);
					++i;
					if (ch == '#')
						++i;
					else if (ch == '{')
						++recursion;
					else if (ch == '}') {
						if (!recursion)
							break;
						--recursion;
					}
				}
			} else if (ch == '#' && i < UNICODE_PRINTER_LENGTH(result_printer)) {
				ch = UNICODE_PRINTER_GETCHAR(result_printer, i);
				if (ch == '{')
					++i; /* Escaped brace (ignore) */
			}
			unicode_printer_erase(result_printer, i, min_line_leading_spaces);
		}
	}
done_return_now:
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
