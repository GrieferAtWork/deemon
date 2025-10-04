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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_RT_COMPILER_C
#define GUARD_DEEMON_RUNTIME_ERROR_RT_COMPILER_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/tpp.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/struct.h>
#include <deemon/system-features.h>
#include <deemon/variant.h>

#include <hybrid/int128.h>
#include <hybrid/typecore.h>
/**/

#include "strings.h"
/**/

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#define Error_init_params "message:?X2?Dstring?N=!N,inner:?X3?DError?O?N=!N"
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
error_print(DeeErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg);



/************************************************************************/
/* Error.CompilerError                                                  */
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
comerr_init_common(DeeCompilerErrorObject *__restrict self) {
	weakref_support_init(self);
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
comerr_deep(DeeCompilerErrorObject *__restrict self,
            DeeCompilerErrorObject *__restrict other) {
	/* TODO */
	(void)self;
	(void)other;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comerr_init_kw(DeeCompilerErrorObject *__restrict self,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/* TODO: Initialization for CompilerError-specific fields */
	int result = (*DeeError_Error.tp_init.tp_alloc.tp_any_ctor_kw)((DeeObject *)self, argc, argv, kw);
	if likely(result == 0)
		comerr_init_common(self);
	return result;
}

PRIVATE NONNULL((1)) void DCALL
comerr_fini(DeeCompilerErrorObject *__restrict self) {
	size_t i, count = self->ce_errorc;
	ASSERTF(self->ce_locs.cl_prev == NULL ||
	        self->ce_locs.cl_file != NULL,
	        "More than one location requires a base file to be present");
	if (self->ce_locs.cl_file) {
		/* Cleanup saved file locations. */
		struct compiler_error_loc *next, *iter;
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
}

PRIVATE NONNULL((1, 2)) void DCALL
comerr_visit(DeeCompilerErrorObject *__restrict self,
             Dee_visit_t proc, void *arg) {
	size_t i, count = self->ce_errorc;
	for (i = 0; i < count; ++i) {
		if (self->ce_errorv[i] != self)
			Dee_Visit(self->ce_errorv[i]);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
comerr_printrepr(DeeCompilerErrorObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	/* TODO */
	return DeeStructObject_PrintRepr((DeeObject *)self, printer, arg);
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&comerr_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&comerr_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCompilerErrorObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&comerr_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&comerr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ &DeeCompilerError_Print, /* TODO: Print "e_message" if non-NULL */
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&comerr_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&comerr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &comerr_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO */
	/* .tp_getsets       = */ NULL, /* TODO */
	/* .tp_members       = */ NULL, /* TODO */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ compiler_error_class_members
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
			{                                                             \
				/* .tp_alloc = */ {                                       \
					/* .tp_ctor      = */ (Dee_funptr_t)NULL,             \
					/* .tp_copy_ctor = */ (Dee_funptr_t)&comerr_copy,     \
					/* .tp_deep_ctor = */ (Dee_funptr_t)&comerr_deep,     \
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,             \
					TYPE_FIXED_ALLOCATOR(DeeCompilerErrorObject),         \
					/* .tp_any_ctor_kw = */ (Dee_funptr_t)&comerr_init_kw,\
				}                                                         \
			},                                                            \
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
