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
#ifndef GUARD_DEEMON_EXECUTE_MODPATH_C
#define GUARD_DEEMON_EXECUTE_MODPATH_C 1

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
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/debug-alignment.h>
#include <hybrid/minmax.h>
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

#include <deemon/compiler/compiler.h>

#include <deemon/compiler/assembler.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/tpp.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#undef token
#undef tok
#undef yield
#undef yieldnb
#undef yieldnbif
#undef skip

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#define token             TPPLexer_Global.l_token
#define tok               TPPLexer_Global.l_token.t_id
#define yield()           TPPLexer_Yield()
#define yieldnb()         TPPLexer_YieldNB()
#define yieldnbif(allow)  ((allow) ? TPPLexer_YieldNB() : TPPLexer_Yield())
#define skip(expected_tok, ...) unlikely(likely(tok == (expected_tok)) ? (yield() < 0) : parser_skip(expected_tok, __VA_ARGS__))

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
#endif /* !PATH_MAX */

DECL_BEGIN

/*[[[deemon
print("#if 1");
print("#define HASHOF_str_deemon ", (_Dee_HashSelect from rt.gen.hash)("deemon"));
print("#else");
print("#define HASHOF_str_deemon DeeString_Hash((DeeObject *)&str_deemon)");
print("#endif");
]]]*/
#if 1
#define HASHOF_str_deemon _Dee_HashSelectC(0x4579666d, 0xeb3bb684d0ec756)
#else
#define HASHOF_str_deemon DeeString_Hash((DeeObject *)&str_deemon)
#endif
/*[[[end]]]*/

INTDEF struct module_symbol empty_module_buckets[];

#define SEP   DeeSystem_SEP
#define SEP_S DeeSystem_SEP_S
#define ISSEP DeeSystem_IsSep
#define ISABS DeeSystem_IsAbs

#ifdef DEE_SYSTEM_FS_ICASE
#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */
#endif /* DEE_SYSTEM_FS_ICASE */


#ifdef DEE_SYSTEM_FS_ICASE
#define fs_memcmp                        memcasecmp
#define fs_bcmp                          memcasecmp
#define fs_hashobj(ob)                   DeeString_HashCase((DeeObject *)Dee_REQUIRES_OBJECT(ob))
#define fs_hashstr(s)                    Dee_HashCaseStr(s)
#define fs_hashutf8(s, n)                Dee_HashCaseUtf8(s, n)
#define fs_hashmodname_equals(mod, hash) 1
#define fs_hashmodpath(mod)              ((mod)->mo_pathihash)
#define fs_hashmodpath_equals(mod, hash) ((mod)->mo_pathihash == (hash))
#else /* DEE_SYSTEM_FS_ICASE */
#define fs_memcmp                        memcmp
#define fs_bcmp                          bcmp
#define fs_hashobj(ob)                   DeeString_Hash((DeeObject *)Dee_REQUIRES_OBJECT(ob))
#define fs_hashstr(s)                    Dee_HashStr(s)
#define fs_hashutf8(s, n)                Dee_HashUtf8(s, n)
#define fs_hashmodpath(mod)              DeeString_HASH((DeeObject *)(mod)->mo_path)
#define fs_hashmodname_equals(mod, hash) (DeeString_HASH((mod)->mo_name) == (hash))
#define fs_hashmodpath_equals(mod, hash) (DeeString_HASH((mod)->mo_path) == (hash))
#endif /* !DEE_SYSTEM_FS_ICASE */

#define DeeString_FS_EQUALS_STR(lhs, rhs)            \
	(DeeString_SIZE(lhs) == DeeString_SIZE(rhs) &&   \
	 fs_bcmp(DeeString_STR(lhs), DeeString_STR(rhs), \
	         DeeString_SIZE(lhs) * sizeof(char)) == 0)
#define DeeString_FS_EQUALS_BUF(lhs, rhs_base, rhs_size) \
	(DeeString_SIZE(lhs) == (rhs_size) &&                \
	 fs_bcmp(DeeString_STR(lhs), rhs_base,               \
	         DeeString_SIZE(lhs) * sizeof(char)) == 0)



/* Begin loading the given module.
 * @return: 0: You're now responsible to load the module.
 * @return: 1: The module has already been loaded.
 * @return: 2: You've already started loading this module. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeModule_BeginLoading(DeeModuleObject *__restrict self) {
	uint16_t flags;
#ifndef CONFIG_NO_THREADS
	DeeThreadObject *caller = DeeThread_Self();
#endif /* !CONFIG_NO_THREADS */
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
begin_loading:
	flags = atomic_fetchor(&self->mo_flags, MODULE_FLOADING);
	if (flags & MODULE_FLOADING) {
		/* Module is already being loaded. */
		while ((flags = atomic_read(&self->mo_flags),
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
	atomic_and(&self->mo_flags, ~(MODULE_FLOADING));
}

PRIVATE NONNULL((1)) void DCALL
DeeModule_DoneLoading(DeeModuleObject *__restrict self) {
	atomic_or(&self->mo_flags, MODULE_FDIDLOAD);
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
			goto err;
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
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
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
	if (COMPILER_BEGIN(compiler))
		goto err_compiler_not_locked;
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
				DeeCompiler_LockEndWrite();
				return result;
			}
		}
	}

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err_compiler;
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_compiler;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		dots->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
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
	if unlikely(yield() < 0) {
		code = NULL;
	} else {
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
	DeeCompiler_LockEndWrite();
	return result;
err_compiler_ast:
	ast_decref(code);
err_compiler:
	DeeCompiler_End();
	Dee_Decref(compiler);
	DeeCompiler_LockEndWrite();
err:
	return -1;
err_compiler_not_locked:
	Dee_Decref(compiler);
	goto err;
}

/* Load the given module from a filestream opened for a source file.
 * @param: self:       The module that should be loaded.
 * @param: input_file: A file object to be used as input stream.
 * @param: start_line: The starting line number of the input stream (zero-based)
 * @param: start_col:  The starting column offset of the input stream (zero-based)
 * @return: -1:        An error occurred and was thrown.
 * @return:  0:        Successfully loaded the given module.
 * @return:  1:        The module has already been loaded/was loading but has finished now.
 * @return:  2:        The module is already being loaded in the calling thread.
 * This is the main interface for manually loading modules, as
 * well as compiling & linking source code that may not be found
 * as files within the real filesystem.
 * NOTE: I highly encourage you to set `options->co_pathname'
 *       to a file within the folder that should be used to
 *       resolve relative imports and #include statements,
 *       as without this information given, the process
 *       working directory will be used instead. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_LoadSourceStream(/*Module*/ DeeObject *__restrict self,
                           /*File*/ DeeObject *__restrict input_file,
                           int start_line, int start_col,
                           struct compiler_options *options) {
	int result;
	ASSERT(self != input_file);
	result = DeeModule_BeginLoading((DeeModuleObject *)self);
	if (result == 0) {
		result = DeeModule_LoadSourceStreamEx((DeeModuleObject *)self,
		                                      input_file,
		                                      start_line,
		                                      start_col,
		                                      options,
		                                      NULL);
		if unlikely(result) {
			DeeModule_FailLoading((DeeModuleObject *)self);
		} else {
			DeeModule_DoneLoading((DeeModuleObject *)self);
		}
	}
	return result;
}







LIST_HEAD(module_object_list, module_object);

/* Filesystem-based module hash table. */
PRIVATE size_t /*               */ modules_c = 0;    /* [lock(modules_lock)] Amount of modules in-cache. */
PRIVATE size_t /*               */ modules_a = 0;    /* [lock(modules_lock)] Allocated hash-map size. */
PRIVATE struct module_object_list *modules_v = NULL; /* [lock(modules_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t modules_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define modules_lock_reading()    Dee_atomic_rwlock_reading(&modules_lock)
#define modules_lock_writing()    Dee_atomic_rwlock_writing(&modules_lock)
#define modules_lock_tryread()    Dee_atomic_rwlock_tryread(&modules_lock)
#define modules_lock_trywrite()   Dee_atomic_rwlock_trywrite(&modules_lock)
#define modules_lock_canread()    Dee_atomic_rwlock_canread(&modules_lock)
#define modules_lock_canwrite()   Dee_atomic_rwlock_canwrite(&modules_lock)
#define modules_lock_waitread()   Dee_atomic_rwlock_waitread(&modules_lock)
#define modules_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&modules_lock)
#define modules_lock_read()       Dee_atomic_rwlock_read(&modules_lock)
#define modules_lock_write()      Dee_atomic_rwlock_write(&modules_lock)
#define modules_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&modules_lock)
#define modules_lock_upgrade()    Dee_atomic_rwlock_upgrade(&modules_lock)
#define modules_lock_downgrade()  Dee_atomic_rwlock_downgrade(&modules_lock)
#define modules_lock_endwrite()   Dee_atomic_rwlock_endwrite(&modules_lock)
#define modules_lock_endread()    Dee_atomic_rwlock_endread(&modules_lock)
#define modules_lock_end()        Dee_atomic_rwlock_end(&modules_lock)

/* Name-based, global module hash table. */
PRIVATE size_t /*               */ modules_glob_c = 0;    /* [lock(modules_glob_lock)] Amount of modules in-cache. */
PRIVATE size_t /*               */ modules_glob_a = 0;    /* [lock(modules_glob_lock)] Allocated hash-map size. */
PRIVATE struct module_object_list *modules_glob_v = NULL; /* [lock(modules_glob_lock)][0..modules_a][owned] Hash-map of modules, sorted by their filenames. */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t modules_glob_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define modules_glob_lock_reading()    Dee_atomic_rwlock_reading(&modules_glob_lock)
#define modules_glob_lock_writing()    Dee_atomic_rwlock_writing(&modules_glob_lock)
#define modules_glob_lock_tryread()    Dee_atomic_rwlock_tryread(&modules_glob_lock)
#define modules_glob_lock_trywrite()   Dee_atomic_rwlock_trywrite(&modules_glob_lock)
#define modules_glob_lock_canread()    Dee_atomic_rwlock_canread(&modules_glob_lock)
#define modules_glob_lock_canwrite()   Dee_atomic_rwlock_canwrite(&modules_glob_lock)
#define modules_glob_lock_waitread()   Dee_atomic_rwlock_waitread(&modules_glob_lock)
#define modules_glob_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&modules_glob_lock)
#define modules_glob_lock_read()       Dee_atomic_rwlock_read(&modules_glob_lock)
#define modules_glob_lock_write()      Dee_atomic_rwlock_write(&modules_glob_lock)
#define modules_glob_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&modules_glob_lock)
#define modules_glob_lock_upgrade()    Dee_atomic_rwlock_upgrade(&modules_glob_lock)
#define modules_glob_lock_downgrade()  Dee_atomic_rwlock_downgrade(&modules_glob_lock)
#define modules_glob_lock_endwrite()   Dee_atomic_rwlock_endwrite(&modules_glob_lock)
#define modules_glob_lock_endread()    Dee_atomic_rwlock_endread(&modules_glob_lock)
#define modules_glob_lock_end()        Dee_atomic_rwlock_end(&modules_glob_lock)


PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_file_module(DeeStringObject *__restrict module_file, dhash_t hash) {
	DeeModuleObject *result = NULL;
	ASSERT(modules_lock_reading());
	if (modules_a) {
		result = LIST_FIRST(&modules_v[hash % modules_a]);
		while (result) {
			ASSERTF(result->mo_path, "All modules found in the file cache must have a path assigned");
			ASSERT_OBJECT_TYPE_EXACT(result->mo_path, &DeeString_Type);
			if (fs_hashmodpath_equals(result, hash) &&
			    DeeString_FS_EQUALS_STR(result->mo_path, module_file))
				break; /* Found it! */
			result = LIST_NEXT(result, mo_link);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_glob_module(DeeStringObject *__restrict module_name) {
	dhash_t hash = fs_hashobj(module_name);
	DeeModuleObject *result = NULL;
	ASSERT(modules_glob_lock_reading());
	if (modules_glob_a) {
		result = LIST_FIRST(&modules_glob_v[hash % modules_glob_a]);
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (fs_hashmodname_equals(result, hash) &&
			    DeeString_FS_EQUALS_STR(result->mo_name, module_name))
				break; /* Found it! */
			result = LIST_NEXT(result, mo_globlink);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeModuleObject *DCALL
find_glob_module_str(/*utf-8*/ char const *__restrict module_name_str,
                     size_t module_name_len) {
	dhash_t hash = fs_hashutf8(module_name_str, module_name_len);
	DeeModuleObject *result = NULL;
	ASSERT(modules_glob_lock_reading());
	if (modules_glob_a) {
		result = LIST_FIRST(&modules_glob_v[hash % modules_glob_a]);
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (fs_hashmodname_equals(result, hash) &&
			    DeeString_FS_EQUALS_BUF(result->mo_name, module_name_str, module_name_len))
				break; /* Found it! */
			result = LIST_NEXT(result, mo_globlink);
		}
	}
	return result;
}

PRIVATE bool DCALL rehash_file_modules(void) {
	struct module_object_list *new_vector, *biter, *bend, *dst;
	DeeModuleObject *iter, *next;
	size_t new_size = modules_a * 2;
	ASSERT(modules_lock_writing());
	if unlikely(!new_size)
		new_size = 4;
do_alloc_new_vector:
	new_vector = (struct module_object_list *)Dee_TryCallocc(new_size, sizeof(struct module_object_list));
	if unlikely(!new_vector) {
		if (modules_a != 0)
			return true; /* Don't actually need to rehash. */
		if (new_size != 1) {
			new_size = 1;
			goto do_alloc_new_vector;
		}
		return false;
	}
	ASSERT(new_size != 0);
	bend = (biter = modules_v) + modules_a;
	for (; biter < bend; ++biter) {
		iter = LIST_FIRST(biter);
		while (iter) {
			next = LIST_NEXT(iter, mo_link);
			ASSERTF(iter->mo_path, "All modules found in the file cache must have a path assigned");
			ASSERT_OBJECT_TYPE_EXACT(iter->mo_path, &DeeString_Type);

			/* Re-hash this entry. */
			dst = &new_vector[fs_hashmodpath(iter) % new_size];
			LIST_REMOVE(iter, mo_link);
			LIST_INSERT_HEAD(dst, iter, mo_link);

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
	struct module_object_list *new_vector, *biter, *bend, *dst;
	DeeModuleObject *iter, *next;
	size_t new_size = modules_glob_a * 2;
	ASSERT(modules_glob_lock_writing());
	if unlikely(!new_size)
		new_size = 4;
do_alloc_new_vector:
	new_vector = (struct module_object_list *)Dee_TryCallocc(new_size, sizeof(struct module_object_list));
	if unlikely(!new_vector) {
		if (modules_glob_a != 0)
			return true; /* Don't actually need to rehash. */
		if (new_size != 1) {
			new_size = 1;
			goto do_alloc_new_vector;
		}
		return false;
	}
	ASSERT(new_size != 0);
	bend = (biter = modules_glob_v) + modules_glob_a;
	for (; biter < bend; ++biter) {
		iter = LIST_FIRST(biter);
		while (iter) {
			next = LIST_NEXT(iter, mo_globlink);
			ASSERT_OBJECT_TYPE_EXACT(iter->mo_name, &DeeString_Type);

			/* Re-hash this entry. */
			dst = &new_vector[fs_hashobj(iter->mo_name) % new_size];
			LIST_REMOVE(iter, mo_globlink);
			LIST_INSERT_HEAD(dst, iter, mo_globlink);

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
	struct module_object_list *bucket;
	ASSERT(!LIST_ISBOUND(self, mo_link));
	ASSERT_OBJECT_TYPE_EXACT(self->mo_path, &DeeString_Type);
	ASSERT(modules_lock_writing());
	if (modules_c >= modules_a && !rehash_file_modules())
		return false;
	ASSERT(modules_a != 0);
	/* Insert the module into the table. */
	hash = fs_hashmodpath(self);
	Dee_DPRINTF("[RT] Caching module by-file %r\n", self->mo_path);
	bucket = &modules_v[hash % modules_a];
	LIST_INSERT_HEAD(bucket, self, mo_link);
	++modules_c;
	return true;
}

PRIVATE NONNULL((1)) bool DCALL
add_glob_module(DeeModuleObject *__restrict self) {
	dhash_t hash;
	struct module_object_list *bucket;
	ASSERT(!LIST_ISBOUND(self, mo_globlink));
	ASSERT(self->mo_name);
#ifndef CONFIG_NO_THREADS
	ASSERT(modules_glob_lock_writing());
#endif /* !CONFIG_NO_THREADS */
	Dee_DPRINTF("[RT] Cached global module %r loaded from %r\n",
	            self->mo_name, self->mo_path ? self->mo_path : (DeeStringObject *)Dee_EmptyString);
	if (modules_glob_c >= modules_glob_a && !rehash_glob_modules())
		return false;
	ASSERT(modules_glob_a != 0);
	/* Insert the module into the table. */
	hash   = fs_hashobj((DeeObject *)self->mo_name);
	bucket = &modules_glob_v[hash % modules_glob_a];
	LIST_INSERT_HEAD(bucket, self, mo_globlink);
	++modules_glob_c;
	return true;
}



INTERN NONNULL((1)) void DCALL
module_unbind(DeeModuleObject *__restrict self) {
	if (LIST_ISBOUND(self, mo_link)) {
		modules_lock_write();
		COMPILER_READ_BARRIER();
		if (LIST_ISBOUND(self, mo_link)) {
			LIST_REMOVE(self, mo_link);
			if (!--modules_c) {
				Dee_Free(modules_v);
				modules_v = NULL;
				modules_a = 0;
			}
			modules_lock_endwrite();
		}
	}
	if (LIST_ISBOUND(self, mo_globlink)) {
		modules_glob_lock_write();
		COMPILER_READ_BARRIER();
		if (LIST_ISBOUND(self, mo_globlink)) {
			LIST_REMOVE(self, mo_globlink);
			if (!--modules_glob_c) {
				Dee_Free(modules_glob_v);
				modules_glob_v = NULL;
				modules_glob_a = 0;
			}
		}
		modules_glob_lock_endwrite();
	}
}

/* Given the filename of a module source file, load it
 * and create a new module from the contained source code.
 * NOTE: In case the module has been loaded before,
 *       return the already-loaded instance instead.
 * NOTE: In case the module is currently being loaded in the calling
 *       thread, that same partially loaded module is returned, meaning
 *       that the caller can easily check for `MODULE_FLOADING && !MODULE_FDIDLOAD'
 * @param: module_global_name: When non-NULL, use this as the module's actual name.
 *                             Also: register the module as a global module under this name when given.
 *                             When not given, the module isn't registered globally, and the
 *                             name of the module will be deduced from its `source_pathname'
 * @param: source_pathname:    The filename of the source file that should be opened.
 *                             When `NULL', simply use the absolute variant of `DeeString_AsUtf8(source_pathname)'
 * @param: throw_error:        When true, throw an error if the module couldn't be
 *                             found and return `NULL', otherwise return `ITER_DONE'.
 * @return: ITER_DONE:        `throw_error' is `true' and `source_pathname' could not be found. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
	module_path_ob = (DREF DeeStringObject *)DeeSystem_MakeAbsolute(source_pathname);
	if unlikely(!module_path_ob)
		goto err;

	/* Quick check if this module had already been opened. */
	modules_lock_read();
	hash   = fs_hashobj(module_path_ob);
	result = find_file_module(module_path_ob, hash);
	if (result && Dee_IncrefIfNotZero(result)) {
		modules_lock_endread();
got_result_modulepath:
		Dee_Decref(module_path_ob);
		goto got_result;
	}
	modules_lock_endread();

	/* Also search for an existing instance
	 * of the specified global module name. */
#if 1 /* This is optional */
	if (ITER_ISOK(module_global_name)) {
		modules_glob_lock_read();
		result = find_glob_module((DeeStringObject *)module_global_name);
		if (result && Dee_IncrefIfNotZero(result)) {
			modules_glob_lock_endread();
			goto got_result_modulepath;
		}
		modules_glob_lock_endread();
	}
#endif

	/* Open the module's source file stream. */
	input_stream = DeeFile_Open((DeeObject *)module_path_ob, OPEN_FRDONLY, 0);
	if unlikely(!ITER_ISOK(input_stream)) {
		result = (DREF DeeModuleObject *)input_stream;
		if (input_stream == ITER_DONE && throw_error) {
			err_file_not_found((DeeObject *)module_path_ob);
			result = NULL;
		}
		goto got_result_modulepath;
	}

	/* Create a new module. */
	if (ITER_ISOK(module_global_name)) {
		module_name_ob = (DREF DeeStringObject *)module_global_name;
	} else {
		char const *name_end, *name_start;
		char const *name;
		size_t size;
		name = DeeString_AsUtf8(source_pathname);
		if unlikely(!name)
			goto err_modulepath_inputstream;
		size      = WSTR_LENGTH(name);
		name_end  = name + size;
		name_start = DeeSystem_BaseName(name, size);

		/* Get rid of a file extension in the module name. */
		while (name_end > name_start && name_end[-1] != '.')
			--name_end;
		while (name_end > name_start && name_end[-1] == '.')
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
#ifdef DEE_SYSTEM_FS_ICASE
	result->mo_pathihash = hash;
#endif /* DEE_SYSTEM_FS_ICASE */
	result->mo_flags |= MODULE_FLOADING;
	COMPILER_WRITE_BARRIER();

	/* Cache the new module as part of the filesystem
	 * module cache, as well as the global module cache. */
	if (module_global_name) {
set_file_module_global:
#ifndef CONFIG_NO_THREADS
		modules_lock_write();
		if (!modules_glob_lock_trywrite()) {
			modules_lock_endwrite();
			modules_glob_lock_write();
			if (!modules_lock_trywrite()) {
				modules_glob_lock_endwrite();
				goto set_file_module_global;
			}
		}
#endif /* !CONFIG_NO_THREADS */
		existing_module = find_file_module(module_path_ob, hash);
		if likely(!existing_module)
			existing_module = find_glob_module(module_name_ob);
		if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
			modules_glob_lock_endwrite();
			modules_lock_endwrite();
			Dee_DecrefDokill(result);
			Dee_Decref_likely(input_stream);
			result = existing_module;
			goto try_load_module_after_failure;
		}

		/* Add the module to the file-cache. */
		if ((modules_c >= modules_a && !rehash_file_modules()) ||
		    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
			modules_lock_endwrite();

			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_file_module_global;
			goto err_inputstream_r;
		}
		add_glob_module(result);
		add_file_module(result);
		modules_glob_lock_endwrite();
		modules_lock_endwrite();
	} else {
set_file_module:
		modules_lock_write();
		existing_module = find_file_module(module_path_ob, hash);
		if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
			modules_lock_endwrite();
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
			modules_lock_endwrite();

			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_file_module;
			goto err_inputstream_r;
		}
		modules_lock_endwrite();
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
			goto err_r;
		}
		DeeModule_DoneLoading(result);
	}
got_result:
	return (DREF DeeObject *)result;
err_inputstream_r:
	Dee_Decref(input_stream);
err_r:
	Dee_Decref(result);
	goto err;
err_modulepath_inputstream:
	Dee_Decref(input_stream);
/*err_modulepath:*/
	Dee_Decref(module_path_ob);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_OpenSourceStream(DeeObject *source_stream,
                           int start_line, int start_col,
                           struct compiler_options *options,
                           /*String*/ DeeObject *source_pathname,
                           /*String*/ DeeObject *module_name) {
	DREF DeeModuleObject *result;
	int load_error;
	/* Create a new module. */
	if (!module_name) {
		if (source_pathname) {
			char const *name = DeeString_STR(source_pathname);
			size_t size = DeeString_SIZE(source_pathname);
			char const *name_end, *name_start;
			DREF DeeObject *name_object;
			name_end   = name + size;
			name_start = DeeSystem_BaseName(name, size);

			/* Get rid of a file extension in the module name. */
			while (name_end > name_start && name_end[-1] != '.')
				--name_end;
			while (name_end > name_start && name_end[-1] == '.')
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
		modules_glob_lock_read();
		result = find_glob_module((DeeStringObject *)module_name);
		if (result && Dee_IncrefIfNotZero(result)) {
			modules_glob_lock_endread();
			goto found_existing_module;
		}
		modules_glob_lock_endread();
		/* Create a new module. */
		result = (DREF DeeModuleObject *)DeeModule_New(module_name);
		/* Add the module to the global module cache. */
set_global_module:
		modules_glob_lock_write();
		existing_module = find_glob_module((DeeStringObject *)module_name);
		if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
			/* The module got created in the mean time. */
			modules_glob_lock_endwrite();
			Dee_Decref(result);
			result = existing_module;
			goto found_existing_module;
		}
		/* Add the module to the global cache. */
		if unlikely(!add_glob_module(result)) {
			modules_glob_lock_endwrite();
			/* Try to collect some memory, then try again. */
			if (Dee_CollectMemory(1))
				goto set_global_module;
			goto err_r;
		}
		modules_glob_lock_endwrite();
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

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_OpenSourceMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                           int start_line, int start_col,
                           struct compiler_options *options,
                           /*String*/ DeeObject *source_pathname,
                           /*String*/ DeeObject *module_name) {
	DREF DeeObject *source_stream, *result;
	source_stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!source_stream)
		goto err;
	result = DeeModule_OpenSourceStream(source_stream,
	                                    start_line,
	                                    start_col,
	                                    options,
	                                    source_pathname,
	                                    module_name);
	DeeFile_ReleaseMemory(source_stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_OpenSourceMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                 int start_line, int start_col, struct compiler_options *options,
                                 /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                 /*utf-8*/ char const *module_name, size_t module_namesize) {
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

/* Create a new module object that has yet to be initialized or loaded. */
PUBLIC WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_New(/*String*/ DeeObject *__restrict name) {
	DeeModuleObject *result;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	result = DeeGCObject_CALLOC(DeeModuleObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeModule_Type);
	result->mo_name    = (DeeStringObject *)name;
	result->mo_bucketv = empty_module_buckets;
	Dee_atomic_rwlock_cinit(&result->mo_lock);
	Dee_Incref(name);
	weakref_support_init(result);
	return DeeGC_Track((DREF DeeObject *)result);
err:
	return NULL;
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
	if (size == 6 && hash == HASHOF_str_deemon &&
	    bcmpc(name, STR_deemon, 6, sizeof(char)) == 0) {
		/* Yes, they did. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}

	modules_glob_lock_read();
	if (modules_glob_a) {
		result = LIST_FIRST(&modules_glob_v[hash % modules_glob_a]);
		while (result) {
			ASSERT_OBJECT_TYPE_EXACT(result->mo_name, &DeeString_Type);
			if (DeeString_FS_EQUALS_BUF(result->mo_name, name, size)) {
				Dee_Incref(result);
				break; /* Found it! */
			}
			result = LIST_NEXT(result, mo_globlink);
		}
	}
	modules_glob_lock_endread();
done:
	return (DREF DeeObject *)result;
}

/* Get a global module that has already been loaded, given its name.
 * If the module hasn't been loaded yet, NULL is returned.
 * NOTE: These functions never throw an error! */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_Get(DeeObject *__restrict module_name) {
	/* TODO: Support for mixed LATIN-1/UTF-8 strings */
	return DeeModule_DoGet(DeeString_STR(module_name),
	                       DeeString_SIZE(module_name),
	                       fs_hashobj(module_name));
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeModule_GetString(/*utf-8*/ char const *__restrict module_name,
                    size_t module_namesize) {
	return DeeModule_DoGet(module_name,
	                       module_namesize,
	                       fs_hashutf8(module_name, module_namesize));
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
	  (UNICODE_ISALPHA | UNICODE_ISLOWER | UNICODE_ISUPPER | UNICODE_ISTITLE | \
	   UNICODE_ISDIGIT | UNICODE_ISSYMSTRT | UNICODE_ISSYMCONT)) ||         \
	 ((ch) == '-' || (ch) == '=' || (ch) == ',' || (ch) == '(' ||          \
	  (ch) == ')' || (ch) == '[' || (ch) == ']' || (ch) == '{' ||          \
	  (ch) == '}' || (ch) == '<' || (ch) == '>' || (ch) == '+'))
#endif

#define Dee_MODULE_OPENINPATH_FNORMAL      0x0000 /* Normal flags */
#define Dee_MODULE_OPENINPATH_FRELMODULE   0x0001 /* The module name is relative */
#define Dee_MODULE_OPENINPATH_FTHROWERROR  0x0002 /* Throw an error if the module isn't found. */


#define SHEXT DeeSystem_SOEXT
#define SHLEN COMPILER_STRLEN(DeeSystem_SOEXT)


PRIVATE WUNUSED DREF DeeModuleObject *DCALL
DeeModule_OpenInPathAbs(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                        /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                        DeeObject *module_global_name,
                        struct compiler_options *options,
                        unsigned int mode) {
	DREF DeeStringObject *module_name_ob;
	DREF DeeStringObject *module_path_ob;
	DREF DeeModuleObject *result;
	char *buf, *dst;
	char const *module_name_start;
	size_t i, len;
	dhash_t hash;
	Dee_DPRINTF("[RT] Searching for %s%k in %$q as %$q\n",
	            module_global_name ? "global module " : STR_module,
	            module_global_name ? module_global_name : Dee_EmptyString,
	            module_pathsize, module_path,
	            module_namesize, module_name);
	{
		size_t buf_alloc;
#if !defined(CONFIG_NO_DEC) && !defined(CONFIG_NO_DEX)
		buf_alloc = module_pathsize + 1 + module_namesize + MAX_C((size_t)5, (size_t)SHLEN) + 1;
#elif !defined(CONFIG_NO_DEC)
		buf_alloc = module_pathsize + 1 + module_namesize + 5 + 1;
#elif !defined(CONFIG_NO_DEX)
		buf_alloc = module_pathsize + 1 + module_namesize + MAX_C((size_t)4, (size_t)SHLEN) + 1;
#else /* ... */
		buf_alloc = module_pathsize + 1 + module_namesize + 4 + 1;
#endif /* !... */
		buf = (char *)Dee_Mallocac(buf_alloc, sizeof(char));
	}
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
				if (dst > buf)
					--dst;
				continue;
			}
		}
		*dst++ = SEP;
	}
	if (dst > buf && dst[-1] != SEP)
		*dst++ = SEP;
	/* Step #1: Check for a cached variant of a user-script. */
	module_name_start = module_name;
	for (i = 0; i < module_namesize; ++i) {
		char ch = module_name[i];
		if (ch == '.') {
			if unlikely(module_name_start == module_name + i)
				goto err_bad_module_name; /* Don't allow multiple consecutive dots here! */
			ch                = SEP;
			module_name_start = module_name + i + 1;
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
	len  = (size_t)(dst - buf) + module_namesize + 4;
	hash = fs_hashutf8(buf, len);
again_search_fs_modules:

	/* Search for modules that have already been cached. */
	modules_lock_read();
	if (modules_a) {
		LIST_FOREACH (result, &modules_v[hash % modules_a], mo_link) {
			char const *utf8_path;
			if (fs_hashmodpath(result) != hash)
				continue;
			utf8_path = DeeString_TryAsUtf8((DeeObject *)result->mo_path);
			if unlikely(!utf8_path) {
				if (!Dee_IncrefIfNotZero(result))
					break;
				modules_lock_endread();
				utf8_path = DeeString_AsUtf8((DeeObject *)result->mo_path);
				if unlikely(!utf8_path)
					goto err_buf_r;
				if (WSTR_LENGTH(utf8_path) == len &&
				    /* TODO: Support for mixed LATIN-1/UTF-8 strings */
				    /* TODO: UTF-8 case compare! */
				    fs_bcmp(utf8_path, buf, len * sizeof(char)) == 0) {
					goto got_result_set_global;
				}
				Dee_Decref(result);
				goto again_search_fs_modules;
			}
			if (WSTR_LENGTH(utf8_path) != len)
				continue;
			/* TODO: Support for mixed LATIN-1/UTF-8 strings */
			/* TODO: UTF-8 case compare! */
			if (fs_bcmp(utf8_path, buf, len * sizeof(char)) != 0)
				continue;
			/* Found it! */
			if (!Dee_IncrefIfNotZero(result))
				break;
			modules_lock_endread();
got_result_set_global:
			if (module_global_name && likely(!LIST_ISBOUND(result, mo_globlink))) {
				DeeModuleObject *existing_module;
				/* Cache the module as global (if it wasn't already) */
again_find_existing_global_module:
				modules_glob_lock_write();
				COMPILER_READ_BARRIER();
				if likely(!LIST_ISBOUND(result, mo_globlink)) {
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
					 *       [const_if(mo_globlink != NULL)]
					 *       [lock_if(mo_globlink == NULL, INTERNAL(modules_glob_lock))]
					 */

					existing_module = find_glob_module(result->mo_name);
					if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
						modules_glob_lock_endwrite();
						Dee_Decref_likely(result);
						result = existing_module;
						goto got_result;
					}
					if (!add_glob_module(result)) {
						modules_glob_lock_endwrite();
						if (Dee_CollectMemory(1))
							goto again_find_existing_global_module;
						goto err_buf_r;
					}
				}
				modules_glob_lock_endwrite();
			}
			goto got_result;
		}
	}
	modules_lock_endread();
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
		memmoveupc(dst + 1,
		           dst,
		           module_namesize + 5,
		           sizeof(char));
		dst[0] = '.';
		ASSERT(dst[module_namesize + 1] == '.');
		ASSERT(dst[module_namesize + 2] == 'd');
		ASSERT(dst[module_namesize + 3] == 'e');
		dst[module_namesize + 4] = 'c';
		ASSERT(dst[module_namesize + 5] == '\0');
		{
			DREF DeeObject *dec_stream;
			dec_stream = DeeFile_OpenString(buf, OPEN_FRDONLY, 0);
			memmovedownc(dst,
			             dst + 1,
			             module_namesize + 5,
			             sizeof(char));
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
#ifdef DEE_SYSTEM_FS_ICASE
				result->mo_pathihash = hash;
#else /* DEE_SYSTEM_FS_ICASE */
				ASSERT(DeeString_Hash((DeeObject *)module_path_ob) == hash);
				DeeString_HASH(module_path_ob) = hash;
#endif /* !DEE_SYSTEM_FS_ICASE */

				result->mo_flags |= MODULE_FLOADING;
				COMPILER_WRITE_BARRIER();
				/* Cache the new module as part of the filesystem
				 * module cache, as well as the global module cache. */
				if (module_global_name) {
set_dec_file_module_global:
#ifndef CONFIG_NO_THREADS
					modules_lock_write();
					if (!modules_glob_lock_trywrite()) {
						modules_lock_endwrite();
						modules_glob_lock_write();
						if (!modules_lock_trywrite()) {
							modules_glob_lock_endwrite();
							goto set_dec_file_module_global;
						}
					}
#endif /* !CONFIG_NO_THREADS */
					existing_module = find_file_module(module_path_ob, hash);
					if likely(!existing_module)
						existing_module = find_glob_module(module_name_ob);
					if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
						modules_glob_lock_endwrite();
						modules_lock_endwrite();
						Dee_DecrefDokill(result);
						Dee_Decref_likely(dec_stream);
						result = existing_module;
						goto try_load_module_after_dec_failure;
					}
					/* Add the module to the file-cache. */
					if ((modules_c >= modules_a && !rehash_file_modules()) ||
					    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
						modules_lock_endwrite();
						/* Try to collect some memory, then try again. */
						if (Dee_CollectMemory(1))
							goto set_dec_file_module_global;
						Dee_Decref_likely(dec_stream);
						goto err_buf_r;
					}
					add_glob_module(result);
					add_file_module(result);
					modules_glob_lock_endwrite();
					modules_lock_endwrite();
				} else {
set_dec_file_module:
					modules_lock_write();
					existing_module = find_file_module(module_path_ob, hash);
					if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
						modules_lock_endwrite();
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
						modules_lock_endwrite();
						/* Try to collect some memory, then try again. */
						if (Dee_CollectMemory(1))
							goto set_dec_file_module;
						Dee_Decref_likely(dec_stream);
						goto err_buf_r;
					}
					modules_lock_endwrite();
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
	module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
	if unlikely(!module_path_ob)
		goto err_buf;
#ifndef DEE_SYSTEM_FS_ICASE
	ASSERT(fs_hashutf8(buf, len) == hash);
	DeeString_HASH(module_path_ob) = hash;
#endif /* !DEE_SYSTEM_FS_ICASE */
#else /* CONFIG_NO_DEX */
	/* Try to load the module from a DEX extension. */
	ASSERT(dst[module_namesize + 0] == '.');
	ASSERT(dst[module_namesize + 1] == 'd');
	ASSERT(dst[module_namesize + 2] == 'e');
	ASSERT(dst[module_namesize + 3] == 'e');
	ASSERT(dst[module_namesize + 4] == '\0');
	{
		void *dex_handle;
		if (SHLEN >= 0 && SHEXT[0] != '.')
			dst[module_namesize + 0] = SHEXT[0];
		if (SHLEN >= 1 && SHEXT[1] != 'd')
			dst[module_namesize + 1] = SHEXT[1];
		if (SHLEN >= 2 && SHEXT[2] != 'e')
			dst[module_namesize + 2] = SHEXT[2];
		if (SHLEN >= 3 && SHEXT[3] != 'e')
			dst[module_namesize + 3] = SHEXT[3];
		__STATIC_IF (SHLEN > 4) {
			memcpyc(&dst[module_namesize + 4],
			        SHEXT + 4,
			        (SHLEN - 4) + 1,
			        sizeof(char));
		}
		dex_handle = DeeSystem_DlOpenString(buf);
		if (dex_handle == DEESYSTEM_DLOPEN_FAILED) {
			if (SHLEN >= 0 && SHEXT[0] != '.')
				dst[module_namesize + 0] = '.';
			if (SHLEN >= 1 && SHEXT[1] != 'd')
				dst[module_namesize + 1] = 'd';
			if (SHLEN >= 2 && SHEXT[2] != 'e')
				dst[module_namesize + 2] = 'e';
			if (SHLEN >= 3 && SHEXT[3] != 'e')
				dst[module_namesize + 3] = 'e';
			__STATIC_IF (SHLEN > 4) {
				dst[module_namesize + 4] = '\0';
			}
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf, len, STRING_ERROR_FSTRICT);
			if unlikely(!module_path_ob)
				goto err_buf;
#ifndef DEE_SYSTEM_FS_ICASE
			ASSERT(fs_hashutf8(buf, len) == hash);
			DeeString_HASH(module_path_ob) = hash;
#endif /* !DEE_SYSTEM_FS_ICASE */
		} else {
			int error;
			DeeModuleObject *existing_module;
			module_path_ob = (DREF DeeStringObject *)DeeString_NewUtf8(buf,
			                                                           len - 4 + SHLEN,
			                                                           STRING_ERROR_FSTRICT);
			if unlikely(!module_path_ob) {
				DeeSystem_DlClose(dex_handle);
				goto err_buf_module_name;
			}
			result = (DREF DeeModuleObject *)DeeDex_New((DeeObject *)module_name_ob);
			if unlikely(!result) {
				DeeSystem_DlClose(dex_handle);
				goto err_buf_module_name_path;
			}
			Dee_Decref_unlikely(module_name_ob);
			result->mo_path = module_path_ob; /* Inherit reference. */
#ifdef DEE_SYSTEM_FS_ICASE
			result->mo_pathihash = hash;
#else /* DEE_SYSTEM_FS_ICASE */
			/* Load the updated path hash (and also force the hash to be pre-cached) */
			hash = fs_hashobj(module_path_ob);
#endif /* !DEE_SYSTEM_FS_ICASE */
			result->mo_flags |= MODULE_FLOADING;
			COMPILER_WRITE_BARRIER();

			/* Register the new dex module globally. */
			if (module_global_name) {
set_dex_file_module_global:
#ifndef CONFIG_NO_THREADS
				modules_lock_write();
				if (!modules_glob_lock_trywrite()) {
					modules_lock_endwrite();
					modules_glob_lock_write();
					if (!modules_lock_trywrite()) {
						modules_glob_lock_endwrite();
						goto set_dex_file_module_global;
					}
				}
#endif /* !CONFIG_NO_THREADS */
				existing_module = find_file_module(module_path_ob, hash);
				if likely(!existing_module)
					existing_module = find_glob_module(module_name_ob);
				if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
					modules_glob_lock_endwrite();
					modules_lock_endwrite();
					Dee_DecrefDokill(result);
					DeeSystem_DlClose(dex_handle);
					result = existing_module;
					goto try_load_module_after_dex_failure;
				}
				/* Add the module to the file-cache. */
				if ((modules_c >= modules_a && !rehash_file_modules()) ||
				    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
					modules_lock_endwrite();
					/* Try to collect some memory, then try again. */
					if (Dee_CollectMemory(1))
						goto set_dex_file_module_global;
					DeeSystem_DlClose(dex_handle);
					goto err_buf_r;
				}
				add_glob_module(result);
				add_file_module(result);
				modules_glob_lock_endwrite();
				modules_lock_endwrite();
			} else {
set_dex_file_module:
				modules_lock_write();
				existing_module = find_file_module(module_path_ob, hash);
				if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
					modules_lock_endwrite();
					Dee_DecrefDokill(result);
					DeeSystem_DlClose(dex_handle);
					result = existing_module;
try_load_module_after_dex_failure:
					if (DeeModule_BeginLoading(result) == 0)
						goto load_module_after_dex_failure;
					goto got_result;
				}
				/* Add the module to the file-cache. */
				if unlikely(!add_file_module(result)) {
					modules_lock_endwrite();
					/* Try to collect some memory, then try again. */
					if (Dee_CollectMemory(1))
						goto set_dex_file_module;
					DeeSystem_DlClose(dex_handle);
					goto err_buf_r;
				}
				modules_lock_endwrite();
			}
load_module_after_dex_failure:
			error = dex_load_handle((DeeDexObject *)result,
			                        (void *)dex_handle,
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
				err_file_not_found((DeeObject *)module_path_ob);
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
#ifdef DEE_SYSTEM_FS_ICASE
		result->mo_pathihash = hash;
#else /* DEE_SYSTEM_FS_ICASE */
		ASSERT(DeeString_HASHOK(module_path_ob));
		ASSERT(DeeString_HASH(module_path_ob) == hash);
#endif /* !DEE_SYSTEM_FS_ICASE */
		result->mo_flags |= MODULE_FLOADING;
		COMPILER_WRITE_BARRIER();

		/* Register the new dex module globally. */
		if (module_global_name) {
set_src_file_module_global:
#ifndef CONFIG_NO_THREADS
			modules_lock_write();
			if (!modules_glob_lock_trywrite()) {
				modules_lock_endwrite();
				modules_glob_lock_write();
				if (!modules_lock_trywrite()) {
					modules_glob_lock_endwrite();
					goto set_src_file_module_global;
				}
			}
#endif /* !CONFIG_NO_THREADS */
			existing_module = find_file_module(module_path_ob, hash);
			if likely(!existing_module)
				existing_module = find_glob_module(module_name_ob);
			if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
				modules_glob_lock_endwrite();
				modules_lock_endwrite();
				Dee_DecrefDokill(result);
				Dee_Decref_likely(source_stream);
				result = existing_module;
				goto try_load_module_after_src_failure;
			}
			/* Add the module to the file-cache. */
			if ((modules_c >= modules_a && !rehash_file_modules()) ||
			    (modules_glob_c >= modules_glob_a && !rehash_glob_modules())) {
				modules_lock_endwrite();
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(1))
					goto set_src_file_module_global;
				Dee_Decref_likely(source_stream);
				goto err_buf_r;
			}
			add_glob_module(result);
			add_file_module(result);
			modules_glob_lock_endwrite();
			modules_lock_endwrite();
		} else {
set_src_file_module:
			modules_lock_write();
			existing_module = find_file_module(module_path_ob, hash);
			if unlikely(existing_module && Dee_IncrefIfNotZero(existing_module)) {
				modules_lock_endwrite();
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
				modules_lock_endwrite();
				/* Try to collect some memory, then try again. */
				if (Dee_CollectMemory(1))
					goto set_src_file_module;
				Dee_Decref_likely(source_stream);
				goto err_buf_r;
			}
			modules_lock_endwrite();
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
	Dee_Freea(buf);
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
	Dee_Freea(buf);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeModuleObject *DCALL
DeeModule_SubOpenInPathAbs(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
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
	return DeeModule_OpenInPathAbs(module_path, module_pathsize,
	                               module_name, module_namesize,
	                               module_global_name,
	                               options,
	                               mode);
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
 * >> SEARCH_MODULE_FILESYSTEM_CACHE(joinpath(module_path, module_name + ".dee"));
 * >>#ifndef CONFIG_NO_DEC
 * >> TRY_LOAD_DEC_FILE(joinpath(module_path, "." + module_name + ".dec"));
 * >>#endif // !CONFIG_NO_DEC
 * >>#ifndef CONFIG_NO_DEX
 * >> TRY_LOAD_DEX_LIBRARY(joinpath(module_path, module_name + DeeSystem_SOEXT));
 * >>#endif // !CONFIG_NO_DEX
 * >> TRY_LOAD_SOURCE_FILE(joinpath(module_path, module_name + ".dee"));
 * EXAMPLES:
 * >> char const *path = "/usr/lib/deemon/lib";
 * >> char const *name = "util";
 * >> // Opens:
 * >> //   - /usr/lib/deemon/lib/
 * >> DeeModule_OpenInPath(path, strlen(path),
 * >>                      name, strlen(name),
 * >>                      NULL, NULL,
 * >>                      Dee_MODULE_OPENINPATH_FTHROWERROR);
 * @return: * :        The module that was imported.
 * @return: ITER_DONE: The module could not be found (only when `Dee_MODULE_OPENINPATH_FTHROWERROR' isn't set)
 * @return: NULL:      An error occurred. */
PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeModule_OpenInPath(/*utf-8*/ char const *__restrict module_path, size_t module_pathsize,
                     /*utf-8*/ char const *__restrict module_name, size_t module_namesize,
                     DeeObject *module_global_name,
                     struct compiler_options *options,
                     unsigned int mode) {
	if unlikely(!DeeSystem_IsAbsN(module_path, module_pathsize)) {
		/* Must make the given module path absolute. */
		DREF DeeStringObject *abs_path; /*utf-8*/
		char const *abs_utf8;
		DREF DeeModuleObject *result;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (DeeSystem_PrintPwd(&printer, true) < 0)
			goto err_printer;
		if (unicode_printer_print(&printer, module_path, module_pathsize) < 0)
			goto err_printer;
		abs_path = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		if unlikely(!abs_path)
			goto err;
		abs_utf8 = DeeString_AsUtf8((DeeObject *)abs_path);
		if unlikely(!abs_utf8)
			goto err_abs_path;
		result = DeeModule_SubOpenInPathAbs(abs_utf8,
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
	return (DREF DeeObject *)DeeModule_SubOpenInPathAbs(module_path, module_pathsize,
	                                                    module_name, module_namesize,
	                                                    module_global_name,
	                                                    options,
	                                                    mode);
err:
	return NULL;
}


/* Open a module, given its name in the global module namespace.
 * Global modules use their own cache that differs from the cache
 * used to unify modules through use of their filename.
 * NOTES:
 *   - Global module names are the raw filenames of modules,
 *     excluding an absolute path prefix or extension suffix.
 *   - When searching for global modules, each string from `DeeModule_GetPath()'
 *     is prepended in ascending order until an existing file is found.
 *   - Using this function, dex extensions and `.dec' (DEeemonCompiled) files
 *     can also be opened in addition to `.dee' (source) files, as well
 *     as the deemon's builtin module when `deemon' is passed as `module_name'.
 *   - Rather than using '/' or '\\' to identify separators between folders, you must instead use `.'
 *   - If `module_name' contains any whitespace or punctuation characters,
 *     this function will fail with an `Error.ValueError'.
 *   - If the host's filesystem is case-insensitive, then module
 *     names may be case-insensitive as well. However if this is
 *     the case, the following must always be true for any module:
 *     >> import Object from deemon;
 *     >> import mymodule;
 *     >> import MyModule;
 *     >> assert mymodule === MyModule;
 *     Note that this code example must only work when the module
 *     system is case-insensitive as well.
 * @param: throw_error: When true, throw an error if the module couldn't be
 *                      found and return `NULL', otherwise return `ITER_DONE'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
	if (DeeString_SIZE(module_name) == 6 &&
	    DeeString_Hash(module_name) == HASHOF_str_deemon &&
	    bcmpc(DeeString_STR(module_name), STR_deemon, 6, sizeof(char)) == 0) {
		/* Yes, it is. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}

	/* Search for a cache entry for this module in the global module cache. */
	modules_glob_lock_read();
	result = find_glob_module((DeeStringObject *)module_name);
	if (result && Dee_IncrefIfNotZero(result)) {
		modules_glob_lock_endread();
		goto done;
	}
	modules_glob_lock_endread();

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
	DREF DeeObject *module_name_ob;
	DREF DeeObject *path;
	DREF DeeModuleObject *result;
	DeeListObject *paths;
	size_t i;

	/* First off: Check if this is a request for the builtin `deemon' module.
	 * NOTE: This check is always done in case-sensitive mode! */
	if (module_namesize == 6 &&
	    bcmpc(module_name, STR_deemon, module_namesize, sizeof(char)) == 0) {
		/* Yes, it is. */
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		goto done;
	}

	/* Search for a cache entry for this module in the global module cache. */
	modules_glob_lock_read();
	result = find_glob_module_str(module_name, module_namesize);
	if (result && Dee_IncrefIfNotZero(result)) {
		modules_glob_lock_endread();
		goto done;
	}
	modules_glob_lock_endread();

	/* Construct the module name object. */
	module_name_ob = DeeString_NewUtf8(module_name,
	                                   module_namesize,
	                                   STRING_ERROR_FSTRICT);
	if unlikely(!module_name_ob)
		goto err;

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
				goto err_path_module_name_ob;
			result = (DREF DeeModuleObject *)DeeModule_OpenInPath(path_str,
			                                                      WSTR_LENGTH(path_str),
			                                                      module_name,
			                                                      module_namesize,
			                                                      module_name_ob,
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
	if (!throw_error) {
		Dee_Decref(module_name_ob);
		return ITER_DONE;
	}
	err_module_not_found(module_name_ob);
err_module_name_ob:
	Dee_Decref(module_name_ob);
err:
	return NULL;
err_path_module_name_ob:
	Dee_Decref(path);
	goto err_module_name_ob;
done_path:
	Dee_Decref(path);
	Dee_Decref(module_name_ob);
done:
	return (DREF DeeObject *)result;
}


/* Same as `DeeModule_OpenGlobal()', but automatically call `DeeModule_RunInit()' on the returned module. */
PUBLIC WUNUSED NONNULL((1)) DREF /*Module*/ DeeObject *DCALL
DeeModule_ImportGlobal(/*String*/ DeeObject *__restrict module_name) {
	DREF DeeObject *result;
	result = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!result)
		goto err;
	if unlikely(DeeModule_RunInit(result) < 0)
		goto err_result;
	return result;
err_result:
	Dee_Decref(result);
err:
	return NULL;
}

PUBLIC WUNUSED DREF /*Module*/ DeeObject *DCALL
DeeModule_ImportGlobalString(/*utf-8*/ char const *__restrict module_name,
                             size_t module_namesize) {
	DREF DeeObject *result;
	result = DeeModule_OpenGlobalString(module_name, module_namesize, NULL, true);
	if unlikely(!result)
		goto err;
	if unlikely(DeeModule_RunInit(result) < 0)
		goto err_result;
	return result;
err_result:
	Dee_Decref(result);
err:
	return NULL;
}



/* Lookup an external symbol.
 * Convenience function (same as `DeeObject_GetAttr(DeeModule_OpenGlobal(...)+DeeModule_RunInit, ...)') */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExtern(/*String*/ DeeObject *__restrict module_name,
                    /*String*/ DeeObject *__restrict global_name) {
	DREF DeeObject *result;
	DREF DeeObject *extern_module;
	extern_module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_GetAttr(extern_module, global_name);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExternString(/*utf-8*/ char const *__restrict module_name,
                          /*utf-8*/ char const *__restrict global_name) {
	DREF DeeObject *result;
	DREF DeeObject *extern_module;
	extern_module = DeeModule_OpenGlobalString(module_name,
	                                           strlen(module_name),
	                                           NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_GetAttrString(extern_module, global_name);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

/* Helper wrapper for `DeeObject_Call(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExtern(/*String*/ DeeObject *__restrict module_name,
                     /*String*/ DeeObject *__restrict global_name,
                     size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeObject *extern_module;
	extern_module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_CallAttr(extern_module, global_name, argc, argv);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExternString(/*utf-8*/ char const *__restrict module_name,
                           /*utf-8*/ char const *__restrict global_name,
                           size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DREF DeeObject *extern_module;
	extern_module = DeeModule_OpenGlobalString(module_name,
	                                           strlen(module_name),
	                                           NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_CallAttrString(extern_module, global_name, argc, argv);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

/* Helper wrapper for `DeeObject_Callf(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternStringf(/*utf-8*/ char const *__restrict module_name,
                            /*utf-8*/ char const *__restrict global_name,
                            char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeModule_VCallExternStringf(module_name,
	                                      global_name,
	                                      format,
	                                      args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternStringf(/*utf-8*/ char const *__restrict module_name,
                             /*utf-8*/ char const *__restrict global_name,
                             char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	DREF DeeObject *extern_module;
	extern_module = DeeModule_OpenGlobalString(module_name,
	                                           strlen(module_name),
	                                           NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_VCallAttrStringf(extern_module, global_name, format, args);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternf(/*String*/ DeeObject *__restrict module_name,
                      /*String*/ DeeObject *__restrict global_name,
                      char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeModule_VCallExternf(module_name,
	                                global_name,
	                                format,
	                                args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternf(/*String*/ DeeObject *__restrict module_name,
                       /*String*/ DeeObject *__restrict global_name,
                       char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	DREF DeeObject *extern_module;
	extern_module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!extern_module)
		goto err;
	if unlikely(DeeModule_RunInit(extern_module) < 0)
		goto err_extern_module;
	result = DeeObject_VCallAttrf(extern_module, global_name, format, args);
	Dee_Decref(extern_module);
	return result;
err_extern_module:
	Dee_Decref(extern_module);
err:
	return NULL;
}




/* Open a module using a relative module name
 * `module_name' that is based off of `module_pathname'
 * NOTE: If the given `module_name' doesn't start with a `.'
 *       character, the given `module_pathname' is ignored and the
 *       call is identical to `DeeModule_OpenGlobal(module_name, options)'
 * HINT: The given `module_pathname' is merely prepended
 *       before the module's actual filename.
 * Example:
 * >> DeeModule_OpenRelative("..foo.bar", "src/scripts");  // `src/foo/bar.dee'
 * >> DeeModule_OpenRelative(".sys.types", ".");           // `./sys/types.dee'
 * >> DeeModule_OpenRelative("thread", "foo/bar");         // `${LIBPATH}/thread.dee'
 * NOTE: This function also tries to open DEX modules, as well as `.*.dec' files.
 * @param: throw_error: When true, throw an error if the module couldn't be
 *                      found and return `NULL', otherwise return `ITER_DONE'. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_OpenRelative(DeeObject *__restrict module_name,
                       /*utf-8*/ char const *__restrict module_pathname,
                       size_t module_pathsize,
                       struct compiler_options *options,
                       bool throw_error) {
	/*utf-8*/ char const *module_name_str;
	module_name_str = DeeString_AsUtf8(module_name);
	if unlikely(!module_name_str)
		goto err;
	if (module_name_str[0] != '.')
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

PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
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

/* Import a module, with relative module paths being imported in relation to `basemodule'
 * Not that these functions invoke `DeeModule_RunInit()' on the returned module!*/
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_ImportRel(/*Module*/ DeeObject *__restrict basemodule,
                    /*String*/ DeeObject *__restrict module_name) {
	DREF DeeObject *result;
	DeeStringObject *path;
	char const *begin, *end;
	ASSERT_OBJECT_TYPE(basemodule, &DeeModule_Type);
	/* Load the path of the currently executing code (for relative imports). */
	path = ((DeeModuleObject *)basemodule)->mo_path;
	if unlikely(!path) {
		result = DeeModule_OpenGlobal(module_name, NULL, true);
	} else {
		ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
		begin = DeeString_AsUtf8((DeeObject *)path);
		if unlikely(!begin)
			goto err;
		end = begin + WSTR_LENGTH(begin);

		/* Find the end of the current path. */
		while (begin < end && !ISSEP(end[-1]))
			--end;
		result = DeeModule_OpenRelative(module_name,
		                                begin,
		                                (size_t)(end - begin),
		                                NULL,
		                                true);
	}
	if likely(result) {
		if unlikely(DeeModule_RunInit(result) < 0)
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_ImportRelString(/*Module*/ DeeObject *__restrict basemodule,
                          /*utf-8*/ char const *__restrict module_name,
                          size_t module_namesize) {
	DREF DeeObject *result;
	DeeStringObject *path;
	char const *begin, *end;
	ASSERT_OBJECT_TYPE(basemodule, &DeeModule_Type);

	/* Load the path of the currently executing code (for relative imports). */
	path = ((DeeModuleObject *)basemodule)->mo_path;
	if unlikely(!path) {
		result = DeeModule_OpenGlobalString(module_name, module_namesize, NULL, true);
	} else {
		ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
		begin = DeeString_AsUtf8((DeeObject *)path);
		if unlikely(!begin)
			goto err;
		end = begin + WSTR_LENGTH(begin);

		/* Find the end of the current path. */
		while (begin < end && !ISSEP(end[-1]))
			--end;
		result = DeeModule_OpenRelativeString(module_name,
		                                      module_namesize,
		                                      begin,
		                                      (size_t)(end - begin),
		                                      NULL,
		                                      true);
	}
	if likely(result) {
		if unlikely(DeeModule_RunInit(result) < 0)
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


PRIVATE WUNUSED int DCALL module_rehash_globals(void) {
	size_t i, new_mask = (current_rootscope->rs_bucketm << 1) | 1;
	struct module_symbol *new_vec;
	ASSERT(!(new_mask & (new_mask + 1)));
	new_vec = (struct module_symbol *)Dee_Callocc(new_mask + 1,
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
		for (;; MODULE_HASHNX(j, perturb)) {
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

struct module_import_data {
	DeeModuleObject *mid_self;    /* [1..1] The module into which to import. */
	unsigned int     mid_mode;    /* Import mode (s.a. `DEE_EXEC_RUNMODE_*') */
	uint16_t         mid_globala; /* Allocated vector size of `mid_self->mo_globalv' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
module_import_symbol(void *arg, DeeObject *name, DeeObject *value) {
	struct module_import_data *data = (struct module_import_data *)arg;
	DeeModuleObject *self = data->mid_self;
	if unlikely(DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (!(data->mid_mode & DEE_EXEC_RUNMODE_FDEFAULTS_ARE_GLOBALS)) {
		struct TPPKeyword *kwd;
		struct symbol *sym;

		/* Define as local constants. */
		kwd = TPPLexer_LookupKeyword(DeeString_STR(name),
		                             DeeString_SIZE(name),
		                             1);
		if unlikely(!kwd)
			goto err;
		sym = get_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd);
		if unlikely(sym) {
			if (sym->s_type == SYMBOL_TYPE_CONST &&
			    sym->s_const == value)
				return 0; /* Already defined. */
			DeeError_Throwf(&DeeError_KeyError,
			                "Default value for %r has already been defined",
			                name);
			goto err;
		}
		sym = new_local_symbol_in_scope((DeeScopeObject *)current_rootscope, kwd, NULL);
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
		if (self->mo_globalc >= data->mid_globala) {
			DREF DeeObject **new_globalv;
			uint16_t new_globala = data->mid_globala * 2;
			if (!new_globala)
				new_globala = 2;
			ASSERT(new_globala > self->mo_globalc);
			new_globalv = (DREF DeeObject **)Dee_TryReallocc(self->mo_globalv,
			                                                 new_globala,
			                                                 sizeof(DREF DeeObject *));
			if unlikely(!new_globalv) {
				new_globala = self->mo_globalc + 1;
				new_globalv = (DREF DeeObject **)Dee_Reallocc(self->mo_globalv,
				                                              new_globala,
				                                              sizeof(DREF DeeObject *));
				if unlikely(!new_globalv)
					goto err;
			}
			self->mo_globalv  = new_globalv;
			data->mid_globala = new_globala;
		}

		/* Append the symbol initializer */
		addr = self->mo_globalc++;
		self->mo_globalv[addr] = value;
		Dee_Incref(value);

		/* Insert the new object into the symbol table. */
		hash    = DeeString_Hash((DeeObject *)name);
		perturb = i = MODULE_HASHST(self, hash);
		for (;; MODULE_HASHNX(i, perturb)) {
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

PRIVATE WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
module_import_symbols(DeeModuleObject *self,
                      DeeObject *default_symbols,
                      unsigned int mode,
                      uint16_t *__restrict p_globala) {
	dssize_t result;
	struct module_import_data mid;
	mid.mid_self    = self;
	mid.mid_mode    = mode;
	mid.mid_globala = *p_globala;
	result = DeeObject_ForeachPair(default_symbols, &module_import_symbol, &mid);
	*p_globala = mid.mid_globala;
	return result;
}


/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executes the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20' */
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
			char const *name = DeeString_STR(source_pathname);
			size_t size = DeeString_SIZE(source_pathname);
			char const *name_end, *name_start;
			name_end   = name + size;
			name_start = DeeSystem_BaseName(name, size);

			/* Get rid of a file extension in the module name. */
			while (name_end > name_start && name_end[-1] != '.')
				--name_end;
			while (name_end > name_start && name_end[-1] == '.')
				--name_end;
			if (name_end == name_start)
				name_end = name + size;
			module_name = DeeString_NewSized(name_start,
			                                 (size_t)(name_end - name_start));
			if unlikely(!module_name)
				goto err;
		} else {
			module_name = DeeString_NewEmpty();
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
	Dee_atomic_rwlock_cinit(&result->mo_lock);
	DeeObject_Init(result, &DeeModule_Type);
	weakref_support_init(result);
	result = (DREF DeeModuleObject *)DeeGC_Track((DREF DeeObject *)result);
	result->mo_flags = MODULE_FLOADING;
#ifndef CONFIG_NO_THREADS
	result->mo_loader = DeeThread_Self();
#endif /* !CONFIG_NO_THREADS */
	compiler = DeeCompiler_New((DeeObject *)result,
	                           options ? options->co_compiler : COMPILER_FNORMAL);
	if unlikely(!compiler)
		goto err_r;
	if (COMPILER_BEGIN(compiler))
		goto err_r_compiler_not_locked;
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
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_r_compiler;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		dots->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
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
		if unlikely(module_import_symbols(result, default_symbols, mode, &result_globala) < 0)
			goto err_r_compiler;
		current_rootscope->rs_globalc = result->mo_globalc;
	}

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if unlikely(yield() < 0) {
		code = NULL;
	} else {
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
		new_vector = (DREF DeeModuleObject **)Dee_TryReallocc(current_rootscope->rs_importv,
		                                                      current_rootscope->rs_importc,
		                                                      sizeof(DREF DeeModuleObject *));
		if likely(new_vector)
			current_rootscope->rs_importv = new_vector;
	}
	if (!result->mo_globalv) {
		result->mo_globalv = (DREF DeeObject **)Dee_Callocc(current_rootscope->rs_globalc,
		                                                    sizeof(DREF DeeObject *));
		if unlikely(!result->mo_globalv)
			goto err_r_compiler;
	} else {
		if (current_rootscope->rs_globalc > result_globala) {
			DREF DeeObject **final_globalv;
			final_globalv = (DREF DeeObject **)Dee_Reallocc(result->mo_globalv,
			                                                current_rootscope->rs_globalc,
			                                                sizeof(DREF DeeObject *));
			if unlikely(!final_globalv)
				goto err_r_compiler;
			result->mo_globalv = final_globalv;
		}
		bzeroc(result->mo_globalv + result->mo_globalc,
		       current_rootscope->rs_globalc - result->mo_globalc,
		       sizeof(DREF DeeObject *));
	}
	result->mo_globalc = current_rootscope->rs_globalc;
	result->mo_importc = current_rootscope->rs_importc;
	atomic_or(&result->mo_flags, current_rootscope->rs_flags);
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
	atomic_or(&result->mo_flags, MODULE_FDIDLOAD);
	return (DREF DeeObject *)result;
err_r_compiler_code:
	ast_xdecref(code);
err_r_compiler:
	COMPILER_END();
err_r_compiler_not_locked:
	Dee_Decref(compiler);
err_r:
	atomic_and(&result->mo_flags, ~(MODULE_FLOADING));
	Dee_Decref_likely(result);
	goto err;
err_module_name:
	Dee_Decref(module_name);
err:
	return NULL;
}





/* List of strings that should be used as base paths when searching for global modules.
 * Access to this list should go through `DeeModule_InitPath()', which will
 * automatically initialize the list to the following default contents upon access:
 *
 * >> Commandline: `-L...' where every occurrance is pre-pended before the home-path.
 *    NOTE: These paths are not added by `DeeModule_InitPath()', but instead
 *          the first encouter of a -L option will call `DeeModule_GetPath()'
 *          before pre-pending the following string at the front of the list,
 *          following other -L paths prepended before then.
 * >> posix.environ.get("DEEMON_PATH", "").split(posix.FS_DELIM)...;
 * >> posix.joinpath(DeeExec_GetHome(), "lib")
 *
 * This list is also used to locate system-include paths for the preprocessor,
 * in that every entry that is a string is an include-path after appending "/include":
 * >> function get_include_paths(): {string...} {
 * >>     for (local x: DeeModule_Path)
 * >>         if (x is string)
 * >>             yield f"{x.rstrip("/")}/include";
 * >> } */
PUBLIC DeeListObject DeeModule_Path = {
	OBJECT_HEAD_INIT(&DeeList_Type),
	/* .l_list = */ DEE_OBJECTLIST_INIT
#ifndef CONFIG_NO_THREADS
	,
	/* .l_lock = */ DEE_ATOMIC_RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
};



/* Figure out how to implement `get_default_home()' */
#undef get_default_home_USE_CONFIG_DEEMON_HOME
#undef get_default_home_USE_GetModuleFileNameW
#undef get_default_home_USE_readlink_proc_self_exe
#ifdef CONFIG_DEEMON_HOME
#define get_default_home_USE_CONFIG_DEEMON_HOME
#elif defined(CONFIG_HAVE_dlmodulename) && defined(CONFIG_HAVE_dlopen)
#define get_default_home_USE_dlmodulename
#elif defined(CONFIG_HOST_WINDOWS)
#define get_default_home_USE_GetModuleFileNameW
#elif defined(CONFIG_HOST_UNIX)
#define get_default_home_USE_readlink_proc_self_exe
#else /* ... */
#define get_default_home_USE_readlink_proc_self_exe
#endif /* !... */

#ifdef get_default_home_USE_dlmodulename
#ifndef DLOPEN_NULL_FLAGS
#if defined(CONFIG_HAVE_RTLD_GLOBAL)
#define DLOPEN_NULL_FLAGS RTLD_GLOBAL
#elif defined(CONFIG_HAVE_RTLD_LOCAL)
#define DLOPEN_NULL_FLAGS RTLD_LOCAL
#else /* ... */
#define DLOPEN_NULL_FLAGS 0
#endif /* !... */
#endif /* !DLOPEN_NULL_FLAGS */
#endif /* get_default_home_USE_dlmodulename */


#ifdef get_default_home_USE_CONFIG_DEEMON_HOME
PRIVATE DEFINE_STRING(default_deemon_home, CONFIG_DEEMON_HOME);
#endif /* get_default_home_USE_CONFIG_DEEMON_HOME */



PRIVATE WUNUSED DREF /*String*/ DeeStringObject *DCALL get_default_home(void) {
#ifndef CONFIG_NO_DEEMON_HOME_ENVIRON
	char const *env;
#ifndef CONFIG_DEEMON_HOME_ENVIRON
#define CONFIG_DEEMON_HOME_ENVIRON "DEEMON_HOME"
#endif /* !CONFIG_DEEMON_HOME_ENVIRON */
	DBG_ALIGNMENT_DISABLE();
	env = getenv(CONFIG_DEEMON_HOME_ENVIRON); /* TODO: system-feature test */
	DBG_ALIGNMENT_ENABLE();
	if (env) {
		DREF DeeStringObject *result;
		size_t len = strlen(env);
		if (len) {
			DREF DeeStringObject *new_result;
			while (len && ISSEP(env[len - 1]))
				--len;
			result = (DREF DeeStringObject *)DeeString_NewUtf8(env, len + 1,
			                                                   STRING_ERROR_FIGNORE);
#define get_default_home_NEED_ERR 1
			if unlikely(!result)
				goto err;
			DeeString_SetChar(result, len - 1, SEP);
			new_result = (DREF DeeStringObject *)DeeSystem_MakeAbsolute((DeeObject *)result);
			Dee_Decref(result);
			return new_result;
		}
	}
#endif /* !CONFIG_NO_DEEMON_HOME_ENVIRON */

#ifdef get_default_home_USE_CONFIG_DEEMON_HOME
#define get_default_home_NO_FALLBACK 1
	return_reference_((DeeStringObject *)&default_deemon_home);
#endif /* get_default_home_USE_CONFIG_DEEMON_HOME */

#ifdef get_default_home_USE_GetModuleFileNameW
	{
		DREF DeeStringObject *result;
		DWORD dwBufSize = PATH_MAX, dwError;
		LPWSTR lpBuffer, lpNewBuffer;
		DREF DeeStringObject *new_result;
		lpBuffer = DeeString_NewWideBuffer(dwBufSize);
#define get_default_home_NEED_ERR 1
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
				if (DeeNTSystem_IsIntr(dwError))
					goto again_chk_intr;
				if (DeeNTSystem_IsBufferTooSmall(dwError))
					goto do_increase_buffer;
				DeeString_FreeWideBuffer(lpBuffer);
				goto fallback;
			}
			DBG_ALIGNMENT_ENABLE();
			if (dwError <= dwBufSize) {
				if (dwError < dwBufSize)
					break;
				DBG_ALIGNMENT_DISABLE();
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (!DeeNTSystem_IsBufferTooSmall(dwError))
					break;
			}
			/* Increase buffer size. */
do_increase_buffer:
			dwBufSize *= 2;
			lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwBufSize);
			if unlikely(!lpNewBuffer)
				goto err_buffer;
			lpBuffer = lpNewBuffer;
		}
		/* TODO: Check if the module's file name is a symbolic link.
		 *       If it turns out to be one, follow it! */

		/* Trim the trailing module filename, but keep 1 trailing slash. */
		while (dwError && !DeeSystem_IsSep(lpBuffer[dwError - 1]))
			--dwError;
		while (dwError && DeeSystem_IsSep(lpBuffer[dwError - 1]))
			--dwError;
		lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError + 1);
		result = (DREF DeeStringObject *)DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
		if unlikely(!result)
			goto err;
		new_result = (DREF DeeStringObject *)DeeSystem_MakeAbsolute((DeeObject *)result);
		Dee_Decref(result);
		return new_result;
	}
#endif /* get_default_home_USE_GetModuleFileNameW */

#ifdef get_default_home_USE_dlmodulename
	{
		size_t length;
		/* dlopen(NULL)   -> Return a handle to the primary binary
		 *                   This is actually behavior that is mandated by POSIX! */
		void *hProc = dlopen(NULL, DLOPEN_NULL_FLAGS);
		/* dlmodulename() -> KOS extension that returns a module's absolute filename.
		 *                   This one's really the perfect solution, since it doesn't
		 *                   involve any additional system calls being made, or making
		 *                   runtime assumptions such as /proc being mounted. */
		char const *filename = dlmodulename(hProc);
		if unlikely(!filename)
			goto fallback;
		length = strlen(filename);
		/* Trim the actual executable filename (which is likely to be `deemon'),
		 * thus getting the absolute path where the executable is placed (which
		 * is likely to be something along the lines of `/bin/' or `/usr/bin/') */
		while (length && filename[length - 1] != '/')
			--length;
		if (length && unlikely(filename[length - 1] == '/')) {
			/* Strip additional slashes, such that only a single one remains */
			while (length >= 2 && filename[length - 2] == '/')
				--length;
		}
		return (DREF DeeStringObject *)DeeString_NewUtf8(filename,
		                                                 length,
		                                                 STRING_ERROR_FIGNORE);
	}
#endif /* get_default_home_USE_dlmodulename */

#ifdef get_default_home_USE_readlink_proc_self_exe
	size_t length;
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeUnixSystem_PrintLinkString(&printer, "/proc/self/exe");
	if unlikely(error != 0) {
		if (error < 0)
			goto err_printer;
		/* Fallback... */
bad_path:
		unicode_printer_fini(&printer);
		goto fallback;
	}
	length = UNICODE_PRINTER_LENGTH(&printer);
	if unlikely(!length)
		goto bad_path;
	while (length && UNICODE_PRINTER_GETCHAR(&printer, length - 1) != '/')
		--length;
	if unlikely(!length)
		goto fallback;
	while (length && UNICODE_PRINTER_GETCHAR(&printer, length - 1) == '/')
		--length;
	unicode_printer_truncate(&printer, length + 1);
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* get_default_home_USE_readlink_proc_self_exe */

#ifndef get_default_home_NO_FALLBACK
fallback:

	/* TODO: Check if `main:argv[0]' is an absolute filename. */
	/* TODO: Check if `main:argv[0]' can be found in $PATH. */
	return (DREF DeeStringObject *)DeeString_New(".");
#endif /* !get_default_home_NO_FALLBACK */

#ifdef get_default_home_NEED_ERR
err:
	return NULL;
#endif /* get_default_home_NEED_ERR */
}


#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t deemon_home_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define deemon_home_lock_reading()    Dee_atomic_rwlock_reading(&deemon_home_lock)
#define deemon_home_lock_writing()    Dee_atomic_rwlock_writing(&deemon_home_lock)
#define deemon_home_lock_tryread()    Dee_atomic_rwlock_tryread(&deemon_home_lock)
#define deemon_home_lock_trywrite()   Dee_atomic_rwlock_trywrite(&deemon_home_lock)
#define deemon_home_lock_canread()    Dee_atomic_rwlock_canread(&deemon_home_lock)
#define deemon_home_lock_canwrite()   Dee_atomic_rwlock_canwrite(&deemon_home_lock)
#define deemon_home_lock_waitread()   Dee_atomic_rwlock_waitread(&deemon_home_lock)
#define deemon_home_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&deemon_home_lock)
#define deemon_home_lock_read()       Dee_atomic_rwlock_read(&deemon_home_lock)
#define deemon_home_lock_write()      Dee_atomic_rwlock_write(&deemon_home_lock)
#define deemon_home_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&deemon_home_lock)
#define deemon_home_lock_upgrade()    Dee_atomic_rwlock_upgrade(&deemon_home_lock)
#define deemon_home_lock_downgrade()  Dee_atomic_rwlock_downgrade(&deemon_home_lock)
#define deemon_home_lock_endwrite()   Dee_atomic_rwlock_endwrite(&deemon_home_lock)
#define deemon_home_lock_endread()    Dee_atomic_rwlock_endread(&deemon_home_lock)
#define deemon_home_lock_end()        Dee_atomic_rwlock_end(&deemon_home_lock)

PRIVATE DREF DeeStringObject *deemon_home = NULL;

/* Get/Set deemon's home path.
 * The home path is used to locate builtin libraries, as well as extensions.
 * Upon first access, `DeeExec_GetHome()' will pre-initialize the home-path as follows:
 * >> deemon_home = fs.environ["DEEMON_HOME"];
 * >> if (deemon_home !is none) {
 * >>     deemon_home = fs.abspath(deemon_home);
 * >> } else {
 * >>#ifdef CONFIG_DEEMON_HOME
 * >>     deemon_home = CONFIG_DEEMON_HOME;
 * >>#else
 * >>     deemon_home = fs.headof(fs.readlink("/proc/self/exe"));
 * >>#endif
 * >> }
 * >> deemon_home = fs.inctrail(deemon_home);
 * That is: Try to lookup an environment variable `DEEMON_HOME', which
 *          if found is then converted into an absolute filename.
 *          When this variable doesn't exist, behavior depends on how deemon was built.
 *          If it was built with the `CONFIG_DEEMON_HOME' option enabled, that
 *          option is interpreted as a string which is then used as the effective
 *          home path, but if that option was disabled, the folder of deemon's
 *          executable is used as home folder instead.
 * @return: NULL: Failed to determine the home folder (an error was set).
 * NOTE: The home path _MUST_ include a trailing slash! */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeExec_GetHome(void) {
	DREF DeeStringObject *result;
	deemon_home_lock_read();
	result = deemon_home;
	if (result) {
		Dee_Incref(result);
		deemon_home_lock_endread();
		return (DREF DeeObject *)result;
	}
	deemon_home_lock_endread();
	/* Re-create the default home path. */
	result = get_default_home();

	/* Save the generated path in the global variable. */
	if likely(result) {
		DREF DeeStringObject *other;
		deemon_home_lock_write();
		other = deemon_home;
		if unlikely(other) {
			Dee_Incref(other);
			deemon_home_lock_endwrite();
			Dee_Decref(result);
			return (DREF DeeObject *)other;
		}
		Dee_Incref(result);
		deemon_home = result;
		deemon_home_lock_endwrite();
	}
	return (DREF DeeObject *)result;
}

/* Set the new home folder, overwriting whatever was set before.
 * HINT: You may pass `NULL' to cause the default home path to be re-created. */
PUBLIC void DCALL
DeeExec_SetHome(/*String*/ DeeObject *new_home) {
	DREF DeeStringObject *old_home;
	ASSERT_OBJECT_TYPE_EXACT_OPT(new_home, &DeeString_Type);
	Dee_XIncref(new_home);
	deemon_home_lock_write();
	old_home    = deemon_home;
	deemon_home = (DREF DeeStringObject *)new_home;
	deemon_home_lock_endwrite();
	Dee_XDecref(old_home);
}


PRIVATE void DCALL do_init_module_path(void) {
	int error;
#ifndef CONFIG_NO_DEEMON_PATH_ENVIRON
	{
		char const *path;
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
				char const *next_path = strchr(path, DeeSystem_DELIM);
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
	}
#endif /* !CONFIG_NO_DEEMON_PATH_ENVIRON */
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
		if (atomic_cmpxch(&module_init_state, INIT_PENDING, INIT_PROGRES)) {
			do_init_module_path();
			atomic_write(&module_init_state, INIT_COMPLET);
		} else {
			while (atomic_read(&module_init_state) != INIT_COMPLET)
				SCHED_YIELD();
		}
#endif /* !CONFIG_NO_THREADS */
	}
}



DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_C */
