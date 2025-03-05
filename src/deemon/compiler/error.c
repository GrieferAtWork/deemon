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
#ifndef GUARD_DEEMON_COMPILER_ERROR_C
#define GUARD_DEEMON_COMPILER_ERROR_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/error.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

/* Includes for TPP */
#include <deemon/class.h>

DECL_BEGIN

#undef assert
#undef assertf
#define assert           Dee_ASSERT
#define assertf(expr, f) Dee_ASSERT(expr)

INTERN struct parser_errors current_parser_errors;

INTDEF char const *TPPCALL find_most_likely_warning(char const *__restrict name);
INTDEF char const *TPPCALL find_most_likely_extension(char const *__restrict name);

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_symbol_declaration(struct unicode_printer *__restrict printer,
                         struct symbol *__restrict sym) {
	dssize_t temp, result;
	if (!sym->s_decl.l_file)
		return 0;
	result = unicode_printer_printf(printer,
	                                TPPLexer_Current->l_flags & TPPLEXER_FLAG_MSVC_MESSAGEFORMAT
	                                ? "\n%s(%d,%d) : "
	                                : "\n%s:%d:%d: ",
	                                TPPFile_Filename(sym->s_decl.l_file, NULL),
	                                sym->s_decl.l_line + 1,
	                                sym->s_decl.l_col + 1);
	if unlikely(result < 0)
		goto err;
	temp = unicode_printer_printf(printer, "See reference to declaration of `%s'", SYMBOL_NAME(sym));
	if unlikely(temp < 0)
		goto err_temp;
	return result + temp;
err_temp:
	return temp;
err:
	return result;
}



PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
print_warning_message(struct unicode_printer *__restrict _printer,
                      int _wnum, va_list _args) {
	static char const nth[4][3] = { "st", "nd", "rd", "th" };
	dssize_t _warnf_temp, _warnf_result = 0;
	struct TPPString *_temp_string = NULL;
#ifndef __INTELLISENSE__
#define WARNF(...)                                                                     \
	do {                                                                               \
		if unlikely((_warnf_temp = unicode_printer_printf(_printer, __VA_ARGS__)) < 0) \
			goto _warnf_err;                                                           \
		_warnf_result += _warnf_temp;                                                  \
	}	__WHILE0
#define PRINT_SYMBOL_DECLARATION(sym)                                            \
	do {                                                                         \
		if unlikely((_warnf_temp = print_symbol_declaration(_printer, sym)) < 0) \
			goto _warnf_err;                                                     \
		_warnf_result += _warnf_temp;                                            \
	}	__WHILE0
#define MARK(x)    "`" x "'"
#define Q(x)       MARK(x)
#define TOK_S      MARK("%$s")
#define TOK_A      (size_t)(token.t_end - token.t_begin), token.t_begin
#define ARG(T)     va_arg(_args, T)
#define FILENAME() (ARG(struct TPPFile *)->f_name)
#define KWDNAME()  (ARG(struct TPPKeyword *)->k_name)
#define CONST_STR() \
	(_temp_string = TPPConst_ToString(ARG(struct TPPConst *)), _temp_string ? _temp_string->s_text : NULL)

#undef FALSE
#ifdef _MSC_VER
#define FALSE 0, 0
#else /* _MSC_VER */
#define FALSE 0
#endif /* !_MSC_VER */

	switch (_wnum) {
#define DECLARE_WARNING_MESSAGES
#define WARNING_MESSAGE(name, expr) \
	case name:                      \
		expr;                       \
		break;
#include "../../../src/tpp/src/tpp-defs.inl"
#undef WARNING_MESSAGE
#undef DECLARE_WARNING_MESSAGES
	default: break;
	}

#undef FALSE
#undef CONST_STR
#undef KWDNAME
#undef FILENAME
#undef ARG
#undef TOK_A
#undef TOK_S
#undef Q
#undef MARK
#undef WARNF
#undef PRINT_SYMBOL_DECLARATION
#endif /* !__INTELLISENSE__ */
_warnf_end:
	if (_temp_string)
		TPPString_Decref(_temp_string);
	return _warnf_result;
_warnf_err:
	_warnf_result = _warnf_temp;
	goto _warnf_end;
}


PRIVATE WUNUSED DREF DeeObject *DCALL
get_warning_message(int wnum, va_list args) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(print_warning_message(&printer, wnum, args) < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


PRIVATE int DCALL resize_errors(void) {
	size_t new_alloc;
	DREF DeeCompilerErrorObject **new_vector;
	new_alloc = current_parser_errors.pe_errora * 2;
	if (!new_alloc)
		new_alloc = 2;
do_realloc_errors:
	new_vector = (DREF DeeCompilerErrorObject **)Dee_TryReallocc(current_parser_errors.pe_errorv, new_alloc,
	                                                             sizeof(DREF DeeCompilerErrorObject *));
	if unlikely(!new_vector) {
		if (new_alloc != current_parser_errors.pe_errorc + 1) {
			new_alloc = current_parser_errors.pe_errorc + 1;
			goto do_realloc_errors;
		}
		if (Dee_CollectMemory(new_alloc * sizeof(DREF DeeCompilerErrorObject *)))
			goto do_realloc_errors;
		return -1;
	}
	current_parser_errors.pe_errorv = new_vector;
	current_parser_errors.pe_errora = new_alloc;
	return 0;
}

INTERN void DCALL
parser_errors_fini(struct parser_errors *__restrict self) {
	DREF DeeCompilerErrorObject **iter, **end;
	end = (iter = self->pe_errorv) + self->pe_errorc;
	for (; iter < end; ++iter)
		Dee_Decref(*iter);
	Dee_Free(self->pe_errorv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
parser_throw(struct compiler_error_object *__restrict error) {
	ASSERT_OBJECT_TYPE(error, &DeeError_CompilerError);
	/* Special case: If TPP was started using `-E' or `-F', then it is
	 *               possible that no compiler is currently active, in
	 *               which case we need to do this ourself! */
	if (!DeeCompiler_Current) {
		int mode = DeeFile_PrintObjectNl(DeeFile_DefaultStddbg, (DeeObject *)error);
		if unlikely(mode < 0)
			goto err;
	} else {
		/* Invoke a user-defined compiler error handler. */
		if (DeeCompiler_Current->cp_options &&
		    DeeCompiler_Current->cp_options->co_error_handler) {
			int mode;
			mode = (*DeeCompiler_Current->cp_options->co_error_handler)(error, error->ce_mode,
			                                                            DeeCompiler_Current->cp_options->co_error_arg);
			if unlikely(mode < 0)
				goto err;
			/* Special handling for ignoring certain errors. */
			switch (mode) {

			case 3:
				if (error->ce_mode != Dee_COMPILER_ERROR_FATALITY_FORCEFATAL)
					goto done;
				break;

			case 2:
				if (error->ce_mode == Dee_COMPILER_ERROR_FATALITY_FATAL)
					error->ce_mode = Dee_COMPILER_ERROR_FATALITY_WARNING;
				ATTR_FALLTHROUGH
			case 1:
				if (error->ce_mode == Dee_COMPILER_ERROR_FATALITY_ERROR)
					error->ce_mode = Dee_COMPILER_ERROR_FATALITY_WARNING;
				break;

			default: break;
			}
		}
	}
	/* Add the error to the current list of errors. */
	ASSERT(current_parser_errors.pe_errorc <=
	       current_parser_errors.pe_errora);
	if (current_parser_errors.pe_errorc ==
	    current_parser_errors.pe_errora &&
	    resize_errors())
		goto err;

	/* Add the error to the vector of active parser errors. */
	Dee_Incref(error);
	current_parser_errors.pe_errorv[current_parser_errors.pe_errorc++] = error;

	/* Set the error as master if it overrules the old. */
	if (error->ce_mode >= Dee_COMPILER_ERROR_FATALITY_ERROR) {
		if (!current_parser_errors.pe_master ||
		    current_parser_errors.pe_master->ce_mode <
		    error->ce_mode)
			current_parser_errors.pe_master = error;
	}
	/* Check if the parser should errors-out as fatal. */
	if (error->ce_mode >= Dee_COMPILER_ERROR_FATALITY_FATAL)
		return 1;
done:
	return 0;
err:
	return -1;
}


INTERN void DCALL
restore_interrupt_error(DeeThreadObject *__restrict ts,
                        /*inherit*/ struct except_frame *__restrict frame);


INTERN int DCALL parser_rethrow(bool must_fail) {
	struct except_frame **p_iter, *iter;
	uint16_t num_errors;
	DeeThreadObject *caller = DeeThread_Self();
	ASSERTF(caller->t_exceptsz >= current_parser_errors.pe_except,
	        "The caller handled errors that didn't belong to them, or "
	        "forgot to call `parser_start()' prior to compilation");
	if (caller->t_exceptsz > current_parser_errors.pe_except) {
		/* New errors have been thrown in the mean time.
		 * We must analyze them and capture any compiler error.
		 * All other errors we must ignore during this first pass. */
		num_errors = caller->t_exceptsz - current_parser_errors.pe_except;
		p_iter     = &caller->t_except;
		while (num_errors--) {
			iter = *p_iter;
			ASSERT(iter != NULL);
			if (DeeObject_InstanceOf(iter->ef_error, &DeeError_CompilerError)) {
				/* This one's a compiler error. */
				if (current_parser_errors.pe_errorc ==
				    current_parser_errors.pe_errora &&
				    resize_errors())
					goto err_handle_all_but_last;
				current_parser_errors.pe_errorv[current_parser_errors.pe_errorc++] =
				(DREF DeeCompilerErrorObject *)iter->ef_error; /* Inherit */
				/* If no master error has been set yet, use this one. */
				if (!current_parser_errors.pe_master)
					current_parser_errors.pe_master = (DeeCompilerErrorObject *)iter->ef_error;
				*p_iter = iter->ef_prev;
				if (ITER_ISOK(iter->ef_trace))
					Dee_Decref(iter->ef_trace);
				--caller->t_exceptsz;
				except_frame_free(iter);
				continue;
			}
			p_iter = &iter->ef_prev;
		}
		ASSERT(caller->t_exceptsz >= current_parser_errors.pe_except);
		if (caller->t_exceptsz != current_parser_errors.pe_except) {
			/* There are additional errors. -> Discard all but the last. */
			while (caller->t_exceptsz != current_parser_errors.pe_except + 1) {
				if (!DeeError_Print("Secondary error during compilation\n", ERROR_PRINT_DOHANDLE))
					break;
			}
			goto err;
		}
	}
	ASSERT(caller->t_exceptsz == current_parser_errors.pe_except);
	/* Pack together an error. */
	if (current_parser_errors.pe_master) {
		DREF DeeCompilerErrorObject *master;
		size_t i, count;
		/* A master (error/fatal) error was set.
		 * Use it to pack everything together. */
handle_master:
		master = current_parser_errors.pe_master;
		count  = current_parser_errors.pe_errorc;
		ASSERT((master->ce_errorc != 0) ==
		       (master->ce_errorv != NULL));
		if (master->ce_errorc) {
			/* The master already has some errors assigned.
			 * With that in mind, append those errors to what has
			 * already occurred, excluding the master itself. */
			ASSERT((master->ce_errorc != 1) ||
			       (master->ce_errorv[0] == master));
			if (master->ce_errorc > 1) {
				DREF DeeCompilerErrorObject **err_dst, **err_iter, **err_end;
				size_t new_count = count + master->ce_errorc - 1;
				if (new_count > current_parser_errors.pe_errora) {
					/* Must allocate more vector space. */
					DREF DeeCompilerErrorObject **new_vector;
					new_vector = (DREF DeeCompilerErrorObject **)Dee_Reallocc(current_parser_errors.pe_errorv, new_count,
					                                                          sizeof(DREF DeeCompilerErrorObject *));
					if unlikely(!new_vector)
						goto err;
					current_parser_errors.pe_errora = new_count;
					current_parser_errors.pe_errorv = new_vector;
				}
				err_dst = current_parser_errors.pe_errorv + count;
				err_end = (err_iter = master->ce_errorv) + master->ce_errorc;
				for (; err_iter != err_end; ++err_iter) {
					if (*err_iter == master)
						continue;   /* Skip the master error. (that one's already been added) */
					*err_dst++ = *err_iter; /* Inherit reference. */
				}
				ASSERT(err_dst == current_parser_errors.pe_errorv + new_count);
				current_parser_errors.pe_errorc = count = new_count;
			}
			Dee_Free(master->ce_errorv);
		}

		if (current_parser_errors.pe_errora != count) {
			DREF DeeCompilerErrorObject **new_vector;
			/* Truncate unused entries. */
			new_vector = (DREF DeeCompilerErrorObject **)Dee_TryReallocc(current_parser_errors.pe_errorv,
			                                                             current_parser_errors.pe_errorc,
			                                                             sizeof(DREF DeeCompilerErrorObject *));
			if likely(new_vector) {
				current_parser_errors.pe_errorv = new_vector;
				current_parser_errors.pe_errora = current_parser_errors.pe_errorc;
			}
		}

		/* Weakly reference the master error in all child errors. */
		for (i = 0; i < count; ++i)
			Dee_weakref_set(&current_parser_errors.pe_errorv[i]->ce_master, (DeeObject *)master);
		master->ce_errorc = current_parser_errors.pe_errorc;
		master->ce_errorv = current_parser_errors.pe_errorv; /* Inherit */

		/* Steal all of this stuff. */
		current_parser_errors.pe_errora = 0;
		current_parser_errors.pe_errorc = 0;
		current_parser_errors.pe_errorv = NULL;

		/* NOTE: At this point, we secretly transfer a reference from
		 *       the master compiler error's vector entry to our local
		 *      `master' variable. */

		/* With the master fully initialized, throw it. */
		DeeError_Throw((DeeObject *)master);
		Dee_Decref(master);
		goto err;
	}
	if (must_fail) {
		DREF DeeObject *error;
		/* The caller _needs_ us to fail... */
		if (current_parser_errors.pe_errorc) {
			/* Just set the first warning as error... */
			current_parser_errors.pe_master = current_parser_errors.pe_errorv[0];
			goto handle_master;
		}
		/* Nothing went wrong?
		 * Whatever... Let's just throw an anonymous compiler error... */
		error = DeeObject_NewDefault(&DeeError_CompilerError);
		if likely(error) {
			DeeError_Throw(error);
			Dee_Decref(error);
		}
		goto err;
	}
	return 0;
err_handle_all_but_last:
	num_errors = caller->t_exceptsz;
	if unlikely(current_parser_errors.pe_except >= num_errors)
		goto err;
	num_errors -= current_parser_errors.pe_except;
	ASSERT(num_errors != 0);
	ASSERT(caller->t_except != NULL);
	p_iter = &caller->t_except->ef_prev;
	/* Display any additional errors. */
	while (num_errors--) {
		iter = *p_iter;
		ASSERT(iter != NULL);
		*p_iter = iter->ef_prev;
		if (DeeObject_IsInterrupt(iter->ef_error)) {
			/* Restore interrupts. */
			if (ITER_ISOK(iter->ef_trace))
				Dee_Decref(iter->ef_trace);
			restore_interrupt_error(caller, iter);
		} else {
			DeeError_Display(NULL, iter->ef_error,
			                 (DeeObject *)except_frame_gettb(iter));
			if (ITER_ISOK(iter->ef_trace))
				Dee_Decref(iter->ef_trace);
			Dee_Decref(iter->ef_error);
			except_frame_free(iter);
		}
		--caller->t_exceptsz;
	}
err:
	return -1;
}

INTERN void DCALL parser_start(void) {
	current_parser_errors.pe_except = DeeThread_Self()->t_exceptsz;
}




PRIVATE int const tpp_warning_mode_matrix[3] = {
	/* [TPP_WARNINGMODE_FATAL] = */ Dee_COMPILER_ERROR_FATALITY_FATAL,
	/* [TPP_WARNINGMODE_ERROR] = */ Dee_COMPILER_ERROR_FATALITY_ERROR,
	/* [TPP_WARNINGMODE_WARN]  = */ Dee_COMPILER_ERROR_FATALITY_WARNING
};



PRIVATE int DCALL
capture_compiler_location(struct TPPFile *__restrict file,
                          struct compiler_error_loc *__restrict result,
                          struct compiler_error_loc **__restrict p_main_loc) {
#if 1 /* ORDER: low --> high */
	struct compiler_error_loc *extension;
	struct compiler_error_loc *start = result;
	ASSERT(file->f_kind != TPPFILE_KIND_EXPLICIT);
	ASSERT(file != &TPPFile_Empty);
	for (;;) {
		struct TPPLCInfo info;
		if (token.t_file == file &&
		    token.t_begin >= file->f_begin &&
		    token.t_begin <= file->f_end) {
			/* For better debug information, prefer the start of the current token. */
			TPPFile_LCAt(file, &info, token.t_begin);
		} else {
			TPPFile_LCAt(file, &info, file->f_pos);
		}
		TPPFile_Incref(file);
		result->cl_file = file; /* Save a reference to this file. */
		result->cl_line = info.lc_line;
		result->cl_col  = info.lc_col;
		result->cl_prev = NULL;
		/* Make sure that the file owns its own filename,
		 * as otherwise, the name may have been de-allocated
		 * once the user actually wants to print it. */
		if (!TPPFile_Copyname(file))
			goto err;
		/* The first text file is the main location. */
		if (file->f_kind == TPPFILE_KIND_TEXT && !*p_main_loc)
			*p_main_loc = result;
		if (!file->f_prev)
			break;
		file = file->f_prev;
		while (file->f_kind == TPPFILE_KIND_EXPLICIT &&
		       file->f_prev)
			file = file->f_prev;
		if (file->f_kind == TPPFILE_KIND_EXPLICIT ||
		    file == &TPPFile_Empty)
			break;
		extension = (struct compiler_error_loc *)Dee_Malloc(sizeof(struct compiler_error_loc));
		if unlikely(!extension)
			goto err;
		result->cl_prev = extension;
		result          = extension;
	}
	return 0;
err:
	/* Cleanup. */
	ASSERT(!result->cl_prev);
	result = start->cl_prev;
	while (result) {
		start = result->cl_prev;
		Dee_Free(result);
		result = start;
	}
	return -1;
#else /* ORDER: high --> low */
	struct TPPLCInfo info;
	ASSERT(file->f_kind != TPPFILE_KIND_EXPLICIT);
	ASSERT(file != &TPPFile_Empty);
	/* Pre-initialize the prev-pointer to NULL. */
	result->cl_prev = NULL;
	/* The first text file that is encountered is the main location. */
	if (file->f_kind == TPPFILE_KIND_TEXT && !*p_main_loc)
		*p_main_loc = result;
	/* NOTE: Generate a traceback not just for macro invocations,
	 *       but for the entirety of the #include-stack also! */
	if (file->f_prev) {
		struct TPPFile *next_file = file->f_prev;
		while (next_file->f_kind == TPPFILE_KIND_EXPLICIT &&
		       next_file->f_prev)
			next_file = next_file->f_prev;
		if (next_file->f_kind != TPPFILE_KIND_EXPLICIT &&
		    file != &TPPFile_Empty) {
			/* Recursively extend debug information */
			struct compiler_error_loc *extension;
			extension = (struct compiler_error_loc *)Dee_Malloc(sizeof(struct compiler_error_loc));
			if unlikely(!extension)
				goto err;
			if unlikely(capture_compiler_location(next_file, extension, p_main_loc)) {
				Dee_Free(extension);
				goto err;
			}
			result->cl_prev = extension;
		}
	}
	if (token.t_file == file &&
	    token.t_begin >= file->f_begin &&
	    token.t_begin <= file->f_end) {
		/* For better debug information, prefer the start of the current token. */
		TPPFile_LCAt(file, &info, token.t_begin);
	} else {
		TPPFile_LCAt(file, &info, file->f_pos);
	}
	/* Make sure that the file owns its own filename,
	 * as otherwise, the name may have been de-allocated
	 * once the user actually wants to print it. */
	if (!TPPFile_Copyname(file))
		goto err;
	TPPFile_Incref(file);
	result->cl_file = file; /* Save a reference to this file. */
	result->cl_line = info.lc_line;
	result->cl_col  = info.lc_col;
	return 0;
err:
	return -1;
#endif
}

INTDEF ATTR_CONST WUNUSED DeeTypeObject *DCALL
get_warning_error_class(int wnum);

INTERN WUNUSED NONNULL((1)) bool DCALL
tpp_is_reachable_file(struct TPPFile *__restrict file) {
	struct TPPFile *iter = token.t_file;
	/* Make sure that the given file is still valid. */
	for (;;) {
		if (!iter)
			goto nope;
		if (file == iter)
			break;
		if (iter->f_prev == iter)
			goto nope;
		iter = iter->f_prev;
	}
	return true;
nope:
	return false;
}


PRIVATE int DCALL
handle_compiler_warning(struct ast_loc *loc,
                        bool force_fatal,
                        bool is_reachable_file,
                        int wnum, va_list args) {
	DREF DeeCompilerErrorObject *error;
	int mode;
	++TPPLexer_Current->l_warncount;
	if (force_fatal) {
		mode = Dee_COMPILER_ERROR_FATALITY_FORCEFATAL;
	} else {
		/* Invoke the warning using current TPP behavior. */
		mode = TPPLexer_InvokeWarning(wnum);
		ASSERT(mode >= 0);
		if (mode >= TPP_WARNINGMODE_IGNORE)
			goto done_nonfatal;

		/* Translate the warning mode. */
		mode = tpp_warning_mode_matrix[mode];
	}

	/* Construct a new compiler error. */
	error = DeeObject_MALLOC(DeeCompilerErrorObject);
	if unlikely(!error)
		goto err;

	/* Generate an error message. */
	error->e_message = (DREF DeeStringObject *)get_warning_message(wnum, args);
	if unlikely(!error->e_message)
		goto err_error;

	/* Initializer other members of the error object. */
	weakref_support_init(error);
	error->e_inner   = NULL;
	error->ce_errorc = 0;
	error->ce_errorv = NULL;
	error->ce_mode   = mode;
	error->ce_wnum   = wnum;
	/* Collect/inherit debug information. */
	if (loc && loc->l_file &&
	    (is_reachable_file || tpp_is_reachable_file(loc->l_file))) {
		/* Make sure that the file owns its own filename,
		 * as otherwise, the name may have been de-allocated
		 * once the user actually wants to print it. */
		if (!TPPFile_Copyname(loc->l_file))
			goto err_message;
		/* Use the provided file location. */
		error->ce_locs.cl_col  = loc->l_col;
		error->ce_locs.cl_line = loc->l_line;
		error->ce_locs.cl_file = loc->l_file;
		error->ce_locs.cl_prev = NULL;
		error->ce_loc          = &error->ce_locs;
		TPPFile_Incref(loc->l_file);
	} else if (wnum == W_IF_WITHOUT_ENDIF) {
		struct TPPIfdefStackSlot *ifdef_slot;
		ifdef_slot             = va_arg(args, struct TPPIfdefStackSlot *);
		error->ce_locs.cl_col  = 0;
		error->ce_locs.cl_line = ifdef_slot->iss_line;
		error->ce_locs.cl_file = ifdef_slot->iss_file;
		error->ce_locs.cl_prev = NULL;
		error->ce_loc          = &error->ce_locs;
		TPPFile_Incref(error->ce_locs.cl_file);
	} else if (wnum == W_SLASHSTAR_INSIDE_OF_COMMENT ||
	           wnum == W_LINE_COMMENT_CONTINUED) {
		struct TPPLCInfo lc_info;
		TPPFile_LCAt(TPPLexer_Current->l_token.t_file,
		             &lc_info,
		             va_arg(args, char *));
		error->ce_locs.cl_col  = lc_info.lc_col;
		error->ce_locs.cl_line = lc_info.lc_line;
		error->ce_locs.cl_file = TPPLexer_Current->l_token.t_file;
		error->ce_locs.cl_prev = NULL;
		error->ce_loc          = &error->ce_locs;
		TPPFile_Incref(error->ce_locs.cl_file);
	} else {
		/* Capture the current file location. */
		struct TPPFile *file = token.t_file;
		while (file->f_kind == TPPFILE_KIND_EXPLICIT &&
		       file->f_prev)
			file = file->f_prev;
		if unlikely(file->f_kind == TPPFILE_KIND_EXPLICIT ||
		            file == &TPPFile_Empty) {
			/* Special case: The input file is an explicit file. */
			error->ce_locs.cl_col  = 0;
			error->ce_locs.cl_line = 0;
			error->ce_locs.cl_file = NULL;
			error->ce_locs.cl_prev = NULL;
			error->ce_loc          = &error->ce_locs;
		} else {
			error->ce_loc = NULL;
			if unlikely(capture_compiler_location(file, &error->ce_locs,
			                                      &error->ce_loc))
				goto err_message;
			if unlikely(!error->ce_loc)
				error->ce_loc = &error->ce_locs;
		}
	}
	Dee_weakref_initempty(&error->ce_master);

	/* NOTE: Use different sub-classes depending
	 *       on wgroups associated with `wnum' */
	{
		DeeTypeObject *error_type;
		error_type = get_warning_error_class(wnum);
		DeeObject_Init(error, error_type);
	}

	/* Finally, throw the error as a parser-error. */
	mode = parser_throw(error);
	Dee_Decref(error);
	if unlikely(mode)
		goto err;
done_nonfatal:
	return 0;
err_message:
	Dee_Decref(error->e_message);
err_error:
	DeeObject_FREE(error);
err:
	/* Switch TPP to the error-state, now
	 * that something fatal has happened.. */
	TPPLexer_SetErr();
	return -1;
}


INTERN ATTR_COLD int (parser_warnf)(int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(NULL, false, false, wnum, args);
	va_end(args);
	return result;
}

INTERN ATTR_COLD int (DCALL parser_vwarnf)(int wnum, va_list args) {
	return handle_compiler_warning(NULL, false, false, wnum, args);
}

INTERN ATTR_COLD int (parser_errf)(int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(NULL, true, true, wnum, args);
	va_end(args);
	ASSERT(result != 0);
	return result;
}

INTERN ATTR_COLD int (parser_warnatf)(struct ast_loc *loc, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(loc, false, false, wnum, args);
	va_end(args);
	return result;
}

INTERN ATTR_COLD int (parser_warnatptrf)(char const *ptr, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	if (ptr) {
		struct ast_loc loc;
		loc.l_file = token.t_file;
		ASSERT(ptr >= loc.l_file->f_begin &&
		       ptr <= loc.l_file->f_end);
		TPPFile_LCAt(loc.l_file, &loc.l_lc, ptr);
		result = handle_compiler_warning(&loc, false, false, wnum, args);
	} else {
		result = handle_compiler_warning(NULL, false, false, wnum, args);
	}
	va_end(args);
	return result;
}


INTERN ATTR_COLD int (parser_erratf)(struct ast_loc *loc, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(loc, true, false, wnum, args);
	va_end(args);
	ASSERT(result != 0);
	return result;
}

INTERN ATTR_COLD int (parser_warnatrf)(struct ast_loc *loc, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(loc, false, true, wnum, args);
	va_end(args);
	return result;
}

INTERN ATTR_COLD int (parser_erratrf)(struct ast_loc *loc, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(loc, true, true, wnum, args);
	va_end(args);
	ASSERT(result != 0);
	return result;
}

INTERN ATTR_COLD int (parser_warnastf)(struct ast *__restrict loc_ast, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(&loc_ast->a_ddi, false, true, wnum, args);
	va_end(args);
	return result;
}

INTERN ATTR_COLD int (parser_errastf)(struct ast *__restrict loc_ast, int wnum, ...) {
	va_list args;
	int result;
	va_start(args, wnum);
	result = handle_compiler_warning(&loc_ast->a_ddi, true, true, wnum, args);
	va_end(args);
	ASSERT(result != 0);
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ERROR_C */
