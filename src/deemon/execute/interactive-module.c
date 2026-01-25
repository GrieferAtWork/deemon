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
#ifndef GUARD_DEEMON_EXECUTE_INTERACTIVE_MODULE_C
#define GUARD_DEEMON_EXECUTE_INTERACTIVE_MODULE_C 1

#include <deemon/api.h>

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#include <deemon/alloc.h>              /* DeeObject_Free, DeeObject_FreeTracker, Dee_*alloc*, Dee_CollectMemoryoc, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/optimize.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>    /* memcpy(), bzero(), ... */
#include <deemon/system.h>             /* DeeSystem_SEP */
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>        /* atomic_inc, atomic_read */
#include <deemon/util/cache.h>         /* DECLARE_STRUCT_CACHE */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */
#include <deemon/util/rlock.h>         /* Dee_rshared_rwlock_* */

#include <hybrid/sequence/list.h> /* LIST_ENTRY_UNBOUND_INIT */

#include "../runtime/runtime_error.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */


DECLARE_STRUCT_CACHE(sym, struct symbol)
#ifndef NDEBUG
#define sym_alloc() sym_dbgalloc(__FILE__, __LINE__)
#endif /* !NDEBUG */


struct compiler_options_mapping {
	struct Dee_compiler_options  com_opt; /* The compiler options copy. */
	struct Dee_compiler_options *com_map; /* [1..1][const] The associated source-options mapping */
};

typedef struct {
	DeeModuleObject          im_module;   /* [OVERRIDE(.mo_path, [const])]
	                                       * [OVERRIDE(.mo_pself, [0..0][const])]
	                                       * [OVERRIDE(.mo_next, [0..0][const])]
	                                       * [OVERRIDE(.mo_globpself, [0..0][const])]
	                                       * [OVERRIDE(.mo_globnext, [0..0][const])]
	                                       * [OVERRIDE(.mo_importc, [lock(:im_exec_lock)])]
	                                       * [OVERRIDE(.mo_globalc, [lock(:im_exec_lock && :mo_lock)])]
	                                       * [OVERRIDE(.mo_flags, [const][== Dee_MODULE_FLOADING|Dee_MODULE_FINITIALIZING])]
	                                       * [OVERRIDE(.mo_bucketm, [lock(:im_exec_lock)])]
	                                       * [OVERRIDE(.mo_bucketv, [lock(:im_exec_lock)])]
	                                       * [OVERRIDE(.mo_importv, [lock(:im_exec_lock)])]
	                                       * [OVERRIDE(.mo_globalv, [lock(:im_exec_lock && :mo_lock)])]
	                                       * [OVERRIDE(.mo_root, [0..1][lock(READ(:im_exec_lock), WRITE(:im_lock))]
	                                       *                     [COMMENT("The currently generated root-code object (when shared "
	                                       *                              "prior to new code being generated, it is replaced with"
	                                       *                              "an exact copy that is then used for modifcations; i.e. "
	                                       *                              "it can be accessed when using `im_lock', and must be "
	                                       *                              "copied before modifications can be made)\n"
	                                       *                              "NOTE: A reference held by :im_frame.cf_func->fo_code does "
	                                       *                              "not count towards the code object being shared, so-long as "
	                                       *                              "that function isn't being shared, either")])]
	                                       * [OVERRIDE(.mo_loader, [lock(:im_exec_lock)][0..1]
	                                       *                       [COMMENT("Non-NULL, and pointing to the thread managing compilation "
	                                       *                                "when new code is currently being compiled")])]
	                                       * The underlying module. */
	unsigned int             im_mode;     /* [const] The module in which source code will be compiled.
	                                       * Set of `MODULE_INTERACTIVE_MODE_F*' */
	DREF DeeObject          *im_stream;   /* [0..1][(!= NULL) == (im_module.mo_root != NULL)][lock(im_lock)]
	                                       * The stream from which source code is read. */
	struct Dee_compiler_options  im_options;  /* [OVERRIDE(.[co_inner->*]->co_inner, [owned])]
	                                       * [OVERRIDE(.[co_inner->*]->co_pathname, [TAG(DREF)])]
	                                       * [OVERRIDE(.[co_inner->*]->co_filename, [TAG(DREF)])]
	                                       * [OVERRIDE(.[co_inner->*]->co_rootname, [TAG(DREF)])]
	                                       * [OVERRIDE(.co_inner->*->co_decoutput, [TAG(DREF)])]
	                                       * [OVERRIDE(.co_decoutput, [0..0])]
	                                       * [OVERRIDE(.co_decwriter, [valid_if(false)])]
	                                       * [OVERRIDE(.co_assembler, [!(. & ASM_FPEEPHOLE)])]
	                                       * [OVERRIDE(.co_assembler, [. & ASM_FNODEC])]
	                                       * [OVERRIDE(.co_assembler, [. & ASM_FNOREUSECONST])]
	                                       * [OVERRIDE(.co_assembler, [. & ASM_FBIGCODE])]
	                                       * [const] Compiler options used for compiling source code. */
	DREF DeeCompilerObject  *im_compiler; /* [0..1][(!= NULL) == (im_module.mo_root != NULL)][lock(im_lock)]
	                                       * The compiler used for compiling source code */
	/*ref*/ struct TPPFile  *im_basefile; /* [0..1][(!= NULL) == (im_module.mo_root != NULL)][lock(im_lock)]
	                                       * The TPP base file that is linked against `im_stream' */
	struct Dee_code_frame        im_frame;    /* [OVERRIDE(.cf_func, [0..1][(!= NULL) == (:im_module.mo_root != NULL)][TAG(DREF)]
	                                       *                     [COMMENT("A function object referring to the code object used "
	                                       *                              "to execute interactive assembly. This object is copied "
	                                       *                              "prior to being modified the same way `im_module.mo_root' "
	                                       *                              "is, meaning that accessing this function will yield a "
	                                       *                              "sort-of snapshot that will only remain up to date until "
	                                       *                              "it new code has to be compiled, after which point it "
	                                       *                              "will refer to the old code")])]
	                                       * [OVERRIDE(.cf_argc, [== DeeTuple_SIZE(cf_vargs)])]
	                                       * [OVERRIDE(.cf_argv, [== DeeTuple_ELEM(cf_vargs)])]
	                                       * [OVERRIDE(.cf_vargs, [1..1][const])]
	                                       * [lock(im_exec_lock)]
	                                       * Code execution frame used when items are yielded from the interactive module. */
#ifndef CONFIG_NO_THREADS
	Dee_rshared_rwlock_t     im_lock;     /* [ORDER(AFTER(im_exec_lock))] Lock used to synchronize compilation. */
	Dee_rshared_rwlock_t     im_exec_lock;/* Lock used to synchronize execution. */
#endif /* !CONFIG_NO_THREADS */
} InteractiveModule;

#define InteractiveModule_LockReading(self)    Dee_rshared_rwlock_reading(&(self)->im_lock)
#define InteractiveModule_LockWriting(self)    Dee_rshared_rwlock_writing(&(self)->im_lock)
#define InteractiveModule_LockTryRead(self)    Dee_rshared_rwlock_tryread(&(self)->im_lock)
#define InteractiveModule_LockTryWrite(self)   Dee_rshared_rwlock_trywrite(&(self)->im_lock)
#define InteractiveModule_LockCanRead(self)    Dee_rshared_rwlock_canread(&(self)->im_lock)
#define InteractiveModule_LockCanWrite(self)   Dee_rshared_rwlock_canwrite(&(self)->im_lock)
#define InteractiveModule_LockWaitRead(self)   Dee_rshared_rwlock_waitread(&(self)->im_lock)
#define InteractiveModule_LockWaitWrite(self)  Dee_rshared_rwlock_waitwrite(&(self)->im_lock)
#define InteractiveModule_LockRead(self)       Dee_rshared_rwlock_read(&(self)->im_lock)
#define InteractiveModule_LockReadNoInt(self)  Dee_rshared_rwlock_read_noint(&(self)->im_lock)
#define InteractiveModule_LockWrite(self)      Dee_rshared_rwlock_write(&(self)->im_lock)
#define InteractiveModule_LockWriteNoInt(self) Dee_rshared_rwlock_write_noint(&(self)->im_lock)
#define InteractiveModule_LockTryUpgrade(self) Dee_rshared_rwlock_tryupgrade(&(self)->im_lock)
#define InteractiveModule_LockUpgrade(self)    Dee_rshared_rwlock_upgrade(&(self)->im_lock)
#define InteractiveModule_LockDowngrade(self)  Dee_rshared_rwlock_downgrade(&(self)->im_lock)
#define InteractiveModule_LockEndWrite(self)   Dee_rshared_rwlock_endwrite(&(self)->im_lock)
#define InteractiveModule_LockEndRead(self)    Dee_rshared_rwlock_endread(&(self)->im_lock)
#define InteractiveModule_LockEnd(self)        Dee_rshared_rwlock_end(&(self)->im_lock)

#define InteractiveModule_ExecLockReading(self)    Dee_rshared_rwlock_reading(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockWriting(self)    Dee_rshared_rwlock_writing(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockTryRead(self)    Dee_rshared_rwlock_tryread(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockTryWrite(self)   Dee_rshared_rwlock_trywrite(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockCanRead(self)    Dee_rshared_rwlock_canread(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockCanWrite(self)   Dee_rshared_rwlock_canwrite(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockWaitRead(self)   Dee_rshared_rwlock_waitread(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockWaitWrite(self)  Dee_rshared_rwlock_waitwrite(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockRead(self)       Dee_rshared_rwlock_read(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockReadNoInt(self)  Dee_rshared_rwlock_read_noint(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockWrite(self)      Dee_rshared_rwlock_write(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockWriteNoInt(self) Dee_rshared_rwlock_write_noint(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockTryUpgrade(self) Dee_rshared_rwlock_tryupgrade(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockUpgrade(self)    Dee_rshared_rwlock_upgrade(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockDowngrade(self)  Dee_rshared_rwlock_downgrade(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockEndWrite(self)   Dee_rshared_rwlock_endwrite(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockEndRead(self)    Dee_rshared_rwlock_endread(&(self)->im_exec_lock)
#define InteractiveModule_ExecLockEnd(self)        Dee_rshared_rwlock_end(&(self)->im_exec_lock)



#ifndef CONFIG_NO_THREADS
INTERN WUNUSED NONNULL((1)) int DCALL
interactivemodule_lockread(InteractiveModule *__restrict self) {
	return InteractiveModule_ExecLockRead(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
interactivemodule_lockwrite(InteractiveModule *__restrict self) {
	return InteractiveModule_ExecLockWrite(self);
}

INTERN NONNULL((1)) void DCALL
interactivemodule_lockendread(InteractiveModule *__restrict self) {
	InteractiveModule_ExecLockEndRead(self);
}

INTERN NONNULL((1)) void DCALL
interactivemodule_lockendwrite(InteractiveModule *__restrict self) {
	InteractiveModule_ExecLockEndWrite(self);
}

LOCAL bool DCALL
is_an_imod(DeeModuleObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	ASSERT_OBJECT(self);
	for (;;) {
		if (tp_self == &DeeInteractiveModule_Type)
			return true;
		if (tp_self == &DeeModule_Type)
			return false;
		tp_self = DeeType_Base(tp_self);
		ASSERTF(tp_self, "%p (%k) isn't a module",
		        self, Dee_TYPE(self));
	}
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeModule_LockSymbols)(DeeModuleObject *__restrict self) {
	if (is_an_imod(self))
		return InteractiveModule_ExecLockRead((InteractiveModule *)self);
	return 0;
}

PUBLIC NONNULL((1)) void
(DCALL DeeModule_UnlockSymbols)(DeeModuleObject *__restrict self) {
	if (is_an_imod(self))
		InteractiveModule_ExecLockEndRead((InteractiveModule *)self);
}
#else /* !CONFIG_NO_THREADS */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeModule_LockSymbols)(DeeModuleObject *__restrict self) {
	COMPILER_IMPURE();
	(void)self;
	return 0;
}

PUBLIC NONNULL((1)) void
(DCALL DeeModule_UnlockSymbols)(DeeModuleObject *__restrict self) {
	COMPILER_IMPURE();
	(void)self;
}
#endif /* CONFIG_NO_THREADS */


typedef struct {
	OBJECT_HEAD
	DREF InteractiveModule *imi_mod; /* [1..1][const] The underlying, interactive module. */
} InteractiveModuleIterator;

PRIVATE bool DCALL is_statement(void) {
	switch (tok) {

		/* Check for statement-like code constructs (i.e. if, switch, goto, etc.)
		 * While not technically reflected by our parser, we also count `class'
		 * and `function', as well as symbol declarations. */

	case '@': /* Tags are designed to only appear in statement-like constructs. */
	case '{': /* A sub-scope is a pretty darn good indicator for a statement. */
	case KWD_if:
	case KWD_return:
	case KWD_yield:
	case KWD_from:
	case KWD_import: /* Even though `import' and `from' can appear in expressions,
	                  * the preferred expression syntax is `foo from bar', which
	                  * is why these count as statements. */
	case KWD_throw:
	case KWD_print:
	case KWD_for:
	case KWD_foreach:
	case KWD_assert:
	case KWD_do:
	case KWD_while:
	case KWD_break:
	case KWD_continue:
	case KWD_with:
	case KWD_try:
	case KWD_del:
	case KWD___asm:
	case KWD___asm__:
	case KWD_goto:
	case KWD_switch:
		return true;

	default:
		/* XXX: What about labels? */
		// case KWD_case:
		// case KWD_default:
		break;
	}
	return false;
}


PRIVATE WUNUSED DREF struct ast *DCALL
imod_parse_statement(InteractiveModule *__restrict self,
                     unsigned int mode) {
	DREF struct ast *result, *merge;
	bool should_yield = false;
	if ((mode & Dee_MODULE_INTERACTIVE_MODE_FONLYBASEFILE) &&
	    token.t_file != self->im_basefile) {
		/* ... */
	} else if ((mode & (Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR |
	                    Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT)) ==
	           (Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR |
	            Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT)) {
		should_yield = true;
	} else if (mode & Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR) {
		should_yield = !is_statement();
	} else if (mode & Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTSTMT) {
		should_yield = is_statement();
	}

	merge = ast_parse_statement(true);
	if unlikely(!merge)
		goto err;
	if (should_yield) {
		result = ast_yield(merge);
		ast_decref(merge);
	} else {
		result = merge;
	}
	return result;
err:
	return NULL;
}

INTDEF int DCALL skip_lf(void);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
imod_next(InteractiveModule *__restrict self) {
	DREF DeeObject *result;
	struct Dee_code_frame *top_frame;
	DREF struct ast *statement_ast;
	DeeCodeObject *current_code;
	struct Dee_traceback_object *current_traceback;
	size_t preexisting_codesize;
	bool is_reusing_code_object;
	uint16_t old_co_flags;
	uint16_t old_co_localc;
	uint16_t old_co_constc;
	uint16_t old_co_exceptc;
	uint32_t old_co_framesize;
	struct Dee_except_handler *old_co_exceptv;
	DeeDDIObject *old_co_ddi;
	int link_error;

	/* The heart piece of the interactive module execution model:
	 * -> The function that compiles a piece of code, then feeds
	 *    it to the interpreter as yielding code, before taking
	 *    its return value and forwarding as an iterator-item. */
	if (InteractiveModule_ExecLockWrite(self))
		return NULL;

	/* Start by trying to execute code that has already been compiled. */
do_exec_code:
	/* TODO: Optimization: if (*self->im_frame.cf_ip == ASM_UD) { ... } */
	result = DeeCode_ExecFrameSafe(&self->im_frame);
	if (result != NULL) {
		/* value, or iter-done. */
		self->im_frame.cf_result = NULL;
		goto done_exec;
	}
	ASSERT(DeeThread_Self()->t_exceptsz != 0);
	ASSERT(DeeThread_Self()->t_except != NULL);
	if (!DeeObject_InstanceOf(DeeThread_Self()->t_except->ef_error,
	                          &DeeError_IllegalInstruction))
		goto done_exec; /* Something other than the illegal-instruction error that we're looking for. */
	current_traceback = except_frame_gettb(DeeThread_Self()->t_except);
	current_code      = self->im_module.mo_root;
	if unlikely(!current_code)
		goto done_exec;
	preexisting_codesize = current_code->co_codebytes;
	if likely(current_traceback) {
		instruction_t *iter, *end;
		if unlikely(!current_traceback->tb_numframes)
			goto done_exec;
		/* Check if the error occurred past the written portion of interactive code. */
		top_frame = &current_traceback->tb_frames[current_traceback->tb_numframes - 1];
		if (top_frame->cf_func != self->im_frame.cf_func)
			goto done_exec;
		/* If the error occurred somewhere within the loaded portion of code,
		 * then this error isn't actually caused by the the absence of code. */
		ASSERT(top_frame->cf_ip >= current_code->co_code);
		ASSERT(top_frame->cf_ip < current_code->co_code +
		                          current_code->co_codebytes);
		iter                 = top_frame->cf_ip;
		end                  = current_code->co_code + current_code->co_codebytes;
		preexisting_codesize = (size_t)(iter - current_code->co_code);
		for (; iter < end; ++iter) {
			if (*iter != ASM_UD)
				goto done_exec;
		}
	} else {
		/* Strip trailing ASM_UD instructions. */
		while (preexisting_codesize &&
		       current_code->co_code[preexisting_codesize - 1] == ASM_UD)
			--preexisting_codesize;
	}
	if (!DeeError_Handled(ERROR_HANDLED_RESTORE))
		goto done_exec;

	InteractiveModule_LockWriteNoInt(self);
	if unlikely(!self->im_compiler) {
		result = ITER_DONE;
		goto done_compiler;
	}
	ASSERT(!result);
	ASSERT(self->im_basefile);
	ASSERT(self->im_stream);
	ASSERT(self->im_module.mo_root);

	/* Activate the context of the interactive module's compiler. */
	if (COMPILER_BEGIN(self->im_compiler)) {
		result = NULL;
		goto done_compiler;
	}

	if (token.t_file == &TPPFile_Empty) {
		/* Push the source-stream file back onto the TPP include stack.
		 * It got popped after the stream indicated EOF as the result of
		 * available data having been consumed. */
		TPPLexer_PushFile(self->im_basefile);
	}

	/* Save the current exception context. */
	parser_start();

	/* Yield the initial token. */
	if unlikely((tok == TOK_EOF && yield() < 0) || skip_lf()) {
		statement_ast = NULL;
	} else if (tok == TOK_EOF) {
		result = ITER_DONE; /* True EOF */
		if (parser_rethrow(false))
			result = NULL;
		COMPILER_END();
		/* TODO: clear compiler-related fields, thus releasing data no longer needed prematurely. */
		InteractiveModule_LockEndWrite(self);
		InteractiveModule_ExecLockEndWrite(self);
		return result;
	} else {
		/* Parse a statement in accordance to the interaction mode. */
		statement_ast = imod_parse_statement(self, self->im_mode);
	}

	/* Rethrow all errors that may have occurred during parsing. */
	if (parser_rethrow(statement_ast == NULL)) {
		ast_xdecref(statement_ast);
		statement_ast = NULL;
	}

	if unlikely(!statement_ast)
		goto done_compiler_end;

	/* Merge code flags. */
	current_basescope->bs_flags |= current_code->co_flags;

	/* Run an additional optimization pass on the
	 * AST before passing it off to the assembler. */
	if (optimizer_flags & OPTIMIZE_FENABLED) {
		int temp;
		temp = ast_optimize_all(statement_ast, false);
		/* Rethrow all errors that may have occurred during optimization. */
		if (parser_rethrow(temp != 0))
			temp = -1;
		if (temp)
			goto done_statement_ast;
	}

	/* Initialize the assembler context to create new code within our module.
	 * NOTE: 2 references:
	 *    - self->im_module.mo_root
	 *    - self->im_frame.cf_func->fo_code
	 */
	ASSERT(atomic_read(&current_code->ob_refcnt) >= 2);
	ASSERT(self->im_module.mo_root == current_code);
	ASSERT(self->im_frame.cf_func->fo_code == current_code);
	is_reusing_code_object = (atomic_read(&current_code->ob_refcnt) == 2 &&
	                          !DeeObject_IsShared(self->im_frame.cf_func));
	if (is_reusing_code_object) {
		DeeGC_Untrack(Dee_AsObject(current_code));
		is_reusing_code_object = (atomic_read(&current_code->ob_refcnt) == 2);
		if unlikely(!is_reusing_code_object)
			current_code = DeeGC_TRACK(DeeCodeObject, current_code);
	}
	ASSERT(current_code->co_refc == 0);
	ASSERT(current_code->co_argc_min == 0);
	ASSERT(current_code->co_argc_max == 0);
	ASSERT(current_code->co_defaultv == NULL);
	ASSERT(current_code->co_keywords == NULL);
	old_co_flags     = current_code->co_flags;
	old_co_localc    = current_code->co_localc;
	old_co_constc    = current_code->co_constc;
	old_co_exceptc   = current_code->co_exceptc;
	old_co_exceptv   = current_code->co_exceptv;
	old_co_framesize = current_code->co_framesize;
	old_co_ddi       = current_code->co_ddi;
	if (is_reusing_code_object) {
		ASSERT(current_code->co_module == (DREF DeeModuleObject *)self);
		DeeObject_FreeTracker(current_code);
		assembler_init_reuse(current_code,
		                     current_code->co_code +
		                     preexisting_codesize);
		Dee_DecrefNokill(&DeeCode_Type);    /* current_code->ob_type */
		Dee_DecrefNokill(&self->im_module); /* current_code->co_module */
		current_assembler.a_constv = (DREF DeeObject **)current_code->co_constv;
		current_code->co_constv    = NULL;
		current_code->co_constc    = 0;
	} else {
		instruction_t *text;
		assembler_init();
		current_assembler.a_constv = (DREF DeeObject **)Dee_Mallocc(current_assembler.a_consta,
		                                                            sizeof(DREF DeeObject *));
		if unlikely(!current_assembler.a_constv)
			goto done_assembler_fini;

		/* Copy over static variables. */
		Dee_Movrefv(current_assembler.a_constv,
		            current_code->co_constv,
		            current_assembler.a_constc);

		/* Copy over all the unwritten text. */
		text = asm_alloc(preexisting_codesize);
		if unlikely(!text)
			goto done_assembler_fini;
		memcpy(text, current_code->co_code, preexisting_codesize);
	}
	if (current_assembler.a_flag & ASM_FREUSELOC) {
		size_t alloc_size            = (current_assembler.a_locala + 7) / 8;
		current_assembler.a_locala   = current_assembler.a_localc;
		current_assembler.a_localuse = (uint8_t *)Dee_Malloc(alloc_size);
		if unlikely(!current_assembler.a_localuse)
			goto done_assembler_fini;
		memset(current_assembler.a_localuse, 0xff, alloc_size);
	}

	/* Setup the assembler in accordance to the
	 * pre-written code, and compiler options. */
	current_assembler.a_flag = self->im_options.co_assembler;
	ASSERT(!(current_assembler.a_flag & ASM_FPEEPHOLE));
	ASSERT(current_assembler.a_flag & ASM_FNODEC);
	ASSERT(current_assembler.a_flag & ASM_FNOREUSECONST);
	ASSERT(current_assembler.a_flag & ASM_FBIGCODE);
	current_assembler.a_localc = old_co_localc;
	current_assembler.a_constc = old_co_constc;
	current_assembler.a_consta = old_co_constc;

	/* Configure the currently active stack-depth. */
	current_assembler.a_stackcur = (uint16_t)(self->im_frame.cf_sp -
	                                          self->im_frame.cf_stack);
	current_assembler.a_stackmax = (uint16_t)DeeCode_StackDepth(current_code);
	if (current_assembler.a_stackmax < current_assembler.a_stackcur)
		current_assembler.a_stackmax = current_assembler.a_stackcur;

	/* Now to actually assemble the statement. */
	if (ast_genasm(statement_ast, ASM_G_FNORMAL))
		goto done_assembler_fini;

	{
		/* Generate code for jumping access secondary assembly sections. */
		size_t i;
		struct asm_sym *code_end = NULL;
		for (i = 1; i < SECTION_COUNT; ++i) {
			if (current_assembler.a_sect[i].sec_iter ==
			    current_assembler.a_sect[i].sec_begin)
				continue; /* Empty section. */
			code_end = asm_newsym();
			if unlikely(!code_end)
				goto done_assembler_fini;
			asm_setcur(SECTION_COUNT - 1);
			break;
		}
		if (code_end) {
			for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
				asm_setcur(i);
				if (asm_gjmp(ASM_JMP, code_end))
					goto done_assembler_fini;
			}
		}
		asm_setcur(SECTION_COUNT - 1);
		/* Append the trailing UD-instruction byte. */
		if (code_end)
			asm_defsym(code_end);
		if (asm_put(ASM_UD))
			goto done_assembler_fini;
	}

	/* Make sure that all used labels have been defined.
	 * -> We don't allow unresolved forward-labels in
	 *    interactive code (for now...) */
	if unlikely(asm_check_user_labels_defined())
		goto done_assembler_fini;

	/* Merge text sections. */
	if unlikely(asm_mergetext())
		goto done_assembler_fini;

	if (current_assembler.a_flag & (ASM_FPEEPHOLE | ASM_FOPTIMIZE)) {
		link_error = asm_linkstack();
		if unlikely(link_error != 0)
			goto err_link;
	}

	/* Merge static variables. */
	if unlikely(asm_mergestatic())
		goto done_assembler_fini;

	/* Apply constant relocations. */
	if unlikely(asm_applyconstrel())
		goto done_assembler_fini;

	/* Link together the text, resolving relocations. */
	link_error = asm_linktext();
	if unlikely(link_error != 0) {
err_link:
		if unlikely(link_error >= 0) {
			/* Already in bigcode mode? - That's not good... */
			DeeError_Throwf(&DeeError_CompilerError,
			                "Failed to link final code: Relocation target is out of bounds");
		}
		goto done_assembler_fini;
	}

	/* Indicate that we want to loop back and try to execute the newly generated code. */
	result = ITER_DONE;
done_assembler_fini:
	ASSERT(result == NULL || result == ITER_DONE);
	ASSERT(!current_assembler.a_refc);
	if (result != NULL) {
		/* TODO: Create new global variables & add new imports. */
		/*current_rootscope->rs_globalc;*/

		/* Update the frame storage for local variables. */
		if (current_assembler.a_localc > old_co_localc) {
			DREF DeeObject **new_localv;
			uint16_t req_localc = current_assembler.a_localc;
			new_localv = (DREF DeeObject **)Dee_Reallocc(self->im_frame.cf_frame,
			                                             req_localc,
			                                             sizeof(DREF DeeObject *));
			if unlikely(!new_localv)
				goto err_result;
			bzeroc(new_localv + old_co_localc,
			       req_localc - old_co_localc,
			       sizeof(DREF DeeObject *));
			/* Install the new stack. */
			self->im_frame.cf_frame = new_localv;
		}

		/* Update the frame storage for required stack memory. */
		if (current_assembler.a_stackmax > self->im_frame.cf_stacksz) {
			DREF DeeObject **new_stackv;
			uint16_t req_stacksz = current_assembler.a_stackmax;
			new_stackv = (DREF DeeObject **)Dee_Reallocc(self->im_frame.cf_stack,
			                                             req_stacksz,
			                                             sizeof(DREF DeeObject *));
			if unlikely(!new_stackv)
				goto err_result;
			/* Install the new stack. */
			self->im_frame.cf_sp      = new_stackv + (self->im_frame.cf_sp - self->im_frame.cf_stack);
			self->im_frame.cf_stack   = new_stackv;
			self->im_frame.cf_stacksz = req_stacksz;
		}

		current_code = asm_gencode();
		if unlikely(!current_code) {
gencode_failed:
			result = NULL;
			if (is_reusing_code_object)
				goto recover_old_code_object;
		} else {
			ASSERT(current_rootscope->rs_code == current_code);
			current_rootscope->rs_code = current_code->co_next;
			Dee_DecrefNokill(current_code); /* Stolen from `current_rootscope->rs_code' */
			current_code->co_next = NULL;
			/* Fill in module-field of the new code object. */
			Dee_Incref(&self->im_module);
			current_code->co_module = &self->im_module;

			ASSERT((current_code->co_exceptc != 0) ==
			       (current_code->co_exceptv != 0));
			if (old_co_exceptc) {
				/* Pre-pend the old exception handler vector before the new one! */
				if (!current_code->co_exceptc) {
					current_code->co_exceptc = old_co_exceptc;
					current_code->co_exceptv = old_co_exceptv;
				} else if (is_reusing_code_object) {
					struct Dee_except_handler *full_exceptv;
					full_exceptv = (struct Dee_except_handler *)Dee_Reallocc(old_co_exceptv,
					                                                     current_code->co_exceptc + old_co_exceptc,
					                                                     sizeof(struct Dee_except_handler));
					if unlikely(!full_exceptv)
						goto gencode_failed;
					memcpyc(full_exceptv + old_co_exceptc,
					        current_code->co_exceptv,
					        current_code->co_exceptc,
					        sizeof(struct Dee_except_handler));
					Dee_Free(current_code->co_exceptv);
					current_code->co_exceptc += old_co_exceptc;
					current_code->co_exceptv = full_exceptv;
				} else {
					struct Dee_except_handler *full_exceptv;
					uint16_t i;
					full_exceptv = (struct Dee_except_handler *)Dee_Reallocc(current_code->co_exceptv,
					                                                     current_code->co_exceptc + old_co_exceptc,
					                                                     sizeof(struct Dee_except_handler));
					if unlikely(!full_exceptv)
						goto gencode_failed;
					memmoveupc(full_exceptv + old_co_exceptc,
					           full_exceptv,
					           current_code->co_exceptc,
					           sizeof(struct Dee_except_handler));
					memcpyc(full_exceptv,
					        old_co_exceptv,
					        old_co_exceptc,
					        sizeof(struct Dee_except_handler));
					for (i = 0; i < old_co_exceptc; ++i)
						Dee_XIncref(full_exceptv[i].eh_mask);
					current_code->co_exceptc += old_co_exceptc;
					current_code->co_exceptv = full_exceptv;
				}
			}

			/* TODO: Merge the old DDI descriptor with the new one.
			 * NOTE: If something goes wrong here, `current_code->co_exceptv'
			 *       must be truncated to a total length of `old_co_exceptc' */
			if (is_reusing_code_object)
				Dee_Decref(old_co_ddi);

			/* Set the new code object. */
			Dee_Incref(current_code);
			ASSERT(current_code->ob_refcnt == 2);

			if (DeeObject_IsShared(self->im_frame.cf_func)) {
				DeeFunctionObject *new_self_func;
				/* Must create a new function-object. */
				new_self_func = (DeeFunctionObject *)DeeFunction_NewNoRefs(current_code);
				Dee_DecrefNokill(current_code);
				if unlikely(!new_self_func) {
					result = NULL;
					if (is_reusing_code_object)
						goto recover_old_code_object;
					goto do_assembler_fini;
				}
				Dee_Decref(self->im_frame.cf_func);
				self->im_frame.cf_func = new_self_func; /* Inherit reference. */
			} else {
				if (!is_reusing_code_object)
					Dee_Decref(self->im_frame.cf_func->fo_code);
				self->im_frame.cf_func->fo_code = current_code; /* Inherit reference. */
#ifdef CONFIG_HAVE_CODE_METRICS
				atomic_inc(&current_code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
				if (self->im_frame.cf_func->fo_hostasm.hafu_data)
					Dee_hostasm_function_data_destroy(self->im_frame.cf_func->fo_hostasm.hafu_data);
				Dee_hostasm_function_init(&self->im_frame.cf_func->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
			}
			if (!is_reusing_code_object)
				Dee_Decref(self->im_module.mo_root);
			self->im_module.mo_root = current_code; /* Inherit reference. */
		}
		/* Set the code-execution frame to continue where it left off before. */
		self->im_frame.cf_ip = current_code->co_code + preexisting_codesize;
	} else {
err_result:
		result = NULL;
		if (is_reusing_code_object) {
			/* Recover the old code object. */
recover_old_code_object:
			/*ASSERT(is_reusing_code_object);*/
			current_code = current_assembler.a_sect[SECTION_TEXT].sec_code;
			current_code->co_code[preexisting_codesize] = ASM_UD;
			current_code->co_flags     = old_co_flags;
			current_code->co_localc    = old_co_localc;
			current_code->co_constc    = old_co_constc;
			current_code->co_constv    = current_assembler.a_constv;
			current_code->co_refc      = 0;
			current_code->co_refstaticc = 0;
			current_code->co_exceptc   = old_co_exceptc;
			current_code->co_exceptv   = old_co_exceptv;
			current_code->co_argc_min  = 0;
			current_code->co_argc_max  = 0;
			current_code->co_framesize = old_co_framesize;
			current_code->co_codebytes = (code_size_t)(preexisting_codesize + 1);
			Dee_Incref(&self->im_module);
			current_code->co_module   = &self->im_module;
			current_code->co_defaultv = NULL;
			current_code->co_keywords = NULL;
			current_code->co_ddi      = old_co_ddi;
#ifdef CONFIG_HAVE_CODE_METRICS
			Dee_code_metrics_init(&current_code->co_metrics);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
			if (current_code->co_hostasm.haco_data)
				Dee_hostasm_code_data_destroy(current_code->co_hostasm.haco_data);
			Dee_hostasm_code_init(&current_code->co_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
			Dee_Decrefv(current_assembler.a_constv, current_assembler.a_constc);
			current_assembler.a_sect[SECTION_TEXT].sec_code  = NULL;
			current_assembler.a_sect[SECTION_TEXT].sec_begin = NULL;
			current_assembler.a_sect[SECTION_TEXT].sec_iter  = NULL;
			current_assembler.a_sect[SECTION_TEXT].sec_end   = NULL;
			current_assembler.a_constc = 0;
			current_assembler.a_consta = 0;
			current_assembler.a_constv = NULL;
		}
	}
do_assembler_fini:
	assembler_fini();
done_statement_ast:
	ast_decref(statement_ast);
	statement_ast = NULL;
done_compiler_end:
	COMPILER_END();
done_compiler:
	InteractiveModule_LockEndWrite(self);
done_exec:
	if (result == ITER_DONE)
		goto do_exec_code;
	InteractiveModule_ExecLockEndWrite(self);
	return result;
}


#define INTERACTIVE_MODULE_DEFAULT_GLOBAL_SYMBOL_MASK 15


PRIVATE void DCALL
incref_options(struct Dee_compiler_options *__restrict self) {
	Dee_XIncref(self->co_pathname);
	Dee_XIncref(self->co_filename);
	Dee_XIncref(self->co_rootname);
	Dee_XIncref(self->co_decoutput);
}

PRIVATE void DCALL
decref_options(struct Dee_compiler_options *__restrict self) {
	Dee_XDecref(self->co_pathname);
	Dee_XDecref(self->co_filename);
	Dee_XDecref(self->co_rootname);
	Dee_XDecref(self->co_decoutput);
}

PRIVATE void DCALL
visit_options(struct Dee_compiler_options *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(self->co_pathname);
	Dee_XVisit(self->co_filename);
	Dee_XVisit(self->co_rootname);
	Dee_XVisit(self->co_decoutput);
}


PRIVATE int DCALL
copy_options_chain(struct compiler_options_mapping **p_root,
                   struct compiler_options_mapping **p_result,
                   struct Dee_compiler_options *__restrict source) {
	struct compiler_options_mapping *iter;
	iter = *p_root;
	/* Search for a pre-existing mapping for `source' */
	for (; iter; iter = (struct compiler_options_mapping *)iter->com_opt.co_inner) {
		if (iter->com_map == source) {
			*p_result = iter;
			return 0;
		}
	}
	/* Create a copy of `source'. */
	iter = (struct compiler_options_mapping *)Dee_Malloc(sizeof(struct compiler_options_mapping));
	if unlikely(!iter)
		goto err;
	memcpy(iter, source, sizeof(struct Dee_compiler_options));
	iter->com_opt.co_inner = NULL;
	iter->com_map          = source;
	/* Save the result. */
	*p_result = iter;
	COMPILER_WRITE_BARRIER();
	if (source->co_inner) {
		/* Copy inner set of options. */
		if (copy_options_chain(p_root,
		                       (struct compiler_options_mapping **)&iter->com_opt.co_inner,
		                       source->co_inner)) {
			/* Undo the copy */
			*p_result = NULL;
			Dee_Free(iter);
			goto err;
		}
	}
	incref_options(&iter->com_opt);
	return 0;
err:
	return -1;
}


PRIVATE void DCALL
free_options_chain(struct Dee_compiler_options *entry,
                   struct Dee_compiler_options *base,
                   unsigned int depth) {
	unsigned int i;
	struct Dee_compiler_options *iter = base;
	for (i = 0; i < depth; ++i) {
		if (iter == entry)
			return; /* Options loop detected */
		iter = iter->co_inner;
	}
	if (entry->co_inner)
		free_options_chain(entry->co_inner, base, depth + 1);
	decref_options(entry);
	Dee_Free(entry);
}

PRIVATE void DCALL
visit_options_chain(struct Dee_compiler_options *entry,
                    struct Dee_compiler_options *base,
                    unsigned int depth, Dee_visit_t proc, void *arg) {
	unsigned int i;
	struct Dee_compiler_options *iter = base;
	for (i = 0; i < depth; ++i) {
		if (iter == entry)
			return; /* Options loop detected */
		iter = iter->co_inner;
	}
	if (entry->co_inner)
		visit_options_chain(entry->co_inner, base, depth + 1, proc, arg);
	visit_options(entry, proc, arg);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
module_rehash_globals(DeeModuleObject *__restrict self) {
	size_t i, new_mask = (self->mo_bucketm << 1) | 1;
	struct Dee_module_symbol *new_vec;
	ASSERT(!(new_mask & (new_mask + 1)));
	new_vec = (struct Dee_module_symbol *)Dee_Callocc(new_mask + 1,
	                                              sizeof(struct Dee_module_symbol));
	if unlikely(!new_vec)
		goto err;
	for (i = 0; i <= self->mo_bucketm; ++i) {
		size_t j, perturb;
		struct Dee_module_symbol *item;
		item = &self->mo_bucketv[i];
		if (!item->ss_name)
			continue;
		perturb = j = item->ss_hash & new_mask;
		for (;; Dee_MODULE_HASHNX(j, perturb)) {
			struct Dee_module_symbol *new_item = &new_vec[j & new_mask];
			if (new_item->ss_name)
				continue;
			/* Copy the old item into this new slot. */
			memcpy(new_item, item, sizeof(struct Dee_module_symbol));
			break;
		}
	}
	/* Free the old bucket vector and assign the new one */
	Dee_Free(self->mo_bucketv);
	self->mo_bucketm = (uint16_t)new_mask;
	self->mo_bucketv = new_vec;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
module_import_symbol(DeeModuleObject *self,
                     DeeStringObject *name,
                     DeeObject *value) {
	Dee_hash_t i, perturb, hash;
	DREF DeeObject **new_globalv;
	/* Rehash the global symbol table is need be. */
	if (self->mo_globalc / 2 >= self->mo_bucketm &&
	    module_rehash_globals(self))
		goto err;
	new_globalv = (DREF DeeObject **)Dee_Reallocc(self->mo_globalv,
	                                              self->mo_globalc + 1,
	                                              sizeof(DREF DeeObject *));
	if unlikely(!new_globalv)
		goto err;
	self->mo_globalv = new_globalv;

	/* Append the symbol initializer */
	new_globalv[self->mo_globalc++] = value;
	Dee_Incref(value);

	/* Insert the new object into the symbol table. */
	hash    = DeeString_Hash(name);
	perturb = i = Dee_MODULE_HASHST(self, hash);
	for (;; Dee_MODULE_HASHNX(i, perturb)) {
		struct Dee_module_symbol *item = Dee_MODULE_HASHIT(self, i);
		if (item->ss_name)
			continue;
		/* Use this item. */
		item->ss_name  = DeeString_STR(name);
		item->ss_flags = Dee_MODSYM_FNAMEOBJ;
		item->ss_doc   = NULL;
		item->ss_hash  = hash;
		item->ss_index = self->mo_globalc - 1;
		Dee_Incref(name);
		break;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
module_import_symbol_pair(void *arg, DeeObject *key, DeeObject *value) {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return module_import_symbol((DeeModuleObject *)arg,
	                            (DeeStringObject *)key, value);
err:
	return -1;
}

#define module_import_symbols(self, default_symbols) \
	DeeObject_ForeachPair(default_symbols, &module_import_symbol_pair, self)




/* - Set the FASSEMBLY flag so we don't have to deal with fast-vs-safe execution,
 *   or any complication caused by having to change the execution model later.
 * - Set the FLENIENT stack flag so make it a little bit easier for us to
 *   automatically allocate more stack memory as new code is generated.
 * - Set the FVARARGS flag because arguments passed to the source code
 *   will be variadic and specified using `argv'
 * - Set the FHEAPFRAME flag because we always need to allocate local variable
 *   memory in the heap, considering how we operate as a yield-like function.
 */
#define INTERACTIVE_MODULE_CODE_FLAGS \
	(Dee_CODE_FASSEMBLY | Dee_CODE_FLENIENT | Dee_CODE_FVARARGS | Dee_CODE_FHEAPFRAME | Dee_CODE_FYIELDING)


INTDEF WUNUSED NONNULL((1)) int DCALL
rehash_scope(DeeScopeObject *__restrict iter);
INTDEF WUNUSED NONNULL((1)) int DCALL
TPPFile_SetStartingLineAndColumn(struct TPPFile *__restrict self,
                                 int start_line, int start_col);


PRIVATE NONNULL((1, 2)) int DCALL
imod_init(InteractiveModule *__restrict self,
          DeeObject *source_stream,
          unsigned int mode,
          int start_line, int start_col,
          struct Dee_compiler_options *options,
          DeeObject *source_pathname,
          DeeObject *module_name,
          DeeObject *argv,
          DeeObject *default_symbols) {
	size_t i;
	ASSERT_OBJECT_TYPE_EXACT_OPT(source_pathname, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT_OPT(module_name, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT_OPT(argv, &DeeTuple_Type);
	if (!argv)
		argv = Dee_EmptyTuple;
	/* First off: Copy compiler options */
	if (options) {
		memcpy(&self->im_options, options, sizeof(struct Dee_compiler_options));
		/* Recursively copy inner options from the given set of compiler options. */
		if (options->co_inner) {
			self->im_options.co_inner = NULL;
			if (copy_options_chain((struct compiler_options_mapping **)&self->im_options.co_inner,
			                       (struct compiler_options_mapping **)&self->im_options.co_inner,
			                       options->co_inner))
				goto err;
		}
	} else {
		bzero(&self->im_options, sizeof(struct Dee_compiler_options));
		/* Enable the LFSTMT option by default. */
		self->im_options.co_parser |= PARSE_FLFSTMT;
	}

	/* Make sure to properly set some compiler
	 * options mandatory for interactive compilation. */
	self->im_options.co_assembler &= ~ASM_FPEEPHOLE;
	self->im_options.co_assembler |= (ASM_FNODEC | ASM_FNOREUSECONST | ASM_FBIGCODE);
	self->im_options.co_decoutput = NULL;
	incref_options(&self->im_options);

	/* Determine the module's name. */
	if (!module_name) {
		if (source_pathname) {
			char const *name;
			size_t size;
			char const *name_end, *name_start;
			name = DeeString_AsUtf8(source_pathname);
			if unlikely(!name)
				goto err_options;
			size       = WSTR_LENGTH(name);
			name_end   = name + size;
			name_start = DeeSystem_BaseName(name, size);

			/* Get rid of a file extension in the module name. */
			while (name_end > name_start && name_end[-1] != '.')
				--name_end;
			while (name_end > name_start && name_end[-1] == '.')
				--name_end;
			if (name_end == name_start)
				name_end = name + size;
			module_name = DeeString_NewUtf8(name_start, (size_t)(name_end - name_start),
			                                STRING_ERROR_FIGNORE);
			if unlikely(!module_name)
				goto err_options;
			self->im_module.mo_name = (DREF DeeStringObject *)module_name; /* Inherit reference */
		} else {
			module_name = DeeString_NewEmpty();
		}
	} else {
		Dee_Incref(module_name);
		self->im_module.mo_name = (DREF DeeStringObject *)module_name;
	}

	/* Fill in the module's path name. */
	ASSERT_OBJECT_TYPE_EXACT_OPT(source_pathname, &DeeString_Type);
	Dee_XIncref(source_pathname);
	self->im_module.mo_path = (DREF DeeStringObject *)source_pathname;

	/* Set global hook members as NULL pointers. */
	LIST_ENTRY_UNBOUND_INIT(&self->im_module.mo_link);
	LIST_ENTRY_UNBOUND_INIT(&self->im_module.mo_globlink);

	/* Reset imports, globals and flags. */
	self->im_module.mo_importc = 0;
	self->im_module.mo_importv = NULL;
	self->im_module.mo_globalc = 0;
	self->im_module.mo_globalv = NULL;
	self->im_module.mo_flags   = Dee_MODULE_FLOADING | Dee_MODULE_FINITIALIZING;
	self->im_module.mo_bucketm = INTERACTIVE_MODULE_DEFAULT_GLOBAL_SYMBOL_MASK;
	self->im_module.mo_bucketv = (struct Dee_module_symbol *)Dee_Callocc(INTERACTIVE_MODULE_DEFAULT_GLOBAL_SYMBOL_MASK + 1,
	                                                                 sizeof(struct Dee_module_symbol));
	if unlikely(!self->im_module.mo_bucketv)
		goto err_name;

	/* If given, import default symbols as globals. */
	if (default_symbols) {
		if unlikely(module_import_symbols(&self->im_module, default_symbols) < 0)
			goto err_globals;
	}

#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_init(&self->im_module.mo_lock);
	/* Setup the module to indicate that it isn't being loaded right now. */
	self->im_module.mo_loader = NULL;
#endif /* !CONFIG_NO_THREADS */

	Dee_weakref_support_init(&self->im_module);

	/* With that, all module-level and some interactive
	 * level components have already been initialized. */

	/* Save some of the given arguments in their proper locations. */
	self->im_stream = source_stream;
	Dee_Incref(source_stream);
	self->im_mode = mode;
	Dee_rshared_rwlock_init(&self->im_lock);
	Dee_rshared_rwlock_init(&self->im_exec_lock);

	/* Setup the contents of the initial code frame. */
	self->im_frame.cf_prev  = Dee_CODE_FRAME_NOT_EXECUTING;
	self->im_frame.cf_argc  = DeeTuple_SIZE(argv);
	self->im_frame.cf_argv  = DeeTuple_ELEM(argv);
	self->im_frame.cf_frame = NULL;
	self->im_frame.cf_stack = NULL;
	self->im_frame.cf_sp    = NULL;
	/* self->im_frame.cf_ip   = ...; // Set below! */
	self->im_frame.cf_vargs   = (DREF DeeTupleObject *)argv;
	self->im_frame.cf_this    = NULL;
	self->im_frame.cf_result  = NULL;
	self->im_frame.cf_stacksz = 0;
	self->im_frame.cf_flags   = INTERACTIVE_MODULE_CODE_FLAGS;
	Dee_Incref(argv);

	/* Allocate the function object for the initial code frame. */
	self->im_frame.cf_func = (DeeFunctionObject *)DeeGCObject_Malloc(offsetof(DeeFunctionObject, fo_refv));
	if unlikely(!self->im_frame.cf_func)
		goto err_stream;
	DeeObject_Init(self->im_frame.cf_func, &DeeFunction_Type);
	/* self->im_frame.cf_func->fo_code = ...; // Set below! */

	/* Allocate the compiler that will be used
	 * to process data from the source stream. */
	self->im_compiler = DeeCompiler_New(Dee_AsObject(&self->im_module),
	                                    self->im_options.co_compiler);
	if unlikely(!self->im_compiler)
		goto err_frame;

	/* Setup the initial configuration that will be used by the compiler,
	 * as well as link in the source stream as the used TPP input file. */
	self->im_compiler->cp_options         = &self->im_options;
	self->im_compiler->cp_inner_options   = self->im_options.co_inner;
	self->im_compiler->cp_parser_flags    = self->im_options.co_parser;
	self->im_compiler->cp_optimizer_flags = self->im_options.co_optimizer;
	self->im_compiler->cp_unwind_limit    = self->im_options.co_unwind_limit;

	if (self->im_options.co_parser & PARSE_FLFSTMT)
		self->im_compiler->cp_lexer.l_flags |= TPPLEXER_FLAG_WANTLF;

	/* Allocate the varargs symbol for the root-scope. */
	{
		struct symbol *dots;
		DeeRootScopeObject *root_scope;
		root_scope = (DeeRootScopeObject *)self->im_compiler->cp_scope;
		if unlikely((dots = sym_alloc()) == NULL)
			goto err_compiler;
		DBG_memset(dots, 0xcc, sizeof(struct symbol));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		dots->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
		dots->s_decltype.da_type = DAST_NONE;
		dots->s_name        = &TPPKeyword_Empty;
		dots->s_flag        = SYMBOL_FALLOC;
		dots->s_nread       = 0;
		dots->s_nwrite      = 0;
		dots->s_nbound      = 0;
		dots->s_scope       = &root_scope->rs_scope.bs_scope;
		dots->s_type        = SYMBOL_TYPE_ARG;
		dots->s_symid       = 0;
		dots->s_decl.l_file = NULL;

		dots->s_next                        = root_scope->rs_scope.bs_scope.s_del;
		root_scope->rs_scope.bs_scope.s_del = dots;

		root_scope->rs_scope.bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
		if unlikely(!root_scope->rs_scope.bs_argv) {
			sym_free(dots);
			goto err_compiler;
		}
		root_scope->rs_scope.bs_argc    = 1;
		root_scope->rs_scope.bs_argv[0] = dots;
		root_scope->rs_scope.bs_varargs = dots;
		root_scope->rs_scope.bs_flags |= INTERACTIVE_MODULE_CODE_FLAGS;
	}

	{
		DeeStringObject *source_path;
		/*ref*/ struct TPPFile *base_file;
		source_path = self->im_options.co_pathname;
		if (!source_path)
			source_path = (DeeStringObject *)source_pathname;
		base_file = TPPFile_OpenStream((stream_t)source_stream,
		                               source_path ? DeeString_STR(source_path) : "");
		if unlikely(!base_file)
			goto err_compiler;
		/* Set the non-blocking I/O flag for the input file. */
		base_file->f_textfile.f_flags |= TPP_TEXTFILE_FLAG_NONBLOCK;

		self->im_basefile = base_file; /* Inherit reference. */
		/* Set the starting-line offset. */
		if (TPPFile_SetStartingLineAndColumn(base_file, start_line, start_col))
			goto err_basefile;

		/* Override the name that is used as the
		 * effective display/DDI string of the file. */
		if (self->im_options.co_filename) {
			struct TPPString *used_name;
			ASSERT_OBJECT_TYPE_EXACT(self->im_options.co_filename, &DeeString_Type);
do_create_used_name:
			used_name = TPPString_New(DeeString_STR(self->im_options.co_filename),
			                          DeeString_SIZE(self->im_options.co_filename));
			if unlikely(!used_name) {
				if (Dee_CollectMemoryoc(offsetof(struct TPPString, s_text),
				                        DeeString_SIZE(self->im_options.co_filename) + 1,
				                        sizeof(char)))
					goto do_create_used_name;
				goto err_basefile;
			}
			ASSERT(!base_file->f_textfile.f_usedname);
			base_file->f_textfile.f_usedname = used_name; /* Inherit */
		}

		/* Set the name of the current base-scope, which
		 * describes the function of the module's root code. */
		if (self->im_options.co_rootname) {
			DREF DeeBaseScopeObject *module_base_scope;
			ASSERT_OBJECT_TYPE_EXACT(self->im_options.co_rootname, &DeeString_Type);
			module_base_scope = self->im_compiler->cp_scope->s_base;
			ASSERT(!module_base_scope->bs_name);
do_create_base_name:
			module_base_scope->bs_name = TPPLexer_LookupKeyword(DeeString_STR(self->im_options.co_rootname),
			                                                    DeeString_SIZE(self->im_options.co_rootname), 1);
			if unlikely(!module_base_scope->bs_name) {
				if (Dee_CollectMemoryoc(offsetof(struct TPPKeyword, k_name),
				                        DeeString_SIZE(self->im_options.co_rootname) + 1,
				                        sizeof(char)))
					goto do_create_base_name;
				goto err_basefile;
			}
		}

		/* Create symbol bindings for all the global variables that had been pre-defined. */
		if (self->im_module.mo_globalc || self->im_options.co_setup) {
			/* NOTE: Sadly we must switch compiler context here,
			 *       just so we can use `TPPLexer_LookupKeyword()' */
			if (COMPILER_BEGIN(self->im_compiler))
				goto err_basefile;
			if (self->im_options.co_setup) {
				int error;
				error = (*self->im_options.co_setup)(self->im_options.co_setup_arg);
				if unlikely(error != 0)
					goto err_compiler_basefile;
			}
			if (self->im_module.mo_globalc) {
				current_rootscope->rs_globalc = self->im_module.mo_globalc;
				for (i = 0; i <= self->im_module.mo_bucketm; ++i) {
					struct symbol *sym, **bucket;
					struct Dee_module_symbol *modsym;
					modsym = &self->im_module.mo_bucketv[i];
					if (!modsym->ss_name)
						continue;
					if unlikely((sym = sym_alloc()) == NULL) {
err_compiler_basefile:
						COMPILER_END();
						goto err_basefile;
					}
					DBG_memset(sym, 0xcc, sizeof(struct symbol));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
					sym->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
					sym->s_name = TPPLexer_LookupKeyword(Dee_MODULE_SYMBOL_GETNAMESTR(modsym),
					                                     Dee_MODULE_SYMBOL_GETNAMELEN(modsym),
					                                     1);
					if unlikely(!sym->s_name)
						goto err_compiler_basefile;
					sym->s_decltype.da_type = DAST_NONE;
					sym->s_type         = SYMBOL_TYPE_GLOBAL;
					sym->s_flag         = SYMBOL_FALLOC;
					sym->s_symid        = Dee_module_symbol_getindex(modsym);
					sym->s_global.g_doc = NULL;
					sym->s_nread        = 0;
					sym->s_nwrite       = 1; /* The initial write done by the pre-initialization. */
					sym->s_nbound       = 0;
					sym->s_decl.l_file  = NULL;
					/* Register the symbol in the current scope. */
					if (++current_scope->s_mapc > current_scope->s_mapa) {
						/* Must rehash this scope. */
						if unlikely(rehash_scope(current_scope))
							goto err_compiler_basefile;
					}
					/* Insert the new symbol into the scope lookup map. */
					ASSERT(current_scope->s_mapa != 0);
					bucket      = &current_scope->s_map[sym->s_name->k_id % current_scope->s_mapa];
					sym->s_next = *bucket;
					*bucket     = sym;
				}
			}
			COMPILER_END();
		}
	}
	/* And with _all_ of that, we've finally configured everything
	 * to represent the initial state of the interactive compiler.
	 * Only thing left is to create the initial code object, which
	 * is done last due to the reference loop it creates. */

	/* Construct a small code object that contains a single `ASM_UD' instruction.
	 * `ASM_UD' is used as a marker for code that hasn't been compiled yet, and
	 * when encountered by the interpreter at the end of the code segment, an
	 * IllegalInstruction error is thrown, which we can handle by checking if it
	 * was thrown by an instruction beyond the end of compiled code, in which case
	 * we know that interactive execution has reached the point where new assembly
	 * has to be generated, and new source code must be compiled.
	 * SO to start out: Create a single-instruction ASM_UD code-object. */
	{
		DREF DeeCodeObject *init_code;
		init_code = DeeCode_Malloc(sizeof(instruction_t));
		if unlikely(!init_code)
			goto err_globals;
		init_code->co_flags     = INTERACTIVE_MODULE_CODE_FLAGS;
		init_code->co_localc    = 0;
		init_code->co_constc    = 0;
		init_code->co_refc      = 0;
		init_code->co_refstaticc = 0;
		init_code->co_exceptc   = 0;
		init_code->co_argc_min  = 0;
		init_code->co_argc_max  = 0;
		init_code->co_framesize = 0;
		init_code->co_codebytes = sizeof(instruction_t);
		init_code->co_module   = (DREF struct Dee_module_object *)self;
		init_code->co_defaultv = NULL;
		init_code->co_constv   = NULL;
		init_code->co_exceptv  = NULL;
		init_code->co_keywords = NULL;
		init_code->co_ddi      = &DeeDDI_Empty;
		init_code->co_code[0]  = ASM_UD;
		Dee_Incref(&self->im_module);
		Dee_Incref(&DeeDDI_Empty);
#ifdef CONFIG_HAVE_CODE_METRICS
		Dee_code_metrics_init(&init_code->co_metrics);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
		Dee_hostasm_code_init(&init_code->co_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
		DeeObject_Init(init_code, &DeeCode_Type);
		init_code = DeeGC_TRACK(DeeCodeObject, init_code);
		self->im_module.mo_root = init_code; /* Inherit reference. */

		/* Set the initial instruction pointer. */
		self->im_frame.cf_ip = init_code->co_code;

		/* Set the code-pointer of the initial function object. */
		Dee_Incref(init_code);
		self->im_frame.cf_func->fo_code = init_code;
#ifdef CONFIG_HAVE_CODE_METRICS
		atomic_inc(&init_code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
		Dee_hostasm_function_init(&self->im_frame.cf_func->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
		Dee_atomic_rwlock_init(&self->im_frame.cf_func->fo_reflock);
	}

	return 0;
err_basefile:
	TPPFile_Decref(self->im_basefile);
err_compiler:
	Dee_Decref(self->im_compiler);
err_frame:
	Dee_DecrefNokill(&DeeFunction_Type);
	DeeObject_FreeTracker(Dee_AsObject(self->im_frame.cf_func));
	DeeObject_Free(Dee_AsObject(self->im_frame.cf_func));
err_stream:
	Dee_Decref(self->im_stream);
err_globals:
	Dee_XDecrefv(self->im_module.mo_globalv, self->im_module.mo_globalc);
	for (i = 0; i <= self->im_module.mo_bucketm; ++i) {
		struct Dee_module_symbol *sym;
		sym = &self->im_module.mo_bucketv[i];
		if (!sym->ss_name)
			continue;
		if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ)
			Dee_Decref(COMPILER_CONTAINER_OF(sym->ss_name, DeeStringObject, s_str));
		if (sym->ss_flags & Dee_MODSYM_FDOCOBJ)
			Dee_Decref(COMPILER_CONTAINER_OF(sym->ss_doc, DeeStringObject, s_str));
	}
	Dee_Free(self->im_module.mo_bucketv);
	Dee_Free(self->im_module.mo_globalv);
err_name:
	Dee_Decref(self->im_module.mo_name);
	Dee_XDecref(self->im_module.mo_path);
err_options:
	free_options_chain(self->im_options.co_inner,
	                   self->im_options.co_inner,
	                   0);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
imod_ctor(InteractiveModule *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *imod_path = NULL;
	DeeObject *imod_name = NULL;
	DeeObject *imod_argv = NULL;
	DeeObject *imod_syms = NULL;
	switch (argc) {

	case 1:
		/* (file stream) */
		break;

	case 2:
		/* (file stream, string pathname) */
		/* (file stream, tuple argv) */
		/* (file stream, mapping default_symbols) */
		if (DeeTuple_Check(argv[1])) {
			imod_argv = argv[1];
		} else if (DeeString_Check(argv[1])) {
			imod_path = argv[1];
		} else {
			imod_syms = argv[1];
		}
		break;

	case 3:
		if (DeeTuple_Check(argv[1])) {
			/* (file stream, tuple argv, mapping default_symbols) */
			imod_argv = argv[1];
			imod_syms = argv[2];
		} else {
			/* (file stream, string pathname, mapping default_symbols) */
			/* (file stream, string pathname, tuple argv) */
			/* (file stream, string pathname, string name) */
			if (DeeObject_AssertTypeExact(argv[1], &DeeString_Type))
				goto err;
			imod_path = argv[1];
			if (DeeTuple_Check(argv[2])) {
				imod_argv = argv[2];
			} else if (DeeString_Check(argv[2])) {
				imod_name = argv[2];
			} else {
				imod_syms = argv[2];
			}
		}
		break;

	case 4:
		if (DeeObject_AssertTypeExact(argv[1], &DeeString_Type))
			goto err;
		imod_path = argv[1];
		if (DeeTuple_Check(argv[2])) {
			/* (file stream, string pathname, tuple argv, mapping default_symbols) */
			imod_argv = argv[2];
			imod_syms = argv[3];
		} else {
			/* (file stream, string pathname, string name, tuple argv) */
			/* (file stream, string pathname, string name, mapping default_symbols) */
			if (DeeObject_AssertTypeExact(argv[2], &DeeString_Type))
				goto err;
			imod_name = argv[2];
			if (DeeTuple_Check(argv[3])) {
				imod_argv = argv[3];
			} else {
				imod_syms = argv[3];
			}
		}
		break;

		/* (file stream, string pathname, string name, tuple argv, mapping default_symbols) */
	case 5:
		if (DeeObject_AssertTypeExact(argv[1], &DeeString_Type))
			goto err;
		if (DeeObject_AssertTypeExact(argv[2], &DeeString_Type))
			goto err;
		if (DeeObject_AssertTypeExact(argv[3], &DeeTuple_Type))
			goto err;
		imod_path = argv[1];
		imod_name = argv[2];
		imod_argv = argv[3];
		imod_syms = argv[4];
		break;

	default:
		err_invalid_argc("_interactivemodule", argc, 1, 5);
		goto err;
	}
	return imod_init(self,
	                 argv[0],
	                 Dee_MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR |
	                 Dee_MODULE_INTERACTIVE_MODE_FONLYBASEFILE,
	                 0,
	                 0,
	                 NULL,
	                 imod_path,
	                 imod_name,
	                 imod_argv,
	                 imod_syms);
err:
	return -1;
}




/* Construct an interactive module
 * Interactive modules are kind-of special, in that they parse,
 * compile, assemble, and link source code a-statement-at-a-time.
 * Additionally, the code that they produce is executed when you
 * iterate the returned module (_InteractiveModule) object, at
 * which point it will return the value of expressions found in
 * the root-scope of the associated source code.
 * NOTE: When constructing an iterator for the returned module,
 *       the iterator cannot be rewound or copied, and will modify
 *       the internal state of the interactive module itself, as
 *       well as various other objects associated with it.
 *       For these reasons, interactive modules do not inherit from
 *       `Sequence', similar to how file objects don't either.
 * PARSING vs. NON-BLOCKING:
 *     - The parser will block until either sufficient tokens have been
 *       made available to construct a full statement (that is guarantied
 *       to be complete), or until the source stream has indicated EOF by
 *       having its read() operator return ZERO.
 *     - As a language, deemon wasn't ~really~ designed to be used in an
 *       interactive fashion, which is why there are some caveats to this:
 *       >> if (foo)
 *       >>     print "bar";
 *       This might seem like a simple and complete statement, so you'd
 *       probably expect the parser to stop blocking feeding it this.
 *       However, I have to disappoint you, because I didn't should you
 *       the full source code:
 *       >> if (foo)
 *       >>     print "bar";
 *       >> else {
 *       >>     print "baz";
 *       >> }
 *       Yeah... As you can see, the parser needed to know if the token
 *       following the true-branch was the keyword else, who's presence
 *       then managed to alter the result of the code generated by the
 *       statement as a whole.
 *       There are a couple of other places with the same problem, such
 *       as try-statements which need to know the trailing token to see
 *       if its `catch' or `finally', in which case another guard would
 *       have to be defined.
 *       However, when dealing with an untrusted, or non-validated data
 *       source, you could force statement completion by balancing unclosed
 *       strings and slash-star style comments, before appending 2 `;'
 *       characters to ensure that an incomplete statement is closed,
 *       while also providing another token that may be needed by the
 *       statement to ensure that no trailing code still exists.
 * LINE-TERMINATED STATEMENTS:
 *     - The ability of interactive compilation often wakes the need of having
 *       a way of interpreting line-feeds as the statement-termination token,
 *       which in the case of deemon is `;'
 *     - Deemon does provide such functionality, which can be enabled
 *       by setting the `PARSE_FLFSTMT' flag in `options->co_parser'.
 *       HINT: The commandline version of deemon exposes this flag as `-C[no-]lfstmt'
 *       HINT: This flag is also enabled by default when `NULL' is passed for `options'
 *       The general rule when it comes to which line-feeds are interpreted
 *       as alias to `;' is fairly simple, in that any non-escaped line-feed
 *       following a non-empty statement, that doesn't appear within parenthesis,
 *       sequence-braces (s.a. abstract sequence syntax), or array-brackets
 *       will be interpreted the same way a `;' would:
 *       >> local x = 10            // OK (plain, old expression)
 *       >> print x                 // OK (plain, old statement)
 *       >> print "x = {}".format({
 *       >>      x
 *       >> })                      // OK (linefeeds in sequence-braces don't count)
 *       An exception to improve usability of language features:
 *       >> local x = ({            // statements found in expressions can also use
 *       >>                         // linefeeds, despite the surrounding parenthesis
 *       >>     local y = 10        // OK (plain, old expression)
 *       >>     y * 2               // OK (plain, old expression)
 *       >> })                      // OK (plain, old expression)
 *       However, an exception to these rules are leading line-feeds following, or
 *       preceding statement-block-prefix-token-sequences, which simply discard
 *       any leading line-feeds:
 *       >> if (foo())     // This does what you think it would, in that
 *       >>                // `print bar' is tuels the true-block of this if-statement,
 *       >>                // despite the fact that there are leading line-feeds.
 *       >> {
 *       >>     print bar
 *       >> }
 *       >> else           // This else still belongs to the `if' above, despite the
 *       >>                // preceiding line-feed
 *       >>     print baz  // This is still the false-block of the if-statement,
 *       >>                // despite the leading line-feeds
 *       >> print "after"  // This statement is no longer apart of the if-statement above.
 *       The same behavior applies to the following statements, with ignored line-feeds
 *       marked as `[]', and `...' meaning another statement or expression:
 *        - `while(...)[] ...'
 *        - `with(...)[] ...'
 *        - `switch(...)[] ...'
 *        - `case ...:[] ...'
 *        - `default:[] ...'
 *        - `...:[] ...' // C-style label
 *        - `try[] ... []finally[] ...'
 *        - `try[] ... []catch[](...)[] ...'
 *        - `do[] ... []while[](...)'
 *        - `if[](...)[] ...'
 *        - `if[](...)[] ... []else[] ...'
 *        - `for(...: ...)[] ... '
 *        - `foreach(...: ...)[] ... '
 *        - `for(...[];[]...[];[]...)[] ... ' // NOTE: Line-feeds can also be used instead of `;' in for-expressions
 *        - `import ... ,[] ...'              // Allow line-feeds after commas in import lists
 *        - `import ... ,[] ... from ...'
 *        - `class ... []{ ... }'
 *        - `function ... [] -> ...'
 *        - `function ... [] { ... }'
 *        - `[] ...'  // Empty lines are ignored (symbolizes line-feeds found before some other statement)
 *       Another simply way of thinking about it is that line-feeds are ignored
 *       before and after any location where a { ... }-style statement could be used.
 *       WARNING: The expression-variants of these statements do not accept
 *                sporadic line-feeds within them, however you can simply wrap
 *                those in parenthesis to force linefeeds to be ignored.
 * RESTRICTIONS:
 *     - If the streamed source code attempts to access the current code object,
 *       the returned object will be a snapshot of the active assembly at the
 *       time when the access was made. This snapshot will not be updated as
 *       additional assembly is generated from new source code.
 *     - Modules of this type will never set the `Dee_MODULE_FDIDLOAD' flag, meaning
 *       that they will never finish ~loading~ in the sense that they will
 *       relinquish their right to modify the module's `mo_importv', `mo_globalv',
 *       `mo_root', `mo_bucketv', etc... fields.
 *     - Interactive code should refrain from starting new threads.
 *       TODO: Currently, not abiding by this rule will result in hard undefined
 *             behavior, potentially resulting in deemon crashing completely.
 *             >> This is a bug that must be resolved at some point!
 *             The problem are the `mo_globalv', `mo_importv' and `mo_bucketv'
 *             vectors of modules, which an interactive module will modify and
 *             replace whenever it pleases while holding an internal lock.
 *             The problem here lies in the fact that due to race conditions,
 *             as well as the fact that some code assumes that these fields
 *             are constant once a module is loaded, also making the assumption
 *             that any module that has made it all the way to the interpreter
 *             will have been completely loaded.
 *             This isn't a situation that should even be able to arise, but
 *             the special locking should be documented, and there should be
 *             a guaranty that modules found in the `mo_importv' vector can
 *             never be interactive modules.
 * @param: source_pathname: The filename of the source file from which data (supposedly) originates.
 *                          Used by `#include' directives, as well as `__FILE__' and ddi information.
 *                          When NULL, an empty string is used internally, which results in the current
 *                          directory being used as base for relative imports.
 * @param: module_name:     When non-NULL, use this as the module's actual name.
 *                          Note however that the module is never made available globally.
 * @param: source_stream:   A stream from which source code is read, which is then compiled immediately.
 * @param: start_line:      The starting line number when compiling code. (zero-based)
 * @param: start_col:       The starting column number when compiling code. (zero-based)
 * @param: options:         A set of compiler options applicable for compiled code.
 *                          Note however that certain options have no effect, such
 *                          as the fact that peephole and other optimizations are
 *                          forced to be disabled, or DEC files are never generated,
 *                          all for reasons that should be quite obvious.
 * @param: mode:            The way that contained source code is able to yield
 *                          expressions to the iterator the interactive module.
 *                          Set of `MODULE_INTERACTIVE_MODE_F*'
 * @param: argv:            An optional tuple object (when NULL, `Dee_EmptyTuple' is used),
 *                          that is passed to module code as arguments (i.e. `[...]').
 * @param: default_symbols: A mapping-like object of type `{(string, Object)...}', that
 *                          contains a set of pre-defined variables that should be made
 *                          available to the interactive source code by use of global
 *                          variables.
 *                          Each item contained within is used to define a global variable
 *                          using the key (which must be a string) as name, and the associated
 *                          value as the initializer to which the resulting global variable
 *                          will be bound prior to the initial launch of interactive assembly.
 *                          Thus, provided symbols are made available by name, left to-be used
 *                          by the module however it pleases. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenInteractive(DeeObject *source_stream,
                          unsigned int mode,
                          int start_line, int start_col,
                          struct Dee_compiler_options *options,
                          DeeObject *source_pathname,
                          DeeObject *module_name,
                          DeeObject *argv,
                          DeeObject *default_symbols) {
	DREF InteractiveModule *result;
	result = DeeGCObject_MALLOC(InteractiveModule);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->im_module, &DeeInteractiveModule_Type);
	if (imod_init(result,
	              source_stream,
	              mode,
	              start_line,
	              start_col,
	              options,
	              source_pathname,
	              module_name,
	              argv,
	              default_symbols))
		goto err_r;
	/* Start tracking the new module as a GC object. */
	return DeeGC_TRACK(DeeModuleObject, &result->im_module);
err_r:
	Dee_DecrefNokill(&DeeInteractiveModule_Type);
	DeeObject_FreeTracker(Dee_AsObject(&result->im_module));
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenInteractiveString(DeeObject *source_stream,
                                unsigned int mode,
                                int start_line, int start_col,
                                struct Dee_compiler_options *options,
                                /*utf-8*/ char const *source_pathname,
                                size_t source_pathsize,
                                /*utf-8*/ char const *module_name,
                                size_t module_namesize,
                                DeeObject *argv,
                                DeeObject *default_symbols) {
	DREF DeeModuleObject *result;
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
	result = DeeModule_OpenInteractive(source_stream,
	                                   mode,
	                                   start_line,
	                                   start_col,
	                                   options,
	                                   source_pathname_ob,
	                                   module_name_ob,
	                                   argv,
	                                   default_symbols);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}




PRIVATE NONNULL((1)) void DCALL
imod_fini(InteractiveModule *__restrict self) {
	Dee_XDecref(self->im_stream);
	Dee_XDecref(self->im_compiler);
	if (self->im_basefile)
		TPPFile_Decref(self->im_basefile);
	if (self->im_module.mo_root)
		Dee_XDecrefv(self->im_frame.cf_frame, self->im_module.mo_root->co_localc);
	Dee_Free(self->im_frame.cf_frame);
	Dee_Decrefv(self->im_frame.cf_stack, (size_t)(self->im_frame.cf_sp -
	                                              self->im_frame.cf_stack));
	Dee_Free(self->im_frame.cf_stack);
	Dee_Decref(self->im_frame.cf_vargs);
	Dee_XDecref(self->im_frame.cf_func);
	if (self->im_options.co_inner) {
		free_options_chain(self->im_options.co_inner,
		                   self->im_options.co_inner,
		                   0);
	}
	decref_options(&self->im_options);
	ASSERT(!(self->im_module.mo_flags & Dee_MODULE_FDIDLOAD));
	Dee_XDecrefv(self->im_module.mo_globalv, self->im_module.mo_globalc);
	Dee_Free(self->im_module.mo_globalv);
}

PRIVATE NONNULL((1)) void DCALL
imod_clear(InteractiveModule *__restrict self) {
	size_t localc, old_globalc;
	DREF DeeObject *old_stream;
	DREF DeeObject **old_globalv;
	struct Dee_compiler_options old_options;
	DREF DeeCompilerObject *old_compiler;
	/*ref*/ struct TPPFile *old_basefile;
	struct Dee_code_frame old_frame;
	InteractiveModule_ExecLockWriteNoInt(self);
	InteractiveModule_LockWriteNoInt(self);
	old_stream        = self->im_stream;
	self->im_stream   = NULL;
	old_basefile      = self->im_basefile;
	self->im_basefile = NULL;
	old_compiler      = self->im_compiler;
	self->im_compiler = NULL;
	memcpy(&old_options, &self->im_options, sizeof(struct Dee_compiler_options));
	bzero(&self->im_options, sizeof(struct Dee_compiler_options));
	self->im_options.co_assembler |= ASM_FNODEC;
	InteractiveModule_LockEndWrite(self);
	memcpy(&old_frame, &self->im_frame, sizeof(struct Dee_code_frame));
	self->im_frame.cf_func    = NULL;
	self->im_frame.cf_frame   = NULL;
	self->im_frame.cf_stack   = NULL;
	self->im_frame.cf_sp      = NULL;
	self->im_frame.cf_ip      = NULL;
	self->im_frame.cf_this    = NULL;
	self->im_frame.cf_result  = NULL;
	self->im_frame.cf_stacksz = 0;
	self->im_frame.cf_flags   = 0;
	localc = self->im_module.mo_root ? self->im_module.mo_root->co_localc : 0;
	ASSERT(!(self->im_module.mo_flags & Dee_MODULE_FDIDLOAD));
	old_globalc = self->im_module.mo_globalc;
	old_globalv = self->im_module.mo_globalv;
	self->im_module.mo_globalc = 0;
	self->im_module.mo_globalv = NULL;
	InteractiveModule_ExecLockEndWrite(self);
	Dee_XDecref(old_stream);
	Dee_XDecref(old_compiler);
	if (old_basefile)
		TPPFile_Decref(old_basefile);
	Dee_XDecrefv(old_frame.cf_frame, localc);
	Dee_Free(old_frame.cf_frame);
	Dee_XDecrefv(old_globalv, old_globalc);
	Dee_Free(old_globalv);
	Dee_Decrefv(old_frame.cf_stack, (size_t)(old_frame.cf_sp - old_frame.cf_stack));
	Dee_Free(old_frame.cf_stack);
	Dee_XDecref(old_frame.cf_func);
	if (old_options.co_inner) {
		free_options_chain(old_options.co_inner,
		                   old_options.co_inner,
		                   0);
	}
	decref_options(&old_options);
}

PRIVATE NONNULL((1, 2)) void DCALL
imod_visit(InteractiveModule *__restrict self, Dee_visit_t proc, void *arg) {
	InteractiveModule_ExecLockReadNoInt(self);
	InteractiveModule_LockReadNoInt(self);
	Dee_Visit(self->im_stream);
	Dee_Visit(self->im_compiler);
	if (self->im_options.co_inner) {
		visit_options_chain(self->im_options.co_inner,
		                    self->im_options.co_inner,
		                    0, proc, arg);
	}
	visit_options(&self->im_options, proc, arg);
	InteractiveModule_LockEndRead(self);
	/* TODO: `TPPFile_Visit(self->im_basefile)' */
	if (self->im_module.mo_root)
		Dee_XVisitv(self->im_frame.cf_frame, self->im_module.mo_root->co_localc);
	Dee_Visitv(self->im_frame.cf_stack, (size_t)(self->im_frame.cf_sp - self->im_frame.cf_stack));
	Dee_Visit(self->im_frame.cf_vargs);
	Dee_Visit(self->im_frame.cf_func);
	InteractiveModule_ExecLockEndRead(self);
}


PRIVATE WUNUSED NONNULL((1)) DREF InteractiveModule *DCALL
imod_iter(InteractiveModule *__restrict self) {
	Dee_Incref(&self->im_module);
	return self;
}

PRIVATE struct type_seq imod_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&imod_iter,
	/* .tp_sizeob       = */ DEFIMPL_UNSUPPORTED(&default__sizeob__unsupported),
	/* .tp_contains     = */ DEFIMPL_UNSUPPORTED(&default__contains__unsupported),
	/* .tp_getitem      = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem      = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem      = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach      = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem                    = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size                       = */ DEFIMPL_UNSUPPORTED(&default__size__unsupported),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
};

PRIVATE struct type_gc tpconst imod_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&imod_clear
};


PUBLIC DeeTypeObject DeeInteractiveModule_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_InteractiveModule",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeModuleObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ InteractiveModule,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &imod_ctor,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&imod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&module_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&module_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&imod_visit,
	/* .tp_gc            = */ &imod_gc,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__8C153DCE147F6A78),
	/* .tp_seq           = */ &imod_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&imod_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


DECL_END
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#endif /* !GUARD_DEEMON_EXECUTE_INTERACTIVE_MODULE_C */
