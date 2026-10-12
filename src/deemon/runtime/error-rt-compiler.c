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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_RT_COMPILER_C
#define GUARD_DEEMON_RUNTIME_ERROR_RT_COMPILER_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/compiler/error.h>     /* DeeCompilerErrorObject, DeeCompilerError_Print, Dee_compiler_error_loc */
#include <deemon/compiler/tpp.h>
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error.h>              /* DeeErrorObject, DeeError_Error, DeeError_NOTIMPLEMENTED */
#include <deemon/object.h>             /* DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_REQUIRES_OBJECT, Dee_formatprinter_t, Dee_ssize_t, Dee_weakref_support_init, OBJECT_HEAD_INIT */
#include <deemon/format.h>
#include <deemon/operator-hints.h>     /* DeeNO_print_t */
#include <deemon/serial.h>             /* DeeSerial, Dee_seraddr_t */
#include <deemon/struct.h>             /* DeeStructObject_Cmp, DeeStructObject_PrintRepr */
#include <deemon/type.h>               /* DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, Dee_visit_t, TF_NONE, TP_FNORMAL, TYPE_MEMBER_CONST, TYPE_MEMBER_END, type_member */
#include <deemon/util/weakref.h>       /* Dee_weakref_fini, Dee_weakref_initempty */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

#define Error_init_params "msg?:?X2?Dstring?O,cause?:?X2?DError?O"
#define error_print_common(self, printer, arg, custom_printer)                  \
	error_print_common(Dee_REQUIRES_OBJECT(DeeErrorObject, self), printer, arg, \
	                   (DeeNO_print_t)(custom_printer))
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL error_print_common)(DeeErrorObject *__restrict self,
                           Dee_formatprinter_t printer, void *arg,
                           DeeNO_print_t custom_printer);


#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeLexerWarningLoc_Print(DeeLexerWarningLoc const *__restrict self,
                         Dee_formatprinter_t printer, void *arg) {
	char const *filename = self->lwl_filename;
	if (filename == NULL)
		filename = "?";
	if (tpp_lcinfo_isvalid(self->lwl_lc)) {
		return DeeFormat_Printf(printer, arg, "%s(%d, %d): ", filename,
		                        tpp_lcinfo_getline(self->lwl_lc) + 1,
		                        tpp_lcinfo_getcol(self->lwl_lc) + 1);
	}
	return DeeFormat_Printf(printer, arg, "%s: ", filename);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeCompilerError_Print(DeeObject *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	DeeCompilerErrorObject *me = (DeeCompilerErrorObject *)self;
	for (i = 0; i < me->ce_warnc; ++i) {
		DeeLexerWarningLoc *from;
		struct Dee_lexer_warning *warn = &me->ce_warnv[i];
		tpp_warning_context_id const ctxid = tpp_warning_invokeinfo_getctxid(&warn->lw_invokeinfo);
		temp = DeeLexerWarningLoc_Print(&warn->lw_loc, printer, arg);
		if (temp < 0)
			goto err_temp;
		result += temp;
		/* Print what this is about... */
		temp = tpp_warning_invokeinfo_getstate(&warn->lw_invokeinfo) == TPP_WSTATE_WARN
		               ? DeeFormat_PRINT(printer, arg, "warning")
		               : DeeFormat_PRINT(printer, arg, "error");
		if (temp < 0)
			goto err_temp;
		result += temp;

		/* Print the relevant context name. */
#if TPP_HAVE_WARNING_NUMBERS
		if (tpp_warning_context_id_isnumber(ctxid)) {
			tpp_warning_id ctx_wid = tpp_warning_context_id_aswarning(ctxid);
			unsigned int number = tpp_warning_getnumbers(ctx_wid)[0];
			if tpp_likely(number != TPP_WARNING_NUMBER_INVALID) {
				temp = DeeFormat_Printf(printer, arg, "[%u]", number);
				if (temp < 0)
					goto err_temp;
				result += temp;
			}
		} else
#endif /* TPP_HAVE_WARNING_NUMBERS */
		{
			tpp_warning_group_id group_id = tpp_warning_context_id_asgroup(ctxid);
			char const *group_name = tpp_warning_group_getnames(group_id);
			if tpp_likely(group_name != NULL) {
				temp = DeeFormat_Printf(printer, arg, "[-W%s]", group_name);
				if (temp < 0)
					goto err_temp;
				result += temp;
			}
		}
		temp = DeeFormat_PRINT(printer, arg, ": ");
		if (temp < 0)
			goto err_temp;
		result += temp;
		temp = DeeString_PrintUtf8(Dee_AsObject(warn->lw_message), printer, arg);
		if (temp < 0)
			goto err_temp;
		result += temp;

		/* Print origin information */
		for (from = warn->lw_loc.lwl_prev; from; from = from->lwl_prev) {
			temp = DeeLexerWarningLoc_Print(from, printer, arg);
			if (temp < 0)
				goto err_temp;
			result += temp;
			temp = DeeFormat_PRINT(printer, arg, "note: originating from here\n");
			if (temp < 0)
				goto err_temp;
			result += temp;
		}
	}
	return result;
err_temp:
	return temp;
}
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */

/************************************************************************/
/* Error.CompilerError                                                  */
/************************************************************************/

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
INTERN NONNULL((1)) void DCALL
DeeLexerWarningLoc_Fini_InError(DeeLexerWarningLoc *__restrict self) {
	if (self->lwl_filename_obj)
		Dee_Decref(self->lwl_filename_obj);
}

INTERN NONNULL((1)) void DCALL
DeeLexerWarning_Fini_InError(DeeLexerWarning *__restrict self) {
	DeeLexerWarningLoc *iter;
	Dee_Decref(self->lw_message);
	DeeLexerWarningLoc_Fini_InError(&self->lw_loc);
	iter = self->lw_loc.lwl_prev;
	while (iter) {
		DeeLexerWarningLoc *next = iter->lwl_prev;
		DeeLexerWarningLoc_Fini_InError(iter);
		DeeLexerWarningLoc_Free(iter);
		iter = next;
	}
}
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */

PRIVATE NONNULL((1)) void DCALL
comerr_init_common(DeeCompilerErrorObject *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	self->ce_warnc = 0;
	self->ce_warnv = NULL;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	Dee_weakref_support_init(self);
	self->ce_mode = 0;
	self->ce_wnum = 0;
	self->ce_locs.cl_prev = NULL;
	self->ce_locs.cl_file = NULL;
	self->ce_locs.cl_line = 0;
	self->ce_locs.cl_col = 0;
	self->ce_loc = NULL;
	Dee_weakref_initempty(&self->ce_master);
	self->ce_errorc = 0;
	self->ce_errorv = NULL;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comerr_copy(DeeCompilerErrorObject *__restrict self,
            DeeCompilerErrorObject *__restrict other) {
	/* TODO */
	(void)self;
	(void)other;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comerr_serialize(DeeCompilerErrorObject *__restrict self,
                 DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	/* TODO */
	(void)self;
	(void)writer;
	(void)addr;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comerr_init_kw(DeeCompilerErrorObject *__restrict self,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/* TODO: Initialization for CompilerError-specific fields */
	int result = (*DeeError_Error.tp_init.tp_alloc.tp_any_ctor_kw)(Dee_AsObject(self), argc, argv, kw);
	if likely(result == 0)
		comerr_init_common(self);
	return result;
}

PRIVATE NONNULL((1)) void DCALL
comerr_fini(DeeCompilerErrorObject *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
	size_t i;
	for (i = 0; i < self->ce_warnc; ++i)
		DeeLexerWarning_Fini_InError(&self->ce_warnv[i]);
	Dee_Free(self->ce_warnv);
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
	size_t i, count = self->ce_errorc;
	ASSERTF(self->ce_locs.cl_prev == NULL ||
	        self->ce_locs.cl_file != NULL,
	        "More than one location requires a base file to be present");
	if (self->ce_locs.cl_file) {
		/* Cleanup saved file locations. */
		struct Dee_compiler_error_loc *next, *iter;
		TPPFile_Decref(self->ce_locs.cl_file);
		iter = self->ce_locs.cl_prev;
		while (iter) {
			next = iter->cl_prev;
			ASSERT(iter->cl_file);
			TPPFile_Decref(iter->cl_file);
			Dee_Free(iter);
			iter = next;
		}
	}
	for (i = 0; i < count; ++i) {
		if (self->ce_errorv[i] != self)
			Dee_Decref(self->ce_errorv[i]);
	}
	Dee_Free(self->ce_errorv);
	Dee_weakref_fini(&self->ce_master);
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
}

#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
#define PTR_comerr_visit NULL /* Could only really visit strings, so there's no point */
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
#define PTR_comerr_visit &comerr_visit
PRIVATE NONNULL((1, 2)) void DCALL
comerr_visit(DeeCompilerErrorObject *__restrict self,
             Dee_visit_t proc, void *arg) {
	size_t i, count = self->ce_errorc;
	for (i = 0; i < count; ++i) {
		if (self->ce_errorv[i] != self)
			Dee_Visit(self->ce_errorv[i]);
	}
}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
comerr_print(DeeCompilerErrorObject *__restrict self,
             Dee_formatprinter_t printer, void *arg) {
#if 0 /* This breaks printing of compiler warnings */
	if (self->e_msg)
		return DeeObject_Print(self->e_msg, printer, arg);
#endif
	return DeeCompilerError_Print(Dee_AsObject(self), printer, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
comerr_printrepr(DeeCompilerErrorObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	/* TODO */
	return DeeStructObject_PrintRepr(Dee_AsObject(self), printer, arg);
}

#define comerr_cmp DeeStructObject_Cmp /* TODO */

PRIVATE struct type_member tpconst compiler_error_class_members[] = {
	TYPE_MEMBER_CONST("SyntaxError", &DeeError_SyntaxError),
	TYPE_MEMBER_CONST("SymbolError", &DeeError_SymbolError),
	TYPE_MEMBER_END
};


#define CompilerError_init_params Error_init_params /* ",TODO" */
PUBLIC DeeTypeObject DeeError_CompilerError = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "CompilerError",
	/* .tp_doc      = */ DOC("(" CompilerError_init_params ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_Error,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCompilerErrorObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &comerr_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &comerr_init_kw,
			/* tp_serialize:   */ &comerr_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&comerr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&comerr_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&comerr_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))PTR_comerr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &comerr_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO */
	/* .tp_getsets       = */ NULL, /* TODO */
	/* .tp_members       = */ NULL, /* TODO */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ compiler_error_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#define INIT_LIKE_COMPILE_ERROR(tp_name, tp_doc, tp_flags,                \
                                tp_base, tp_str, tp_print,                \
                                tp_methods, tp_getsets, tp_class_members) \
	{                                                                     \
		OBJECT_HEAD_INIT(&DeeType_Type),                                  \
		/* .tp_name     = */ tp_name,                                     \
		/* .tp_doc      = */ DOC(tp_doc),                                 \
		/* .tp_flags    = */ TP_FNORMAL | (tp_flags),                     \
		/* .tp_weakrefs = */ 0,                                           \
		/* .tp_features = */ TF_NONE,                                     \
		/* .tp_base     = */ tp_base,                                     \
		/* .tp_init = */ {                                                \
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(                              \
				/* T:              */ DeeCompilerErrorObject,             \
				/* tp_ctor:        */ NULL,                               \
				/* tp_copy_ctor:   */ &comerr_copy,                       \
				/* tp_any_ctor:    */ NULL,                               \
				/* tp_any_ctor_kw: */ &comerr_init_kw,                    \
				/* tp_serialize:   */ &comerr_serialize                   \
			),                                                            \
			/* .tp_dtor        = */ NULL,                                 \
			/* .tp_assign      = */ NULL,                                 \
			/* .tp_move_assign = */ NULL                                  \
		},                                                                \
		/* .tp_cast = */ {                                                \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ NULL,                                   \
			/* .tp_bool      = */ NULL,                                   \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&comerr_printrepr, \
		},                                                                \
		/* .tp_visit         = */ NULL,                                   \
		/* .tp_gc            = */ NULL,                                   \
		/* .tp_math          = */ NULL,                                   \
		/* .tp_cmp           = */ &comerr_cmp,                            \
		/* .tp_seq           = */ NULL,                                   \
		/* .tp_iter_next     = */ NULL,                                   \
		/* .tp_iterator      = */ NULL,                                   \
		/* .tp_attr          = */ NULL,                                   \
		/* .tp_with          = */ NULL,                                   \
		/* .tp_buffer        = */ NULL,                                   \
		/* .tp_methods       = */ tp_methods,                             \
		/* .tp_getsets       = */ tp_getsets,                             \
		/* .tp_members       = */ NULL,                                   \
		/* .tp_class_methods = */ NULL,                                   \
		/* .tp_class_getsets = */ NULL,                                   \
		/* .tp_class_members = */ tp_class_members                        \
	}                                                                     \

/************************************************************************/
/* Error.CompilerError.SyntaxError                                      */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_SyntaxError =
INIT_LIKE_COMPILE_ERROR("SyntaxError", "(" CompilerError_init_params ")",
                        TP_FNORMAL, &DeeError_CompilerError, NULL, &DeeCompilerError_Print,
                        NULL, NULL, NULL);


/************************************************************************/
/* Error.CompilerError.SymbolError                                      */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_SymbolError =
INIT_LIKE_COMPILE_ERROR("SymbolError", "(" CompilerError_init_params ")",
                        TP_FNORMAL, &DeeError_CompilerError, NULL, &DeeCompilerError_Print,
                        NULL, NULL, NULL);

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ERROR_RT_COMPILER_C */
