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
#ifndef GUARD_DEEMON_RUNTIME_ERROR_RT_C
#define GUARD_DEEMON_RUNTIME_ERROR_RT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/method-hints.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/struct.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/variant.h>

#include <hybrid/int128.h>
#include <hybrid/limitcore.h>
#include <hybrid/typecore.h>
/**/

#include "strings.h"
/**/

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#ifndef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */
#ifndef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

#ifndef __SIZEOF_BOOL__
#define __SIZEOF_BOOL__ __SIZEOF_CHAR__
#endif /* !__SIZEOF_BOOL__ */

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
#define INIT_LIKE_BASECLASS(tp_name, tp_doc, tp_flags,                  \
                            tp_base, T, tp_str, tp_print,               \
                            tp_methods, tp_getsets, tp_class_members)   \
	INIT_CUSTOM_ERROR_EX(tp_name, tp_doc, (tp_flags) | TP_FINHERITCTOR, \
	                     0, tp_base, T,                                 \
	                     NULL, NULL, NULL, NULL,                        \
	                     NULL, NULL, NULL, NULL,                        \
	                     tp_str, tp_print,                              \
	                     tp_methods, tp_getsets, NULL,                  \
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

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
error_init(DeeObject *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeErrorObject *me = (DeeErrorObject *)self;
	me->e_message = NULL;
	me->e_inner   = NULL;
	_DeeArg_Unpack0Or1Or2(err, argc, argv, DeeType_GetName(Dee_TYPE(me)),
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
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
		return DeeArg_BadArgcEx(DeeType_GetName(Dee_TYPE(me)), argc, 0, 2);
	}
	Dee_XIncref(me->e_message);
	Dee_XIncref(me->e_inner);
	if unlikely(DeeKwArgs_Done(&kwds, argc, DeeType_GetName(Dee_TYPE(me))))
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
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
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
	if (self->e_message)
		return error_str((DeeErrorObject *)self);
	return DeeString_Newf("Failed to allocated %" PRFuSIZ " bytes",
	                      self->nm_allocsize);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
nomemory_print(DeeNoMemoryErrorObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	if (self->e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "Failed to allocated %" PRFuSIZ " bytes",
	                        self->nm_allocsize);
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
INIT_LIKE_ERROR("TypeError", "(" Error_init_params ")",
                TP_FNORMAL, &DeeError_Error, NULL, NULL,
                NULL, NULL, NULL);
/* END::TypeError */









/************************************************************************/
/* Error.ValueError                                                     */
/************************************************************************/
typedef struct {
	ERROR_OBJECT_HEAD
	struct Dee_variant ve_value; /* [const] Value that caused the error (exact meaning is specific to sub-class) */
} ValueError;

PRIVATE struct type_member tpconst ValueError_class_members[] = {
	TYPE_MEMBER_CONST("ArithmeticError", &DeeError_ArithmeticError),
	TYPE_MEMBER_CONST("SequenceError", &DeeError_SequenceError),
	TYPE_MEMBER_CONST("UnicodeError", &DeeError_UnicodeError),
	TYPE_MEMBER_CONST("ReferenceError", &DeeError_ReferenceError),
	TYPE_MEMBER_CONST("BufferError", &DeeError_BufferError),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ValueError_members[] = {
#define ValueError_init_params Error_init_params ",value?"
	TYPE_MEMBER_FIELD("value", STRUCT_VARIANT | STRUCT_CONST, offsetof(ValueError, ve_value)),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_ValueError =
INIT_CUSTOM_ERROR("ValueError", "(" ValueError_init_params ")",
                  TP_FNORMAL, &DeeError_Error, ValueError, NULL, NULL,
                  NULL, NULL, ValueError_members,
                  ValueError_class_members);



/************************************************************************/
/* Error.ValueError.ArithmeticError                                     */
/************************************************************************/
PRIVATE struct type_member tpconst arithmetic_class_members[] = {
	TYPE_MEMBER_CONST("IntegerOverflow", &DeeError_IntegerOverflow),
	TYPE_MEMBER_CONST("DivideByZero", &DeeError_DivideByZero),
	TYPE_MEMBER_CONST("NegativeShift", &DeeError_NegativeShift),
	TYPE_MEMBER_END
};

typedef ValueError ArithmeticError;
#define ArithmeticError_init_params Error_init_params ",value?:?DNumeric"
PUBLIC DeeTypeObject DeeError_ArithmeticError =
INIT_LIKE_BASECLASS("ArithmeticError", "(" ArithmeticError_init_params ")",
                    TP_FNORMAL, &DeeError_ValueError, ArithmeticError,
                    NULL, NULL, NULL, NULL, arithmetic_class_members);


/************************************************************************/
/* Error.ValueError.ArithmeticError.DivideByZero                        */
/************************************************************************/
typedef struct {
	ArithmeticError    dbz_base; /* Base error (value is the left-hand-side of division) */
	struct Dee_variant dbz_rhs;  /* right-hand-side of division */
} DivideByZero;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DivideByZero_print(DivideByZero *__restrict self,
                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	struct Dee_variant lhs, rhs;
	if (self->dbz_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	Dee_variant_init_copy(&lhs, &self->dbz_base.ve_value);
	Dee_variant_init_copy(&rhs, &self->dbz_rhs);
	if (!Dee_variant_isbound_nonatomic(&rhs)) {
#if 1
		Dee_variant_init_uint32(&rhs, 0);
#else
		Dee_variant_setuint32(&rhs, 0);
#endif
	}
	result = DeeFormat_Printf(printer, arg,
	                          "Divide by Zero: `%Vk / %Vk'",
	                          &lhs, &rhs);
	Dee_variant_fini(&rhs);
	Dee_variant_fini(&lhs);
	return result;
}

PRIVATE struct type_member tpconst DivideByZero_members[] = {
#define DivideByZero_init_params Error_init_params ",lhs?:?DNumeric,rhs?:?DNumeric"
	TYPE_MEMBER_FIELD_DOC("lhs", STRUCT_VARIANT | STRUCT_CONST, offsetof(DivideByZero, dbz_base.ve_value), "->?DNumeric"),
	TYPE_MEMBER_FIELD_DOC("rhs", STRUCT_VARIANT | STRUCT_CONST, offsetof(DivideByZero, dbz_rhs), "->?DNumeric"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_DivideByZero =
INIT_CUSTOM_ERROR("DivideByZero", "(" DivideByZero_init_params ")",
                  TP_FNORMAL, &DeeError_ArithmeticError, DivideByZero,
                  NULL, &DivideByZero_print, NULL, NULL, DivideByZero_members, NULL);

/* Throws an `DeeError_DivideByZero' indicating that a zero-division attempt has taken place. */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrDivideByZero)(DeeObject *lhs) {
	DREF DivideByZero *result = DeeObject_MALLOC(DivideByZero);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->dbz_base, &DeeError_DivideByZero);
	result->dbz_base.e_message = NULL;
	result->dbz_base.e_inner   = NULL;
	Dee_variant_init_object(&result->dbz_base.ve_value, lhs);
	Dee_variant_init_unbound(&result->dbz_rhs);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrDivideByZeroEx)(DeeObject *lhs, DeeObject *rhs) {
	DREF DivideByZero *result = DeeObject_MALLOC(DivideByZero);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->dbz_base, &DeeError_DivideByZero);
	result->dbz_base.e_message = NULL;
	result->dbz_base.e_inner   = NULL;
	Dee_variant_init_object(&result->dbz_base.ve_value, lhs);
	Dee_variant_init_object(&result->dbz_rhs, rhs);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrDivideByZeroVar)(struct Dee_variant *lhs, struct Dee_variant *rhs) {
	DREF DivideByZero *result = DeeObject_MALLOC(DivideByZero);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->dbz_base, &DeeError_DivideByZero);
	result->dbz_base.e_message = NULL;
	result->dbz_base.e_inner   = NULL;
	Dee_variant_init_copy(&result->dbz_base.ve_value, lhs);
	Dee_variant_init_copy(&result->dbz_rhs, rhs);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}




/************************************************************************/
/* Error.ValueError.ArithmeticError.NegativeShift                       */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_NegativeShift =
INIT_LIKE_BASECLASS("NegativeShift", "(" ArithmeticError_init_params ")",
                    TP_FNORMAL, &DeeError_ArithmeticError, ArithmeticError,
                    NULL, NULL, NULL, NULL, NULL);


/************************************************************************/
/* Error.ValueError.SequenceError                                       */
/************************************************************************/
typedef ValueError SequenceError;
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
SequenceError_GetSeqType(SequenceError *__restrict self) {
	DREF DeeTypeObject *result = Dee_variant_getobjecttype(&self->ve_value);
	if (result == NULL) {
		result = &DeeSeq_Type;
		Dee_Incref(result);
	}
	return result;
}

PRIVATE struct type_member tpconst SequenceError_members[] = {
#define SequenceError_init_params Error_init_params ",seq?:?DSequence"
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_VARIANT | STRUCT_CONST, offsetof(SequenceError, ve_value), "->?DSequence"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst SequenceError_class_members[] = {
	TYPE_MEMBER_CONST("KeyError", &DeeError_KeyError),
	TYPE_MEMBER_CONST("UnpackError", &DeeError_UnpackError),
	TYPE_MEMBER_CONST("ItemNotFound", &DeeError_ItemNotFound),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_SequenceError =
INIT_CUSTOM_ERROR("SequenceError", "(" SequenceError_init_params ")",
                  TP_FNORMAL, &DeeError_ValueError, SequenceError, NULL, NULL,
                  NULL, NULL, SequenceError_members, SequenceError_class_members);


/************************************************************************/
/* Error.ValueError.SequenceError.KeyError                              */
/************************************************************************/
typedef struct {
	SequenceError      ke_base;
	struct Dee_variant ke_key; /* [const] Key in question */
} KeyError;

PRIVATE struct type_member tpconst KeyError_members[] = {
#define KeyError_init_params SequenceError_init_params ",key?"
	TYPE_MEMBER_FIELD("key", STRUCT_VARIANT | STRUCT_CONST, offsetof(KeyError, ke_key)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst KeyError_class_members[] = {
	TYPE_MEMBER_CONST("IndexError", &DeeError_IndexError),
	TYPE_MEMBER_CONST("UnboundItem", &DeeError_UnboundItem),
	TYPE_MEMBER_CONST("UnknownKey", &DeeError_UnknownKey),
	TYPE_MEMBER_CONST("ReadOnlyKey", &DeeError_ReadOnlyKey),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_KeyError =
INIT_CUSTOM_ERROR("KeyError", "(" KeyError_init_params ")",
                  TP_FNORMAL, &DeeError_SequenceError, KeyError, NULL, NULL,
                  NULL, NULL, KeyError_members, KeyError_class_members);


/************************************************************************/
/* Error.ValueError.SequenceError.KeyError.IndexError                   */
/************************************************************************/
typedef struct {
	KeyError           ie_base;   /* "key" is renamed to "index" */
	struct Dee_variant ie_length; /* [const] Length of the sequence (or "unbound" if lazily calculated) */
} IndexError;

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
IndexError_GetLength(IndexError *__restrict self,
                     struct Dee_variant *__restrict p_length) {
	Dee_variant_init_copy(p_length, &self->ie_length);
	if (!Dee_variant_isbound_nonatomic(p_length)) {
		DREF DeeObject *seq = Dee_variant_getobject(&self->ie_base.ke_base.ve_value);
		if (seq) {
			size_t seq_length;
			seq_length = DeeObject_InvokeMethodHint(seq_operator_size, seq);
			Dee_Decref_unlikely(seq);
			if unlikely(seq_length == (size_t)-1)
				goto err;
			Dee_variant_setsize_if_unbound(&self->ie_length, seq_length);
			Dee_variant_init_size(p_length, seq_length);
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE struct type_member tpconst IndexError_members[] = {
#define IndexError_init_params SequenceError_init_params ",index?:?X2?DNumeric?Dint,length?:?X2?DNumeric?Dint"
	TYPE_MEMBER_FIELD_DOC("index", STRUCT_VARIANT | STRUCT_CONST, offsetof(IndexError, ie_base.ke_key), "->?X2?DNumeric?Dint"),
	/* TODO: Have another "getset" that shadows "length" and calls "IndexError_GetLength()" */
	TYPE_MEMBER_FIELD_DOC("length", STRUCT_VARIANT | STRUCT_CONST, offsetof(IndexError, ie_length), "->?X2?DNumeric?Dint"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
IndexError_print(IndexError *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	struct Dee_variant length;
	if (self->ie_base.ke_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	if unlikely(IndexError_GetLength(self, &length))
		goto err;
	result = DeeFormat_Printf(printer, arg,
	                          "Index %Vr lies outside the valid bounds [0,%Vr) of %K",
	                          &self->ie_base.ke_key, &length,
	                          SequenceError_GetSeqType(&self->ie_base.ke_base));
	Dee_variant_fini(&length);
	return result;
err:
	return -1;
}

PRIVATE struct type_member tpconst IndexError_class_members[] = {
	TYPE_MEMBER_CONST("EmptySequence", &DeeError_EmptySequence),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_IndexError =
INIT_CUSTOM_ERROR("IndexError", "(" IndexError_init_params ")",
                  TP_FNORMAL, &DeeError_KeyError, IndexError, NULL, &IndexError_print,
                  NULL, NULL, IndexError_members, IndexError_class_members);

/* Throws an `DeeError_IndexError' indicating that a given index is out-of-bounds */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIndexOutOfBounds)(DeeObject *seq, size_t index, size_t length) {
	DREF IndexError *result = DeeObject_MALLOC(IndexError);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ie_base.ke_base, &DeeError_IndexError);
	result->ie_base.ke_base.e_message = NULL;
	result->ie_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ie_base.ke_base.ve_value, seq);
	Dee_variant_init_size(&result->ie_base.ke_key, index);
	Dee_variant_init_size(&result->ie_length, length);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrIndexOutOfBoundsObj)(DeeObject *seq, DeeObject *index, DeeObject *length) {
	DREF IndexError *result = DeeObject_MALLOC(IndexError);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ie_base.ke_base, &DeeError_IndexError);
	result->ie_base.ke_base.e_message = NULL;
	result->ie_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ie_base.ke_base.ve_value, seq);
	Dee_variant_init_object(&result->ie_base.ke_key, index);
	Dee_variant_init_object(&result->ie_length, length);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

#ifdef CONFIG_BUILDING_DEEMON
INTERN ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrVaIndexOutOfBounds)(struct Dee_code_frame const *__restrict frame,
                                    size_t index) {
	int result;
	DREF DeeTupleObject *varargs = frame->cf_vargs;
	if (varargs) {
		Dee_Incref(varargs);
	} else {
		DeeCodeObject *code = frame->cf_func->fo_code;
		if (frame->cf_argc <= code->co_argc_max) {
			varargs = (DREF DeeTupleObject *)DeeTuple_NewEmpty();
		} else {
			varargs = (DREF DeeTupleObject *)DeeTuple_NewVector((size_t)(frame->cf_argc - code->co_argc_max),
			                                                    frame->cf_argv + code->co_argc_max);
			if unlikely(!varargs)
				return -1;
		}
	}
	result = DeeRT_ErrIndexOutOfBounds((DeeObject *)varargs, index,
	                                   DeeTuple_SIZE(varargs));
	Dee_Decref_unlikely(varargs);
	return result;
}
#endif /* CONFIG_BUILDING_DEEMON */


/************************************************************************/
/* Error.ValueError.SequenceError.KeyError.IndexError.EmptySequence     */
/************************************************************************/
typedef IndexError EmptySequence;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
EmptySequence_print(IndexError *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	if (self->ie_base.ke_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "Empty sequence of type `%K' encountered",
	                        SequenceError_GetSeqType(&self->ie_base.ke_base));
}

PUBLIC DeeTypeObject DeeError_EmptySequence =
INIT_LIKE_BASECLASS("EmptySequence", "(" IndexError_init_params ")",
                    TP_FNORMAL, &DeeError_IndexError, EmptySequence, NULL, &EmptySequence_print,
                    NULL, NULL, NULL);

/* Throws an `DeeError_EmptySequence' indicating that a given sequence is empty */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrEmptySequence)(DeeObject *seq) {
	DREF EmptySequence *result = DeeObject_MALLOC(EmptySequence);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ie_base.ke_base, &DeeError_EmptySequence);
	result->ie_base.ke_base.e_message = NULL;
	result->ie_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ie_base.ke_base.ve_value, seq);
	Dee_variant_init_unbound(&result->ie_base.ke_key);
	Dee_variant_init_unbound(&result->ie_length);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}



/************************************************************************/
/* Error.ValueError.SequenceError.KeyError.UnknownKey                   */
/************************************************************************/
typedef KeyError UnknownKey;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
UnknownKey_print(UnknownKey *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	if (self->ke_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "Could not find key %Vr in %K: %Vk",
	                        &self->ke_key, SequenceError_GetSeqType(&self->ke_base),
	                        &self->ke_base.ve_value);
}

PUBLIC DeeTypeObject DeeError_UnknownKey =
INIT_CUSTOM_ERROR("UnknownKey", "(" KeyError_init_params ")",
                  TP_FNORMAL, &DeeError_KeyError, UnknownKey, NULL, &UnknownKey_print,
                  NULL, NULL, NULL, NULL);

/* Throws an `DeeError_UnknownKey' indicating that a given index/key is unknown */
PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnknownKey)(DeeObject *map, DeeObject *key) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	Dee_variant_init_object(&result->ke_key, key);
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrUnknownKeyWithInner)(DeeObject *map, DeeObject *key,
                                     /*inherit(always)*/ DREF DeeObject *inner) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = inner; /* Inherit reference */
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	Dee_variant_init_object(&result->ke_key, key);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	Dee_Decref(inner); /* Always inherited */
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrUnknownKeyInt)(DeeObject *map, size_t key) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	Dee_variant_init_size(&result->ke_key, key);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnknownKeyStr)(DeeObject *map, char const *key) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstr_maybe(&result->ke_key, key))
		goto err_r;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrUnknownKeyStrWithInner)(DeeObject *map, char const *key,
                                        /*inherit(always)*/ DREF DeeObject *inner) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstr_maybe(&result->ke_key, key))
		goto err_r;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = inner; /* Inherit reference */
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	Dee_Decref(inner); /* Always inherited */
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnknownKeyStrLen)(DeeObject *map, char const *key, size_t keylen) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstrlen_maybe(&result->ke_key, key, keylen))
		goto err_r;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 4)) int
(DCALL DeeRT_ErrUnknownKeyStrLenWithInner)(DeeObject *map, char const *key, size_t keylen,
                                           /*inherit(always)*/ DREF DeeObject *inner) {
	DREF UnknownKey *result = DeeObject_MALLOC(UnknownKey);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstrlen_maybe(&result->ke_key, key, keylen))
		goto err_r;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = inner; /* Inherit reference */
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	DeeObject_Init(&result->ke_base, &DeeError_UnknownKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	Dee_Decref(inner); /* Always inherited */
	return -1;
}


/************************************************************************/
/* Error.ValueError.SequenceError.KeyError.ReadOnlyKey                   */
/************************************************************************/
typedef KeyError ReadOnlyKey;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ReadOnlyKey_print(ReadOnlyKey *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	if (self->ke_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "Key %Vr of instance of %K: %Vk is read-only and cannot be modified",
	                        &self->ke_key, SequenceError_GetSeqType(&self->ke_base),
	                        &self->ke_base.ve_value);
}

PUBLIC DeeTypeObject DeeError_ReadOnlyKey =
INIT_CUSTOM_ERROR("ReadOnlyKey", "(" KeyError_init_params ")",
                  TP_FNORMAL, &DeeError_KeyError, ReadOnlyKey, NULL, &ReadOnlyKey_print,
                  NULL, NULL, NULL, NULL);

/* Throws an `DeeError_ReadOnlyKey' indicating that a given index/key is unknown */
PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrReadOnlyKey)(DeeObject *map, DeeObject *key) {
	DREF ReadOnlyKey *result = DeeObject_MALLOC(ReadOnlyKey);
	if unlikely(!result)
		goto err;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	Dee_variant_init_object(&result->ke_key, key);
	DeeObject_Init(&result->ke_base, &DeeError_ReadOnlyKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrReadOnlyKeyInt)(DeeObject *map, size_t key) {
	DREF ReadOnlyKey *result = DeeObject_MALLOC(ReadOnlyKey);
	if unlikely(!result)
		goto err;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	Dee_variant_init_size(&result->ke_key, key);
	DeeObject_Init(&result->ke_base, &DeeError_ReadOnlyKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrReadOnlyKeyStr)(DeeObject *map, char const *key) {
	DREF ReadOnlyKey *result = DeeObject_MALLOC(ReadOnlyKey);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstr_maybe(&result->ke_key, key))
		goto err_r;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	DeeObject_Init(&result->ke_base, &DeeError_ReadOnlyKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrReadOnlyKeyStrLen)(DeeObject *map, char const *key, size_t keylen) {
	DREF ReadOnlyKey *result = DeeObject_MALLOC(ReadOnlyKey);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstrlen_maybe(&result->ke_key, key, keylen))
		goto err_r;
	result->ke_base.e_message = NULL;
	result->ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ke_base.ve_value, map);
	DeeObject_Init(&result->ke_base, &DeeError_ReadOnlyKey);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}



/************************************************************************/
/* Error.ValueError.SequenceError.KeyError.UnboundItem                  */
/************************************************************************/
typedef struct {
	KeyError ui_base;
	bool     ui_iskey; /* [const] True if this is about an unbound key (like in mappings)
	                    *         False if this is about an unbound index (like in sequences) */
} UnboundItem;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
UnboundItem_print(UnboundItem *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	if (self->ui_base.ke_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "%s `%Vr' of instance of `%K': %Vk has not been bound",
	                        self->ui_iskey ? "Key" : "Index",
	                        &self->ui_base.ke_key, SequenceError_GetSeqType(&self->ui_base.ke_base),
	                        &self->ui_base.ke_base.ve_value);
}

PRIVATE struct type_member tpconst UnboundItem_members[] = {
#define UnboundItem_init_params SequenceError_init_params ",index?:?X2?DNumeric?Dint,iskey=!f"
	TYPE_MEMBER_FIELD_DOC("index", STRUCT_VARIANT | STRUCT_CONST, offsetof(UnboundItem, ui_base.ke_key),
	                      "->?X2?DNumeric?Dint"),
	TYPE_MEMBER_FIELD_DOC("iskey", STRUCT_BOOL(__SIZEOF_BOOL__) | STRUCT_CONST, offsetof(UnboundItem, ui_iskey),
	                      "!t if this is about an unbound key (like in mappings). "
	                      "!f if this is about an unbound index (like in sequences)"),
	TYPE_MEMBER_END
};

/* TODO: Custom "constructor" that also accepts "key" as an argument (and if given: defaults "iskey=true") */
/* TODO: Custom "operator repr" that prints "key" or "index" based on "iskey" */
PUBLIC DeeTypeObject DeeError_UnboundItem =
INIT_CUSTOM_ERROR("UnboundItem", "(" UnboundItem_init_params ")",
                  TP_FNORMAL, &DeeError_KeyError, UnboundItem, NULL, &UnboundItem_print,
                  NULL, NULL, UnboundItem_members, NULL);

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
DeeRT_ErrUnboundItemImpl(DeeObject *seq, DeeObject *key_or_index, bool is_key) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	Dee_variant_init_object(&result->ui_base.ke_key, key_or_index);
	result->ui_iskey = is_key;
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PRIVATE ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrUnboundItemInt)(DeeObject *seq, size_t key_or_index, bool is_key) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	Dee_variant_init_size(&result->ui_base.ke_key, key_or_index);
	result->ui_iskey = is_key;
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnboundKey)(DeeObject *seq, DeeObject *key) {
	return DeeRT_ErrUnboundItemImpl(seq, key, true);
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrUnboundKeyWithInner)(DeeObject *seq, DeeObject *key,
                                     /*inherit(always)*/ DREF DeeObject *inner) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = inner; /* Inherit reference */
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	Dee_variant_init_object(&result->ui_base.ke_key, key);
	result->ui_iskey = true;
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	Dee_Decref(inner); /* Always inherited */
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnboundKeyStr)(DeeObject *seq, char const *key) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstr_maybe(&result->ui_base.ke_key, key))
		goto err_r;
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	result->ui_iskey = true;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrUnboundKeyStrWithInner)(DeeObject *seq, char const *key,
                                        /*inherit(always)*/ DREF DeeObject *inner) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstr_maybe(&result->ui_base.ke_key, key))
		goto err_r;
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = inner; /* Inherit reference */
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	result->ui_iskey = true;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	Dee_Decref(inner); /* Always inherited */
	return -1;
}


PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnboundKeyStrLen)(DeeObject *seq, char const *key, size_t keylen) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstrlen_maybe(&result->ui_base.ke_key, key, keylen))
		goto err_r;
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = NULL;
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	result->ui_iskey = true;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 4)) int
(DCALL DeeRT_ErrUnboundKeyStrLenWithInner)(DeeObject *seq, char const *key, size_t keylen,
                                           /*inherit(always)*/ DREF DeeObject *inner) {
	DREF UnboundItem *result = DeeObject_MALLOC(UnboundItem);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_variant_init_cstrlen_maybe(&result->ui_base.ke_key, key, keylen))
		goto err_r;
	result->ui_base.ke_base.e_message = NULL;
	result->ui_base.ke_base.e_inner   = inner; /* Inherit reference */
	Dee_variant_init_object(&result->ui_base.ke_base.ve_value, seq);
	result->ui_iskey = true;
	DeeObject_Init(&result->ui_base.ke_base, &DeeError_UnboundItem);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	Dee_Decref(inner); /* Always inherited */
	return -1;
}

PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrUnboundKeyInt)(DeeObject *seq, size_t key) {
	return DeeRT_ErrUnboundItemInt(seq, key, true);
}

PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrUnboundIndex)(DeeObject *seq, size_t index) {
	return DeeRT_ErrUnboundItemInt(seq, index, false);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrUnboundIndexObj)(DeeObject *seq, DeeObject *index) {
	return DeeRT_ErrUnboundItemImpl(seq, index, false);
}


/************************************************************************/
/* Error.ValueError.SequenceError.ItemNotFound                           */
/************************************************************************/
typedef struct {
	SequenceError      inf_base;
	DREF DeeObject    *inf_item;  /* [0..1][const] Missing item */
	size_t             inf_start; /* [const] Sequence start index */
	struct Dee_variant inf_end;   /* [const] Sequence end index (or unbound if unlimited) */
	DREF DeeObject    *inf_key;   /* [0..1][const] Key function applied to items */
} ItemNotFound;

PRIVATE struct type_member tpconst ItemNotFound_members[] = {
#define ItemNotFound_init_params SequenceError_init_params ",item?,start=!0,end?:?Dint,key?:?DCallable"
	TYPE_MEMBER_FIELD("item", STRUCT_OBJECT, offsetof(ItemNotFound, inf_item)),
	TYPE_MEMBER_FIELD("start", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ItemNotFound, inf_start)),
	TYPE_MEMBER_FIELD_DOC("end", STRUCT_VARIANT | STRUCT_CONST, offsetof(ItemNotFound, inf_end), "->?Dint"),
	TYPE_MEMBER_FIELD_DOC("key", STRUCT_OBJECT, offsetof(ItemNotFound, inf_key), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ItemNotFound_class_members[] = {
	TYPE_MEMBER_CONST("RegexNotFound", &DeeError_RegexNotFound),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ItemNotFound_print(ItemNotFound *__restrict self,
                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	struct Dee_variant active_end;
	if (self->inf_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	Dee_variant_init_copy(&active_end, &self->inf_end);
	if ((Dee_variant_isbound_nonatomic(&active_end) || self->inf_start) && self->inf_key) {
		result = DeeFormat_Printf(printer, arg,
		                          "Could not locate item `%k(%Vk)' in sequence `%Vk' [%" PRFuSIZ ",%Vk)",
		                          self->inf_key, &self->inf_item, &self->inf_base.ve_value,
		                          self->inf_start, &active_end);
	} else if (Dee_variant_isbound_nonatomic(&active_end) || self->inf_start) {
		result = DeeFormat_Printf(printer, arg,
		                          "Could not locate item `%Vk' in sequence `%Vk' [%" PRFuSIZ ",%Vk)",
		                          &self->inf_item, &self->inf_base.ve_value,
		                          self->inf_start, &active_end);
	} else if (self->inf_key) {
		result = DeeFormat_Printf(printer, arg,
		                          "Could not locate item `%k(%Vk)' in sequence `%Vk'",
		                          self->inf_key, &self->inf_item, &self->inf_base.ve_value);
	} else {
		result = DeeFormat_Printf(printer, arg,
		                          "Could not locate item `%Vk' in sequence `%Vk'",
		                          &self->inf_item, &self->inf_base.ve_value);
	}
	Dee_variant_fini(&active_end);
	return result;
}

PUBLIC DeeTypeObject DeeError_ItemNotFound =
INIT_CUSTOM_ERROR("ItemNotFound", "(" ItemNotFound_init_params ")",
                  TP_FNORMAL, &DeeError_SequenceError, ItemNotFound, NULL, &ItemNotFound_print,
                  NULL, NULL, ItemNotFound_members, ItemNotFound_class_members);

/* Throws an `DeeError_ItemNotFound' indicating that a given item could not be found within some sequence */
PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrItemNotFound)(DeeObject *seq, DeeObject *item) {
	return DeeRT_ErrItemNotFoundEx(seq, item, 0, (size_t)-1, NULL);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrItemNotFoundEx)(DeeObject *seq, DeeObject *item,
                                size_t start, size_t end, DeeObject *key) {
	DREF ItemNotFound *result = DeeObject_MALLOC(ItemNotFound);
	if unlikely(!result)
		goto err;
	if (key && DeeNone_Check(key))
		key = NULL;
	result->inf_base.e_message = NULL;
	result->inf_base.e_inner   = NULL;
	Dee_variant_init_object(&result->inf_base.ve_value, seq);
	Dee_Incref(item);
	result->inf_item = item;
	result->inf_start = start;
	if (end == (size_t)-1) {
		Dee_variant_init_unbound(&result->inf_end);
	} else {
		Dee_variant_init_size(&result->inf_end, end);
	}
	Dee_XIncref(key);
	result->inf_key = key;
	DeeObject_Init(&result->inf_base, &DeeError_ItemNotFound);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}




/************************************************************************/
/* Error.ValueError.SequenceError.ItemNotFound.RegexNotFound            */
/************************************************************************/
typedef struct {
	ItemNotFound rnf_base;  /* "seq" -> "data", "item" -> "regex", "key" -> "rules" */
	size_t       rnf_range; /* [const] Regex scan range (in characters) */
} RegexNotFound;

PRIVATE struct type_member tpconst RegexNotFound_members[] = {
#define RegexNotFound_init_params Error_init_params ",data?:?X2?Dstring?DBytes,regex?:?Dstring,start=!0,end?:?Dint,range=!0,rules?:?Dstring"
	TYPE_MEMBER_FIELD_DOC("data", STRUCT_OBJECT, offsetof(RegexNotFound, rnf_base.inf_base.ve_value), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_FIELD_DOC("regex", STRUCT_OBJECT, offsetof(RegexNotFound, rnf_base.inf_item), "->?Dstring"),
	TYPE_MEMBER_FIELD("start", STRUCT_SIZE_T | STRUCT_CONST, offsetof(RegexNotFound, rnf_base.inf_start)),
	TYPE_MEMBER_FIELD_DOC("end", STRUCT_VARIANT | STRUCT_CONST, offsetof(RegexNotFound, rnf_base.inf_end), "->?Dint"),
	TYPE_MEMBER_FIELD_DOC("range", STRUCT_SIZE_T | STRUCT_CONST, offsetof(RegexNotFound, rnf_range), "->?Dint"),
	TYPE_MEMBER_FIELD_DOC("rules", STRUCT_OBJECT, offsetof(RegexNotFound, rnf_base.inf_key), "->?Dstring"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
RegexNotFound_print(RegexNotFound *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	struct Dee_variant active_end;
	if (self->rnf_base.inf_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	result = DeeFormat_Printf(printer, arg,
	                          "Could not locate regex pattern %r in %Vr",
	                          self->rnf_base.inf_item,
	                          &self->rnf_base.inf_base.ve_value);
	if unlikely(result < 0)
		return result;
	Dee_variant_init_copy(&active_end, &self->rnf_base.inf_end);
	if (Dee_variant_isbound_nonatomic(&active_end) || self->rnf_base.inf_start) {
		temp = DeeFormat_Printf(printer, arg, " in range [%" PRFuSIZ ",%Vk)",
		                        self->rnf_base.inf_start, &active_end);
		if unlikely(temp < 0) {
			Dee_variant_fini(&active_end);
			goto err_temp;
		}
		result += temp;
	}
	Dee_variant_fini(&active_end);
	if (self->rnf_range != 0 && self->rnf_range != (size_t)-1) {
		temp = DeeFormat_Printf(printer, arg, " after %" PRFuSIZ " attempts",
		                        self->rnf_range);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	if (self->rnf_base.inf_key) {
		temp = DeeFormat_Printf(printer, arg, " with rules %r",
		                        self->rnf_base.inf_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PUBLIC DeeTypeObject DeeError_RegexNotFound =
INIT_CUSTOM_ERROR("RegexNotFound", "(" RegexNotFound_init_params ")",
                  TP_FNORMAL, &DeeError_ItemNotFound, RegexNotFound, NULL, &RegexNotFound_print,
                  NULL, NULL, RegexNotFound_members, NULL);

/* Throws an `DeeError_RegexNotFound' indicating that
 * the given "regex" could not be found within "data"
 * @param: eflags: Set of `DEE_RE_EXEC_*' */
PUBLIC ATTR_COLD NONNULL((1, 2)) int
(DCALL DeeRT_ErrRegexNotFound)(DeeObject *data, DeeObject *regex,
                               size_t start, size_t end, size_t range,
                               DeeObject *rules, unsigned int eflags) {
	DREF RegexNotFound *result = DeeObject_MALLOC(RegexNotFound);
	if unlikely(!result)
		goto err;
	if (rules == NULL)
		rules = Dee_EmptyString;
	result->rnf_base.inf_base.e_message = NULL;
	result->rnf_base.inf_base.e_inner   = NULL;
	Dee_variant_init_object(&result->rnf_base.inf_base.ve_value, data);
	Dee_Incref(regex);
	result->rnf_base.inf_item  = regex;
	result->rnf_base.inf_start = start;
	if (end == (size_t)-1) {
		Dee_variant_init_unbound(&result->rnf_base.inf_end);
	} else {
		Dee_variant_init_size(&result->rnf_base.inf_end, end);
	}
	Dee_Incref(rules);
	result->rnf_base.inf_key = rules;
	result->rnf_range = range;
	(void)eflags; /* TODO: Once user-code can set these, must also save them here! */
	DeeObject_Init(&result->rnf_base.inf_base, &DeeError_RegexNotFound);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}


/************************************************************************/
/* Error.ValueError.SequenceError.UnpackError                           */
/************************************************************************/
typedef struct {
	SequenceError      ue_base;
	struct Dee_variant ue_count;    /* [const] Actual value count */
	struct Dee_variant ue_mincount; /* [const] Lower bound for expected count */
	struct Dee_variant ue_maxcount; /* [const] Upper bound for expected count */
} UnpackError;

PRIVATE struct type_member tpconst UnpackError_members[] = {
#define UnpackError_init_params SequenceError_init_params ",count?:?X2?DNumeric?Dint,mincount?:?X2?DNumeric?Dint,maxcount?:?X2?DNumeric?Dint"
	TYPE_MEMBER_FIELD_DOC("count", STRUCT_VARIANT | STRUCT_CONST, offsetof(UnpackError, ue_count), "->?X2?DNumeric?Dint"),
	TYPE_MEMBER_FIELD_DOC("mincount", STRUCT_VARIANT | STRUCT_CONST, offsetof(UnpackError, ue_mincount), "->?X2?DNumeric?Dint"),
	TYPE_MEMBER_FIELD_DOC("maxcount", STRUCT_VARIANT | STRUCT_CONST, offsetof(UnpackError, ue_maxcount), "->?X2?DNumeric?Dint"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_UnpackError =
INIT_CUSTOM_ERROR("UnpackError", "(" UnpackError_init_params ")",
                  TP_FNORMAL, &DeeError_SequenceError, UnpackError, NULL, NULL,
                  NULL, NULL, UnpackError_members, NULL);


/************************************************************************/
/* Error.ValueError.UnicodeError                                        */
/************************************************************************/
typedef ValueError UnicodeError;
PRIVATE struct type_member tpconst UnicodeError_members[] = {
#define UnicodeError_init_params Error_init_params ",string?:?Dstring"
	TYPE_MEMBER_FIELD_DOC("string", STRUCT_VARIANT | STRUCT_CONST, offsetof(UnicodeError, ve_value), "->?Dstring"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst unicodeerror_class_members[] = {
	TYPE_MEMBER_CONST("UnicodeDecodeError", &DeeError_UnicodeDecodeError),
	TYPE_MEMBER_CONST("UnicodeEncodeError", &DeeError_UnicodeEncodeError),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_UnicodeError =
INIT_CUSTOM_ERROR("UnicodeError", "(" UnicodeError_init_params ")",
                  TP_FNORMAL, &DeeError_ValueError, UnicodeError, NULL, NULL,
                  NULL, NULL, UnicodeError_members, unicodeerror_class_members);


/************************************************************************/
/* Error.ValueError.UnicodeError.UnicodeDecodeError                     */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_UnicodeDecodeError =
INIT_LIKE_BASECLASS("UnicodeDecodeError", "(" UnicodeError_init_params ")",
                    TP_FNORMAL, &DeeError_UnicodeError, UnicodeError, NULL, NULL,
                    NULL, NULL, NULL);


/************************************************************************/
/* Error.ValueError.UnicodeError.UnicodeDecodeError                     */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_UnicodeEncodeError =
INIT_LIKE_BASECLASS("UnicodeEncodeError", "(" UnicodeError_init_params ")",
                    TP_FNORMAL, &DeeError_UnicodeError, UnicodeError, NULL, NULL,
                    NULL, NULL, NULL);


/************************************************************************/
/* Error.ValueError.ReferenceError                                      */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_ReferenceError =
INIT_LIKE_BASECLASS("ReferenceError", "(" ValueError_init_params ")",
                    TP_FNORMAL, &DeeError_ValueError, ValueError, NULL, NULL,
                    NULL, NULL, NULL);


/************************************************************************/
/* Error.ValueError.BufferError                                         */
/************************************************************************/
typedef ValueError BufferError;
PRIVATE struct type_member tpconst BufferError_members[] = {
#define BufferError_init_params Error_init_params ",buffer?:?X2?DBytes?O"
	TYPE_MEMBER_FIELD_DOC("buffer", STRUCT_VARIANT | STRUCT_CONST, offsetof(BufferError, ve_value), "->?X2?DBytes?O"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_BufferError =
INIT_CUSTOM_ERROR("BufferError", "(" BufferError_init_params ")",
                  TP_FNORMAL, &DeeError_ValueError, BufferError, NULL, NULL,
                  NULL, NULL, BufferError_members, NULL);





/************************************************************************/
/* Error.ValueError.ArithmeticError.IntegerOverflow                     */
/************************************************************************/
typedef struct {
	ArithmeticError    io_base;
	struct Dee_variant io_minval;   /* [const] Min valid value */
	struct Dee_variant io_maxval;   /* [const] Max valid value */
	bool               io_positive; /* [const] Is the overflow positive? */
} IntegerOverflow;

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
integer_overflow_print(IntegerOverflow *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	if (self->io_base.e_message)
		return DeeString_PrintUtf8((DeeObject *)self->io_base.e_message, printer, arg);
	return DeeFormat_Printf(printer, arg,
	                        "%s integer overflow: %Vk exceeds range of valid values [%Vk,%Vk]",
	                        self->io_positive ? "positive" : "negative",
	                        &self->io_base.ve_value, &self->io_minval, &self->io_maxval);
}

PRIVATE struct type_member tpconst integer_overflow_members[] = {
	/*TYPE_MEMBER_FIELD_DOC("value", STRUCT_VARIANT | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_base.ve_value),
	                      "->?DNumeric\n"
	                      "Value that is overflowing"),*/
	TYPE_MEMBER_FIELD_DOC("minval", STRUCT_VARIANT | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_minval),
	                      "->?DNumeric\n"
	                      "Smallest acceptable value (?#value is either less than this, or greater than ?#maxval)"),
	TYPE_MEMBER_FIELD_DOC("maxval", STRUCT_VARIANT | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_maxval),
	                      "->?DNumeric\n"
	                      "Greatest acceptable value (?#value is either greater than this, or less than ?#minval)"),
	TYPE_MEMBER_FIELD_DOC("positive", STRUCT_BOOL(__SIZEOF_BOOL__) | STRUCT_CONST,
	                      offsetof(IntegerOverflow, io_positive),
	                      "If true, ?#value is greater than ?#maxval, else it is less than ?#minval"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_IntegerOverflow =
INIT_CUSTOM_ERROR("IntegerOverflow",
                  "(" ArithmeticError_init_params ",minval?:?DNumeric,maxval?:?DNumeric,positive=!f)",
                  TP_FNORMAL, &DeeError_ArithmeticError,
                  IntegerOverflow, NULL, &integer_overflow_print,
                  NULL, NULL, integer_overflow_members, NULL);

/* Throws a `DeeError_IntegerOverflow' indicating that some an integer
 * object or native (C) value cannot be used/processed because its value
 * exceeds the maximum supported value bounds within some context-of-use.
 *
 * The unsigned overflow throwing functions will only take the upper
 * bound (greatest) of valid values, and assume that the lower bound
 * is equal to `0'
 *
 * @param: positive: When true, assume "value > maxval".
 *                   Else, assume "value < maxval" */
PUBLIC ATTR_COLD NONNULL((1, 2, 3)) int
(DCALL DeeRT_ErrIntegerOverflow)(/*Numeric*/ DeeObject *value,
                                 /*Numeric*/ DeeObject *minval,
                                 /*Numeric*/ DeeObject *maxval,
                                 bool positive) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->io_base.e_message = NULL;
	result->io_base.e_inner   = NULL;
	Dee_variant_init_object(&result->io_base.ve_value, value);
	Dee_variant_init_object(&result->io_minval, minval);
	Dee_variant_init_object(&result->io_maxval, maxval);
	result->io_positive = positive;
	DeeObject_Init(&result->io_base, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

/* Same as "DeeRT_ErrIntegerOverflow", but minval/maxval are set as
 * >> minval   = (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) ? -(1 << (num_bits - 1)) : 0;
 * >> maxval   = (flags & DeeRT_ErrIntegerOverflowEx_F_SIGNED) ? ((1 << (num_bits - 1)) - 1) : ((1 << num_bits) - 1);
 * >> positive = (flags & DeeRT_ErrIntegerOverflowEx_F_POSITIVE) != 0; */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIntegerOverflowEx)(/*Numeric*/ DeeObject *value,
                                   size_t num_bits,
                                   unsigned int flags) {
	size_t max_bits;
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->io_base.e_message = NULL;
	result->io_base.e_inner   = NULL;
	max_bits = num_bits;
	if (flags & DeeRT_ErrIntegerOverflowEx_F_SIGNED)
		--max_bits;
	Dee_variant_init_uint32(&result->io_minval, 0);
	if (num_bits <= 32) {
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN))
			Dee_variant_init_int32(&result->io_minval, -((int32_t)1 << (num_bits - 1)));
		Dee_variant_init_uint32(&result->io_maxval, max_bits == 32 ? UINT32_MAX : ((uint32_t)1 << max_bits) - 1);
	} else if (num_bits <= 64) {
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN))
			Dee_variant_init_int64(&result->io_minval, -((int64_t)1 << (num_bits - 1)));
		Dee_variant_init_uint64(&result->io_maxval, max_bits == 64 ? UINT64_MAX : ((uint64_t)1 << max_bits) - 1);
	} else if (num_bits <= 128) {
		Dee_uint128_t maxval;
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) {
			Dee_int128_t minval;
			__hybrid_int128_setone(minval);
			__hybrid_int128_shl(minval, num_bits - 1);
			__hybrid_int128_neg(minval);
			Dee_variant_init_int128(&result->io_minval, minval);
		}
		if (max_bits == 128) {
			__hybrid_uint128_setzero(maxval);
		} else {
			__hybrid_uint128_setone(maxval);
			__hybrid_uint128_shl(maxval, max_bits);
		}
		__hybrid_uint128_dec(maxval);
		Dee_variant_init_uint128(&result->io_maxval, maxval);
	} else {
		DREF DeeObject *maxval, *shift;
		shift = DeeInt_NewSize(max_bits);
		if unlikely(!shift)
			goto err_r;
		maxval = DeeObject_Shl(DeeInt_One, shift);
		Dee_Decref(shift);
		if unlikely(!maxval)
			goto err_r;
		if unlikely(DeeObject_Dec(&maxval)) {
			Dee_Decref(maxval);
			goto err_r;
		}
		Dee_variant_init_object_inherited(&result->io_maxval, maxval);
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) {
			DREF DeeObject *temp, *minval;
			shift = DeeInt_NewSize(num_bits - 1);
			if unlikely(!shift) {
err_r_maxval:
				Dee_variant_fini(&result->io_maxval);
				goto err_r;
			}
			temp = DeeObject_Shl(DeeInt_One, shift);
			Dee_Decref(shift);
			if unlikely(!temp)
				goto err_r_maxval;
			minval = DeeObject_Neg(temp);
			Dee_Decref(temp);
			if unlikely(!minval)
				goto err_r_maxval;
			Dee_variant_init_object_inherited(&result->io_minval, minval);
		}
	}
	Dee_variant_init_object(&result->io_base.ve_value, value);
	result->io_positive = (flags & DeeRT_ErrIntegerOverflowEx_F_POSITIVE) != 0;
	DeeObject_Init(&result->io_base, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err_r:
	DeeObject_FREE(result);
err:
	return -1;
}


#if __SIZEOF_SIZE_T__ < 8
PRIVATE ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflow32_impl)(int32_t value,
                                        int32_t minval,
                                        int32_t maxval,
                                        bool is_signed) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->io_base.e_message = NULL;
	result->io_base.e_inner   = NULL;
	Dee_variant_init_int32(&result->io_base.ve_value, value);
	Dee_variant_init_int32(&result->io_minval, minval);
	Dee_variant_init_int32(&result->io_maxval, maxval);
	result->io_positive = value > maxval;
	if (!is_signed) {
		result->io_base.ve_value.var_type = Dee_VARIANT_UINT32;
		result->io_minval.var_type        = Dee_VARIANT_UINT32;
		result->io_maxval.var_type        = Dee_VARIANT_UINT32;
		result->io_positive = true;
	}
	DeeObject_Init(&result->io_base, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}
#endif /* __SIZEOF_SIZE_T__ < 8 */

PRIVATE ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflow64_impl)(int64_t value,
                                        int64_t minval,
                                        int64_t maxval,
                                        bool is_signed) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->io_base.e_message = NULL;
	result->io_base.e_inner   = NULL;
	Dee_variant_init_int64(&result->io_base.ve_value, value);
	Dee_variant_init_int64(&result->io_minval, minval);
	Dee_variant_init_int64(&result->io_maxval, maxval);
	result->io_positive = value > maxval;
	if (!is_signed) {
		result->io_base.ve_value.var_type = Dee_VARIANT_UINT64;
		result->io_minval.var_type        = Dee_VARIANT_UINT64;
		result->io_maxval.var_type        = Dee_VARIANT_UINT64;
		result->io_positive = true;
	}
	DeeObject_Init(&result->io_base, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PRIVATE ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflow128_impl)(Dee_int128_t value,
                                         Dee_int128_t minval,
                                         Dee_int128_t maxval,
                                         bool is_signed) {
	DREF IntegerOverflow *result = DeeObject_MALLOC(IntegerOverflow);
	if unlikely(!result)
		goto err;
	result->io_base.e_message = NULL;
	result->io_base.e_inner   = NULL;
	Dee_variant_init_int128(&result->io_base.ve_value, value);
	Dee_variant_init_int128(&result->io_minval, minval);
	Dee_variant_init_int128(&result->io_maxval, maxval);
	result->io_positive = __hybrid_int128_gr128(value, maxval);
	if (!is_signed) {
		result->io_base.ve_value.var_type = Dee_VARIANT_UINT128;
		result->io_minval.var_type        = Dee_VARIANT_UINT128;
		result->io_maxval.var_type        = Dee_VARIANT_UINT128;
		result->io_positive = true;
	}
	DeeObject_Init(&result->io_base, &DeeError_IntegerOverflow);
	return DeeError_ThrowInherited((DeeObject *)result);
err:
	return -1;
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowS)(Dee_ssize_t value,
                                  Dee_ssize_t minval,
                                  Dee_ssize_t maxval) {
#if __SIZEOF_SIZE_T__ < 8
	return DeeRT_ErrIntegerOverflow32_impl(value, minval, maxval, true);
#else /* __SIZEOF_SIZE_T__ < 8 */
	return DeeRT_ErrIntegerOverflow64_impl(value, minval, maxval, true);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowU)(size_t value, size_t maxval) {
#if __SIZEOF_SIZE_T__ < 8
	return DeeRT_ErrIntegerOverflow32_impl((int32_t)value, 0,
	                                       (int32_t)maxval, false);
#else /* __SIZEOF_SIZE_T__ < 8 */
	return DeeRT_ErrIntegerOverflow64_impl((int64_t)value, 0,
	                                       (int64_t)maxval, false);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}

#if __SIZEOF_SIZE_T__ < 8
PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowS64)(int64_t value,
                                    int64_t minval,
                                    int64_t maxval) {
	return DeeRT_ErrIntegerOverflow64_impl(value, minval, maxval, true);
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowU64)(uint64_t value, uint64_t maxval) {
	return DeeRT_ErrIntegerOverflow64_impl((int64_t)value, 0,
	                                       (int64_t)maxval, false);
}
#endif /* __SIZEOF_SIZE_T__ < 8 */

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowS128)(Dee_int128_t value,
                                     Dee_int128_t minval,
                                     Dee_int128_t maxval) {
	return DeeRT_ErrIntegerOverflow128_impl(value, minval, maxval, true);
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowU128)(Dee_uint128_t value,
                                     Dee_uint128_t maxval) {
	Dee_int128_t used_value;
	Dee_int128_t used_minval;
	Dee_int128_t used_maxval;
	memcpy(&used_value, &value, sizeof(used_value));
	__hybrid_int128_setzero(used_minval);
	memcpy(&used_maxval, &maxval, sizeof(used_maxval));
	return DeeRT_ErrIntegerOverflow128_impl(used_value,
	                                        used_minval,
	                                        used_maxval,
	                                        false);
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowUMul)(size_t lhs, size_t rhs) {
#if __SIZEOF_SIZE_T__ < 8
	uint64_t value = (uint64_t)lhs * (uint64_t)rhs;
	return DeeRT_ErrIntegerOverflowU64(value, SIZE_MAX);
#else /* __SIZEOF_SIZE_T__ < 8 */
	Dee_uint128_t value, maxval;
	__hybrid_uint128_set(value, lhs);
	__hybrid_uint128_mul(value, rhs);
	__hybrid_uint128_set(maxval, SIZE_MAX);
	return DeeRT_ErrIntegerOverflowU128(value, maxval);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}

PUBLIC ATTR_COLD int
(DCALL DeeRT_ErrIntegerOverflowUAdd)(size_t lhs, size_t rhs) {
#if __SIZEOF_SIZE_T__ < 8
	uint64_t value = (uint64_t)lhs + (uint64_t)rhs;
	return DeeRT_ErrIntegerOverflowU64(value, SIZE_MAX);
#else /* __SIZEOF_SIZE_T__ < 8 */
	Dee_uint128_t value, maxval;
	__hybrid_uint128_set(value, lhs);
	__hybrid_uint128_add(value, rhs);
	__hybrid_uint128_set(maxval, SIZE_MAX);
	return DeeRT_ErrIntegerOverflowU128(value, maxval);
#endif /* __SIZEOF_SIZE_T__ >= 8 */
}




/* Check if the currently-thrown exception is an `IntegerOverflow'. If so, wrap that
 * error within an `IndexError' (setting it as the `IndexError's "inner"), and using
 * `seq' as the accompanying sequence.
 *
 * If the currently-thrown exception isn't an `IntegerOverflow', do nothing.
 *
 * @return: -1: Always returns `-1', no matter what this function ended up doing. */
PUBLIC ATTR_COLD NONNULL((1)) int
(DCALL DeeRT_ErrIndexOverflow)(DeeObject *seq) {
	DREF DeeThreadObject *me = DeeThread_Self();
	struct except_frame *error = me->t_except;
	ASSERT(error != NULL);
	ASSERT(me->t_exceptsz > 0);
	if (DeeObject_InstanceOf(error->ef_error, &DeeError_IntegerOverflow) &&
	    likely(!(atomic_read(&me->t_state) & Dee_THREAD_STATE_TERMINATED))) {
		IntegerOverflow *overflow = (IntegerOverflow *)error->ef_error;
		DREF IndexError *result = DeeObject_MALLOC(IndexError);
		if unlikely(!result) {
			struct except_frame *oom = me->t_except;
			if likely(oom != error) {
				ASSERT(me->t_exceptsz >= 2);
				ASSERT(oom->ef_prev == error);
				oom->ef_prev = error->ef_prev;
				error->ef_prev = oom;
				me->t_except = error;
				/* Handle the original IntegerOverflow error so only the OOM error remains */
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			}
			goto done;
		}

		/* Set IndexError parameters:
		 * - "seq"    (ve_value)  = <caller-given "seq">
		 * - "index"  (ke_key)    = <value from "IntegerOverflow" error>
		 * - "length" (ie_length) = <unbound (lazily load from "seq")> */
		Dee_variant_init_object(&result->ie_base.ke_base.ve_value, seq);
		Dee_variant_init_copy(&result->ie_base.ke_key, &overflow->io_base.ve_value);
		Dee_variant_init_unbound(&result->ie_length);
		result->ie_base.ke_base.e_message = NULL;
		result->ie_base.ke_base.e_inner   = (DREF DeeObject *)overflow; /* Inherit reference */
		DeeObject_Init(&result->ie_base.ke_base, &DeeError_IndexError);
		error->ef_error = (DREF DeeObject *)result; /* Inherit reference */
	}
done:
	return -1;
}






PRIVATE DeeErrorObject RT_ErrNoActiveException = {
	OBJECT_HEAD_INIT(&DeeError_RuntimeError),
	/* .e_message = */ (DeeStringObject *)&str_No_active_exception,
	/* .e_inner   = */ NULL,
};

PUBLIC ATTR_COLD int (DCALL DeeRT_ErrNoActiveException)(void) {
	return DeeError_Throw((DeeObject *)&RT_ErrNoActiveException);
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ERROR_RT_C */
