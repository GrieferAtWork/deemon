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
#ifndef GUARD_DEEMON_COMPILER_TPP_C
#define GUARD_DEEMON_COMPILER_TPP_C 1
#define TPP_SYMARRAY_SIZE 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* DeeObject_*alloc*, DeeObject_Free, Dee_Alloca, Dee_Free, Dee_Try*alloc* */
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/exec.h>              /* DeeModule_GetLibPath, DeeModule_GetPath */
#include <deemon/file.h>              /* DeeFileObject, DeeFile_*, Dee_FILEIO_FNONBLOCKING, Dee_STDOUT, OPEN_FCLOEXEC, OPEN_FRDONLY */
#include <deemon/format.h>            /* DeeFormat_Printf, Dee_sprintf, Dee_vsprintf */
#include <deemon/list.h>              /* DeeListObject, DeeList_* */
#include <deemon/object.h>
#include <deemon/string.h>            /* DeeString*, Dee_string_utf_fini, Dee_string_utf_free */
#include <deemon/system-features.h>   /* DeeSystem_DEFINE_memrchr, mempcpyc */
#include <deemon/system.h>            /* DeeSystem_* */
#include <deemon/thread.h>            /* DeeThread_CheckInterrupt */
#include <deemon/tuple.h>             /* DeeTuple* */
#include <deemon/types.h>             /* DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, ITER_DONE */

#include <hybrid/typecore.h> /* __SIZEOF_INT__, __SIZEOF_POINTER__ */

#include <stdarg.h>  /* va_end, va_list, va_start */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint8_t */


#define PRIVDEF PRIVATE
#undef PUBLIC
#undef TPPFUN
#define PUBLIC  INTERN
#define TPPFUN  INTDEF
#define TPP_USERDEFS                  <deemon/compiler/lexer.def>
#define TPP_USERSTREAM_FCLOSE(stream) Dee_Decref(stream)

/* Includes for TPP */
/* clang-format off */
#include <deemon/class.h>
#include <deemon/compiler/error.h>
#include <deemon/error.h> /* DeeError_* */
#include <deemon/error_types.h> /* DeeCompilerErrorObject, DeeCompilerError_Print, Dee_compiler_error_loc */
#include <deemon/module.h> /* Dee_COMPILER_ERROR_FATALITY_WARNING */
#include <hybrid/byteswap.h> /* BSWAP* */
/* clang-format on */

DECL_BEGIN

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

#define TPP_USERSTREAM_FOPEN(filename) \
	tpp_userstream_fopen(filename)
PRIVATE stream_t DCALL
tpp_userstream_fopen(char const *__restrict filename) {
	stream_t result;
	result = (stream_t)DeeFile_OpenString(filename, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
	if (!result) {
		TPPLexer_SetErr(); /* Set a lexer error to indicate failure. */
	} else if (result == (stream_t)ITER_DONE) {
		result = NULL; /* Return NULL to indicate file-not-found. */
	}
	return result;
}

#define TPP_USERSTREAM_FREAD(stream, buf, bufsize) \
	tpp_userstream_fread(stream, buf, bufsize)
#define TPP_USERSTREAM_FREAD_NONBLOCK(stream, buf, bufsize) \
	tpp_userstream_fread_nonblock(stream, buf, bufsize)

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
tpp_userstream_fread(DeeFileObject *__restrict self,
                     void *__restrict buffer, size_t bufsize) {
	size_t result;
	/* Check for interrupts so the user can stop very long compilation processes. */
	if (DeeThread_CheckInterrupt())
		goto err;
	result = DeeFile_Read(Dee_AsObject(self), buffer, bufsize);
	if unlikely(result == (size_t)-1)
		goto err;
	return result;
err:
	/* Set the error flag. */
	TPPLexer_SetErr();
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
tpp_userstream_fread_nonblock(DeeFileObject *__restrict self,
                              void *__restrict buffer, size_t bufsize) {
	size_t result;
	result = DeeFile_Readf(Dee_AsObject(self), buffer, bufsize,
	                       Dee_FILEIO_FNONBLOCKING);
	if unlikely(result == (size_t)-1)
		goto err;
	return result;
err:
	/* Set the error flag. */
	TPPLexer_SetErr();
	return 0;
}

DECL_END

#undef malloc
#undef calloc
#undef realloc
#undef free

/* Redirect heap allocations to use deemon's general-purpose heap. */
#define malloc(s)     Dee_TryMalloc(s)
#define calloc(c, s)  Dee_TryCallocc(c, s)
#define realloc(p, s) Dee_TryRealloc(p, s)
#define free(p)       Dee_Free(p)
#if !defined(alloca) && defined(Dee_Alloca)
#define alloca        Dee_Alloca
#endif /* !alloca && Dee_Alloca */

#define bswap_16      BSWAP16
#define bswap_32      BSWAP32
#define bswap_64      BSWAP64

#define TPP_CONFIG_CALLBACK_ON_DESTROY_FILE(self) \
	DeeCompiler_DelItem(self)

#undef PRIVATE
#define PRIVATE      INTERN


/* Hooks for filesystem properties */
#undef HAVE_INSENSITIVE_PATHS
#undef SEP
#undef ALTSEP
#undef ISABS
#ifdef DeeSystem_HAVE_FS_ICASE
#define HAVE_INSENSITIVE_PATHS
#endif /* DeeSystem_HAVE_FS_ICASE */
#ifndef DeeSystem_ALTSEP
#define SEP DeeSystem_SEP
#elif DeeSystem_SEP == '\\' && DeeSystem_ALTSEP == '/'
#define SEP    '/'
#define ALTSEP '\\'
#else /* ... */
#define SEP    DeeSystem_SEP
#define ALTSEP DeeSystem_ALTSEP
#endif /* !... */
#define ISABS(x) DeeSystem_IsAbs(x)




/* Force tpp to use our own version of [v]sprintf, thus preventing a potential libc
 * dependency for which we'd need to add another system-features test. And given the
 * fact that deemon _needs_ its own format-printer (due to extensions such as %r for
 * printing the representation of some DeeObject), we can simply have this one point
 * back to ourselves. */
#undef sprintf
#undef vsprintf
#define sprintf(buf, ...) \
	(int)(unsigned int)(size_t)(Dee_sprintf(buf, __VA_ARGS__) - (buf))
#define vsprintf(buf, format, args) \
	(int)(unsigned int)(size_t)(Dee_vsprintf(buf, format, args) - (buf))

/* TODO: TPP has a non-redundant dependency on `time_t time(time_t *ptr)' */
/* TODO: TPP has a non-redundant dependency on `struct tm *localtime(time_t const *tmr)' */
/* TODO: TPP has a non-redundant dependency on `void srand(unsigned int seed)' */
/* TODO: TPP has a non-redundant dependency on `int rand(void)' */



/* Configure #pragma message to output text via deemon's stdout file. */
PRIVATE Dee_ssize_t DCALL
tpp_pragma_message_printf(char const *format, ...) {
	Dee_ssize_t result;
	DREF DeeObject *stdout_file;
	va_list args;
	stdout_file = DeeFile_GetStd(Dee_STDOUT);
	if unlikely(!stdout_file)
		goto err;
	va_start(args, format);
	result = DeeFile_VPrintf(stdout_file, format, args);
	va_end(args);
	Dee_Decref(stdout_file);
	return result;
err:
	return -1;
}

#define TPP_PRAGMA_MESSAGE_PRINTF(err_label, printf_args)      \
	do {                                                       \
		if unlikely(tpp_pragma_message_printf printf_args < 0) \
			goto err_label;                                    \
	}	__WHILE0
#define TPP_PRAGMA_MESSAGE_WRITE(err_label, str, length) \
	TPP_PRAGMA_MESSAGE_PRINTF(err_label, ("%$s", (size_t)(length), str))


/* Prevent tpp from unconditionally including these headers. If they do
 * exist, then they've already been included by <deemon/system-features.h> */
#define NO_INCLUDE_ENDIAN_H   1
#define NO_INCLUDE_FCNTL_H    1
#define NO_INCLUDE_UNISTD_H   1
#define NO_INCLUDE_SYS_STAT_H 1
#define NO_INCLUDE_TIME_H     1
#define NO_INCLUDE_ERRNO_H    1
#define NO_INCLUDE_STRING_H   1
#define NO_INCLUDE_STDLIB_H   1
#define NO_INCLUDE_STDIO_H    1
#define NO_INCLUDE_MALLOC_H   1
#define NO_INCLUDE_ALLOCA_H   1

#undef token
#undef tok
#undef yield
#undef yieldnb
#undef yieldnbif
#undef skip

#ifndef __INTELLISENSE__
#include "../../tpp/src/tpp.c"
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

INTERN struct TPPKeyword TPPKeyword_Empty = {
	/* .k_next  = */ NULL,
	/* .k_macro = */ NULL,
	/* .k_rare  = */ NULL,
	/* .k_id    = */ TOK_KEYWORD_BEGIN,
#if __SIZEOF_POINTER__ > __SIZEOF_INT__
	/* .k_pad   = */ { 0 },
#endif /* __SIZEOF_POINTER__ > __SIZEOF_INT__ */
	/* .k_size  = */ 0,
	/* .k_hash  = */ 1,
	/* .k_zero  = */ { 0 }
};


INTERN WUNUSED NONNULL((1)) char *DCALL advance_wraplf(char *__restrict p) {
	++p;
	while (SKIP_WRAPLF(p, token.t_file->f_end))
		;
	return p;
}

INTERN WUNUSED struct TPPKeyword *DCALL tok_without_underscores(void) {
	struct TPPKeyword *result = NULL;
	if (TPP_ISKEYWORD(tok)) {
		result = token.t_kwd;
		if (result->k_name[0] == '_' ||
		    result->k_name[result->k_size - 1] == '_') {
			char const *begin, *end;
			/* Keyword has leading/terminating underscores.
			 * >> Remove them and use that keyword instead! */
			end = (begin = result->k_name) + result->k_size;
			while (begin < end && *begin == '_')
				++begin;
			while (end > begin && end[-1] == '_')
				--end;
			/* NOTE: Don't create the keyword if it doesn't exist!
			 *    >> Callers only use this function to unify attribute names & arguments! */
			result = TPPLexer_LookupKeyword(begin, (size_t)(end - begin), 0);
		}
	}
	return result;
}

INTDEF WUNUSED NONNULL((1, 2)) char *TPPCALL
skip_whitespace_and_comments(char *iter, char *end);

INTERN WUNUSED char *DCALL
peek_next_token(struct TPPFile **tok_file) {
	if (tok_file)
		*tok_file = token.t_file;
	return peek_next_advance(token.t_end, tok_file);
}

INTERN WUNUSED NONNULL((1)) char *DCALL
peek_next_advance(char *p, struct TPPFile **tok_file) {
	char *result, *end, *file_begin;
	result = p;
	struct TPPFile *curfile;
	if (tok_file) {
		curfile = *tok_file;
		ASSERT(curfile != NULL);
	} else {
		curfile = token.t_file;
	}
	ASSERT(p >= curfile->f_begin &&
	       p <= curfile->f_end);
again:
	end = curfile->f_end;

	if (TPPLexer_Current->l_flags & TPPLEXER_FLAG_WANTLF) {
		if (*p == '\r' && p[1] == '\n')
			p += 2;
		while (result < end && tpp_isspace_nolf(*result))
			++result;
		if (*result != '/')
			goto set_result;
		result = skip_whitespace_and_comments(result, end);
	} else {
		result = skip_whitespace_and_comments(result, end);
	}
	if (result == end) {
		int extend_error;
		file_begin = curfile->f_begin;
		/* Special case: Must extend the file. */
		extend_error = TPPFile_NextChunk(curfile, TPPFILE_NEXTCHUNK_FLAG_EXTEND);
		if (curfile == token.t_file) {
			token.t_begin = curfile->f_begin + (token.t_begin - file_begin);
			token.t_end   = curfile->f_begin + (token.t_end - file_begin);
		}
		result = curfile->f_begin + (result - file_begin);
		/* If the file was extended, search for the next token again. */
		if likely(extend_error) {
			if unlikely(extend_error < 0)
				goto err;
			goto again;
		}
		/* Continue searching through the include-stack. */
		if (curfile->f_prev) {
			curfile = curfile->f_prev;
			result  = curfile->f_pos;
			goto again;
		}
	}
set_result:
	if (tok_file)
		*tok_file = curfile;
	return result;
err:
	return NULL;
}


#ifndef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1)) struct TPPKeyword *TPPCALL
lookup_escaped_keyword(char const *__restrict name, size_t namelen,
                       size_t unescaped_size, int create_missing);

INTERN ATTR_CONST WUNUSED bool DCALL tpp_is_keyword_start(char ch) {
	uint8_t chflags = CH_ISALPHA;
	if (HAVE_EXTENSION_EXTENDED_IDENTS)
		chflags |= CH_ISANSI;
	if (!((chrattr[(uint8_t)ch] & chflags) ||
	      (HAVE_EXTENSION_DOLLAR_IS_ALPHA && ch == '$')))
		return false; /* Not-a-keyword. */
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) struct TPPKeyword *DCALL
peek_keyword(struct TPPFile *__restrict tok_file,
             char *__restrict tok_begin, int create_missing) {
	struct TPPKeyword *kwd_entry;
	size_t name_escapesize, name_size;
	uint8_t chflags;
	char *iter;
	name_size = 1;
	chflags   = CH_ISALPHA;
	iter      = tok_begin;
	ASSERT(tok_begin >= tok_file->f_begin);
	ASSERT(tok_begin <= tok_file->f_end);
	while (SKIP_WRAPLF(iter, tok_file->f_end))
		;
	if (iter >= tok_file->f_end)
		return NULL; /* EOF */
	/* Set the ANSI flag if we're supporting those characters. */
	if (HAVE_EXTENSION_EXTENDED_IDENTS)
		chflags |= CH_ISANSI;
	if (!((chrattr[(uint8_t)*iter] & chflags) ||
	      (HAVE_EXTENSION_DOLLAR_IS_ALPHA && *iter == '$')))
		return NULL; /* Not-a-keyword. */
	/* All non-first characters are allowed to be digits as well. */
	chflags |= CH_ISDIGIT;
	++iter;
	/* keyword: scan until a non-alnum character is found. */
	if (HAVE_EXTENSION_DOLLAR_IS_ALPHA) {
		for (;;) {
			while (SKIP_WRAPLF(iter, tok_file->f_end))
				;
			if (!(chrattr[(uint8_t)*iter] & chflags))
				break;
			++iter;
			++name_size;
		}
	} else {
		for (;;) {
			while (SKIP_WRAPLF(iter, tok_file->f_end))
				;
			if (!(chrattr[(uint8_t)*iter] & chflags) || *iter == '$')
				break;
			++iter;
			++name_size;
		}
	}
	/* Lookup/generate the token id of this keyword. */
	name_escapesize = (size_t)(iter - tok_begin);
	if (name_size == name_escapesize) {
		kwd_entry = TPPLexer_LookupKeyword(tok_begin, name_size, create_missing);
	} else {
		kwd_entry = lookup_escaped_keyword(tok_begin, name_escapesize, name_size, create_missing);
	}
	return kwd_entry;
}

INTERN WUNUSED struct TPPKeyword *DCALL
peek_next_keyword(int create_missing) {
	struct TPPFile *tok_file;
	char *tok_begin = peek_next_token(&tok_file);
	if unlikely(!tok_begin)
		return NULL;
	return peek_keyword(tok_file, tok_begin, create_missing);
}

#if 0
INTERN ATTR_PURE WUNUSED NONNULL((1)) hash_t DCALL
hashof_lower(void const *data, size_t size) {
	hash_t result = 1;
	unsigned char const *iter, *end;
	end = (iter = (unsigned char const *)data) + size;
	for (; iter < end; ++iter)
		result = result * 263 + tolower(*iter);
	return result;
}


INTERN WUNUSED NONNULL((1)) struct TPPKeyword *DCALL
lowercase_keyword(char const *__restrict name,
                  size_t namelen, int create_missing) {
	hash_t namehash;
	struct TPPKeyword *kwd_entry, **bucket;
	namehash = hashof_lower(name, namelen);
	/* Try to rehash the keyword map. */
	if (TPPKeywordMap_SHOULDHASH(&CURRENT.l_keywords)) {
		ASSERTF(CURRENT.l_keywords.km_entryc > CURRENT.l_keywords.km_bucketc,
		        ("New size %lu isn't greater than old size %lu",
		         (unsigned long)CURRENT.l_keywords.km_entryc,
		         (unsigned long)CURRENT.l_keywords.km_bucketc));
		rehash_keywords(CURRENT.l_keywords.km_entryc);
	}
	ASSERT(CURRENT.l_keywords.km_bucketc);
	ASSERT(CURRENT.l_keywords.km_bucketv);
	bucket = &CURRENT.l_keywords.km_bucketv[namehash %
	                                        CURRENT.l_keywords.km_bucketc];
	kwd_entry = *bucket;
	while (kwd_entry) {
		if (kwd_entry->k_hash == namehash &&
		    kwd_entry->k_size == namelen &&
		    !memcasecmp(kwd_entry->k_name, name, namelen * sizeof(char)))
			return kwd_entry; /* Found it! */
		kwd_entry = kwd_entry->k_next;
	}
	if unlikely(!create_missing)
		return NULL;
	/* Must allocate a new keyword entry. */
	kwd_entry = (struct TPPKeyword *)malloc(TPP_OFFSETOF(struct TPPKeyword, k_name) +
	                                        (namelen + 1) * sizeof(char));
	if unlikely(!kwd_entry)
		return NULL;
	/* Setup the new keyword entry. */
	kwd_entry->k_rare  = NULL;
	kwd_entry->k_macro = NULL;
	kwd_entry->k_id    = _KWD_BACK + (CURRENT.l_keywords.km_entryc++); /* Unique user-keyword ID. */
	kwd_entry->k_size  = namelen;
	kwd_entry->k_hash  = namehash;
	memcpyc(kwd_entry->k_name, name, namelen, sizeof(char));
	{
		char *iter, *end;
		end = (iter = kwd_entry->k_name) + namelen;
		for (; iter < end; ++iter)
			*iter = tolower(*iter);
	}
	kwd_entry->k_name[namelen] = '\0';
	kwd_entry->k_next          = *bucket;
	return *bucket             = kwd_entry;
}

INTERN bool DCALL
token_replace_lowercase(int create_missing) {
	struct TPPKeyword *lowername;
	if (!TPP_ISKEYWORD(tok))
		return false;
	lowername = lowercase_keyword(token.t_kwd->k_name,
	                              token.t_kwd->k_size,
	                              create_missing);
	if (token.t_kwd == lowername)
		return false;
	token.t_kwd = lowername;
	token.t_id  = lowername->k_id;
	return true;
}
#endif


/* Return the error type used by a given warning number. */
INTERN ATTR_CONST WUNUSED DeeTypeObject *DCALL
get_warning_error_class(int wnum) {
	unsigned int wid = wnum2id(wnum);
	if (wid_isvalid(wid)) {
		wgroup_t const *wgroups;
		wgroup_t group;
		wgroups = w_associated_groups[wid - WG_COUNT];
		for (; (group = *wgroups) != 0; ++wgroups) {
			/* Use sub-classes of `CompilerError' for certain warning groups. */
			if (group == WG_SYNTAX)
				return &DeeError_SyntaxError;
			if (group == WG_SYMBOL)
				return &DeeError_SymbolError;
		}
	}
	return &DeeError_CompilerError;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeCompilerError_Print(DeeObject *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
#if defined(_MSC_VER) && 0
#define LOG_PREFIX "../"
#else /* _MSC_VER */
#define LOG_PREFIX ""
#endif /* !_MSC_VER */
#define PRINT(str) print(str, COMPILER_STRLEN(str))
#define print(p, s)                                     \
	do {                                                \
		if unlikely((temp = (*printer)(arg, p, s)) < 0) \
			goto err;                                   \
		result += temp;                                 \
	}	__WHILE0
#define printf(...)                                                           \
	do {                                                                      \
		if unlikely((temp = DeeFormat_Printf(printer, arg, __VA_ARGS__)) < 0) \
			goto err;                                                         \
		result += temp;                                                       \
	}	__WHILE0
#define printob(ob)                                                 \
	do {                                                            \
		if unlikely((temp = DeeObject_Print(ob, printer, arg)) < 0) \
			goto err;                                               \
		result += temp;                                             \
	}	__WHILE0
	DeeCompilerErrorObject *me;
	Dee_ssize_t temp, result = 0;
	size_t i, count;
	struct Dee_compiler_error_loc *main_loc;
	struct Dee_compiler_error_loc *iter;
	char const *file_and_line;
	me = (DeeCompilerErrorObject *)self;
	count         = me->ce_errorc;
	main_loc      = me->ce_loc;
	file_and_line = TPPLexer_Current->l_flags & TPPLEXER_FLAG_MSVC_MESSAGEFORMAT
	                ? LOG_PREFIX "%s(%d,%d) : "
	                : LOG_PREFIX "%s:%d:%d: ";
	if (main_loc && main_loc->cl_file) { /* Print file+line+col. */
		ASSERT(main_loc->cl_file);
		printf(file_and_line,
		       TPPFile_Filename(main_loc->cl_file, NULL),
		       main_loc->cl_line + 1, main_loc->cl_col + 1);
	}
	/* Print information about the warning number. */
	printf("%c%.4d(",
	       (me->ce_mode == Dee_COMPILER_ERROR_FATALITY_WARNING) ? 'W' : 'E',
	       me->ce_wnum);
	{
		unsigned int wid = wnum2id(me->ce_wnum);
		if (wid_isvalid(wid)) {
			wgroup_t const *wgroups;
			wgroups = w_associated_groups[wid - WG_COUNT];
#if 1
			if (*wgroups >= 0)
				printf("\"-W%s\"", wgroup_names[*wgroups]);
#else
			while (*wgroups >= 0) {
				char const *name = wgroup_names[*wgroups++];
				printf("\"-W%s\"%s", name, (*wgroups >= 0) ? "," : "");
			}
#endif
		}
		PRINT("): ");
	}
	if likely(me->e_msg)
		printob(me->e_msg);
	if (main_loc) {
		for (iter = &me->ce_locs; iter != main_loc;
		     iter = iter->cl_prev) {
			if (!iter->cl_file)
				continue;
			PRINT("\n");
			printf(file_and_line,
			       TPPFile_Filename(iter->cl_file, NULL),
			       iter->cl_line + 1, iter->cl_col + 1);
			printf("In expansion of macro `%$s'",
			       iter->cl_file->f_namesize,
			       iter->cl_file->f_name);
		}
		for (;;) {
			main_loc = main_loc->cl_prev;
			if (!main_loc)
				break;
			if (!main_loc->cl_file)
				continue;
			PRINT("\n");
			printf(file_and_line,
			       TPPFile_Filename(main_loc->cl_file, NULL),
			       main_loc->cl_line + 1, main_loc->cl_col + 1);
			PRINT("In file included from here");
		}
	}
	for (i = 0; i < count; ++i) {
		if (me->ce_errorv[i] == me)
			continue;
		temp = (*printer)(arg, "\n", 1);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		temp = DeeObject_Print(Dee_AsObject(me->ce_errorv[i]),
		                       printer, arg);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
#undef printob
#undef printf
#undef print
#undef PRINT
}
#endif

#ifdef __INTELLISENSE__
#define SEP DeeSystem_SEP
#ifdef DeeSystem_ALTSEP
#define ALTSEP DeeSystem_ALTSEP
#endif /* DeeSystem_ALTSEP */
#define TPPKeyword_API_MAKERARE(self) \
	TPPKeyword_MAKERARE(self)
#endif /* __INTELLISENSE__ */


/* Prefix inserted between a system library path and a include-header. */
PRIVATE char const include_prefix[] = "include/";

INTERN WUNUSED NONNULL((2)) struct TPPFile *DCALL
tpp_unknown_file(int mode, char *__restrict filename,
                 size_t filename_size,
                 struct TPPKeyword **p_keyword_entry) {
	DeeStringObject *buffer, *new_buffer;
	size_t buflen;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DREF DeeTupleObject *libpath;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	DeeListObject *libpath;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	size_t i;
	struct TPPKeyword *keyword_entry;
	DREF DeeObject *path, *stream;
	struct TPPFile *result;

	/* Only search the library path if the
	 * include mode is that of a system header. */
	if (!(mode & TPPLEXER_OPENFILE_MODE_SYSTEM))
		return NULL;

	/* Try to pre-allocate a decently-sized buffer. */
	buflen = 128 + filename_size;
	buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
	                                                 buflen + 1, sizeof(char));
	if unlikely(!buffer) {
		buflen = 0;
	} else {
		buffer->s_data = NULL;
		buffer->s_hash = (Dee_hash_t)-1;
	}
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	libpath = (DREF DeeTupleObject *)DeeModule_GetLibPath();
	if unlikely(!libpath) {
		Dee_XDecref(buffer);
		goto err;
	}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	libpath = DeeModule_GetPath();
	DeeList_LockRead(libpath);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Go through all library paths and generate the filename of a system header. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	for (i = 0; i < DeeTuple_SIZE(libpath); ++i)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	for (i = 0; i < DeeList_SIZE(libpath); ++i)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	{
		size_t req_length;
		char *dst;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		path = DeeTuple_GET(libpath, i);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		path = DeeList_GET(libpath, i);
		if (!DeeString_Check(path))
			continue;     /* Ignore anything that isn't a string. */
		Dee_Incref(path); /* Keep a reference to prevent the path from getting deleted. */
		DeeList_LockEndRead(libpath);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		req_length = (DeeString_SIZE(path) +            /* /foo/bar */
		              1 +                               /* /  (Optional, but always allocated) */
		              COMPILER_STRLEN(include_prefix) + /* include/ */
		              filename_size);                   /* baz.dee */
		if unlikely(req_length > buflen) {
			/* Need a larger buffer. */
			new_buffer = (DeeStringObject *)DeeObject_Reallocc(buffer, offsetof(DeeStringObject, s_str),
			                                                   req_length + 1, sizeof(char));
			if unlikely(!new_buffer)
				goto err_path;
			if (!buffer) {
				new_buffer->s_data = NULL;
				new_buffer->s_hash = (Dee_hash_t)-1;
			}
			buffer = new_buffer;
			buflen = req_length;
		}
		ASSERT(buffer);
		dst = buffer->s_str;

		/* Copy the library path. */
#ifdef ALTSEP
		{
			char const *iter, *end;
			end = (iter = DeeString_STR(path)) + DeeString_SIZE(path);
			for (; iter < end; ++iter, ++dst) {
				char ch = *iter;
				if (ch == ALTSEP)
					ch = SEP;
				*dst = ch;
			}
		}
#else /* ALTSEP */
		dst = (char *)mempcpyc(dst,
		                       DeeString_STR(path),
		                       DeeString_SIZE(path),
		                       sizeof(char));
#endif /* !ALTSEP */

		/* Drop our reference to the library path. */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		Dee_Decref(path);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

		/* Add a separator after the library path (if there wasn't one to being with) */
		if (dst > buffer->s_str && dst[-1] != SEP)
			*dst++ = SEP;

		/* Now copy the include prefix. */
		dst = (char *)mempcpyc(dst, include_prefix,
		                       COMPILER_STRLEN(include_prefix),
		                       sizeof(char));

		/* Finally, copy the filename that's been given to us by TPP. */
		dst = (char *)mempcpyc(dst, filename, filename_size, sizeof(char));

		/* Ensure ZERO-termination. */
		*dst       = 0;
		req_length = (size_t)(dst - buffer->s_str);

		/* Lookup an existing keyword (cache) entry for this filename. */
		keyword_entry = TPPLexer_LookupKeyword(buffer->s_str, req_length, 0);
		if (keyword_entry && keyword_entry->k_rare &&
		    (result = keyword_entry->k_rare->kr_file) != NULL) {
			/* This file has been included before! (it's still cached) */
			if (buffer && buffer->s_data) {
				Dee_string_utf_fini(buffer->s_data, buffer);
				Dee_string_utf_free(buffer->s_data);
			}
			DeeObject_Free(buffer);
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			Dee_Decref_unlikely(libpath);
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			return result;
		}

		/* Initialize the buffer string.
		 * NOTE: The reference to `DeeString_Type' is added if we succeed in opening the file. */
		DeeObject_InitInherited(buffer, &DeeString_Type);
		buffer->s_len = req_length;

		/* Try to truncate the used portion of the buffer. */
		if (buffer->s_len != buflen) {
			new_buffer = (DeeStringObject *)DeeObject_TryReallocc(buffer, offsetof(DeeStringObject, s_str),
			                                                      buffer->s_len + 1, sizeof(char));
			if likely(new_buffer) {
				buffer = new_buffer;
				buflen = buffer->s_len;
			}
		}
		stream = DeeFile_Open(Dee_AsObject(buffer), OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
		if (stream != ITER_DONE) {       /* Error or success. */
			Dee_Incref(&DeeString_Type); /* Finalize initialization of the buffer. */

			/* Check for errors that may have occurred during `DeeFile_Open()' */
			if unlikely(!stream)
				goto err_streamopen_failed;

			/* Use the stream to open a new TPP file descriptor. */
			result = TPPFile_OpenStream((stream_t)stream, DeeString_STR(buffer));
			Dee_Decref(buffer); /* Drop our own reference to the buffer. */
			if unlikely(!result) {
				/* Failed to create the TPP descriptor. */
				Dee_Decref(stream);
				goto err;
			}

			/* Setup the TPP descriptor such that it inherits the stream. */
			result->f_textfile.f_ownedstream = (stream_t)stream;

			/* All right! We've got the new file descriptor.
			 * Now to cache it in a keyword entry. */
			if (!keyword_entry) {
				keyword_entry = TPPLexer_LookupKeyword(buffer->s_str, req_length, 1);
				if unlikely(!keyword_entry)
					goto err_r;
			}

			/* Ensure that rare data has been allocated for the keyword. */
			if unlikely(!TPPKeyword_API_MAKERARE(keyword_entry))
				goto err_r;
			ASSERT(!keyword_entry->k_rare->kr_file);
			keyword_entry->k_rare->kr_file = result; /* Inherit reference. */
#ifdef HAVE_CALLBACK_NEW_TEXTFILE
			/* Invoke the new-textfile callback if it was defined. */
			if (HAVE_CALLBACK_NEW_TEXTFILE &&
			    !CALLBACK_NEW_TEXTFILE(result, is_system_header)) {
				kwd_entry->k_rare->kr_file = NULL;
				goto err_r;
			}
#endif /* HAVE_CALLBACK_NEW_TEXTFILE */
			if (p_keyword_entry)
				*p_keyword_entry = keyword_entry;
			return result;
		}
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		DeeList_LockRead(libpath);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DeeList_LockEndRead(libpath);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	Dee_Decref_unlikely(libpath);
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
done_notfound:
	result = NULL;

/*done:*/
	/* Free the temporary filename-buffer. */
	if (buffer && buffer->s_data) {
		Dee_string_utf_fini(buffer->s_data, buffer);
		Dee_string_utf_free(buffer->s_data);
	}
	DeeObject_Free(buffer);
done_result:
	return NULL;
err_path:
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	Dee_Decref_unlikely(libpath);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	Dee_Decref(path);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	TPPLexer_SetErr();
	goto done_notfound;
err_streamopen_failed:
	Dee_Decref(buffer);
	TPPLexer_SetErr();
err:
	result = NULL;
	goto done_result;
err_r:
	TPPFile_Decref(result);
	goto err;
}




/* Warn about use of `pack' (but only if we're not currently inside of a macro) */
INTERN WUNUSED int DCALL
parser_warn_pack_used(struct ast_loc *loc) {
	struct TPPFile *file = token.t_file;
	if (loc && loc->l_file)
		file = loc->l_file;
	if (file->f_kind != TPPFILE_KIND_TEXT)
		return 0; /* Only warn inside of regular files */
	return parser_warnatf(loc, W_PACK_USED_OUTSIDE_OF_MACRO);

}

PRIVATE WUNUSED int DCALL
parser_skip_maybe_seek(tok_t expected_tok) {
	/* Depending on which token was expected, and what the current token is,
	 * skip ahead a couple of tokens in search of what we're looking for. */
again:
	if (tok == expected_tok) {
		if unlikely(yield() < 0)
			goto err;
		return 1;
	}
	if (tok == 0)
		return 0;
	if (TPP_ISKEYWORD(tok) && !TPP_ISKEYWORD(expected_tok)) {
		/* Skip unexpected keywords */
/*yield_and_again:*/
		if unlikely(yield() < 0)
			goto err;
		goto again;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED int DFCALL
parser_skip(tok_t expected_tok, int wnum, ...) {
	if likely(tok != expected_tok) {
		va_list args;
		int result;
		va_start(args, wnum);
		result = parser_vwarnf(wnum, args);
		va_end(args);
		if (result != 0)
			return result;
		if unlikely(parser_skip_maybe_seek(expected_tok) < 0)
			goto err;
	} else {
		if unlikely(yield() < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DFCALL
_parser_paren_begin(bool *__restrict p_has_paren, int wnum) {
	ASSERT(tok != '(');
	if (tok == KWD_pack) {
		struct ast_loc packloc;
		loc_here(&packloc);
		if unlikely(yield() < 0)
			goto err;
		*p_has_paren = tok == '(';
		if (*p_has_paren) {
			if unlikely(yield() < 0)
				goto err;
		} else {
			/* Warn about use of `pack' (if done so outside
			 * of a macro, and only if not followed by a `(') */
			if unlikely(parser_warn_pack_used(&packloc))
				goto err;
		}
	} else {
		int temp;
		if unlikely(parser_warnf(wnum))
			goto err;
		temp = parser_skip_maybe_seek('(');
		if unlikely(temp < 0)
			goto err;
		*p_has_paren = temp > 0;
	}
	return 0;
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_TPP_C */
