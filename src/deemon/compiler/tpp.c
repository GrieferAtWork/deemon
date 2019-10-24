/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_TPP_C
#define GUARD_DEEMON_COMPILER_TPP_C 1
#define TPP_SYMARRAY_SIZE 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/error.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/string.h>

#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable : 4005)
#endif /* _MSC_VER */

#define PRIVDEF PRIVATE
#undef PUBLIC
#undef TPPFUN
#define PUBLIC  INTERN
#define TPPFUN  INTDEF
#define TPP_USERDEFS  <deemon/compiler/lexer.def>
#define TPP_USERSTREAM_FCLOSE(stream)        Dee_Decref(stream)

DECL_BEGIN
#define TPP_USERSTREAM_FOPEN(filename) \
	tpp_userstream_fopen(filename)
PRIVATE stream_t DCALL
tpp_userstream_fopen(char const *__restrict filename) {
	stream_t result;
	result = (stream_t)DeeFile_OpenString(filename, OPEN_FRDONLY, 0);
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
	dssize_t result;
	/* Check for interrupts so the user can stop very long compilation processes. */
	if (DeeThread_CheckInterrupt())
		result = -1;
	else {
		result = DeeFile_Read((DeeObject *)self, buffer, bufsize);
	}
	if unlikely(result < 0) {
		/* Set the error flag. */
		TPPLexer_SetErr();
		result = 0;
	}
	return (size_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
tpp_userstream_fread_nonblock(DeeFileObject *__restrict self,
                              void *__restrict buffer, size_t bufsize) {
	dssize_t result;
	result = DeeFile_Readf((DeeObject *)self, buffer, bufsize,
	                       Dee_FILEIO_FNONBLOCKING);
	if unlikely(result < 0) {
		/* Set the error flag. */
		TPPLexer_SetErr();
		result = 0;
	}
	return (size_t)result;
}

DECL_END

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef empty_code

/* Redirect heap allocations to use deemon's general-purpose heap. */
#define malloc(s)     Dee_TryMalloc(s)
#define calloc(c, s)  Dee_TryCalloc((c) * (s))
#define realloc(p, s) Dee_TryRealloc(p, s)
#define free(p)       Dee_Free(p)

#define TPP_CONFIG_CALLBACK_ON_DESTROY_FILE(self) \
	DeeCompiler_DelItem(self)

#undef PRIVATE
#define PRIVATE      INTERN

#ifndef __INTELLISENSE__
#undef SKIP_WRAPLF
#undef SKIP_WRAPLF_REV
#include "../../tpp/tpp.c"
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

INTERN struct TPPKeyword TPPKeyword_Empty = {
	/* .k_next  = */ NULL,
	/* .k_macro = */ NULL,
	/* .k_rare  = */ NULL,
	/* .k_id    = */ TOK_KEYWORD_BEGIN,
#if __SIZEOF_POINTER__ > __SIZEOF_INT__
	/* .k_pad   = */ { 0 },
#endif
	/* .k_size  = */ 0,
	/* .k_hash  = */ 1,
	/* .k_zero  = */ {0},
};


INTERN char *DCALL advance_wraplf(char *__restrict p) {
	++p;
	while (SKIP_WRAPLF(p, token.t_file->f_end))
		;
	return p;
}

INTERN struct TPPKeyword *DCALL tok_without_underscores(void) {
	struct TPPKeyword *result = NULL;
	if (TPP_ISKEYWORD(tok)) {
		result = token.t_kwd;
		if (result->k_name[0] == '_' ||
		    result->k_name[result->k_size - 1] == '_') {
			char const *begin, *end;
			/* Keyword has leading/terminating underscores.
			 * >> Remove them and use that keyword instead! */
			end = (begin = result->k_name) + result->k_size;
			while (begin != end && *begin == '_')
				++begin;
			while (end != begin && end[-1] == '_')
				--end;
			/* NOTE: Don't create the keyword if it doesn't exist!
			 *    >> Callers only use this function to unify attribute names & arguments! */
			result = TPPLexer_LookupKeyword(begin, (size_t)(end - begin), 0);
		}
	}
	return result;
}

INTDEF char *TPPCALL skip_whitespace_and_comments(char *iter, char *end);
INTERN char *DCALL peek_next_token(struct TPPFile **tok_file) {
	if (tok_file)
		*tok_file = token.t_file;
	return peek_next_advance(token.t_end, tok_file);
}

INTERN char *DCALL peek_next_advance(char *p, struct TPPFile *__restrict *tok_file) {
	char *result = p, *end, *file_begin;
	struct TPPFile *curfile;
	if (tok_file) {
		curfile = *tok_file;
		ASSERT(curfile);
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
				return NULL;
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
}


#ifndef __INTELLISENSE__
INTDEF struct TPPKeyword *TPPCALL
lookup_escaped_keyword(char const *__restrict name, size_t namelen,
                       size_t unescaped_size, int create_missing);

INTERN struct TPPKeyword *DCALL
peek_keyword(struct TPPFile *__restrict tok_file,
             char *__restrict tok_begin, int create_missing) {
	struct TPPKeyword *kwd_entry;
	size_t name_escapesize, name_size = 1;
	uint8_t chflags = CH_ISALPHA;
	char *iter      = tok_begin, *end;
	ASSERT(tok_file);
	ASSERT(tok_begin);
	ASSERT(tok_begin >= tok_file->f_begin);
	ASSERT(tok_begin <= tok_file->f_end);
	end = tok_file->f_end;
	while (SKIP_WRAPLF(iter, end))
		;
	if (iter == end)
		return NULL; /* EOF */
	/* Set the ANSI flag if we're supporting those characters. */
	if (HAVE_EXTENSION_EXTENDED_IDENTS)
		chflags |= CH_ISANSI;
	if (!(chrattr[(uint8_t)*iter] & chflags) ||
	    (!HAVE_EXTENSION_DOLLAR_IS_ALPHA && *iter == '$'))
		return NULL; /* Not-a-keyword. */
	/* All non-first characters are allowed to be digits as well. */
	chflags |= CH_ISDIGIT;
	++iter;
	/* keyword: scan until a non-alnum character is found. */
	if (HAVE_EXTENSION_DOLLAR_IS_ALPHA) {
		for (;;) {
			while (SKIP_WRAPLF(iter, end))
				;
			if (!(chrattr[(uint8_t)*iter] & chflags))
				break;
			++iter, ++name_size;
		}
	} else {
		for (;;) {
			while (SKIP_WRAPLF(iter, end))
				;
			if (!(chrattr[(uint8_t)*iter] & chflags) || *iter == '$')
				break;
			++iter, ++name_size;
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

INTERN struct TPPKeyword *DCALL
peek_next_keyword(int create_missing) {
	struct TPPFile *tok_file;
	char *tok_begin = peek_next_token(&tok_file);
	if unlikely(!tok_begin)
		return NULL;
	return peek_keyword(tok_file, tok_begin, create_missing);
}

#if 0
INTERN hash_t DCALL
hashof_lower(void const *data, size_t size) {
	hash_t result = 1;
	unsigned char const *iter, *end;
	end = (iter = (unsigned char const *)data) + size;
	for (; iter != end; ++iter)
		result = result * 263 + tolower(*iter);
	return result;
}


INTERN struct TPPKeyword *DCALL
lowercase_keyword(char const *__restrict name,
                  size_t namelen, int create_missing) {
	hash_t namehash;
	struct TPPKeyword *kwd_entry, **bucket;
	ASSERT(TPPLexer_Current);
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
	memcpy(kwd_entry->k_name, name, namelen * sizeof(char));
	{
		char *iter, *end;
		end = (iter = kwd_entry->k_name) + namelen;
		for (; iter != end; ++iter)
			*iter = tolower(*iter);
	}
	kwd_entry->k_name[namelen] = '\0';
	kwd_entry->k_next          = *bucket;
	return *bucket             = kwd_entry;
}

INTERN bool DCALL token_replace_lowercase(int create_missing) {
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
INTERN DeeTypeObject *DCALL
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

INTERN dssize_t DCALL
comerr_print(DeeCompilerErrorObject *__restrict self,
             dformatprinter printer, void *arg) {
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
	} __WHILE0
#define printf(...)                                                           \
	do {                                                                      \
		if unlikely((temp = DeeFormat_Printf(printer, arg, __VA_ARGS__)) < 0) \
			goto err;                                                         \
		result += temp;                                                       \
	} __WHILE0
#define printob(ob)                                                 \
	do {                                                            \
		if unlikely((temp = DeeObject_Print(ob, printer, arg)) < 0) \
			goto err;                                               \
		result += temp;                                             \
	} __WHILE0
	dssize_t temp, result = 0;
	size_t i, count       = self->ce_errorc;
	struct compiler_error_loc *main_loc = self->ce_loc;
	struct compiler_error_loc *iter;
	char const *file_and_line;
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
	       (self->ce_mode == Dee_COMPILER_ERROR_FATALITY_WARNING) ? 'W' : 'E',
	       self->ce_wnum);
	{
		unsigned int wid = wnum2id(self->ce_wnum);
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
	if likely(self->e_message)
		printob((DeeObject *)self->e_message);
	if (main_loc) {
		for (iter                   = &self->ce_locs;
		     iter != main_loc; iter = iter->cl_prev) {
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
		if (self->ce_errorv[i] == self)
			continue;
		temp = (*printer)(arg, "\n", 1);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		temp = DeeObject_Print((DeeObject *)self->ce_errorv[i],
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
#define SEP '/'
#ifdef CONFIG_HOST_WINDOWS
#define ALTSEP '\\'
#endif /* CONFIG_HOST_WINDOWS */
#define TPPKeyword_API_MAKERARE(self) \
	TPPKeyword_MAKERARE(self)
#endif /* __INTELLISENSE__ */


/* Prefix inserted between a system library path and a include-header. */
PRIVATE char const include_prefix[] = "include/";

INTERN struct TPPFile *DCALL
tpp_unknown_file(int mode, char *__restrict filename,
                 size_t filename_size,
                 struct TPPKeyword **pkeyword_entry) {
	DeeStringObject *buffer, *new_buffer;
	size_t buflen;
	DeeObject *libpath;
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
	buffer = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
	                                                (buflen + 1) * sizeof(char));
	if unlikely(!buffer)
		buflen = 0;
	else {
		buffer->s_data = NULL;
		buffer->s_hash = (dhash_t)-1;
	}
	libpath = DeeModule_GetPath();
	DeeList_LockRead(libpath);
	/* Go through all library paths and generate the filename of a system header. */
	for (i = 0; i < DeeList_SIZE(libpath); ++i) {
		size_t req_length;
		char *dst;
		path = DeeList_GET(libpath, i);
		if (!DeeString_Check(path))
			continue;     /* Ignore anything that isn't a string. */
		Dee_Incref(path); /* Keep a reference to prevent the path from getting deleted. */
		DeeList_LockEndRead(libpath);
		req_length = (DeeString_SIZE(path) +            /* /foo/bar */
		              1 +                               /* /  (Optional, but always allocated) */
		              COMPILER_STRLEN(include_prefix) + /* include/ */
		              filename_size);                   /* baz.dee */
		if unlikely(req_length > buflen) {
			/* Need a larger buffer. */
			new_buffer = (DeeStringObject *)DeeObject_Realloc(buffer, offsetof(DeeStringObject, s_str) +
			                                                          (req_length + 1) * sizeof(char));
			if unlikely(!new_buffer)
				goto err_path;
			if (!buffer) {
				new_buffer->s_data = NULL;
				new_buffer->s_hash = (dhash_t)-1;
			}
			buffer = new_buffer;
			buflen = req_length;
		}
		dst = buffer->s_str;
		/* Copy the library path. */
#ifdef ALTSEP
		{
			char *iter, *end;
			end = (iter = DeeString_STR(path)) + DeeString_SIZE(path);
			for (; iter != end; ++iter, ++dst) {
				char ch = *iter;
				if (ch == ALTSEP)
					ch = SEP;
				*dst = ch;
			}
		}
#else /* ALTSEP */
		memcpy(dst, DeeString_STR(path), DeeString_SIZE(path) * sizeof(char));
		dst += DeeString_SIZE(path);
#endif /* !ALTSEP */
		/* Drop our reference to the library path. */
		Dee_Decref(path);
		/* Add a separator after the library path (if there wasn't one to being with) */
		if (dst != buffer->s_str && dst[-1] != SEP)
			*dst++ = SEP;
		/* Now copy the include prefix. */
		memcpy(dst, include_prefix, COMPILER_STRLEN(include_prefix) * sizeof(char));
		dst += COMPILER_STRLEN(include_prefix);
		/* Finally, copy the filename that's been given to us by TPP. */
		memcpy(dst, filename, filename_size * sizeof(char));
		dst += filename_size;
		/* Ensure ZERO-termination. */
		*dst       = 0;
		req_length = (size_t)(dst - buffer->s_str);

		/* Lookup an existing keyword (cache) entry for this filename. */
		keyword_entry = TPPLexer_LookupKeyword(buffer->s_str, req_length, 0);
		if (keyword_entry && keyword_entry->k_rare &&
		    (result = keyword_entry->k_rare->kr_file) != NULL) {
			/* This file has been included before! (it's still cached) */
			if (buffer && buffer->s_data) {
				string_utf_fini(buffer->s_data, buffer);
				Dee_string_utf_free(buffer->s_data);
			}
			DeeObject_Free(buffer);
			return result;
		}

		/* Initialize the buffer string.
		 * NOTE: The reference to `DeeString_Type' is added if we succeed in opening the file. */
		DeeObject_InitNoref(buffer, &DeeString_Type);
		buffer->s_len = req_length;
		/* Try to truncate the used portion of the buffer. */
		if (buffer->s_len != buflen) {
			new_buffer = (DeeStringObject *)DeeObject_TryRealloc(buffer, offsetof(DeeStringObject, s_str) +
			                                                             (buffer->s_len + 1) * sizeof(char));
			if likely(new_buffer) {
				buffer = new_buffer;
				buflen = buffer->s_len;
			}
		}
		stream = DeeFile_Open((DeeObject *)buffer, OPEN_FRDONLY, 0);
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
			if (pkeyword_entry)
				*pkeyword_entry = keyword_entry;
			return result;
		}
		DeeList_LockRead(libpath);
	}
	DeeList_LockEndRead(libpath);
done_notfound:
	result = NULL;
/*done:*/
	/* Free the temporary filename-buffer. */
	if (buffer && buffer->s_data) {
		string_utf_fini(buffer->s_data, buffer);
		Dee_string_utf_free(buffer->s_data);
	}
	DeeObject_Free(buffer);
done_result:
	return NULL;
err_path:
	Dee_Decref(path);
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


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_TPP_C */
