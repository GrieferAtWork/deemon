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
#ifndef GUARD_DEX_IPC_GENERIC_CMDLINE_C_INL
#define GUARD_DEX_IPC_GENERIC_CMDLINE_C_INL 1
#define DEE_SOURCE
#define _KOS_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

#include "libipc.h"

#if (!defined(CONFIG_HAVE_LIBCMDLINE) && !defined(CONFIG_NO_LIBCMDLINE) && \
     (__has_include(<libcmdline/api.h>) && __has_include(<libcmdline/encode.h>) && \
      __has_include(<libcmdline/decode.h>) && __has_include(<dlfcn.h>)) || \
    (defined(__KOS__) && defined(__KOS_VERSION__) && (__KOS_VERSION__ >= 400)))
#define CONFIG_HAVE_LIBCMDLINE
#endif /* ... */

#ifdef CONFIG_HAVE_LIBCMDLINE
#include <dlfcn.h>

#include <libcmdline/api.h>
#include <libcmdline/decode.h>
#include <libcmdline/encode.h>

#ifndef LIBCMDLINE_LIBRARY_NAME
#undef CONFIG_HAVE_LIBCMDLINE
#endif /* !LIBCMDLINE_LIBRARY_NAME */
#endif /* CONFIG_HAVE_LIBCMDLINE */

#ifndef CONFIG_HAVE_LIBCMDLINE
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* strend() */
#endif /* !CONFIG_HAVE_LIBCMDLINE */

DECL_BEGIN

/* Encode the given `arg_start' argument by escaping characters
 * that would confuse the commandline decoder, and print the
 * resulting string using the given `printer' with `arg'
 * The caller is still responsible to insert space-separators with
 * a width of at least 1 space-character (' ') between successive
 * arguments. Alternatively, you may also use `cmdline_encode()' to
 * encode an entire commandline at once.
 * @return: * : The sum of return values of `printer'
 * @return: <0: The propagation of the first negative return value of `printer' (if any) */
PRIVATE NONNULL((1, 3)) dssize_t DCALL
process_cmdline_encode_argument(dformatprinter printer, void *arg,
                                char const *arg_start, size_t arg_len);

/* Decode and transform a given `cmdline' (which must be a \0-terminated string),
 * and invoke `arg_printer' with each individual argument segment.
 * NOTE: When `arg_printer' return an error ((*arg_printer)(...) < 0), that error
 *       is propagated and re-returned by this function. In this case, `cmdline'
 *       is left in an undefined state.
 * This function recognizes the following commandline rules:
 *   - >\x<    can be used to escape a given character `x'
 *             This character can be anything, and in the leading \-character
 *             is always removed, meaning that in order to include a \-character
 *             in the actual argument vector, \\ must be written
 *   - >a b<   Any sort of space characters (s.a. `DeeUni_IsSpace()') is recognized
 *             as a suitable separator between arguments.
 *             Multiple consecutive space characters are merged into a single one.
 *   - >"a b"< Write text in space-escaped mode. - In this mode, space characters
 *             do not mark separate arguments. Additionally, the leading and trailing
 *             "-characters are removed from the generated arguments
 *   - >'a b'< Same as "a b", but with this, you can do >"How's it going"< or
 *             >'I said "Hello"'< instead of having to use >How\'s it going<
 *             and >I said \"Hello\"<
 *   - >""<    Special case: When >""< or >''< is encountered, but is surrounded
 *             by whitespace, or the start/end of the commandline, an empty argument
 *             will be emit (see examples below)
 * Examples:
 *     >a b c<                  { "a", "b", "c" }
 *     >ls "New Folder"<        { "ls", "New Folder" }
 *     >ls New\ Folder<         { "ls", "New Folder" }
 *     >ls "" foo<              { "ls", "", "foo" }     // Empty argument!
 */
PRIVATE NONNULL((1, 2)) dssize_t DCALL
process_cmdline_decode(char *cmdline,
                       dformatprinter arg_printer,
                       void *arg_printer_arg);


/* Decode a given commandline in its entirety, returning
 * the argv tuple, and storing the executable name in `*pexe' */
PRIVATE NONNULL((1, 2)) DREF DeeObject *DCALL
process_cmdline_decode_full(char const *__restrict cmdline,
                            DREF DeeObject **__restrict pexe);


#ifdef CONFIG_HAVE_LIBCMDLINE

#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */

#ifndef CMDLINE_ENCODE_ARGUMENT_NAME
#define CMDLINE_ENCODE_ARGUMENT_NAME "cmdline_encode_argument"
#endif /* !CMDLINE_ENCODE_ARGUMENT_NAME */

#ifndef CMDLINE_DECODE_NAME
#define CMDLINE_DECODE_NAME "cmdline_decode"
#endif /* !CMDLINE_DECODE_NAME */

PRIVATE PCMDLINE_ENCODE_ARGUMENT pdyn_cmdline_encode_argument = NULL;
PRIVATE PCMDLINE_DECODE pdyn_cmdline_decode = NULL;
PRIVATE void *libcmdline = NULL;

#define HAVE_LIBCMDLINE_FINI 1
PRIVATE NONNULL((1)) void DCALL
libcmdline_fini(DeeDexObject *__restrict UNUSED(self)) {
	if (libcmdline)
		dlclose(libcmdline);
}

PRIVATE bool DCALL libcmdline_init(void) {
	void *lib = ATOMIC_READ(libcmdline);
	if (lib) {
		if (lib == (void *)(uintptr_t)-1)
			goto err;
		return true;
	}
	lib = dlopen(LIBCMDLINE_LIBRARY_NAME, RTLD_LOCAL);
	if unlikely(!lib)
		goto err_disable;
	*(void **)&pdyn_cmdline_encode_argument = dlsym(lib, CMDLINE_ENCODE_ARGUMENT_NAME);
	if unlikely(!pdyn_cmdline_encode_argument)
		goto err_disable;
	*(void **)&pdyn_cmdline_decode = dlsym(lib, CMDLINE_DECODE_NAME);
	if unlikely(!pdyn_cmdline_decode)
		goto err_disable;
	if (!ATOMIC_CMPXCH(libcmdline, NULL, lib))
		dlclose(lib);
	return true;
err_disable:
	ATOMIC_WRITE(libcmdline, (void *)(uintptr_t)-1);
err:
	DeeError_Throwf(&DeeError_NotImplemented,
	                "Failed to load system library %q",
	                LIBCMDLINE_LIBRARY_NAME);
	return false;
}


struct cmdline_encode_argument_wrapper_data {
	dformatprinter wd_printer;
	void          *wd_arg;
};

PRIVATE dssize_t __LIBCCALL
cmdline_encode_argument_wrapper_func(void *arg, char const *data, size_t datalen) {
	struct cmdline_encode_argument_wrapper_data *p;
	p = (struct cmdline_encode_argument_wrapper_data *)arg;
	return (*p->wd_printer)(p->wd_arg, data, datalen);
}

PRIVATE NONNULL((1, 3)) dssize_t DCALL
process_cmdline_encode_argument(dformatprinter printer, void *arg,
                                char const *arg_start, size_t arg_len) {
	struct cmdline_encode_argument_wrapper_data p;
	if (!libcmdline_init())
		goto err;
	p.wd_printer = printer;
	p.wd_arg     = arg;
	return (*pdyn_cmdline_encode_argument)(&cmdline_encode_argument_wrapper_func,
	                                       &p, arg_start, arg_len);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) dssize_t DCALL
process_cmdline_decode(char *cmdline,
                       dformatprinter arg_printer,
                       void *arg_printer_arg) {
	struct cmdline_encode_argument_wrapper_data p;
	if (!libcmdline_init())
		goto err;
	p.wd_printer = arg_printer;
	p.wd_arg     = arg_printer_arg;
	return (*pdyn_cmdline_decode)(cmdline,
	                              &cmdline_encode_argument_wrapper_func,
	                              &p);
err:
	return -1;
}

#else /* CONFIG_HAVE_LIBCMDLINE */

LOCAL NONNULL((1, 3, 4)) dssize_t DCALL
encode_escape_backslash(dformatprinter printer, void *arg,
                        char const *arg_start,
                        char const *arg_end) {
	dssize_t temp, result = 0;
	uint32_t ch;
	char const *flush_start = arg_start, *iter, *prev;
	for (iter = prev = arg_start; (ch = utf8_readchar(&iter, arg_end)) != 0; prev = iter) {
		if (ch == '\\') {
			temp = (*printer)(arg, flush_start, (size_t)(prev - flush_start));
			if unlikely(temp < 0)
				goto err;
			result += temp;
			temp = (*printer)(arg, "\\", 1);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			flush_start = prev;
		}
	}
	if (flush_start < arg_end) {
		temp = (*printer)(arg, flush_start, (size_t)(arg_end - flush_start));
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

LOCAL NONNULL((1, 3, 4)) dssize_t DCALL
encode_escape_spc(dformatprinter printer, void *arg,
                  char const *arg_start,
                  char const *arg_end) {
	dssize_t temp, result = 0;
	uint32_t ch;
	char const *flush_start = arg_start, *iter, *prev;
	for (iter = prev = arg_start; (ch = utf8_readchar(&iter, arg_end)) != 0; prev = iter) {
		if (DeeUni_IsSpace(ch) || ch == '\\') {
			temp = (*printer)(arg, flush_start, (size_t)(prev - flush_start));
			if unlikely(temp < 0)
				goto err;
			result += temp;
			temp = (*printer)(arg, "\\", 1);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			flush_start = prev;
		}
	}
	if (flush_start < arg_end) {
		temp = (*printer)(arg, flush_start, (size_t)(arg_end - flush_start));
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

LOCAL NONNULL((1, 3, 4)) dssize_t DCALL
encode_escape_all(dformatprinter printer, void *arg,
                  char const *arg_start,
                  char const *arg_end) {
	dssize_t temp, result = 0;
	uint32_t ch;
	char const *flush_start = arg_start, *iter, *prev;
	for (iter = prev = arg_start; (ch = utf8_readchar(&iter, arg_end)) != 0; prev = iter) {
		if (DeeUni_IsSpace(ch) || ch == '\'' || ch == '\"' || ch == '\\') {
			temp = (*printer)(arg, flush_start, (size_t)(prev - flush_start));
			if unlikely(temp < 0)
				goto err;
			result += temp;
			temp = (*printer)(arg, "\\", 1);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			flush_start = prev;
		}
	}
	if (flush_start < arg_end) {
		temp = (*printer)(arg, flush_start, (size_t)(arg_end - flush_start));
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

LOCAL NONNULL((1, 3, 4)) dssize_t DCALL
encode_escape_q(dformatprinter printer, void *arg,
                char const *arg_start,
                char const *arg_end, char qchar) {
	dssize_t temp, result = 0;
	uint32_t ch;
	char const *flush_start = arg_start, *iter, *prev;
	for (iter = prev = arg_start; (ch = utf8_readchar(&iter, arg_end)) != 0; prev = iter) {
		if (ch == (uint32_t)(unsigned char)qchar || ch == '\\') {
			temp = (*printer)(arg, flush_start, (size_t)(prev - flush_start));
			if unlikely(temp < 0)
				goto err;
			result += temp;
			temp = (*printer)(arg, "\\", 1);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			flush_start = prev;
		}
	}
	if (flush_start < arg_end) {
		temp = (*printer)(arg, flush_start, (size_t)(arg_end - flush_start));
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

LOCAL NONNULL((1, 3)) dssize_t DCALL
encode_quote(dformatprinter printer, void *arg,
             char const *arg_start,
             size_t arg_len, char qchar,
             unsigned int num_slash) {
	dssize_t temp, result;
	result = (*printer)(arg, &qchar, 1);
	if unlikely(result < 0)
		goto done;
	temp = num_slash ? encode_escape_backslash(printer, arg, arg_start, arg_start + arg_len)
	                 : (*printer)(arg, arg_start, arg_len);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	temp = (*printer)(arg, &qchar, 1);
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

LOCAL NONNULL((1, 3, 4)) dssize_t DCALL
encode_escape_quote(dformatprinter printer, void *arg,
                    char const *arg_start,
                    char const *arg_end,
                    char qchar) {
	dssize_t temp, result;
	result = (*printer)(arg, &qchar, 1);
	if unlikely(result < 0)
		goto done;
	temp = encode_escape_q(printer, arg, arg_start, arg_end, qchar);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	temp = (*printer)(arg, &qchar, 1);
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

PRIVATE NONNULL((1, 3)) dssize_t DCALL
process_cmdline_encode_argument(dformatprinter printer, void *arg,
                                char const *arg_start, size_t arg_len) {
	dssize_t result;
	uint32_t ch;
	char const *iter, *arg_end;
	unsigned int num_spaces, num_dquote, num_squote, num_slash;
	/* Check for special case: Empty argument */
	if (!arg_len)
		return (*printer)(arg, "\"\"", 2);
	arg_end = arg_start + arg_len;
	num_spaces = 0; /* Number of space characters */
	num_dquote = 0; /* Number of "-characters */
	num_squote = 0; /* Number of '-characters */
	num_slash  = 0; /* Number of \-characters */
	for (iter = arg_start; (ch = utf8_readchar(&iter, arg_end)) != 0; ) {
		if (DeeUni_IsSpace(ch))
			++num_spaces;
		else {
			if (ch == '\'') {
				++num_squote;
			} else if (ch == '\"') {
				++num_dquote;
			} else if (ch == '\\') {
				++num_slash;
			}
		}
	}
	/* Choose the most compact encoding. */
	if (!num_squote && !num_dquote) {
		if (!num_spaces) {
			if (!num_slash) {
				/* Nothing needs to be escaped (pretty much the most likely case...) */
				result = (*printer)(arg, arg_start, arg_len);
			} else {
				result = encode_escape_backslash(printer, arg, arg_start, arg_end);
			}
		} else {
			if (num_spaces <= 1) {
				/* Only escape the space characters */
				result = encode_escape_spc(printer, arg, arg_start, arg_end);
			} else {
				/* Surround the argument with quotes (which quote is used doesn't matter for this). */
				goto escape_double_quote;
			}
		}
	} else if (!num_squote) {
		if ((num_dquote + num_spaces) < 2) {
escape_all:
			result = encode_escape_all(printer, arg, arg_start, arg_end);
		} else {
			/* Surround the argument with single quotes. */
			result = encode_quote(printer, arg, arg_start, arg_len, '\'', num_slash);
		}
	} else if (!num_dquote) {
		if ((num_squote + num_spaces) < 2)
			goto escape_all;
		/* Surround the argument with double quotes. */
escape_double_quote:
		result = encode_quote(printer, arg, arg_start, arg_len, '\"', num_slash);
	} else {
		/* Special case: If we don't need to escape any space characters,
		 * and have exactly 1 of each quotes, we can still encode the entire
		 * argument with only 2 additional characters injected to escape both
		 * quotes.
		 * Without this check, such a string would be encoded with +3 additional
		 * characters:
		 *   encode(fo"b'ar) --> fo\"b\'ar   // This optimization
		 *                   --> "fo\"b'ar"  // What would otherwise happen
		 */
#if 1
		if (num_squote == 1 && num_dquote == 1 && !num_spaces)
			goto escape_all;
#endif
		/* There are both single-quote and double-quote characters
		 * If there are more ' and ", escape all ", else escape all with ' */
		if (num_squote >= num_dquote) {
			result = encode_escape_quote(printer, arg, arg_start, arg_end, '\"');
		} else {
			result = encode_escape_quote(printer, arg, arg_start, arg_end, '\'');
		}
	}
	return result;
}

/* Trait checking for commandline special characters. */
#define cmdline_isquote(ch) ((ch) == '\'' || (ch) == '\"')
#define cmdline_isbslsh(ch) ((ch) == '\\')

PRIVATE NONNULL((1, 2)) dssize_t DCALL
process_cmdline_decode(char *cmdline,
                       dformatprinter arg_printer,
                       void *arg_printer_arg) {
	char *cmdline_end;
	dssize_t temp, result = 0;
	char *next_ch, *arg_start;
	uint32_t ch;
	/* Skip leading space. */
	for (;;) {
		next_ch = cmdline;
		ch      = utf8_readchar_u((char const **)&next_ch);
		if (!DeeUni_IsSpace(ch))
			break;
		cmdline = next_ch;
	}
	/* Check for empty commandline. */
	if unlikely(!ch)
		goto done;
	arg_start   = cmdline;
	cmdline_end = strend(cmdline);
	for (;;) {
parse_next_ch:
		/* Escaped characters. */
		if (cmdline_isbslsh(ch)) {
			if (next_ch >= cmdline_end)
				goto done_cmdline;
			memmovedownc(cmdline, next_ch,
			             (size_t)(cmdline_end - next_ch),
			             sizeof(char));
			cmdline_end -= (size_t)(next_ch - cmdline);
			next_ch = cmdline;
			ch      = utf8_readchar_u((char const **)&next_ch); /* Skip the next character */
			/* Read the next non-escaped character */
			goto read_next_ch;
		}
		/* String arguments. */
		if (cmdline_isquote(ch)) {
			uint32_t end_ch = ch;
			if (next_ch >= cmdline_end)
				goto done_cmdline;
			memmovedownc(cmdline, next_ch,
			             (size_t)(cmdline_end - next_ch),
			             sizeof(char));
			cmdline_end -= (size_t)(next_ch - cmdline);
			next_ch = cmdline;
			for (;;) {
				cmdline = next_ch;
				if unlikely(next_ch >= cmdline_end)
					goto done_cmdline;
				ch = utf8_readchar_u((char const **)&next_ch);
				if (cmdline_isbslsh(ch)) {
					if (next_ch >= cmdline_end)
						goto done_cmdline;
					memmovedownc(cmdline, next_ch,
					             (size_t)(cmdline_end - next_ch),
					             sizeof(char));
					cmdline_end -= (size_t)(next_ch - cmdline);
					next_ch = cmdline;
					ch      = utf8_readchar_u((char const **)&next_ch); /* Skip the next character */
					/* Read the next non-escaped character */
					cmdline = next_ch;
					if (next_ch >= cmdline_end)
						goto done_cmdline;
					utf8_readchar_u((char const **)&next_ch);
					continue;
				}
				if (ch == end_ch) {
					if (next_ch >= cmdline_end)
						goto done_cmdline;
					memmovedownc(cmdline, next_ch,
					             (size_t)(cmdline_end - next_ch),
					             sizeof(char));
					cmdline_end -= (size_t)(next_ch - cmdline);
					next_ch = cmdline;
					if (next_ch >= cmdline_end) {
						if unlikely(arg_start == cmdline) {
							/* Special case: Empty, trailing argument */
							arg_start[0] = '\0';
							temp         = (*arg_printer)(arg_printer_arg, arg_start, 0);
							if unlikely(temp < 0)
								goto err;
							result += temp;
						}
						goto done_cmdline;
					}
					ch = utf8_readchar_u((char const **)&next_ch);
					if unlikely(arg_start == cmdline && DeeUni_IsSpace(ch)) {
						/* Special case: Empty argument (foo bar "" baz --> { "foo", "bar", "", "baz" }) */
						/* To keep all strings apart of the given cmdline, reclaim 1 character
						 * which we can then set to \0. - Note that we are guarantied to have at
						 * least 2 bytes available, due to the 2 " or '-characters */
						memmoveupc(arg_start + 1, arg_start,
						           (size_t)(cmdline_end - arg_start),
						           sizeof(char));
						arg_start[0] = '\0';
						temp         = (*arg_printer)(arg_printer_arg, arg_start, 0);
						if unlikely(temp < 0)
							goto err;
						result += temp;
						++cmdline_end;
						arg_start = cmdline = ++next_ch;
						goto skip_secondary_whitespace;
					}
					goto parse_next_ch;
				}
			}
		}
		/* Space -> Argument separator. */
		if (DeeUni_IsSpace(ch)) {
			*cmdline = '\0'; /* Terminate the previous argument. */
			/* Invoke the caller's function with this new argument. */
			temp = (*arg_printer)(arg_printer_arg,
			                      arg_start,
			                      (size_t)(cmdline - arg_start));
			if unlikely(temp < 0)
				goto err;
			result += temp;
skip_secondary_whitespace:
			/* Skip multiple consecutive spaces. */
			for (;;) {
				if (next_ch >= cmdline_end)
					goto done; /* Commandline ends with space */
				arg_start = next_ch;
				ch = utf8_readchar_u((char const **)&next_ch);
				if (!DeeUni_IsSpace(ch))
					break;
			}
			/* Next non-space character was found */
			cmdline = arg_start;
			goto parse_next_ch;
		}
read_next_ch:
		cmdline = next_ch;
		if (next_ch >= cmdline_end)
			goto done_cmdline;
		ch = utf8_readchar_u((char const **)&next_ch);
	}
done_cmdline:
	*cmdline = '\0';
	temp = (*arg_printer)(arg_printer_arg,
	                      arg_start,
	                      (size_t)(cmdline - arg_start));
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

#endif /* !CONFIG_HAVE_LIBCMDLINE */

struct process_cmdline_decode_full_data {
	DREF DeeObject *dfd_restuple; /* [1..1] Result tuple. */
	DREF DeeObject *dfd_resexe;   /* [0..1] Result exe. */
};

PRIVATE WUNUSED NONNULL((2)) dssize_t DPRINTER_CC
process_cmdline_decode_full_func(void *arg,
                                 char const *__restrict data,
                                 size_t datalen) {
	DREF DeeObject *new_argv;
	DREF DeeObject *data_str;
	struct process_cmdline_decode_full_data *p;
	p = (struct process_cmdline_decode_full_data *)arg;
	data_str = DeeString_NewUtf8(data, datalen, STRING_ERROR_FIGNORE);
	if unlikely(!data_str)
		goto err;
	if (!p->dfd_resexe) {
		Dee_Incref(data_str);
		p->dfd_resexe = data_str;
	}
	new_argv = DeeTuple_Append(p->dfd_restuple, data_str);
	if unlikely(!new_argv)
		goto err_data_str;
	p->dfd_restuple = new_argv;
	Dee_DecrefNokill(data_str);
	return 0;
err_data_str:
	Dee_Decref(data_str);
err:
	return -1;
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
process_cmdline_decode_full_inplace(char *__restrict cmdline,
                                    DREF DeeObject **pexe) {
	struct process_cmdline_decode_full_data data;
	Dee_Incref(&DeeTuple_Empty);
	data.dfd_restuple = &DeeTuple_Empty;
	data.dfd_resexe   = NULL;
	if (process_cmdline_decode(cmdline,
	                           &process_cmdline_decode_full_func,
	                           &data) < 0) {
		Dee_Decref(data.dfd_restuple);
		Dee_XDecref(data.dfd_resexe);
		return NULL;
	}
	if (!data.dfd_resexe) {
		data.dfd_resexe = Dee_EmptyString;
		Dee_Incref(Dee_EmptyString);
	}
	if (pexe) {
		*pexe = data.dfd_resexe; /* Inherit reference */
	} else {
		Dee_Decref(data.dfd_resexe);
	}
	return data.dfd_restuple;
}


/* Decode a given commandline in its entirety, returning
 * the argv tuple, and storing the executable name in `*pexe' */
PRIVATE NONNULL((1)) DREF DeeObject *DCALL
process_cmdline_decode_full(char const *__restrict cmdline,
                            DREF DeeObject **pexe) {
	DREF DeeObject *result;
	size_t cmdline_len;
	char *cmdline_copy;
	cmdline_len  = strlen(cmdline);
	cmdline_copy = (char *)Dee_Mallocc(cmdline_len + 1, sizeof(char));
	if unlikely(!cmdline_copy)
		goto err;
	memcpyc(cmdline_copy, cmdline, cmdline_len + 1, sizeof(char));
	result = process_cmdline_decode_full_inplace(cmdline_copy, pexe);
	Dee_Free(cmdline_copy);
	return result;
err:
	return NULL;
}




DECL_END

#endif /* !GUARD_DEX_IPC_GENERIC_CMDLINE_C_INL */
