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
#ifndef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
#define GUARD_DEEMON_OBJECTS_ERROR_TYPES_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/compiler/tpp.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/exec.h>
#include <deemon/format.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/struct.h>
#include <deemon/system-features.h> /* bzero(), ... */
#include <deemon/system.h>
#include <deemon/tuple.h>

#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* size_t, offsetof */
/**/

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"

#undef token
#undef tok
#undef yield
#undef yieldnb
#undef yieldnbif
#undef skip

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

/* TODO: Migrate everything from in here into "error-rt.c" */


/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("message");
print define_Dee_HashStr("inner");
]]]*/
#define Dee_HashStr__message _Dee_HashSelectC(0x14820755, 0xbeaa4b97155366df)
#define Dee_HashStr__inner _Dee_HashSelectC(0x20e82985, 0x4f20d07bb803c1fe)
/*[[[end]]]*/


#define INIT_CUSTOM_ERROR(tp_name, tp_doc, tp_flags,                        \
                          tp_base, T, tp_str, tp_print,                     \
                          tp_methods, tp_getsets, tp_members,               \
                          tp_class_members)                                 \
	INIT_CUSTOM_ERROR_EX(tp_name, tp_doc, tp_flags, TF_TPVISIT, tp_base, T, \
	                     &DeeStructObject_Ctor, &DeeStructObject_Copy,      \
	                     &DeeStructObject_Deep, &DeeStructObject_Init,      \
	                     &DeeStructObject_InitKw, &DeeStructObject_Fini,    \
	                     &DeeStructObject_Visit, &DeeStructObject_Cmp,      \
	                     tp_str, tp_print,                                  \
	                     tp_methods, tp_getsets, tp_members,                \
	                     tp_class_members)

/* Initialize an error type that uses `DeeErrorObject' as its struct type */
#define INIT_LIKE_ERROR(tp_name, tp_doc, tp_flags,                          \
                        tp_base, tp_str, tp_print,                          \
                        tp_methods, tp_getsets, tp_class_members)           \
	INIT_CUSTOM_ERROR_EX(tp_name, tp_doc, tp_flags, TF_NONE,                \
	                     tp_base, DeeErrorObject, &error_ctor, &error_copy, \
	                     &error_deep, &error_init, &error_init_kw,          \
	                     NULL, NULL, NULL, tp_str, tp_print,                \
	                     tp_methods, tp_getsets, NULL,                      \
	                     tp_class_members)
#define INIT_CUSTOM_ERROR_EX(tp_name, tp_doc, tp_flags, tp_features,                                   \
                             tp_base, T, tp_ctor, tp_copy, tp_deep, tp_init,                           \
                             tp_init_kw, tp_fini, tp_visit, tp_cmp,                                    \
                             tp_str, tp_print,                                                         \
                             tp_methods, tp_getsets, tp_members,                                       \
                             tp_class_members)                                                         \
	{                                                                                                  \
		OBJECT_HEAD_INIT(&DeeType_Type),                                                               \
		/* .tp_name     = */ tp_name,                                                                  \
		/* .tp_doc      = */ DOC(tp_doc),                                                              \
		/* .tp_flags    = */ tp_flags,                                                                 \
		/* .tp_weakrefs = */ 0,                                                                        \
		/* .tp_features = */ TF_NONE | (tp_features),                                                  \
		/* .tp_base     = */ tp_base,                                                                  \
		/* .tp_init = */ {                                                                             \
			{                                                                                          \
				/* .tp_alloc = */ {                                                                    \
					/* .tp_ctor      = */ (Dee_funptr_t)(tp_ctor),                                     \
					/* .tp_copy_ctor = */ (Dee_funptr_t)(tp_copy),                                     \
					/* .tp_deep_ctor = */ (Dee_funptr_t)(tp_deep),                                     \
					/* .tp_any_ctor  = */ (Dee_funptr_t)(tp_init),                                     \
					TYPE_FIXED_ALLOCATOR(T),                                                           \
					/* .tp_any_ctor_kw = */ (Dee_funptr_t)(tp_init_kw)                                 \
				}                                                                                      \
			},                                                                                         \
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))(tp_fini),                  \
			/* .tp_assign      = */ NULL,                                                              \
			/* .tp_move_assign = */ NULL                                                               \
		},                                                                                             \
		/* .tp_cast = */ {                                                                             \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str),          \
			/* .tp_repr      = */ NULL,                                                                \
			/* .tp_bool      = */ NULL,                                                                \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ &DeeStructObject_PrintRepr,                                          \
		},                                                                                             \
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))(tp_visit), \
		/* .tp_gc            = */ NULL,                                                                \
		/* .tp_math          = */ NULL,                                                                \
		/* .tp_cmp           = */ tp_cmp,                                                              \
		/* .tp_seq           = */ NULL,                                                                \
		/* .tp_iter_next     = */ NULL,                                                                \
		/* .tp_iterator      = */ NULL,                                                                \
		/* .tp_attr          = */ NULL,                                                                \
		/* .tp_with          = */ NULL,                                                                \
		/* .tp_buffer        = */ NULL,                                                                \
		/* .tp_methods       = */ tp_methods,                                                          \
		/* .tp_getsets       = */ tp_getsets,                                                          \
		/* .tp_members       = */ tp_members,                                                          \
		/* .tp_class_methods = */ NULL,                                                                \
		/* .tp_class_getsets = */ NULL,                                                                \
		/* .tp_class_members = */ tp_class_members                                                     \
	}



/* BEGIN::Error */
PRIVATE struct type_member tpconst error_class_members[] = {
	TYPE_MEMBER_CONST("AttributeError", &DeeError_AttributeError),
	TYPE_MEMBER_CONST("CompilerError", &DeeError_CompilerError),
	TYPE_MEMBER_CONST("ThreadCrash", &DeeError_ThreadCrash),
	TYPE_MEMBER_CONST("NoMemory", &DeeError_NoMemory),
	TYPE_MEMBER_CONST("RuntimeError", &DeeError_RuntimeError),
	TYPE_MEMBER_CONST("TypeError", &DeeError_TypeError),
	TYPE_MEMBER_CONST("ValueError", &DeeError_ValueError),
	TYPE_MEMBER_CONST("SystemError", &DeeError_SystemError),
	TYPE_MEMBER_CONST("AppExit", &DeeError_AppExit),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
error_str(DeeErrorObject *__restrict self) {
	if (self->e_message)
		return_reference_((DeeObject *)self->e_message);
	if (self->e_inner)
		return DeeString_Newf("%k -> %k", Dee_TYPE(self), self->e_inner);
	return DeeObject_Str((DeeObject *)Dee_TYPE(self));
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
type_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
error_print(DeeErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	if (self->e_message)
		return DeeString_PrintUtf8((DeeObject *)self->e_message, printer, arg);
	if (self->e_inner)
		return DeeFormat_Printf(printer, arg, "%k -> %k", Dee_TYPE(self), self->e_inner);
	return type_print((DeeObject *)Dee_TYPE(self), printer, arg);
}

#ifdef __OPTIMIZE_SIZE__
#define error_fini        DeeStructObject_Fini
#define error_visit       DeeStructObject_Visit
#define error_tp_features TF_TPVISIT
#define error_ctor        DeeStructObject_Ctor
#define error_copy        DeeStructObject_Copy
#define error_deep        DeeStructObject_Deep
#define error_init        DeeStructObject_Init
#define error_init_kw     DeeStructObject_InitKw
#else /* __OPTIMIZE_SIZE__ */
#define error_tp_features TF_NONE
PRIVATE NONNULL((1)) void DCALL
error_fini(DeeObject *__restrict self) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	Dee_XDecref(me->e_message);
	Dee_XDecref(me->e_inner);
}

PRIVATE NONNULL((1, 2)) void DCALL
error_visit(DeeObject *__restrict self, Dee_visit_t proc, void *arg) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	Dee_XVisit(me->e_message);
	Dee_XVisit(me->e_inner);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
error_ctor(DeeObject *__restrict self) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	me->e_message = NULL;
	me->e_inner   = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
error_copy(DeeObject *__restrict self, DeeObject *__restrict other_ob) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	DeeErrorObject *other = (DeeErrorObject *)other_ob;
	me->e_message = other->e_message;
	me->e_inner   = other->e_inner;
	Dee_XIncref(me->e_message);
	Dee_XIncref(me->e_inner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
error_deep(DeeObject *__restrict self, DeeObject *__restrict other_ob) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	DeeErrorObject *other = (DeeErrorObject *)other_ob;
	me->e_inner = NULL;
	if (other->e_inner) {
		me->e_inner = DeeObject_DeepCopy(other->e_inner);
		if unlikely(!me->e_inner)
			goto err;
	}
	me->e_message = other->e_message;
	Dee_XIncref(me->e_message);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
error_init(DeeObject *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	me->e_message = NULL;
	me->e_inner   = NULL;
	_DeeArg_Unpack0Or1Or2(err, argc, argv, Dee_TYPE(me)->tp_name,
	                      &me->e_message, &me->e_inner);
	if (me->e_message) {
		if (DeeObject_AssertTypeExact(me->e_message, &DeeString_Type))
			goto err;
		Dee_Incref(me->e_message);
	}
	Dee_XIncref(me->e_inner);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
error_init_kw(DeeObject *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	DeeKwArgs kwds;
	if unlikely(!kw)
		return error_init((DeeObject *)me, argc, argv);
	if unlikely(DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
	switch (argc) {
	case 0:
		me->e_message = (DeeStringObject *)DeeKwArgs_TryGetItemNRStringHash(&kwds, "message", Dee_HashStr__message);
		if unlikely(!me->e_message)
			goto err;
		if (me->e_message == (DeeStringObject *)ITER_DONE)
			me->e_message = (DeeStringObject *)NULL;
		__IF0 {
	case 1:
			me->e_message = (DeeStringObject *)argv[0];
		}
		me->e_inner = DeeKwArgs_TryGetItemNRStringHash(&kwds, "inner", Dee_HashStr__inner);
		if unlikely(!me->e_inner)
			goto err;
		if (me->e_inner == ITER_DONE)
			me->e_inner = NULL;
		break;
	case 2:
		me->e_message = (DeeStringObject *)argv[0];
		me->e_inner   = argv[1];
		break;
	default:
		return DeeArg_BadArgcEx(Dee_TYPE(me)->tp_name, argc, 0, 2);
	}
	Dee_XIncref(me->e_message);
	Dee_XIncref(me->e_inner);
	if unlikely(DeeKwArgs_Done(&kwds, argc, Dee_TYPE(me)->tp_name))
		goto err_self;
	return 0;
err_self:
	Dee_XDecref_unlikely(me->e_inner);
	Dee_XDecref_unlikely(me->e_message);
err:
	return -1;
}
#endif /* !__OPTIMIZE_SIZE__ */


PRIVATE struct type_member tpconst error_members[] = {
#define Error_init_params "message:?X2?Dstring?N=!N,inner:?X3?DError?O?N=!N"
	TYPE_MEMBER_FIELD_DOC("message", STRUCT_OBJECT_OPT, offsetof(DeeErrorObject, e_message),
	                      "->?X2?Dstring?N\n"
	                      "The error message associated with this Error object, or ?N when not set"),
	TYPE_MEMBER_FIELD_DOC("inner", STRUCT_OBJECT_OPT, offsetof(DeeErrorObject, e_inner),
	                      "->?X3?DError?O?N\n"
	                      "An optional inner error object, or ?N when not set"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_Error = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Error),
	/* .tp_doc      = */ DOC("Base class for all errors thrown by the runtime\n"
	                         "\n"

	                         "(" Error_init_params ")\n"
	                         "Create a new error object with the given @message and @inner error"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ error_tp_features,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)&error_ctor,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)&error_copy,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)&error_deep,
				/* .tp_any_ctor    = */ (Dee_funptr_t)&error_init,
				TYPE_FIXED_ALLOCATOR(DeeErrorObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&error_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&error_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&error_str,
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&error_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&DeeStructObject_PrintRepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&error_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ error_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ error_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};
/* END::Error */











/* BEGIN::AttributeError */
PRIVATE struct type_member tpconst attribute_error_class_members[] = {
	TYPE_MEMBER_CONST("UnboundAttribute", &DeeError_UnboundAttribute),
	TYPE_MEMBER_END
};
#define AttributeError_init_params Error_init_params
PUBLIC DeeTypeObject DeeError_AttributeError =
INIT_LIKE_ERROR("AttributeError", "(" AttributeError_init_params ")",
                TP_FNORMAL, &DeeError_Error, NULL, NULL,
                NULL, NULL, attribute_error_class_members);
PUBLIC DeeTypeObject DeeError_UnboundAttribute =
INIT_LIKE_ERROR("UnboundAttribute", "(" AttributeError_init_params ")",
                TP_FNORMAL, &DeeError_AttributeError, NULL, NULL,
                NULL, NULL, NULL);
/* END::AttributeError */






/* BEGIN::CompilerError */
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
comerr_ctor(DeeCompilerErrorObject *__restrict self) {
	int result = error_ctor((DeeObject *)self);
	if likely(result == 0)
		comerr_init_common(self);
	return result;
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
comerr_init(DeeCompilerErrorObject *__restrict self,
            size_t argc, DeeObject *const *argv) {
	/* TODO: Initialization for CompilerError-specific fields */
	int result = error_init((DeeObject *)self, argc, argv);
	if likely(result == 0)
		comerr_init_common(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comerr_init_kw(DeeCompilerErrorObject *__restrict self,
               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/* TODO: Initialization for CompilerError-specific fields */
	int result = error_init_kw((DeeObject *)self, argc, argv, kw);
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
				/* .tp_ctor      = */ (Dee_funptr_t)&comerr_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&comerr_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&comerr_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&comerr_init,
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
		/* .tp_print     = */ &DeeCompilerError_Print,
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
					/* .tp_ctor      = */ (Dee_funptr_t)&comerr_ctor,     \
					/* .tp_copy_ctor = */ (Dee_funptr_t)&comerr_copy,     \
					/* .tp_deep_ctor = */ (Dee_funptr_t)&comerr_deep,     \
					/* .tp_any_ctor  = */ (Dee_funptr_t)&comerr_init,     \
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

PUBLIC DeeTypeObject DeeError_SyntaxError =
INIT_LIKE_COMPILE_ERROR("SyntaxError", "(" CompilerError_init_params ")",
                        TP_FNORMAL, &DeeError_CompilerError, NULL, &DeeCompilerError_Print,
                        NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_SymbolError =
INIT_LIKE_COMPILE_ERROR("SymbolError", "(" CompilerError_init_params ")",
                        TP_FNORMAL, &DeeError_CompilerError, NULL, &DeeCompilerError_Print,
                        NULL, NULL, NULL);
/* END::CompilerError */






/* BEGIN::ThreadCrash */
#define ThreadCrash_init_params Error_init_params
PUBLIC DeeTypeObject DeeError_ThreadCrash =
INIT_LIKE_ERROR("ThreadCrash", "(" ThreadCrash_init_params ")",
                TP_FNORMAL, &DeeError_Error, NULL, NULL,
                NULL, NULL, NULL);
/* END::ThreadCrash */




/* BEGIN::NoMemory */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nomemory_str(DeeNoMemoryErrorObject *__restrict self) {
	if (self->nm_allocsize) {
		return DeeString_Newf("Failed to allocated %" PRFuSIZ " bytes",
		                      self->nm_allocsize);
	}
	return error_str((DeeErrorObject *)self);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
nomemory_print(DeeNoMemoryErrorObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	if (self->nm_allocsize) {
		return DeeFormat_Printf(printer, arg,
		                        "Failed to allocated %" PRFuSIZ " bytes",
		                        self->nm_allocsize);
	}
	return error_print((DeeErrorObject *)self, printer, arg);
}

PRIVATE struct type_member tpconst nomemory_members[] = {
#define NoMemory_init_params Error_init_params ",size=!0"
	TYPE_MEMBER_FIELD(STR_size, STRUCT_SIZE_T, offsetof(DeeNoMemoryErrorObject, nm_allocsize)),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_NoMemory =
INIT_CUSTOM_ERROR("NoMemory", "(" NoMemory_init_params ")",
                  TP_FNORMAL, &DeeError_Error, DeeNoMemoryErrorObject,
                  &nomemory_str, &nomemory_print,
                  NULL, NULL, nomemory_members, NULL);
/* END::NoMemory */




/* BEGIN::RuntimeError */
PRIVATE struct type_member tpconst runtimeerror_class_members[] = {
	TYPE_MEMBER_CONST("NotImplemented", &DeeError_NotImplemented),
	TYPE_MEMBER_CONST("AssertionError", &DeeError_AssertionError),
	TYPE_MEMBER_CONST("UnboundLocal", &DeeError_UnboundLocal),
	TYPE_MEMBER_CONST("StackOverflow", &DeeError_StackOverflow),
	TYPE_MEMBER_CONST("SegFault", &DeeError_SegFault),
	TYPE_MEMBER_CONST("IllegalInstruction", &DeeError_IllegalInstruction),
	TYPE_MEMBER_END
};
#define RuntimeError_init_params Error_init_params
PUBLIC DeeTypeObject DeeError_RuntimeError =
INIT_LIKE_ERROR("RuntimeError", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_Error, NULL, NULL,
                NULL, NULL, runtimeerror_class_members);
PUBLIC DeeTypeObject DeeError_NotImplemented =
INIT_LIKE_ERROR("NotImplemented", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_RuntimeError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_AssertionError = /* TODO: Include details about the failing assert expression */
INIT_LIKE_ERROR("AssertionError", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_RuntimeError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_UnboundLocal = /* TODO: Include the Code/lid */
INIT_LIKE_ERROR("UnboundLocal", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_RuntimeError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_StackOverflow =
INIT_LIKE_ERROR("StackOverflow", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_RuntimeError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_SegFault =
INIT_LIKE_ERROR("SegFault", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_RuntimeError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_IllegalInstruction = /* TODO: Include the illegal instruction in question */
INIT_LIKE_ERROR("IllegalInstruction", "(" RuntimeError_init_params ")",
                TP_FNORMAL, &DeeError_RuntimeError, NULL, NULL,
                NULL, NULL, NULL);
/* END::RuntimeError */




/* BEGIN::TypeError */
PUBLIC DeeTypeObject DeeError_TypeError =
INIT_LIKE_ERROR("IllegalInstruction", "(" Error_init_params ")",
                TP_FNORMAL, &DeeError_Error, NULL, NULL,
                NULL, NULL, NULL);
/* END::TypeError */




/* BEGIN::ValueError */
PRIVATE struct type_member tpconst valueerror_class_members[] = {
	TYPE_MEMBER_CONST("ArithmeticError", &DeeError_ArithmeticError),
	TYPE_MEMBER_CONST("KeyError", &DeeError_KeyError),
	TYPE_MEMBER_CONST("IndexError", &DeeError_IndexError),
	TYPE_MEMBER_CONST("SequenceError", &DeeError_SequenceError),
	TYPE_MEMBER_CONST("UnicodeError", &DeeError_UnicodeError),
	TYPE_MEMBER_CONST("ReferenceError", &DeeError_ReferenceError),
	TYPE_MEMBER_CONST("UnpackError", &DeeError_UnpackError),
	TYPE_MEMBER_CONST("BufferError", &DeeError_BufferError),
	TYPE_MEMBER_END
};
#define ValueError_init_params Error_init_params
PUBLIC DeeTypeObject DeeError_ValueError =
INIT_LIKE_ERROR("ValueError", "(" ValueError_init_params ")",
                TP_FNORMAL, &DeeError_Error, NULL, NULL,
                NULL, NULL, valueerror_class_members);

/* BEGIN::ValueError.ArithmeticError */
PRIVATE struct type_member tpconst arithmetic_class_members[] = {
	TYPE_MEMBER_CONST("IntegerOverflow", &DeeError_IntegerOverflow),
	TYPE_MEMBER_CONST("DivideByZero", &DeeError_DivideByZero),
	TYPE_MEMBER_CONST("NegativeShift", &DeeError_NegativeShift),
	TYPE_MEMBER_END
};
#define ArithmeticError_init_params ValueError_init_params
PUBLIC DeeTypeObject DeeError_ArithmeticError =
INIT_LIKE_ERROR("ArithmeticError", "(" ArithmeticError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,

                                NULL, NULL, arithmetic_class_members);
PUBLIC DeeTypeObject DeeError_DivideByZero =
INIT_LIKE_ERROR("DivideByZero", "(" ArithmeticError_init_params ")",
                TP_FNORMAL, &DeeError_ArithmeticError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_NegativeShift =
INIT_LIKE_ERROR("NegativeShift", "(" ArithmeticError_init_params ")",
                TP_FNORMAL, &DeeError_ArithmeticError, NULL, NULL,
                NULL, NULL, NULL);
/* END::ValueError.ArithmeticError */
PUBLIC DeeTypeObject DeeError_KeyError =
INIT_LIKE_ERROR("KeyError", "(" ValueError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,
                NULL, NULL, NULL);
/* BEGIN::ValueError.IndexError */
PRIVATE struct type_member tpconst indexerror_class_members[] = {
	TYPE_MEMBER_CONST("UnboundItem", &DeeError_UnboundItem),
	TYPE_MEMBER_END
};
#define IndexError_init_params ValueError_init_params
PUBLIC DeeTypeObject DeeError_IndexError =
INIT_LIKE_ERROR("IndexError", "(" IndexError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,
                NULL, NULL, indexerror_class_members);
PUBLIC DeeTypeObject DeeError_UnboundItem =
INIT_LIKE_ERROR("UnboundItem", "(" IndexError_init_params ")",
                TP_FNORMAL, &DeeError_IndexError, NULL, NULL,
                NULL, NULL, NULL);
/* END::ValueError.IndexError */
PUBLIC DeeTypeObject DeeError_SequenceError =
INIT_LIKE_ERROR("SequenceError", "(" ValueError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,
                NULL, NULL, NULL);
/* BEGIN::ValueError.UnicodeError */
PRIVATE struct type_member tpconst unicodeerror_class_members[] = {
	TYPE_MEMBER_CONST("UnicodeDecodeError", &DeeError_UnicodeDecodeError),
	TYPE_MEMBER_CONST("UnicodeEncodeError", &DeeError_UnicodeEncodeError),
	TYPE_MEMBER_END
};
#define UnicodeError_init_params ValueError_init_params
PUBLIC DeeTypeObject DeeError_UnicodeError =
INIT_LIKE_ERROR("UnicodeError", "(" UnicodeError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,

                                NULL, NULL, unicodeerror_class_members);
PUBLIC DeeTypeObject DeeError_UnicodeDecodeError =
INIT_LIKE_ERROR("UnicodeDecodeError", "(" UnicodeError_init_params ")",
                TP_FNORMAL, &DeeError_UnicodeError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_UnicodeEncodeError =
INIT_LIKE_ERROR("UnicodeEncodeError", "(" UnicodeError_init_params ")",
                TP_FNORMAL, &DeeError_UnicodeError, NULL, NULL,
                NULL, NULL, NULL);
/* END::ValueError.UnicodeError */
PUBLIC DeeTypeObject DeeError_ReferenceError =
INIT_LIKE_ERROR("ReferenceError", "(" ValueError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_UnpackError =
INIT_LIKE_ERROR("UnpackError", "(" ValueError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,
                NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_BufferError =
INIT_LIKE_ERROR("BufferError", "(" ValueError_init_params ")",
                TP_FNORMAL, &DeeError_ValueError, NULL, NULL,
                NULL, NULL, NULL);
/* END::ValueError */





/* BEGIN::SystemError */
PRIVATE struct type_member tpconst systemerror_class_members[] = {
	TYPE_MEMBER_CONST("UnsupportedAPI", &DeeError_UnsupportedAPI),
	TYPE_MEMBER_CONST("FSError", &DeeError_FSError),
	TYPE_MEMBER_CONST_DOC("IOError", &DeeError_FSError, "Deprecated alias for ?#FSError"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
systemerror_init_kw(DeeSystemErrorObject *__restrict self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DeeObject *obj_errno = NULL;
#ifdef CONFIG_HOST_WINDOWS
	DeeObject *obj_nterr_np = NULL;
	DWORD dwLastError = GetLastError();
	self->e_inner   = NULL;
	self->e_message = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw,
	                    kwlist__message_inner_errno_nterr_np,
	                    "|oooo:SystemError",
	                    &self->e_message, &self->e_inner,
	                    &obj_errno, &obj_nterr_np))
		goto err;
	if (obj_nterr_np) {
		if (DeeObject_AsUInt32(obj_nterr_np, &self->se_lasterror))
			goto err;
		if (obj_errno) {
			if (DeeObject_AsInt(obj_errno, &self->se_errno))
				goto err;
		} else {
			self->se_errno = DeeNTSystem_TranslateErrno(self->se_lasterror);
		}
	} else if (obj_errno) {
		if (DeeObject_AsInt(obj_errno, &self->se_errno))
			goto err;
		self->se_lasterror = DeeNTSystem_TranslateNtError(self->se_errno);
	} else {
		self->se_lasterror = dwLastError;
		self->se_errno     = DeeNTSystem_TranslateErrno(dwLastError);
	}
#else /* CONFIG_HOST_WINDOWS */
	int last_errno = DeeSystem_GetErrno();
	self->e_inner   = NULL;
	self->e_message = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw,
	                    kwlist__message_inner_errno,
	                    "|ooo:SystemError",
	                    &self->e_message, &self->e_inner,
	                    &obj_errno))
		goto err;
	if (obj_errno) {
		if (DeeObject_AsInt(obj_errno, &self->se_errno))
			goto err;
	} else {
		self->se_errno = last_errno;
	}
#endif /* !CONFIG_HOST_WINDOWS */
	Dee_XIncref(self->e_message);
	Dee_XIncref(self->e_inner);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_call_posix_function(DeeSystemErrorObject *__restrict self,
                                char const *__restrict name) {
	return DeeModule_CallExternStringf("posix", name, "d", self->se_errno);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_getstrerrorname(DeeSystemErrorObject *__restrict self) {
	return systemerror_call_posix_function(self, "strerrorname");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_getstrerror(DeeSystemErrorObject *__restrict self) {
	return systemerror_call_posix_function(self, "strerror");
}

#ifdef CONFIG_HOST_WINDOWS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_getnterrmsg_np(DeeSystemErrorObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeNTSystem_FormatErrorMessage(self->se_lasterror);
	ASSERT(!result || DeeString_Check(result));
	if (result && DeeString_IsEmpty(result)) {
		Dee_Decref_unlikely(result);
		result = DeeNone_NewRef();
	}
	return result;
}
#endif /* CONFIG_HOST_WINDOWS */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
systemerror_print(DeeSystemErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	DREF DeeObject *errno_name, *errno_desc;
	result = DeeObject_Print(self->e_message
	                         ? (DeeObject *)self->e_message
	                         : (DeeObject *)Dee_TYPE(self),
	                         printer, arg);
	if unlikely(result < 0)
		goto done;
	if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
		errno_name = systemerror_getstrerrorname(self);
		if unlikely(!errno_name) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL)) {
				temp = -1;
				goto err;
			}
			errno_name = DeeNone_NewRef();
		}
		errno_desc = systemerror_getstrerror(self);
		if unlikely(!errno_desc) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL)) {
				temp = -1;
				goto err_errno_name;
			}
			errno_desc = DeeNone_NewRef();
		}
		DO(err_errno_desc, DeeFormat_Printf(printer, arg, "\nerrno(%d)", self->se_errno));
		if (!DeeNone_Check(errno_name) || !DeeNone_Check(errno_desc)) {
#ifdef CONFIG_HOST_WINDOWS
			if (self->se_lasterror != NO_ERROR) {
				size_t unix_code_length;
				size_t windows_code_length;
				unix_code_length    = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%d", self->se_errno);
				windows_code_length = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%#I32x", self->se_lasterror);
				if (windows_code_length > unix_code_length) {
					DO(err_errno_desc, DeeFormat_Repeat(printer, arg, ' ',
					                                    windows_code_length -
					                                    unix_code_length));
				}
			}
#endif /* CONFIG_HOST_WINDOWS */
			DO(err_errno_desc, DeeFormat_PRINT(printer, arg, ": "));
			DO(err_errno_desc,
			   DeeNone_Check(errno_name) ? DeeFormat_PRINT(printer, arg, "?")
			                             : DeeFormat_PrintObject(printer, arg, errno_name));
			if (!DeeNone_Check(errno_desc)) {
				DO(err_errno_desc, DeeFormat_PRINT(printer, arg, " ("));
				DO(err_errno_desc, DeeFormat_PrintObject(printer, arg, errno_desc));
				DO(err_errno_desc, DeeFormat_PRINT(printer, arg, ")"));
			}
		}
		Dee_Decref(errno_desc);
		Dee_Decref(errno_name);
	}

#ifdef CONFIG_HOST_WINDOWS
	if (self->se_lasterror != NO_ERROR) {
		bool success;
		DO(err, DeeFormat_Printf(printer, arg, "\nnterr(%#I32x)", self->se_lasterror));
		if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
			size_t unix_code_length;
			size_t windows_code_length;
			unix_code_length    = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%d", self->se_errno);
			windows_code_length = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%#I32x", self->se_lasterror);
			if (unix_code_length > windows_code_length) {
				DO(err, DeeFormat_Repeat(printer, arg, ' ',
				                         unix_code_length -
				                         windows_code_length));
			}
		}
		DO(err, DeeFormat_PRINT(printer, arg, ": "));
		DO(err, DeeNTSystem_PrintFormatMessage(printer, arg, FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		                                       (DWORD)self->se_lasterror,
		                                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		                                       NULL, &success));
		if unlikely(!success) {
			/* No message string printed... */
			DO(err, DeeFormat_PRINT(printer, arg, "?"));
		}
	}
#endif /* CONFIG_HOST_WINDOWS */
done:
	return result;
err_errno_desc:
	Dee_Decref(errno_desc);
err_errno_name:
	Dee_Decref(errno_name);
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
systemerror_printrepr(DeeSystemErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	bool is_first = true;
	result = DeeFormat_Printf(printer, arg, "%k(", Dee_TYPE(self));
	if unlikely(result < 0)
		goto done;
	if (self->e_message) {
		DO(err, DeeString_PrintRepr((DeeObject *)self->e_message, printer, arg));
		is_first = false;
	}
	if (self->e_inner) {
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "inner: %r", self->e_inner));
		is_first = false;
	}
	if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
		DREF DeeObject *errno_name;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_PRINT(printer, arg, "errno: "));
		errno_name = systemerror_getstrerrorname(self);
		if (!errno_name) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
				goto err;
			errno_name = DeeNone_NewRef();
		}
		temp = DeeNone_Check(errno_name)
		       ? DeeFormat_Printf(printer, arg, "%d", self->se_errno)
		       : DeeFormat_Printf(printer, arg, "posix.%k", errno_name);
		Dee_Decref(errno_name);
		if unlikely(temp < 0)
			goto err;
		result += temp;
#ifdef CONFIG_HOST_WINDOWS
		is_first = false;
#endif /* CONFIG_HOST_WINDOWS */
	}
#ifdef CONFIG_HOST_WINDOWS
	if (self->se_lasterror != NO_ERROR) {
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "nterr_np: %#I32x", self->se_lasterror));
	}
#endif /* CONFIG_HOST_WINDOWS */
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
}

#undef DO

PRIVATE struct type_getset tpconst systemerror_getsets[] = {
	TYPE_GETTER_AB_F("strerrorname", &systemerror_getstrerrorname, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "The name of the associated ?#errno (s.a. ?Eposix:strerrorname)\n"
	                 "Returns ?N if ?#errno doesn't have a known name"),
	TYPE_GETTER_AB_F("strerror", &systemerror_getstrerror, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "A description of the associated ?#errno (s.a. ?Eposix:strerror)\n"
	                 "Returns ?N if ?#errno doesn't have a description"),
#ifdef CONFIG_HOST_WINDOWS
	TYPE_GETTER_AB_F("nterrmsg_np", &systemerror_getnterrmsg_np, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "A description of the associated ?#nterr_np (s.a. ?Ewin32:FormatErrorMessage)\n"
	                 "Returns ?N if no message description is available"),
#endif /* CONFIG_HOST_WINDOWS */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst systemerror_members[] = {
	TYPE_MEMBER_FIELD_DOC("errno", STRUCT_CONST | STRUCT_INT,
	                      offsetof(DeeSystemErrorObject, se_errno),
	                      "The associated system errno value (one of #C{E*} from ?R!Mposix])"),
#ifdef CONFIG_HOST_WINDOWS
	TYPE_MEMBER_FIELD_DOC("nterr_np", STRUCT_CONST | STRUCT_UINT32_T,
	                      offsetof(DeeSystemErrorObject, se_lasterror),
	                      "The windows-specific error code, as returned by ?Ewin32:GetLastError"),
#endif /* CONFIG_HOST_WINDOWS */
	TYPE_MEMBER_END
};

#ifdef CONFIG_HOST_WINDOWS
#define SystemError_init_params "message:?Dstring,inner:?DError,errno?:?X2?Dint?Dstring,nterr_np?:?Dint"
#else /* CONFIG_HOST_WINDOWS */
#define SystemError_init_params "message:?Dstring,inner:?DError,errno?:?X2?Dint?Dstring"
#endif /* !CONFIG_HOST_WINDOWS */

PUBLIC DeeTypeObject DeeError_SystemError = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "SystemError",
	/* .tp_doc      = */ DOC("(" SystemError_init_params ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_Error,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeSystemErrorObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&systemerror_init_kw
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&systemerror_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&systemerror_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ systemerror_getsets,
	/* .tp_members       = */ systemerror_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ systemerror_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#define INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS(tp_name, tp_doc, tp_flags,                \
                                               tp_base, T, tp_str, tp_print,             \
                                               tp_methods, tp_getsets, tp_class_members) \
	{                                                                                    \
		OBJECT_HEAD_INIT(&DeeType_Type),                                                 \
		/* .tp_name     = */ tp_name,                                                    \
		/* .tp_doc      = */ DOC(tp_doc),                                                \
		/* .tp_flags    = */ TP_FNORMAL | (tp_flags),                                    \
		/* .tp_weakrefs = */ 0,                                                          \
		/* .tp_features = */ TF_NONE,                                                    \
		/* .tp_base     = */ tp_base,                                                    \
		/* .tp_init = */ {                                                               \
			{                                                                            \
				/* .tp_alloc = */ {                                                      \
					/* .tp_ctor        = */ (Dee_funptr_t)NULL,                          \
					/* .tp_copy_ctor   = */ (Dee_funptr_t)NULL,                          \
					/* .tp_deep_ctor   = */ (Dee_funptr_t)NULL,                          \
					/* .tp_any_ctor    = */ (Dee_funptr_t)NULL,                          \
					TYPE_FIXED_ALLOCATOR(T),                                             \
					/* .tp_any_ctor_kw = */ (Dee_funptr_t)&systemerror_init_kw           \
				}                                                                        \
			},                                                                           \
			/* .tp_dtor        = */ NULL,                                                \
			/* .tp_assign      = */ NULL,                                                \
			/* .tp_move_assign = */ NULL,                                                \
		},                                                                               \
		/* .tp_cast = */ {                                                               \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ NULL,                                                  \
			/* .tp_bool      = */ NULL,                                                  \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&systemerror_printrepr, \
		},                                                                               \
		/* .tp_visit         = */ NULL,                                                  \
		/* .tp_gc            = */ NULL,                                                  \
		/* .tp_math          = */ NULL,                                                  \
		/* .tp_cmp           = */ &DeeStructObject_Cmp,                                  \
		/* .tp_seq           = */ NULL,                                                  \
		/* .tp_iter_next     = */ NULL,                                                  \
		/* .tp_iterator      = */ NULL,                                                  \
		/* .tp_attr          = */ NULL,                                                  \
		/* .tp_with          = */ NULL,                                                  \
		/* .tp_buffer        = */ NULL,                                                  \
		/* .tp_methods       = */ tp_methods,                                            \
		/* .tp_getsets       = */ tp_getsets,                                            \
		/* .tp_members       = */ NULL,                                                  \
		/* .tp_class_methods = */ NULL,                                                  \
		/* .tp_class_getsets = */ NULL,                                                  \
		/* .tp_class_members = */ tp_class_members,                                      \
	}                                                                                    \

PUBLIC DeeTypeObject DeeError_UnsupportedAPI = /* TODO: Include name of unsupported API */
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("UnsupportedAPI", "(" SystemError_init_params ")",
                                       TP_FNORMAL, &DeeError_SystemError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);

PRIVATE struct type_member tpconst FSError_class_members[] = {
	TYPE_MEMBER_CONST("FileAccessError", &DeeError_FileAccessError),
	TYPE_MEMBER_CONST("FileNotFound", &DeeError_FileNotFound),
	TYPE_MEMBER_CONST("FileExists", &DeeError_FileExists),
	TYPE_MEMBER_CONST("FileClosed", &DeeError_FileClosed),
	TYPE_MEMBER_CONST("CrossDeviceLink", &DeeError_CrossDeviceLink),
	TYPE_MEMBER_CONST("BusyFile", &DeeError_BusyFile),
	TYPE_MEMBER_END
};
#define FSError_init_params SystemError_init_params
PUBLIC DeeTypeObject DeeError_FSError = /* TODO: Include filename of affected file */
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FSError", "(" FSError_init_params ")",
                                       TP_FNORMAL, &DeeError_SystemError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FSError_class_members);


PRIVATE struct type_member tpconst FileAccessError_class_members[] = {
	TYPE_MEMBER_CONST("ReadOnlyFile", &DeeError_ReadOnlyFile),
	TYPE_MEMBER_END
};
#define FileAccessError_init_params FSError_init_params
PUBLIC DeeTypeObject DeeError_FileAccessError =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileAccessError",
                                       "(" FileAccessError_init_params ")\n"
                                       "An error derived from :FSError that is thrown when attempting "
                                       /**/ "to access a file or directory without the necessary permissions",
                                       TP_FNORMAL, &DeeError_SystemError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FileAccessError_class_members);

PUBLIC DeeTypeObject DeeError_ReadOnlyFile =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("ReadOnlyFile",
                                       "(" FileAccessError_init_params ")\n"
                                       "An error derived from :FileAccessError that is thrown when attempting "
                                       /**/ "to modify a file or directory when it or the filesystem is read-only",
                                       TP_FNORMAL, &DeeError_FileAccessError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);

PRIVATE struct type_member tpconst FileNotFound_class_members[] = {
	TYPE_MEMBER_CONST("NoDirectory", &DeeError_NoDirectory),
	TYPE_MEMBER_CONST("NoSymlink", &DeeError_NoSymlink),
	TYPE_MEMBER_END
};
#define FileNotFound_init_params FSError_init_params
PUBLIC DeeTypeObject DeeError_FileNotFound =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileNotFound", "(" FileNotFound_init_params ")",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FileNotFound_class_members);

PRIVATE struct type_member tpconst FileExists_class_members[] = {
	TYPE_MEMBER_CONST("IsDirectory", &DeeError_IsDirectory),
	TYPE_MEMBER_CONST("DirectoryNotEmpty", &DeeError_DirectoryNotEmpty),
	TYPE_MEMBER_END
};
#define FileExists_init_params FSError_init_params
PUBLIC DeeTypeObject DeeError_FileExists =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileExists",
                                       "(" FileExists_init_params ")\n"
                                       "An error derived from :FSError that is thrown when attempting "
                                       /**/ "to create a filesystem object when the target path already exists",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FileExists_class_members);

PUBLIC DeeTypeObject DeeError_FileClosed =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileClosed", "(" FSError_init_params ")",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_NoDirectory =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("NoDirectory",
                                       "(" FileNotFound_init_params ")\n"
                                       "An error derived from :FileNotFound that is thrown when a "
                                       /**/ "directory was expected, but something different was found",
                                       TP_FNORMAL, &DeeError_FileNotFound, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_IsDirectory =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("IsDirectory",
                                       "(" FileExists_init_params ")\n"
                                       "An error derived from :FileExists that is thrown when something "
                                       /**/ "other than a directory was expected, but one was found none-the-less",
                                       TP_FNORMAL, &DeeError_FileExists, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_CrossDeviceLink =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("CrossDeviceLink",
                                       "(" FSError_init_params ")\n"
                                       "An error derived from :FSError that is thrown when attempting "
                                       /**/ "to move a file between different devices or partitions",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_DirectoryNotEmpty =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("DirectoryNotEmpty",
                                       "(" FileExists_init_params ")\n"
                                       "An error derived from :FileExists that is thrown when "
                                       /**/ "attempting to remove a directory that isn't empty",
                                       TP_FNORMAL, &DeeError_FileExists, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_BusyFile =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("BusyFile",
                                       "(" FSError_init_params ")\n"
                                       "An error derived from :FSError that is thrown when "
                                       /**/ "attempting to remove a file or directory that is being "
                                       /**/ "used by another process",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_NoSymlink =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("NoSymlink",
                                       "(" FileNotFound_init_params ")\n"
                                       "An error derived from :FileNotFound that is thrown when attempting "
                                       /**/ "to invoke ?Eposix:readlink on a file that isn't a symbolic link",
                                       TP_FNORMAL, &DeeError_FileNotFound, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
/* END::SystemError */



















/************************************************************************/
/* SPECIAL CASE: AppExit                                                */
/************************************************************************/

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif /* !EXIT_SUCCESS */

PRIVATE int DCALL
appexit_init(struct appexit_object *__restrict self,
             size_t argc, DeeObject *const *argv) {
	int result;
	self->ae_exitcode = EXIT_SUCCESS;
	/* Read the exitcode from arguments. */
	result = DeeArg_UnpackStruct(argc, argv,
	                             "|d:appexit",
	                             &self->ae_exitcode);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
appexit_copy(struct appexit_object *__restrict self,
             struct appexit_object *__restrict other) {
	self->ae_exitcode = other->ae_exitcode;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
appexit_print(struct appexit_object *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<AppExit with exitcode %d>", self->ae_exitcode);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
appexit_printrepr(struct appexit_object *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "AppExit(%d)", self->ae_exitcode);
}

PRIVATE struct type_member tpconst appexit_members[] = {
	TYPE_MEMBER_FIELD("exitcode", STRUCT_CONST | STRUCT_INT,
	                  offsetof(struct appexit_object, ae_exitcode)),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_class_atexit(DeeTypeObject *UNUSED(self),
                     size_t argc, DeeObject *const *argv) {
	DeeObject *callback, *args = Dee_EmptyTuple;
	_DeeArg_Unpack1Or2(err, argc, argv, "atexit", &callback, &args);
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (Dee_AtExit(callback, args))
		goto err;
	return_none;
err:
	return NULL;
}

/* Terminate the application the same way `deemon.Error.AppExit.exit()' would,
 * either through use of `exit()' from <stdlib.h>, or by throwing an exception.
 * NOTE: When available, calling stdlib's `exit()' is identical to this.
 * @return: -1: If this function returns at all, it always returns `-1' */
PUBLIC int DCALL Dee_Exit(int exitcode, bool run_atexit) {
	(void)exitcode;
	(void)run_atexit;
	COMPILER_IMPURE();
#ifdef CONFIG_HAVE__Exit
#ifdef CONFIG_HAVE_exit
	if (run_atexit)
		exit(exitcode);
#endif /* CONFIG_HAVE_exit */
	_Exit(exitcode);
#else /* CONFIG_HAVE__Exit */
	/* If callbacks aren't supposed to be executed, discard
	 * all of them and prevent the addition of new ones. */
	if (!run_atexit)
		Dee_RunAtExit(DEE_RUNATEXIT_FDONTRUN);
#ifdef CONFIG_HAVE_exit
	exit(exitcode);
#else /* CONFIG_HAVE_exit */
	/* No stdlib support. Instead, we must throw an AppExit error. */
	{
		struct appexit_object *error;
		error = DeeObject_MALLOC(struct appexit_object);
		if likely(error) {
			/* Initialize the appexit error. */
			error->ae_exitcode = exitcode;
			DeeObject_Init(error, &DeeError_AppExit);
			/* Throw the appexit error. */
			DeeError_ThrowInherited((DeeObject *)error);
		}
	}
	return -1;
#endif /* !CONFIG_HAVE_exit */
#endif /* !CONFIG_HAVE__Exit */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_class_exit(DeeTypeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	struct {
		int exitcode;
		bool run_atexit;
	} args;
	args.exitcode   = EXIT_SUCCESS;
	args.run_atexit = true;
	if (DeeArg_UnpackStruct(argc, argv, "|db:exit", &args))
		goto err;
	Dee_Exit(args.exitcode, args.run_atexit);
err:
	return NULL;
}

PRIVATE struct type_method tpconst appexit_class_methods[] = {
	TYPE_METHOD("exit", &appexit_class_exit,
	            "()\n"
	            "(exitcode:?Dint,run_atexit=!t)\n"
	            "Terminate execution of deemon after invoking ?#atexit callbacks when @run_atexit is ?t\n"
	            "Termination is done using the C #Cexit or #C_exit functions, if available. However if these "
	            /**/ "functions are not provided by the host, an :AppExit error is thrown instead\n"
	            "When no @exitcode is given, the host's default default value of #CEXIT_SUCCESS, or $1 is used\n"
	            "This function never returns normally"),
	TYPE_METHOD("atexit", &appexit_class_atexit,
	            "(callback:?DCallable,args=!T0)\n"
	            "#tRuntimeError{Additional atexit-callbacks can no longer be registered}"
	            "#tNotImplemented{Deemon was built without support for ?#atexit}"
	            "Register a given @callback to be executed before deemon terminates"),
	TYPE_METHOD_END
};

PUBLIC DeeTypeObject DeeError_AppExit = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "AppExit",
	/* .tp_doc      = */ DOC("An AppExit object is a special kind of interrupt that "
	                         /**/ "is not derived from ?O, and has no base class at "
	                         /**/ "all. It's purpose is to allow user-code to throw an "
	                         /**/ "instance of it and have the stack unwind itself, alongside "
	                         /**/ "all existing objects being destroyed normally before deemon "
	                         /**/ "will be terminated with the given exitcode\n"
	                         "\n"

	                         "()\n"
	                         "(exitcode:?Dint)\n"
	                         "Construct a new AppExit object using the given @exitcode "
	                         /**/ "or the host's default value for #CEXIT_SUCCESS, or $1"),
	/* .tp_flags    = */ TP_FFINAL | TP_FINTERRUPT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL, /* No base type (to only allow AppExit-interrupt handlers,
	                            * and all-interrupt handlers to catch this error) */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&appexit_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&appexit_copy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&appexit_init,
				TYPE_FIXED_ALLOCATOR(struct appexit_object)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str      = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr     = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool     = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_strrepr  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&appexit_print,
		/* .tp_reprrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&appexit_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ appexit_members,
	/* .tp_class_methods = */ appexit_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};






/* Fallback instance when throwing no-memory errors with an unknown size. */
PUBLIC DeeNoMemoryErrorObject DeeError_NoMemory_instance = {
	OBJECT_HEAD_INIT(&DeeError_NoMemory),
	/* .e_message    = */ (DREF DeeStringObject *)&str_nomemory,
	/* .e_inner      = */ NULL,
	/* .nm_allocsize = */ (size_t)-1
};








/* ==== Signal type subsystem ==== */
#define INIT_CUSTOM_SIGNAL(tp_name, tp_doc, tp_flags, tp_base,           \
                           tp_ctor, tp_copy, tp_deep, tp_init,           \
                           tp_init_kw, tp_visit, T,                      \
                           tp_str, tp_print, tp_repr, tp_printrepr,      \
                           tp_methods, tp_getsets, tp_members,           \
                           tp_class_members)                             \
	{                                                                    \
		OBJECT_HEAD_INIT(&DeeType_Type),                                 \
		/* .tp_name     = */ tp_name,                                    \
		/* .tp_doc      = */ DOC(tp_doc),                                \
		/* .tp_flags    = */ tp_flags,                                   \
		/* .tp_weakrefs = */ 0,                                          \
		/* .tp_features = */ TF_NONE | TF_TPVISIT,                       \
		/* .tp_base     = */ tp_base,                                    \
		/* .tp_init = */ {                                               \
			{                                                            \
				/* .tp_alloc = */ {                                      \
					/* .tp_ctor      = */ (Dee_funptr_t)(tp_ctor),       \
					/* .tp_copy_ctor = */ (Dee_funptr_t)(tp_copy),       \
					/* .tp_deep_ctor = */ (Dee_funptr_t)(tp_deep),       \
					/* .tp_any_ctor  = */ (Dee_funptr_t)(tp_init),       \
					TYPE_FIXED_ALLOCATOR(T),                             \
					/* .tp_any_ctor_kw = */ (Dee_funptr_t)(tp_init_kw)   \
				}                                                        \
			},                                                           \
			/* .tp_dtor        = */ NULL,                                \
			/* .tp_assign      = */ NULL,                                \
			/* .tp_move_assign = */ NULL                                 \
		},                                                               \
		/* .tp_cast = */ {                                               \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_repr), \
			/* .tp_bool      = */ NULL,                                  \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_printrepr),                                  \
		},                                                               \
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))(tp_visit), \
		/* .tp_gc            = */ NULL,                                  \
		/* .tp_math          = */ NULL,                                  \
		/* .tp_cmp           = */ &DeeStructObject_Cmp,                  \
		/* .tp_seq           = */ NULL,                                  \
		/* .tp_iter_next     = */ NULL,                                  \
		/* .tp_iterator      = */ NULL,                                  \
		/* .tp_attr          = */ NULL,                                  \
		/* .tp_with          = */ NULL,                                  \
		/* .tp_buffer        = */ NULL,                                  \
		/* .tp_methods       = */ tp_methods,                            \
		/* .tp_getsets       = */ tp_getsets,                            \
		/* .tp_members       = */ tp_members,                            \
		/* .tp_class_methods = */ NULL,                                  \
		/* .tp_class_getsets = */ NULL,                                  \
		/* .tp_class_members = */ tp_class_members                       \
	}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
signal_printrepr(DeeSignalObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%k()", Dee_TYPE(self));
}


/************************************************************************/
/* Signal.StopIteration                                                 */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_StopIteration =
INIT_CUSTOM_SIGNAL("StopIteration", NULL, TP_FNORMAL, &DeeError_Signal,
                   &DeeNone_OperatorCtor, &DeeNone_OperatorCopy, &DeeNone_OperatorCopy, NULL,
                   NULL, NULL, DeeSignalObject,
                   NULL, NULL, NULL, &signal_printrepr,
                   NULL, NULL, NULL, NULL);

/************************************************************************/
/* Signal.Interrupt                                                     */
/************************************************************************/
PRIVATE struct type_member tpconst interrupt_class_members[] = {
	TYPE_MEMBER_CONST("KeyboardInterrupt", &DeeError_KeyboardInterrupt),
	TYPE_MEMBER_CONST("ThreadExit", &DeeError_ThreadExit),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_Interrupt =
INIT_CUSTOM_SIGNAL("Interrupt", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Signal,
                   &DeeNone_OperatorCtor, &DeeNone_OperatorCopy, &DeeNone_OperatorCopy, NULL,
                   NULL, NULL, DeeSignalObject,
                   NULL, NULL, NULL, &signal_printrepr,
                   NULL, NULL, NULL, interrupt_class_members);


/************************************************************************/
/* Signal.Interrupt.ThreadExit                                          */
/************************************************************************/
PRIVATE struct type_member tpconst threadexit_members[] = {
	TYPE_MEMBER_FIELD("__result__", STRUCT_OBJECT, offsetof(struct threadexit_object, te_result)),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_ThreadExit =
INIT_CUSTOM_SIGNAL("ThreadExit", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Interrupt,
                   NULL, &DeeStructObject_Copy, &DeeStructObject_Deep, &DeeStructObject_Init,
                   &DeeStructObject_InitKw, &DeeStructObject_Visit, struct threadexit_object,
                   NULL, NULL, NULL, &DeeStructObject_PrintRepr,
                   NULL, NULL, threadexit_members, interrupt_class_members);


/************************************************************************/
/* Signal.Interrupt.KeyboardInterrupt                                   */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_KeyboardInterrupt =
INIT_CUSTOM_SIGNAL("KeyboardInterrupt", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Interrupt,
                   &DeeNone_OperatorCtor, &DeeNone_OperatorCopy, &DeeNone_OperatorCopy, NULL,
                   NULL, NULL, DeeSignalObject,
                   NULL, NULL, NULL, &signal_printrepr,
                   NULL, NULL, NULL, NULL);


/************************************************************************/
/* Signal                                                               */
/************************************************************************/
PRIVATE struct type_member tpconst signal_class_members[] = {
	TYPE_MEMBER_CONST("Interrupt", &DeeError_Interrupt),
	TYPE_MEMBER_CONST("StopIteration", &DeeError_StopIteration),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_Signal = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Signal),
	/* .tp_doc      = */ DOC("Base class for signaling exceptions\n"
	                         "\n"
	                         "()"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&DeeNone_OperatorCtor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeSignalObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&signal_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ signal_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

PUBLIC DeeSignalObject DeeError_StopIteration_instance = {
	OBJECT_HEAD_INIT(&DeeError_StopIteration)
};

PUBLIC DeeSignalObject DeeError_Interrupt_instance = {
	OBJECT_HEAD_INIT(&DeeError_Interrupt)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */
