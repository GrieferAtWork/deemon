/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
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
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/exec.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bzero(), ... */
#include <deemon/system.h>
#include <deemon/tuple.h>

#include "../runtime/strings.h"

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */


DECL_BEGIN


#define INIT_CUSTOM_ERROR(tp_name, tp_doc, tp_flags, tp_base,                                          \
                          tp_ctor, tp_copy_ctor, tp_deep_ctor, tp_any_ctor,                            \
                          T, tp_dtor, tp_str, tp_visit,                                                \
                          tp_methods, tp_getsets, tp_members,                                          \
                          tp_class_members)                                                            \
	{                                                                                                  \
		OBJECT_HEAD_INIT(&DeeType_Type),                                                               \
		/* .tp_name     = */ tp_name,                                                                  \
		/* .tp_doc      = */ DOC(tp_doc),                                                              \
		/* .tp_flags    = */ tp_flags,                                                                 \
		/* .tp_weakrefs = */ 0,                                                                        \
		/* .tp_features = */ TF_NONE,                                                                  \
		/* .tp_base     = */ tp_base,                                                                  \
		/* .tp_init = */ {                                                                             \
			{                                                                                          \
				/* .tp_alloc = */ {                                                                    \
					/* .tp_ctor      = */ (void *)(tp_ctor),                                           \
					/* .tp_copy_ctor = */ (void *)(tp_copy_ctor),                                      \
					/* .tp_deep_ctor = */ (void *)(tp_deep_ctor),                                      \
					/* .tp_any_ctor  = */ (void *)(tp_any_ctor),                                       \
					TYPE_FIXED_ALLOCATOR(T)                                                            \
				}                                                                                      \
			},                                                                                         \
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))(tp_dtor),                  \
			/* .tp_assign      = */ NULL,                                                              \
			/* .tp_move_assign = */ NULL                                                               \
		},                                                                                             \
		/* .tp_cast = */ {                                                                             \
			/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str),               \
			/* .tp_repr = */ NULL,                                                                     \
			/* .tp_bool = */ NULL                                                                      \
		},                                                                                             \
		/* .tp_call          = */ NULL,                                                                \
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))(tp_visit), \
		/* .tp_gc            = */ NULL,                                                                \
		/* .tp_math          = */ NULL,                                                                \
		/* .tp_cmp           = */ NULL,                                                                \
		/* .tp_seq           = */ NULL,                                                                \
		/* .tp_iter_next     = */ NULL,                                                                \
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


#ifndef CONFIG_NO_OBJECT_SLABS
LOCAL size_t DCALL get_slab_size(void (DCALL *tp_free)(void *__restrict ob)) {
#define CHECK_SIZE(index, size)                 \
	if (tp_free == &DeeObject_SlabFree##size || \
	    tp_free == &DeeGCObject_SlabFree##size) \
		return size * __SIZEOF_POINTER__;
	DeeSlab_ENUMERATE(CHECK_SIZE)
#undef CHECK_SIZE
	return 0;
}

#define GET_INSTANCE_SIZE(self)                                \
	(Dee_TYPE(self)->tp_init.tp_alloc.tp_free                  \
	 ? get_slab_size(Dee_TYPE(self)->tp_init.tp_alloc.tp_free) \
	 : Dee_TYPE(self)->tp_init.tp_alloc.tp_instance_size)
#else /* !CONFIG_NO_OBJECT_SLABS */
#define GET_INSTANCE_SIZE(self) \
	(Dee_TYPE(self)->tp_init.tp_alloc.tp_instance_size)
#endif /* CONFIG_NO_OBJECT_SLABS */


/* Try to construct the given error object using its legacy constructor.
 * Note that any additional memory are zero-initialized upon success (true),
 * and that no error will be thrown on failure (false).
 * NOTE: Upon failure, the message and inner fields are initialized to NULL. */
PRIVATE bool DCALL
error_try_init(DeeErrorObject *__restrict self,
               size_t argc, DeeObject *const *argv) {
	size_t instance_size;
	ASSERT(!(Dee_TYPE(self)->tp_flags & TP_FVARIABLE));
	instance_size = GET_INSTANCE_SIZE(self);
	ASSERT(instance_size >= sizeof(DeeErrorObject));
	switch (argc) {

	case 0:
		self->e_message = NULL;
		self->e_inner   = NULL;
		goto done_ok;

	case 1:
		if (!DeeString_Check(argv[0]))
			break;
		self->e_message = (DeeStringObject *)argv[0];
		Dee_Incref(self->e_message);
		self->e_inner = NULL;
		goto done_ok;

	case 2:
		if (!DeeString_Check(argv[0]))
			break;
		self->e_message = (DeeStringObject *)argv[0];
		self->e_inner   = argv[1];
		Dee_Incref(self->e_message);
		Dee_Incref(self->e_inner);
		goto done_ok;

	default: break;
	}
	self->e_message = NULL;
	self->e_inner   = NULL;
	return false;
done_ok:
	/* Clear our any additional fields. */
	bzero(self + 1, instance_size - sizeof(DeeErrorObject));
	return true;
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
error_ctor(DeeErrorObject *__restrict self) {
	size_t instance_size;
	ASSERT(!(Dee_TYPE(self)->tp_flags & TP_FVARIABLE));
	instance_size = GET_INSTANCE_SIZE(self);
	ASSERT(instance_size >= sizeof(DeeErrorObject));
	bzero(&self->e_message,
	      instance_size -
	      offsetof(DeeErrorObject, e_message));
	ASSERT(!self->e_inner);
	ASSERT(!self->e_message);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
error_copy(DeeErrorObject *__restrict self,
           DeeErrorObject *__restrict other) {
	size_t instance_size;
	ASSERT(!(Dee_TYPE(self)->tp_flags & TP_FVARIABLE));
	instance_size = GET_INSTANCE_SIZE(self);
	ASSERT(instance_size >= sizeof(DeeErrorObject));
	self->e_inner   = other->e_inner;
	self->e_message = other->e_message;
	Dee_XIncref(self->e_inner);
	Dee_XIncref(self->e_message);
	bzero(self + 1,
	      instance_size -
	      sizeof(DeeErrorObject));
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
error_deep(DeeErrorObject *__restrict self,
           DeeErrorObject *__restrict other) {
	size_t instance_size;
	ASSERT(!(Dee_TYPE(self)->tp_flags & TP_FVARIABLE));
	instance_size = GET_INSTANCE_SIZE(self);
	ASSERT(instance_size >= sizeof(DeeErrorObject));
	self->e_inner = NULL;
	if (other->e_inner) {
		self->e_inner = DeeObject_DeepCopy(other->e_inner);
		if unlikely(!self->e_inner)
			return -1;
	}
	self->e_message = other->e_message;
	Dee_XIncref(self->e_message);
	bzero(self + 1,
	      instance_size -
	      sizeof(DeeErrorObject));
	return 0;
}

PRIVATE char const error_init_fmt[]        = "|oo:Error";
PRIVATE struct keyword error_init_kwlist[] = { K(message), K(inner), KEND };

PRIVATE WUNUSED NONNULL((1)) int DCALL
error_init(DeeErrorObject *__restrict self,
           size_t argc, DeeObject *const *argv) {
	size_t instance_size;
	ASSERT(!(Dee_TYPE(self)->tp_flags & TP_FVARIABLE));
	instance_size = GET_INSTANCE_SIZE(self);
	ASSERTF(instance_size >= sizeof(DeeErrorObject),
	        "instance_size = %Iu\n"
	        "name = %q\n",
	        instance_size,
	        Dee_TYPE(self)->tp_name);
	self->e_message = NULL;
	self->e_inner   = NULL;
	if (DeeArg_Unpack(argc, argv, error_init_fmt, &self->e_message, &self->e_inner))
		goto err;
	if (self->e_message &&
	    DeeObject_AssertTypeExact(self->e_message, &DeeString_Type))
		goto err;
	Dee_XIncref(self->e_message);
	Dee_XIncref(self->e_inner);
	bzero(self + 1,
	      instance_size -
	      sizeof(DeeErrorObject));
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
error_init_kw(DeeErrorObject *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t instance_size;
	ASSERT(!(Dee_TYPE(self)->tp_flags & TP_FVARIABLE));
	instance_size = GET_INSTANCE_SIZE(self);
	ASSERT(instance_size >= sizeof(DeeErrorObject));
	self->e_message = NULL;
	self->e_inner   = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, error_init_kwlist, error_init_fmt, &self->e_message, &self->e_inner))
		goto err;
	if (self->e_message &&
	    DeeObject_AssertTypeExact(self->e_message, &DeeString_Type))
		goto err;
	Dee_XIncref(self->e_message);
	Dee_XIncref(self->e_inner);
	bzero(self + 1,
	      instance_size -
	      sizeof(DeeErrorObject));
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
error_fini(DeeErrorObject *__restrict self) {
	Dee_XDecref(self->e_message);
	Dee_XDecref(self->e_inner);
}

PRIVATE NONNULL((1, 2)) void DCALL
error_visit(DeeErrorObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->e_message);
	Dee_XVisit(self->e_inner);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
error_str(DeeErrorObject *__restrict self) {
	if (self->e_message)
		return_reference_((DeeObject *)self->e_message);
	if (self->e_inner)
		return DeeString_Newf("%k -> %k", Dee_TYPE(self), self->e_inner);
	return DeeObject_Str((DeeObject *)Dee_TYPE(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
error_repr(DeeErrorObject *__restrict self) {
	if (self->e_inner) {
		if (self->e_message) {
			return DeeString_Newf("%k(%r, inner: %r)",
			                      Dee_TYPE(self),
			                      self->e_message,
			                      self->e_inner);
		}
		return DeeString_Newf("%k(inner: %r)",
		                      Dee_TYPE(self),
		                      self->e_inner);
	}
	return DeeString_Newf("%k(%r)",
	                      Dee_TYPE(self),
	                      self->e_message);
}

PRIVATE struct type_member tpconst error_members[] = {
	TYPE_MEMBER_FIELD_DOC("inner", STRUCT_OBJECT_OPT, offsetof(DeeErrorObject, e_inner),
	                      "->?X3?DError?O?N\n"
	                      "An optional inner error object, or ?N when not set"),
	TYPE_MEMBER_FIELD_DOC("message", STRUCT_OBJECT_OPT, offsetof(DeeErrorObject, e_message),
	                      "->?X2?Dstring?N\n"
	                      "The error message associated with this Error object, or ?N when not set"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_Error = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Error),
	/* .tp_doc      = */ DOC("Base class for all errors thrown by the runtime\n"
	                         "\n"
	                         "(message?:?Dstring,inner?:?X2?DError?O)\n"
	                         "Create a new error object with the given @message and @inner error"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&error_ctor,
				/* .tp_copy_ctor = */ (void *)&error_copy,
				/* .tp_deep_ctor = */ (void *)&error_deep,
				/* .tp_any_ctor  = */ (void *)&error_init,
				TYPE_FIXED_ALLOCATOR(DeeErrorObject),
				/* .tp_any_ctor_kw = */ (void *)&error_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&error_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&error_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&error_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&error_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ error_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ error_class_members
};
/* END::Error */











/* BEGIN::AttributeError */
PRIVATE struct type_member tpconst attribute_error_class_members[] = {
	TYPE_MEMBER_CONST("UnboundAttribute", &DeeError_UnboundAttribute),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_AttributeError =
INIT_CUSTOM_ERROR("AttributeError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, attribute_error_class_members);
PUBLIC DeeTypeObject DeeError_UnboundAttribute =
INIT_CUSTOM_ERROR("UnboundAttribute", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_AttributeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::AttributeError */






/* BEGIN::CompilerError */
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
             dvisit_t proc, void *arg) {
	size_t i, count = self->ce_errorc;
	for (i = 0; i < count; ++i) {
		if (self->ce_errorv[i] != self)
			Dee_Visit(self->ce_errorv[i]);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
comerr_str(DeeCompilerErrorObject *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeCompilerError_Print((DeeObject *)self,
	                                   &unicode_printer_print,
	                                   &printer) < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE struct type_member tpconst compiler_error_class_members[] = {
	TYPE_MEMBER_CONST("SyntaxError", &DeeError_SyntaxError),
	TYPE_MEMBER_CONST("SymbolError", &DeeError_SymbolError),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_CompilerError =
INIT_CUSTOM_ERROR("CompilerError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, NULL, NULL, NULL, DeeCompilerErrorObject,
                  &comerr_fini, &comerr_str, &comerr_visit, NULL, NULL, NULL,
                  compiler_error_class_members);
PUBLIC DeeTypeObject DeeError_SyntaxError =
INIT_CUSTOM_ERROR("SyntaxError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_CompilerError,
                  NULL, NULL, NULL, NULL, DeeCompilerErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_SymbolError =
INIT_CUSTOM_ERROR("SymbolError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_CompilerError,
                  NULL, NULL, NULL, NULL, DeeCompilerErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::CompilerError */






PUBLIC DeeTypeObject DeeError_ThreadCrash =
INIT_CUSTOM_ERROR("ThreadCrash", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);




/* BEGIN::NoMemory */
PRIVATE struct type_member tpconst nomemory_members[] = {
	TYPE_MEMBER_FIELD("size", STRUCT_SIZE_T, offsetof(DeeNoMemoryErrorObject, nm_allocsize)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
nomemory_ctor(DeeNoMemoryErrorObject *__restrict self,
              size_t argc, DeeObject *const *argv) {
	if (error_try_init((DeeErrorObject *)self, argc, argv))
		goto done;
	self->nm_allocsize = 1;
	if (DeeArg_Unpack(argc, argv, "|Iu:NoMemory", &self->nm_allocsize))
		return -1;
done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
nomemory_copy(DeeNoMemoryErrorObject *__restrict self,
              DeeNoMemoryErrorObject *__restrict other) {
	self->nm_allocsize = other->nm_allocsize;
	return error_copy((DeeErrorObject *)self, (DeeErrorObject *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
nomemory_deep(DeeNoMemoryErrorObject *__restrict self,
              DeeNoMemoryErrorObject *__restrict other) {
	self->nm_allocsize = other->nm_allocsize;
	return error_deep((DeeErrorObject *)self, (DeeErrorObject *)other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nomemory_str(DeeNoMemoryErrorObject *__restrict self) {
	if (self->nm_allocsize)
		return DeeString_Newf("Failed to allocated %Iu bytes",
		                      self->nm_allocsize);
	return error_str((DeeErrorObject *)self);
}

PUBLIC DeeTypeObject DeeError_NoMemory =
INIT_CUSTOM_ERROR("NoMemory", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, &nomemory_copy, &nomemory_deep, &nomemory_ctor,
                  DeeNoMemoryErrorObject, NULL, &nomemory_str, NULL,
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
PUBLIC DeeTypeObject DeeError_RuntimeError =
INIT_CUSTOM_ERROR("RuntimeError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, runtimeerror_class_members);
PUBLIC DeeTypeObject DeeError_NotImplemented =
INIT_CUSTOM_ERROR("NotImplemented", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_RuntimeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_AssertionError =
INIT_CUSTOM_ERROR("AssertionError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_RuntimeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_UnboundLocal =
INIT_CUSTOM_ERROR("UnboundLocal", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_RuntimeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_StackOverflow =
INIT_CUSTOM_ERROR("StackOverflow", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_RuntimeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_SegFault =
INIT_CUSTOM_ERROR("SegFault", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_RuntimeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_IllegalInstruction =
INIT_CUSTOM_ERROR("IllegalInstruction", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_RuntimeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::RuntimeError */




PUBLIC DeeTypeObject DeeError_TypeError =
INIT_CUSTOM_ERROR("TypeError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);




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
PUBLIC DeeTypeObject DeeError_ValueError =
INIT_CUSTOM_ERROR("ValueError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Error,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, valueerror_class_members);
/* BEGIN::ValueError.ArithmeticError */
PRIVATE struct type_member tpconst arithmetic_class_members[] = {
	TYPE_MEMBER_CONST("IntegerOverflow", &DeeError_IntegerOverflow),
	TYPE_MEMBER_CONST("DivideByZero", &DeeError_DivideByZero),
	TYPE_MEMBER_CONST("NegativeShift", &DeeError_NegativeShift),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_ArithmeticError =
INIT_CUSTOM_ERROR("ArithmeticError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, arithmetic_class_members);
PUBLIC DeeTypeObject DeeError_IntegerOverflow =
INIT_CUSTOM_ERROR("IntegerOverflow", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ArithmeticError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_DivideByZero =
INIT_CUSTOM_ERROR("DivideByZero", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ArithmeticError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_NegativeShift =
INIT_CUSTOM_ERROR("NegativeShift", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ArithmeticError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::ValueError.ArithmeticError */
PUBLIC DeeTypeObject DeeError_KeyError =
INIT_CUSTOM_ERROR("KeyError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* BEGIN::ValueError.IndexError */
PRIVATE struct type_member tpconst indexerror_class_members[] = {
	TYPE_MEMBER_CONST("UnboundItem", &DeeError_UnboundItem),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_IndexError =
INIT_CUSTOM_ERROR("IndexError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, indexerror_class_members);
PUBLIC DeeTypeObject DeeError_UnboundItem =
INIT_CUSTOM_ERROR("UnboundItem", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_IndexError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::ValueError.IndexError */
PUBLIC DeeTypeObject DeeError_SequenceError =
INIT_CUSTOM_ERROR("SequenceError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* BEGIN::ValueError.UnicodeError */
PRIVATE struct type_member tpconst unicodeerror_class_members[] = {
	TYPE_MEMBER_CONST("UnicodeDecodeError", &DeeError_UnicodeDecodeError),
	TYPE_MEMBER_CONST("UnicodeEncodeError", &DeeError_UnicodeEncodeError),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_UnicodeError =
INIT_CUSTOM_ERROR("UnicodeError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, unicodeerror_class_members);
PUBLIC DeeTypeObject DeeError_UnicodeDecodeError =
INIT_CUSTOM_ERROR("UnicodeDecodeError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_UnicodeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_UnicodeEncodeError =
INIT_CUSTOM_ERROR("UnicodeEncodeError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_UnicodeError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::ValueError.UnicodeError */
PUBLIC DeeTypeObject DeeError_ReferenceError =
INIT_CUSTOM_ERROR("ReferenceError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_UnpackError =
INIT_CUSTOM_ERROR("UnpackError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_BufferError =
INIT_CUSTOM_ERROR("BufferError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_ValueError,
                  NULL, NULL, NULL, NULL, DeeErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
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
	PRIVATE DEFINE_KWLIST(kwlist, { K(message), K(inner), K(errno), K(nterr_np), KEND });
	self->e_inner   = NULL;
	self->e_message = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|oooo:SystemError",
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
	PRIVATE DEFINE_KWLIST(kwlist, { K(message), K(inner), K(errno), KEND });
	int last_errno = DeeSystem_GetErrno();
	self->e_inner   = NULL;
	self->e_message = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|ooo:SystemError",
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
	return DeeModule_CallExternf("posix", name, "d", self->se_errno);
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
		result = Dee_None;
		Dee_Incref(result);
	}
	return result;
}
#endif /* CONFIG_HOST_WINDOWS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_str(DeeSystemErrorObject *__restrict self) {
	DREF DeeObject *errno_name, *errno_desc;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(unicode_printer_printobject(&printer,
	                                        self->e_message
	                                        ? (DeeObject *)self->e_message
	                                        : (DeeObject *)Dee_TYPE(self)) < 0)
		goto err;
	if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
#ifdef CONFIG_HOST_WINDOWS
		Dee_ssize_t unix_code_length;
#endif /* CONFIG_HOST_WINDOWS */
		if unlikely(unicode_printer_putascii(&printer, '\n'))
			goto err;
		errno_name = systemerror_getstrerrorname(self);
		if unlikely(!errno_name) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
				goto err;
			errno_name = Dee_None;
			Dee_Incref(errno_name);
		}
		errno_desc = systemerror_getstrerror(self);
		if unlikely(!errno_desc) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
				goto err_errno_name;
			errno_desc = Dee_None;
			Dee_Incref(errno_desc);
		}
#ifdef CONFIG_HOST_WINDOWS
		unix_code_length = unicode_printer_printf(&printer, "errno(%d)", self->se_errno);
		if unlikely(unix_code_length < 0)
			goto err_errno_desc;
#else /* CONFIG_HOST_WINDOWS */
		if unlikely(unicode_printer_printf(&printer, "errno(%d)", self->se_errno) < 0)
			goto err_errno_desc;
#endif /* !CONFIG_HOST_WINDOWS */
		if (!DeeNone_Check(errno_name) || !DeeNone_Check(errno_desc)) {
#ifdef CONFIG_HOST_WINDOWS
			if (self->se_lasterror != NO_ERROR) {
				size_t windows_code_length;
				windows_code_length = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%#I32x", self->se_lasterror);
				unix_code_length -= 7;
				if (windows_code_length > (size_t)unix_code_length) {
					if unlikely(unicode_printer_repeatascii(&printer, ' ',
					                                        windows_code_length -
					                                        (size_t)unix_code_length) < 0)
						goto err_errno_desc;
				}
			}
#endif /* CONFIG_HOST_WINDOWS */
			if unlikely(UNICODE_PRINTER_PRINT(&printer, ": ") < 0)
				goto err_errno_desc;
			if (DeeNone_Check(errno_name)) {
				if unlikely(unicode_printer_putascii(&printer, '?'))
					goto err_errno_desc;
			} else {
				if unlikely(unicode_printer_printobject(&printer, errno_name) < 0)
					goto err_errno_desc;
			}
			if (!DeeNone_Check(errno_desc)) {
				if unlikely(UNICODE_PRINTER_PRINT(&printer, " (") < 0)
					goto err_errno_desc;
				if unlikely(unicode_printer_printobject(&printer, errno_desc) < 0)
					goto err_errno_desc;
				if unlikely(unicode_printer_putascii(&printer, ')'))
					goto err_errno_desc;
			}
		}
		Dee_Decref(errno_desc);
		Dee_Decref(errno_name);
	}

#ifdef CONFIG_HOST_WINDOWS
	if (self->se_lasterror != NO_ERROR) {
		int message_error;
		Dee_ssize_t windows_code_length;
		if unlikely(unicode_printer_putascii(&printer, '\n'))
			goto err;
		windows_code_length = unicode_printer_printf(&printer, "nterr(%#I32x)", self->se_lasterror);
		if unlikely(windows_code_length < 0)
			goto err;
		if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
			size_t unix_code_length;
			unix_code_length = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%d", self->se_errno);
			windows_code_length -= 7;
			if (unix_code_length > (size_t)windows_code_length) {
				if unlikely(unicode_printer_repeatascii(&printer, ' ',
				                                        unix_code_length -
				                                        (size_t)windows_code_length) < 0)
					goto err;
			}
		}
		if unlikely(UNICODE_PRINTER_PRINT(&printer, ": ") < 0)
			goto err;
		message_error = DeeNTSystem_PrintFormatMessage(&printer, FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		                                               (DWORD)self->se_lasterror,
		                                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), NULL);
		if unlikely(message_error < 0)
			goto err;
		if unlikely(message_error > 0) {
			/* No message string printed... */
			if unlikely(unicode_printer_putascii(&printer, '?'))
				goto err;
		}
	}
#endif /* CONFIG_HOST_WINDOWS */
	return unicode_printer_pack(&printer);
err_errno_desc:
	Dee_Decref(errno_desc);
err_errno_name:
	Dee_Decref(errno_name);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_repr(DeeSystemErrorObject *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	bool is_first = true;
	if unlikely(unicode_printer_printf(&printer, "%k(", Dee_TYPE(self)) < 0)
		goto err;
	if (self->e_message) {
		if unlikely(unicode_printer_printobjectrepr(&printer, (DeeObject *)self->e_message) < 0)
			goto err;
		is_first = false;
	}
	if (self->e_inner) {
		if (!is_first && unlikely(UNICODE_PRINTER_PRINT(&printer, ", ") < 0))
			goto err;
		if unlikely(UNICODE_PRINTER_PRINT(&printer, "inner: ") < 0)
			goto err;
		if unlikely(unicode_printer_printobjectrepr(&printer, (DeeObject *)self->e_inner) < 0)
			goto err;
		is_first = false;
	}
	if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
		DREF DeeObject *errno_name;
		Dee_ssize_t print_error;
		if (!is_first && unlikely(UNICODE_PRINTER_PRINT(&printer, ", ") < 0))
			goto err;
		if unlikely(UNICODE_PRINTER_PRINT(&printer, "errno: ") < 0)
			goto err;
		errno_name = systemerror_getstrerrorname(self);
		if (!errno_name) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
				goto err;
			errno_name = Dee_None;
			Dee_Incref(errno_name);
		}
		print_error = DeeNone_Check(errno_name)
		              ? unicode_printer_printf(&printer, "%d", self->se_errno)
		              : unicode_printer_printobject(&printer, errno_name);
		Dee_Decref(errno_name);
		if unlikely(print_error < 0)
			goto err;
#ifdef CONFIG_HOST_WINDOWS
		is_first = false;
#endif /* CONFIG_HOST_WINDOWS */
	}
#ifdef CONFIG_HOST_WINDOWS
	if (self->se_lasterror != NO_ERROR) {
		if (!is_first && unlikely(UNICODE_PRINTER_PRINT(&printer, ", ") < 0))
			goto err;
		if unlikely(unicode_printer_printf(&printer, "nterr_np: %#I32x", self->se_lasterror) < 0)
			goto err;
	}
#endif /* CONFIG_HOST_WINDOWS */
	if unlikely(unicode_printer_putascii(&printer, ')'))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE struct type_getset tpconst systemerror_getsets[] = {
	{ "strerrorname",
	  (DREF DeeObject * (DCALL *)(DeeObject *__restrict))&systemerror_getstrerrorname, NULL, NULL,
	  DOC("->?Dstring\n"
	      "The name of the associated ?#errno (s.a. ?Eposix:strerrorname)\n"
	      "Returns ?Dnone if ?#errno doesn't have a known name") },
	{ "strerror",
	  (DREF DeeObject * (DCALL *)(DeeObject *__restrict))&systemerror_getstrerror, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "A description of the associated ?#errno (s.a. ?Eposix:strerror)\n"
	      "Returns ?Dnone if ?#errno doesn't have a description") },
#ifdef CONFIG_HOST_WINDOWS
	{ "nterrmsg_np",
	  (DREF DeeObject * (DCALL *)(DeeObject *__restrict))&systemerror_getnterrmsg_np, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "A description of the associated ?#nterr_np (s.a. ?Ewin32:FormatErrorMessage)\n"
	      "Returns ?Dnone if no message description is available") },
#endif /* CONFIG_HOST_WINDOWS */
	{ NULL }
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

PUBLIC DeeTypeObject DeeError_SystemError = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "SystemError",
#ifdef CONFIG_HOST_WINDOWS
	/* .tp_doc      = */ DOC("(message:?Dstring,inner:?DError,errno?:?X2?Dint?Dstring,nterr_np?:?Dint)"),
#else /* CONFIG_HOST_WINDOWS */
	/* .tp_doc      = */ DOC("(message:?Dstring,inner:?DError,errno?:?X2?Dint?Dstring)"),
#endif /* !CONFIG_HOST_WINDOWS */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_Error,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeSystemErrorObject),
				/* .tp_any_ctor_kw = */ &systemerror_init_kw
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&systemerror_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&systemerror_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ systemerror_getsets,
	/* .tp_members       = */ systemerror_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ systemerror_class_members
};

PUBLIC DeeTypeObject DeeError_UnsupportedAPI =
INIT_CUSTOM_ERROR("UnsupportedAPI", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_SystemError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PRIVATE struct type_member tpconst fserror_class_members[] = {
	TYPE_MEMBER_CONST("FileAccessError", &DeeError_FileAccessError),
	TYPE_MEMBER_CONST("FileNotFound", &DeeError_FileNotFound),
	TYPE_MEMBER_CONST("FileExists", &DeeError_FileExists),
	TYPE_MEMBER_CONST("FileClosed", &DeeError_FileClosed),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_FSError =
INIT_CUSTOM_ERROR("FSError", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_SystemError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, fserror_class_members);
PRIVATE struct type_member tpconst accesserror_members[] = {
	TYPE_MEMBER_CONST("ReadOnlyFile", &DeeError_ReadOnlyFile),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_FileAccessError =
INIT_CUSTOM_ERROR("FileAccessError", "An error derived from :FSError that is thrown when attempting "
                                     "to access a file or directory without the necessary permissions",
                  TP_FNORMAL | TP_FINHERITCTOR, &DeeError_FSError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, accesserror_members);
PUBLIC DeeTypeObject DeeError_ReadOnlyFile =
INIT_CUSTOM_ERROR("ReadOnlyFile", "An error derived from :FileAccessError that is thrown when attempting "
                                  "to modify a file or directory when it or the filesystem is read-only",
                  TP_FNORMAL | TP_FINHERITCTOR, &DeeError_FileAccessError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_FileNotFound =
INIT_CUSTOM_ERROR("FileNotFound", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_FSError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_FileExists =
INIT_CUSTOM_ERROR("FileExists", "An error derived from :FSError that is thrown when attempting "
                                "to create a filesystem object when the target path already exists",
                  TP_FNORMAL | TP_FINHERITCTOR, &DeeError_FSError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_FileClosed =
INIT_CUSTOM_ERROR("FileClosed", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_FSError,
                  NULL, NULL, NULL, NULL, DeeSystemErrorObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
/* END::SystemError */



PRIVATE int DCALL
appexit_init(struct appexit_object *__restrict self,
             size_t argc, DeeObject *const *argv) {
	int result;
	self->ae_exitcode = EXIT_FAILURE;
	/* Read the exitcode from arguments. */
	result = DeeArg_Unpack(argc, argv,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_str(struct appexit_object *__restrict self) {
	return DeeString_Newf("exit:%d", self->ae_exitcode);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_repr(struct appexit_object *__restrict self) {
	return DeeString_Newf("AppExit(%d)", self->ae_exitcode);
}

PRIVATE struct type_member tpconst appexit_members[] = {
	TYPE_MEMBER_FIELD("exitcode", STRUCT_CONST | STRUCT_INT,
	                  offsetof(struct appexit_object, ae_exitcode)),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_class_atexit(DeeObject *UNUSED(self),
                     size_t argc, DeeObject *const *argv) {
	DeeObject *callback, *args = Dee_EmptyTuple;
	if (DeeArg_Unpack(argc, argv, "o|o:atexit", &callback, &args))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (Dee_AtExit(callback, args))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC int DCALL Dee_Exit(int exitcode, bool run_atexit) {
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
			DeeError_Throw((DeeObject *)error);
			Dee_Decref(error);
		}
	}
	return -1;
#endif /* !CONFIG_HAVE_exit */
#endif /* !CONFIG_HAVE__Exit */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_class_exit(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	int exitcode = EXIT_FAILURE;
	bool run_atexit = true;
	if (DeeArg_Unpack(argc, argv, "|db:exit", &exitcode, &run_atexit))
		goto err;
	Dee_Exit(exitcode, run_atexit);
err:
	return NULL;
}

PRIVATE struct type_method tpconst appexit_class_methods[] = {
	{ "exit", &appexit_class_exit,
	  DOC("()\n"
	      "(exitcode:?Dint,invoke_atexit=!t)\n"
	      "Terminate execution of deemon after invoking ?#atexit callbacks when @invoke_atexit is ?Dtrue\n"
	      "Termination is done using the C #Cexit or #C_exit functions, if available. However if these "
	      "functions are not provided by the host, an :AppExit error is thrown instead\n"
	      "When no @exitcode is given, the host's default default value of #CEXIT_FAILURE, or $1 is used\n"
	      "This function never returns normally") },
	{ "atexit", &appexit_class_atexit,
	  DOC("(callback:?DCallable,args=!T0)\n"
	      "@throw RuntimeError Additional atexit-callbacks can no longer be registered\n"
	      "@throw NotImplemented Deemon was built without support for ?#atexit\n"
	      "Register a given @callback to be executed before deemon terminates") },
	{ NULL }
};

PUBLIC DeeTypeObject DeeError_AppExit = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "AppExit",
	/* .tp_doc      = */ DOC("An AppExit object is a special kind of interrupt that "
	                         "is not derived from ?O, and has no base class at "
	                         "all. It's purpose is to allow user-code to throw an "
	                         "instance of it and have the stack unwind itself, alongside "
	                         "all existing objects being destroyed normally before deemon "
	                         "will be terminated with the given exitcode\n"

	                         "\n"
	                         "()\n"
	                         "(exitcode:?Dint)\n"
	                         "Construct a new AppExit object using the given @exitcode "
	                         "or the host's default value for #CEXIT_FAILURE, or $1"),
	/* .tp_flags    = */ TP_FFINAL|TP_FINTERRUPT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL, /* No base type (to only allow AppExit-interrupt handlers,
	                            * and all-interrupt handlers to catch this error) */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ &appexit_copy,
				/* .tp_deep_ctor = */ &appexit_copy,
				/* .tp_any_ctor  = */ &appexit_init,
				TYPE_FIXED_ALLOCATOR(struct appexit_object)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&appexit_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&appexit_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ appexit_members,
	/* .tp_class_methods = */ appexit_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};






/* Fallback instance when throwing no-memory errors with an unknown size. */
PUBLIC DeeNoMemoryErrorObject DeeError_NoMemory_instance = {
	OBJECT_HEAD_INIT(&DeeError_NoMemory),
	/* .e_message    = */ (DREF DeeStringObject *)&str_nomemory,
	/* .e_inner      = */ NULL,
	/* .nm_allocsize = */ (size_t)-1
};








/* ==== Signal type subsystem ==== */
PUBLIC DeeTypeObject DeeError_StopIteration =
INIT_CUSTOM_ERROR("StopIteration", NULL, TP_FNORMAL | TP_FINHERITCTOR, &DeeError_Signal,
                  NULL, NULL, NULL, NULL, DeeSignalObject, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
PRIVATE struct type_member tpconst interrupt_class_members[] = {
	TYPE_MEMBER_CONST("KeyboardInterrupt", &DeeError_KeyboardInterrupt),
	TYPE_MEMBER_CONST("ThreadExit", &DeeError_ThreadExit),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_Interrupt =
INIT_CUSTOM_ERROR("Interrupt", NULL, TP_FNORMAL | TP_FINHERITCTOR | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Signal,
                  NULL, NULL, NULL, NULL, DeeSignalObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, interrupt_class_members);
PRIVATE int DCALL
threadexit_init(struct threadexit_object *__restrict self,
                size_t argc, DeeObject *const *argv) {
	self->te_result = Dee_None;
	if (DeeArg_Unpack(argc, argv, "|o", &self->te_result))
		return -1;
	Dee_Incref(self->te_result);
	return 0;
}

PRIVATE void DCALL
threadexit_fini(struct threadexit_object *__restrict self) {
	Dee_Decref(self->te_result);
}

PRIVATE void DCALL
threadexit_visit(struct threadexit_object *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->te_result);
}

PRIVATE struct type_member tpconst threadexit_members[] = {
	TYPE_MEMBER_FIELD("__result__", STRUCT_OBJECT, offsetof(struct threadexit_object, te_result)),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_ThreadExit =
INIT_CUSTOM_ERROR("ThreadExit", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Signal,
                  NULL, NULL, NULL, &threadexit_init, struct threadexit_object,
                  &threadexit_fini, NULL, &threadexit_visit,
                  NULL, NULL, threadexit_members, interrupt_class_members);
PUBLIC DeeTypeObject DeeError_KeyboardInterrupt =
INIT_CUSTOM_ERROR("KeyboardInterrupt", NULL, TP_FNORMAL | TP_FINHERITCTOR | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Interrupt,
                  NULL, NULL, NULL, NULL, DeeSignalObject, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL);
INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));
PRIVATE struct type_member tpconst signal_class_members[] = {
	TYPE_MEMBER_CONST("Interrupt", &DeeError_Interrupt),
	TYPE_MEMBER_CONST("StopIteration", &DeeError_StopIteration),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
signal_repr(DeeSignalObject *__restrict self) {
	return DeeString_Newf("%k()", Dee_TYPE(self));
}

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
				/* .tp_ctor      = */ (void *)&none_i1,
				/* .tp_copy_ctor = */ (void *)&none_i2,
				/* .tp_deep_ctor = */ (void *)&none_i2,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeSignalObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&signal_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ signal_class_members
};

PUBLIC DeeSignalObject DeeError_StopIteration_instance = {
	OBJECT_HEAD_INIT(&DeeError_StopIteration)
};

PUBLIC DeeSignalObject DeeError_Interrupt_instance = {
	OBJECT_HEAD_INIT(&DeeError_Interrupt)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */
