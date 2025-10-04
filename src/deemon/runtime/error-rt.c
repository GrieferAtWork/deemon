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
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/struct.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/variant.h>

#include <hybrid/int128.h>
#include <hybrid/minmax.h>
#include <hybrid/sched/yield.h>
#include <hybrid/typecore.h>
/**/

#include "strings.h"
#include "runtime_error.h"
/**/

#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

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
print define_Dee_HashStr("ob");
print define_Dee_HashStr("attr");
print define_Dee_HashStr("decl");
]]]*/
#define Dee_HashStr__message _Dee_HashSelectC(0x14820755, 0xbeaa4b97155366df)
#define Dee_HashStr__inner _Dee_HashSelectC(0x20e82985, 0x4f20d07bb803c1fe)
#define Dee_HashStr__ob _Dee_HashSelectC(0xdfa5fee2, 0x80a90888850ad043)
#define Dee_HashStr__attr _Dee_HashSelectC(0x55cfee3, 0xe4311a2c8443755d)
#define Dee_HashStr__decl _Dee_HashSelectC(0x95fe81e2, 0xdc35fdc1dce5cffc)
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
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









/************************************************************************/
/* Error.AttributeError                                                 */
/************************************************************************/
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
ClassDescriptor_IsInstanceAttr(DeeClassDescriptorObject *__restrict self,
                               struct Dee_class_attribute const *__restrict attr) {
	return attr >= self->cd_iattr_list &&
	       attr <= (self->cd_iattr_list + self->cd_iattr_mask);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
ClassDescriptor_IsClassAttr(DeeClassDescriptorObject *__restrict self,
                            struct Dee_class_attribute const *__restrict attr) {
	return attr >= self->cd_cattr_list &&
	       attr <= (self->cd_cattr_list + self->cd_cattr_mask);
}

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) bool DCALL
type_methods_contains(struct type_method const *chain,
                      struct type_method const *attr) {
	struct type_method const *iter;
	if (!chain)
		return false;
	if (attr < chain)
		return false;
	iter = chain;
	while (iter->m_name)
		++iter;
	return attr < iter;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) bool DCALL
type_getsets_contains(struct type_getset const *chain,
                      struct type_getset const *attr) {
	struct type_getset const *iter;
	if (!chain)
		return false;
	if (attr < chain)
		return false;
	iter = chain;
	while (iter->gs_name)
		++iter;
	return attr < iter;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) bool DCALL
type_members_contains(struct type_member const *chain,
                      struct type_member const *attr) {
	struct type_member const *iter;
	if (!chain)
		return false;
	if (attr < chain)
		return false;
	iter = chain;
	while (iter->m_name)
		++iter;
	return attr < iter;
}

struct type_member_buffer {
	char const                *mb_name;
	union Dee_type_member_desc mb_desc;
};

#define type_member_buffer_init(self, member)  \
	(void)((self)->mb_name = (member)->m_name, \
	       (self)->mb_desc = (member)->m_desc)
#define type_member_buffer_asmember(self) \
	COMPILER_CONTAINER_OF(&(self)->mb_name, struct type_member, m_name)

PRIVATE ATTR_PURE WUNUSED NONNULL((2)) struct type_member const *DCALL
type_members_findbuffer(struct type_member const *chain,
                        struct type_member_buffer const *attr) {
	if (!chain)
		return NULL;
	for (; chain->m_name; ++chain) {
		if (chain->m_name == attr->mb_name &&
		    memcmp(&chain->m_desc, &attr->mb_desc, sizeof(chain->m_desc)) == 0)
			return chain;
	}
	return NULL;
}

typedef struct {
	ERROR_OBJECT_HEAD
	DREF DeeObject     *ae_obj;   /* [0..1][const] Object whose attributes were accessed */
#define Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT    (Dee_ATTRINFO_COUNT + 0) /* Special type for "AttributeError_F_LAZYDECL": "v_any" is a "uint16_t" and an instance object address ("ai_decl" is unused; "ae_obj" is the relevant class-"DeeTypeObject") */
#define Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT (Dee_ATTRINFO_COUNT + 1) /* Special type for "AttributeError_F_LAZYDECL": "v_any" is a "uint16_t" and an instance object address ("ai_decl" is the relevant class-"DeeTypeObject"; "ae_obj" is the instance with the unbound member) */
	struct Dee_attrdesc ae_desc;  /* [valid_if(ae_obj != NULL)] Attribute descriptor. Fields in here are loaded when:
	                               * - ad_name:          [1..1][valid_if(ae_obj != NULL && !AttributeError_F_LAZYDECL)]
	                               * - ad_info.ai_decl:  [0..1][valid_if(!AttributeError_F_LAZYDECL)][DREF]
	                               *                     [if(ae_flags & AttributeError_F_INFOLOADED, [1..1])]
	                               *   When wanting to set "AttributeError_F_INFOLOADED", if
	                               *   this field is still "NULL" (i.e.: wasn't set during
	                               *   object construction), it is initialized then
	                               * - ad_info.ai_type:  [valid_if(ae_flags & AttributeError_F_INFOLOADED)]
	                               * - ad_info.ai_value: [valid_if(ae_flags & AttributeError_F_INFOLOADED)]
	                               * - ad_doc:           [valid_if(ae_flags & AttributeError_F_DESCLOADED)]
	                               * - ad_perm:          [valid_if(ae_flags & AttributeError_F_DESCLOADED)]
	                               * - ad_type:          [valid_if(ae_flags & AttributeError_F_DESCLOADED)]
	                               * NOTE: The "Dee_ATTRPERM_F_NAMEOBJ" flag in "ad_perm" is [const]
	                               *       and set during object construction.
	                               * Also: all fields are [lock(WRITE_ONCE)] (with initialization
	                               *       happening **BEFORE** the relevant `AttributeError_F_*'
	                               *       flag is set) */
#define AttributeError_F_GET DeeRT_ATTRIBUTE_ACCESS_GET /* Attempted to get attribute */
#define AttributeError_F_DEL DeeRT_ATTRIBUTE_ACCESS_DEL /* Attempted to del attribute */
#define AttributeError_F_SET DeeRT_ATTRIBUTE_ACCESS_SET /* Attempted to set attribute */
#define AttributeError_F_INFOLOADED 0x0100 /* "struct Dee_attrinfo"-portion was loaded (requires "AttributeError_F_DECLLOADED" to also be set) */
#define AttributeError_F_DESCLOADED 0x0200 /* "struct Dee_attrdesc"-portion was loaded (requires "AttributeError_F_INFOLOADED" to also be set) */
#define AttributeError_F_ISDEFAULT  0x0400 /* Set if "ae_desc" is the result of "DeeObject_FindAttrInfoStringLenHash" */
#define AttributeError_F_NODEFAULT  0x0800 /* Set if "ae_desc" isn't the result of "DeeObject_FindAttrInfoStringLenHash" */
#define AttributeError_F_LAZYDECL   0x1000 /* Set if "ad_info.ai_decl" and "ad_name" (except when "Dee_ATTRINFO_CUSTOM" is used) must be
                                            * determined lazily (based on "ae_obj", "ae_desc.ad_info.ai_type" and "ae_desc.ad_info.ai_value"). */
#ifndef CONFIG_NO_THREADS
#define AttributeError_F_LOADLOCK 0x8000 /* Lock to ensure only 1 thread does the loading */
#endif /* !CONFIG_NO_THREADS */
	unsigned int        ae_flags; /* [valid_if(ae_obj != NULL)] */
	union {
		struct type_member_buffer eas_type_member; /* Maybe used internally when "AttributeError_F_LAZYDECL" is set */
	} ea_storage;
} AttributeError;

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) struct class_attribute const *DCALL
ClassDescriptor_InstanceAttrAt(DeeClassDescriptorObject const *__restrict self, uint16_t addr) {
	size_t i;
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		struct class_attribute const *attr;
		attr = &self->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			continue;
		return attr;
	}
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) struct class_attribute const *DCALL
ClassDescriptor_ClassAttrAt(DeeClassDescriptorObject const *__restrict self, uint16_t addr) {
	size_t i;
	struct class_attribute const *attr;
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		attr = &self->cd_cattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr >= attr->ca_addr &&
		    addr < (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			return attr;
	}
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		attr = &self->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
			continue;
		if (addr >= attr->ca_addr &&
		    addr < (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			return attr;
	}
	return NULL;
}

PRIVATE NONNULL((1)) DeeObject *DCALL
AttributeError_GetDecl_impl(AttributeError *__restrict self) {
	DeeTypeObject *decl_type;
	DeeObject *ob = self->ae_obj;
	ASSERTF(ob, "No object, but 'AttributeError_F_LAZYDECL' flag is set?");
	switch (self->ae_desc.ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		DeeTypeMRO mro;
		struct type_attr const *custom = self->ae_desc.ad_info.ai_value.v_custom;
		ASSERTF(self->ae_desc.ad_name, "Dee_ATTRINFO_CUSTOM requires the caller to fill in the attribute name");
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (decl_type->tp_attr == custom)
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (decl_type->tp_attr == custom)
					goto return_decl_type;
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_MODSYM: {
		struct module_symbol const *sym = self->ae_desc.ad_info.ai_value.v_modsym;
		self->ae_desc.ad_name = sym->ss_name;
		self->ae_desc.ad_perm = 0;
		if (sym->ss_flags & Dee_MODSYM_FNAMEOBJ) {
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
		}
		if likely(DeeModule_Check(ob)) {
			DeeModuleObject *mod = (DeeModuleObject *)ob;
			if likely(sym >= (mod->mo_bucketv) &&
			          sym <= (mod->mo_bucketv + mod->mo_bucketm)) {
				return (DeeObject *)mod;
			}
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_ATTR:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		DeeTypeMRO mro;
		struct class_attribute const *attr;
		attr = self->ae_desc.ad_info.ai_value.v_attr;
		self->ae_desc.ad_name = DeeString_STR(attr->ca_name);
		Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
		self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (DeeType_IsClass(decl_type) &&
			    ClassDescriptor_IsInstanceAttr(DeeClass_DESC(decl_type)->cd_desc, attr))
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (DeeType_IsClass(decl_type) &&
				    ClassDescriptor_IsClassAttr(DeeClass_DESC(decl_type)->cd_desc, attr))
					goto return_decl_type;
				if (DeeType_IsClass(decl_type) &&
				    ClassDescriptor_IsInstanceAttr(DeeClass_DESC(decl_type)->cd_desc, attr)) {
					self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_ATTR;
					goto return_decl_type;
				}
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
return_decl_type:
		return (DeeObject *)decl_type;
	}	break;

	case Dee_ATTRINFO_METHOD:
	case Dee_ATTRINFO_INSTANCE_METHOD: {
		DeeTypeMRO mro;
		struct type_method const *method;
		method = self->ae_desc.ad_info.ai_value.v_method;
		self->ae_desc.ad_name = method->m_name;
		self->ae_desc.ad_perm = 0;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (type_methods_contains(decl_type->tp_methods, method))
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (type_methods_contains(decl_type->tp_class_methods, method))
					goto return_decl_type;
				if (type_methods_contains(decl_type->tp_methods, method)) {
					self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_METHOD;
					goto return_decl_type;
				}
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_GETSET:
	case Dee_ATTRINFO_INSTANCE_GETSET: {
		DeeTypeMRO mro;
		struct type_getset const *getset;
		getset = self->ae_desc.ad_info.ai_value.v_getset;
		self->ae_desc.ad_name = getset->gs_name;
		self->ae_desc.ad_perm = 0;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		do {
			if (type_getsets_contains(decl_type->tp_getsets, getset))
				goto return_decl_type;
		} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		if (DeeType_Check(ob)) {
			decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
			do {
				if (type_getsets_contains(decl_type->tp_class_getsets, getset))
					goto return_decl_type;
				if (type_getsets_contains(decl_type->tp_getsets, getset)) {
					self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_GETSET;
					goto return_decl_type;
				}
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
		}
		goto fail_no_clear_name;
	}	break;

	case Dee_ATTRINFO_MEMBER:
	case Dee_ATTRINFO_INSTANCE_MEMBER: {
		DeeTypeMRO mro;
		struct type_member const *member;
		member = self->ae_desc.ad_info.ai_value.v_member;
		self->ae_desc.ad_name = member->m_name;
		self->ae_desc.ad_perm = 0;
		decl_type = DeeTypeMRO_Init(&mro, Dee_TYPE(ob));
		if (member == type_member_buffer_asmember(&self->ea_storage.eas_type_member)) {
			do {
				member = type_members_findbuffer(decl_type->tp_members,
				                                 &self->ea_storage.eas_type_member);
				if (member)
					goto got_member_from_buffer;
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			if (DeeType_Check(ob)) {
				decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
				do {
					member = type_members_findbuffer(decl_type->tp_class_members,
					                                 &self->ea_storage.eas_type_member);
					if (member)
						goto got_member_from_buffer;
					member = type_members_findbuffer(decl_type->tp_members,
					                                 &self->ea_storage.eas_type_member);
					if (member) {
						self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_MEMBER;
						goto got_member_from_buffer;
					}
				} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			}
		} else {
			do {
				if (type_members_contains(decl_type->tp_members, member))
					goto return_decl_type;
			} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			if (DeeType_Check(ob)) {
				decl_type = DeeTypeMRO_Init(&mro, (DeeTypeObject *)ob);
				do {
					if (type_members_contains(decl_type->tp_class_members, member))
						goto return_decl_type;
					if (type_members_contains(decl_type->tp_members, member)) {
						self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_INSTANCE_MEMBER;
						goto return_decl_type;
					}
				} while ((decl_type = DeeTypeMRO_Next(&mro, decl_type)) != NULL);
			}
		}
		goto fail_no_clear_name;
got_member_from_buffer:
		self->ae_desc.ad_info.ai_value.v_member = member;
		goto return_decl_type;
	}	break;

	case Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT: {
		uint16_t addr = (uint16_t)(uintptr_t)self->ae_desc.ad_info.ai_value.v_any;
		DeeTypeObject *class_type = (DeeTypeObject *)self->ae_obj;
		struct class_attribute const *attr;
		ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(class_type));
		attr = ClassDescriptor_ClassAttrAt(DeeClass_DESC(class_type)->cd_desc, addr);
		if likely(attr) {
			self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR;
			self->ae_desc.ad_info.ai_value.v_attr = attr;
			self->ae_desc.ad_name = DeeString_STR(attr->ca_name);
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
			return (DeeObject *)class_type;
		}
		goto fail_clear_name;
	}	break;

	case Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT: {
		uint16_t addr = (uint16_t)(uintptr_t)self->ae_desc.ad_info.ai_value.v_any;
		DeeTypeObject *class_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		struct class_attribute const *attr;
		ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(class_type));
		attr = ClassDescriptor_InstanceAttrAt(DeeClass_DESC(class_type)->cd_desc, addr);
		if likely(attr) {
			self->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR;
			self->ae_desc.ad_info.ai_value.v_attr = attr;
			self->ae_desc.ad_name = DeeString_STR(attr->ca_name);
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
			return (DeeObject *)class_type;
		}
		goto fail_clear_name;
	}	break;

	default: break;
	}

	/* Shouldn't happen (unable to locate declaration) */
fail_clear_name:
	self->ae_desc.ad_name = NULL;
	self->ae_desc.ad_perm = 0;
fail_no_clear_name:
	return NULL;
}

PRIVATE NONNULL((1)) DeeObject *DCALL
AttributeError_GetDecl(AttributeError *__restrict self) {
	DeeObject *result;
	for (;;) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (!(flags & AttributeError_F_LAZYDECL))
			return atomic_read(&self->ae_desc.ad_info.ai_decl);
#ifndef CONFIG_NO_THREADS
		flags = atomic_fetchor(&self->ae_flags, AttributeError_F_LOADLOCK);
		if (flags & AttributeError_F_LOADLOCK) {
			SCHED_YIELD();
			continue;
		}
#endif /* !CONFIG_NO_THREADS */
		break;
	}
	ASSERTF(self->ae_obj, "No object, but 'AttributeError_F_LAZYDECL' flag is set?");
	result = AttributeError_GetDecl_impl(self);
	if unlikely(!result) {
		atomic_write(&self->ae_desc.ad_info.ai_decl, NULL);
		atomic_write(&self->ae_flags, 0);
		return NULL;
	}
	Dee_Incref(result);
	atomic_write(&self->ae_desc.ad_info.ai_decl, result);
#ifdef AttributeError_F_LOADLOCK
	atomic_and(&self->ae_flags, ~(AttributeError_F_LAZYDECL | AttributeError_F_LOADLOCK));
#else /* AttributeError_F_LOADLOCK */
	atomic_and(&self->ae_flags, ~(AttributeError_F_LAZYDECL));
#endif /* !AttributeError_F_LOADLOCK */
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
AttributeError_LoadInfo_impl(AttributeError *__restrict self,
                             struct Dee_attrinfo *__restrict info) {
	bool ok;
	char const *name;
	size_t namelen;
	Dee_hash_t namehash;
	DeeObject *decl;

	/* Load attribute name */
	name = self->ae_desc.ad_name;
	if unlikely(!name)
		return false;
	if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *name_ob;
		name_ob  = COMPILER_CONTAINER_OF(name, DeeStringObject, s_str);
		namelen  = DeeString_SIZE(name_ob);
		namehash = DeeString_Hash((DeeObject *)name_ob);
	} else {
		namelen  = strlen(name);
		namehash = Dee_HashPtr(name, namelen);
	}

	/* Load attribute info */
	ASSERTF(!(self->ae_flags & AttributeError_F_LAZYDECL),
	        "Must checked for- and cleared by the caller");
	decl = atomic_read(&self->ae_desc.ad_info.ai_decl);
	if (decl != NULL) {
		DeeObject *obj = self->ae_obj;
		DeeTypeObject *tp_obj = Dee_TYPE(obj);
		if (tp_obj == &DeeSuper_Type) {
			tp_obj = DeeSuper_TYPE(obj);
			obj    = DeeSuper_SELF(obj);
		}
		if (decl != self->ae_obj && DeeType_Check(decl) &&
		    DeeType_Implements(tp_obj, (DeeTypeObject *)decl)) {
			ok = DeeObject_TFindPrivateAttrInfoStringLenHash((DeeTypeObject *)decl,
			                                                 obj, name, namelen,
			                                                 namehash, info);
		} else {
			ok = DeeObject_FindAttrInfoStringLenHash(obj, name, namelen, namehash, info);
		}
		if (ok && info->ai_decl != decl)
			ok = false; /* Ensure that the correct declaring object is referenced */
	} else {
		ok = DeeObject_FindAttrInfoStringLenHash(self->ae_obj, name, namelen, namehash, info);
	}
	return ok;
}

/* Ensure that "AttributeError_F_INFOLOADED" is set
 * @return: true:  Success
 * @return: false: Attribute info cannot be loaded */
PRIVATE NONNULL((1)) bool DCALL
AttributeError_LoadInfo(AttributeError *__restrict self) {
	bool ok;
	struct Dee_attrinfo info;
	for (;;) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (flags & AttributeError_F_INFOLOADED)
			return true;
		if (flags & AttributeError_F_LAZYDECL) {
			AttributeError_GetDecl(self);
			continue;
		}
		if unlikely(!self->ae_obj)
			return false;
#ifndef CONFIG_NO_THREADS
		flags = atomic_fetchor(&self->ae_flags, AttributeError_F_LOADLOCK);
		if (flags & AttributeError_F_LOADLOCK) {
			SCHED_YIELD();
			continue;
		}
#endif /* !CONFIG_NO_THREADS */
		break;
	}

	ok = AttributeError_LoadInfo_impl(self, &info);

	/* If the specified attribute doesn't exist, then we can't access it */
	if likely(ok) {
		if (atomic_cmpxch(&self->ae_desc.ad_info.ai_decl, NULL, info.ai_decl))
			Dee_Incref(info.ai_decl);
		self->ae_desc.ad_info.ai_type  = info.ai_type;
		self->ae_desc.ad_info.ai_value = info.ai_value;
		COMPILER_WRITE_BARRIER();
		atomic_or(&self->ae_flags, (AttributeError_F_INFOLOADED |
		                            AttributeError_F_ISDEFAULT));
	}
#ifndef CONFIG_NO_THREADS
	atomic_and(&self->ae_flags, ~AttributeError_F_LOADLOCK);
#endif /* !CONFIG_NO_THREADS */
	return ok;
}

/* Ensure that "AttributeError_F_DESCLOADED" is set
 * @return: true:  Success
 * @return: false: Attribute info cannot be loaded */
PRIVATE NONNULL((1)) bool DCALL
AttributeError_LoadDesc(AttributeError *__restrict self) {
	for (;;) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (flags & AttributeError_F_DESCLOADED)
			return true;
		if (flags & AttributeError_F_LAZYDECL) {
			AttributeError_GetDecl(self);
			continue;
		}
		if (!(flags & AttributeError_F_INFOLOADED)) {
			if (!AttributeError_LoadInfo(self))
				return false;
			continue;
		}
#ifndef CONFIG_NO_THREADS
		flags = atomic_fetchor(&self->ae_flags, AttributeError_F_LOADLOCK);
		if (flags & AttributeError_F_LOADLOCK) {
			SCHED_YIELD();
			continue;
		}
#endif /* !CONFIG_NO_THREADS */
		break;
	}

	/* Initialize to defaults */
	self->ae_desc.ad_doc  = NULL;
	self->ae_desc.ad_type = NULL;
	self->ae_desc.ad_perm &= Dee_ATTRPERM_F_NAMEOBJ;

	/* Load attribute properties based on implementation */
	switch (self->ae_desc.ad_info.ai_type) {
	case Dee_ATTRINFO_MODSYM: {
		struct module_symbol const *sym;
		DeeModuleObject *mod;
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET;
		sym = self->ae_desc.ad_info.ai_value.v_modsym;
		if (!(sym->ss_flags & MODSYM_FREADONLY))
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET;
		if (sym->ss_flags & MODSYM_FPROPERTY) {
			self->ae_desc.ad_perm &= ~(Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
			if (!(sym->ss_flags & MODSYM_FCONSTEXPR))
				self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PROPERTY;
		}
		if (sym->ss_doc) {
			self->ae_desc.ad_doc = sym->ss_doc;
			if (sym->ss_flags & Dee_MODSYM_FDOCOBJ) {
				Dee_Incref(Dee_attrdesc_docobj(&self->ae_desc));
				self->ae_desc.ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
			}
		}
		mod = (DeeModuleObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(mod, &DeeModule_Type);
		if (mod->mo_flags & MODULE_FDIDINIT) {
			DeeModule_LockRead(mod);
			if (sym->ss_flags & MODSYM_FPROPERTY) {
				/* Check which property operations have been bound. */
				if (mod->mo_globalv[sym->ss_index + MODULE_PROPERTY_GET])
					self->ae_desc.ad_perm |= ATTR_ACCESS_GET;
				if (!(sym->ss_flags & MODSYM_FREADONLY)) {
					/* These callbacks are only allocated if the READONLY flag isn't set. */
					if (mod->mo_globalv[sym->ss_index + MODULE_PROPERTY_DEL])
						self->ae_desc.ad_perm |= ATTR_ACCESS_DEL;
					if (mod->mo_globalv[sym->ss_index + MODULE_PROPERTY_SET])
						self->ae_desc.ad_perm |= ATTR_ACCESS_SET;
				}
			}
			DeeModule_LockEndRead(mod);
		}
	}	break;

	case Dee_ATTRINFO_METHOD: {
		DeeTypeObject *decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		if (type_methods_contains(decl_type->tp_class_methods, self->ae_desc.ad_info.ai_value.v_method)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (type_methods_contains(decl_type->tp_methods, self->ae_desc.ad_info.ai_value.v_method)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
	}	/* ... */
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_METHOD:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER |
			                         Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_CANGET |
			                         Dee_ATTRPERM_F_CANCALL;
		}
		self->ae_desc.ad_doc = self->ae_desc.ad_info.ai_value.v_method->m_doc;
		break;

	case Dee_ATTRINFO_GETSET: {
		DeeTypeObject *decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		if (type_getsets_contains(decl_type->tp_class_getsets, self->ae_desc.ad_info.ai_value.v_getset)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (type_getsets_contains(decl_type->tp_getsets, self->ae_desc.ad_info.ai_value.v_getset)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
	}	/* ... */
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PROPERTY;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_GETSET:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER |
			                         Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_PROPERTY;
		}
		if (self->ae_desc.ad_info.ai_value.v_getset->gs_get)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET;
		if (self->ae_desc.ad_info.ai_value.v_getset->gs_del)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANDEL;
		if (self->ae_desc.ad_info.ai_value.v_getset->gs_set)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANSET;
		self->ae_desc.ad_doc = self->ae_desc.ad_info.ai_value.v_getset->gs_doc;
		break;

	case Dee_ATTRINFO_MEMBER: {
		DeeTypeObject *decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		if (type_members_contains(decl_type->tp_class_members, self->ae_desc.ad_info.ai_value.v_member)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (type_members_contains(decl_type->tp_members, self->ae_desc.ad_info.ai_value.v_member)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
	}	/* ... */
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_MEMBER:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_WRAPPER;
		}
		self->ae_desc.ad_doc = self->ae_desc.ad_info.ai_value.v_member->m_doc;
		if (TYPE_MEMBER_ISCONST(self->ae_desc.ad_info.ai_value.v_member)) {
			/* Constant -- cannot del or set */
		} else if (self->ae_desc.ad_info.ai_value.v_member->m_desc.md_field.mdf_type & STRUCT_CONST) {
			/* Read-only field -- cannot del or set */
		} else {
			self->ae_desc.ad_perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		}
		break;

	case Dee_ATTRINFO_ATTR: {
		struct class_attribute const *attr;
		struct instance_desc *inst;
		DeeTypeObject *decl_type;
		decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(decl_type));
		if (ClassDescriptor_IsClassAttr(DeeClass_DESC(decl_type)->cd_desc, self->ae_desc.ad_info.ai_value.v_attr)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER;
		} else if (ClassDescriptor_IsInstanceAttr(DeeClass_DESC(decl_type)->cd_desc, self->ae_desc.ad_info.ai_value.v_attr)) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_IMEMBER;
		}
		self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET;
		__IF0 {
	case Dee_ATTRINFO_INSTANCE_ATTR:
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_IMEMBER |
			                         Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_CANGET |
			                         Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET;
		}
		attr = self->ae_desc.ad_info.ai_value.v_attr;
		if (attr->ca_doc) {
			self->ae_desc.ad_doc = DeeString_STR(attr->ca_doc);
			Dee_Incref(Dee_attrdesc_docobj(&self->ae_desc));
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PRIVATE;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_PROPERTY;
		} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			self->ae_desc.ad_perm |= Dee_ATTRPERM_F_CANCALL;
		}
		inst = NULL;
		decl_type = (DeeTypeObject *)self->ae_desc.ad_info.ai_decl;
		ASSERT_OBJECT_TYPE(decl_type, &DeeType_Type);
		ASSERT(DeeType_IsClass(decl_type));
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			inst = class_desc_as_instance(DeeClass_DESC(decl_type));
		} else if (DeeObject_InstanceOf(self->ae_obj, decl_type)) {
			inst = DeeInstance_DESC(DeeClass_DESC(decl_type), self->ae_obj);
		}
		if (inst != NULL) {
			Dee_instance_desc_lock_read(inst);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
					self->ae_desc.ad_perm &= ~Dee_ATTRPERM_F_CANGET;
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
					if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
						self->ae_desc.ad_perm &= ~Dee_ATTRPERM_F_CANDEL;
					if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
						self->ae_desc.ad_perm &= ~Dee_ATTRPERM_F_CANSET;
				}
			}
			Dee_instance_desc_lock_endread(inst);
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			self->ae_desc.ad_perm &= ~(Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	}	break;

	default: break;
	}

	atomic_or(&self->ae_flags, AttributeError_F_DESCLOADED);
#ifndef CONFIG_NO_THREADS
	atomic_and(&self->ae_flags, ~AttributeError_F_LOADLOCK);
#endif /* !CONFIG_NO_THREADS */
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
AttributeError_LoadIsDefault(AttributeError *__restrict self) {
	bool ok;
	struct Dee_attrinfo info;
	unsigned int flags = atomic_read(&self->ae_flags);
	if (flags & AttributeError_F_ISDEFAULT)
		return true;
	if (flags & AttributeError_F_NODEFAULT)
		return false;
	if (!(flags & AttributeError_F_INFOLOADED))
		return AttributeError_LoadDesc(self);
	if (!AttributeError_LoadDesc(self))
		return false;
	ok = AttributeError_LoadInfo_impl(self, &info);
	if (ok) {
		ok = (info.ai_decl == AttributeError_GetDecl(self) &&
		      info.ai_type == self->ae_desc.ad_info.ai_type &&
		      info.ai_value.v_any == self->ae_desc.ad_info.ai_value.v_any);
	}
	atomic_or(&self->ae_flags, ok ? AttributeError_F_ISDEFAULT
	                              : AttributeError_F_NODEFAULT);
	return ok;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_init_kw(AttributeError *__restrict self, size_t argc,
                       DeeObject *const *argv, DeeObject *kw) {
	DeeKwArgs kwds;
	DeeObject *attr;
	if unlikely(DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
#define LOADARG(dst, i, name)               \
	do {                                    \
		if ((i) < argc)                     \
			*(DeeObject **)(dst) = argv[i]; \
		else {                              \
			*(DeeObject **)(dst) = DeeKwArgs_TryGetItemNRStringHash(&kwds, #name, Dee_HashStr__##name); \
			if (!ITER_ISOK(*(dst))) {       \
				if unlikely(*(dst) == NULL) \
					goto err;               \
				*(dst) = NULL;              \
			}                               \
		}                                   \
	}	__WHILE0
#define AttributeError_init_params Error_init_params ",ob?,attr?:?X2?Dstring?DAttribute,decl?:?X3?DType?DModule?O"
	LOADARG(&self->e_message, 0, message);
	LOADARG(&self->e_inner, 1, inner);
	LOADARG(&self->ae_obj, 2, ob);
	LOADARG(&attr, 3, attr);
	LOADARG(&self->ae_desc.ad_info.ai_decl, 4, decl);
#undef LOADARG
	if (argc > 5)
		return DeeArg_BadArgcEx(Dee_TYPE(self)->tp_name, argc, 0, 5);
	if (attr) {
		if unlikely(!self->ae_obj)
			return DeeError_Throwf(&DeeError_TypeError, "%s: 'attr' given, but no 'ob'", Dee_TYPE(self)->tp_name);
		self->ae_flags = 0;
		if (DeeObject_InstanceOf(attr, &DeeAttribute_Type)) {
			DeeAttributeObject *attrib = (DeeAttributeObject *)attr;
			if unlikely(self->ae_desc.ad_info.ai_decl)
				return DeeError_Throwf(&DeeError_TypeError, "%s: 'decl' given, but 'attr' is an Attribute and not a string", Dee_TYPE(self)->tp_name);
			Dee_attrdesc_init_copy(&self->ae_desc, &attrib->a_desc);
			ASSERT(self->ae_desc.ad_info.ai_decl == attrib->a_desc.ad_info.ai_decl);
			ASSERT(self->ae_desc.ad_info.ai_decl);
			Dee_Incref(self->ae_desc.ad_info.ai_decl);
			self->ae_flags |= (AttributeError_F_INFOLOADED | AttributeError_F_DESCLOADED);
		} else if (DeeString_Check(attr)) {
			Dee_Incref(attr);
			self->ae_desc.ad_name = DeeString_STR(attr);
			self->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
		} else {
			DeeObject_TypeAssertFailed2(attr, &DeeAttribute_Type, &DeeString_Type);
			goto err;
		}
		Dee_XIncref(self->ae_desc.ad_info.ai_decl);
		Dee_Incref(self->ae_obj);
	} else {
		if unlikely(self->ae_desc.ad_info.ai_decl)
			return DeeError_Throwf(&DeeError_TypeError, "%s: 'decl' given, but no 'attr'", Dee_TYPE(self)->tp_name);
		if unlikely(self->ae_obj)
			return DeeError_Throwf(&DeeError_TypeError, "%s: 'ob' given, but no 'attr'", Dee_TYPE(self)->tp_name);
	}
	Dee_XIncref(self->e_message);
	Dee_XIncref(self->e_inner);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_copy(AttributeError *__restrict self,
                    AttributeError *__restrict other) {
	self->e_message = other->e_message;
	Dee_XIncref(self->e_message);
	self->e_inner = other->e_inner;
	Dee_XIncref(self->e_inner);
	(void)AttributeError_LoadDesc(other); /* Make sure that "other" is stable. */
	(void)AttributeError_LoadDesc(other); /* ... */
	self->ae_obj = other->ae_obj;
	self->ae_flags = other->ae_flags;
	if (self->ae_obj) {
		self->ae_desc = other->ae_desc;
		Dee_XIncref(self->ae_desc.ad_info.ai_decl);
		if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
			Dee_Incref(Dee_attrdesc_nameobj(&self->ae_desc));
		if (self->ae_flags & AttributeError_F_DESCLOADED) {
			if ((self->ae_desc.ad_perm & Dee_ATTRPERM_F_DOCOBJ) && self->ae_desc.ad_doc)
				Dee_Incref(Dee_attrdesc_docobj(&self->ae_desc));
			Dee_XIncref(self->ae_desc.ad_type);
		}
		Dee_Incref(self->ae_obj);
	}
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
AttributeError_fini(AttributeError *__restrict self) {
	if (self->ae_obj) {
		if (!(self->ae_flags & AttributeError_F_LAZYDECL)) {
			Dee_XDecref(self->ae_desc.ad_info.ai_decl);
			if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				Dee_Decref(Dee_attrdesc_nameobj(&self->ae_desc));
			if (self->ae_flags & AttributeError_F_DESCLOADED) {
				if ((self->ae_desc.ad_perm & Dee_ATTRPERM_F_DOCOBJ) && self->ae_desc.ad_doc)
					Dee_Decref(Dee_attrdesc_docobj(&self->ae_desc));
				Dee_XDecref(self->ae_desc.ad_type);
			}
		}
		Dee_Decref(self->ae_obj);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_deep(AttributeError *__restrict self,
                    AttributeError *__restrict other) {
	if unlikely(AttributeError_copy(self, other))
		goto err;
	if unlikely(DeeObject_XInplaceDeepCopy((DeeObject **)&self->e_inner))
		goto err_self;
	if unlikely(DeeObject_XInplaceDeepCopy((DeeObject **)&self->ae_obj))
		goto err_self;
	return 0;
err_self:
	AttributeError_fini(self);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
AttributeError_visit(AttributeError *__restrict self, Dee_visit_t proc, void *arg) {
	if (self->ae_obj) {
		unsigned int flags = atomic_read(&self->ae_flags);
		if (!(flags & AttributeError_F_LAZYDECL)) {
			DeeObject *decl = atomic_read(&self->ae_desc.ad_info.ai_decl);
			Dee_Visit(decl);
		}
		if (self->ae_flags & AttributeError_F_DESCLOADED)
			Dee_XVisit(self->ae_desc.ad_type);
		Dee_Visit(self->ae_obj);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_print_attr_str(AttributeError *__restrict self,
                              Dee_formatprinter_t printer, void *arg) {
	DeeObject *decl = AttributeError_GetDecl(self);
	if (decl == NULL)
		decl = self->ae_obj;
	if (decl == NULL)
		return DeeFormat_PRINT(printer, arg, "?.?");
	ASSERT(self->ae_desc.ad_name);
	if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		return DeeFormat_Printf(printer, arg, "%k.%k", decl,
		                        Dee_attrdesc_nameobj(&self->ae_desc));
	}
	return DeeFormat_Printf(printer, arg, "%k.%s",
	                        decl, self->ae_desc.ad_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_print(AttributeError *__restrict self,
                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (self->e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	result = DeeFormat_Printf(printer, arg, "<%s ", Dee_TYPE(self)->tp_name);
	if unlikely(result < 0)
		goto done;
	DO(err_temp, AttributeError_print_attr_str(self, printer, arg));
	DO(err_temp, DeeFormat_PRINT(printer, arg, ">"));
done:
	return result;
err_temp:
	return temp;
}

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
attr_printrepr_impl(struct Dee_attrdesc const *__restrict self,
                    Dee_formatprinter_t printer, void *arg);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
AttributeError_printrepr(AttributeError *__restrict self,
                         Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char const *prefix = "";
	result = DeeFormat_Printf(printer, arg, "%s(", Dee_TYPE(self)->tp_name);
	if unlikely(result < 0)
		goto done;
	if (self->e_message) {
		DO(err_temp, DeeFormat_Printf(printer, arg, "message: %r", self->e_message));
		prefix = ", ";
	}
	if (self->e_inner) {
		DO(err_temp, DeeFormat_Printf(printer, arg, "%sinner: %r", prefix, self->e_inner));
		prefix = ", ";
	}
	if (self->ae_obj) {
		DeeObject *decl = AttributeError_GetDecl(self);
		if (self->ae_desc.ad_name) {
			DO(err_temp, DeeFormat_Printf(printer, arg, "%sob: %r, attr: ", prefix, self->ae_obj));
			if (AttributeError_LoadDesc(self) && !AttributeError_LoadIsDefault(self)) {
				DO(err_temp, attr_printrepr_impl(&self->ae_desc, printer, arg));
			} else {
				DO(err_temp, (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				   ? DeeString_PrintRepr((DeeObject *)Dee_attrdesc_nameobj(&self->ae_desc), printer, arg)
				   : DeeFormat_Printf(printer, arg, "%q", self->ae_desc.ad_name));
				if (decl != NULL)
					DO(err_temp, DeeFormat_Printf(printer, arg, ", decl: %r", decl));
			}
		} else {
			DO(err_temp, DeeFormat_Printf(printer, arg, "%sob: %r", prefix, self->ae_obj));
		}
	}
	DO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return -1;
}

PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
dee_memcmp2(void const *lhs, size_t lhs_size,
            void const *rhs, size_t rhs_size) {
	size_t common = MIN(lhs_size, rhs_size);
	int result = memcmp(lhs, rhs, common * sizeof(char));
	if (result < -1) {
		result = -1;
	} else if (result > 1) {
		result = 1;
	} else if (result == 0) {
		if (lhs_size < rhs_size) {
			result = -1;
		} else if (lhs_size > rhs_size) {
			result = 1;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
AttributeError_compare_impl(AttributeError *lhs, AttributeError *rhs,
                            int (DCALL *cmp)(DeeObject *, DeeObject *)) {
	DeeObject *lhs_decl, *rhs_decl;
	int result;
	if (lhs->e_message != rhs->e_message) {
		if (!lhs->e_message)
			return -1;
		if (!rhs->e_message)
			return 1;
		result = (*cmp)((DeeObject *)lhs->e_message,
		                (DeeObject *)rhs->e_message);
		if (result != 0)
			return result;
	}
	if (lhs->e_inner != rhs->e_inner) {
		if (!lhs->e_inner)
			return -1;
		if (!rhs->e_inner)
			return 1;
		result = (*cmp)(lhs->e_inner, rhs->e_inner);
		if (result != 0)
			return result;
	}
	if (lhs->ae_obj != rhs->ae_obj) {
		if (!lhs->ae_obj)
			return -1;
		if (!rhs->ae_obj)
			return 1;
#if 1 /* If instances differ, then then the errors aren't the same */
		return Dee_CompareNe(DeeObject_Id(lhs->ae_obj),
		                     DeeObject_Id(rhs->ae_obj));
#else
		result = (*cmp)(lhs->ae_obj, rhs->ae_obj);
		if (result != 0)
			return result;
#endif
	}

	AttributeError_LoadInfo(lhs);
	AttributeError_LoadInfo(rhs);
	lhs_decl = AttributeError_GetDecl(lhs);
	rhs_decl = AttributeError_GetDecl(rhs);
	/* Only touch names after decls were loaded. */
	if (lhs->ae_desc.ad_name != rhs->ae_desc.ad_name) {
		if (!lhs->ae_desc.ad_name)
			return -1;
		if (!rhs->ae_desc.ad_name)
			return 1;
		if ((lhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) &&
		    (rhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ)) {
			DeeStringObject *lhs_name = Dee_attrdesc_nameobj(&lhs->ae_desc);
			DeeStringObject *rhs_name = Dee_attrdesc_nameobj(&rhs->ae_desc);
			result = (*cmp)((DeeObject *)lhs_name, (DeeObject *)rhs_name);
		} else {
			char const *lhs_str = lhs->ae_desc.ad_name;
			char const *rhs_str = rhs->ae_desc.ad_name;
			size_t lhs_len = (lhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) ? WSTR_LENGTH(lhs_str) : strlen(lhs_str);
			size_t rhs_len = (rhs->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) ? WSTR_LENGTH(rhs_str) : strlen(rhs_str);
			result = dee_memcmp2(lhs_str, lhs_len, rhs_str, rhs_len);
		}
		if (result != 0)
			return result;
	}
	if (lhs_decl != rhs_decl) {
		if (!lhs_decl)
			return -1;
		if (!rhs_decl)
			return 1;
#if 1 /* If instances differ, then then the errors aren't the same */
		return Dee_CompareNe(DeeObject_Id(lhs_decl),
		                     DeeObject_Id(rhs_decl));
#else
		if ((lhs_decl == lhs->ae_obj) &&
		    (rhs_decl == rhs->ae_obj)) {
			/* Already compared (and was equal) */
		} else {
			result = (*cmp)(lhs_decl,
			                rhs_decl);
			if (result != 0)
				return result;
		}
#endif
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
AttributeError_hash(AttributeError *__restrict self) {
	Dee_hash_t result = DeeObject_HashGeneric(Dee_TYPE(self));
	if (self->e_message)
		result = Dee_HashCombine(result, DeeObject_Hash((DeeObject *)self->e_message));
	if (self->e_inner)
		result = Dee_HashCombine(result, DeeObject_Hash(self->e_inner));
	if (self->ae_obj) {
		DeeObject *decl;
		result = Dee_HashCombine(result, DeeObject_Hash(self->ae_obj));
		AttributeError_LoadInfo(self);
		decl = AttributeError_GetDecl(self);
		/* Only touch names after decls were loaded. */
		if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
			result = Dee_HashCombine(result, DeeString_Hash((DeeObject *)Dee_attrdesc_nameobj(&self->ae_desc)));
		} else {
			result = Dee_HashCombine(result, Dee_HashStr(self->ae_desc.ad_name));
		}
		if (decl && decl != self->ae_obj)
			result = Dee_HashCombine(result, DeeObject_Hash(decl));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_compare_eq(AttributeError *self, AttributeError *some_object) {
	if (DeeObject_AssertTypeExact(some_object, Dee_TYPE(self)))
		goto err;
	return AttributeError_compare_impl(self, some_object, &DeeObject_TryCompareEq);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_compare(AttributeError *self, AttributeError *some_object) {
	if (DeeObject_AssertTypeExact(some_object, Dee_TYPE(self)))
		goto err;
	return AttributeError_compare_impl(self, some_object, &DeeObject_Compare);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
AttributeError_trycompare_eq(AttributeError *self, AttributeError *some_object) {
	if (!DeeObject_InstanceOfExact(some_object, Dee_TYPE(self)))
		return 1;
	return AttributeError_compare_impl(self, some_object, &DeeObject_TryCompareEq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeAttributeObject *DCALL
AttributeError_get_attr(AttributeError *__restrict self) {
	DREF DeeAttributeObject *result;
	if unlikely(!AttributeError_LoadDesc(self))
		goto err_unbound;
	result = DeeObject_MALLOC(DeeAttributeObject);
	if unlikely(!result)
		goto err;
	Dee_attrdesc_init_copy(&result->a_desc, &self->ae_desc);
	Dee_Incref(result->a_desc.ad_info.ai_decl);
	DeeObject_Init(result, &DeeAttribute_Type);
	return result;
err_unbound:
	return (DREF DeeAttributeObject *)DeeRT_ErrTUnboundAttrCStr(&DeeError_AttributeError, self, "attr");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_bound_attr(AttributeError *__restrict self) {
	bool has_desc = AttributeError_LoadDesc(self);
	return Dee_BOUND_FROMBOOL(has_desc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
AttributeError_get_name(AttributeError *__restrict self) {
	AttributeError_GetDecl(self);
	if unlikely(!self->ae_desc.ad_name)
		goto err_unbound;
	if (self->ae_desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		DeeStringObject *result = Dee_attrdesc_nameobj(&self->ae_desc);
		Dee_Incref(result);
		return (DREF DeeObject *)result;
	}
	return DeeString_New(self->ae_desc.ad_name);
err_unbound:
	return DeeRT_ErrTUnboundAttrCStr(&DeeError_AttributeError, self, "name");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_bound_name(AttributeError *__restrict self) {
	AttributeError_GetDecl(self);
	return Dee_BOUND_FROMBOOL(self->ae_desc.ad_name != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
AttributeError_get_decl(AttributeError *__restrict self) {
	DeeObject *result = AttributeError_GetDecl(self);
	if unlikely(!result)
		goto err_unbound;
	Dee_Incref(result);
	return result;
err_unbound:
	return DeeRT_ErrTUnboundAttrCStr(&DeeError_AttributeError, self, "decl");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
AttributeError_bound_decl(AttributeError *__restrict self) {
	DeeObject *decl = AttributeError_GetDecl(self);
	return Dee_BOUND_FROMBOOL(decl != NULL);
}

PRIVATE struct Dee_type_cmp AttributeError_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&AttributeError_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&AttributeError_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *self, DeeObject *))&AttributeError_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&AttributeError_trycompare_eq,
};

PRIVATE struct type_getset tpconst AttributeError_getsets[] = {
	TYPE_GETTER_BOUND_F("attr", &AttributeError_get_attr, &AttributeError_bound_attr,
	                    METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                    "->?DAttribute\n"
	                    "The attribute that caused the error (or unbound if "
	                    /**/ "the attribute is unknown, or the point of the error is "
	                    /**/ "to complain about an unknown attribute)"),
	TYPE_GETTER_BOUND_F("name", &AttributeError_get_name, &AttributeError_bound_name,
	                    METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                    "->?Dstring\n"
	                    "Name of the attribute that was being accessed"),
	TYPE_GETTER_BOUND_F("decl", &AttributeError_get_decl, &AttributeError_bound_decl,
	                    METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                    "->?X3?DType?DModule?O\n"
	                    "The object (or ?DType or ?DModule) that is declaring the attribute"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst AttributeError_members[] = {
	TYPE_MEMBER_FIELD_DOC("ob", STRUCT_OBJECT, offsetof(AttributeError, ae_obj),
	                      "The object whose attributes were accessed"),
	TYPE_MEMBER_BITFIELD_DOC("isget", STRUCT_CONST, AttributeError, ae_flags, AttributeError_F_GET,
	                         "True if the error happened during an attribute read"),
	TYPE_MEMBER_BITFIELD_DOC("isdel", STRUCT_CONST, AttributeError, ae_flags, AttributeError_F_DEL,
	                         "True if the error happened during an attribute delete"),
	TYPE_MEMBER_BITFIELD_DOC("isset", STRUCT_CONST, AttributeError, ae_flags, AttributeError_F_SET,
	                         "True if the error happened during an attribute write"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst AttributeError_class_members[] = {
	TYPE_MEMBER_CONST("UnboundAttribute", &DeeError_UnboundAttribute),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_AttributeError = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "AttributeError",
	/* .tp_doc      = */ "(" AttributeError_init_params ")",
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_Error,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&AttributeError_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&AttributeError_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(AttributeError),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)&AttributeError_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&AttributeError_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&AttributeError_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&AttributeError_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&AttributeError_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &AttributeError_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ AttributeError_getsets,
	/* .tp_members       = */ AttributeError_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ AttributeError_class_members
};


#define INIT_LIKE_ATTRIBUTE_ERROR(tp_name, tp_doc, tp_flags,                \
                                  tp_base, tp_str, tp_print,                \
                                  tp_methods, tp_getsets, tp_class_members) \
	{                                                                       \
		OBJECT_HEAD_INIT(&DeeType_Type),                                    \
		/* .tp_name     = */ tp_name,                                       \
		/* .tp_doc      = */ DOC(tp_doc),                                   \
		/* .tp_flags    = */ TP_FNORMAL | (tp_flags),                       \
		/* .tp_weakrefs = */ 0,                                             \
		/* .tp_features = */ TF_NONE,                                       \
		/* .tp_base     = */ tp_base,                                       \
		/* .tp_init = */ {                                                  \
			{                                                               \
				/* .tp_alloc = */ {                                         \
					/* .tp_ctor      = */ (Dee_funptr_t)NULL,               \
					/* .tp_copy_ctor = */ (Dee_funptr_t)&AttributeError_copy, \
					/* .tp_deep_ctor = */ (Dee_funptr_t)&AttributeError_deep, \
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,               \
					TYPE_FIXED_ALLOCATOR(AttributeError),                   \
					/* .tp_any_ctor_kw = */ (Dee_funptr_t)&AttributeError_init_kw, \
				}                                                           \
			},                                                              \
			/* .tp_dtor        = */ NULL,                                   \
			/* .tp_assign      = */ NULL,                                   \
			/* .tp_move_assign = */ NULL                                    \
		},                                                                  \
		/* .tp_cast = */ {                                                  \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ NULL,                                     \
			/* .tp_bool      = */ NULL,                                     \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&AttributeError_printrepr, \
		},                                                                  \
		/* .tp_visit         = */ NULL,                                     \
		/* .tp_gc            = */ NULL,                                     \
		/* .tp_math          = */ NULL,                                     \
		/* .tp_cmp           = */ &AttributeError_cmp,                      \
		/* .tp_seq           = */ NULL,                                     \
		/* .tp_iter_next     = */ NULL,                                     \
		/* .tp_iterator      = */ NULL,                                     \
		/* .tp_attr          = */ NULL,                                     \
		/* .tp_with          = */ NULL,                                     \
		/* .tp_buffer        = */ NULL,                                     \
		/* .tp_methods       = */ tp_methods,                               \
		/* .tp_getsets       = */ tp_getsets,                               \
		/* .tp_members       = */ NULL,                                     \
		/* .tp_class_methods = */ NULL,                                     \
		/* .tp_class_getsets = */ NULL,                                     \
		/* .tp_class_members = */ tp_class_members                          \
	}

PRIVATE ATTR_COLD NONNULL((1, 3, 4)) DeeObject *
(DCALL DeeRT_ErrAttributeError_impl)(DeeTypeObject *error_type, DeeObject *decl,
                                     DeeObject *ob, DeeObject *attr,
                                     unsigned int flags) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERTF((flags & ~(AttributeError_F_GET |
	                   AttributeError_F_DEL |
	                   AttributeError_F_SET)) == 0,
	        "Only these flags may be specified");
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_name = DeeString_STR(attr);
	result->ae_desc.ad_perm = Dee_ATTRPERM_F_NAMEOBJ;
	Dee_XIncref(decl);
	result->ae_desc.ad_info.ai_decl = decl;
	result->ae_flags = flags;
	DeeObject_Init(result, error_type);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((1, 3, 4)) DeeObject *
(DCALL DeeRT_ErrAttributeErrorCStr_impl)(DeeTypeObject *error_type, DeeObject *decl,
                                         DeeObject *ob, /*static*/ char const *attr,
                                         unsigned int flags) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERTF((flags & ~(AttributeError_F_GET |
	                   AttributeError_F_DEL |
	                   AttributeError_F_SET)) == 0,
	        "Only these flags may be specified");
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_name = attr;
	result->ae_desc.ad_perm = 0;
	Dee_XIncref(decl);
	result->ae_desc.ad_info.ai_decl = decl;
	result->ae_flags = flags;
	DeeObject_Init(result, error_type);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}



/************************************************************************/
/* Error.AttributeError.UnboundAttribute                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
UnboundAttribute_print(AttributeError *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	if (self->e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	result = DeeFormat_PRINT(printer, arg, "Unbound attribute ");
	if likely(result >= 0) {
		temp = AttributeError_print_attr_str(self, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PUBLIC DeeTypeObject DeeError_UnboundAttribute =
INIT_LIKE_ATTRIBUTE_ERROR("UnboundAttribute", "(" AttributeError_init_params ")",
                          TP_FNORMAL, &DeeError_AttributeError, NULL, &UnboundAttribute_print,
                          NULL, NULL, NULL);

/* Throws an `DeeError_UnboundAttribute' indicating that some attribute isn't bound
 * @return: NULL: Always returns "NULL" (for easy chaining when called form getters) */
PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundAttr)(DeeObject *ob, /*string*/ DeeObject *attr) {
	return DeeRT_ErrAttributeError_impl(&DeeError_UnboundAttribute, NULL, ob, attr, AttributeError_F_GET);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundAttrCStr)(DeeObject *ob, /*static*/ char const *attr) {
	return DeeRT_ErrAttributeErrorCStr_impl(&DeeError_UnboundAttribute, NULL, ob, attr, AttributeError_F_GET);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundMember)(DeeObject *ob, struct Dee_type_member const *attr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_MEMBER; /* Changed to "Dee_ATTRINFO_INSTANCE_MEMBER" if appropriate */
	result->ae_desc.ad_info.ai_value.v_member = (struct type_member const *)&result->ea_storage.eas_type_member;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	/* Special case: We might get here from a context where "attr" is a stack-allocated copy
	 *               of an incomplete (as in: missing the doc string) member descriptor that
	 *               was previously copied out of `struct Dee_membercache_slot'.
	 * To deal with this case, we have to create yet another copy that will then be resolved
	 * lazily when the attribute error's declaration location is loaded. */
	type_member_buffer_init(&result->ea_storage.eas_type_member, attr);
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrUnboundAttrEx)(DeeObject *ob, struct Dee_attrdesc const *attr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	Dee_attrdesc_init_copy(&result->ae_desc, attr);
	Dee_Incref(result->ae_desc.ad_info.ai_decl);
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_DESCLOADED;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) DeeObject *
(DCALL DeeRT_ErrTUnboundAttr)(DeeObject *decl, DeeObject *ob, /*string*/ DeeObject *attr) {
	return DeeRT_ErrAttributeError_impl(&DeeError_UnboundAttribute, decl, ob,
	                                    attr, AttributeError_F_GET);
}

PUBLIC ATTR_COLD NONNULL((1, 2, 3)) DeeObject *
(DCALL DeeRT_ErrTUnboundAttrCStr)(DeeObject *decl, DeeObject *ob, /*static*/ char const *attr) {
	return DeeRT_ErrAttributeErrorCStr_impl(&DeeError_UnboundAttribute, decl, ob,
	                                        attr, AttributeError_F_GET);
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrCUnboundAttrCA)(DeeObject *ob, struct Dee_class_attribute const *attr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(ob);
	result->ae_obj = ob;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTR; /* Changed to "Dee_ATTRINFO_INSTANCE_ATTR" if appropriate */
	result->ae_desc.ad_info.ai_value.v_attr = attr;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1, 2)) DeeObject *
(DCALL DeeRT_ErrCUnboundInstanceMember)(DeeTypeObject *class_type,
                                        DeeObject *instance, uint16_t addr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERT_OBJECT_TYPE_A(instance, class_type);
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(instance);
	result->ae_obj = instance;
	result->ae_desc.ad_info.ai_decl = (DeeObject *)class_type;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTRIBUTEERROR_INSTANCE_SLOT;
	result->ae_desc.ad_info.ai_value.v_any = (void *)(uintptr_t)addr;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}

PUBLIC ATTR_COLD NONNULL((1)) DeeObject *
(DCALL DeeRT_ErrCUnboundClassMember)(DeeTypeObject *class_type, uint16_t addr) {
	DREF AttributeError *result = DeeObject_MALLOC(AttributeError);
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(*result));
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	result->e_message = NULL;
	result->e_inner   = NULL;
	Dee_Incref(class_type);
	result->ae_obj = (DeeObject *)class_type;
	result->ae_desc.ad_info.ai_type = Dee_ATTRINFO_ATTRIBUTEERROR_CLASS_SLOT;
	result->ae_desc.ad_info.ai_value.v_any = (void *)(uintptr_t)addr;
	result->ae_flags = AttributeError_F_GET | AttributeError_F_INFOLOADED | AttributeError_F_LAZYDECL;
	DeeObject_Init(result, &DeeError_UnboundAttribute);
	DeeError_ThrowInherited((DeeObject *)result);
err:
	return NULL;
}





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
PUBLIC DeeTypeObject DeeError_DivideByZero =
INIT_LIKE_BASECLASS("DivideByZero", "(" ArithmeticError_init_params ")",
                    TP_FNORMAL, &DeeError_ArithmeticError, ArithmeticError,
                    NULL, NULL, NULL, NULL, NULL);


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
	struct Dee_variant ie_length; /* [const] Length of the sequence */
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
	struct Dee_variant length;
	if (self->ie_base.ke_base.e_message)
		return error_print((DeeErrorObject *)self, printer, arg);
	if unlikely(IndexError_GetLength(self, &length))
		goto err;
	return DeeFormat_Printf(printer, arg,
	                        "Index `%Vr' lies outside the valid bounds `0...%VR' of sequence of type `%K'",
	                        &self->ie_base.ke_base, &length,
	                        SequenceError_GetSeqType(&self->ie_base.ke_base));
err:
	return -1;
}

PUBLIC DeeTypeObject DeeError_IndexError =
INIT_CUSTOM_ERROR("IndexError", "(" IndexError_init_params ")",
                  TP_FNORMAL, &DeeError_KeyError, IndexError, NULL, &IndexError_print,
                  NULL, NULL, IndexError_members, NULL);

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
	Dee_Decref(inner);
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
	Dee_Decref(inner);
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

PUBLIC ATTR_COLD NONNULL((1, 2, 5)) int
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
		Dee_variant_init_uint32(&result->io_maxval, ((uint32_t)1 << max_bits) - 1);
	} else if (num_bits <= 64) {
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN))
			Dee_variant_init_int64(&result->io_minval, -((int64_t)1 << (num_bits - 1)));
		Dee_variant_init_uint64(&result->io_maxval, ((uint64_t)1 << max_bits) - 1);
	} else if (num_bits <= 128) {
		Dee_uint128_t maxval;
		if (flags & (DeeRT_ErrIntegerOverflowEx_F_SIGNED | DeeRT_ErrIntegerOverflowEx_F_ANYSIGN)) {
			Dee_int128_t minval;
			__hybrid_int128_setone(minval);
			__hybrid_int128_shl(minval, num_bits - 1);
			__hybrid_int128_neg(minval);
			Dee_variant_init_int128(&result->io_minval, minval);
		}
		__hybrid_uint128_setone(maxval);
		__hybrid_uint128_shl(maxval, max_bits);
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



/* TODO: Custom handling for: DeeError_IndexError (include attributes for relevant sequence / index) */
/* TODO: Custom handling for: DeeError_UnboundItem */



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
