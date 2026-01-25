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
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/error.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/computed-operators.h>
#include <deemon/dec.h>                /* DeeDecWriter, DeeDecWriter_*, DeeDec_* */
#include <deemon/exec.h>               /* DeeExec_* */
#include <deemon/gc.h>                 /* DeeGC_TRACK */
#include <deemon/module.h>             /* DeeModule*, Dee_compiler_options, Dee_module_object */
#include <deemon/object.h>
#include <deemon/serial.h>             /* DeeSerial, Dee_serial */
#include <deemon/string.h>             /* DeeString*, Dee_unicode_printer_fini */
#include <deemon/system-features.h>    /* bzero, memcpy, memset */
#include <deemon/util/rlock.h>         /* Dee_RSHARED_RWLOCK_INIT, Dee_rshared_rwlock_t */

#include <hybrid/typecore.h> /* __REGISTER_TYPE__ */

#include <stdbool.h> /* false, true */
#include <stddef.h>  /* NULL, size_t */
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
		memxch(&TPPLexer_Global, &compiler->cp_lexer, sizeof(struct TPPLexer));
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
		memxch(&TPPLexer_Global, &compiler->cp_lexer, sizeof(struct TPPLexer));
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
		        "(`a -> b -> a' is illegal)!");
		if ((compiler->cp_prev = DeeCompiler_Current) != NULL) {
			ASSERT(compiler_loaded != compiler);
			ASSERT(compiler_loaded == DeeCompiler_Current);
			/* Safe the state of another compiler during recursion. */
			save_compiler(DeeCompiler_Current);
			goto do_load_compiler;
		}
		if (compiler_loaded != compiler) {
			if (compiler_loaded) {
				/* WARNING: The reference counter of `compiler_loaded'
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
			/* NOTE: We intentionally leave `compiler_loaded' dangling,
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
	ASSERT(DeeCompiler_Check(compiler));
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
/* Construct a new compiler for generating the source for the given `module'.
 * @param: flags: Set of `COMPILER_F*' (see above) */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PUBLIC WUNUSED DREF DeeCompilerObject *DCALL
DeeCompiler_New(uint16_t flags)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
PUBLIC WUNUSED NONNULL((1)) DREF DeeCompilerObject *DCALL
DeeCompiler_New(DeeObject *__restrict module, uint16_t flags)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
{
	DREF DeeCompilerObject *result;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	ASSERT_OBJECT_TYPE(module, &DeeModule_Type);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	ASSERTF(!(flags & ~COMPILER_FMASK), "Invalid compiler flags in %x", flags);
	result = DeeObject_MALLOC(DeeCompilerObject);
	if unlikely(!result)
		goto done;
	/* Create the new root scope object. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	result->cp_scope = (DREF DeeScopeObject *)DeeObject_NewDefault(&DeeRootScope_Type);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result->cp_scope = (DREF DeeScopeObject *)DeeObject_New(&DeeRootScope_Type, 1,
	                                                        (DeeObject **)&module);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
		if (!TPPLexer_Init(&result->cp_lexer))
			goto err_scope;
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
		/* Mirror MSVC's file-and-line syntax. */
		result->cp_lexer.l_flags |= TPPLEXER_FLAG_MSVC_MESSAGEFORMAT;
#endif /* CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC */
		result->cp_lexer.l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
	}
	if (!(flags & COMPILER_FKEEPERROR))
		parser_errors_init(&result->cp_errors);
	DeeObject_Init(result, &DeeCompiler_Type);
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
	self->cp_lexer.l_flags |= TPPLEXER_FLAG_ERROR;

	/* Then: Destroy its components. */
	if (!(self->cp_flags & COMPILER_FKEEPERROR))
		parser_errors_fini(&self->cp_errors);
	if (!(self->cp_flags & COMPILER_FKEEPLEXER))
		TPPLexer_Quit(&self->cp_lexer);
	Dee_Decref(self->cp_scope);
	Dee_Free(self->cp_items.cis_list);
}

PRIVATE NONNULL((1, 2)) void DCALL
compiler_visit(DeeCompilerObject *__restrict self, Dee_visit_t proc, void *arg) {
	/* First: Make sure that the compiler is fully unloaded. */
	DeeCompiler_Unload(self);

	/* TODO: parser_errors_visit(&self->cp_errors, proc, arg); */
	// TPP uses DeeObject for its streams, meaning we're holding reference to those!
	/* TODO: TPPLexer_Visit(&self->cp_lexer, proc, arg); */
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
	/* .tp_doc      = */ DOC("(module:?X3?N?Dstring?DModule)"),
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
			/* tp_deep_ctor:   */ NULL,
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
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
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



#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
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

/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executes the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20' */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeExec_CompileModuleStream_impl(struct Dee_serial *__restrict writer, DeeObject *source_stream,
                                 int start_line, int start_col, unsigned int mode,
                                 struct Dee_compiler_options *options, DeeObject *default_symbols)
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
INTERN WUNUSED NONNULL((1)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeExec_CompileModuleStream_impl(DeeObject *source_stream,
                                 int start_line, int start_col, unsigned int mode,
                                 struct Dee_compiler_options *options, DeeObject *default_symbols)
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
{
	struct TPPFile *base_file;
	DREF DeeCodeObject *root_code;
	DREF DeeCompilerObject *compiler;
	DREF struct ast *code;
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	int result;
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	DREF DeeModuleObject *result;
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	uint16_t assembler_flags;

	compiler = DeeCompiler_New(options ? options->co_compiler : COMPILER_FNORMAL);
	if unlikely(!compiler)
		goto err;
	if (COMPILER_BEGIN(compiler))
		goto err_compiler_not_locked;
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
			ASSERT_OBJECT_TYPE_EXACT(options->co_rootname, &DeeString_Type);
			current_basescope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(options->co_rootname),
			                                                    DeeString_SIZE(options->co_rootname),
			                                                    1);
			if unlikely(!current_basescope->bs_name)
				goto err_compiler;
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
				goto err_compiler;
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
	if unlikely(yield() < 0) {
		code = NULL;
	} else {
		/* Parse statements until the end of the source stream. */
		switch (mode & DeeExec_RUNMODE_MASK) {

		default:
			code = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, TOK_EOF);
			break;

		case DeeExec_RUNMODE_STMT:
			code = ast_parse_statement(false);
			goto pack_code_in_return;

		case DeeExec_RUNMODE_EXPR:
			code = ast_parse_comma(AST_COMMA_NORMAL,
			                       AST_FMULTIPLE_KEEPLAST,
			                       NULL);
			goto pack_code_in_return;

		case DeeExec_RUNMODE_FULLEXPR:
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
		}
	}
	if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
		TPPLexer_ClearIfdefStack();

	/* Rethrow all errors that may have occurred during parsing. */
	if unlikely(parser_rethrow(code == NULL))
		goto err_compiler_code;
	if unlikely(!code)
		goto err_compiler;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		int error = ast_optimize_all(code, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(error != 0))
			error = -1;
		if (error)
			goto err_compiler_code;
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
		goto err_compiler;

	/* Finally, put together the module itself. */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	result = module_compile(writer, root_code);
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	result = module_compile(root_code);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	Dee_Decref(root_code);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#if 0 /* Doesn't throw any new compiler errors... */
	/* Rethrow all errors that may have occurred during module linkage. */
	if (parser_rethrow(result == NULL))
		Dee_Clear(result);
#endif

	DeeCompiler_End();
	Dee_Decref(compiler);
	DeeCompiler_LockEndWrite();
	return result;
err_compiler_code:
	ast_xdecref(code);
err_compiler:
	COMPILER_END();
err_compiler_not_locked:
	Dee_Decref(compiler);
err:
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
	return -1;
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	return NULL;
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
}

PUBLIC WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeExec_CompileModuleStream(DeeObject *source_stream,
                            int start_line, int start_col, unsigned int mode,
                            struct Dee_compiler_options *options, DeeObject *default_symbols) {
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
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
	ehdr = DeeDecWriter_PackEhdr(&writer, NULL, 0, DeeModule_IMPORT_F_NOGDEC);
	if unlikely(!ehdr)
		goto err_writer;

	/* Convert EHDR into a proper module (by executing relocations stored in `writer') */
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
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
	DREF DeeModuleObject *result;
	result = DeeExec_CompileModuleStream_impl(source_stream, start_line, start_col,
	                                          mode, options, default_symbols);
	if likely(result)
		result = DeeGC_TRACK(DeeModuleObject, result);
	return result;
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
}

#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_COMPILER_C */
