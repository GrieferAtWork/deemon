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
#ifdef __INTELLISENSE__
#include "format.c"
//#define DEFINE_DeeString_FormatPrinter
#define DEFINE_DeeString_FormatWStr
//#define DEFINE_DeeString_Format
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include <hybrid/limitcore.h> /* __SSIZE_MAX__, __SSIZE_MIN__ */

#include <stddef.h> /* NULL, size_t */

#undef SSIZE_MIN
#undef SSIZE_MAX
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

#if (defined(DEFINE_DeeString_FormatPrinter) + \
     defined(DEFINE_DeeString_FormatWStr) +    \
     defined(DEFINE_DeeString_Format)) > 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeString_Format... */

#ifndef STRING_FORMAT_WSTR_FOREACH_DONE
#define STRING_FORMAT_WSTR_FOREACH_DONE SSIZE_MIN
#endif /* !STRING_FORMAT_WSTR_FOREACH_DONE */

DECL_BEGIN

#ifdef DEFINE_DeeString_FormatPrinter
#define LOCAL_string_format_data                         generic_string_format_data
#define LOCAL_string_format_advanced                     generic_string_format_advanced
#define LOCAL_parse_format_template_for_object_format_ex generic_parse_format_template_for_object_format_ex
#define LOCAL_parse_format_template_for_object_format    generic_parse_format_template_for_object_format
#define LOCAL_string_format_wstr_foreach_cb              generic_string_format_wstr_foreach_cb
#else /* DEFINE_DeeString_FormatPrinter */
#define LOCAL_string_format_data                         unicode_string_format_data
#define LOCAL_string_format_advanced                     unicode_string_format_advanced
#define LOCAL_parse_format_template_for_object_format_ex unicode_parse_format_template_for_object_format_ex
#define LOCAL_parse_format_template_for_object_format    unicode_parse_format_template_for_object_format
#define LOCAL_string_format_wstr_foreach_cb              unicode_string_format_wstr_foreach_cb
#endif /* !DEFINE_DeeString_FormatPrinter */

#if (defined(DEFINE_DeeString_FormatPrinter) ? !defined(GENERIC_STRING_FORMAT_DATA_DEFINED) \
                                             : !defined(UNICODE_STRING_FORMAT_DATA_DEFINED))
#ifdef DEFINE_DeeString_FormatPrinter
#define GENERIC_STRING_FORMAT_DATA_DEFINED
#else /* DEFINE_DeeString_FormatPrinter */
#define UNICODE_STRING_FORMAT_DATA_DEFINED
#endif /* !DEFINE_DeeString_FormatPrinter */
struct LOCAL_string_format_data {
	struct string_format_parser sfd_parser;   /* Underlying parser. */
#ifdef DEFINE_DeeString_FormatPrinter
	Dee_formatprinter_t         sfd_sprinter; /* [1..1] Output printer for static data */
	Dee_formatprinter_t         sfd_dprinter; /* [1..1] Output printer for dynamic data */
	void                       *sfd_printarg; /* [?..?] Cookie for `sfd_sprinter' and `sfd_dprinter' */
	Dee_ssize_t                 sfd_result;   /* Result of invocations of `sfd_sprinter' and `sfd_dprinter' (or negative after error) */
#else /* DEFINE_DeeString_FormatPrinter */
	struct Dee_unicode_printer      sfd_uprinter; /* Output printer (content leading up to `sfd_parser.sfp_iter' is already printed) */
#endif /* !DEFINE_DeeString_FormatPrinter */
};
#endif /* ... */


#ifdef DEFINE_DeeString_FormatPrinter
#define LOCAL_string_format_data_account(err, sfd, temp) \
	do {                                                 \
		if unlikely((temp) < 0) {                        \
			(sfd)->sfd_result = (temp);                  \
			goto err;                                    \
		}                                                \
		(sfd)->sfd_result += (temp);                     \
	}	__WHILE0
#define LOCAL_string_format_data_getsprinter(sfd) ((sfd)->sfd_sprinter)
#define LOCAL_string_format_data_getdprinter(sfd) ((sfd)->sfd_dprinter)
#define LOCAL_string_format_data_getprintarg(sfd) ((sfd)->sfd_printarg)
#else /* DEFINE_DeeString_FormatPrinter */
#define LOCAL_string_format_data_getsprinter(sfd) ((Dee_formatprinter_t)&Dee_unicode_printer_print)
#define LOCAL_string_format_data_getdprinter(sfd) (&Dee_unicode_printer_print)
#define LOCAL_string_format_data_getprintarg(sfd) (&(sfd)->sfd_uprinter)
#endif /* !DEFINE_DeeString_FormatPrinter */

#ifndef LOCAL_string_format_data_account
#define LOCAL_string_format_data_account(err, sfd, temp) \
	do {                                                 \
		if unlikely((temp) < 0)                          \
			goto err;                                    \
	}	__WHILE0
#endif /* !LOCAL_string_format_data_account */

#define LOCAL_string_format_data_sprint(sfd, data, datalen) \
	((*LOCAL_string_format_data_getsprinter(sfd))(LOCAL_string_format_data_getprintarg(sfd), data, datalen))
#define LOCAL_string_format_data_dprint(sfd, data, datalen) \
	((*LOCAL_string_format_data_getdprinter(sfd))(LOCAL_string_format_data_getprintarg(sfd), data, datalen))


#if (defined(DEFINE_DeeString_FormatPrinter) || \
     (defined(DEFINE_DeeString_FormatWStr) && !defined(__OPTIMIZE_SIZE__)))

/************************************************************************/
/* ADVANCED STRING FORMATTING                                           */
/************************************************************************/


/* Implements advanced string format processing.
 * - When called, `data->sfd_parser.sfp_iter' points at the first character after the
 *   first unescaped '{', which is guarantied to not be one of "}!:", and
 *   `data->sfd_uprinter' contains any static template text leading up to
 *   (but obviously not including) this first unescaped '{'.
 * - Upon success (return >= 0), `data->sfd_uprinter' contains the final
 *   string, but other fields are left undefined.
 * - Upon error (return < 0), `data->sfd_uprinter' must be finalized by
 *   the caller, and other fields may be undefined.
 */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) Dee_ssize_t DFCALL
LOCAL_string_format_advanced(struct LOCAL_string_format_data *__restrict data, DeeObject *args) {
	Dee_ssize_t temp;
	char const *next_brace;
	struct string_format_advanced sfa;
	sfa.sfa_parser.sfp_iter = data->sfd_parser.sfp_iter;
	sfa.sfa_parser.sfp_wend = data->sfd_parser.sfp_wend;
	sfa.sfa_args            = args;
	sfa.sfa_printer         = LOCAL_string_format_data_getdprinter(data);
	sfa.sfa_arg             = LOCAL_string_format_data_getprintarg(data);

print_advanced_format_arg:
	sfa_yield(&sfa); /* Yield first token */
	temp = string_format_advanced_do1(&sfa);
	LOCAL_string_format_data_account(err_temp, data, temp);
	next_brace = find_next_brace(sfa.sfa_parser.sfp_iter,
	                             sfa.sfa_parser.sfp_wend);
handle_next_brace:
	if (!next_brace) {
		/* End of template pattern -> append remainder and exit foreach loop */
		temp = LOCAL_string_format_data_sprint(data, sfa.sfa_parser.sfp_iter,
		                                       (size_t)(sfa.sfa_parser.sfp_wend -
		                                                sfa.sfa_parser.sfp_iter));
		LOCAL_string_format_data_account(err_temp, data, temp);
		return 0;
	}

#ifdef DEFINE_DeeString_FormatPrinter
	/* Make sure there is at least 1 more char after the '{' */
	if likely((next_brace + 1) >= sfa.sfa_parser.sfp_wend)
		return err_invalid_char_after_lbrace_in_simple("");
#endif /* DEFINE_DeeString_FormatPrinter */

	/* Print static data until the next '{' or '}' */
	temp = LOCAL_string_format_data_sprint(data, sfa.sfa_parser.sfp_iter,
	                                       (size_t)(next_brace - sfa.sfa_parser.sfp_iter));
	LOCAL_string_format_data_account(err_temp, data, temp);

	/* Check for {{ or }}-escape */
	if unlikely(next_brace[0] == next_brace[1]) {
		sfa.sfa_parser.sfp_iter = next_brace + 1;
		next_brace = find_next_brace(next_brace + 2, sfa.sfa_parser.sfp_wend);
		goto handle_next_brace;
	}

	/* Make sure that it isn't a '}' */
	if unlikely(*next_brace == '}')
		return err_unmatched_rbrace_in_advanced();
	ASSERT(*next_brace == '{');
	sfa.sfa_parser.sfp_iter = next_brace + 1;
	goto print_advanced_format_arg;
err_temp:
	return temp;
}





/************************************************************************/
/* SIMPLE STRING FORMATTING                                             */
/************************************************************************/

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
LOCAL_parse_format_template_for_object_format_ex(struct LOCAL_string_format_data *data,
                                                 char const *flush_start, DeeObject *elem) {
	Dee_ssize_t result;
	char const *next_brace;
	struct Dee_unicode_printer format_str_printer;
	DREF DeeObject *format_str;
	Dee_unicode_printer_init(&format_str_printer);
	if unlikely(Dee_unicode_printer_printutf8(&format_str_printer, data->sfd_parser.sfp_iter,
	                                          (size_t)(flush_start - data->sfd_parser.sfp_iter) - 1) < 0)
		goto err_format_str_printer;
	next_brace = find_next_brace(flush_start, data->sfd_parser.sfp_wend);
again_handle_next_brace:
	if unlikely(!next_brace)
		goto err_format_str_printer_unmatched;
	if unlikely(next_brace[0] == next_brace[1]) {
		++next_brace;
/*handle_brace_after_escape_character:*/
		if unlikely(Dee_unicode_printer_printutf8(&format_str_printer, flush_start,
		                                          (size_t)(next_brace - flush_start) - 1) < 0)
			goto err_format_str_printer;
		flush_start = next_brace;
		next_brace  = find_next_brace(flush_start + 1, data->sfd_parser.sfp_wend);
		goto again_handle_next_brace;
	}
	if unlikely(next_brace[0] == '{')
		goto err_format_str_printer_err_lbrace_in_format_in_simple;
	if unlikely(Dee_unicode_printer_printutf8(&format_str_printer, flush_start,
	                                          (size_t)(next_brace - flush_start)) < 0)
		goto err_format_str_printer;
	data->sfd_parser.sfp_iter = next_brace;
	format_str = Dee_unicode_printer_pack(&format_str_printer);
	if unlikely(!format_str)
		goto err;
	result = DeeObject_PrintFormat(elem,
	                               LOCAL_string_format_data_getdprinter(data),
	                               LOCAL_string_format_data_getprintarg(data),
	                               format_str);
	Dee_Decref(format_str);
	return result;
err_format_str_printer_err_lbrace_in_format_in_simple:
	err_lbrace_in_format_in_simple();
	goto err_format_str_printer;
err_format_str_printer_unmatched:
	err_unmatched_lbrace_in_simple();
err_format_str_printer:
	Dee_unicode_printer_fini(&format_str_printer);
err:
	return -1;
}



/* Handler for "{:...}" patterns in simple template strings.
 * Called with `data->sfd_parser.sfp_iter' already pointing after the ':'-character.
 * - Upon success (return >= 0), `data->sfd_parser.sfp_iter' points at the closing '}'
 * - On error (return < 0), `data->sfd_parser.sfp_iter' is left undefined. */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_parse_format_template_for_object_format(struct LOCAL_string_format_data *data, DeeObject *elem) {
	char const *next_brace, *format_start;
	next_brace = find_next_brace(data->sfd_parser.sfp_iter, data->sfd_parser.sfp_wend);
	if unlikely(!next_brace)
		return err_unmatched_lbrace_in_simple();
	/* Check for {{ or }}-escape */
#ifdef DEFINE_DeeString_FormatPrinter
	if unlikely((next_brace + 1) < data->sfd_parser.sfp_wend &&
	            (next_brace[0] == next_brace[1]))
#else /* DEFINE_DeeString_FormatPrinter */
	if unlikely(next_brace[0] == next_brace[1])
#endif /* !DEFINE_DeeString_FormatPrinter */
	{
		return LOCAL_parse_format_template_for_object_format_ex(data, next_brace + 1, elem);
	}
	if unlikely(next_brace[0] == '{')
		return err_lbrace_in_format_in_simple();
	ASSERT(next_brace[0] == '}');
	format_start = data->sfd_parser.sfp_iter;
	data->sfd_parser.sfp_iter = next_brace;
	return DeeObject_PrintFormatString(elem,
	                                   LOCAL_string_format_data_getdprinter(data),
	                                   LOCAL_string_format_data_getprintarg(data),
	                                   format_start, (size_t)(next_brace - format_start));
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
LOCAL_string_format_wstr_foreach_cb(void *arg, DeeObject *elem) {
#ifdef DEFINE_DeeString_FormatPrinter
#define LOCAL_err_printerr done
#else /* DEFINE_DeeString_FormatPrinter */
#define LOCAL_err_printerr err
#endif /* !DEFINE_DeeString_FormatPrinter */
	Dee_ssize_t temp;
	char const *next_brace;
	struct LOCAL_string_format_data *data;
	data = (struct LOCAL_string_format_data *)arg;
	switch (*data->sfd_parser.sfp_iter) {

	case '}':
do_append_str:
		temp = DeeObject_Print(elem,
		                       LOCAL_string_format_data_getdprinter(data),
		                       LOCAL_string_format_data_getprintarg(data));
		break;

	case '!':
		++data->sfd_parser.sfp_iter;
		switch (__builtin_expect(*data->sfd_parser.sfp_iter, 'r')) {
		case 'r':
			++data->sfd_parser.sfp_iter;
			if unlikely(*data->sfd_parser.sfp_iter != '}')
				goto err_missing_rbrace;
/*do_append_repr:*/
			temp = DeeObject_PrintRepr(elem,
			                           LOCAL_string_format_data_getdprinter(data),
			                           LOCAL_string_format_data_getprintarg(data));
			break;
		case 's':
		case 'a':
			++data->sfd_parser.sfp_iter;
			if unlikely(*data->sfd_parser.sfp_iter != '}') {
err_missing_rbrace:
				return err_invalid_char_after_lbrace_exclaim_spec_in_simple(data->sfd_parser.sfp_iter[-1],
				                                                            data->sfd_parser.sfp_iter);
			}
			goto do_append_str;
		default:
			return err_unknown_repr_mode_in_simple(data->sfd_parser.sfp_iter);
		}
		break;

	case ':':
		/* Must use `DeeObject_PrintFormatString()' */
		++data->sfd_parser.sfp_iter;
		temp = LOCAL_parse_format_template_for_object_format(data, elem);
		break;

	default:
		/* When using a non-simple pattern string, *all*
		 * format elements must use a non-simple format. */
		return err_invalid_char_after_lbrace_in_simple(data->sfd_parser.sfp_iter);
	}
	LOCAL_string_format_data_account(LOCAL_err_printerr, data, temp);
	ASSERT(*data->sfd_parser.sfp_iter == '}');
	++data->sfd_parser.sfp_iter;
	next_brace = find_next_brace(data->sfd_parser.sfp_iter,
	                             data->sfd_parser.sfp_wend);
handle_next_lbrace:
	if (!next_brace) {
		/* End of template pattern -> append remainder and exit foreach loop */
		temp = LOCAL_string_format_data_sprint(data, data->sfd_parser.sfp_iter,
		                                       (size_t)(data->sfd_parser.sfp_wend -
		                                                data->sfd_parser.sfp_iter));
		LOCAL_string_format_data_account(LOCAL_err_printerr, data, temp);
		goto done;
	}
	if (next_brace[1] == next_brace[0]) {
		/* Special case: double left-or-right brace */
		++next_brace;
/*handle_brace_after_escape_character:*/
		temp = LOCAL_string_format_data_sprint(data, data->sfd_parser.sfp_iter,
		                                       (size_t)(next_brace - data->sfd_parser.sfp_iter) - 1);
		LOCAL_string_format_data_account(LOCAL_err_printerr, data, temp);
		data->sfd_parser.sfp_iter = next_brace;
		++next_brace;
		next_brace = find_next_brace(next_brace, data->sfd_parser.sfp_wend);
		goto handle_next_lbrace;
	}
	if unlikely(*next_brace == '}')
		return err_unmatched_rbrace_in_simple();
	ASSERT(*next_brace == '{');
	temp = LOCAL_string_format_data_sprint(data, data->sfd_parser.sfp_iter,
	                                       (size_t)(next_brace - data->sfd_parser.sfp_iter));
	LOCAL_string_format_data_account(LOCAL_err_printerr, data, temp);
	data->sfd_parser.sfp_iter = next_brace + 1;
	return 0;
done:
	return STRING_FORMAT_WSTR_FOREACH_DONE;
#ifndef DEFINE_DeeString_FormatPrinter
err:
	return -1;
#endif /* !DEFINE_DeeString_FormatPrinter */
#undef LOCAL_err_printerr
}
#else /* DEFINE_DeeString_FormatPrinter || (DEFINE_DeeString_FormatWStr && !__OPTIMIZE_SIZE__) */

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) Dee_ssize_t DFCALL
LOCAL_string_format_advanced(struct LOCAL_string_format_data *__restrict data,
                             DeeObject *args);

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
LOCAL_string_format_wstr_foreach_cb(void *arg, DeeObject *elem);

#endif /* !DEFINE_DeeString_FormatPrinter && (!DEFINE_DeeString_FormatWStr || __OPTIMIZE_SIZE__) */



#ifdef DEFINE_DeeString_FormatPrinter
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeString_FormatPrinter(char const *pattern, size_t pattern_length, DeeObject *args,
                        Dee_formatprinter_t pattern_printer,
                        Dee_formatprinter_t data_printer, void *arg)
#define LOCAL_pattern        pattern
#define LOCAL_pattern_length pattern_length
#elif defined(DEFINE_DeeString_FormatWStr)
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
DeeString_FormatWStr(/*utf-8*/ char const *pattern_wstr, DeeObject *args)
#define LOCAL_pattern        pattern_wstr
#define LOCAL_pattern_length WSTR_LENGTH(pattern_wstr)
#else /* DEFINE_DeeString_Format */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
DeeString_Format(DeeObject *pattern, DeeObject *args)
#define LOCAL_pattern        pattern_wstr
#define LOCAL_pattern_length WSTR_LENGTH(pattern_wstr)
#endif
{
#if defined(DEFINE_DeeString_Format) && defined(__OPTIMIZE_SIZE__)
	char const *pattern_wstr;
	pattern_wstr = DeeString_AsUtf8(pattern);
	if unlikely(!pattern_wstr)
		return NULL;
	return DeeString_FormatWStr(pattern_wstr, args);
#elif defined(DEFINE_DeeString_FormatWStr) && defined(__OPTIMIZE_SIZE__)
	Dee_ssize_t status;
	struct Dee_unicode_printer printer;
	Dee_unicode_printer_init(&printer);
	status = DeeString_FormatPrinter(pattern_wstr, WSTR_LENGTH(pattern_wstr), args,
	                                 (Dee_formatprinter_t)&Dee_unicode_printer_printutf8,
	                                 &Dee_unicode_printer_print, &printer);
	if unlikely(status < 0)
		goto err;
	return Dee_unicode_printer_pack(&printer);
err:
	Dee_unicode_printer_fini(&printer);
	return NULL;
#else /* ... */
#ifdef DEFINE_DeeString_Format
	char const *pattern_wstr;
#endif /* DEFINE_DeeString_Format */
	Dee_ssize_t temp;
	struct LOCAL_string_format_data data;
#ifdef DEFINE_DeeString_Format
	pattern_wstr = DeeString_AsUtf8(pattern);
	if unlikely(!pattern_wstr)
		goto err;
#define WANT_err
#endif /* DEFINE_DeeString_Format */

	data.sfd_parser.sfp_wend = LOCAL_pattern + LOCAL_pattern_length;
	data.sfd_parser.sfp_iter = find_next_brace(LOCAL_pattern, data.sfd_parser.sfp_wend);
	if unlikely(!data.sfd_parser.sfp_iter) {
#ifdef DEFINE_DeeString_Format
		return_reference_(pattern); /* Can just re-return "pattern" as-is. */
#elif defined(DEFINE_DeeString_FormatWStr)
		return DeeString_NewUtf8(LOCAL_pattern, LOCAL_pattern_length, STRING_ERROR_FIGNORE);
#elif defined(DEFINE_DeeString_FormatPrinter)
		return (*pattern_printer)(arg, LOCAL_pattern, LOCAL_pattern_length);
#endif /* ... */
	}

#ifdef DEFINE_DeeString_FormatPrinter
	data.sfd_sprinter = pattern_printer;
	data.sfd_dprinter = data_printer;
	data.sfd_printarg = arg;
	data.sfd_result   = 0;
#define LOCAL_err_printerr done
#else /* DEFINE_DeeString_FormatPrinter */
	Dee_unicode_printer_init(&data.sfd_uprinter);
#define LOCAL_err_printerr err_printer
#endif /* !DEFINE_DeeString_FormatPrinter */
again_handle_brace:
#ifdef DEFINE_DeeString_FormatPrinter
	/* Make sure there is at least 1 more char after the '{' */
	if unlikely((data.sfd_parser.sfp_iter + 1) >= data.sfd_parser.sfp_wend)
		return err_invalid_char_after_lbrace_in_simple("");
#endif /* DEFINE_DeeString_FormatPrinter */
	temp = LOCAL_string_format_data_sprint(&data, LOCAL_pattern,
	                                       (size_t)(data.sfd_parser.sfp_iter - LOCAL_pattern));
	LOCAL_string_format_data_account(LOCAL_err_printerr, &data, temp);
	if unlikely(*data.sfd_parser.sfp_iter == '}') {
		if likely(data.sfd_parser.sfp_iter[1] == '}') {
			/* Special case: double right brace '}}' */
/*handle_brace_before_escape_character:*/
			++data.sfd_parser.sfp_iter;
			goto handle_brace_after_escape_character;
		}
#ifdef DEFINE_DeeString_FormatPrinter
		data.sfd_result = err_unmatched_rbrace_in_simple();
#else /* DEFINE_DeeString_FormatPrinter */
		err_unmatched_rbrace_in_simple();
#endif /* !DEFINE_DeeString_FormatPrinter */
		goto LOCAL_err_printerr;
	}

	/* At this point, we've got the first unescaped '{' at "data.sfd_parser.sfp_iter".
	 * Figure out if this is a simple, or advanced pattern and act accordingly. */
	++data.sfd_parser.sfp_iter;
	switch (__builtin_expect(*data.sfd_parser.sfp_iter, '}')) {
	case '}':
	case '!':
	case ':': {
		/* Simple pattern */

		/* Assume that this is a "simple" format string, as generated by template strings.
		 * >> local x = f"foo = {foo}, bar = {bar}";
		 * generated:
		 * >> local x = "foo = {}, bar = {}".format({ foo, bar });
		 *
		 * In this case, we use DeeObject_Foreach() to enumerate `args'
		 * and append format string parts, as well as argument elements
		 * onto the resulting string. */
		temp = DeeObject_Foreach(args, &LOCAL_string_format_wstr_foreach_cb, &data);
		if likely(temp == STRING_FORMAT_WSTR_FOREACH_DONE)
			goto done; /* Likely case: the pattern was fully printed without a premature end of `args' */
		if unlikely(temp < 0) {
#ifdef DEFINE_DeeString_FormatPrinter
			data.sfd_result = temp;
#endif /* DEFINE_DeeString_FormatPrinter */
			goto LOCAL_err_printerr;
		}
		/* Error: `args' ended too early */
		goto err_not_enough_args;
	}	break;

	case '{':
handle_brace_after_escape_character:
		/* Special case: double left-or-right brace */
		LOCAL_pattern = data.sfd_parser.sfp_iter;
		data.sfd_parser.sfp_iter = find_next_brace(LOCAL_pattern + 1, data.sfd_parser.sfp_wend);
		if unlikely(!data.sfd_parser.sfp_iter) {
/*done_printer_remainder:*/
			temp = LOCAL_string_format_data_sprint(&data, LOCAL_pattern,
			                                       (size_t)(data.sfd_parser.sfp_wend - LOCAL_pattern));
			LOCAL_string_format_data_account(LOCAL_err_printerr, &data, temp);
			goto done;
		}
		goto again_handle_brace;

	default:
		/* Advanced pattern */
		temp = LOCAL_string_format_advanced(&data, args);
		if unlikely(temp < 0) {
#ifdef DEFINE_DeeString_FormatPrinter
			data.sfd_result = temp;
#endif /* DEFINE_DeeString_FormatPrinter */
			goto LOCAL_err_printerr;
		}
		break;
	}
done:
#ifdef DEFINE_DeeString_FormatPrinter
	return data.sfd_result;
#else /* DEFINE_DeeString_FormatPrinter */
	return Dee_unicode_printer_pack(&data.sfd_uprinter);
#endif /* !DEFINE_DeeString_FormatPrinter */
err_not_enough_args:
	DeeError_Throwf(&DeeError_UnpackError,
	                "Insufficient number of arguments");
#ifndef DEFINE_DeeString_FormatPrinter
err_printer:
	Dee_unicode_printer_fini(&data.sfd_uprinter);
#endif /* !DEFINE_DeeString_FormatPrinter */
#ifdef WANT_err
#undef WANT_err
err:
#endif /* WANT_err */
#ifdef DEFINE_DeeString_FormatPrinter
	return -1;
#else /* DEFINE_DeeString_FormatPrinter */
	return NULL;
#endif /* !DEFINE_DeeString_FormatPrinter */
#undef LOCAL_err_printerr
#undef LOCAL_pattern_length
#undef LOCAL_pattern
#endif /* !... */
}


#undef LOCAL_string_format_data
#undef LOCAL_string_format_advanced
#undef LOCAL_parse_format_template_for_object_format_ex
#undef LOCAL_parse_format_template_for_object_format
#undef LOCAL_string_format_wstr_foreach_cb

#undef LOCAL_string_format_data_getsprinter
#undef LOCAL_string_format_data_getdprinter
#undef LOCAL_string_format_data_getprintarg
#undef LOCAL_string_format_data_account

DECL_END

#undef DEFINE_DeeString_FormatPrinter
#undef DEFINE_DeeString_FormatWStr
#undef DEFINE_DeeString_Format
