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
#ifndef GUARD_DEEMON_COMPILER_COMPILER_C
#define GUARD_DEEMON_COMPILER_COMPILER_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/code.h>               /* DeeCodeObject, Dee_CODE_FVARARGS */
#include <deemon/compiler/assembler.h> /* ASM_FNORMAL, asm_symbol_ref, code_compile, module_compile */
#include <deemon/compiler/ast.h>       /* AST_FMULTIPLE_KEEPLAST, ast, ast_* */
#include <deemon/compiler/compiler.h>  /* COMPILER_*, DeeCompiler*, DeeLexer_OfCompiler */
#include <deemon/compiler/error.h>     /* current_parser_errors, parser_* */
#include <deemon/compiler/lexer.h>     /* AST_COMMA_*, PARSE_FLFSTMT, PARSE_FNORMAL, ast_parse_*, ast_tags, current_tags, inner_compiler_options, parser_flags */
#include <deemon/compiler/optimize.h>  /* OPTIMIZE_FENABLED, OPTIMIZE_FNORMAL, ast_optimize_all, optimizer_flags, optimizer_unwind_limit */
#include <deemon/compiler/symbol.h>    /* CONFIG_SYMBOL_HAS_REFCNT, DAST_NONE, DeeRootScope_Type, DeeScopeObject, SYMBOL_FALLOC, SYMBOL_TYPE_ARG, current_basescope, current_rootscope, current_scope, new_unnamed_symbol, symbol */
#include <deemon/compiler/tpp.h>
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/dec.h>                /* DeeDecWriter, DeeDecWriter_*, DeeDec_* */
#include <deemon/exec.h>               /* DeeExec_RUNMODE_* */
#include <deemon/module.h>             /* DeeModule*, Dee_compiler_options, Dee_module_object */
#include <deemon/object.h>             /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_NewDefault, DeeObject_Type, DeeTypeObject, Dee_AsObject, Dee_Clear, Dee_Decref, Dee_Incref, Dee_WEAKREF_SUPPORT_ADDR, Dee_XClear, Dee_weakref_support_fini, Dee_weakref_support_init, OBJECT_HEAD_INIT */
#include <deemon/serial.h>             /* DeeSerial, Dee_serial */
#include <deemon/string.h>             /* DeeString_AsUtf8, DeeString_Type, Dee_unicode_printer_fini, WSTR_LENGTH */
#include <deemon/system-features.h>    /* bzero, memcpy*, memset */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, Dee_visit_t, TF_NONE, TP_FNORMAL, type_* */
#include <deemon/util/rlock.h>         /* Dee_rshared_rwlock_t */
#include <deemon/util/weakref.h>       /* Dee_WEAKREF, Dee_weakref_clear, Dee_weakref_set */

#include <hybrid/typecore.h> /* __REGISTER_TYPE__ */

#include <stdbool.h> /* false, true */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint8_t, uint16_t */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#if !defined(CONFIG_NO_THREADS) || 1 /* Always needs to be defined for binary compat */
PUBLIC Dee_rshared_rwlock_t DeeCompiler_Lock = Dee_RSHARED_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3

PUBLIC DeeCompiler *DeeCompiler_Current = NULL;

/* -------- Compiler Implementation -------- */

PUBLIC ATTR_COLD WUNUSED NONNULL((1)) tpp_errno TPPCALL
DeeCompiler_CliWarnf(DeeCompiler *__restrict self, tpp_char const *token_start,
                     tpp_size token_size, tpp_warning_id id, ...) {
	tpp_lexer *const lexer = &self->cp_lexer.dl_lexer;
	tpp_file *const file = tpp_lexer_getfile(lexer);
	tpp_errno result;
	va_list args;
	tpp_lexer_pushcore(lexer);
	tpp_file_init_text_utf8(file, TPP_CONFIG_CLI_FILENAME,
	                        NULL, token_start, token_size,
	                        TPP_LCINFO_INVALID, TPP_FILE_FLAGS_NORMAL);
	tpp_lexer_settokenrange(lexer, token_start, token_start + token_size);
	va_start(args, id);
	result = tpp_lexer_vwarnf(lexer, id, args);
	va_end(args);
	tpp_lexer_finifile(lexer);
	tpp_lexer_popcore(lexer);
	return result;
}


#define DeeCompilerCliLoader_SetOptimizationLevel(self, level) \
	DeeCompiler_SetOptimizationLevel((self)->ccl_compiler, level)
PRIVATE NONNULL((1)) void DCALL
DeeCompiler_SetOptimizationLevel(DeeCompiler *__restrict self, int level) {
	if (level > 4)
		level = 4;
	self->cp_optimizer_flags &= ~(OPTIMIZE_FENABLED | OPTIMIZE_FCSE |
	                              OPTIMIZE_FCONSTSYMS | OPTIMIZE_FNOUSESYMS
#ifdef OPTIMIZE_FASSUME
	                              | OPTIMIZE_FASSUME
#endif /* OPTIMIZE_FASSUME */
	                              );
	self->cp_assembler &= ~(ASM_FSTACKDISP | ASM_FPEEPHOLE | ASM_FOPTIMIZE | ASM_FREUSELOC | ASM_FNODDI);
	self->cp_unwind_limit = 0;
	if (level == -1) {
		/* Optimize for size. */
		self->cp_optimizer_flags |= (OPTIMIZE_FCSE | OPTIMIZE_FNOUSESYMS |
		                             OPTIMIZE_FENABLED | OPTIMIZE_FCONSTSYMS);
#if defined(OPTIMIZE_FASSUME) && 0
		self->cp_optimizer_flags |= OPTIMIZE_FASSUME;
#endif /* OPTIMIZE_FASSUME */
		self->cp_unwind_limit = 1; /* Only unwind loops with 0, or 1 iteration! */
		self->cp_assembler |= (ASM_FREUSELOC | ASM_FSTACKDISP |
		                       ASM_FPEEPHOLE | ASM_FOPTIMIZE |
		                       ASM_FOPTIMIZE_SIZE);
	} else {
		/* Level #4: Disable features that hinder optimization (i.e. debug info) */
		if (level >= 4) {
			self->cp_assembler |= ASM_FNODDI;
			self->cp_optimizer_flags |= (OPTIMIZE_FCSE |     /* CSE results in somewhat obscured DDI
			                                                  * info, so we only enable it at level#4 */
			                             OPTIMIZE_FNOUSESYMS /* Removing unused symbols obviously leads to those
			                                                  * symbols not showing up in DDI information, thus
			                                                  * resulting in those symbols also not showing up
			                                                  * in generated debug information, or assembly. */
#ifdef OPTIMIZE_FASSUME
			                             | OPTIMIZE_FASSUME /* Similar to `OPTIMIZE_FNOUSESYMS`, assumptions can lead
			                                                 * to symbols being turned into constants at unexpected
			                                                 * times, which is why we only enable them here.
			                                                 * Another reason is that they are quite expensive... */
#endif /* OPTIMIZE_FASSUME */
			                             );
			self->cp_unwind_limit = 4;
		}

		/* Level #3: Enable the AST-level optimization pass.
		 *        -> This is mainly where constant propagation is implemented,
		 *           among other, minor optimizations such as double-casts to
		 *           known types */
		if (level >= 3) {
			self->cp_optimizer_flags |= (OPTIMIZE_FENABLED |
			                             OPTIMIZE_FCONSTSYMS);
			self->cp_assembler |= (ASM_FREUSELOC);
		}

		/* Level #2: Enable initialization-is-allocation for __stack variable & peephole optimization.
		 *        -> Note that peephole also implements dead-code elimination, as well
		 *           as various other optimizations, such as elimination or variable
		 *           reads/writes, among other things.
		 *           However, peephole is greatly restricted by debug information where the
		 *           existence of DDI checkpoints prevents inter-opcode optimizations. */
		if (level >= 2) {
			self->cp_assembler |= (ASM_FSTACKDISP | ASM_FPEEPHOLE);
		}

		/* Level #1: Enable general assembly optimizations (mainly affects automatic
		 *           instruction width selection, used to minimize assembly size, as
		 *           well as rudimentary deletion of `adjstack #SP + 0` instructions). */
		if (level >= 1) {
			self->cp_assembler |= (ASM_FOPTIMIZE);
		}
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) tpp_errno DCALL
DeeCompilerCliLoader_SetOptimization(DeeCompilerCliLoader *__restrict self,
                                     char const *__restrict level) {
	if (strcmp(level, "s") == 0) {
		DeeCompilerCliLoader_SetOptimizationLevel(self, -1);
	} else {
		int intlevel;
		if (Dee_TAtoi(int, level, strlen(level), 0, &intlevel))
			goto err;
		DeeCompilerCliLoader_SetOptimizationLevel(self, intlevel);
	}
	return TPP_EOK;
err:
	return TPP_EDEEMON;
}


enum {
	_Dee_COMPILER_CLI_STATE_FIRST_INTERNAL = Dee_COMPILER_CLI_STATE_DDASH,
	Dee_COMPILER_CLI_STATE_OPTIMIZE, /* "--optimize" */
};

/* Parse arguments/flags
 * @return: TPP_EOK:     Success
 * @return: TPP_ENOENT:  Unknown argument/flag (try handling it elsewhere)
 * @return: TPP_EDEEMON: An error was thrown */
PUBLIC WUNUSED NONNULL((1, 2)) tpp_errno DCALL
DeeCompilerCliLoader_ParseArg(DeeCompilerCliLoader *__restrict self,
                              char const *arg) {
#define cli_streq(at, CONSTstr) \
	(bcmp(at, CONSTstr, sizeof(CONSTstr) - sizeof(char)) == 0)
	switch (self->ccl_state) {

	case Dee_COMPILER_CLI_STATE_NORMAL:
		switch (*arg++) {
		case '-': {
			char const *after_dash = arg;
			switch (*arg++) {
			case '\0':
				break;

			case '-':
				switch (*arg++) {
				case '\0':
					self->ccl_state = Dee_COMPILER_CLI_STATE_DDASH; /* -- */
					return TPP_EOK;

				case 'o': {
					if (cli_streq(arg, "ptimize=")) {
						arg += (sizeof("ptimize=") - sizeof(char));
						return DeeCompilerCliLoader_SetOptimization(self, arg);
					} else if (cli_streq(arg, "ptimize\0")) {
						self->ccl_state = Dee_COMPILER_CLI_STATE_OPTIMIZE;
						return TPP_EOK;
					}
				}	break;

				/* TODO: All missing CLI flags (see non-CONFIG_EXPERIMENTAL_USE_TPP3 code in main.c) */
				/* TODO: -Wp,... */
				/* TODO: -Wa,... */
				/* TODO: -Wl,... */

				default: break;
				}
				break;

			default: break;
			}
		}	break;

		default: break;
		}
		break;

	case Dee_COMPILER_CLI_STATE_DDASH:
		break; /* Don't accept any more arguments */

	case Dee_COMPILER_CLI_STATE_OPTIMIZE:
		self->ccl_state = Dee_COMPILER_CLI_STATE_NORMAL;
		return DeeCompilerCliLoader_SetOptimization(self, arg);

	default: __builtin_unreachable();
	}
#undef cli_streq
	return TPP_ENOENT;
}

PUBLIC WUNUSED NONNULL((1, 2)) tpp_errno DCALL
DeeCompilerCliLoader_ParseFlag(DeeCompilerCliLoader *__restrict self,
                               char const **p_arg) {
	char const *arg = *p_arg;
	switch (*arg++) {

	case 'O':
		if (*arg == 's') {
			DeeCompilerCliLoader_SetOptimizationLevel(self, -1);
			++arg;
			goto done;
		} else if (*arg >= '0' && *arg <= '9') {
			DeeCompilerCliLoader_SetOptimizationLevel(self, *arg - '0');
			++arg;
			goto done;
		}
		break;

	default: break;
	}
	return TPP_ENOENT;
done:
	*p_arg = arg;
	return TPP_EOK;
}

PUBLIC WUNUSED NONNULL((1)) tpp_errno DCALL
DeeCompilerCliLoader_Flush(DeeCompilerCliLoader *tpp_restrict self) {
	if (self->ccl_state != Dee_COMPILER_CLI_STATE_NORMAL &&
	    self->ccl_state != Dee_COMPILER_CLI_STATE_DDASH) {
		tpp_errno error = DeeCompiler_CliWarnf(self->ccl_compiler, NULL, 0,
		                                       TPP_W_MISSING_CLI_ARGUMENT);
		if (TPP_ISERR(error))
			return error;
	}
	return TPP_EOK;
}


struct compiler_cli_parser {
	DeeCompilerCliLoader    ccp_comp;   /* Compiler CLI loader */
	tpp_cli_loader          ccp_cli;    /* Lexer CLI loader */
	tpp_makefile_cli_loader ccp_cli_mf; /* Makefile CLI loader */
};

static WUNUSED NONNULL((1, 2)) tpp_errno DCALL
compiler_cli_parser_parsearg(struct compiler_cli_parser *tpp_restrict self,
                             char const *tpp_restrict arg) {
	tpp_errno error;
	/* Pass argument to respective sub-loaders
	 * if those aren't in their "default" state */
	if (!tpp_cli_loader_hasdefaultstate(&self->ccp_cli))
		return tpp_cli_loader_parsearg(&self->ccp_cli, arg);
	if (!tpp_makefile_cli_loader_hasdefaultstate(&self->ccp_cli_mf))
		return tpp_makefile_cli_loader_parsearg(&self->ccp_cli_mf, arg);

	/* Try to handle argument using different CLI loaders */
	error = DeeCompilerCliLoader_ParseArg(&self->ccp_comp, arg);
	if (error == TPP_ENOENT)
		error = tpp_cli_loader_parsearg(&self->ccp_cli, arg);
	if (error == TPP_ENOENT)
		error = tpp_makefile_cli_loader_parsearg(&self->ccp_cli_mf, arg);

	/* Try to handle argument as a set of flags */
	if (error == TPP_ENOENT && (arg[0] == '-' &&
	                            arg[1] != '-' &&
	                            arg[1] != '\0')) {
		char const *after_dash = arg + 1;
		while (*after_dash) {
			error = DeeCompilerCliLoader_ParseFlag(&self->ccp_comp, &after_dash);
			if (error == TPP_ENOENT)
				error = tpp_cli_loader_parseflag(&self->ccp_cli, &after_dash);
			if (error == TPP_ENOENT)
				error = tpp_makefile_cli_loader_parseflag(&self->ccp_cli_mf, &after_dash);
			if (TPP_ISERR(error))
				return error;
		}
	}
	return error;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) tpp_errno DCALL
DeeCompiler_ParseArgv(struct compiler_cli_parser *__restrict parser,
                      struct Dee_compiler_options const *__restrict options,
                      int *p_argc, char ***p_argv) {
	tpp_errno result = TPP_EOK;
	int argc    = *p_argc;
	char **argv = *p_argv;
	unsigned int unknown_count = 0;
	while (argc > 0) {
		char *arg = argv[0];
		result = compiler_cli_parser_parsearg(parser, arg);
		if (TPP_ISERR(result)) {
			if (result != TPP_ENOENT)
				break;
			/* Add "arg" to trailing list of unknown arguments */
			--argc;
			(void)tpp_memmovedown(&argv[0], &argv[1],
			                      (argc + unknown_count) *
			                      sizeof(char *));
			argv[argc + unknown_count] = arg;
			++unknown_count;
			result = TPP_EOK;
			continue;
		}
		if (parser->ccp_comp.ccl_state == Dee_COMPILER_CLI_STATE_DDASH) {
			if (unknown_count) {
				/* Right now, "argv" looks like this:
				 * >> argv = { "--", "file1.c", "file2.c", "-unknown-arg", "file0.c" }
				 * >> argc = 3
				 * >> unknown_count = 2
				 *
				 * Our job now is to make `argv` look like this:
				 * >> argv = { "-unknown-arg", "file0.c", "--", "file1.c", "file2.c" } */
				unsigned int shift_count = unknown_count;
				unsigned int total_count_minus_1 = argc + unknown_count - 1;
				while (shift_count--) {
					arg = argv[0];
					(void)tpp_memmovedown(&argv[0], &argv[1], total_count_minus_1 * sizeof(char *));
					argv[total_count_minus_1] = arg;
				}
			}
			break;
		}
		++argv;
		--argc;
	}
	*p_argc = argc + unknown_count;
	*p_argv = argv;
	return result;
}

#if TPP_HAVE_FILE_NOCLOSE
#define tpp_lexer_initfile_stream(self, options, source_stream, start_lc)      \
	tpp_lexer_initfile_io_ex2(self, (options) ? (options)->co_pathname : NULL, \
	                          source_stream, TPP_FILE_FLAGS_NOCLOSE,           \
	                          start_lc, TPP_FILE_ENCODING_UTF8)
#define tpp_lexer_pushfile_stream(self, options, source_stream, start_lc)      \
	tpp_lexer_pushfile_io_ex2(self, (options) ? (options)->co_pathname : NULL, \
	                          source_stream, TPP_FILE_FLAGS_NOCLOSE,           \
	                          start_lc, TPP_FILE_ENCODING_UTF8)
#else /* TPP_HAVE_FILE_NOCLOSE */
#define tpp_lexer_initfile_stream(self, options, source_stream, start_lc)        \
	(Dee_Incref(source_stream), /* Inherited by `tpp_lexer_initfile_io_ex2()` */ \
	 tpp_lexer_initfile_io_ex2(self, (options) ? (options)->co_pathname : NULL,  \
	                           source_stream, TPP_FILE_FLAGS_NORMAL,             \
	                           start_lc, TPP_FILE_ENCODING_UTF8))
LOCAL WUNUSED NONNULL((1, 3)) tpp_errno DCALL
tpp_lexer_pushfile_stream(tpp_lexer *self,
                          struct Dee_compiler_options const *options,
                          DeeObject *source_stream, tpp_lcinfo start_lc) {
	tpp_errno result;
	Dee_Incref(source_stream); /* Inherited by `tpp_lexer_pushfile_io_ex2()` */
	result = tpp_lexer_pushfile_io_ex2(self, options ? options->co_pathname : NULL,
	                                   source_stream, TPP_FILE_FLAGS_NORMAL,
	                                   start_lc, TPP_FILE_ENCODING_UTF8);
	if (TPP_ISERR(result))
		Dee_DecrefNokill(source_stream);
	return result;
}
#endif /* !TPP_HAVE_FILE_NOCLOSE */

PRIVATE WUNUSED NONNULL((1, 2, 3)) tpp_errno DCALL
DeeCompiler_Configure(DeeCompiler *__restrict self,
                      struct Dee_compiler_options const *__restrict options,
                      DeeObject *source_stream, tpp_lcinfo start_lc) {
	int argc = options->co_argc;
	char **argv = (char **)options->co_argv;
	bool only_makefile;
	tpp_errno error;
	struct compiler_cli_parser parser;
	DeeCompilerCliLoader_Init(&parser.ccp_comp, self);
	tpp_cli_loader_init(&parser.ccp_cli, &self->cp_lexer.dl_lexer);
	tpp_makefile_cli_loader_init(&parser.ccp_cli_mf, &self->cp_lexer.dl_makefile);

	/* Parse arguments... */
	error = DeeCompiler_ParseArgv(&parser, options, &argc, &argv);
	if (TPP_ISERR(error))
		goto err_parser;

	/* Initialize file-stack */
	if (argc) {
		error = tpp_cli_loader_setinputs(&parser.ccp_cli, argc, argv);
		if (TPP_ISERR(error))
			goto err_parser;
		if (!DeeNone_Check(source_stream)) {
			error = tpp_lexer_pushfile_stream(&self->cp_lexer.dl_lexer, options, source_stream, start_lc);
			if (TPP_ISERR(error))
				goto err_parser_filestack;
		}
	} else {
		tpp_lexer_initfile_stream(&self->cp_lexer.dl_lexer, options, source_stream, start_lc);
	}

	/* Makefile should only check ENV variables if this is the __MAIN__ file */
	if (self->cp_flags & Dee_COMPILER_FLAG_MAIN)
		tpp_makefile_cli_loader_enablecheckenv(&parser.ccp_cli_mf);

	/* Flush CLI loaders */
	error = tpp_makefile_cli_loader_flush(&parser.ccp_cli_mf, options ? options->co_output : NULL);
	if (TPP_ISERR(error))
		goto err_parser_filestack;
	error = tpp_cli_loader_flush(&parser.ccp_cli);
	if (TPP_ISERR(error))
		goto err_parser_filestack;
	error = DeeCompilerCliLoader_Flush(&parser.ccp_comp);
	if (TPP_ISERR(error))
		goto err_parser_filestack;

	/* Cleanup */
	only_makefile = tpp_makefile_cli_loader_getonlymakefile(&parser.ccp_cli_mf);
	tpp_makefile_cli_loader_fini(&parser.ccp_cli_mf);
	tpp_cli_loader_fini(&parser.ccp_cli);

	if (only_makefile) {
		/* TODO: Throw an error */
	}

	return error;
err_parser_filestack:
	tpp_lexer_finifile(&self->cp_lexer.dl_lexer);
err_parser:
	tpp_makefile_cli_loader_fini(&parser.ccp_cli_mf);
	tpp_cli_loader_fini(&parser.ccp_cli);
	DeeCompilerCliLoader_Fini(&parser.ccp_comp);
	return error;
}

PRIVATE NONNULL((1)) void DCALL
DeeCompiler_Fini_nofiles(DeeCompiler *__restrict self) {
	if (self->cp_tags.at_anno.an_annov) {
		if unlikely(self->cp_tags.at_anno.an_annoc) {
			DeeCompiler_LockWriteNoInt();
			while (self->cp_tags.at_anno.an_annoc--)
				ast_decref(self->cp_tags.at_anno.an_annov[self->cp_tags.at_anno.an_annoc].aa_func);
			DeeCompiler_LockEndWrite();
		}
		Dee_Free(self->cp_tags.at_anno.an_annov);
		self->cp_tags.at_anno.an_annoa = 0;
		self->cp_tags.at_anno.an_annov = NULL;
	}
	Dee_unicode_printer_fini(&self->cp_tags.at_decl);
	Dee_unicode_printer_fini(&self->cp_tags.at_doc);
	Dee_Decref(self->cp_scope);
	DeeLexer_Fini(&self->cp_lexer);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeCompiler_Init(DeeCompiler *__restrict self, DeeObject *source_stream,
                 tpp_lcinfo start_lc, struct Dee_compiler_options const *options) {
	DeeRootScopeObject *scope;

	/* Create the new root scope object. */
	scope = (DREF DeeRootScopeObject *)DeeObject_NewDefault(&DeeRootScope_Type);
	if unlikely(!scope)
		goto err;
	self->cp_scope = (DREF DeeScopeObject *)scope;
	bzero(&self->cp_tags, sizeof(self->cp_tags));
	self->cp_prev      = NULL;
	self->cp_recursion = 0;

	/* NOTE: This is the default-configuration for imported modules. */
	self->cp_parser_flags    = PARSE_FNORMAL;
	self->cp_optimizer_flags = OPTIMIZE_FENABLED | OPTIMIZE_FCONSTSYMS;
	self->cp_unwind_limit    = 0;
	self->cp_assembler       = ASM_FOPTIMIZE | ASM_FPEEPHOLE | ASM_FREUSELOC | ASM_FSTACKDISP;
	self->cp_flags           = Dee_COMPILER_FLAG_NORMAL;
#ifndef CONFIG_LANGUAGE_NO_ASM
	self->cp_uasm_unique = 0;
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	DeeLexer_Init(&self->cp_lexer);

	if (options) {
		/* Inherit compilation flags. */
		self->cp_flags = options->co_flags;

		/* NOTE: This is the default-configuration for __MAIN__ */
		if (self->cp_flags & Dee_COMPILER_FLAG_MAIN) {
			self->cp_optimizer_flags = OPTIMIZE_FNORMAL;
			self->cp_assembler       = ASM_FNORMAL;
		}

		/* Override the name that is used as the
		 * effective display/DDI string of the file. */
		if (options->co_filename) {
			TPP_REF tpp_string *used_name;
			char const *filename_utf8;
			ASSERT_OBJECT_TYPE_EXACT(options->co_filename, &DeeString_Type);
			filename_utf8 = DeeString_AsUtf8(options->co_filename);
			if unlikely(!filename_utf8)
				goto err_self;
			used_name = tpp_string_malloc(WSTR_LENGTH(filename_utf8));
			if unlikely(!used_name)
				goto err_self;
			memcpyc(tpp_string_str(used_name), filename_utf8,
			        WSTR_LENGTH(filename_utf8), sizeof(char));
			tpp_file_setfilename(DeeLexer_GetFile(&self->cp_lexer), used_name);
			tpp_string_decref_nokill(used_name);
		}
	
		/* Set the name of the current base-scope, which
		 * describes the function of the module's root code. */
		if (options->co_rootname) {
			char const *rootname_utf8;
			tpp_keyword const *rootname_kwd;
			ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
			rootname_utf8 = DeeString_AsUtf8(options->co_rootname);
			if unlikely(!rootname_utf8)
				goto err_self;
			rootname_kwd = DeeLexer_NewKeyword(&self->cp_lexer,
			                                   (tpp_char const *)rootname_utf8,
			                                   WSTR_LENGTH(rootname_utf8));
			if unlikely(!rootname_kwd)
				goto err_self;
			scope->rs_scope.bs_name = rootname_kwd;
		}

		/* Parse CLI arguments + configure file-stack */
		if (options->co_argc) {
			tpp_errno error;
			error = DeeCompiler_Configure(self, options, source_stream, start_lc);
			if (TPP_ISERR(error))
				goto err_self;
		} else {
			goto setup_default_file_stack;
		}
	} else {
		/* Simply case: just initialize file-stack using the given stream. */
setup_default_file_stack:
		tpp_lexer_initfile_stream(&self->cp_lexer.dl_lexer,
		                          options, source_stream, start_lc);
	}

	return 0;
err_self:
	DeeCompiler_Fini_nofiles(self);
err:
	return -1;
}


PUBLIC NONNULL((1)) void DCALL
DeeCompiler_Fini(DeeCompiler *__restrict self) {
	tpp_lexer_finifile(&self->cp_lexer.dl_lexer);
	DeeCompiler_Fini_nofiles(self);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeCompiler_Compile_impl(DeeCompiler *__restrict compiler,
                         struct Dee_serial *__restrict writer,
                         unsigned int mode, DeeObject *default_symbols) {
	DREF DeeCodeObject *root_code;
	DREF struct ast *code;
	DeeLexer *const lexer = DeeLexer_OfCompiler(compiler);

	if (parser_flags & PARSE_FLFSTMT)
		tpp_lexer_enablefeature(&lexer->dl_lexer, TPP_FEAT_TOK_LF);

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err;
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
		dots->s_decltype.da_type = DAST_NONE;
		dots->s_type  = SYMBOL_TYPE_ARG;
		dots->s_symid = 0;
		dots->s_flag |= SYMBOL_FALLOC;
		current_basescope->bs_argc    = 1;
		current_basescope->bs_argv[0] = dots;
		current_basescope->bs_varargs = dots;
		current_basescope->bs_flags |= Dee_CODE_FVARARGS;
	}

	(void)default_symbols; /* TODO */

	/* Yield the initial token. */
	if (TPP_TOK_ISERR(DeeLexer_Yield(lexer)))
		goto err;
	/* Parse statements until the end of the source stream. */
	switch (mode & DeeExec_RUNMODE_MASK) {

	default:
		code = ast_parse_statements_until(lexer, AST_FMULTIPLE_KEEPLAST, TPP_TOK_EOF);
		break;

	case DeeExec_RUNMODE_STMT:
		code = ast_parse_statement(lexer, false);
		goto pack_code_in_return;

	case DeeExec_RUNMODE_EXPR:
		code = ast_parse_comma(lexer, AST_COMMA_NORMAL,
		                       AST_FMULTIPLE_KEEPLAST,
		                       NULL);
		goto pack_code_in_return;

	case DeeExec_RUNMODE_FULLEXPR: {
		DREF struct ast *return_ast;
		code = ast_parse_comma(lexer,
		                       AST_COMMA_NORMAL |
		                       AST_COMMA_ALLOWVARDECLS |
		                       AST_COMMA_ALLOWTYPEDECL,
		                       AST_FMULTIPLE_KEEPLAST,
		                       NULL);
pack_code_in_return:
		if unlikely(!code)
			goto err;
		return_ast = ast_putddi(ast_return(code), &code->a_ddi);
		ast_decref(code);
		code = return_ast;
	}	break;

	}

	/* Rethrow all errors that may have occurred during parsing. */
	if unlikely(!code)
		goto err;
	if (DeeLexer_GetErrorCount(lexer)) {
		DeeLexer_TPP_RaiseLexErrorHook(&lexer->dl_lexer);
		goto err;
	}

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		/* Rethrow all errors that may have occurred during optimization. */
		if (ast_optimize_all(code, false))
			goto err;
		if (DeeLexer_GetErrorCount(lexer)) {
			DeeLexer_TPP_RaiseLexErrorHook(&lexer->dl_lexer);
			goto err;
		}
	}

	{
		uint16_t refc;
		struct asm_symbol_ref *refv;
		root_code = code_compile(code, compiler->cp_assembler,
		                         true, &refc, &refv);
		ASSERT(!root_code || !refc);
		ASSERT(!root_code || !refv);
	}
	ast_decref(code);

	/* Check for errors during assembly. */
	if unlikely(!root_code)
		goto err;

	/* Rethrow all errors that may have occurred during text assembly. */
	if (DeeLexer_GetErrorCount(lexer)) {
		DeeLexer_TPP_RaiseLexErrorHook(&lexer->dl_lexer);
		Dee_Decref(root_code);
		goto err;
	}

	/* Finally, put together the module itself. */
	return module_compile(writer, root_code);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeCompiler_Compile(DeeCompiler *__restrict compiler,
                    struct Dee_serial *__restrict writer,
                    unsigned int mode, DeeObject *default_symbols) {
	int result;
	DREF DeeScopeObject *const saved__current_scope    = current_scope;
	DeeBaseScopeObject *const saved__current_basescope = current_basescope;
	DeeRootScopeObject *const saved__current_rootscope = current_rootscope;
	struct ast_tags const saved__current_tags          = current_tags;
	uint16_t const saved__parser_flags                 = parser_flags;
	uint16_t const saved__optimizer_flags              = optimizer_flags;
	uint16_t const saved__optimizer_unwind_limit       = optimizer_unwind_limit;
	unsigned int const saved__optimizer_count          = optimizer_count;
	DeeCompiler *const saved__DeeCompiler_Current      = DeeCompiler_Current;

	/* Load global context components (that haven't been migrated yet) */
	DeeCompiler_Current = compiler;
	current_scope = compiler->cp_scope;
	ASSERT_OBJECT(current_scope);
	current_basescope = current_scope->s_base;
	ASSERT_OBJECT(&current_basescope->bs_scope);
	current_rootscope = current_basescope->bs_root;
	ASSERT_OBJECT(&current_rootscope->rs_scope.bs_scope);

	memcpy(&current_tags, &compiler->cp_tags, sizeof(struct ast_tags));
	parser_flags           = compiler->cp_parser_flags;
	optimizer_flags        = compiler->cp_optimizer_flags;
	optimizer_unwind_limit = compiler->cp_unwind_limit;

	/* Perform compilation */
	result = DeeCompiler_Compile_impl(compiler, writer, mode, default_symbols);

	/* Update state of "compiler" */
	compiler->cp_scope = current_scope;
	memcpy(&compiler->cp_tags, &current_tags, sizeof(struct ast_tags));
	compiler->cp_parser_flags    = parser_flags;
	compiler->cp_optimizer_flags = optimizer_flags;
	compiler->cp_unwind_limit    = optimizer_unwind_limit;

	/* Restore saved context. */
	DeeCompiler_Current    = saved__DeeCompiler_Current;
	current_scope          = saved__current_scope;
	current_basescope      = saved__current_basescope;
	current_rootscope      = saved__current_rootscope;
	current_tags           = saved__current_tags;
	parser_flags           = saved__parser_flags;
	optimizer_flags        = saved__optimizer_flags;
	optimizer_unwind_limit = saved__optimizer_unwind_limit;
	optimizer_count        = saved__optimizer_count;
	return result;
}

/* Similar to `DeeExec_RunStream()`, but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executes the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20` */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeExec_CompileModuleStream_impl(struct Dee_serial *__restrict writer, DeeObject *source_stream,
                                 int start_line, int start_col, unsigned int mode,
                                 struct Dee_compiler_options *options, DeeObject *default_symbols) {
	int result;
	DeeCompiler compiler;

	/* Initialize compiler. */
	if unlikely(DeeCompiler_Init(&compiler, source_stream,
	                             tpp_lcinfo_of(start_line, start_col),
	                             options))
		goto err;

	/* Lock compiler context */
	result = DeeCompiler_LockWrite();
	if likely(result == 0) {
		result = DeeCompiler_Compile(&compiler, writer, mode, default_symbols);
		DeeCompiler_LockEndWrite();
	}
	DeeCompiler_Fini(&compiler);
	return result;
err:
	return -1;
}

#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */

PUBLIC Dee_WEAKREF(DeeCompilerObject) DeeCompiler_Active = Dee_WEAKREF_INIT;
PRIVATE DeeCompilerObject *compiler_loaded = NULL; /* == DeeCompiler_Active */

PRIVATE void *DCALL
memxch(void *a, void *b, size_t num_bytes) {
	typedef __REGISTER_TYPE__ wordtype;
	uint8_t *pa = (uint8_t *)a;
	uint8_t *pb = (uint8_t *)b;
	while (num_bytes >= sizeof(wordtype)) {
		wordtype temp;
		temp            = *(wordtype *)pa;
		*(wordtype *)pa = *(wordtype *)pb;
		*(wordtype *)pb = temp;
		pa += sizeof(wordtype);
		pb += sizeof(wordtype);
		num_bytes -= sizeof(wordtype);
	}
	while (num_bytes--) {
		uint8_t temp;
		temp = *pa;
		*pa  = *pb;
		*pb  = temp;
		++pa;
		++pb;
	}
	return a;
}


/* compiler --> GLOBAL */
PRIVATE NONNULL((1)) void DCALL
load_compiler(DeeCompilerObject *__restrict compiler) {
	ASSERT(DeeCompiler_LockWriting());
	current_scope = compiler->cp_scope;
	ASSERT_OBJECT(current_scope);
	current_basescope = current_scope->s_base;
	ASSERT_OBJECT(&current_basescope->bs_scope);
	current_rootscope = current_basescope->bs_root;
	ASSERT_OBJECT(&current_rootscope->rs_scope.bs_scope);
	inner_compiler_options = compiler->cp_inner_options;
	memcpy(&current_tags, &compiler->cp_tags, sizeof(struct ast_tags));
	parser_flags           = compiler->cp_parser_flags;
	optimizer_flags        = compiler->cp_optimizer_flags;
	optimizer_unwind_limit = compiler->cp_unwind_limit;
	if (!(compiler->cp_flags & COMPILER_FKEEPLEXER))
		memxch(&TPPLexer_Global, &compiler->cp_lexer.dl_lexer, sizeof(struct TPPLexer));
	if (!(compiler->cp_flags & COMPILER_FKEEPERROR))
		memxch(&current_parser_errors, &compiler->cp_errors, sizeof(struct parser_errors));
}


/* GLOBAL --> compiler */
PRIVATE NONNULL((1)) void DCALL
save_compiler(DeeCompilerObject *__restrict compiler) {
	ASSERT(DeeCompiler_LockWriting());
	ASSERT_OBJECT(current_scope);
	ASSERT_OBJECT(&current_basescope->bs_scope);
	ASSERT_OBJECT(&current_rootscope->rs_scope.bs_scope);
	ASSERT(current_basescope == current_scope->s_base);
	ASSERT(current_rootscope == current_basescope->bs_root);
	compiler->cp_scope         = current_scope;
	compiler->cp_inner_options = inner_compiler_options;
	memcpy(&compiler->cp_tags, &current_tags, sizeof(struct ast_tags));
	DBG_memset(&current_scope, 0xcc, sizeof(current_scope));
	DBG_memset(&current_basescope, 0xcc, sizeof(current_scope));
	DBG_memset(&current_rootscope, 0xcc, sizeof(current_scope));
	DBG_memset(&inner_compiler_options, 0xcc, sizeof(inner_compiler_options));
	DBG_memset(&current_tags, 0xcc, sizeof(current_tags));
	compiler->cp_parser_flags    = parser_flags;
	compiler->cp_optimizer_flags = optimizer_flags;
	compiler->cp_unwind_limit    = optimizer_unwind_limit;
	if (!(compiler->cp_flags & COMPILER_FKEEPLEXER))
		memxch(&TPPLexer_Global, &compiler->cp_lexer.dl_lexer, sizeof(struct TPPLexer));
	if (!(compiler->cp_flags & COMPILER_FKEEPERROR))
		memxch(&current_parser_errors, &compiler->cp_errors, sizeof(struct parser_errors));
}


PUBLIC DREF DeeCompilerObject *DeeCompiler_Current = NULL;
PUBLIC NONNULL((1)) void DCALL
DeeCompiler_Begin(DREF DeeCompilerObject *__restrict compiler) {
	ASSERT(DeeCompiler_LockWriting());
	ASSERT(DeeCompiler_Check(compiler));
	if (DeeCompiler_Current != compiler) {
		ASSERTF(compiler->cp_recursion == 0,
		        "Cannot use interweaved compiler recursion "
		        "(`a -> b -> a` is illegal)!");
		if ((compiler->cp_prev = DeeCompiler_Current) != NULL) {
			ASSERT(compiler_loaded != compiler);
			ASSERT(compiler_loaded == DeeCompiler_Current);
			/* Safe the state of another compiler during recursion. */
			save_compiler(DeeCompiler_Current);
			goto do_load_compiler;
		}
		if (compiler_loaded != compiler) {
			if (compiler_loaded) {
				/* WARNING: The reference counter of `compiler_loaded`
				 *          may already be ZERO(0) at this point, but that is OK. */
				/* Safe the state of a dangling compiler. */
				save_compiler(compiler_loaded);
			}
do_load_compiler:
			/* Load the new compiler. */
			load_compiler(compiler);
			compiler_loaded = compiler;
			Dee_weakref_set(&DeeCompiler_Active, Dee_AsObject(compiler));
		}
		DeeCompiler_Current = compiler;
	}
	ASSERT(compiler_loaded == compiler);
	ASSERT(DeeCompiler_Active.wr_obj == Dee_AsObject(compiler));
	++compiler->cp_recursion;
}

PUBLIC void DCALL
DeeCompiler_End(void) {
	DeeCompilerObject *curr;
	ASSERT(DeeCompiler_LockWriting());
	curr = DeeCompiler_Current;
	ASSERT(curr != NULL);
	ASSERT(compiler_loaded == curr);
	ASSERT(DeeCompiler_Active.wr_obj == Dee_AsObject(curr));
	if (!--curr->cp_recursion) {
		DeeCompiler_Current = curr->cp_prev;
		curr->cp_prev       = NULL;
		if (DeeCompiler_Current) {
			/* Reload the previously active compiler. */
			save_compiler(curr);
			load_compiler(DeeCompiler_Current);
			compiler_loaded = DeeCompiler_Current;
			Dee_weakref_set(&DeeCompiler_Active, Dee_AsObject(DeeCompiler_Current));
		} else {
			/* NOTE: We intentionally leave `compiler_loaded` dangling,
			 *       so we can optimize for cases in which only one compiler
			 *       exists, but it constantly starts and stops.
			 *       In such cases, we leave its state loaded, so we don't
			 *       have to re-load it every time a new operation begins. */
#if 0
			save_compiler(curr);
			compiler_loaded = NULL;
			Dee_weakref_clear(&DeeCompiler_Active);
#endif
		}
	}
}

PUBLIC NONNULL((1)) void DCALL
DeeCompiler_Unload(DREF DeeCompilerObject *__restrict compiler) {
	DeeCompiler_LockWriteNoInt();
	ASSERT(compiler != DeeCompiler_Current);
	ASSERT(compiler->cp_recursion == 0);
	if (compiler_loaded == compiler) {
		save_compiler(compiler);
		compiler_loaded = NULL;
		/* NOTE: Depending on order of destruction, the runtime
		 *       may have already cleared this reference. */
		Dee_weakref_clear(&DeeCompiler_Active);
	}
	DeeCompiler_LockEndWrite();
}








/* -------- Compiler Object Implementation -------- */
/* Construct a new compiler for generating the source for the given `mod`.
 * @param: flags: Set of `COMPILER_F*` (see above) */
PUBLIC WUNUSED DREF DeeCompilerObject *DCALL
DeeCompiler_New(uint16_t flags) {
	DREF DeeCompilerObject *result;
	ASSERTF(!(flags & ~COMPILER_FMASK), "Invalid compiler flags in %x", flags);
	result = DeeObject_MALLOC(DeeCompilerObject);
	if unlikely(!result)
		goto done;
	/* Create the new root scope object. */
	result->cp_scope = (DREF DeeScopeObject *)DeeObject_NewDefault(&DeeRootScope_Type);
	if unlikely(!result->cp_scope)
		goto err_r;
	Dee_weakref_support_init(result);
	bzero(&result->cp_tags, sizeof(result->cp_tags));
	bzero(&result->cp_items, sizeof(result->cp_items));
	result->cp_flags           = flags;
	result->cp_prev            = NULL;
	result->cp_recursion       = 0;
	result->cp_options         = NULL;
	result->cp_inner_options   = NULL;
	result->cp_parser_flags    = PARSE_FNORMAL;
	result->cp_optimizer_flags = OPTIMIZE_FNORMAL;
	result->cp_unwind_limit    = 0;
#ifndef CONFIG_LANGUAGE_NO_ASM
	result->cp_uasm_unique = 0;
#endif /* !CONFIG_LANGUAGE_NO_ASM */
	if (!(flags & COMPILER_FKEEPLEXER)) {
		if (!TPPLexer_Init(&result->cp_lexer.dl_lexer))
			goto err_scope;
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
		/* Mirror MSVC's file-and-line syntax. */
		result->cp_lexer.dl_lexer.l_flags |= TPPLEXER_FLAG_MSVC_MESSAGEFORMAT;
#endif /* CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
		result->cp_lexer.dl_lexer.l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
	}
	if (!(flags & COMPILER_FKEEPERROR))
		parser_errors_init(&result->cp_errors);
	DeeObject_InitStatic(result, &DeeCompiler_Type);
done:
	return result;
err_scope:
	Dee_Decref(result->cp_scope);
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
compiler_fini(DeeCompilerObject *__restrict self) {
	Dee_weakref_support_fini(self);

	/* Make sure that the compiler is fully unloaded. */
	DeeCompiler_Unload(self);

	if (self->cp_tags.at_anno.an_annov) {
		if unlikely(self->cp_tags.at_anno.an_annoc) {
			DeeCompiler_LockWriteNoInt();
			while (self->cp_tags.at_anno.an_annoc--)
				ast_decref(self->cp_tags.at_anno.an_annov[self->cp_tags.at_anno.an_annoc].aa_func);
			DeeCompiler_LockEndWrite();
		}
		Dee_Free(self->cp_tags.at_anno.an_annov);
		self->cp_tags.at_anno.an_annoa = 0;
		self->cp_tags.at_anno.an_annov = NULL;
	}
	Dee_unicode_printer_fini(&self->cp_tags.at_decl);
	Dee_unicode_printer_fini(&self->cp_tags.at_doc);

	/* Always set the error-flag to prevent TPP from attempting
	 * to warn about stuff like unclosed if-blocks, because now
	 * that the compiler has been unloaded, we are no longer
	 * allowed to emit any warnings. */
	self->cp_lexer.dl_lexer.l_flags |= TPPLEXER_FLAG_ERROR;

	/* Then: Destroy its components. */
	if (!(self->cp_flags & COMPILER_FKEEPERROR))
		parser_errors_fini(&self->cp_errors);
	if (!(self->cp_flags & COMPILER_FKEEPLEXER))
		TPPLexer_Quit(&self->cp_lexer.dl_lexer);
	Dee_Decref(self->cp_scope);
	Dee_Free(self->cp_items.cis_list);
}

PRIVATE NONNULL((1, 2)) void DCALL
compiler_visit(DeeCompilerObject *__restrict self, Dee_visit_t proc, void *arg) {
	/* First: Make sure that the compiler is fully unloaded. */
	DeeCompiler_Unload(self);

	/* TODO: parser_errors_visit(&self->cp_errors, proc, arg); */
	// TPP uses DeeObject for its streams, meaning we're holding reference to those!
	/* TODO: TPPLexer_Visit(&self->cp_lexer.dl_lexer, proc, arg); */
	Dee_Visit(self->cp_scope);
}

INTDEF struct type_method tpconst compiler_methods[];
INTDEF struct type_getset tpconst compiler_getsets[];
INTDEF struct type_member tpconst compiler_class_members[];

INTDEF WUNUSED NONNULL((1)) int DCALL
compiler_init(DeeCompilerObject *__restrict self,
              size_t argc, DeeObject *const *argv,
              DeeObject *kw);

PUBLIC DeeTypeObject DeeCompiler_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Compiler",
	/* .tp_doc      = */ NULL,
	/* TODO: This must be a GC object, because user-code may create const-symbols
	 *       that re-reference the compiler, creating a reference loop:
	 * >> import Compiler from rt;
	 * >> local com = Compiler();
	 * >> local sym = com.rootscope.newlocal("foo", loc: none);
	 * >> sym.setconst(com); // Reference loop */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeCompilerObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &compiler_init,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&compiler_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&object_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&compiler_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__FA8008618F75C42A),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ compiler_methods,
	/* .tp_getsets       = */ compiler_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ compiler_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
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
		tpp_string_decref(self->f_text);
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

/* Similar to `DeeExec_RunStream()`, but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executes the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20` */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeExec_CompileModuleStream_impl(struct Dee_serial *__restrict writer, DeeObject *source_stream,
                                 int start_line, int start_col, unsigned int mode,
                                 struct Dee_compiler_options *options, DeeObject *default_symbols) {
	struct TPPFile *base_file;
	uint16_t assembler_flags;
	DREF DeeCompilerObject *compiler;
	DREF DeeCodeObject *root_code;
	DREF struct ast *code;
	DeeLexer *lexer;
	int result;

	compiler = DeeCompiler_New(options ? options->co_compiler : COMPILER_FNORMAL);
	if unlikely(!compiler)
		goto err;
	if (COMPILER_BEGIN(compiler))
		goto err_compiler_not_locked;
	lexer = DeeLexer_OfCompiler(compiler);

	/* Push base file */
	base_file = TPPFile_OpenStream((stream_t)source_stream,
	                               (options && options->co_pathname)
	                               ? options->co_pathname
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
		TPP_REF tpp_string *used_name;
		char const *filename_utf8;
		ASSERT_OBJECT_TYPE_EXACT(options->co_filename, &DeeString_Type);
		filename_utf8 = DeeString_AsUtf8(options->co_filename);
		if unlikely(!filename_utf8)
			goto err_compiler_files;
		used_name = tpp_string_malloc(WSTR_LENGTH(filename_utf8));
		if unlikely(!used_name)
			goto err_compiler_files;
		memcpyc(tpp_string_str(used_name), filename_utf8,
		        WSTR_LENGTH(filename_utf8), sizeof(char));
		ASSERT(!base_file->f_textfile.f_usedname);
		base_file->f_textfile.f_usedname = used_name; /* Inherit */
	}
	ASSERT(!current_basescope->bs_name);

#if 0
	if (!(mode & DeeExec_RUNMODE_FHASPP)) {
		/* Disable preprocessor directives & macros. */
		TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_NO_DIRECTIVES |
		                              TPPLEXER_FLAG_NO_MACROS |
		                              TPPLEXER_FLAG_NO_BUILTIN_MACROS);
	}
#endif

	inner_compiler_options = NULL;
	if (options) {
		/* Set the name of the current base-scope, which
		 * describes the function of the module's root code. */
		if (options->co_rootname) {
			char const *rootname_utf8;
			tpp_keyword const *rootname_kwd;
			ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
			rootname_utf8 = DeeString_AsUtf8(options->co_rootname);
			if unlikely(!rootname_utf8)
				goto err_compiler_files;
			rootname_kwd = DeeLexer_NewKeyword(lexer, (tpp_char const *)rootname_utf8,
			                                   WSTR_LENGTH(rootname_utf8));
			if unlikely(!rootname_kwd)
				goto err_compiler_files;
			current_basescope->bs_name = rootname_kwd;
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
				goto err_compiler_files;
		}
	}

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots = new_unnamed_symbol();
		if unlikely(!dots)
			goto err_compiler_files;
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err_compiler_files;
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
		dots->s_decltype.da_type = DAST_NONE;
		dots->s_type  = SYMBOL_TYPE_ARG;
		dots->s_symid = 0;
		dots->s_flag |= SYMBOL_FALLOC;
		current_basescope->bs_argc    = 1;
		current_basescope->bs_argv[0] = dots;
		current_basescope->bs_varargs = dots;
		current_basescope->bs_flags |= Dee_CODE_FVARARGS;
	}

	(void)default_symbols; /* TODO */

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if (TPP_TOK_ISERR(DeeLexer_Yield(lexer))) {
		code = NULL;
	} else {
		/* Parse statements until the end of the source stream. */
		switch (mode & DeeExec_RUNMODE_MASK) {

		default:
			code = ast_parse_statements_until(lexer, AST_FMULTIPLE_KEEPLAST, TPP_TOK_EOF);
			break;

		case DeeExec_RUNMODE_STMT:
			code = ast_parse_statement(lexer, false);
			goto pack_code_in_return;

		case DeeExec_RUNMODE_EXPR:
			code = ast_parse_comma(lexer, AST_COMMA_NORMAL,
			                       AST_FMULTIPLE_KEEPLAST,
			                       NULL);
			goto pack_code_in_return;

		case DeeExec_RUNMODE_FULLEXPR:
			code = ast_parse_comma(lexer,
			                       AST_COMMA_NORMAL |
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
		}
	}

	if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
		TPPLexer_ClearIfdefStack();

	/* Rethrow all errors that may have occurred during parsing. */
	if unlikely(parser_rethrow(code == NULL))
		goto err_compiler_files_code;
	if unlikely(!code)
		goto err_compiler_files;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		int error = ast_optimize_all(code, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(error != 0))
			error = -1;
		if (error)
			goto err_compiler_files_code;
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
		goto err_compiler_files;

	/* Finally, put together the module itself. */
	result = module_compile(writer, root_code);

#if 0 /* Doesn't throw any new compiler errors... */
	/* Rethrow all errors that may have occurred during module linkage. */
	if (parser_rethrow(result == NULL))
		Dee_Clear(result);
#endif

	DeeCompiler_End();
	Dee_Decref(compiler);
	DeeCompiler_LockEndWrite();
	return result;
err_compiler_files_code:
	ast_xdecref(code);
err_compiler_files:
err_compiler:
	COMPILER_END();
err_compiler_not_locked:
	Dee_Decref(compiler);
err:
	return -1;
}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

PUBLIC WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeExec_CompileModuleStream(DeeObject *source_stream,
                            int start_line, int start_col, unsigned int mode,
                            struct Dee_compiler_options *options, DeeObject *default_symbols) {
	DREF DeeModuleObject *result;
	DeeDecWriter writer;
	DeeDec_Ehdr *ehdr;
	int status = DeeDecWriter_Init(&writer, DeeDecWriter_F_NORMAL);
	if unlikely(status)
		goto err;

	/* Compile module to serial writer */
	status = DeeExec_CompileModuleStream_impl((DeeSerial *)&writer, source_stream,
	                                          start_line, start_col,
	                                          mode, options, default_symbols);
	if unlikely(status)
		goto err_writer;

	/* Package module into a (simplified, because 'DeeModule_IMPORT_F_NOGDEC' is set) EHDR */
#ifdef CONFIG_NO_DEC
	ehdr = DeeDecWriter_PackEhdr(&writer, NULL, 0, DeeModule_IMPORT_F_NORMAL);
#else /* CONFIG_NO_DEC */
	ehdr = DeeDecWriter_PackEhdr(&writer, NULL, 0, DeeModule_IMPORT_F_NOGDEC);
#endif /* !CONFIG_NO_DEC */
	if unlikely(!ehdr)
		goto err_writer;

	/* Convert EHDR into a proper module (by executing relocations stored in `writer`) */
	result = DeeDecWriter_PackModule(&writer, ehdr);
	if unlikely(!result)
		goto err_writer_ehdr;

	/* Cleanup... */
	DeeDecWriter_Fini(&writer);

	/* Start tracking the newly created module. */
	return DeeDec_Track(result);
err_writer_ehdr:
	DeeDec_Ehdr_Destroy(ehdr);
err_writer:
	DeeDecWriter_Fini(&writer);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_COMPILER_C */
