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
#ifndef GUARD_DEEMON_EXECUTE_MODPATH_C
#define GUARD_DEEMON_EXECUTE_MODPATH_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/gc.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/thread.h>

#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>

#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif /* !CONFIG_NO_DEX */

#ifndef CONFIG_NO_DEC
#include <deemon/dec.h>
#endif /* !CONFIG_NO_DEC */

/**/
#include <deemon/exec.h>
/**/

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <deemon/compiler/compiler.h>

#include <deemon/compiler/assembler.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/tpp.h>
#include <deemon/string.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#include <sys/stat.h>

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#endif /* !CONFIG_HOST_WINDOWS */

#if defined(CONFIG_HOST_WINDOWS) && !defined(__CYGWIN__)
/* NOTE: Don't use LoadLibrary() on cygwin. It does some crazy hacking
 *       to get fork() working properly with dynamic linking, so better
 *       not interfere with it by bypassing its mechanisms. */
#define USE_LOADLIBRARY 1
#else /* Windows... */
#include <dlfcn.h>
#endif /* Unix... */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef __USE_KOS
#define strend(x) ((x) + strlen(x))
#endif /* !__USE_KOS */

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif

DECL_BEGIN


INTDEF struct module_symbol empty_module_buckets[];

#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a, b, s) (_memicmp(a, b, s) == 0)
#else
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif

#ifndef __USE_GNU
#define memrchr dee_memrchr
LOCAL void *dee_memrchr(void const *__restrict p, int c, size_t n) {
	uint8_t *iter = (uint8_t *)p + n;
	while (iter != (uint8_t *)p) {
		if (*--iter == c)
			return iter;
	}
	return NULL;
}
#endif /* !__USE_GNU */


#ifdef CONFIG_HOST_WINDOWS
#define CONFIG_NOCASE_FS 1
#define SEP              '\\'
#define SEP_S            "\\"
#define ISSEP(x) ((x) == '\\' || (x) == '/')
#define ISABS(x) ((x)[0] && (x)[1] == ':')
#else /* CONFIG_HOST_WINDOWS */
#define SEP              '/'
#define SEP_S            "/"
#define ISSEP(x) ((x) == '/')
#define ISABS(x) ((x)[0] == '/')
#endif /* !CONFIG_HOST_WINDOWS */

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
print_pwd(struct unicode_printer *__restrict printer) {
#ifdef CONFIG_HOST_WINDOWS
	LPWSTR buffer;
	DWORD new_bufsize, bufsize = 256;
	buffer = unicode_printer_alloc_wchar(printer, bufsize);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	new_bufsize = GetCurrentDirectoryW(bufsize + 1, buffer);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!new_bufsize) {
		nt_ThrowLastError();
		goto err_release;
	}
	if (new_bufsize > bufsize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, new_bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		bufsize = new_bufsize;
		goto again;
	}
	if unlikely(unicode_printer_confirm_wchar(printer, buffer, new_bufsize) < 0)
		goto err;
	if ((!printer->up_length ||
	     UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1) != SEP) &&
	    unicode_printer_putascii(printer, SEP))
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
#else
	char *buffer, *new_buffer;
	size_t bufsize = 256;
	buffer         = unicode_printer_alloc_utf8(printer, bufsize);
	if unlikely(!buffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!getcwd(buffer, bufsize + 1)) {
		DBG_ALIGNMENT_ENABLE();
		/* Increase the buffer and try again. */
		if (errno != ERANGE) {
			DeeError_Throwf(&DeeError_SystemError,
			                "Failed to determine the current working directory");
			goto err_release;
		}
		bufsize *= 2;
		new_buffer = unicode_printer_resize_utf8(printer, buffer, bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	bufsize = strlen(buffer);
	if unlikely(unicode_printer_confirm_utf8(printer, buffer, bufsize) < 0)
		goto err;
	/* Make sure there is a trailing slash */
	if ((!printer->up_length ||
	     UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1) != SEP) &&
	    unicode_printer_putascii(printer, SEP))
		goto err;
	return 0;
err_release:
	unicode_printer_free_utf8(printer, buffer);
err:
	return -1;
#endif
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
make_absolute(DeeObject *__restrict path) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char *iter, *begin, *end, *flush_start, *flush_end, ch;
	begin = DeeString_AsUtf8(path);
	if unlikely(!begin)
		goto err;
	end = begin + WSTR_LENGTH(begin);
	/* Strip leading space. */
	begin = utf8_skipspace(begin, end);
	if (!ISABS(begin)) {
		/* Print the current working directory when the given path isn't absolute. */
		if unlikely(print_pwd(&printer) < 0)
			goto err;
#ifdef CONFIG_HOST_WINDOWS
		/* Handle drive-relative paths. */
		if (ISSEP(begin[0]) && UNICODE_PRINTER_LENGTH(&printer)) {
			size_t index = 0;
			/* This sep must exist because it was printed by `print_pwd()' */
			while ((++index, UNICODE_PRINTER_GETCHAR(&printer, index - 1) != SEP))
				;
			unicode_printer_truncate(&printer, index);
			/* Strip leading slashes. */
			for (;;) {
				begin = utf8_skipspace(begin, end);
				if (begin >= end)
					break;
				if (!ISSEP(*begin))
					break;
				++begin;
			}
		}
#endif /* CONFIG_HOST_WINDOWS */
	}
	iter = flush_start = begin;
	ASSERTF(*end == '\0',
	        "path = %r\n"
	        "end = %p(%q)\n",
	        path, end, end);
next:
	ch = *iter++;
	switch (ch) {

	/* NOTE: The following part has been mirrored in `fs_pathexpand'
	 *       that is apart of the `fs' DEX implementation file: `fs/path.c'
	 *       If a bug is found in this code, it should be fixed here, as
	 *       well as within the DEX source file. */
#if SEP != '/'
	case '/':
#endif
	case SEP:
	case '\0': {
		char const *sep_loc;
		bool did_print_sep;
		sep_loc = flush_end = iter - 1;
		/* Skip multiple slashes and whitespace following a path separator. */
		for (;;) {
			iter = utf8_skipspace(iter, end);
			if (iter >= end)
				break;
			if (!ISSEP(*iter))
				break;
			++iter;
		}
		flush_end = utf8_skipspace_rev(flush_end, flush_start);
		/* Analyze the last path portion for being a special name (`.' or `..') */
		if (flush_end[-1] == '.') {
			if (flush_end[-2] == '.' && flush_end - 2 == flush_start) {
				dssize_t new_end;
				size_t printer_length;
				/* Parent-directory-reference. */
				/* Delete the last directory that was written. */
				if (!printer.up_buffer)
					goto do_flush_after_sep;
				printer_length = printer.up_length;
				if (!printer_length)
					goto do_flush_after_sep;
				if (UNICODE_PRINTER_GETCHAR(&printer, printer_length - 1) == SEP)
					--printer_length;
				new_end = unicode_printer_memrchr(&printer, SEP, 0, printer_length);
				if (new_end < 0)
					goto do_flush_after_sep;
				++new_end;
				/* Truncate the valid length of the printer to after the previous slash. */
				printer.up_length = (size_t)new_end;
				unicode_printer_truncate(&printer, (size_t)new_end);
				goto done_flush;
			} else if (flush_end[-3] == SEP && flush_end - 3 >= flush_start) {
				/* Parent-directory-reference. */
				char *new_end;
				new_end = (char *)memrchr(flush_start, SEP,
				                          (size_t)((flush_end - 3) - flush_start));
				if (!new_end)
					goto done_flush;
				flush_end = new_end + 1; /* Include the previous sep in this flush. */
				if (unicode_printer_print(&printer, flush_start,
				                          (size_t)(flush_end - flush_start)) < 0)
					goto err;
				goto done_flush;
			} else if (flush_end - 1 == flush_start) {
				/* Self-directory-reference. */
done_flush:
				flush_start = iter;
				goto done_flush_nostart;
			} else if (flush_end[-2] == SEP &&
			           flush_end - 2 >= flush_start) {
				/* Self-directory-reference. */
				flush_end -= 2;
			}
		}
do_flush_after_sep:
		/* Check if we need to fix anything */
		if (flush_end == iter - 1
#ifdef CONFIG_HOST_WINDOWS
		    && (*sep_loc == SEP || iter == end + 1)
#endif /* CONFIG_HOST_WINDOWS */
		    ) {
			goto done_flush_nostart;
		}
		/* If we can already include a slash in this part, do so. */
		did_print_sep = false;
		if (sep_loc == flush_end
#ifdef CONFIG_HOST_WINDOWS
		    && (*sep_loc == SEP)
#endif /* !CONFIG_HOST_WINDOWS */
		    ) {
			++flush_end;
			did_print_sep = true;
		}
		/* Flush everything prior to the path. */
		ASSERT(flush_end >= flush_start);
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(flush_end - flush_start)) < 0)
			goto err;
		flush_start = iter;
		if (did_print_sep)
			; /* The slash has already been been printed: `foo/ bar' */
		else if (sep_loc == iter - 1
#ifdef CONFIG_HOST_WINDOWS
		         && (!*sep_loc || *sep_loc == SEP)
#endif /* !CONFIG_HOST_WINDOWS */
		         ) {
			--flush_start; /* The slash will be printed as part of the next flush: `foo /bar' */
		} else {
			/* The slash must be printed explicitly: `foo / bar' */
			if (unicode_printer_putascii(&printer, SEP) < 0)
				goto err;
		}
done_flush_nostart:
		if (iter == end + 1)
			goto done;
		goto next;
	}
	default: goto next;
	}
done:
	--iter;
	/* Print the remainder. */
	if (iter > flush_start) {

		/* Check for special case: The printer was never used.
		 * If this is the case, we can simply re-return the given path. */
		if (!UNICODE_PRINTER_LENGTH(&printer)) {
			unicode_printer_fini(&printer);
			return_reference_(path);
		}
		/* Actually print the remainder. */
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(iter - flush_start)) < 0)
			goto err;
	}
	/* Pack everything together. */
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


/* Begin loading the given module.
 * @return: 0: You're now responsible to load the module.
 * @return: 1: The module has already been loaded.
 * @return: 2: You've already started loading this module. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_BeginLoading(DeeModuleObject *__restrict self) {
	uint16_t flags;
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *caller = DeeThread_Self();
#endif
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
begin_loading:
	flags = ATOMIC_FETCHOR(self->mo_flags, MODULE_FLOADING);
	if (flags & MODULE_FLOADING) {
		/* Module is already being loaded. */
		while ((flags = ATOMIC_READ(self->mo_flags),
		        (flags & (MODULE_FLOADING | MODULE_FDIDLOAD)) ==
		        MODULE_FLOADING)) {
#ifdef CONFIG_NO_THREADS
			return 2;
#else /* CONFIG_NO_THREADS */
			/* Check if the module is being loaded in the current thread. */
			if (self->mo_loader == caller)
				return 2;
#ifdef CONFIG_HOST_WINDOWS
			/* Sleep a bit longer than usually. */
			DBG_ALIGNMENT_DISABLE();
			__NAMESPACE_INT_SYM SleepEx(1000, 0);
			DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_HOST_WINDOWS */
			SCHED_YIELD();
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !CONFIG_NO_THREADS */
		}
		/* If the module has now been marked as having finished loading,
		 * then simply act as though it was us that did it. */
		if (flags & MODULE_FDIDLOAD)
			return 1;
		goto begin_loading;
	}
#ifndef CONFIG_NO_THREADS
	/* Setup the module to indicate that we're the ones loading it. */
	self->mo_loader = caller;
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
DeeModule_FailLoading(DeeModuleObject *__restrict self) {
	ATOMIC_FETCHAND(self->mo_flags, ~(MODULE_FLOADING));
}

PRIVATE NONNULL((1)) void DCALL
DeeModule_DoneLoading(DeeModuleObject *__restrict self) {
	ATOMIC_FETCHOR(self->mo_flags, MODULE_FDIDLOAD);
}

INTERN WUNUSED NONNULL((1)) int DCALL
TPPFile_SetStartingLineAndColumn(struct TPPFile *__restrict self,
                                 int start_line, int start_col) {
	/* Set the starting-line offset. */
	self->f_textfile.f_lineoff = start_line;
	if (start_col > 0) {
		struct TPPString *pad_text;
		/* Insert some padding white-space to make it look like the first line
		 * starts with a whole bunch of whitespace, thereby adjusting the offset
		 * of the starting column number in the first line. */
		pad_text = TPPString_NewSized((size_t)(unsigned int)start_col);
		if unlikely(!pad_text)
			return -1;
		/* Use space characters to pad text. */
		memset(pad_text->s_text, ' ', pad_text->s_size);
		TPPString_Decref(self->f_text);
		self->f_text  = pad_text; /* Inherit reference */
		self->f_begin = pad_text->s_text;
		self->f_end   = pad_text->s_text + pad_text->s_size;
		/* Start scanning _after_ the padding text (don't produce white-space tokens before then!) */
		self->f_pos = self->f_end;
	}
	return 0;
}


INTERN int DCALL
DeeModule_LoadSourceStreamEx(DeeModuleObject *__restrict self,
                             DeeObject *__restrict input_file,
                             int start_line, int start_col,
                             struct compiler_options *options,
                             struct string_object *input_pathname) {
	DREF DeeCompilerObject *compiler;
	struct TPPFile *base_file;
	DREF struct ast *code;
	DREF DeeCodeObject *root_code;
	int result;
	uint16_t assembler_flags;
	uint16_t compiler_flags;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	ASSERT_OBJECT_TYPE(input_file, (DeeTypeObject *)&DeeFile_Type);
	ASSERT_OBJECT_TYPE_EXACT_OPT(input_pathname, &DeeString_Type);
#if 1 /* Always prefer the manual override */
	if (options && options->co_pathname)
		input_pathname = options->co_pathname;
#else
	if (!input_pathname && options)
		input_pathname = options->co_pathname;
#endif
	compiler_flags = COMPILER_FNORMAL;
	if (options)
		compiler_flags = options->co_compiler;
	/* Create a new compiler for the module. */
	compiler = DeeCompiler_New((DeeObject *)self, compiler_flags);
	if unlikely(!compiler)
		goto err;
	/* Start working with this compiler. */
	COMPILER_BEGIN(compiler);
	base_file = TPPFile_OpenStream((stream_t)input_file,
	                               input_pathname
	                               ? DeeString_STR(input_pathname)
	                               : "");
	if unlikely(!base_file)
		goto err_compiler;
	/* Set the starting-line offset. */
	if (TPPFile_SetStartingLineAndColumn(base_file, start_line, start_col)) {
		TPPFile_Decref(base_file);
		goto err_compiler;
	}

	/* Push the initial source file onto the #include-stack,
	 * and TPP inherit our reference to it. */
	TPPLexer_PushFileInherited(base_file);

	/* Override the name that is used as the
	 * effective display/DDI string of the file. */
	if (options && options->co_filename) {
		struct TPPString *used_name;
		ASSERT_OBJECT_TYPE_EXACT(options->co_filename, &DeeString_Type);
		used_name = TPPString_New(DeeString_STR(options->co_filename),
		                          DeeString_SIZE(options->co_filename));
		if unlikely(!used_name)
			goto err_compiler;
		ASSERT(!base_file->f_textfile.f_usedname);
		base_file->f_textfile.f_usedname = used_name; /* Inherit */
	}
	ASSERT(!current_basescope->bs_name);
	/* Set the name of the current base-scope, which
	 * describes the function of the module's root code. */
	if (options && options->co_rootname) {
		ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
		current_basescope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(options->co_rootname),
		                                                    DeeString_SIZE(options->co_rootname), 1);
		if unlikely(!current_basescope->bs_name)
			goto err_compiler;
	}

	assembler_flags        = 0;
	inner_compiler_options = NULL;
	if (options) {
		/* Load custom parser/optimizer flags. */
		assembler_flags        = options->co_assembler;
		compiler->cp_options   = options;
		inner_compiler_options = options->co_inner;
		parser_flags           = options->co_parser;
		optimizer_flags        = options->co_optimizer;
		optimizer_unwind_limit = options->co_unwind_limit;
		if (options->co_tabwidth)
			TPPLexer_Current->l_tabsize = (size_t)options->co_tabwidth;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;

		if (options->co_setup) {
			/* Run a custom setup protocol. */
			result = (*options->co_setup)(options->co_setup_arg);
			if unlikely(result < 0) {
				DeeCompiler_End();
				Dee_Decref(compiler);
				recursive_rwlock_endwrite(&DeeCompiler_Lock);
				return result;
			}
		}
	}

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err_compiler;
		current_basescope->bs_argv = (struct symbol **)Dee_Malloc(1 * sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_compiler;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
		dots->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
		dots->s_type  = SYMBOL_TYPE_ARG;
		dots->s_symid = 0;
		dots->s_flag |= SYMBOL_FALLOC;
		current_basescope->bs_argc    = 1;
		current_basescope->bs_argv[0] = dots;
		current_basescope->bs_varargs = dots;
		current_basescope->bs_flags |= CODE_FVARARGS;
	}

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if unlikely(yield() < 0)
		code = NULL;
	else {
		/* Parse statements until the end of the source stream. */
		code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, TOK_EOF);
	}

	if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
		TPPLexer_ClearIfdefStack();

	/* Rethrow all errors that may have occurred during parsing. */
	if (parser_rethrow(code == NULL)) {
		ast_xdecref(code);
		code = NULL;
	}

	if unlikely(!code)
		goto err_compiler;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		result = ast_optimize_all(code, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(result != 0))
			result = -1;
		if (result)
			goto err_compiler_ast;
	}

	{
		uint16_t refc;
		struct asm_symbol_ref *refv;
		root_code = code_compile(code, assembler_flags, true, &refc, &refv);
		ASSERT(!root_code || !refc);
		ASSERT(!root_code || !refv);
	}
	ast_decref(code);

	/* Rethrow all errors that may have occurred during text assembly. */
	if (parser_rethrow(root_code == NULL))
		Dee_XClear(root_code);

	/* Check for errors during assembly. */
	if unlikely(!root_code)
		goto err_compiler;

	/* Finally, put together the module itself. */
	result = module_compile(self, root_code, assembler_flags);
	Dee_Decref(root_code);

	/* Rethrow all errors that may have occurred during module linkage. */
	if (parser_rethrow(result != 0))
		result = -1;

	DeeCompiler_End();
	Dee_Decref(compiler);
	recursive_rwlock_endwrite(&DeeCompiler_Lock);
	return result;
err_compiler_ast:
	ast_decref(code);
err_compiler:
	DeeCompiler_End();
	Dee_Decref(compiler);
	recursive_rwlock_endwrite(&DeeCompiler_Lock);
err:
	return -1;
}

PUBLIC int DCALL
DeeModule_LoadSourceStream(DeeObject *__restrict self,
                           DeeObject *__restrict input_file,
                           int start_line, int start_col,
                           struct compiler_options *options) {
	int result = DeeModule_BeginLoading((DeeModuleObject *)self);
	if (result == 0) {
		result = DeeModule_LoadSourceStreamEx((DeeModuleObject *)self,
		                                      input_file,
		                                      start_line,
		                                      start_col,
		                                      options,
		                                      NULL);
		if unlikely(result)
			DeeModule_FailLoading((DeeModuleObject *)self);
		else DeeModule_DoneLoading((DeeModuleObject *)self);
	}
	return result;
}








struct module_bucket {
	DeeModuleObject *mb_list; /* [0..1][weak] Chain of modules in this bucket. */
};

/* Filesystem-based module hash table. */
PRIVATE size_t                modules_c = 0;    /* [lock(modules_lock)] Amount of modules in-cache. */
PRIVATE size_t                modules_a = 0;    /* [lock(modules_lock)] Allocated hash-map size. */
PRIVATE struct module_bucket *modules_v = NULL; /* [lock(modules_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(modules_lock);
#endif /* !CONFIG_NO_THREADS */

/* Name-based, global module hash table. */
PRIVATE size_t                modules_glob_c = 0;    /* [lock(modules_glob_lock)] Amount of modules in-cache. */
PRIVATE size_t                modules_glob_a = 0;    /* [lock(modules_glob_lock)] Allocated hash-map size. */
PRIVATE struct module_bucket *modules_glob_v = NULL; /* [lock(modules_glob_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(modules_glob_lock);
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_file_module(DeeStringObject *__restrict module_file, dhash_t hash) {
	DeeModuleObject *result = NULL;
#ifndef CONFIG_NO_THREADS
	ASSERT(rwlock_reading(&modules_lock));
#endif /* !CONFIG_NO_THREADS */
	if (modules_a) {
		result = modules_v[hash % modules_a].mb_list;
		while (result) {
			ASSERTF(result->mo_path, "All modules found in the file cache must have a path assigned");
			ASSERT_OBJECT_TYPE_EXACT(result->mo_path, &DeeString_Type);
			if (/**/
#ifdef CONFIG_HOST_WINDOWS
			    result->mo_pathhash == hash &&
#elif !defined(CONFIG_NOCASE_FS)
			    DeeString_HASH(result->mo_path) == hash &&
#endif
			    DeeString_SIZE(result->mo_path) == DeeString_SIZE(module_file) &&
#ifdef CONFIG_NOCASE_FS
			    MEMCASEEQ(DeeString_STR(result->mo_path), DeeString_STR(module_file),
			              DeeString_SIZE(module_file) * sizeof(char))
#else /* CONFIG_NOCASE_FS */
			    memcmp(DeeString_STR(result->mo_path), DeeString_STR(module_file),
			           DeeString_SIZE(module_file) * sizeof(char)) == 0
#endif /* !CONFIG_NOCASE_FS */
			    ) {
				break; /* Found it! */
			}
			result = result->mo_next;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_glob_module(DeeStringObject *__restrict module_name) {
#ifdef CONFIG_NOCASE_FS
	dhash_t hash = DeeString_HashCase((DeeObject *)module_name);
#else /* CONFIG_NOCASE_FS */
	dhash_t hash = DeeString_Hash((DeeObject *)module_name);
#endif /* !CONFIG_NOCASE_FS */
	DeeModuleObject *result = NULL;
#ifndef CONFIG_NO_THREADS
	ASSERT(rwlock_reading(&modules_glob_lock));
#endif /* !CONFIG_NO_THREADS */
	if (modules_glob_a) {
		result = modules_glob_v[hash % modules_glob_a].mb_list;
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (/**/
#ifndef CONFIG_NOCASE_FS
			    DeeString_HASH(result->mo_name) == hash &&
#endif /* !CONFIG_NOCASE_FS */
			    /* TODO: This comparison doesn't work for mixed LATIN-1/UTF-8 strings */
			    DeeString_SIZE(result->mo_name) == DeeString_SIZE(module_name) &&
#ifdef CONFIG_NOCASE_FS
			    MEMCASEEQ(DeeString_STR(result->mo_name), DeeString_STR(module_name),
			              DeeString_SIZE(module_name) * sizeof(char))
#else /* CONFIG_NOCASE_FS */
			    memcmp(DeeString_STR(result->mo_name), DeeString_STR(module_name),
			           DeeString_SIZE(module_name) * sizeof(char)) == 0
#endif /* !CONFIG_NOCASE_FS */
			    ) {
				break; /* Found it! */
			}
			result = result->mo_globnext;
		}
	}
	return result;
}

PRIVATE bool DCALL rehash_file_modules(void) {
	struct module_bucket *new_vector, *biter, *bend, *dst;
	DeeModuleObject *iter, *next;
	size_t new_size = modules_a * 2;
#ifndef CONFIG_NO_THREADS
	ASSERT(rwlock_writing(&modules_lock));
#endif /* !CONFIG_NO_THREADS */
	if unlikely(!new_size)
		new_size = 4;
do_alloc_new_vector:
	new_vector = (struct module_bucket *)Dee_TryCalloc(new_size * sizeof(struct module_bucket));
	if unlikely(!new_vector) {
		if (modules_a != 0)
			return true; /* Don't actually need to rehash. */
		if (new_size != 1) {
			new_size = 1;
			goto do_alloc_new_vector;
		}
		return false;
	}
	ASSERT(new_size);
	bend = (biter = modules_v) + modules_a;
	for (; biter != bend; ++biter) {
		iter = biter->mb_list;
		while (iter) {
			next = iter->mo_next;
			ASSERTF(iter->mo_path, "All modules found in the file cache must have a path assigned");
			ASSERT_OBJECT_TYPE_EXACT(iter->mo_path, &DeeString_Type);
			/* Re-hash this entry. */
#ifdef CONFIG_HOST_WINDOWS
			dst = &new_vector[iter->mo_pathhash % new_size];
#elif defined(CONFIG_NOCASE_FS)
			dst = &new_vector[DeeString_HashCase((DeeObject *)iter->mo_path) % new_size];
#else
			dst = &new_vector[DeeString_HASH((DeeObject *)iter->mo_path) % new_size];
#endif
			if ((iter->mo_next = dst->mb_list) != NULL)
				iter->mo_next->mo_pself = &iter->mo_next;
			iter->mo_pself = &dst->mb_list;
			dst->mb_list   = iter;
			/* Continue with the next. */
			iter = next;
		}
	}
	Dee_Free(modules_v);
	modules_v = new_vector;
	modules_a = new_size;
	return true;
}

PRIVATE bool DCALL rehash_glob_modules(void) {
	struct module_bucket *new_vector, *biter, *bend, *dst;
	DeeModuleObject *iter, *next;
	size_t new_size = modules_glob_a * 2;
#ifndef CONFIG_NO_THREADS
	ASSERT(rwlock_writing(&modules_glob_lock));
#endif /* !CONFIG_NO_THREADS */
	if unlikely(!new_size)
		new_size = 4;
do_alloc_new_vector:
	new_vector = (struct module_bucket *)Dee_TryCalloc(new_size * sizeof(struct module_bucket));
	if unlikely(!new_vector) {
		if (modules_glob_a != 0)
			return true; /* Don't actually need to rehash. */
		if (new_size != 1) {
			new_size = 1;
			goto do_alloc_new_vector;
		}
		return false;
	}
	ASSERT(new_size);
	bend = (biter = modules_glob_v) + modules_glob_a;
	for (; biter != bend; ++biter) {
		iter = biter->mb_list;
		while (iter) {
			next = iter->mo_globnext;
			ASSERT_OBJECT_TYPE_EXACT(iter->mo_name, &DeeString_Type);
			/* Re-hash this entry. */
#ifdef CONFIG_NOCASE_FS
			dst = &new_vector[DeeString_HashCase((DeeObject *)iter->mo_name) % new_size];
#else /* CONFIG_NOCASE_FS */
			dst = &new_vector[DeeString_HASH((DeeObject *)iter->mo_name) % new_size];
#endif /* !CONFIG_NOCASE_FS */
			if ((iter->mo_globnext = dst->mb_list) != NULL)
				iter->mo_globnext->mo_globpself = &iter->mo_globnext;
			iter->mo_globpself = &dst->mb_list;
			dst->mb_list       = iter;
			/* Continue with the next. */
			iter = next;
		}
	}
	Dee_Free(modules_glob_v);
	modules_glob_v = new_vector;
	modules_glob_a = new_size;
	return true;
}


PRIVATE NONNULL((1)) bool DCALL
add_file_module(DeeModuleObject *__restrict self) {
	dhash_t hash;
	struct module_bucket *bucket;
	ASSERT(!self->mo_pself);
	ASSERT_OBJECT_TYPE_EXACT(self->mo_path, &DeeString_Type);
#ifndef CONFIG_NO_THREADS
	ASSERT(rwlock_writing(&modules_lock));
#endif /* !CONFIG_NO_THREADS */
	if (modules_c >= modules_a &&
	    !rehash_file_modules())
		return false;
	ASSERT(modules_a);
	/* Insert the module into the table. */
#ifdef CONFIG_HOST_WINDOWS
	hash = self->mo_pathhash;
#elif defined(CONFIG_NOCASE_FS)
	hash = DeeString_HashCase((DeeObject *)self->mo_path);
#else
	hash = DeeString_Hash((DeeObject *)self->mo_path);
#endif
	bucket = &modules_v[hash % modules_a];
	if ((self->mo_next = bucket->mb_list) != NULL)
		self->mo_next->mo_pself = &self->mo_next;
	self->mo_pself  = &bucket->mb_list;
	bucket->mb_list = self;
	++modules_c;
	return true;
}

PRIVATE NONNULL((1)) bool DCALL
add_glob_module(DeeModuleObject *__restrict self) {
	dhash_t hash;
	struct module_bucket *bucket;
	DEE_DPRINTF("[RT] Cached global module %r loaded from %r\n",
	            self->mo_name, self->mo_path ? self->mo_path : (DeeStringObject *)Dee_EmptyString);
	ASSERT(!self->mo_globpself);
	ASSERT(self->mo_name);
#ifndef CONFIG_NO_THREADS
	ASSERT(rwlock_writing(&modules_glob_lock));
#endif /* !CONFIG_NO_THREADS */
	if (modules_glob_c >= modules_glob_a &&
	    !rehash_glob_modules())
		return false;
	ASSERT(modules_glob_a);
	/* Insert the module into the table. */
#ifdef CONFIG_NOCASE_FS
	hash = DeeString_HashCase((DeeObject *)self->mo_name);
#else /* CONFIG_NOCASE_FS */
	hash = DeeString_Hash((DeeObject *)self->mo_name);
#endif /* !CONFIG_NOCASE_FS */
	bucket = &modules_glob_v[hash % modules_glob_a];
	if ((self->mo_globnext = bucket->mb_list) != NULL)
		self->mo_globnext->mo_globpself = &self->mo_globnext;
	self->mo_globpself = &bucket->mb_list;
	bucket->mb_list    = self;
	++modules_glob_c;
	return true;
}



INTERN NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self) {
	if (self->mo_pself) {
		rwlock_write(&modules_lock);
		if ((*self->mo_pself = self->mo_next) != NULL)
			self->mo_next->mo_pself = self->mo_pself;
		if (!--modules_c) {
			Dee_Free(modules_v);
			modules_v = NULL;
			modules_a = 0;
		}
		rwlock_endwrite(&modules_lock);
	}
	if (self->mo_globpself) {
		rwlock_write(&modules_glob_lock);
		if ((*self->mo_globpself = self->mo_globnext) != NULL)
			self->mo_globnext->mo_globpself = self->mo_globpself;
		if (!--modules_glob_c) {
			Dee_Free(modules_glob_v);
			modules_glob_v = NULL;
			modules_glob_a = 0;
		}
		rwlock_endwrite(&modules_glob_lock);
	}
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenSourceFile(DeeObject *__restrict source_pathname,
                         DeeObject *module_global_name,
                         struct compiler_options *options,
                         bool throw_error) {
	DREF DeeModuleObject *existing_module;
	DREF DeeModuleObject *result;
	DREF DeeStringObject *module_name_ob;
	DREF DeeStringObject *module_path_ob;
	DREF DeeObject *input_stream;
	dhash_t hash;
	ASSERT_OBJECT_TYPE(source_pathname, &DeeString_Type);
	ASSERT_OBJECT_TYPE_OPT(module_global_name, &DeeString_Type);
	module_path_ob = (DREF DeeStringObject *)make_absolute(source_pathname);
	if unlikely(!module_path_ob)
		goto err;

	/* Quick check if this module had already been opened. */
	rwlock_read(&modules_lock);
	hash   = DeeString_HashCase((DeeObject *)module_path_ob);
	result = find_file_module(module_path_ob, hash);
	if (result) {
		Dee_Incref(result);
		rwlock_endread(&modules_lock);
got_result_modulepath:
		Dee_Decref(module_path_ob);
		goto got_result;
	}
	rwlock_endread(&modules_lock);

	/* Also search for an existing instance
	 * of the specified global module name. */
#if 1 /* This is optional */
	if (ITER_ISOK(module_global_name)) {
		rwlock_read(&modules_glob_lock);
		result = find_glob_module((DeeStringObject *)module_global_name);
		if (result) {
			Dee_Incref(result);
			rwlock_endread(&modules_glob_lock);
			goto got_result_modulepath;
		}
		rwlock_endread(&modules_glob_lock);
	}
#endif

	/* Open the module's source file stream. */
	input_stream = DeeFile_Open((DeeObject *)module_path_ob, OPEN_FRDONLY, 0);
	if unlikely(!ITER_ISOK(input_stream)) {
		result = (DREF DeeModuleObject *)input_stream;
		if (input_stream == ITER_DONE && throw_error)
			err_file_not_found_ob((DeeObject *)module_path_ob),
			result = NULL;
		goto got_result_modulepath;
	}

	/* Create a new module. */
	if (ITER_ISOK(module_global_name)) {
		module_name_ob = (DREF DeeStringObject *)module_global_name;
	} else {
		char *name_end, *name_start;
		char *name;
		size_t size;
		name = DeeString_AsUtf8(source_pathname);
		if unlikely(!name)
			goto err_modulepath_inputstream;
		size     = WSTR_LENGTH(name);
		name_end = name + size;
#ifdef CONFIG_HOST_WINDOWS
		name_start = name_end;
		while (name_start != name && !ISSEP(name_start[-1]))
			--name_start;
#else /* CONFIG_HOST_WINDOWS */
		name_start = (char *)memrchr(name, SEP, size);
		if (!name_start)
			name_start = name - 1;
		++name_start;
#endif /* !CONFIG_HOST_WINDOWS */
		/* Get rid of a file extension in the module name. */
		while (name_end != name_start && name_end[-1] != '.')
			--name_end;
		while (name_end != name_start && name_end[-1] == '.')
			--name_end;
		if (name_end == name_start)
			name_end = name + size;
		module_name_ob = (DREF DeeStringObject *)DeeString_NewUtf8(name_start,
		                                                           (size_t)(name_end - name_start),
		                                                           STRING_ERROR_FIGNORE);
		if unlikely(!module_name_ob)
			goto err_modulepath_inputstream;
	}

	/* Create the new module. */
	result = (DREF DeeModuleObject *)DeeModule_New((DeeObject *)module_name_ob);
	Dee_Decref_unlikely(module_name_ob);
	if unlikely(!result)
		goto err_modulepath_inputstream;

	/* Register the module in the filesystem & global cache. */
	result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef CONFIG_HOST_WINDOWS
	result->mo_pathhash = hash;
#endif /* CONFIG_HOST_WINDOWS */
	result->mo_flags |= MODULE_FLOADING;
	COMPILER_WRITE_BARRIER();
	/* Cache the new module as part of the filesystem
	 * module cache, as well as the global module cache. */
	if (module_global_name) {
set_file_module_global:
#ifndef CONFIG_NO_THREADS
		rwlock_write(&modules_lock);
		if (!rwlock_trywrite(&modules_glob_lock)) {
			rwlock_endwrite(&modules_lock);
			rwlock_write(&modules_glob_lock);
			if (!rwlock_trywrite(&modules_lock)) {
				rwlock_endwrite(&modules_glob_lock);
				goto set_file_module_global;
			}
		}
#endif /* !CONFIG_NO_THREADS */
		existing_module = find_file_module(module_path_ob, hash);
		if likely(!existing_module)
			existing_module = find_glob_module(module_name_ob);
		if unlikely(existing_module) {
			Dee_Incref(existing_module);
			rwlock_endwrite(&modules_glob_lock);
			rwlock_endwrite(&modules_lock);
			Dee_DecrefDokill(result);
			Dee_Decref_likely(input_stream);
			result = existing_module;
			goto try_load_module_after_failure;
		}
		/* Add the module to the file-cache. */
		if ((modules_c >= modules_a && !rehash_file_modules()) ||
		    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
			rwlock_endwrite(&modules_lock);
			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_file_module_global;
			Dee_Decref_likely(input_stream);
			goto err_inputstream_r;
		}
		add_glob_module(result);
		add_file_module(result);
		rwlock_endwrite(&modules_glob_lock);
		rwlock_endwrite(&modules_lock);
	} else {
set_file_module:
		rwlock_write(&modules_lock);
		existing_module = find_file_module(module_path_ob, hash);
		if unlikely(existing_module) {
			Dee_Incref(existing_module);
			rwlock_endwrite(&modules_lock);
			Dee_DecrefDokill(result);
			Dee_Decref_likely(input_stream);
			result = existing_module;
try_load_module_after_failure:
			if (DeeModule_BeginLoading(result) == 0)
				goto load_module_after_failure;
			goto got_result;
		}
		/* Add the module to the file-cache. */
		if unlikely(!add_file_module(result)) {
			rwlock_endwrite(&modules_lock);
			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_file_module;
			Dee_Decref_likely(input_stream);
			goto err_inputstream_r;
		}
		rwlock_endwrite(&modules_lock);
	}
load_module_after_failure:
	/* Actually load the module from its source stream. */
	{
		int error;
		error = DeeModule_LoadSourceStreamEx(result,
		                                     input_stream,
		                                     0,
		                                     0,
		                                     options,
		                                     module_path_ob);
		Dee_Decref(input_stream);
		if unlikely(error) {
			DeeModule_FailLoading(result);
			goto err_inputstream_r;
		}
		DeeModule_DoneLoading(result);
	}
got_result:
	return (DREF DeeObject *)result;
err_inputstream_r:
	Dee_Decref(result);
	Dee_Decref(input_stream);
	goto err;
err_modulepath_inputstream:
	Dee_Decref(input_stream);
/*err_modulepath:*/
	Dee_Decref(module_path_ob);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenSourceFileString(/*utf-8*/ char const *__restrict source_pathname,
                               size_t source_pathsize,
                               /*utf-8*/ char const *module_name,
                               size_t module_namesize,
                               struct compiler_options *options,
                               bool throw_error) {
	DREF DeeObject *result;
	DREF DeeObject *module_name_ob = NULL;
	DREF DeeObject *source_pathname_ob;
	source_pathname_ob = DeeString_NewUtf8(source_pathname,
	                                       source_pathsize,
	                                       STRING_ERROR_FSTRICT);
	if unlikely(!source_pathname_ob)
		goto err;
	if (module_namesize) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeModule_OpenSourceFile(source_pathname_ob,
	                                  module_name_ob,
	                                  options,
	                                  throw_error);
	Dee_XDecref(module_name_ob);
	Dee_Decref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_Decref(source_pathname_ob);
err:
	return NULL;
}


/* Very similar to `DeeModule_OpenSourceMemory()', and used to implement it,
 * however source data is made available using a stream object derived
 * from `File from deemon' */
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeModule_OpenSourceStream(DeeObject *source_stream,
                           int start_line, int start_col,
                           struct compiler_options *options,
                           DeeObject *source_pathname,
                           DeeObject *module_name) {
	DREF DeeModuleObject *result;
	int load_error;
	/* Create a new module. */
	if (!module_name) {
		if (source_pathname) {
			char *name  = DeeString_STR(source_pathname);
			size_t size = DeeString_SIZE(source_pathname);
			char *name_end, *name_start;
			DREF DeeObject *name_object;
			name_end = name + size;
#ifdef CONFIG_HOST_WINDOWS
			name_start = name_end;
			while (name_start != name && !ISSEP(name_start[-1]))
				--name_start;
#else /* CONFIG_HOST_WINDOWS */
			name_start = (char *)memrchr(name, SEP, size);
			if (!name_start)
				name_start = name - 1;
			++name_start;
#endif /* !CONFIG_HOST_WINDOWS */

			/* Get rid of a file extension in the module name. */
			while (name_end != name_start && name_end[-1] != '.')
				--name_end;
			while (name_end != name_start && name_end[-1] == '.')
				--name_end;
			if (name_end == name_start)
				name_end = name + size;
			name_object = DeeString_NewSized(name_start,
			                                 (size_t)(name_end - name_start));
			if unlikely(!name_object)
				goto err;
			result = (DREF DeeModuleObject *)DeeModule_New(name_object);
			Dee_Decref(name_object);
		} else {
			result = (DREF DeeModuleObject *)DeeModule_New(Dee_EmptyString);
		}
	} else {
		DeeModuleObject *existing_module;
		/* Check if the module is already loaded in the global cache. */
		rwlock_read(&modules_glob_lock);
		result = find_glob_module((DeeStringObject *)module_name);
		if (result) {
			Dee_Incref(result);
			rwlock_endread(&modules_glob_lock);
			goto found_existing_module;
		}
		rwlock_endread(&modules_glob_lock);
		/* Create a new module. */
		result = (DREF DeeModuleObject *)DeeModule_New(module_name);
		/* Add the module to the global module cache. */
set_global_module:
		rwlock_write(&modules_glob_lock);
		existing_module = find_glob_module((DeeStringObject *)module_name);
		if unlikely(existing_module) {
			/* The module got created in the mean time. */
			Dee_Incref(existing_module);
			rwlock_endwrite(&modules_glob_lock);
			Dee_Decref(result);
			result = existing_module;
			goto found_existing_module;
		}
		/* Add the module to the global cache. */
		if unlikely(!add_glob_module(result)) {
			rwlock_endwrite(&modules_glob_lock);
			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_global_module;
			goto err_r;
		}
		rwlock_endwrite(&modules_glob_lock);
	}
found_existing_module:
	load_error = DeeModule_BeginLoading(result);
	if (load_error != 0)
		goto done;
	/* If the input stream hasn't been opened yet, open it now. */
	load_error = DeeModule_LoadSourceStreamEx(result,
	                                          source_stream,
	                                          start_line,
	                                          start_col,
	                                          options,
	                                          (DeeStringObject *)source_pathname);
	/* Depending on a load error having occurred, either signify
	 * that the module has been loaded, or failed to be loaded. */
	if unlikely(load_error) {
		DeeModule_FailLoading(result);
		Dee_Clear(result);
	} else {
		DeeModule_DoneLoading(result);
	}
done:
	return (DREF DeeObject *)result;
err_r:
	Dee_Decref(result);
err:
	result = NULL;
	goto done;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeModule_OpenSourceStreamString(DeeObject *source_stream,
                                 int start_line, int start_col,
                                 struct compiler_options *options,
                                 /*utf-8*/ char const *source_pathname,
                                 size_t source_pathsize,
                                 /*utf-8*/ char const *module_name,
                                 size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *module_name_ob     = NULL;
	DREF DeeObject *source_pathname_ob = NULL;
	if (source_pathname) {
		source_pathname_ob = DeeString_NewUtf8(source_pathname,
		                                       source_pathsize,
		                                       STRING_ERROR_FSTRICT);
		if unlikely(!source_pathname_ob)
			goto err;
	}
	if (module_name) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeModule_OpenSourceStream(source_stream,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname_ob,
	                                    module_name_ob);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}


/* Construct a module from a memory source-code blob.
 * NOTE: Unlike `DeeModule_OpenSourceFile()', this function will not bind `source_pathname'
 *       to the returned module, meaning that the module object returned will be entirely
 *       anonymous, except for when `module_name' was passed as non-NULL, in which case
 *       the returned module will be made available as a global import with that same name,
 *       and be available for later addressing using `DeeModule_OpenGlobal()'
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 *                          When NULL, an empty string is used internally, which results in the current
 *                          directory being used as base for relative imports.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Also: register the module as a global module.
 * @param: data:            A pointer to the raw source-code that should be parsed as
 *                          the deemon source for the module.
 * @param: data_size:       The size of the `data' blob (in characters)
 * @param: start_line:      The starting line number of the data blob (zero-based)
 * @param: start_col:       The starting column offset of the data blob (zero-based)
 * @param: options:         An optional set of extended compiler options. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenSourceMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                           int start_line, int start_col,
                           struct compiler_options *options,
                           DeeObject *source_pathname,
                           DeeObject *module_name) {
	DREF DeeObject *source_stream, *result;
	source_stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!source_stream)
		return NULL;
	result = DeeModule_OpenSourceStream(source_stream,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname,
	                                    module_name);
	DeeFile_ReleaseMemory(source_stream);
	return result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenSourceMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                 int start_line, int start_col,
                                 struct compiler_options *options,
                                 /*utf-8*/ char const *source_pathname,
                                 size_t source_pathsize,
                                 /*utf-8*/ char const *module_name,
                                 size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *module_name_ob     = NULL;
	DREF DeeObject *source_pathname_ob = NULL;
	if (source_pathsize) {
		source_pathname_ob = DeeString_NewUtf8(source_pathname,
		                                       source_pathsize,
		                                       STRING_ERROR_FSTRICT);
		if unlikely(!source_pathname_ob)
			goto err;
	}
	if (module_namesize) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeModule_OpenSourceMemory(data,
	                                    data_size,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname_ob,
	                                    module_name_ob);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}



PUBLIC WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_NewString(/*utf-8*/ char const *__restrict name, size_t namelen) {
	DREF DeeObject *name_object, *result;
	name_object = DeeString_NewUtf8(name,
	                                namelen,
	                                STRING_ERROR_FSTRICT);
	if unlikely(!name_object)
		goto err;
	result = DeeModule_New(name_object);
	Dee_Decref(name_object);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_New(/*String*/ DeeObject *__restrict name) {
	DeeModuleObject *result;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	result = DeeGCObject_CALLOC(DeeModuleObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeModule_Type);
	result->mo_name    = (DeeStringObject *)name;
	result->mo_bucketv = empty_module_buckets;
	rwlock_cinit(&result->mo_lock);
	Dee_Incref(name);
	weakref_support_init(result);
	DeeGC_Track((DREF DeeObject *)result);
done:
	return (DREF DeeObject *)result;
}


PRIVATE ATTR_COLD int DCALL
err_invalid_module_name_s(char const *__restrict module_name, size_t module_namesize) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "%$q is not a valid module name",
	                       module_namesize, module_name);
}

PRIVATE ATTR_COLD int DCALL
err_module_not_found(DeeObject *__restrict module_name) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "Module %r could not be found",
	                       module_name);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_DoGet(char const *__restrict name,
                size_t size, dhash_t hash) {
	DREF DeeModuleObject *result = NULL;
	/* Check if the caller requested the builtin deemon module. */
	if (size == DeeString_SIZE(&str_deemon) &&
	    hash == DeeString_Hash(&str_deemon) &&
	    memcmp(name, DeeString_STR(&str_deemon),
	           DeeString_SIZE(&str_deemon) * sizeof(char)) == 0) {
		/* Yes, they did. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}
	rwlock_read(&modules_glob_lock);
	if (modules_glob_a) {
		result = modules_glob_v[hash % modules_glob_a].mb_list;
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (DeeString_SIZE(result->mo_name) == size &&
#ifdef CONFIG_NOCASE_FS
			    /* TODO: This comparison doesn't work for mixed LATIN-1/UTF-8 strings */
			    MEMCASEEQ(DeeString_STR(result->mo_name), name,
			              size * sizeof(char))
#else /* CONFIG_NOCASE_FS */
			    memcmp(DeeString_STR(result->mo_name), name,
			           size * sizeof(char)) == 0
#endif /* !CONFIG_NOCASE_FS */
			    ) {
				Dee_Incref(result);
				break; /* Found it! */
			}
			result = result->mo_globnext;
		}
	}
	rwlock_endread(&modules_glob_lock);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_Get(DeeObject *__restrict module_name) {
	/* TODO: Support for mixed LATIN-1/UTF-8 strings */
	return DeeModule_DoGet(DeeString_STR(module_name),
	                       DeeString_SIZE(module_name),
#ifdef CONFIG_NOCASE_FS
	                       DeeString_HashCase(module_name)
#else /* CONFIG_NOCASE_FS */
	                       DeeString_Hash(module_name)
#endif /* !CONFIG_NOCASE_FS */
	                       );
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_GetString(/*utf-8*/ char const *__restrict module_name,
                    size_t module_namesize) {
	return DeeModule_DoGet(module_name,
	                       module_namesize,
#ifdef CONFIG_NOCASE_FS
	                       Dee_HashCaseUtf8(module_name, module_namesize)
#else /* CONFIG_NOCASE_FS */
	                       Dee_HashUtf8(module_name, module_namesize)
#endif /* !CONFIG_NOCASE_FS */
	                       );
}


#if 0
#define IS_VALID_MODULE_CHARACTER(ch)                \
	(!((ch) == '/' || (ch) == '\\' || (ch) == '|' || \
	   (ch) == '&' || (ch) == '~' || (ch) == '%' ||  \
	   (ch) == '$' || (ch) == '?' || (ch) == '!' ||  \
	   (ch) == '*' || (ch) == '\'' || (ch) == '\"'))
#else
#define IS_VALID_MODULE_CHARACTER(ch)                                      \
	((DeeUni_Flags(ch) &                                                   \
	  (UNICODE_FALPHA | UNICODE_FLOWER | UNICODE_FUPPER | UNICODE_FTITLE | \
	   UNICODE_FDECIMAL | UNICODE_FSYMSTRT | UNICODE_FSYMCONT)) ||         \
	 ((ch) == '-' || (ch) == '=' || (ch) == ',' || (ch) == '(' ||          \
	  (ch) == ')' || (ch) == '[' || (ch) == ']' || (ch) == '{' ||          \
	  (ch) == '}' || (ch) == '<' || (ch) == '>' || (ch) == '+'))
#endif

#define Dee_MODULE_OPENINPATH_FNORMAL      0x0000 /* Normal flags */
#define Dee_MODULE_OPENINPATH_FRELMODULE   0x0001 /* The module name is relative */
#define Dee_MODULE_OPENINPATH_FTHROWERROR  0x0002 /* Throw an error if the module isn't found. */

LOCAL DREF DeeModuleObject *DCALL
DeeModule_OpenInPathAbs(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                        /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                        DeeObject *module_global_name,
                        struct compiler_options *options,
                        unsigned int mode) {
	DREF DeeStringObject *module_name_ob;
	DREF DeeStringObject *module_path_ob;
	DREF DeeModuleObject *result;
	char *buf, *dst, *module_name_start;
	size_t i, len;
	dhash_t hash;
	DEE_DPRINTF("[RT] Searching for %s%k in %$q as %$q\n",
	            module_global_name ? "global module " : "module",
	            module_global_name ? module_global_name : Dee_EmptyString,
	            module_pathsize, module_path,
	            module_namesize, module_name);
#ifndef CONFIG_NO_DEC
	buf = (char *)Dee_AMalloc((module_pathsize + 1 + module_namesize + 6) * sizeof(char));
#else /* !CONFIG_NO_DEC */
	buf = (char *)Dee_AMalloc((module_pathsize + 1 + module_namesize + 5) * sizeof(char));
#endif /* CONFIG_NO_DEC */
	if unlikely(!buf)
		goto err;
	dst = buf;
	for (i = 0; i < module_pathsize;) {
		char ch = module_path[i++];
		if (!ISSEP(ch)) {
			*dst++ = ch;
			continue;
		}
		while (i < module_pathsize && ISSEP(module_path[i]))
			++i;
		if (dst >= buf + 1 && dst[-1] == '.') {
			/* Skip self-directory references. */
			if (dst == buf + 1 || dst[-2] == SEP) {
				--dst;
				continue;
			}
			if (dst >= buf + 2 && dst[-2] == '.' &&
			    (dst == buf + 2 || dst[-3] == SEP)) {
				/* Skip parent-directory references. */
				dst -= 3;
				while (dst > buf && dst[-1] != SEP)
					--dst;
				if (dst != buf)
					--dst;
				continue;
			}
		}
		*dst++ = SEP;
	}
	if (dst > buf && dst[-1] != SEP)
		*dst++ = SEP;
	/* Step #1: Check for a cached variant of a user-script. */
	module_name_start = (char *)module_name;
	for (i = 0; i < module_namesize; ++i) {
		char ch = module_name[i];
		if (ch == '.') {
			if unlikely(module_name_start == (char *)module_name + i)
				goto err_bad_module_name; /* Don't allow multiple consecutive dots here! */
			ch                = SEP;
			module_name_start = (char *)module_name + i + 1;
		} else if (!IS_VALID_MODULE_CHARACTER(ch)) {
err_bad_module_name:
			err_invalid_module_name_s(module_name, module_namesize);
			goto err_buf;
		}
		dst[i] = ch;
	}
	dst += (size_t)(module_name_start - module_name);
	module_namesize -= (size_t)(module_name_start - module_name);
	module_name = module_name_start;

	dst[module_namesize + 0] = '.';
	dst[module_namesize + 1] = 'd';
	dst[module_namesize + 2] = 'e';
	dst[module_namesize + 3] = 'e';
	dst[module_namesize + 4] = '\0';
	len                      = (size_t)(dst - buf) + module_namesize + 4;
#ifdef CONFIG_NOCASE_FS
	hash = Dee_HashCaseUtf8(buf, len);
#else /* CONFIG_NOCASE_FS */
	hash = Dee_HashUtf8(buf, len);
#endif /* !CONFIG_NOCASE_FS */
again_search_fs_modules:

	/* Search for modules that have already been cached. */
	rwlock_read(&modules_lock);
	if (modules_a) {
		result = modules_v[hash % modules_a].mb_list;
		for (; result; result = result->mo_next) {
			char *utf8_path;
#ifdef CONFIG_HOST_WINDOWS
			if (hash != result->mo_pathhash)
				continue;
#elif defined(CONFIG_NOCASE_FS)
			if (hash != DeeString_HashCase((DeeObject *)result->mo_path))
				continue;
#else
			if (hash != DeeString_Hash((DeeObject *)result->mo_path))
				continue;
#endif
			utf8_path = DeeString_TryAsUtf8((DeeObject *)result->mo_path);
			if unlikely(!utf8_path) {
				Dee_Incref(result);
				rwlock_endread(&modules_lock);
				utf8_path = DeeString_AsUtf8((DeeObject *)result->mo_path);
				if unlikely(!utf8_path)
					goto err_buf_r;
				if (WSTR_LENGTH(utf8_path) == len &&
				/* TODO: Support for mixed LATIN-1/UTF-8 strings */
#ifdef CONFIG_NOCASE_FS
				    MEMCASEEQ(utf8_path, buf, len * sizeof(char)) /* TODO: UTF-8 case compare! */
#else /* CONFIG_NOCASE_FS */
				    memcmp(utf8_path, buf, len * sizeof(char)) == 0
#endif /* !CONFIG_NOCASE_FS */
				    ) {
					goto got_result_set_global;
				}
				Dee_Decref(result);
				goto again_search_fs_modules;
			}
			if (WSTR_LENGTH(utf8_path) != len)
				continue;
				/* TODO: Support for mixed LATIN-1/UTF-8 strings */
#ifdef CONFIG_NOCASE_FS
			if (!MEMCASEEQ(utf8_path, buf, len * sizeof(char))) /* TODO: UTF-8 case compare! */
				continue;
#else /* CONFIG_NOCASE_FS */
			if (memcmp(utf8_path, buf, len * sizeof(char)) != 0)
				continue;
#endif /* !CONFIG_NOCASE_FS */
			/* Found it! */
			Dee_Incref(result);
			rwlock_endread(&modules_lock);
got_result_set_global:
			if (module_global_name && likely(!result->mo_globpself)) {
				DeeModuleObject *existing_module;
				/* Cache the module as global (if it wasn't already) */
again_find_existing_global_module:
				rwlock_write(&modules_glob_lock);
				COMPILER_READ_BARRIER();
				if likely(result->mo_globpself == NULL) {
					/* TODO: Must change `result->mo_name' to `module_global_name'
					 * ${LIBPATH}/foo/bar.dee:
					 * >> global helper = import(".bar");
					 * ${LIBPATH}/foo/baz.dee:
					 * >> print "Hi, I'm a helper module";
					 * main.dee:
					 * >> local a = import("foo.bar");
					 * >> print a.__name__;            // "foo.bar"
					 * >> print a.helper.__name__;     // "baz"
					 * >> print a.helper.__isglobal__; // false
					 * >> assert a.helper === import("foo.baz");
					 * >> // The re-import as global must changed the name to "foo.baz".
					 * >> // If we don't do this, then "foo.baz" will (incorrectly) become
					 * >> // available as `import("baz")', even though it's file location
					 * >> // in relation to the system library path would require it to be
					 * >> // addressed as "foo.baz".
					 * >> print a.helper.__name__;     // "foo.baz" (currently, and wrongly still "baz")
					 * >> print a.helper.__isglobal__; // true
					 * NOTE: This requires some changes to the runtime, as `mo_name' is
					 *       currently assumed to be `[const]', when that must to be changed to:
					 *      [const_if(mo_globpself != NULL)]
					 *      [lock_if(mo_globpself == NULL,INTERNAL(modules_glob_lock))]
					 */

					existing_module = find_glob_module(result->mo_name);
					if unlikely(existing_module) {
						Dee_Incref(existing_module);
						rwlock_endwrite(&modules_glob_lock);
						Dee_Decref_likely(result);
						result = existing_module;
						goto got_result;
					}
					if (!add_glob_module(result)) {
						rwlock_endwrite(&modules_glob_lock);
						if (Dee_CollectMemory(1))
							goto again_find_existing_global_module;
						goto err_buf_r;
					}
				}
				rwlock_endwrite(&modules_glob_lock);
			}
			goto got_result;
		}
#if !defined(CONFIG_NO_DEX) && 0 /* Dex modules are still cached under their real name! */
		/* Also search for cached dex extensions. */
		{
			dhash_t dex_hash;
#ifdef CONFIG_HOST_WINDOWS
#define dex_len (len)
			dst[module_namesize + 2] = 'l';
			dst[module_namesize + 3] = 'l';
#else /* CONFIG_HOST_WINDOWS */
#define dex_len (len - 1)
			dst[module_namesize + 1] = 's';
			dst[module_namesize + 2] = 'o';
			dst[module_namesize + 3] = '\0';
#endif /* !CONFIG_HOST_WINDOWS */
#ifdef CONFIG_NOCASE_FS
			dex_hash = Dee_HashCaseUtf8(buf, dex_len);
#else /* CONFIG_NOCASE_FS */
			dex_hash                 = Dee_HashUtf8(buf, dex_len);
#endif /* !CONFIG_NOCASE_FS */
			result = modules_v[dex_hash % modules_a].mb_list;
			for (; result; result = result->mo_next) {
				char *utf8_path;
#ifdef CONFIG_HOST_WINDOWS
				if (dex_hash != result->mo_pathhash)
					continue;
#elif defined(CONFIG_NOCASE_FS)
				if (dex_hash != DeeString_HashCase((DeeObject *)result->mo_path))
					continue;
#else
				if (dex_hash != DeeString_Hash((DeeObject *)result->mo_path))
					continue;
#endif
				utf8_path = DeeString_TryAsUtf8((DeeObject *)result->mo_path);
				if unlikely(!utf8_path) {
					Dee_Incref(result);
					rwlock_endread(&modules_lock);
					utf8_path = DeeString_AsUtf8((DeeObject *)result->mo_path);
					if unlikely(!utf8_path)
						goto err_buf_r;
					if (WSTR_LENGTH(utf8_path) == dex_len &&
#ifdef CONFIG_NOCASE_FS
					    MEMCASEEQ(utf8_path, buf, dex_len * sizeof(char)) /* TODO: UTF-8 case compare! */
#else /* CONFIG_NOCASE_FS */
					    memcmp(utf8_path, buf, dex_len * sizeof(char)) == 0
#endif /* !CONFIG_NOCASE_FS */
					    ) {
						goto got_result_set_global;
					}
					Dee_Decref(result);
					goto again_search_fs_modules;
				}
				if (WSTR_LENGTH(utf8_path) != dex_len)
					continue;
#ifdef CONFIG_NOCASE_FS
				if (!MEMCASEEQ(utf8_path, buf, dex_len * sizeof(char))) /* TODO: UTF-8 case compare! */
					continue;
#else /* CONFIG_NOCASE_FS */
				if (memcmp(utf8_path, buf, dex_len * sizeof(char)) != 0)
					continue;
#endif /* !CONFIG_NOCASE_FS */
				/* Found it! */
				Dee_Incref(result);
				rwlock_endread(&modules_lock);
				goto got_result_set_global;
			}
#ifndef CONFIG_HOST_WINDOWS
			dst[module_namesize + 1] = 'd';
#endif /* !CONFIG_HOST_WINDOWS */
			dst[module_namesize + 2] = 'e';
			dst[module_namesize + 3] = 'e';
		}
#endif /* !CONFIG_NO_DEX */
	}
	rwlock_endread(&modules_lock);
	if (ITER_ISOK(module_global_name)) {
		module_name_ob = (DREF DeeStringObject *)module_global_name;
		Dee_Incref(module_global_name);
	} else {
		module_name_ob = (DREF DeeStringObject *)DeeString_NewUtf8(module_name,
		                                                           module_namesize,
		                                                           STRING_ERROR_FIGNORE);
		if unlikely(!module_name_ob)
			goto err_buf;
	}

	/* The module hasn't been loaded, yet.
	 * Try to load it now! */
#ifndef CONFIG_NO_DEC
	if (!options || !(options->co_decloader & DEC_FDISABLE)) {
		/* Step #1: Try to load the module from a pre-compiled .dec file.
		 * By checking this before searching for trying to load a DEX extension,
		 * we allow the user to override extensions with user-code scripts by
		 * simply generating a dec file using `deemon -c', without having to
		 * actually delete the dex library. */
		memmove(dst + 1, dst, (module_namesize + 5) * sizeof(char));
		dst[0] = '.';
		ASSERT(dst[module_namesize + 1] == '.');
		ASSERT(dst[module_namesize + 2] == 'd');
		ASSERT(dst[module_namesize + 3] == 'e');
		dst[module_namesize + 4] = 'c';
		ASSERT(dst[module_namesize + 5] == '\0');
		{
			DREF DeeObject *dec_stream;
			dec_stream = DeeFile_OpenString(buf, OPEN_FRDONLY, 0);
			memmove(dst, dst + 1, (module_namesize + 5) * sizeof(char));
			ASSERT(dst[module_namesize + 0] == '.');
			ASSERT(dst[module_namesize + 1] == 'd');
			ASSERT(dst[module_namesize + 2] == 'e');
			dst[module_namesize + 3] = 'e';
			ASSERT(dst[module_namesize + 4] == '\0');
			if (dec_stream != ITER_DONE) {
				int error;
				DeeModuleObject *existing_module;
				/* The compiled file _does_ exist! */
				if unlikely(!dec_stream)
					goto err_buf_module_name;
				module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FIGNORE);
				if unlikely(!module_path_ob) {
err_buf_name_dec_stream:
					Dee_Decref_likely(dec_stream);
					goto err_buf_module_name;
				}
				result = (DREF DeeModuleObject *)DeeModule_New((DeeObject *)module_name_ob);
				if unlikely(!result) {
/*err_buf_name_dec_stream_path:*/
					Dee_Decref_likely(module_path_ob);
					goto err_buf_name_dec_stream;
				}
				Dee_Decref_unlikely(module_name_ob);
				result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef CONFIG_HOST_WINDOWS
				result->mo_pathhash = hash;
#endif /* CONFIG_HOST_WINDOWS */
				result->mo_flags |= MODULE_FLOADING;
				COMPILER_WRITE_BARRIER();
				/* Cache the new module as part of the filesystem
				 * module cache, as well as the global module cache. */
				if (module_global_name) {
set_dec_file_module_global:
#ifndef CONFIG_NO_THREADS
					rwlock_write(&modules_lock);
					if (!rwlock_trywrite(&modules_glob_lock)) {
						rwlock_endwrite(&modules_lock);
						rwlock_write(&modules_glob_lock);
						if (!rwlock_trywrite(&modules_lock)) {
							rwlock_endwrite(&modules_glob_lock);
							goto set_dec_file_module_global;
						}
					}
#endif /* !CONFIG_NO_THREADS */
					existing_module = find_file_module(module_path_ob, hash);
					if likely(!existing_module)
						existing_module = find_glob_module(module_name_ob);
					if unlikely(existing_module) {
						Dee_Incref(existing_module);
						rwlock_endwrite(&modules_glob_lock);
						rwlock_endwrite(&modules_lock);
						Dee_DecrefDokill(result);
						Dee_Decref_likely(dec_stream);
						result = existing_module;
						goto try_load_module_after_dec_failure;
					}
					/* Add the module to the file-cache. */
					if ((modules_c >= modules_a && !rehash_file_modules()) ||
					    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
						rwlock_endwrite(&modules_lock);
						/* Try to collect some memory, then try again. */
						if (Dee_CollectMemory(1))
							goto set_dec_file_module_global;
						Dee_Decref_likely(dec_stream);
						goto err_buf_r;
					}
					add_glob_module(result);
					add_file_module(result);
					rwlock_endwrite(&modules_glob_lock);
					rwlock_endwrite(&modules_lock);
				} else {
set_dec_file_module:
					rwlock_write(&modules_lock);
					existing_module = find_file_module(module_path_ob, hash);
					if unlikely(existing_module) {
						Dee_Incref(existing_module);
						rwlock_endwrite(&modules_lock);
						Dee_DecrefDokill(result);
						Dee_Decref_likely(dec_stream);
						result = existing_module;
try_load_module_after_dec_failure:
						if (DeeModule_BeginLoading(result) == 0)
							goto load_module_after_dec_failure;
						goto got_result;
					}
					/* Add the module to the file-cache. */
					if unlikely(!add_file_module(result)) {
						rwlock_endwrite(&modules_lock);
						/* Try to collect some memory, then try again. */
						if (Dee_CollectMemory(1))
							goto set_dec_file_module;
						Dee_Decref_likely(dec_stream);
						goto err_buf_r;
					}
					rwlock_endwrite(&modules_lock);
				}
				error = DeeModule_OpenDec(result, dec_stream, options);
				Dee_Decref_likely(dec_stream);
				if likely(error == 0) {
					/* Successfully loaded the DEC file. */
					DeeModule_DoneLoading(result);
					goto got_result;
				}
				if unlikely(error < 0) {
					/* Hard error. */
					DeeModule_FailLoading(result);
					goto err_buf_r;
				}
load_module_after_dec_failure:
				/* Must try to load the module from its source file. */
				dec_stream = DeeFile_OpenString(buf, OPEN_FRDONLY, 0);
				if unlikely(!ITER_ISOK(dec_stream)) {
					DeeModule_FailLoading(result);
					if (dec_stream == ITER_DONE) {
						DeeError_Throwf(&DeeError_FileNotFound,
						                "Missing source file `%s' when the associated dec file does found",
						                buf);
					}
					goto err_buf_r;
				}
				error = DeeModule_LoadSourceStreamEx(result,
				                                     dec_stream,
				                                     0,
				                                     0,
				                                     options,
				                                     result->mo_path);
				Dee_Decref_likely(dec_stream);
				if unlikely(error) {
					DeeModule_FailLoading(result);
					goto err_buf_r;
				}
				DeeModule_DoneLoading(result);
				goto got_result;
			}
		}
	}
#endif /* !CONFIG_NO_DEC */
#ifdef CONFIG_NO_DEX
	module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(dst, len, STRING_ERROR_FSTRICT);
	if unlikely(!module_path_ob)
		goto err_buf;
#else /* CONFIG_NO_DEX */
	/* Try to load the module from a DEX extension. */
	ASSERT(dst[module_namesize + 0] == '.');
	ASSERT(dst[module_namesize + 1] == 'd');
	ASSERT(dst[module_namesize + 2] == 'e');
	ASSERT(dst[module_namesize + 3] == 'e');
	ASSERT(dst[module_namesize + 4] == '\0');
#ifdef CONFIG_HOST_WINDOWS
	dst[module_namesize + 2] = 'l';
	dst[module_namesize + 3] = 'l';
#ifdef USE_LOADLIBRARY
	module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
	if unlikely(!module_path_ob)
		goto err_buf_module_name;
#endif /* USE_LOADLIBRARY */
#endif /* CONFIG_HOST_WINDOWS */
	{
#ifdef USE_LOADLIBRARY
		HMODULE hModule;
		{
			LPCWSTR wPath;
			wPath = (LPCWSTR)DeeString_AsWide((DeeObject *)module_path_ob);
			if unlikely(!wPath)
				goto err_buf_module_name_path;
			hModule = LoadLibraryW(wPath);
		}
#define CLOSE_MODULE(x) FreeLibrary(x)
		if (hModule == NULL)
#else /* USE_LOADLIBRARY */
		void *hModule;
#ifndef CONFIG_HOST_WINDOWS
		dst[module_namesize + 1] = 's';
		dst[module_namesize + 2] = 'o';
		dst[module_namesize + 3] = '\0';
#endif /* !CONFIG_HOST_WINDOWS */
		hModule = dlopen(buf,
		                 RTLD_LOCAL |
#ifdef RTLD_LAZY
		                 RTLD_LAZY
#else /* RTLD_LAZY */
		                 RTLD_NOW
#endif /* !RTLD_LAZY */
		);
#define CLOSE_MODULE(x) dlclose(x)
		if (hModule == NULL)
#endif /* !USE_LOADLIBRARY */
		{
#ifdef USE_LOADLIBRARY
			{
				size_t temp = DeeString_WLEN(module_path_ob);
				DeeString_SetChar(module_path_ob, temp - 2, 'e'); /* Was `l' */
				DeeString_SetChar(module_path_ob, temp - 1, 'e'); /* Was `l' */
			}
#else /* USE_LOADLIBRARY */
#ifndef CONFIG_HOST_WINDOWS
			dst[module_namesize + 1] = 'd';
#endif /* !CONFIG_HOST_WINDOWS */
			dst[module_namesize + 2] = 'e';
			dst[module_namesize + 3] = 'e';
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
			if unlikely(!module_path_ob)
				goto err_buf;
#endif /* !USE_LOADLIBRARY */
		} else {
			int error;
			DeeModuleObject *existing_module;
#ifndef USE_LOADLIBRARY
#ifdef CONFIG_HOST_WINDOWS
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
#else /* CONFIG_HOST_WINDOWS */
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len - 1, STRING_ERROR_FSTRICT);
#endif /* !CONFIG_HOST_WINDOWS */
			if unlikely(!module_path_ob) {
				CLOSE_MODULE(hModule);
				goto err_buf_module_name;
			}
#endif /* !USE_LOADLIBRARY */
			result = (DREF DeeModuleObject *)DeeDex_New((DeeObject *)module_name_ob);
			if unlikely(!result) {
				CLOSE_MODULE(hModule);
				goto err_buf_module_name_path;
			}
			Dee_Decref_unlikely(module_name_ob);
			result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef CONFIG_HOST_WINDOWS
			result->mo_pathhash = hash;
#endif /* CONFIG_HOST_WINDOWS */
			result->mo_flags |= MODULE_FLOADING;
			COMPILER_WRITE_BARRIER();

			/* Register the new dex module globally. */
			if (module_global_name) {
set_dex_file_module_global:
#ifndef CONFIG_NO_THREADS
				rwlock_write(&modules_lock);
				if (!rwlock_trywrite(&modules_glob_lock)) {
					rwlock_endwrite(&modules_lock);
					rwlock_write(&modules_glob_lock);
					if (!rwlock_trywrite(&modules_lock)) {
						rwlock_endwrite(&modules_glob_lock);
						goto set_dex_file_module_global;
					}
				}
#endif /* !CONFIG_NO_THREADS */
				existing_module = find_file_module(module_path_ob, hash);
				if likely(!existing_module)
					existing_module = find_glob_module(module_name_ob);
				if unlikely(existing_module) {
					Dee_Incref(existing_module);
					rwlock_endwrite(&modules_glob_lock);
					rwlock_endwrite(&modules_lock);
					Dee_DecrefDokill(result);
					CLOSE_MODULE(hModule);
					result = existing_module;
					goto try_load_module_after_dex_failure;
				}
				/* Add the module to the file-cache. */
				if ((modules_c >= modules_a && !rehash_file_modules()) ||
				    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
					rwlock_endwrite(&modules_lock);
					/* Try to collect some memory, then try again. */
					if (Dee_CollectMemory(1))
						goto set_dex_file_module_global;
					CLOSE_MODULE(hModule);
					goto err_buf_r;
				}
				add_glob_module(result);
				add_file_module(result);
				rwlock_endwrite(&modules_glob_lock);
				rwlock_endwrite(&modules_lock);
			} else {
set_dex_file_module:
				rwlock_write(&modules_lock);
				existing_module = find_file_module(module_path_ob, hash);
				if unlikely(existing_module) {
					Dee_Incref(existing_module);
					rwlock_endwrite(&modules_lock);
					Dee_DecrefDokill(result);
					CLOSE_MODULE(hModule);
					result = existing_module;
try_load_module_after_dex_failure:
					if (DeeModule_BeginLoading(result) == 0)
						goto load_module_after_dex_failure;
					goto got_result;
				}
				/* Add the module to the file-cache. */
				if unlikely(!add_file_module(result)) {
					rwlock_endwrite(&modules_lock);
					/* Try to collect some memory, then try again. */
					if (Dee_CollectMemory(1))
						goto set_dex_file_module;
					CLOSE_MODULE(hModule);
					goto err_buf_r;
				}
				rwlock_endwrite(&modules_lock);
			}
load_module_after_dex_failure:
			error = dex_load_handle((DeeDexObject *)result,
			                        (void *)hModule,
			                        (DeeObject *)result->mo_path);
			if unlikely(error) {
				DeeModule_FailLoading(result);
				goto err_buf_r;
			}
			DeeModule_DoneLoading(result);
			goto got_result;
		}
	}
#endif /* !CONFIG_NO_DEX */
	ASSERT(module_path_ob != NULL);
	/* Load a regular, old source file. */
	{
		DeeModuleObject *existing_module;
		DREF DeeObject *source_stream;
		int error;
		source_stream = DeeFile_Open((DeeObject *)module_path_ob, OPEN_FRDONLY, 0);
		if unlikely(!ITER_ISOK(source_stream)) {
			Dee_Decref(module_name_ob);
			if (source_stream == ITER_DONE) {
				/* The source file doesn't exist! */
				if (!(mode & Dee_MODULE_OPENINPATH_FTHROWERROR)) {
					Dee_Decref_likely(module_path_ob);
					result = (DREF DeeModuleObject *)ITER_DONE;
					goto got_result;
				}
				err_file_not_found_ob((DeeObject *)module_path_ob);
			}
			goto err_buf_module_path;
		}
		result = (DREF DeeModuleObject *)DeeModule_New((DeeObject *)module_name_ob);
		if unlikely(!result) {
/*err_buf_name_source_stream:*/
			Dee_Decref_likely(source_stream);
			goto err_buf_module_name_path;
		}
		Dee_Decref_unlikely(module_name_ob);
		result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef CONFIG_HOST_WINDOWS
		result->mo_pathhash = hash;
#endif /* CONFIG_HOST_WINDOWS */
		result->mo_flags |= MODULE_FLOADING;
		COMPILER_WRITE_BARRIER();

		/* Register the new dex module globally. */
		if (module_global_name) {
set_src_file_module_global:
#ifndef CONFIG_NO_THREADS
			rwlock_write(&modules_lock);
			if (!rwlock_trywrite(&modules_glob_lock)) {
				rwlock_endwrite(&modules_lock);
				rwlock_write(&modules_glob_lock);
				if (!rwlock_trywrite(&modules_lock)) {
					rwlock_endwrite(&modules_glob_lock);
					goto set_src_file_module_global;
				}
			}
#endif /* !CONFIG_NO_THREADS */
			existing_module = find_file_module(module_path_ob, hash);
			if likely(!existing_module)
				existing_module = find_glob_module(module_name_ob);
			if unlikely(existing_module) {
				Dee_Incref(existing_module);
				rwlock_endwrite(&modules_glob_lock);
				rwlock_endwrite(&modules_lock);
				Dee_DecrefDokill(result);
				Dee_Decref_likely(source_stream);
				result = existing_module;
				goto try_load_module_after_src_failure;
			}
			/* Add the module to the file-cache. */
			if ((modules_c >= modules_a && !rehash_file_modules()) ||
			    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
				rwlock_endwrite(&modules_lock);
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(1))
					goto set_src_file_module_global;
				Dee_Decref_likely(source_stream);
				goto err_buf_r;
			}
			add_glob_module(result);
			add_file_module(result);
			rwlock_endwrite(&modules_glob_lock);
			rwlock_endwrite(&modules_lock);
		} else {
set_src_file_module:
			rwlock_write(&modules_lock);
			existing_module = find_file_module(module_path_ob, hash);
			if unlikely(existing_module) {
				Dee_Incref(existing_module);
				rwlock_endwrite(&modules_lock);
				Dee_DecrefDokill(result);
				Dee_Decref_likely(source_stream);
				result = existing_module;
try_load_module_after_src_failure:
				if (DeeModule_BeginLoading(result) == 0)
					goto load_module_after_src_failure;
				goto got_result;
			}
			/* Add the module to the file-cache. */
			if unlikely(!add_file_module(result)) {
				rwlock_endwrite(&modules_lock);
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(1))
					goto set_src_file_module;
				Dee_Decref_likely(source_stream);
				goto err_buf_r;
			}
			rwlock_endwrite(&modules_lock);
		}
load_module_after_src_failure:
		error = DeeModule_LoadSourceStreamEx(result,
		                                     source_stream,
		                                     0,
		                                     0,
		                                     options,
		                                     result->mo_path);
		Dee_Decref_likely(source_stream);
		if unlikely(error) {
			DeeModule_FailLoading(result);
			goto err_buf_r;
		}
		DeeModule_DoneLoading(result);
		/*goto got_result;*/
	}
got_result:
	Dee_AFree(buf);
	return result;
/*
err_buf_module_name_r:
	Dee_Decref_unlikely(result);*/
err_buf_module_name_path:
	Dee_Decref_likely(module_path_ob);
err_buf_module_name:
	Dee_Decref_likely(module_name_ob);
	goto err_buf;
err_buf_module_path:
	Dee_Decref_likely(module_path_ob);
	goto err_buf;
err_buf_r:
	Dee_Decref_unlikely(result);
err_buf:
	Dee_AFree(buf);
err:
	return NULL;
}


/* Low-level module import processing function, used for importing modules
 * relative to some given base-path, while also able to process relative module
 * names, as well as support all of the various form in which modules can appear.
 * @param: module_path:        The base path from which to offset `module_name'
 *                             If this path is relative, it will be made absolute to
 *                             the current working directory.
 * @param: module_pathsize:    The length of `module_path' in bytes
 * @param: module_name:        The demangled name of the module to import
 * @param: module_namesize:    The length of `module_name' in bytes
 * @param: module_global_name: The name that should be used to register the module
 *                             in the global module namespace, or `NULL' if the module
 *                             should not be registered as global, or `ITER_DONE' if
 *                             the name should automatically be generated from `module_path'
 *                             NOTE: If another module with the same global name already
 *                                   exists by the time to module gets registered as global,
 *                                   that module will be returned instead!
 * @param: options:            Compiler options detailing how a module should be loaded
 * @param: mode:               The open mode (set of `MODULE_OPENINPATH_F*')
 * Module files are attempted to be opened in the following order:
 * >> SEARCH_MODULE_FILESYSTEM_CACHE(joinpath(module_path,module_name + ".dee"));
 * >>#ifndef CONFIG_NO_DEC
 * >> TRY_LOAD_DEC_FILE(joinpath(module_path,"." + module_name + ".dec"));
 * >>#endif // !CONFIG_NO_DEC
 * >>#ifndef CONFIG_NO_DEX
 * >>#ifdef CONFIG_HOST_WINDOWS
 * >> TRY_LOAD_DEX_LIBRARY(joinpath(module_path,module_name + ".dll"));
 * >>#else
 * >> TRY_LOAD_DEX_LIBRARY(joinpath(module_path,module_name + ".so"));
 * >>#endif
 * >>#endif // !CONFIG_NO_DEX
 * >> TRY_LOAD_SOURCE_FILE(joinpath(module_path,module_name + ".dee"));
 * EXAMPLES:
 * >> char const *path = "/usr/lib/deemon/lib";
 * >> char const *name = "util";
 * >> // Opens:
 * >> //   - /usr/lib/deemon/lib/
 * >> DeeModule_OpenInPath(path,strlen(path),
 * >>                      name,strlen(name),
 * >>                      NULL,NULL,
 * >>                      Dee_MODULE_OPENINPATH_FTHROWERROR);
 * @return: * :        The module that was imported.
 * @return: ITER_DONE: The module could not be found (only when `Dee_MODULE_OPENINPATH_FTHROWERROR' isn't set)
 * @return: NULL:      An error occurred. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenInPath(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                     /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                     DeeObject *module_global_name,
                     struct compiler_options *options,
                     unsigned int mode) {
	size_t additional_count;
	/* Walk up the directory path for upwards references in the relative path. */
	additional_count = 0;
	for (;;) {
		while (module_pathsize && ISSEP(module_path[module_pathsize - 1]))
			--module_pathsize;
		if (module_pathsize >= 1 &&
		    module_path[module_pathsize - 1] == '.') {
			if (module_pathsize == 1 || ISSEP(module_path[module_pathsize - 2])) {
				/* Current-directory reference. */
				module_pathsize -= 2;
				continue;
			}
			if (module_pathsize >= 2 &&
			    module_path[module_pathsize - 2] == '.' &&
			    (module_pathsize == 2 || ISSEP(module_path[module_pathsize - 3]))) {
				/* Parent-directory reference. */
				++additional_count;
				module_pathsize -= 3;
				continue;
			}
		}
		if (additional_count) {
			--additional_count;
		} else {
			if (!(mode & Dee_MODULE_OPENINPATH_FRELMODULE))
				break;
			++module_name;
			--module_namesize;
			if (!module_namesize || *module_name != '.')
				break;
		}
		while (module_pathsize && !ISSEP(module_path[module_pathsize - 1]))
			--module_pathsize;
	}
#ifdef CONFIG_HOST_WINDOWS
	if unlikely(module_pathsize < 2 || module_path[1] != ':')
#else /* CONFIG_HOST_WINDOWS */
	if unlikely(!module_pathsize || module_path[0] != '/')
#endif /* !CONFIG_HOST_WINDOWS */
	{
		/* Must make the given module path absolute. */
		DREF DeeStringObject *abs_path; /*utf-8*/
		char *abs_utf8;
		DREF DeeModuleObject *result;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (print_pwd(&printer) < 0)
			goto err_printer;
		if (unicode_printer_print(&printer, module_path, module_pathsize) < 0)
			goto err_printer;
		abs_path = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		if unlikely(!abs_path)
			goto err;
		abs_utf8 = DeeString_AsUtf8((DeeObject *)abs_path);
		if unlikely(!abs_utf8)
			goto err_abs_path;
		result = DeeModule_OpenInPathAbs(abs_utf8,
		                                 WSTR_LENGTH(abs_utf8),
		                                 module_name, module_namesize,
		                                 module_global_name,
		                                 options,
		                                 mode);
		Dee_Decref(abs_path);
		return (DREF DeeObject *)result;
err_abs_path:
		Dee_Decref(abs_path);
		goto err;
err_printer:
		unicode_printer_fini(&printer);
		goto err;
	}
	return (DREF DeeObject *)DeeModule_OpenInPathAbs(module_path, module_pathsize,
	                                                 module_name, module_namesize,
	                                                 module_global_name,
	                                                 options,
	                                                 mode);
err:
	return NULL;
}


PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenGlobal(DeeObject *__restrict module_name,
                     struct compiler_options *options,
                     bool throw_error) {
	DREF DeeObject *path;
	DREF DeeModuleObject *result;
	DeeListObject *paths;
	size_t i;
	/*utf-8*/ char const *module_namestr;
	size_t module_namelen;
	ASSERT_OBJECT_TYPE_EXACT(module_name, &DeeString_Type);
	/* First off: Check if this is a request for the builtin `deemon' module.
	 * NOTE: This check is always done in case-sensitive mode! */
	if (DeeString_SIZE(module_name) == DeeString_SIZE(&str_deemon) &&
	    DeeString_Hash(module_name) == DeeString_Hash(&str_deemon) &&
	    memcmp(DeeString_STR(module_name), DeeString_STR(&str_deemon),
	           DeeString_SIZE(&str_deemon) * sizeof(char)) == 0) {
		/* Yes, it is. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}

	/* Search for a cache entry for this module in the global module cache. */
	rwlock_read(&modules_glob_lock);
	result = find_glob_module((DeeStringObject *)module_name);
	if (result) {
		Dee_Incref(result);
		rwlock_endread(&modules_glob_lock);
		goto done;
	}
	rwlock_endread(&modules_glob_lock);

	module_namestr = DeeString_AsUtf8(module_name);
	if unlikely(!module_namestr)
		goto err;
	module_namelen = WSTR_LENGTH(module_namestr);

	/* Default case: Must load a new module. */
	paths = DeeModule_GetPath();
	DeeList_LockRead(paths);
	for (i = 0; i < DeeList_SIZE(paths); ++i) {
		path = DeeList_GET(paths, i);
		Dee_Incref(path);
		DeeList_LockEndRead(paths);
		if (DeeString_Check(path)) {
			/*utf-8*/ char const *path_str;
			path_str = DeeString_AsUtf8(path);
			if unlikely(!path_str)
				goto err_path;
			result = (DREF DeeModuleObject *)DeeModule_OpenInPath(path_str,
			                                                      WSTR_LENGTH(path_str),
			                                                      module_namestr,
			                                                      module_namelen,
			                                                      module_name,
			                                                      options,
			                                                      Dee_MODULE_OPENINPATH_FNORMAL);
			if (result != (DREF DeeModuleObject *)ITER_DONE)
				goto done_path;
		} else {
			/* `path' isn't a string */
		}
		Dee_Decref(path);
		DeeList_LockRead(paths);
	}
	DeeList_LockEndRead(paths);
	if (!throw_error)
		return ITER_DONE;
	err_module_not_found(module_name);
err:
	return NULL;
err_path:
	Dee_Decref(path);
	goto err;
done_path:
	Dee_Decref(path);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenGlobalString(/*utf-8*/ char const *__restrict module_name,
                           size_t module_namesize,
                           struct compiler_options *options,
                           bool throw_error) {
	/* TODO: This function can be written to be faster */
	DREF DeeObject *name_object, *result;
	name_object = DeeString_NewUtf8(module_name,
	                                module_namesize,
	                                STRING_ERROR_FSTRICT);
	if unlikely(!name_object)
		return NULL;
	result = DeeModule_OpenGlobal(name_object, options, throw_error);
	Dee_Decref(name_object);
	return result;
}



PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenRelative(DeeObject *__restrict module_name,
                       /*utf-8*/ char const *__restrict module_pathname,
                       size_t module_pathsize,
                       struct compiler_options *options,
                       bool throw_error) {
	/*utf-8*/ char *module_name_str;
	module_name_str = DeeString_AsUtf8(module_name);
	if unlikely(!module_name_str)
		goto err;
	if (*module_name_str != '.')
		return DeeModule_OpenGlobal(module_name, options, throw_error);
	return DeeModule_OpenInPath(module_pathname,
	                            module_pathsize,
	                            module_name_str,
	                            WSTR_LENGTH(module_name_str),
	                            NULL,
	                            options,
	                            throw_error ? (Dee_MODULE_OPENINPATH_FRELMODULE | Dee_MODULE_OPENINPATH_FTHROWERROR)
	                                        : (Dee_MODULE_OPENINPATH_FRELMODULE));
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_OpenRelativeString(/*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                             /*utf-8*/ char const *__restrict module_pathname, size_t module_pathsize,
                             struct compiler_options *options, bool throw_error) {
	/* Shouldn't happen: Not actually a relative module name. */
	if (!module_namesize || *module_name != '.')
		return DeeModule_OpenGlobalString(module_name, module_namesize, options, throw_error);
	return (DREF DeeObject *)DeeModule_OpenInPath(module_pathname,
	                                              module_pathsize,
	                                              module_name,
	                                              module_namesize,
	                                              NULL,
	                                              options,
	                                              throw_error ? (Dee_MODULE_OPENINPATH_FRELMODULE | Dee_MODULE_OPENINPATH_FTHROWERROR)
	                                                          : (Dee_MODULE_OPENINPATH_FRELMODULE));
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeModule_Import(DeeObject *__restrict module_name,
                 struct compiler_options *options,
                 bool throw_error) {
	DREF DeeObject *result;
	struct code_frame *frame = DeeThread_Self()->t_exec;
	if (frame) {
		DeeStringObject *path;
		char *begin, *end;
		/* Load the path of the currently executing code (for relative imports). */
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE_EXACT(frame->cf_func->fo_code, &DeeCode_Type);
		ASSERT_OBJECT_TYPE(frame->cf_func->fo_code->co_module, &DeeModule_Type);
		path = frame->cf_func->fo_code->co_module->mo_path;
		if unlikely(!path)
			goto open_normal;
		ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
		begin = DeeString_AsUtf8((DeeObject *)path);
		if unlikely(!begin)
			goto err;
		end = (begin = DeeString_STR(path)) + DeeString_SIZE(path);
		/* Find the end of the current path. */
		while (begin != end && !ISSEP(end[-1]))
			--end;
		result = DeeModule_OpenRelative(module_name, begin, (size_t)(end - begin), options, throw_error);
	} else {
open_normal:
		/* Without an execution frame, dismiss the relative import() code handling. */
		result = DeeModule_OpenGlobal(module_name, options, throw_error);
	}
	return result;
err:
	return NULL;
}


/* Same as `DeeModule_Import', but relative module
 * paths are imported in relation to `basemodule' */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_ImportRel(DeeObject *__restrict basemodule,
                    DeeObject *__restrict module_name,
                    struct compiler_options *options,
                    bool throw_error) {
	DeeStringObject *path;
	char *begin, *end;
	ASSERT_OBJECT_TYPE(basemodule, &DeeModule_Type);
	/* Load the path of the currently executing code (for relative imports). */
	path = ((DeeModuleObject *)basemodule)->mo_path;
	if unlikely(!path)
		return DeeModule_OpenGlobal(module_name, options, throw_error);
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	begin = DeeString_AsUtf8((DeeObject *)path);
	if unlikely(!begin)
		goto err;
	end = begin + WSTR_LENGTH(begin);
	/* Find the end of the current path. */
	while (begin < end && !ISSEP(end[-1]))
		--end;
	return DeeModule_OpenRelative(module_name,
	                              begin,
	                              (size_t)(end - begin),
	                              options,
	                              throw_error);
err:
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeModule_ImportRelString(DeeObject *__restrict basemodule,
                          /*utf-8*/ char const *__restrict module_name,
                          size_t module_namesize,
                          struct compiler_options *options,
                          bool throw_error) {
	DeeStringObject *path;
	char *begin, *end;
	ASSERT_OBJECT_TYPE(basemodule, &DeeModule_Type);
	/* Load the path of the currently executing code (for relative imports). */
	path = ((DeeModuleObject *)basemodule)->mo_path;
	if unlikely(!path)
		return DeeModule_OpenGlobalString(module_name, module_namesize, options, throw_error);
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	begin = DeeString_AsUtf8((DeeObject *)path);
	if unlikely(!begin)
		goto err;
	end = begin + WSTR_LENGTH(begin);
	/* Find the end of the current path. */
	while (begin < end && !ISSEP(end[-1]))
		--end;
	return DeeModule_OpenRelativeString(module_name,
	                                    module_namesize,
	                                    begin,
	                                    (size_t)(end - begin),
	                                    options,
	                                    throw_error);
err:
	return NULL;
}


PRIVATE int DCALL
module_rehash_globals(void) {
	size_t i, new_mask = (current_rootscope->rs_bucketm << 1) | 1;
	struct module_symbol *new_vec;
	ASSERT(!(new_mask & (new_mask + 1)));
	new_vec = (struct module_symbol *)Dee_Calloc((new_mask + 1) *
	                                             sizeof(struct module_symbol));
	if unlikely(!new_vec)
		goto err;
	for (i = 0; i <= current_rootscope->rs_bucketm; ++i) {
		size_t j, perturb;
		struct module_symbol *item;
		item = &current_rootscope->rs_bucketv[i];
		if (!item->ss_name)
			continue;
		perturb = j = item->ss_hash & new_mask;
		for (;; j = MODULE_HASHNX(j, perturb), MODULE_HASHPT(perturb)) {
			struct module_symbol *new_item = &new_vec[j & new_mask];
			if (new_item->ss_name)
				continue;
			/* Copy the old item into this new slot. */
			memcpy(new_item, item, sizeof(struct module_symbol));
			break;
		}
	}
	/* Free the old bucket vector and assign the new one */
	Dee_Free(current_rootscope->rs_bucketv);
	current_rootscope->rs_bucketm = (uint16_t)new_mask;
	current_rootscope->rs_bucketv = new_vec;
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
module_import_symbol(DeeModuleObject *__restrict self,
                     DeeStringObject *__restrict name,
                     DeeObject *__restrict value,
                     unsigned int mode,
                     uint16_t *__restrict pglobala) {
	if (!(mode & DEE_EXEC_RUNMODE_FDEFAULTS_ARE_GLOBALS)) {
		struct TPPKeyword *kwd;
		struct symbol *sym;
		/* Define as local constants. */
		kwd = TPPLexer_LookupKeyword(DeeString_STR(name),
		                             DeeString_SIZE(name),
		                             1);
		if unlikely(!kwd)
			goto err;
		sym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope,
		                                kwd);
		if unlikely(sym) {
			if (sym->s_type == SYMBOL_TYPE_CONST &&
			    sym->s_const == value)
				return 0; /* Already defined. */
			DeeError_Throwf(&DeeError_KeyError,
			                "Default value for %r has already been defined",
			                name);
			goto err;
		}
		sym = new_local_symbol_in_scope((DeeScopeObject *)current_rootscope,
		                                kwd,
		                                NULL);
		if unlikely(!sym)
			goto err;
		sym->s_type  = SYMBOL_TYPE_CONST;
		sym->s_const = value;
		Dee_Incref(value);
	} else {
		dhash_t i, perturb, hash;
		uint16_t addr;
		/* Rehash the global symbol table is need be. */
		if (self->mo_globalc / 2 >= current_rootscope->rs_bucketm &&
		    module_rehash_globals())
			goto err;
		if (self->mo_globalc >= *pglobala) {
			DREF DeeObject **new_globalv;
			uint16_t new_globala = *pglobala * 2;
			if (!new_globala)
				new_globala = 2;
			ASSERT(new_globala > self->mo_globalc);
			new_globalv = (DREF DeeObject **)Dee_TryRealloc(self->mo_globalv,
			                                                new_globala *
			                                                sizeof(DREF DeeObject *));
			if unlikely(!new_globalv) {
				new_globala = self->mo_globalc + 1;
				new_globalv = (DREF DeeObject **)Dee_Realloc(self->mo_globalv,
				                                             new_globala *
				                                             sizeof(DREF DeeObject *));
				if unlikely(!new_globalv)
					goto err;
			}
			self->mo_globalv = new_globalv;
			*pglobala        = new_globala;
		}

		/* Append the symbol initializer */
		addr                   = self->mo_globalc++;
		self->mo_globalv[addr] = value;
		Dee_Incref(value);

		/* Insert the new object into the symbol table. */
		hash    = DeeString_Hash((DeeObject *)name);
		perturb = i = MODULE_HASHST(self, hash);
		for (;; i = MODULE_HASHNX(i, perturb), MODULE_HASHPT(perturb)) {
			struct module_symbol *item = MODULE_HASHIT(self, i);
			if (item->ss_name)
				continue;
			/* Use this item. */
			item->ss_name  = DeeString_STR(name);
			item->ss_flags = MODSYM_FNAMEOBJ;
			item->ss_doc   = NULL;
			item->ss_hash  = hash;
			item->ss_index = addr;
			Dee_Incref(name);
			break;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
module_import_symbol_pair(DeeModuleObject *__restrict self,
                          DeeObject *__restrict symbol_pair,
                          unsigned int mode,
                          uint16_t *__restrict pglobala) {
	DREF DeeObject *key_and_value[2];
	int result;
	if (DeeObject_Unpack(symbol_pair, 2, key_and_value))
		goto err;
	if (DeeObject_AssertTypeExact(key_and_value[0], &DeeString_Type))
		result = -1;
	else {
		result = module_import_symbol(self,
		                              (DeeStringObject *)key_and_value[0],
		                              key_and_value[1],
		                              mode,
		                              pglobala);
	}
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE int DCALL
module_import_symbols(DeeModuleObject *__restrict self,
                      DeeObject *__restrict default_symbols,
                      unsigned int mode,
                      uint16_t *__restrict pglobala) {
	DREF DeeObject *iterator, *elem;
	int temp;
	iterator = DeeObject_IterSelf(default_symbols);
	if unlikely(!iterator)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = module_import_symbol_pair(self, elem, mode, pglobala);
		Dee_Decref(elem);
		if unlikely(temp)
			goto err;
	}
	Dee_Decref(iterator);
	return 0;
err:
	return -1;
}


/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module or the module's root function used to describe the code
 * that is being executed. */
PUBLIC WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleStream(DeeObject *source_stream,
                            unsigned int mode,
                            int start_line, int start_col,
                            struct compiler_options *options,
                            DeeObject *default_symbols,
                            DeeObject *source_pathname,
                            DeeObject *module_name) {
	struct TPPFile *base_file;
	DREF DeeCodeObject *root_code;
	DREF DeeModuleObject *result;
	DREF DeeCompilerObject *compiler;
	DREF struct ast *code;
	uint16_t assembler_flags, result_globala;
	/* Create a new module. */
	if (!module_name) {
		if (source_pathname) {
			char *name  = DeeString_STR(source_pathname);
			size_t size = DeeString_SIZE(source_pathname);
			char *name_end, *name_start;
			name_end = name + size;
#ifdef CONFIG_HOST_WINDOWS
			name_start = name_end;
			while (name_start != name && !ISSEP(name_start[-1]))
				--name_start;
#else /* CONFIG_HOST_WINDOWS */
			name_start = (char *)memrchr(name, SEP, size);
			if (!name_start)
				name_start = name - 1;
			++name_start;
#endif /* !CONFIG_HOST_WINDOWS */

			/* Get rid of a file extension in the module name. */
			while (name_end != name_start && name_end[-1] != '.')
				--name_end;
			while (name_end != name_start && name_end[-1] == '.')
				--name_end;
			if (name_end == name_start)
				name_end = name + size;
			module_name = DeeString_NewSized(name_start,
			                                 (size_t)(name_end - name_start));
			if unlikely(!module_name)
				goto err;
		} else {
			module_name = Dee_EmptyString;
			Dee_Incref(Dee_EmptyString);
		}
	} else {
		Dee_Incref(module_name);
	}
	/* Create the new module. */
	result = DeeGCObject_CALLOC(DeeModuleObject);
	if unlikely(!result)
		goto err_module_name;
	result->mo_name    = (DREF DeeStringObject *)module_name; /* Inherit reference. */
	result->mo_bucketv = empty_module_buckets;
	rwlock_cinit(&result->mo_lock);
	DeeObject_Init(result, &DeeModule_Type);
	weakref_support_init(result);
	DeeGC_Track((DREF DeeObject *)result);
	result->mo_flags = MODULE_FLOADING;
#ifndef CONFIG_NO_THREADS
	result->mo_loader = DeeThread_Self();
#endif /* !CONFIG_NO_THREADS */
	compiler = DeeCompiler_New((DeeObject *)result,
	                           options ? options->co_compiler : COMPILER_FNORMAL);
	if unlikely(!compiler)
		goto err_r;
	COMPILER_BEGIN(compiler);
	base_file = TPPFile_OpenStream((stream_t)source_stream,
	                               source_pathname
	                               ? DeeString_STR(source_pathname)
	                               : "");
	if unlikely(!base_file)
		goto err_r_compiler;
	/* Set the starting-line offset. */
	if (TPPFile_SetStartingLineAndColumn(base_file, start_line, start_col)) {
		TPPFile_Decref(base_file);
		goto err_r_compiler;
	}

	/* Push the initial source file onto the #include-stack,
	 * and TPP inherit our reference to it. */
	TPPLexer_PushFileInherited(base_file);

	/* Override the name that is used as the
	 * effective display/DDI string of the file. */
	if (options && options->co_filename) {
		struct TPPString *used_name;
		ASSERT_OBJECT_TYPE_EXACT(options->co_filename, &DeeString_Type);
		used_name = TPPString_New(DeeString_STR(options->co_filename),
		                          DeeString_SIZE(options->co_filename));
		if unlikely(!used_name)
			goto err_r_compiler;
		ASSERT(!base_file->f_textfile.f_usedname);
		base_file->f_textfile.f_usedname = used_name; /* Inherit */
	}
	ASSERT(!current_basescope->bs_name);

	if (!(mode & DEE_EXEC_RUNMODE_FHASPP)) {
		/* Disable preprocessor directives & macros. */
		TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_NO_DIRECTIVES |
		                              TPPLEXER_FLAG_NO_MACROS |
		                              TPPLEXER_FLAG_NO_BUILTIN_MACROS);
	}

	inner_compiler_options = NULL;
	if (options) {
		/* Set the name of the current base-scope, which
		 * describes the function of the module's root code. */
		if (options->co_rootname) {
			ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
			current_basescope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(options->co_rootname),
			                                                    DeeString_SIZE(options->co_rootname),
			                                                    1);
			if unlikely(!current_basescope->bs_name)
				goto err_r_compiler;
		}

		compiler->cp_options   = options;
		inner_compiler_options = options->co_inner;
		parser_flags           = options->co_parser;
		optimizer_flags        = options->co_optimizer;
		optimizer_unwind_limit = options->co_unwind_limit;
		if (options->co_tabwidth)
			TPPLexer_Current->l_tabsize = (size_t)options->co_tabwidth;
		if (parser_flags & PARSE_FLFSTMT)
			TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
		if (options->co_setup) {
			/* Run a custom setup protocol. */
			if unlikely((*options->co_setup)(options->co_setup_arg) < 0)
				goto err_r_compiler;
		}
	}

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err_r_compiler;
		current_basescope->bs_argv = (struct symbol **)Dee_Malloc(1 * sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_r_compiler;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_HAVE_DECLARATION_DOCUMENTATION
		dots->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_HAVE_DECLARATION_DOCUMENTATION */
		dots->s_type  = SYMBOL_TYPE_ARG;
		dots->s_symid = 0;
		dots->s_flag |= SYMBOL_FALLOC;
		current_basescope->bs_argc    = 1;
		current_basescope->bs_argv[0] = dots;
		current_basescope->bs_varargs = dots;
		current_basescope->bs_flags |= CODE_FVARARGS;
	}

	result_globala = 0;
	if (default_symbols) {
		/* Provide default symbols as though they were defined as globals. */
		if unlikely(module_import_symbols(result, default_symbols, mode, &result_globala))
			goto err_r_compiler;
		current_rootscope->rs_globalc = result->mo_globalc;
	}

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if unlikely(yield() < 0)
		code = NULL;
	else {
		/* Parse statements until the end of the source stream. */
		switch (mode & DEE_EXEC_RUNMODE_MASK) {

		case DEE_EXEC_RUNMODE_STMT:
			code = ast_parse_statement(false);
			goto pack_code_in_return;

		case DEE_EXEC_RUNMODE_EXPR:
			code = ast_parse_comma(AST_COMMA_NORMAL,
			                       AST_FMULTIPLE_KEEPLAST,
			                       NULL);
			goto pack_code_in_return;

		case DEE_EXEC_RUNMODE_FULLEXPR:
			code = ast_parse_comma(AST_COMMA_NORMAL |
			                       AST_COMMA_ALLOWVARDECLS |
			                       AST_COMMA_ALLOWTYPEDECL,
			                       AST_FMULTIPLE_KEEPLAST,
			                       NULL);
pack_code_in_return:
			if likely(code) {
				DREF struct ast *return_ast;
				return_ast = ast_putddi(ast_return(code), &code->a_ddi);
				ast_decref(code);
				code = return_ast;
			}
			break;

		default:
			code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, TOK_EOF);
			break;
		}
	}
	if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
		TPPLexer_ClearIfdefStack();

	/* Rethrow all errors that may have occurred during parsing. */
	if unlikely(parser_rethrow(code == NULL))
		goto err_r_compiler_code;
	if unlikely(!code)
		goto err_r_compiler;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		int error = ast_optimize_all(code, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(error != 0))
			error = -1;
		if (error)
			goto err_r_compiler_code;
	}

	assembler_flags = ASM_FNORMAL;
	if (options)
		assembler_flags = options->co_assembler;
	{
		uint16_t refc;
		struct asm_symbol_ref *refv;
		root_code = code_compile(code,
		                         assembler_flags,
		                         true,
		                         &refc,
		                         &refv);
		ASSERT(!root_code || !refc);
		ASSERT(!root_code || !refv);
	}
	ast_decref(code);

	/* Rethrow all errors that may have occurred during text assembly. */
	if (parser_rethrow(root_code == NULL))
		Dee_XClear(root_code);

	/* Check for errors during assembly. */
	if unlikely(!root_code)
		goto err_r_compiler;

	/* Finally, put together the module itself. */
	if (current_rootscope->rs_importa != current_rootscope->rs_importc) {
		DREF DeeModuleObject **new_vector;
		new_vector = (DREF DeeModuleObject **)Dee_TryRealloc(current_rootscope->rs_importv,
		                                                     current_rootscope->rs_importc *
		                                                     sizeof(DREF DeeModuleObject *));
		if likely(new_vector)
			current_rootscope->rs_importv = new_vector;
	}
	if (!result->mo_globalv) {
		result->mo_globalv = (DREF DeeObject **)Dee_Calloc(current_rootscope->rs_globalc *
		                                                   sizeof(DREF DeeObject *));
		if unlikely(!result->mo_globalv)
			goto err_r_compiler;
	} else {
		if (current_rootscope->rs_globalc > result_globala) {
			DREF DeeObject **final_globalv;
			final_globalv = (DREF DeeObject **)Dee_Realloc(result->mo_globalv,
			                                               current_rootscope->rs_globalc *
			                                               sizeof(DREF DeeObject *));
			if unlikely(!final_globalv)
				goto err_r_compiler;
			result->mo_globalv = final_globalv;
		}
		MEMSET_PTR(result->mo_globalv + result->mo_globalc, 0,
		           current_rootscope->rs_globalc - result->mo_globalc);
	}
	result->mo_globalc = current_rootscope->rs_globalc;
	result->mo_importc = current_rootscope->rs_importc;
#ifdef CONFIG_NO_THREADS
	result->mo_flags |= current_rootscope->rs_flags;
#else /* CONFIG_NO_THREADS */
	ATOMIC_FETCHOR(result->mo_flags, current_rootscope->rs_flags);
#endif /* !CONFIG_NO_THREADS */
	result->mo_bucketm = current_rootscope->rs_bucketm;
	result->mo_bucketv = current_rootscope->rs_bucketv;
	result->mo_importv = current_rootscope->rs_importv;
	result->mo_root    = root_code; /* Inherit reference. */

	/* Yes, we're just stealing all of these. */
	current_rootscope->rs_importv = NULL;
	current_rootscope->rs_importc = 0;
	current_rootscope->rs_importa = 0;
	current_rootscope->rs_bucketv = empty_module_buckets;
	current_rootscope->rs_bucketm = 0;

	{
		DREF DeeCodeObject *iter, *next;
		iter                       = current_rootscope->rs_code;
		current_rootscope->rs_code = NULL;
		while (iter) {
			next            = iter->co_next;
			iter->co_module = result;
			Dee_Incref(result); /* Create the new module-reference now stored in `iter->co_module'. */
			Dee_Decref(iter);   /* This reference was owned by the chain before. */
			iter = next;
		}
	}

	COMPILER_END();
	Dee_Decref(compiler);
	ATOMIC_FETCHOR(result->mo_flags, MODULE_FDIDLOAD);
	return (DREF DeeObject *)result;
err_r_compiler_code:
	ast_xdecref(code);
err_r_compiler:
	COMPILER_END();
	Dee_Decref(compiler);
err_r:
	ATOMIC_FETCHAND(result->mo_flags, ~(MODULE_FLOADING));
	Dee_Decref_likely(result);
	goto err;
err_module_name:
	Dee_Decref(module_name);
err:
	return NULL;
}





PUBLIC DeeListObject DeeModule_Path = {
	OBJECT_HEAD_INIT(&DeeList_Type),
	/* .l_alloc = */ 0,
	/* .l_size  = */ 0,
	/* .l_elem  = */ NULL
#ifndef CONFIG_NO_THREADS
	,
	/* .l_lock  = */ RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
};

#ifdef CONFIG_HOST_WINDOWS
#define DELIM ';'
#else /* CONFIG_HOST_WINDOWS */
#define DELIM ':'
#endif /* !CONFIG_HOST_WINDOWS */


#ifdef CONFIG_DEEMON_HOME
PRIVATE DEFINE_STRING(default_deemon_home,CONFIG_DEEMON_HOME);
#endif /* CONFIG_DEEMON_HOME */


#if !defined(CONFIG_DEEMON_HOME) && \
   (!defined(CONFIG_HOST_WINDOWS) && defined(CONFIG_HOST_UNIX))
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unix_readlink(/*utf-8*/ char const *__restrict path) {
	char *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	dssize_t req_size;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	bufsize                        = PATH_MAX;
	buffer                         = unicode_printer_alloc_utf8(&printer, bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		struct stat st;
		if (DeeThread_CheckInterrupt())
			goto err;
		DBG_ALIGNMENT_DISABLE();
		req_size = readlink(path, buffer, bufsize + 1);
		if unlikely(req_size < 0) {
handle_error:
			DBG_ALIGNMENT_ENABLE();
			error = errno;
			DeeError_SysThrowf(&DeeError_FSError, error,
			                   "Failed to read symbolic link %q",
			                   path);
			goto err;
		}
		DBG_ALIGNMENT_ENABLE();
		if ((size_t)req_size <= bufsize)
			break;
		DBG_ALIGNMENT_DISABLE();
		if (lstat(path, &st))
			goto handle_error;
		DBG_ALIGNMENT_ENABLE();
		/* Ensure that this is still a symbolic link. */
		if (!S_ISLNK(st.st_mode)) {
			error = EINVAL;
			goto handle_error;
		}
		new_size = (size_t)st.st_size;
		if (new_size <= bufsize)
			break; /* Shouldn't happen, but might due to race conditions? */
		new_buffer = unicode_printer_resize_utf8(&printer, buffer, new_size);
		if unlikely(!new_buffer)
			goto err;
		buffer  = new_buffer;
		bufsize = new_size;
	}
	/* Release unused data. */
	unicode_printer_confirm_utf8(&printer, buffer, (size_t)req_size);
	bufsize = UNICODE_PRINTER_LENGTH(&printer);
	while (bufsize && UNICODE_PRINTER_GETCHAR(&printer, bufsize - 1) != '/')
		--bufsize;
	while (bufsize && UNICODE_PRINTER_GETCHAR(&printer, bufsize - 1) == '/')
		--bufsize;
	UNICODE_PRINTER_SETCHAR(&printer, bufsize, '/');
	unicode_printer_truncate(&printer, bufsize + 1);
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}
#endif /* !CONFIG_DEEMON_HOME && (!CONFIG_HOST_WINDOWS && CONFIG_HOST_UNIX) */

PRIVATE WUNUSED DREF /*String*/DeeStringObject *
DCALL get_default_home(void) {
	DREF DeeStringObject *result;
#ifndef CONFIG_NO_DEEMON_HOME_ENVIRON
	char *env;
#ifndef CONFIG_DEEMON_HOME_ENVIRON
#define CONFIG_DEEMON_HOME_ENVIRON "DEEMON_HOME"
#endif /* !CONFIG_DEEMON_HOME_ENVIRON */
	DBG_ALIGNMENT_DISABLE();
	env = getenv(CONFIG_DEEMON_HOME_ENVIRON);
	DBG_ALIGNMENT_ENABLE();
	if (env) {
		size_t len = strlen(env);
		if (len) {
			DREF DeeStringObject *new_result;
			while (len && ISSEP(env[len - 1]))
				--len;
			result = (DREF DeeStringObject *)DeeString_NewUtf8(env, len + 1, STRING_ERROR_FIGNORE);
			if unlikely(!result)
				goto err;
			DeeString_SetChar(result, len - 1, SEP);
			new_result = (DREF DeeStringObject *)make_absolute((DeeObject *)result);
			Dee_Decref(result);
			return new_result;
		}
	}
#endif /* !CONFIG_NO_DEEMON_HOME_ENVIRON */
#ifdef CONFIG_DEEMON_HOME
	return_reference_((DeeStringObject *)&default_deemon_home);
#elif defined(CONFIG_HOST_WINDOWS)
	{
		DWORD dwBufSize = PATH_MAX, dwError;
		LPWSTR lpBuffer, lpNewBuffer;
		DREF DeeStringObject *new_result;
		lpBuffer = DeeString_NewWideBuffer(dwBufSize);
		if unlikely(!lpBuffer)
			goto err;
again_chk_intr:
		if (DeeThread_CheckInterrupt()) {
err_buffer:
			DeeString_FreeWideBuffer(lpBuffer);
			goto err;
		}
		for (;;) {
			DBG_ALIGNMENT_DISABLE();
			SetLastError(0);
			dwError = GetModuleFileNameW(NULL, lpBuffer, dwBufSize + 1);
			if (!dwError) {
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (dwError == ERROR_OPERATION_ABORTED)
					goto again_chk_intr;
				DeeString_FreeWideBuffer(lpBuffer);
				err_system_error_code("GetModuleFileName", dwError);
				goto err_buffer;
			}
			DBG_ALIGNMENT_ENABLE();
			if (dwError <= dwBufSize) {
				if (dwError < dwBufSize)
					break;
				DBG_ALIGNMENT_DISABLE();
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (dwError != ERROR_INSUFFICIENT_BUFFER)
					break;
			}
			/* Increase buffer size. */
			dwBufSize *= 2;
			lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwBufSize);
			if unlikely(!lpNewBuffer)
				goto err_buffer;
			lpBuffer = lpNewBuffer;
		}
		/* TODO: Check if the module's file name is a symbolic link.
		 *       If it turns out to be one, follow it! */

		/* Trim the trailing module filename, but keep 1 trailing slash. */
		while (dwError && lpBuffer[dwError - 1] != '\\')
			--dwError;
		while (dwError && lpBuffer[dwError - 1] == '\\')
			--dwError;
		lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError + 1);
		result = (DREF DeeStringObject *)DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
		if unlikely(!result)
			goto err;
		new_result = (DREF DeeStringObject *)make_absolute((DeeObject *)result);
		Dee_Decref(result);
		return new_result;
	}
#elif defined(CONFIG_HOST_UNIX)
	return (DREF DeeStringObject *)unix_readlink("/proc/self/exe");
#else
	return (DREF DeeStringObject *)DeeString_New(".");
#endif
err:
	return NULL;
}


#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(deemon_home_lock);
#endif /* !CONFIG_NO_THREADS */

PRIVATE DREF DeeStringObject *deemon_home = NULL;
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL DeeExec_GetHome(void) {
	DREF DeeStringObject *result;
	rwlock_read(&deemon_home_lock);
	result = deemon_home;
	if (result) {
		Dee_Incref(result);
		rwlock_endread(&deemon_home_lock);
		return (DREF DeeObject *)result;
	}
	rwlock_endread(&deemon_home_lock);
	/* Re-create the default home path. */
	result = get_default_home();

	/* Save the generated path in the global variable. */
	if likely(result) {
		DREF DeeStringObject *other;
		rwlock_write(&deemon_home_lock);
		other = deemon_home;
		if unlikely(other) {
			Dee_Incref(other);
			rwlock_endwrite(&deemon_home_lock);
			Dee_Decref(result);
			return (DREF DeeObject *)other;
		}
		Dee_Incref(result);
		deemon_home = result;
		rwlock_endwrite(&deemon_home_lock);
	}
	return (DREF DeeObject *)result;
}

PUBLIC void DCALL
DeeExec_SetHome(/*String*/ DeeObject *new_home) {
	DREF DeeStringObject *old_home;
	ASSERT_OBJECT_TYPE_EXACT_OPT(new_home, &DeeString_Type);
	Dee_XIncref(new_home);
	rwlock_write(&deemon_home_lock);
	old_home    = deemon_home;
	deemon_home = (DREF DeeStringObject *)new_home;
	rwlock_endwrite(&deemon_home_lock);
	Dee_XDecref(old_home);
}


PRIVATE void DCALL do_init_module_path(void) {
	int error;
#ifndef CONFIG_NO_DEEMON_PATH_ENVIRON
	{
		char *path;
		DREF DeeObject *path_part;
#ifndef CONFIG_DEEMON_PATH_ENVIRON
#define CONFIG_DEEMON_PATH_ENVIRON "DEEMON_PATH"
#endif /* !CONFIG_DEEMON_PATH_ENVIRON */
		DBG_ALIGNMENT_DISABLE();
		path = getenv(CONFIG_DEEMON_PATH_ENVIRON);
		DBG_ALIGNMENT_ENABLE();
		if (path) {
			while (*path) {
				/* Split the module path. */
				char *next_path = strchr(path, DELIM);
				if (next_path) {
					path_part = DeeString_NewUtf8(path,
					                              (size_t)(next_path - path),
					                              STRING_ERROR_FIGNORE);
					++next_path;
				} else {
					next_path = strend(path);
					path_part = DeeString_NewUtf8(path,
					                              (size_t)(next_path - path),
					                              STRING_ERROR_FIGNORE);
				}
				if unlikely(!path_part)
					goto init_error;
				error = DeeList_Append((DeeObject *)&DeeModule_Path, path_part);
				Dee_Decref(path_part);
				if unlikely(error)
					goto init_error;
				path = next_path;
			}
		}
#endif /* !CONFIG_NO_DEEMON_PATH_ENVIRON */
	}
#ifdef CONFIG_DEEMON_PATH
#define APPEND_PATH(str)                                       \
	{                                                          \
		PRIVATE DEFINE_STRING(_libpath_string, str);           \
		error = DeeList_Append((DeeObject *)&DeeModule_Path,   \
		                       (DeeObject *)&_libpath_string); \
		if unlikely(error)                                     \
			goto init_error;                                   \
	}
	CONFIG_DEEMON_PATH(APPEND_PATH)
#undef APPEND_PATH
#else /* CONFIG_DEEMON_PATH */
	/* Add the default path based on deemon-home. */
	{
		DREF DeeObject *default_path;
		default_path = DeeString_Newf("%Klib", DeeExec_GetHome());
		if unlikely(!default_path)
			goto init_error;
		error = DeeList_Append((DeeObject *)&DeeModule_Path, default_path);
		Dee_Decref(default_path);
		if unlikely(error)
			goto init_error;
	}
#endif /* !CONFIG_DEEMON_PATH */
	return;
init_error:
	DeeError_Print("Failed to initialize module path\n",
	               ERROR_PRINT_DOHANDLE);
}



#ifdef CONFIG_NO_THREADS
#define INIT_PENDING 0
#define INIT_COMPLET 1
#else /* CONFIG_NO_THREADS */
#define INIT_PENDING 0
#define INIT_PROGRES 1
#define INIT_COMPLET 2
#endif /* !CONFIG_NO_THREADS */

PRIVATE int module_init_state = INIT_PENDING;
PUBLIC void DCALL DeeModule_InitPath(void) {
	/* Lazily calculate hashes of exported objects upon first access. */
	if unlikely(module_init_state != INIT_COMPLET) {
#ifdef CONFIG_NO_THREADS
		do_init_module_path();
		module_init_state = INIT_COMPLET;
#else /* CONFIG_NO_THREADS */
		COMPILER_READ_BARRIER();
		if (ATOMIC_CMPXCH(module_init_state, INIT_PENDING, INIT_PROGRES)) {
			do_init_module_path();
			ATOMIC_WRITE(module_init_state, INIT_COMPLET);
		} else {
			while (ATOMIC_READ(module_init_state) != INIT_COMPLET)
				SCHED_YIELD();
		}
#endif /* !CONFIG_NO_THREADS */
	}
}



DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_C */
