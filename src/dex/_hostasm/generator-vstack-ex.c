/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_EX_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_EX_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

DECL_BEGIN

#ifndef CONFIG_HAVE_strchrnul
#define CONFIG_HAVE_strchrnul
#undef strchrnul
#define strchrnul dee_strchrnul
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
dee_strchrnul(char const *haystack, int needle) {
	for (; *haystack; ++haystack) {
		if ((unsigned char)*haystack == (unsigned char)needle)
			break;
	}
	return (char *)haystack;
}
#endif /* !CONFIG_HAVE_strchrnul */

/************************************************************************/
/* HIGH-LEVEL VSTACK CONTROLS                                           */
/************************************************************************/

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err

/* NOTE: This config right here must match "src/deemon/runtime/attribute.c" */
#undef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
#undef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
#undef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
#define CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
//#define CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE /* Don't enable this again. - It's better if this is off. */
//#define CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC


struct attr_info {
#define ATTR_TYPE_CUSTOM          0 /* Custom attribute operators are present. */
#define ATTR_TYPE_ATTR            1 /* Wrapper for producing `DeeInstanceMethod_Type' or directly accessing a property/member */
#define ATTR_TYPE_METHOD          2 /* Wrapper for producing `DeeObjMethod_Type' / `DeeKwObjMethod_Type' */
#define ATTR_TYPE_GETSET          3 /* GetSet that uses the original "this"-argument. */
#define ATTR_TYPE_MEMBER          4 /* Member that uses the original "this"-argument. */
#define ATTR_TYPE_INSTANCE_ATTR   5 /* Wrapper for producing `DeeInstanceMember_Type' / `DeeInstanceMethod_Type' / `DeeProperty_Type' */
#define ATTR_TYPE_INSTANCE_METHOD 6 /* Wrapper for producing `DeeClsMethod_Type' / `DeeKwClsMethod_Type' */
#define ATTR_TYPE_INSTANCE_GETSET 7 /* Wrapper for producing `DeeClsProperty_Type' */
#define ATTR_TYPE_INSTANCE_MEMBER 8 /* Wrapper for producing `DeeClsMember_Type' */
#define ATTR_TYPE_MODSYM          9 /* Access a module symbol */
	uintptr_t  ai_type; /* Type of attribute (one of `ATTR_TYPE_*'). */
	DeeObject *ai_decl; /* [1..1] Declaring object (the type implementing the operators/attribute/instance-attribute, or the module for ATTR_TYPE_MODSYM) */
	union {
		struct Dee_type_attr const       *v_custom;          /* [1..1][ATTR_TYPE_CUSTOM] Custom attribute access operators. */
		struct Dee_class_attribute const *v_attr;            /* [1..1][ATTR_TYPE_ATTR] Attribute to access or produce a `DeeInstanceMethod_Type' for */
		struct Dee_type_method const     *v_method;          /* [1..1][ATTR_TYPE_METHOD] Method to create a `DeeObjMethod_Type' / `DeeKwObjMethod_Type' for */
		struct Dee_type_getset const     *v_getset;          /* [1..1][ATTR_TYPE_GETSET] Getset that should be accessed */
		struct Dee_type_member const     *v_member;          /* [1..1][ATTR_TYPE_MEMBER] Member that should be accessed */
		struct Dee_class_attribute const *v_instance_attr;   /* [1..1][ATTR_TYPE_INSTANCE_ATTR] Attribute to wrap as `DeeInstanceMember_Type' / `DeeInstanceMethod_Type' / `DeeProperty_Type' */
		struct Dee_type_method const     *v_instance_method; /* [1..1][ATTR_TYPE_INSTANCE_METHOD] Method to wrap as `DeeClsMethod_Type' / `DeeKwClsMethod_Type' */
		struct Dee_type_getset const     *v_instance_getset; /* [1..1][ATTR_TYPE_INSTANCE_GETSET] Getset to wrap as `DeeClsProperty_Type' */
		struct Dee_type_member const     *v_instance_member; /* [1..1][ATTR_TYPE_INSTANCE_MEMBER] Member to wrap as `DeeClsMember_Type' */
		struct Dee_module_symbol const   *v_modsym;          /* [1..1][ATTR_TYPE_MODSYM] Symbol that should be accessed */
	} ai_value;
};

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) struct Dee_type_method const *DCALL
Dee_type_method_find(struct Dee_type_method const *chain, DeeStringObject *attr) {
	char const *attr_str = DeeString_STR(attr);
	for (; chain->m_name; ++chain) {
		if (strcmp(chain->m_name, attr_str) == 0)
			return chain;
	}
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) struct Dee_type_getset const *DCALL
Dee_type_getset_find(struct Dee_type_getset const *chain, DeeStringObject *attr) {
	char const *attr_str = DeeString_STR(attr);
	for (; chain->gs_name; ++chain) {
		if (strcmp(chain->gs_name, attr_str) == 0)
			return chain;
	}
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) struct Dee_type_member const *DCALL
Dee_type_member_find(struct Dee_type_member const *chain, DeeStringObject *attr) {
	char const *attr_str = DeeString_STR(attr);
	for (; chain->m_name; ++chain) {
		if (strcmp(chain->m_name, attr_str) == 0)
			return chain;
	}
	return NULL;
}

/* Try to figure out information on how to access `attr' of `self'
 * @return: true:  Information was filled in.
 * @return: false: Failed to determine attribute info. */
#define DeeObject_GenericFindAttrInfo(self, attr, result) \
	DeeObject_TGenericFindAttrInfo(Dee_TYPE(self), attr, result)
PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
DeeObject_TGenericFindAttrInfo(DeeTypeObject const *tp_self, DeeStringObject *attr,
                               struct attr_info *__restrict result) {
	DeeTypeMRO mro;
	DeeTypeObject const *iter = tp_self;
	DeeTypeMRO_Init(&mro, iter);
	do {
		/* Check for C-level attribute declarations */
		if (iter->tp_methods) {
			struct Dee_type_method const *item;
			item = Dee_type_method_find(iter->tp_methods, attr);
			if (item) {
				result->ai_type = ATTR_TYPE_METHOD;
				result->ai_decl = (DeeObject *)iter;
				result->ai_value.v_method = item;
				return true;
			}
		}
		if (iter->tp_getsets) {
			struct Dee_type_getset const *item;
			item = Dee_type_getset_find(iter->tp_getsets, attr);
			if (item) {
				result->ai_type = ATTR_TYPE_GETSET;
				result->ai_decl = (DeeObject *)iter;
				result->ai_value.v_getset = item;
				return true;
			}
		}
		if (iter->tp_members) {
			struct Dee_type_member const *item;
			item = Dee_type_member_find(iter->tp_members, attr);
			if (item) {
				result->ai_type = ATTR_TYPE_MEMBER;
				result->ai_decl = (DeeObject *)iter;
				result->ai_value.v_member = item;
				return true;
			}
		}

	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return false;
}

/* Try to figure out information on how to access `attr' of `self'
 * @return: true:  Information was filled in.
 * @return: false: Failed to determine attribute info. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
DeeType_FindAttrInfo(DeeTypeObject const *self, DeeStringObject *attr,
                     struct attr_info *__restrict result) {
	DeeTypeObject const *iter;
	DeeTypeMRO mro;
	iter = self;
	DeeTypeMRO_Init(&mro, iter);
	do {
		if (DeeType_IsClass(iter)) {
			struct Dee_class_attribute const *item;
			item = DeeClass_QueryClassAttribute(iter, (DeeObject *)attr);
			if (item) {
				result->ai_type = ATTR_TYPE_ATTR;
				result->ai_decl = (DeeObject *)iter;
				result->ai_value.v_attr = item;
				return true;
			}
			item = DeeClass_QueryInstanceAttribute(iter, (DeeObject *)attr);
			if (item) {
				result->ai_type = ATTR_TYPE_INSTANCE_ATTR;
				result->ai_decl = (DeeObject *)iter;
				result->ai_value.v_instance_attr = item;
				return true;
			}
		} else {
			/* Check for C-level class attribute declarations */
			if (iter->tp_class_methods) {
				struct Dee_type_method const *item;
				item = Dee_type_method_find(iter->tp_class_methods, attr);
				if (item) {
					result->ai_type = ATTR_TYPE_METHOD;
					result->ai_decl = (DeeObject *)iter;
					result->ai_value.v_method = item;
					return true;
				}
			}
			if (iter->tp_class_getsets) {
				struct Dee_type_getset const *item;
				item = Dee_type_getset_find(iter->tp_class_getsets, attr);
				if (item) {
					result->ai_type = ATTR_TYPE_GETSET;
					result->ai_decl = (DeeObject *)iter;
					result->ai_value.v_getset = item;
					return true;
				}
			}
			if (iter->tp_class_members) {
				struct Dee_type_member const *item;
				item = Dee_type_member_find(iter->tp_class_members, attr);
				if (item) {
					result->ai_type = ATTR_TYPE_MEMBER;
					result->ai_decl = (DeeObject *)iter;
					result->ai_value.v_member = item;
					return true;
				}
			}

#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods) { /* Access instance methods using `DeeClsMethodObject' */
					struct Dee_type_method const *item;
					item = Dee_type_method_find(iter->tp_methods, attr);
					if (item) {
						result->ai_type = ATTR_TYPE_INSTANCE_METHOD;
						result->ai_decl = (DeeObject *)iter;
						result->ai_value.v_instance_method = item;
						return true;
					}
				}
				if (iter->tp_getsets) { /* Access instance getsets using `DeeClsPropertyObject' */
					struct Dee_type_getset const *item;
					item = Dee_type_getset_find(iter->tp_getsets, attr);
					if (item) {
						result->ai_type = ATTR_TYPE_INSTANCE_GETSET;
						result->ai_decl = (DeeObject *)iter;
						result->ai_value.v_instance_getset = item;
						return true;
					}
				}
				if (iter->tp_members) { /* Access instance members using `DeeClsMemberObject' */
					struct Dee_type_member const *item;
					item = Dee_type_member_find(iter->tp_members, attr);
					if (item) {
						result->ai_type = ATTR_TYPE_INSTANCE_MEMBER;
						result->ai_decl = (DeeObject *)iter;
						result->ai_value.v_instance_member = item;
						return true;
					}
				}
			}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
			if (DeeObject_GenericFindAttrInfo((DeeObject *)iter, attr, result))
				return true;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
		}
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	return DeeObject_GenericFindAttrInfo((DeeObject *)self, attr, result);
#else /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	return false;
#endif /* !CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
}

/* Try to figure out information on how to access `attr' of `self'
 * @return: true:  Information was filled in.
 * @return: false: Failed to determine attribute info. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
DeeModule_FindAttrInfo(DeeModuleObject const *self, DeeStringObject *attr,
                       struct attr_info *__restrict result) {
	struct module_symbol const *sym;
	sym = DeeModule_GetSymbol(self, (DeeObject *)attr);
	if (sym) {
		result->ai_type = ATTR_TYPE_MODSYM;
		result->ai_decl = (DeeObject *)self;
		result->ai_value.v_modsym = sym;
		return true;
	}
	return DeeObject_GenericFindAttrInfo(self, attr, result);
}


/* Try to figure out information on how to access `attr' of `self'
 * @return: true:  Information was filled in.
 * @return: false: Failed to determine attribute info. */
#define DeeObject_FindAttrInfo(self, attr, result) \
	DeeObject_TFindAttrInfo(Dee_TYPE(self), self, attr, result)
PRIVATE WUNUSED NONNULL((1, 3, 4)) bool DCALL
DeeObject_TFindAttrInfo(DeeTypeObject const *tp_self, DeeObject const *self,
                        DeeStringObject *attr, struct attr_info *__restrict result) {
	DeeTypeMRO mro;
	DeeTypeObject const *tp_iter = tp_self;
again:
	if (tp_iter->tp_attr != NULL)
		goto do_tp_iter_attr;
	DeeTypeMRO_Init(&mro, tp_iter);
	for (;;) {
		if (DeeType_IsClass(tp_iter)) {
			struct Dee_class_attribute const *item;
			item = DeeClass_QueryInstanceAttribute(tp_iter, (DeeObject *)attr);
			if (item) {
				result->ai_type = ATTR_TYPE_ATTR;
				result->ai_decl = (DeeObject *)tp_iter;
				result->ai_value.v_attr = item;
				return true;
			}
		} else {
			/* Check for C-level attribute declarations */
			if (tp_iter->tp_methods) {
				struct Dee_type_method const *item;
				item = Dee_type_method_find(tp_iter->tp_methods, attr);
				if (item) {
					result->ai_type = ATTR_TYPE_METHOD;
					result->ai_decl = (DeeObject *)tp_iter;
					result->ai_value.v_method = item;
					return true;
				}
			}
			if (tp_iter->tp_getsets) {
				struct Dee_type_getset const *item;
				item = Dee_type_getset_find(tp_iter->tp_getsets, attr);
				if (item) {
					result->ai_type = ATTR_TYPE_GETSET;
					result->ai_decl = (DeeObject *)tp_iter;
					result->ai_value.v_getset = item;
					return true;
				}
			}
			if (tp_iter->tp_members) {
				struct Dee_type_member const *item;
				item = Dee_type_member_find(tp_iter->tp_members, attr);
				if (item) {
					result->ai_type = ATTR_TYPE_MEMBER;
					result->ai_decl = (DeeObject *)tp_iter;
					result->ai_value.v_member = item;
					return true;
				}
			}
		}

		/* Move on to the next base class. */
		tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
		if (!tp_iter)
			break;

		/* Check for user-defined attribute operators. */
		if (tp_iter->tp_attr != NULL) {
do_tp_iter_attr:
			if (self != NULL) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *self, /*String*/ DeeObject *name);
				tp_getattr = tp_iter->tp_attr->tp_getattr;
				if (tp_getattr == DeeType_Type.tp_attr->tp_getattr)
					return DeeType_FindAttrInfo((DeeTypeObject *)self, attr, result);
				if (tp_getattr == DeeModule_Type.tp_attr->tp_getattr)
					return DeeModule_FindAttrInfo((DeeModuleObject *)self, attr, result);
				if (tp_getattr == DeeSuper_Type.tp_attr->tp_getattr) {
					tp_iter = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = tp_iter;
					goto again;
				}
			}
			result->ai_value.v_custom = tp_iter->tp_attr;
			return true;
		}
	}
	return false;
}






#ifdef CONFIG_HAVE_FPU
/* API function called by `operator float()' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
api_object_as_float(DeeObject *__restrict self) {
	double result;
	if unlikely(DeeObject_AsDouble(self, &result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}
#endif /* CONFIG_HAVE_FPU */

struct host_operator_specs {
	void   *hos_apifunc; /* [0..1] API function (or NULL if fallback handling must be used) */
	uint8_t hos_argc;    /* Argument count (1-4) */
	uint8_t hos_cc;      /* Operator calling convention (one of `VCALLOP_CC_*') */
};

PRIVATE struct host_operator_specs const operator_apis[] = {
	/* [OPERATOR_CONSTRUCTOR]  = */ { (void *)NULL },
	/* [OPERATOR_COPY]         = */ { (void *)&DeeObject_Copy, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DEEPCOPY]     = */ { (void *)&DeeObject_DeepCopy, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DESTRUCTOR]   = */ { (void *)NULL },
	/* [OPERATOR_ASSIGN]       = */ { (void *)&DeeObject_Assign, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_MOVEASSIGN]   = */ { (void *)&DeeObject_MoveAssign, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_STR]          = */ { (void *)&DeeObject_Str, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_REPR]         = */ { (void *)&DeeObject_Repr, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_BOOL]         = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_ITERNEXT]     = */ { (void *)NULL }, /* Special handling (because `DeeObject_IterNext' can return ITER_DONE) */
	/* [OPERATOR_CALL]         = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INT]          = */ { (void *)&DeeObject_Int, 1, VCALLOP_CC_OBJECT },
#ifdef CONFIG_HAVE_FPU
	/* [OPERATOR_FLOAT]        = */ { (void *)&api_object_as_float, 1, VCALLOP_CC_OBJECT },
#else /* CONFIG_HAVE_FPU */
	/* [OPERATOR_FLOAT]        = */ { (void *)NULL },
#endif /* !CONFIG_HAVE_FPU */
	/* [OPERATOR_INV]          = */ { (void *)&DeeObject_Inv, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_POS]          = */ { (void *)&DeeObject_Pos, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_NEG]          = */ { (void *)&DeeObject_Neg, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_ADD]          = */ { (void *)&DeeObject_Add, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SUB]          = */ { (void *)&DeeObject_Sub, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_MUL]          = */ { (void *)&DeeObject_Mul, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DIV]          = */ { (void *)&DeeObject_Div, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_MOD]          = */ { (void *)&DeeObject_Mod, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SHL]          = */ { (void *)&DeeObject_Shl, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SHR]          = */ { (void *)&DeeObject_Shr, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_AND]          = */ { (void *)&DeeObject_And, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_OR]           = */ { (void *)&DeeObject_Or, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_XOR]          = */ { (void *)&DeeObject_Xor, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_POW]          = */ { (void *)&DeeObject_Pow, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_INC]          = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_DEC]          = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_ADD]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_SUB]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_MUL]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_DIV]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_MOD]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_SHL]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_SHR]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_AND]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_OR]   = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_XOR]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_POW]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_HASH]         = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_EQ]           = */ { (void *)&DeeObject_CompareEqObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_NE]           = */ { (void *)&DeeObject_CompareNeObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_LO]           = */ { (void *)&DeeObject_CompareLoObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_LE]           = */ { (void *)&DeeObject_CompareLeObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_GR]           = */ { (void *)&DeeObject_CompareGrObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_GE]           = */ { (void *)&DeeObject_CompareGeObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_ITERSELF]     = */ { (void *)&DeeObject_IterSelf, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SIZE]         = */ { (void *)&DeeObject_SizeObject, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_CONTAINS]     = */ { (void *)&DeeObject_Contains, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_GETITEM]      = */ { (void *)&DeeObject_GetItem, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DELITEM]      = */ { (void *)&DeeObject_DelItem, 2, VCALLOP_CC_INT },
	/* [OPERATOR_SETITEM]      = */ { (void *)&DeeObject_SetItem, 3, VCALLOP_CC_INT },
	/* [OPERATOR_GETRANGE]     = */ { (void *)&DeeObject_GetRange, 3, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DELRANGE]     = */ { (void *)&DeeObject_DelRange, 3, VCALLOP_CC_INT },
	/* [OPERATOR_SETRANGE]     = */ { (void *)&DeeObject_SetRange, 4, VCALLOP_CC_INT },
	/* [OPERATOR_GETATTR]      = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_DELATTR]      = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_SETATTR]      = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_ENUMATTR]     = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_ENTER]        = */ { (void *)&DeeObject_Enter, 1, VCALLOP_CC_INT },
	/* [OPERATOR_LEAVE]        = */ { (void *)&DeeObject_Leave, 1, VCALLOP_CC_INT },
};

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vop(struct Dee_function_generator *__restrict self,
                           uint16_t operator_name, Dee_vstackaddr_t argc,
                           bool push_result) {
	struct Dee_memstate *state;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < argc)
		return err_illegal_stack_effect();

	/* Special handling for certain operators. */
	switch (operator_name) {

	case OPERATOR_BOOL:
		if (argc == 1) {
			if (Dee_memloc_typeof(Dee_function_generator_vtop(self)) == &DeeBool_Type)
				return 0;
			DO(Dee_function_generator_vopbool(self));
			goto done_with_result;
		}
		break;

	case OPERATOR_STR:
		if (argc == 1) {
			if (Dee_memloc_typeof(Dee_function_generator_vtop(self)) == &DeeString_Type)
				return 0;
		}
		break;

	case OPERATOR_CALL:
		if (argc == 2) {
			DO(Dee_function_generator_vopcalltuple(self));
			goto done_with_result;
		}
		if (argc == 3) {
			DO(Dee_function_generator_vopcalltuplekw(self));
			goto done_with_result;
		}
		break;

	case OPERATOR_GETATTR:
		if (argc == 2) {
			DO(Dee_function_generator_vopgetattr(self));
			goto done_with_result;
		}
		break;
	case OPERATOR_DELATTR:
		if (argc == 2) {
			DO(Dee_function_generator_vopdelattr(self));
			goto done_without_result;
		}
		break;

	case OPERATOR_SETATTR:
		if (argc == 3) {
			DO(Dee_function_generator_vopsetattr(self));
			goto done_without_result;
		}
		break;

	default: break;
	}

	/* TODO: If the type of the "this"-operand is known, try to
	 *       directly dispatch to the operator implementation. */

	/* Check if there is a dedicated API function. */
	if (operator_name < COMPILER_LENOF(operator_apis)) {
		struct host_operator_specs const *specs = &operator_apis[operator_name];
		if (specs->hos_apifunc != NULL && specs->hos_argc == argc) {
			DO(Dee_function_generator_vcallapi(self, specs->hos_apifunc,
			                                   specs->hos_cc, argc));
			/* Always make sure to return some value on-stack. */
			if (specs->hos_cc != VCALLOP_CC_OBJECT) {
done_without_result:
				if (push_result)
					return Dee_function_generator_vpush_const(self, Dee_None);
				return 0;
			}

			/* Certain operators always return values with certain types. */
			struct Dee_memloc *result = Dee_function_generator_vtop(self);
			switch (operator_name) {
			case OPERATOR_STR:
			case OPERATOR_REPR:
				result->ml_valtyp = &DeeString_Type;
				break;
			/*case OPERATOR_BOOL: // Special handling
				result->ml_valtyp = &DeeBool_Type;
				break;*/
			case OPERATOR_INT:
				result->ml_valtyp = &DeeInt_Type;
				break;
			case OPERATOR_FLOAT:
				result->ml_valtyp = &DeeFloat_Type;
				break;
			default: break;
			}
			return 0;
		}
	}

	/* Fallback: encode a call to `DeeObject_InvokeOperator()' */
	if unlikely(argc < 1)
		return err_illegal_stack_effect();
	--argc; /* The "this"-argument is passed individually */
	DO(Dee_function_generator_vlinear(self, argc, true));        /* this, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 2));            /* [args...], argv, this */
	DO(Dee_function_generator_vpush_imm16(self, operator_name)); /* [args...], argv, this, opname */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));         /* [args...], argv, this, opname, argc */
	DO(Dee_function_generator_vlrot(self, 4));                   /* [args...], this, opname, argc, argv */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_InvokeOperator, VCALLOP_CC_RAWINT, 4)); /* [args...], UNCHECKED(result) */
	DO(Dee_function_generator_vrrot(self, argc + 1));            /* UNCHECKED(result), [args...] */
	DO(Dee_function_generator_vpopmany(self, argc));             /* UNCHECKED(result) */
	DO(Dee_function_generator_vcheckobj(self));
done_with_result:
	if (!push_result)
		return Dee_function_generator_vpop(self);
	return 0;
err:
	return -1;
}

/* [args...], UNCHECKED(result) -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vpop_args_before_unchecked_result(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t argc) {
	DO(Dee_function_generator_vrrot(self, argc + 1));   /* UNCHECKED(result), [args...] */
	return Dee_function_generator_vpopmany(self, argc); /* UNCHECKED(result) */
err:
	return -1;
}


/* Pop "kw" (as used for `DeeObject_CallKw') and assert that it is NULL or empty:
 * >> if (__builtin_constant_p(kw) ? kw != NULL : 1) {
 * >>     size_t kw_length;
 * >>     if (DeeKwds_Check(kw)) {
 * >>         kw_length = DeeKwds_SIZE(kw);
 * >>     } else {
 * >>         kw_length = DeeObject_Size(kw);
 * >>         if unlikely(kw_length == (size_t)-1)
 * >>             goto err;
 * >>     }
 * >>     if (kw_length != 0) {
 * >>         ...
 * >>     }
 * >> }
 */
PRIVATE WUNUSED NONNULL((1)) int DCALL
vpop_empty_kwds(struct Dee_function_generator *__restrict self) {
	DREF struct Dee_memstate *enter_state;
	DREF struct Dee_memstate *leave_state;
	struct Dee_memloc *kwloc;

	/* Check for simple case: compile-time constant NULL */
	ASSERT(self->fg_state->ms_stackc >= 1);
	DO(Dee_function_generator_vdirect(self, 1));
	kwloc = Dee_function_generator_vtop(self);
	if (kwloc->ml_type == MEMLOC_TYPE_CONST) {
		/* Special case: keyword arguments are described by a compile-time constant. */
		DeeObject *kw = kwloc->ml_value.v_const;
		if (kw != NULL)
			DO(libhostasm_rt_assert_empty_kw(kw));
		return Dee_function_generator_vpop(self);
	}
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Generate inline code. */
		struct Dee_memloc loc_DeeKwds_Type, loc_kwds_ob_type, loc_size;
		struct Dee_host_symbol *Lnot_kwds;
		struct Dee_host_symbol *Lgot_size;
		Lnot_kwds = Dee_function_generator_newsym(self);
		if unlikely(!Lnot_kwds)
			goto err;
		Lgot_size = Dee_function_generator_newsym(self);
		if unlikely(!Lgot_size)
			goto err;
		DO(Dee_function_generator_vdup(self));             /* kw, kw */
		DO(Dee_function_generator_voptypeof(self, false)); /* kw, type(kw) */
		loc_kwds_ob_type = *Dee_function_generator_vtop(self);
		ASSERT(loc_kwds_ob_type.ml_flags & MEMLOC_F_NOREF);
		DO(Dee_function_generator_vpop(self)); /* kw */
		loc_DeeKwds_Type.ml_type = MEMLOC_TYPE_CONST;
		loc_DeeKwds_Type.ml_value.v_const = (DeeObject *)&DeeKwds_Type;
		DO(_Dee_function_generator_gjcmp(self, &loc_kwds_ob_type, &loc_DeeKwds_Type, false,
		                                 NULL, Lnot_kwds, NULL));
		enter_state = self->fg_state; /* kw */
		Dee_memstate_incref(enter_state);
		EDO(err_enter_state, Dee_function_generator_vdup(self));                                   /* kw, kw */
		EDO(err_enter_state, Dee_function_generator_vind(self, offsetof(DeeKwdsObject, kw_size))); /* kw, kw->kw_size */
		EDO(err_enter_state, Dee_function_generator_vreg(self, NULL));                             /* kw, reg:kw->kw_size */
		if (self->fg_sect == &self->fg_block->bb_hcold) {
			/* >>     jmp .Lgot_size
			 * >> .Lnot_kwds:
			 * >>     ...
			 * >> .Lgot_size: */
			EDO(err_enter_state, _Dee_function_generator_gjmp(self, Lgot_size));
			leave_state = self->fg_state; /* Inherit reference */
			self->fg_state = enter_state; /* Inherit reference */
			HA_printf(".Lnot_kwds:\n");
			Dee_host_symbol_setsect(Lnot_kwds, self->fg_sect);                                                 /* kw */
			EDO(err_leave_state, Dee_function_generator_vdup(self));                                           /* kw, kw */
			EDO(err_leave_state, Dee_function_generator_vcallapi(self, &DeeObject_Size, VCALLOP_CC_M1INT, 1)); /* kw, size */
			EDO(err_leave_state, Dee_function_generator_vmorph(self, leave_state));
			EDO(err_leave_state, _Dee_function_generator_gjmp(self, Lgot_size));
			HA_printf(".Lgot_size:\n");
			Dee_host_symbol_setsect(Lgot_size, self->fg_sect);
		} else {
			struct Dee_host_section *old_text;
			/* >> .section .cold
			 * >> .Lnot_kwds:
			 * >>     ...
			 * >>     jmp .Lgot_size
			 * >> .section .text
			 * >> .Lgot_size: */
			HA_printf(".section .cold\n");
			old_text = self->fg_sect;
			self->fg_sect = &self->fg_block->bb_hcold;
			leave_state = self->fg_state; /* Inherit reference */
			self->fg_state = enter_state; /* Inherit reference */
			HA_printf(".Lnot_kwds:\n");
			Dee_host_symbol_setsect(Lnot_kwds, self->fg_sect);                                                 /* kw */
			EDO(err_leave_state, Dee_function_generator_vdup(self));                                           /* kw, kw */
			EDO(err_leave_state, Dee_function_generator_vcallapi(self, &DeeObject_Size, VCALLOP_CC_M1INT, 1)); /* kw, size */
			EDO(err_leave_state, Dee_function_generator_vmorph(self, leave_state));
			EDO(err_leave_state, _Dee_function_generator_gjmp(self, Lgot_size));
			HA_printf(".section .text\n");
			self->fg_sect = old_text;
			HA_printf(".Lgot_size:\n");
			Dee_host_symbol_setsect(Lgot_size, self->fg_sect);
		}
		Dee_memstate_decref(self->fg_state);
		self->fg_state = leave_state; /* Inherit reference */

		loc_size = *Dee_function_generator_vtop(self); /* kw, size */
		ASSERT(loc_kwds_ob_type.ml_flags & MEMLOC_F_NOREF);
		DO(Dee_function_generator_vpop(self)); /* kw */

		if (self->fg_sect == &self->fg_block->bb_hcold) {
			struct Dee_host_symbol *Lsize_is_zero;
			/* >>     jz <loc_size>, .Lsize_is_zero
			 * >>     ...
			 * >> .Lsize_is_zero: */
			Lsize_is_zero = Dee_function_generator_newsym(self);
			if unlikely(!Lsize_is_zero)
				goto err;
			DO(_Dee_function_generator_gjz(self, &loc_size, Lsize_is_zero));
			enter_state = self->fg_state;
			Dee_memstate_incref(enter_state);
			EDO(err_enter_state, Dee_function_generator_vcallapi(self, &libhostasm_rt_err_nonempty_kw, VCALLOP_CC_EXCEPT, 1));
			HA_printf(".Lsize_is_zero:\n");
			Dee_host_symbol_setsect(Lsize_is_zero, self->fg_sect);
		} else {
			struct Dee_host_symbol *Lsize_is_not_zero;
			struct Dee_host_section *old_text;
			/* >>     jnz <loc_size>, .Lsize_is_not_zero
			 * >> .section .cold
			 * >> .Lsize_is_not_zero:
			 * >>     ...
			 * >> .section .text
			 */
			Lsize_is_not_zero = Dee_function_generator_newsym(self);
			if unlikely(!Lsize_is_not_zero)
				goto err;
			DO(_Dee_function_generator_gjnz(self, &loc_size, Lsize_is_not_zero));
			enter_state = self->fg_state;
			Dee_memstate_incref(enter_state);
			HA_printf(".section .cold\n");
			old_text = self->fg_sect;
			self->fg_sect = &self->fg_block->bb_hcold;
			HA_printf(".Lsize_is_not_zero:\n");
			Dee_host_symbol_setsect(Lsize_is_not_zero, self->fg_sect);
			EDO(err_enter_state, Dee_function_generator_vcallapi(self, &libhostasm_rt_err_nonempty_kw, VCALLOP_CC_EXCEPT, 1));
			HA_printf(".section .text\n");
			self->fg_sect = old_text;
		}
		Dee_memstate_decref(self->fg_state);
		self->fg_state = enter_state;             /* kw */
		return Dee_function_generator_vpop(self); /* - */
	}

	/* Use an API function to do the assert for us. */
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_assert_empty_kw, VCALLOP_CC_INT, 1);
err_leave_state:
	Dee_memstate_decref(leave_state);
	goto err;
err_enter_state:
	Dee_memstate_decref(enter_state);
err:
	return -1;
}


struct docinfo {
	char const      *di_doc; /* [0..1] Doc string. */
	DeeModuleObject *di_mod; /* [0..1] Associated module. */
	DeeTypeObject   *di_typ; /* [0..1] Associated type. */
};

#define isnulorlf(ch) ((ch) == '\0' || (ch) == '\n')

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) char const *DCALL
seek_after_unescaped_char(char const *iter, char findme) {
	while (!isnulorlf(*iter)) {
		char ch = *iter++;
		if (ch == findme) {
			break;
		} else if (ch == '\\') {
			if (!isnulorlf(*iter))
				++iter;
		} else if (ch == '{') {
			iter = seek_after_unescaped_char(iter, '}');
		}
	}
	return iter;
}

struct typexpr_parser {
	char const                    *txp_iter; /* [1..1] Pointer into type information. */
	struct docinfo const          *txp_info; /* [1..1][const] Doc information. */
	struct Dee_function_generator *txp_gen;  /* [1..1][const] Function generator. */
};

struct type_expression_name {
	char const           *ten_start; /* [1..1] Start of name */
	char const           *ten_end;   /* [1..1] End of name */
	DREF DeeStringObject *ten_str;   /* [0..1] Name string (in case an extended name was used) */
};

#define type_expression_name_fini(self) Dee_XDecref((self)->ten_str)

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_expression_name_unescape(struct type_expression_name *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char const *iter, *end, *flush_start;

	/* Parse the string and unescape special symbols. */
	iter = self->ten_start;
	end  = self->ten_end;
	flush_start = iter;
	while (iter < end) {
		char ch = *iter++;
		if (ch == '\\') { /* Remove every first '\'-character */
			if unlikely(unicode_printer_print(&printer, flush_start,
			                                  (size_t)((iter - 1) - flush_start)) < 0)
				goto err_printer;
			flush_start = iter;
			if (iter < end)
				++iter; /* Don't remove the character following '\', even if it's another '\' */
		}
	}
	if (flush_start < end) {
		if unlikely(unicode_printer_print(&printer, flush_start,
		                                  (size_t)(end - flush_start)) < 0)
			goto err_printer;
	}

	/* Pack the unicode string */
	self->ten_str = (DREF DeeStringObject *)unicode_printer_pack(&printer);
	if unlikely(!self->ten_str)
		goto err;
	self->ten_start = DeeString_AsUtf8((DeeObject *)self->ten_str);
	if unlikely(!self->ten_start)
		goto err_ten_str;
	self->ten_end = self->ten_start + WSTR_LENGTH(self->ten_start);
	return 0;
err_ten_str:
	Dee_Decref(self->ten_str);
	goto err;
err_printer:
	unicode_printer_fini(&printer);
err:
	return -1;
}

/* Parse a type-expression `<NAME>' element
 * @return: 0 : Success (*result was initialized)
 * @return: -1: An error was thrown (*result is in an undefined state) */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
typexpr_parser_parsename(struct typexpr_parser *__restrict self,
                         /*out*/ struct type_expression_name *__restrict result) {
	char const *doc = self->txp_iter;
	result->ten_str = NULL;
	if (*doc != '{') {
		result->ten_start = doc;
		while (DeeUni_IsSymCont(*doc))
			++doc;
		result->ten_end = doc;
		self->txp_iter  = doc;
		return 0;
	}
	++doc;
	result->ten_start = doc;
	doc = strchr(doc, '}');
	if unlikely(!doc)
		goto err_bad_doc_string;
	result->ten_end = doc;
	self->txp_iter  = doc + 1;

	/* Check if the string must be unescaped (i.e. contains any '\' characters) */
	if (memchr(result->ten_start, '\\',
	           (size_t)(result->ten_end - result->ten_start)) != NULL)
		return type_expression_name_unescape(result);
	return 0;
err_bad_doc_string:
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Malformed type annotation: Missing '}' after '{' in %q",
	                       self->txp_iter);
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
appears_in_import_tree(DeeModuleObject const *import_tree_of_this,
                       DeeModuleObject const *contains_this) {
	uint16_t mid;
	for (mid = 0; mid < import_tree_of_this->mo_importc; ++mid) {
		if (import_tree_of_this->mo_importv[mid] == contains_this)
			return true;
	}
	for (mid = 0; mid < import_tree_of_this->mo_importc; ++mid) {
		if (appears_in_import_tree(import_tree_of_this->mo_importv[mid],
		                           contains_this))
			return true;
	}
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeModule_CheckStaticAndDecref(struct typexpr_parser *__restrict self,
                               /*inherit(on_success)*/ DREF DeeModuleObject *mod) {
	DeeModuleObject *commod;
	if unlikely(!DeeObject_IsShared(mod))
		return false;
	if (mod == (DREF DeeModuleObject *)&DeeModule_Deemon)
		goto ok;
	if (mod == (DREF DeeModuleObject *)self->txp_info->di_mod)
		goto ok;
	commod = self->txp_gen->fg_assembler->fa_code->co_module;
	if (commod == mod)
		goto ok;
	if (appears_in_import_tree(commod, mod))
		goto ok;
	return false;
ok:
	Dee_DecrefNokill(mod);
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeType_CheckStaticAndDecref(struct typexpr_parser *__restrict self,
                             /*inherit(on_success)*/ DREF DeeTypeObject *type) {
	if unlikely(!DeeObject_IsShared(type))
		return false;
	if (!DeeType_IsCustom(type)) {
		DREF DeeModuleObject *type_module;
		if (type == self->txp_info->di_typ) {
ok:
			Dee_DecrefNokill(type);
			return true;
		}
		type_module = (DREF DeeModuleObject *)DeeType_GetModule(type);
		if likely(type_module) {
			if (DeeModule_CheckStaticAndDecref(self, type_module))
				goto ok;
		}
	}
	return false;
}

/* @return: * :   The encoded object
 * @return: NULL: The encoded object could not be determined (generic / multiple-choice)
 * @return: ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
typexpr_parser_parse_object_after_qmark(struct typexpr_parser *__restrict self) {
	DeeObject *result;
	struct type_expression_name name;
	switch (*self->txp_iter++) {

	case '.':
		return (DeeObject *)self->txp_info->di_typ;

	case 'N':
		return (DeeObject *)&DeeNone_Type;
	case 'O':
		return (DeeObject *)&DeeObject_Type;

	case '#':
	case 'D':
	case 'G':
	case 'E':
	case 'A': {
		char how = self->txp_iter[-1];
		if (typexpr_parser_parsename(self, &name))
			goto err;
		switch (how) {
		case '#':
			/* Use current context as base */
			result = (DeeObject *)self->txp_info->di_typ;
			if (result == NULL) {
		case 'G':
				result = (DeeObject *)self->txp_info->di_mod;
			if (result == NULL)
				goto unknown;
			}
			break;
		case 'D':
			result = (DeeObject *)&DeeModule_Deemon;
			break;

		case 'E': {
			DREF DeeObject *mod_export;
			if (name.ten_str) {
				result = (DeeObject *)DeeModule_OpenGlobal((DeeObject *)name.ten_str, NULL, false);
			} else {
				result = (DeeObject *)DeeModule_OpenGlobalString(name.ten_start,
				                                                 (size_t)(name.ten_end - name.ten_start),
				                                                 NULL, false);
			}
			type_expression_name_fini(&name);
			if (result == ITER_DONE)
				goto unknown;
			if (result == NULL)
				goto err;
			if (*self->txp_iter != ':') {
				Dee_Decref(result);
				goto unknown;
			}
			++self->txp_iter;
			if (typexpr_parser_parsename(self, &name)) {
				Dee_Decref(result);
				goto err;
			}
			if (name.ten_str) {
				mod_export = DeeObject_GetAttr(result, (DeeObject *)name.ten_str);
			} else {
				mod_export = DeeObject_GetAttrStringLen(result, name.ten_start,
				                                        (size_t)(name.ten_end - name.ten_start));
			}
			type_expression_name_fini(&name);
			Dee_Decref(result);
			result = mod_export;
			goto fini_name_and_check_result_after_getattr;
		}	break;

		case 'A':
			if (*self->txp_iter != '?')
				goto unknown;
			++self->txp_iter;
			result = typexpr_parser_parse_object_after_qmark(self);
			if (!ITER_ISOK(result)) {
				type_expression_name_fini(&name);
				return result;
			}
			break;
		default: __builtin_unreachable();
		}
		ASSERT(result);
		if (name.ten_str) {
			result = DeeObject_GetAttr(result, (DeeObject *)name.ten_str);
		} else {
			result = DeeObject_GetAttrStringLen(result, name.ten_start,
			                                    (size_t)(name.ten_end - name.ten_start));
		}
fini_name_and_check_result_after_getattr:
		type_expression_name_fini(&name);
		if unlikely(!result) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			goto unknown;
		}
		/* Validate that "result" can be used. */
		if (DeeType_Check(result)) {
			if (!DeeType_CheckStaticAndDecref(self, (DeeTypeObject *)result))
				goto unknown_decref_result;
		} else if (DeeModule_Check(result)) {
			if (!DeeModule_CheckStaticAndDecref(self, (DeeModuleObject *)result))
				goto unknown_decref_result;
		} else {
			Dee_Decref(result);
			goto unknown;
		}
	}	break;

	default:
		goto unknown;
	}
	return result;
unknown_decref_result:
	Dee_Decref(result);
unknown:
	return NULL; /* Unknown... */
err:
	return ITER_DONE;
}

/* @return: * :   The return type of the overload
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
typexpr_parser_extract_overload_return_type(struct typexpr_parser *__restrict self) {
	DeeTypeObject *result;
	if (self->txp_iter[0] == '(')
		self->txp_iter = seek_after_unescaped_char(self->txp_iter, ')');
	if (self->txp_iter[0] != '-' || self->txp_iter[1] != '>')
		return &DeeNone_Type; /* No return pointer -> function returns "none" */
	self->txp_iter += 2;
	if (self->txp_iter[0] != '?')
		return &DeeObject_Type; /* Nothing after return pointer -> function returns "Object" */
	self->txp_iter += 1;
	result = (DeeTypeObject *)typexpr_parser_parse_object_after_qmark(self);
	if (ITER_ISOK(result) && !DeeType_Check(result))
		result = NULL; /* Not actually a type */
	return result;
}

/* @return: * :   The return type of the overload
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeTypeObject *DCALL
overload_extract_return_type(char const *iter, struct docinfo const *__restrict info,
                             struct Dee_function_generator *__restrict generator) {
	struct typexpr_parser parser;
	parser.txp_iter = iter;
	parser.txp_info = info;
	parser.txp_gen  = generator;
	return typexpr_parser_extract_overload_return_type(&parser);
}

#define OVERLOAD_MATCHES_ERR   (-1) /* Error */
#define OVERLOAD_MATCHES_NO    0    /* No match */
#define OVERLOAD_MATCHES_MAYBE 1    /* Potential match (insufficient type information available) */
#define OVERLOAD_MATCHES_YES   2    /* Full match */

/* Check how/if the overload for "iter" matches the parameter list. */
PRIVATE WUNUSED NONNULL((1, 3, 4, 5)) int DCALL
overload_matches_arguments(struct Dee_function_generator *__restrict self,
                           Dee_vstackaddr_t argc, char const *iter,
                           struct docinfo const *__restrict info,
                           struct Dee_function_generator *__restrict generator) {
	struct Dee_memstate const *state = self->fg_state;
#define LOCAL_locfor_arg(argi) (&state->ms_stackv[state->ms_stackc - argc + (argi)])
#define LOCAL_typeof_arg(argi) Dee_memloc_typeof(LOCAL_locfor_arg(argi))
	int result = OVERLOAD_MATCHES_YES;
	size_t argi = 0;
	if (iter[0] == '(') {
		struct typexpr_parser parser;
		parser.txp_iter = iter + 1;
		parser.txp_info = info;
		parser.txp_gen  = generator;
		while (!isnulorlf(*parser.txp_iter) && *parser.txp_iter != ')') {
			/* Seek until after the argument name */
			for (;;) {
				char ch = *parser.txp_iter++;
				if (isnulorlf(ch) || strchr("!?,=:)", ch)) {
					--parser.txp_iter;
					break;
				}
				if (ch == '{') {
					parser.txp_iter = seek_after_unescaped_char(parser.txp_iter, '}');
					continue;
				}
				if (ch == '\\' && *parser.txp_iter)
					++parser.txp_iter;
			}
			if (strchr("?!=", *parser.txp_iter)) {
				/* Optional or varargs from here on. */
				return result;
			} else if (*parser.txp_iter == ':') {
				/* Check if there is a type encoded here. */
				DeeTypeObject *want_argtype = &DeeObject_Type;
				DeeTypeObject *have_argtype;
				++parser.txp_iter;
				if (*parser.txp_iter == '?') {
					++parser.txp_iter;
					want_argtype = (DeeTypeObject *)typexpr_parser_parse_object_after_qmark(&parser);
					if unlikely(want_argtype == (DeeTypeObject *)ITER_DONE)
						goto err;
					if (want_argtype) {
						if (!DeeType_Check(want_argtype))
							want_argtype = NULL;
					} else {
						for (;;) {
							char ch = *parser.txp_iter++;
							if (isnulorlf(ch) || strchr(",=)", ch)) {
								--parser.txp_iter;
								break;
							}
							if (ch == '{') {
								parser.txp_iter = seek_after_unescaped_char(parser.txp_iter, '}');
								continue;
							}
							if (ch == '\\' && *parser.txp_iter)
								++parser.txp_iter;
						}
					}
				}
				if (*parser.txp_iter == '=')
					return result; /* Optional from here on. */
				if (argi >= argc)
					return OVERLOAD_MATCHES_NO; /* Too few arguments for call */
				if (want_argtype != &DeeObject_Type) {
					if (want_argtype == NULL) {
						/* Failed to determined wanted argument type (or expression too complex) */
						result = OVERLOAD_MATCHES_MAYBE;
					} else {
						have_argtype = LOCAL_typeof_arg(argi);
						if (have_argtype == NULL) {
							/* Unknown, but may be a potential overload... */
							result = OVERLOAD_MATCHES_MAYBE;
						} else if (!DeeType_Implements(have_argtype, want_argtype)) {
							/* Wrong argument type -> incorrect overload */
							return OVERLOAD_MATCHES_NO;
						}
					}
				}
			}

			/* Seek to the next argument in case the current one wasn't parsed fully */
			for (;;) {
				char ch = *parser.txp_iter++;
				if (isnulorlf(ch) || ch == ')') {
					--parser.txp_iter;
					break;
				}
				if (ch == '{') {
					parser.txp_iter = seek_after_unescaped_char(parser.txp_iter, '}');
					continue;
				}
				if (ch == ',')
					break;
				if (ch == '\\' && *parser.txp_iter)
					++parser.txp_iter;
			}
			++argi;
		}
	}
	if (argi != argc)
		return OVERLOAD_MATCHES_NO;
	return result;
err:
	return OVERLOAD_MATCHES_ERR;
#undef LOCAL_typeof_arg
#undef LOCAL_locfor_arg
}

/* @return: * :   The correct return type
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 3)) DeeTypeObject *DCALL
impl_extra_return_type_from_doc(struct Dee_function_generator *__restrict self,
                                Dee_vstackaddr_t argc,
                                struct docinfo const *__restrict info) {
	bool is_first;
	char const *iter, *next;
	DeeTypeObject *maybe_matched_overload_type = NULL;
	ASSERT(info->di_doc != NULL);
	iter = info->di_doc;
	is_first = true;
	while (*iter) {
		int how;
		next = strchrnul(iter, '\n');
		if (*next)
			++next;
		if (!(iter[0] == '(' || (iter[0] == '-' && iter[1] == '>')))
			break; /* End of prototype declaration list. */
		if (is_first && !(next[0] == '(' || (next[0] == '-' && next[1] == '>')))
			return overload_extract_return_type(iter, info, self); /* Only a singular overload exists. */
		how = overload_matches_arguments(self, argc, iter, info, self);
		if (how != OVERLOAD_MATCHES_NO) {
			if unlikely(how == OVERLOAD_MATCHES_ERR)
				goto err;
			if (how == OVERLOAD_MATCHES_YES)
				return overload_extract_return_type(iter, info, self);
			ASSERT(how == OVERLOAD_MATCHES_MAYBE);
			if (maybe_matched_overload_type != (DeeTypeObject *)ITER_DONE) {
				DeeTypeObject *overload_type = overload_extract_return_type(iter, info, self);
				if unlikely(overload_type == (DeeTypeObject *)ITER_DONE)
					goto err;
				if (overload_type == NULL)
					overload_type = (DeeTypeObject *)ITER_DONE;
				if (maybe_matched_overload_type == NULL) {
					maybe_matched_overload_type = overload_type; /* Initial option */
				} else if (maybe_matched_overload_type != overload_type) {
					maybe_matched_overload_type = (DeeTypeObject *)ITER_DONE; /* Multiple options... */
				}
			}
		}
		iter = next;
		is_first = false;
	}

	/* If we got exactly 1 potential overload, then that one has to be it. */
	if (maybe_matched_overload_type == (DeeTypeObject *)ITER_DONE)
		maybe_matched_overload_type = NULL;
	return maybe_matched_overload_type;
err:
	return (DeeTypeObject *)ITER_DONE;
}


/* [args...] -> [args...]
 * Try to extract the type of object returned by a C function as per `info'
 * NOTE: *only* do this for C functions (since type annotations from user-code
 *       may not be correct and thus cannot be trusted unconditionally)
 * NOTE: This function assumes that `info->di_doc != NULL'
 * @return: * :   The correct return type
 * @return: NULL: Return type could not be determined (generic / multiple-choice)
 * @return: (DeeTypeObject *)ITER_DONE: Error */
PRIVATE WUNUSED NONNULL((1, 3)) DeeTypeObject *DCALL
extra_return_type_from_doc(struct Dee_function_generator *__restrict self,
                           Dee_vstackaddr_t argc, struct docinfo *info) {
	DeeTypeObject *result;
	ASSERT(info->di_doc != NULL);
	if (info->di_typ != NULL && info->di_mod == NULL) {
		info->di_mod = (DeeModuleObject *)DeeType_GetModule(info->di_typ);
		result = impl_extra_return_type_from_doc(self, argc, info);
		Dee_XDecref(info->di_mod);
	} else {
		result = impl_extra_return_type_from_doc(self, argc, info);
	}
	if (result && result != (DeeTypeObject *)ITER_DONE && DeeType_IsAbstract(result)) {
		/* Ignore abstract return type information (like sequence proxies).
		 * We want the *exact* return type (not some base class). As such,
		 * simply disregard abstract types.
		 * XXX: Technically, we'd need to disregard anything that's not a
		 *      final type here, since it's technically OK to document a
		 *      non-abstract, non-final base class as return type a sub-
		 *      class of it. (does that happen anywhere?) */
		result = NULL;
	}
	return result;
}

/* [args...], UNCHECKED(result) -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
vpop_args_before_unchecked_result_with_doc(struct Dee_function_generator *__restrict self,
                                           Dee_vstackaddr_t argc, struct docinfo *doc) {
	ASSERT(doc);
	if (doc->di_doc != NULL && !(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
		DeeTypeObject *result_type;
		DO(Dee_function_generator_state_unshare(self));
		--self->fg_state->ms_stackc; /* Cheat a little here... */
		result_type = extra_return_type_from_doc(self, argc, doc);
		++self->fg_state->ms_stackc;
		if (result_type != NULL) {
			if unlikely(result_type == (DeeTypeObject *)ITER_DONE)
				goto err;
			DO(Dee_function_generator_vsettyp(self, result_type));
		}
	}
	return vpop_args_before_unchecked_result(self, argc);
err:
	return -1;
}


/* this -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vcall_getmethod_unchecked(struct Dee_function_generator *__restrict self,
                          Dee_getmethod_t func, struct docinfo *doc) {
	int result = Dee_function_generator_vcallapi(self, func, VCALLOP_CC_RAWINT, 1);
	if likely(result == 0)
		result = vpop_args_before_unchecked_result_with_doc(self, 0, doc);
	return result;
}

/* this -> result */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_boundmethod(struct Dee_function_generator *__restrict self, Dee_boundmethod_t func) {
	DO(Dee_function_generator_vcallapi(self, func, VCALLOP_CC_M1INT, 1));
	DO(Dee_function_generator_vdirect(self, 1));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
	Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* this -> N/A */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_delmethod(struct Dee_function_generator *__restrict self, Dee_delmethod_t func) {
	return Dee_function_generator_vcallapi(self, func, VCALLOP_CC_INT, 1);
}

/* this, value -> N/A */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vcall_setmethod(struct Dee_function_generator *__restrict self, Dee_setmethod_t func) {
	return Dee_function_generator_vcallapi(self, func, VCALLOP_CC_INT, 2);
}

/* this, [args...] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_objmethod_unchecked(struct Dee_function_generator *__restrict self,
                          Dee_objmethod_t func, Dee_vstackaddr_t argc,
                          struct docinfo *doc) {
	DO(Dee_function_generator_vlinear(self, argc, true));                  /* this, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 2));                      /* [args...], argv, this */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));                   /* [args...], argv, this, argc */
	DO(Dee_function_generator_vlrot(self, 3));                             /* [args...], this, argc, argv */
	DO(Dee_function_generator_vcallapi(self, func, VCALLOP_CC_RAWINT, 3)); /* [args...], UNCHECKED(result) */
	return vpop_args_before_unchecked_result_with_doc(self, argc, doc);    /* UNCHECKED(result) */
err:
	return -1;
}

/* this, [args...], kw -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_kwobjmethod_unchecked(struct Dee_function_generator *__restrict self,
                            Dee_kwobjmethod_t func, Dee_vstackaddr_t argc,
                            struct docinfo *doc) {
	DO(Dee_function_generator_vrrot(self, argc + 1));                      /* this, kw, [args...]*/
	DO(Dee_function_generator_vlinear(self, argc, true));                  /* this, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 3));                      /* kw, [args...], argv, this */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));                   /* kw, [args...], argv, this, argc */
	DO(Dee_function_generator_vlrot(self, 3));                             /* kw, [args...], this, argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 4));                      /* [args...], this, argc, argv, kw */
	DO(Dee_function_generator_vcallapi(self, func, VCALLOP_CC_RAWINT, 4)); /* [args...], UNCHECKED(result) */
	return vpop_args_before_unchecked_result_with_doc(self, argc, doc);    /* UNCHECKED(result) */
err:
	return -1;
}

/* [args...] -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_cmethod_unchecked(struct Dee_function_generator *__restrict self,
                        Dee_cmethod_t func, Dee_vstackaddr_t argc,
                        struct docinfo *doc) {
	DO(Dee_function_generator_vlinear(self, argc, true));                  /* [args...], argv */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));                   /* [args...], argv, argc */
	DO(Dee_function_generator_vswap(self));                                /* [args...], argc, argv */
	DO(Dee_function_generator_vcallapi(self, func, VCALLOP_CC_RAWINT, 2)); /* [args...], UNCHECKED(result) */
	return vpop_args_before_unchecked_result_with_doc(self, argc, doc);    /* UNCHECKED(result) */
err:
	return -1;
}

/* [args...], kw -> UNCHECKED(result) */
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
vcall_kwcmethod_unchecked(struct Dee_function_generator *__restrict self,
                          Dee_kwcmethod_t func, Dee_vstackaddr_t argc,
                          struct docinfo *doc) {
	DO(Dee_function_generator_vrrot(self, argc + 1));                      /* kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true));                  /* kw, [args...], argv */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));                   /* kw, [args...], argv, argc */
	DO(Dee_function_generator_vswap(self));                                /* kw, [args...], argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 3));                      /* [args...], argc, argv, kw */
	DO(Dee_function_generator_vcallapi(self, func, VCALLOP_CC_RAWINT, 3)); /* [args...], UNCHECKED(result) */
	return vpop_args_before_unchecked_result_with_doc(self, argc, doc);    /* UNCHECKED(result) */
err:
	return -1;
}

/* func, [args...], kw -> UNCHECKED(result)
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `Dee_TYPE(func_obj)'
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
vopcallkw_constfunc_unchecked(struct Dee_function_generator *__restrict self,
                              DeeObject *func_obj, Dee_vstackaddr_t true_argc) {
	struct docinfo doc;
	DeeTypeObject *func_type = Dee_TYPE(func_obj);
	bzero(&doc, sizeof(doc));
	if (func_type == &DeeObjMethod_Type) {
		DeeObjMethodObject *func = (DeeObjMethodObject *)func_obj;
		DO(vpop_empty_kwds(self));                                   /* func, [args...] */
		DO(Dee_function_generator_vlrot(self, true_argc + 1));       /* [args...], func */
		DO(Dee_function_generator_vpop(self));                       /* [args...] */
		DO(Dee_function_generator_vpush_const(self, func->om_this)); /* [args...], this */
		DO(Dee_function_generator_vrrot(self, true_argc + 1));       /* this, [args...] */
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
			doc.di_doc = DeeObjMethod_GetDoc((DeeObject *)func);
			if (doc.di_doc != NULL)
				doc.di_typ = DeeObjMethod_GetType((DeeObject *)func);
		}
		return vcall_objmethod_unchecked(self, func->om_func, true_argc, &doc);
	} else if (func_type == &DeeKwObjMethod_Type) {
		DeeKwObjMethodObject *func = (DeeKwObjMethodObject *)func_obj;
		DO(Dee_function_generator_vlrot(self, true_argc + 2));       /* [args...], kw, func */
		DO(Dee_function_generator_vpop(self));                       /* [args...], kw */
		DO(Dee_function_generator_vpush_const(self, func->om_this)); /* [args...], kw, this */
		DO(Dee_function_generator_vrrot(self, true_argc + 2));       /* this, [args...], kw */
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
			doc.di_doc = DeeKwObjMethod_GetDoc((DeeObject *)func);
			if (doc.di_doc != NULL)
				doc.di_typ = DeeKwObjMethod_GetType((DeeObject *)func);
		}
		return vcall_kwobjmethod_unchecked(self, func->om_func, true_argc, &doc);
	} else if (func_type == &DeeClsMethod_Type) {
		DeeClsMethodObject *func = (DeeClsMethodObject *)func_obj;
		if (true_argc >= 1) {
			Dee_vstackaddr_t argc = true_argc - 1; /* Account for "this" argument */
			DO(vpop_empty_kwds(self));                                    /* func, this, [args...] */
			DO(Dee_function_generator_vlrot(self, argc + 2));             /* this, [args...], func */
			DO(Dee_function_generator_vpop(self));                        /* this, [args...] */
			DO(Dee_function_generator_vlrot(self, argc + 1));             /* [args...], this */
			DO(Dee_function_generator_vassert_type(self, func->ob_type)); /* [args...], this */
			DO(Dee_function_generator_vrrot(self, argc + 1));             /* this, [args...] */
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
				doc.di_doc = DeeClsMethod_GetDoc((DeeObject *)func);
				doc.di_typ = func->cm_type;
			}
			return vcall_objmethod_unchecked(self, func->cm_func, argc, &doc);
		}
	} else if (func_type == &DeeKwClsMethod_Type) {
		DeeKwClsMethodObject *func = (DeeKwClsMethodObject *)func_obj;
		if (true_argc >= 1) {
			Dee_vstackaddr_t argc = true_argc - 1; /* Account for "this" argument */
			DO(Dee_function_generator_vlrot(self, argc + 3));             /* this, [args...], kw, func */
			DO(Dee_function_generator_vpop(self));                        /* this, [args...], kw */
			DO(Dee_function_generator_vlrot(self, argc + 2));             /* [args...], kw, this */
			DO(Dee_function_generator_vassert_type(self, func->ob_type)); /* [args...], kw, this */
			DO(Dee_function_generator_vrrot(self, argc + 2));             /* this, [args...], kw */
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
				doc.di_doc = DeeKwClsMethod_GetDoc((DeeObject *)func);
				doc.di_typ = func->cm_type;
			}
			return vcall_kwobjmethod_unchecked(self, func->cm_func, argc, &doc);
		}
	} else if (func_type == &DeeClsProperty_Type) {
		DeeClsPropertyObject *func = (DeeClsPropertyObject *)func_obj;
		if (func->cp_get && true_argc == 1) {
			DO(vpop_empty_kwds(self));                                    /* func, this */
			DO(Dee_function_generator_vassert_type(self, func->cp_type)); /* func, this */
			DO(Dee_function_generator_vswap(self));                       /* this, func */
			DO(Dee_function_generator_vpop(self));                        /* this */
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE)) {
				doc.di_doc = DeeClsProperty_GetDoc((DeeObject *)func);
				doc.di_typ = func->cp_type;
			}
			return vcall_getmethod_unchecked(self, func->cp_get, &doc);
		}
	} else if (func_type == &DeeClsMember_Type) {
		DeeClsMemberObject *func = (DeeClsMemberObject *)func_obj;
		if (true_argc == 1) {
			DO(vpop_empty_kwds(self));                                    /* func, this */
			DO(Dee_function_generator_vassert_type(self, func->cm_type)); /* func, this */
			DO(Dee_function_generator_vswap(self));                       /* this, func */
			DO(Dee_function_generator_vpop(self));                        /* this */
			/* XXX: Look at current instruction to see if the result needs to be a reference. */
			return Dee_function_generator_vpush_type_member(self, func->cm_type, &func->cm_memb, true);
		}
	} else if (func_type == &DeeCMethod_Type) {
		int result;
		DeeCMethodObject *func = (DeeCMethodObject *)func_obj;
		DO(vpop_empty_kwds(self));                             /* func, [args...] */
		DO(Dee_function_generator_vlrot(self, true_argc + 1)); /* [args...], func */
		DO(Dee_function_generator_vpop(self));                 /* [args...] */
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE) {
			result = vcall_cmethod_unchecked(self, func->cm_func, true_argc, &doc);
		} else {
			struct cmethod_docinfo di;
			DeeCMethod_DocInfo(func->cm_func, &di);
			doc.di_doc = di.dmdi_doc;
			doc.di_mod = di.dmdi_mod;
			doc.di_typ = di.dmdi_typ;
			result = vcall_cmethod_unchecked(self, func->cm_func, true_argc, &doc);
			Dee_cmethod_docinfo_fini(&di);
		}
		return result;
	} else if (func_type == &DeeKwCMethod_Type) {
		int result;
		DeeKwCMethodObject *func = (DeeKwCMethodObject *)func_obj;
		DO(Dee_function_generator_vlrot(self, true_argc + 2)); /* [args...], kw, func */
		DO(Dee_function_generator_vpop(self));                 /* [args...], kw */
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NORTTITYPE) {
			result = vcall_kwcmethod_unchecked(self, func->cm_func, true_argc, &doc);
		} else {
			struct cmethod_docinfo di;
			DeeKwCMethod_DocInfo(func->cm_func, &di);
			doc.di_doc = di.dmdi_doc;
			doc.di_mod = di.dmdi_mod;
			doc.di_typ = di.dmdi_typ;
			result = vcall_kwcmethod_unchecked(self, func->cm_func, true_argc, &doc);
			Dee_cmethod_docinfo_fini(&di);
		}
		return result;
	}
	return 1; /* No dedicated optimization available */
err:
	return -1;
}

/* func, [args...], kw -> UNCHECKED(result) */
INTERN WUNUSED NONNULL((1)) int DCALL
impl_vopcallkw_unchecked(struct Dee_function_generator *__restrict self,
                         Dee_vstackaddr_t true_argc, bool prefer_thiscall) {
	DeeTypeObject *func_type;
	struct Dee_memloc *funcloc;
	if unlikely(self->fg_state->ms_stackc < (true_argc + 2))
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	funcloc = Dee_function_generator_vtop(self) - (true_argc + 1);
	if (!MEMLOC_VMORPH_ISDIRECT(funcloc->ml_vmorph)) {
		DO(Dee_function_generator_gdirect(self, funcloc));
		funcloc = Dee_function_generator_vtop(self) - (true_argc + 1);
	}

	/* Optimizations for when the is a constant. (e.g. `DeeObjMethodObject') */
	if (funcloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject *func_obj = funcloc->ml_value.v_const;
		int temp = vopcallkw_constfunc_unchecked(self, func_obj, true_argc);
		if (temp <= 0)
			return temp; /* Constant call encoded, or error */
	}

	/* Optimizations when `type(func)' is known by skipping operator
	 * resolution and directly invoking the tp_call[_kw]-operator. */
	func_type = Dee_memloc_typeof(funcloc);
	if (func_type != NULL && DeeType_InheritOperator(func_type, OPERATOR_CALL)) {
		ASSERT(func_type->tp_call || func_type->tp_call_kw);
		/* TODO */
	}

	/* Fallback: generate code to do a dynamic call at runtime. */
	if (prefer_thiscall) {
		Dee_vstackaddr_t argc = true_argc - 1;
		DO(Dee_function_generator_vrrot(self, argc + 1)); /* func, this, kw, [args...] */
		/* TODO: If generating the linear version of `[args...]' combined with `this' prefixed
		 *       is not any more complex than it is without, then include it in the argument
		 *       list and encode as `DeeObject_ThisCall()' instead. */
		DO(Dee_function_generator_vlinear(self, argc, true)); /* func, this, kw, [args...], argv */
		DO(Dee_function_generator_vlrot(self, argc + 4));     /* this, kw, [args...], argv, func */
		DO(Dee_function_generator_vlrot(self, argc + 4));     /* kw, [args...], argv, func, this */
		DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* kw, [args...], argv, func, this, argc */
		DO(Dee_function_generator_vlrot(self, 4));            /* kw, [args...], func, this, argc, argv */
		DO(Dee_function_generator_vlrot(self, argc + 5));     /* [args...], func, this, argc, argv, kw */
		if (Dee_memloc_isnull(Dee_function_generator_vtop(self))) {
			DO(Dee_function_generator_vpop(self));            /* [args...], func, this, argc, argv */
			DO(Dee_function_generator_vcallapi(self, &DeeObject_ThisCall, VCALLOP_CC_RAWINT, 4)); /* [args...], UNCHECKED(result) */
		} else {
			DO(Dee_function_generator_vcallapi(self, &DeeObject_ThisCallKw, VCALLOP_CC_RAWINT, 5)); /* [args...], UNCHECKED(result) */
		}
		--true_argc; /* Because "this" was already popped */
	} else {
		DO(Dee_function_generator_vrrot(self, true_argc + 1)); /* func, kw, [args...] */
		/* TODO: If generating the linear version of `true_argc' is much more complicated
		 *       than doing the same for `true_argc - 1', then encode as `DeeObject_ThisCall()'
		 *       instead. */
		DO(Dee_function_generator_vlinear(self, true_argc, true)); /* func, kw, [args...], argv */
		DO(Dee_function_generator_vlrot(self, true_argc + 2));     /* func, [args...], argv, kw */
		DO(Dee_function_generator_vlrot(self, true_argc + 3));     /* [args...], argv, kw, func */
		DO(Dee_function_generator_vrrot(self, 3));                 /* [args...], func, argv, kw */
		DO(Dee_function_generator_vpush_immSIZ(self, true_argc));  /* [args...], func, argv, kw, true_argc */
		DO(Dee_function_generator_vrrot(self, 3));                 /* [args...], func, true_argc, argv, kw */
		if (Dee_memloc_isnull(Dee_function_generator_vtop(self))) {
			DO(Dee_function_generator_vpop(self));                 /* [args...], func, true_argc, argv */
			DO(Dee_function_generator_vcallapi(self, &DeeObject_Call, VCALLOP_CC_RAWINT, 3)); /* [args...], UNCHECKED(result) */
		} else {
			DO(Dee_function_generator_vcallapi(self, &DeeObject_CallKw, VCALLOP_CC_RAWINT, 4)); /* [args...], UNCHECKED(result) */
		}
	}                                                          /* [args...], UNCHECKED(result) */
	return vpop_args_before_unchecked_result(self, true_argc); /* UNCHECKED(result) */
err:
	return -1;
}

/* this -> wrapper_object
 * @param: method: When `wrapper_type == DeeKwObjMethod_Type', this must be `Dee_kwobjmethod_t'
 * @param: wrapper_type: Either `DeeObjMethod_Type' or `DeeKwObjMethod_Type'
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
vnew_ObjMethod(struct Dee_function_generator *__restrict self,
               Dee_objmethod_t method,
               DeeTypeObject *__restrict wrapper_type) {
	ASSERT(wrapper_type == &DeeObjMethod_Type ||
	       wrapper_type == &DeeKwObjMethod_Type);
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* TODO: Inline the call to `DeeObjMethod_New()' / `DeeKwObjMethod_New()' */
	}
	DO(Dee_function_generator_vpush_addr(self, (void const *)method)); /* this, method */
	DO(Dee_function_generator_vswap(self));                            /* method, this */
	return Dee_function_generator_vcallapi(self,
	                                       wrapper_type == &DeeObjMethod_Type ? (void const *)&DeeObjMethod_New
	                                                                          : (void const *)&DeeKwObjMethod_New,
	                                       VCALLOP_CC_OBJECT, 2);
err:
	return -1;
}

/* this, attr -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopgetattr_constattr(struct Dee_function_generator *__restrict self,
                     struct attr_info const *__restrict attr) {
	switch (attr->ai_type) {

	case ATTR_TYPE_CUSTOM: {
		struct type_attr const *item = attr->ai_value.v_custom;
		if (item->tp_getattr)
			return Dee_function_generator_vcallapi(self, item->tp_getattr, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ATTR_TYPE_ATTR:
		DO(Dee_function_generator_vpop(self)); /* this */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                  attr->ai_value.v_attr, true);

	case ATTR_TYPE_METHOD: {
		/* Return a `DeeObjMethod_Type' / `DeeKwObjMethod_Type' wrapper */
		struct type_method const *item = attr->ai_value.v_method;
		Dee_objmethod_t method = item->m_func;
		DeeTypeObject *wrapper_type = &DeeObjMethod_Type;
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS)
			wrapper_type = &DeeKwObjMethod_Type;
		DO(Dee_function_generator_vpop(self)); /* this */
		return vnew_ObjMethod(self, method, wrapper_type);
	}	break;

	case ATTR_TYPE_GETSET: {
		struct docinfo di;
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_get == NULL)
			break;
		di.di_doc = item->gs_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl; /* this, attr */
		DO(Dee_function_generator_vpop(self));      /* this */
		return vcall_getmethod_unchecked(self, item->gs_get, &di);
	}	break;

	case ATTR_TYPE_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;        /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_type_member(self, (DeeTypeObject *)attr->ai_decl, item, true);
	}	break;

	case ATTR_TYPE_INSTANCE_ATTR: {
		/* Wrapper for producing `DeeInstanceMember_Type' / `DeeInstanceMethod_Type' / `DeeProperty_Type' */
		/* XXX: This can also be implemented as inline, potentially even as a constant expression! */
	}	break;

	case ATTR_TYPE_INSTANCE_METHOD: {
		/* Wrapper for producing `DeeClsMethod_Type' / `DeeKwClsMethod_Type' */
		struct type_method const *item = attr->ai_value.v_instance_method;
		DREF DeeObject *value;
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS) {
			value = DeeKwClsMethod_New((DeeTypeObject *)attr->ai_decl,
			                           (Dee_kwobjmethod_t)(void const *)item->m_func);
		} else {
			value = DeeClsMethod_New((DeeTypeObject *)attr->ai_decl, item->m_func);
		}
		if unlikely(!value)
			goto err;
		value = Dee_function_generator_inlineref(self, value);
		if unlikely(!value)
			goto err;                          /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, value);
	}	break;

	case ATTR_TYPE_INSTANCE_GETSET: {
		/* Wrapper for producing `DeeClsProperty_Type' */
		struct type_getset const *item = attr->ai_value.v_instance_getset;
		DREF DeeObject *value;
		value = DeeClsProperty_New((DeeTypeObject *)attr->ai_decl,
		                           item->gs_get,
		                           item->gs_del,
		                           item->gs_set);
		if unlikely(!value)
			goto err;
		value = Dee_function_generator_inlineref(self, value);
		if unlikely(!value)
			goto err;                          /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, value);
	}	break;

	case ATTR_TYPE_INSTANCE_MEMBER: {
		/* Wrapper for producing `DeeClsMember_Type' */
		struct type_member const *item = attr->ai_value.v_instance_member;
		DREF DeeObject *value;
		value = DeeClsMember_New((DeeTypeObject *)attr->ai_decl, item);
		if unlikely(!value)
			goto err;
		value = Dee_function_generator_inlineref(self, value);
		if unlikely(!value)
			goto err;                          /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, value);
	}	break;

	case ATTR_TYPE_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
			/* TODO: Inline the behavior of `DeeModule_GetAttrSymbol()'! */
			/* TODO: Always do the inline if it results in us being able to propagate a constant! */
		}                                                 /* this, attr */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vpush_addr(self, sym)); /* this, sym */
		return Dee_function_generator_vcallapi(self, &DeeModule_GetAttrSymbol, VCALLOP_CC_OBJECT, 2);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}

/* this, attr -> result
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopboundattr_constattr(struct Dee_function_generator *__restrict self,
                       struct attr_info const *__restrict attr) {
	switch (attr->ai_type) {

	case ATTR_TYPE_CUSTOM:
		/* XXX: Inline a call to tp_getattr, then check for UnboundAttribute on error? */
		break;

	case ATTR_TYPE_ATTR:
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vbound_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                   attr->ai_value.v_attr);

	case ATTR_TYPE_GETSET: {
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_bound == NULL)
			break;
		return vcall_boundmethod(self, item->gs_bound);
	}	break;

	case ATTR_TYPE_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;        /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vbound_type_member(self, item);
	}	break;

	case ATTR_TYPE_METHOD:
	case ATTR_TYPE_INSTANCE_ATTR:
	case ATTR_TYPE_INSTANCE_METHOD:
	case ATTR_TYPE_INSTANCE_GETSET:
	case ATTR_TYPE_INSTANCE_MEMBER:
		DO(Dee_function_generator_vpop(self)); /* this */
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, Dee_True);

	case ATTR_TYPE_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
			/* TODO: Inline the behavior of `DeeModule_BoundAttrSymbol()'! */
		}                                                 /* this, attr */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vpush_addr(self, sym)); /* this, sym */
		DO(Dee_function_generator_vcallapi(self, &DeeModule_BoundAttrSymbol, VCALLOP_CC_M1INT, 2));
		DO(Dee_function_generator_vdirect(self, 1));
		ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
		Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
		return 0;
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}

/* this, attr -> N/A
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopdelattr_constattr(struct Dee_function_generator *__restrict self,
                     struct attr_info const *__restrict attr) {
	switch (attr->ai_type) {

	case ATTR_TYPE_CUSTOM: {
		struct type_attr const *item = attr->ai_value.v_custom;
		if (item->tp_delattr)
			return Dee_function_generator_vcallapi(self, item->tp_delattr, VCALLOP_CC_INT, 2);
	}	break;

	case ATTR_TYPE_ATTR:
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vdel_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                 attr->ai_value.v_attr);

	case ATTR_TYPE_GETSET: {
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_del == NULL)
			break;
		DO(Dee_function_generator_vpop(self)); /* this */
		return vcall_delmethod(self, item->gs_del);
	}	break;

	case ATTR_TYPE_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;        /* this, attr */
		DO(Dee_function_generator_vpop(self)); /* this */
		return Dee_function_generator_vdel_type_member(self, item);
	}	break;

	case ATTR_TYPE_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
			/* TODO: Inline the behavior of `DeeModule_DelAttrSymbol()'! */
		}                                                 /* this, attr */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vpush_addr(self, sym)); /* this, sym */
		return Dee_function_generator_vcallapi(self, &DeeModule_DelAttrSymbol, VCALLOP_CC_INT, 2);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}

/* this, attr, value -> N/A
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vopsetattr_constattr(struct Dee_function_generator *__restrict self,
                     struct attr_info const *__restrict attr) {
	switch (attr->ai_type) {

	case ATTR_TYPE_CUSTOM: {
		struct type_attr const *item = attr->ai_value.v_custom;
		if (item->tp_delattr)
			return Dee_function_generator_vcallapi(self, item->tp_delattr, VCALLOP_CC_INT, 2);
	}	break;

	case ATTR_TYPE_ATTR:                        /* this, attr, value */
		DO(Dee_function_generator_vswap(self)); /* this, value, attr */
		DO(Dee_function_generator_vpop(self));  /* this, value */
		return Dee_function_generator_vpop_instance_attr(self, (DeeTypeObject *)attr->ai_decl,
		                                                 attr->ai_value.v_attr);

	case ATTR_TYPE_GETSET: {
		struct type_getset const *item;
		item = attr->ai_value.v_getset;
		if (item->gs_set == NULL)
			break;                              /* this, attr, value */
		DO(Dee_function_generator_vswap(self)); /* this, value, attr */
		DO(Dee_function_generator_vpop(self));  /* this, value */
		return vcall_setmethod(self, item->gs_set);
	}	break;

	case ATTR_TYPE_MEMBER: {
		struct type_member const *item;
		item = attr->ai_value.v_member;         /* this, attr, value */
		DO(Dee_function_generator_vswap(self)); /* this, value, attr */
		DO(Dee_function_generator_vpop(self));  /* this, value */
		return Dee_function_generator_vpop_type_member(self, item);
	}	break;

	case ATTR_TYPE_MODSYM: {
		/* Access a module symbol */
		struct Dee_module_symbol const *sym = attr->ai_value.v_modsym;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
			/* TODO: Inline the behavior of `DeeModule_SetAttrSymbol()'! */
		}                                                 /* this, attr, value */
		DO(Dee_function_generator_vswap(self));           /* this, value, attr */
		DO(Dee_function_generator_vpop(self));            /* this, value */
		DO(Dee_function_generator_vpush_addr(self, sym)); /* this, value, sym */
		DO(Dee_function_generator_vswap(self));           /* this, sym, value */
		return Dee_function_generator_vcallapi(self, &DeeModule_SetAttrSymbol, VCALLOP_CC_INT, 3);
	}	break;

	default: break;
	}
	return 1;
err:
	return -1;
}


/* this, attr, [args...], kw -> UNCHECKED(result)
 * @return: 0 : Optimization successfully applied
 * @return: 1 : No dedicated optimization available for `attr'
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
vopcallattrkw_constattr_unchecked(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t argc,
                                  struct attr_info const *__restrict attr) {
	switch (attr->ai_type) {

	case ATTR_TYPE_ATTR:
		/* NOTE: In this case, we're allowed to assume that "this" is an instance of "attr->ai_decl" */
		DO(Dee_function_generator_vlrot(self, argc + 2)); /* this, [args...], kw, attr */
		DO(Dee_function_generator_vpop(self));            /* this, [args...], kw */
		return Dee_function_generator_vcall_instance_attrkw_unchecked(self, (DeeTypeObject *)attr->ai_decl,
		                                                              attr->ai_value.v_attr, argc);

	case ATTR_TYPE_METHOD: {
		struct docinfo di;
		struct type_method const *item;
		item      = attr->ai_value.v_method;
		di.di_doc = item->m_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl;
		DO(Dee_function_generator_vlrot(self, argc + 2)); /* this, [args...], kw, attr */
		DO(Dee_function_generator_vpop(self));            /* this, [args...], kw */
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS)
			return vcall_kwobjmethod_unchecked(self, (Dee_kwobjmethod_t)(void const *)item->m_func, argc, &di);
		DO(vpop_empty_kwds(self)); /* this, [args...] */
		return vcall_objmethod_unchecked(self, item->m_func, argc, &di);
	}	break;

	case ATTR_TYPE_INSTANCE_ATTR: {
		uint16_t callback_addr;
		struct class_attribute const *item;
		if (argc < 1)
			break;
		--argc; /* type, attr, this, [args...], kw */
		item = attr->ai_value.v_instance_attr;
		/* Behavior here mirrors `DeeClass_CallInstanceAttributeKw()' */
		if (!(item->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
			if (argc != 0)
				break;
			/* XXX: "kw" here doesn't need to be empty. It is also allowed to be "thisarg: foo" */
			DO(vpop_empty_kwds(self));                 /* type, attr, this */
			DO(Dee_function_generator_vrrot(self, 3)); /* this, type, attr */
			DO(Dee_function_generator_vpop(self));     /* this, type */
			DO(Dee_function_generator_vpop(self));     /* this */
			DO(Dee_function_generator_vassert_type(self, (DeeTypeObject *)attr->ai_decl)); /* this */
			/* XXX: Look at current instruction to see if the result needs to be a reference. */
			return Dee_function_generator_vpush_instance_attr(self, (DeeTypeObject *)attr->ai_decl, item, true);
		}                                                 /* type, attr, this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 4)); /* attr, this, [args...], kw, type */
		DO(Dee_function_generator_vpop(self));            /* attr, this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 3)); /* this, [args...], kw, attr */
		DO(Dee_function_generator_vpop(self));            /* this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 2)); /* [args...], kw, this */
		DO(Dee_function_generator_vassert_type(self, (DeeTypeObject *)attr->ai_decl)); /* [args...], kw, this */
		DO(Dee_function_generator_vpush_const(self, attr->ai_decl)); /* [args...], kw, this, decl_type */
		callback_addr = item->ca_addr;
#if CLASS_GETSET_GET != 0
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			callback_addr += CLASS_GETSET_GET;
#endif /* CLASS_GETSET_GET != 0 */
		DO(Dee_function_generator_vpush_imember(self, callback_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], kw, func */
		DO(Dee_function_generator_vlrot(self, argc + 2)); /* func, [args...], kw */
		return Dee_function_generator_vopcallkw_unchecked(self, argc); /* UNCHECKED(result) */
	}	break;

	case ATTR_TYPE_INSTANCE_METHOD: {
		struct docinfo di;
		struct type_method const *item;
		if (argc < 1)
			break;
		item      = attr->ai_value.v_instance_method;
		di.di_doc = item->m_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl;
		--argc;                                           /* type, attr, this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 4)); /* attr, this, [args...], kw, type */
		DO(Dee_function_generator_vpop(self));            /* attr, this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 3)); /* this, [args...], kw, attr */
		DO(Dee_function_generator_vpop(self));            /* this, [args...], kw */
		DO(Dee_function_generator_vlrot(self, argc + 2)); /* [args...], kw, this */
		DO(Dee_function_generator_vassert_type(self, (DeeTypeObject *)attr->ai_decl)); /* [args...], kw, this */
		DO(Dee_function_generator_vrrot(self, argc + 2)); /* this, [args...], kw */
		if (item->m_flag & Dee_TYPE_METHOD_FKWDS)
			return vcall_kwobjmethod_unchecked(self, (Dee_kwobjmethod_t)(void const *)item->m_func, argc, &di);
		DO(vpop_empty_kwds(self)); /* this, [args...] */
		return vcall_objmethod_unchecked(self, item->m_func, argc, &di);
	}	break;

	case ATTR_TYPE_INSTANCE_GETSET: {
		struct docinfo di;
		struct type_getset const *item;
		if (argc != 1)
			break;
		item = attr->ai_value.v_instance_getset;
		if (item->gs_get == NULL)
			break;
		di.di_doc = item->gs_doc;
		di.di_typ = (DeeTypeObject *)attr->ai_decl;       /* type, attr, this, kw */
		DO(vpop_empty_kwds(self));                        /* type, attr, this */
		DO(Dee_function_generator_vrrot(self, 3));        /* this, type, attr */
		DO(Dee_function_generator_vpop(self));            /* this, type */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vassert_type(self, (DeeTypeObject *)attr->ai_decl)); /* this */
		return vcall_getmethod_unchecked(self, item->gs_get, &di);
	}	break;

	case ATTR_TYPE_INSTANCE_MEMBER: {
		struct type_member const *item;
		if (argc != 1)
			break;
		item = attr->ai_value.v_instance_member;
		DO(vpop_empty_kwds(self));                        /* type, attr, this */
		DO(Dee_function_generator_vrrot(self, 3));        /* this, type, attr */
		DO(Dee_function_generator_vpop(self));            /* this, type */
		DO(Dee_function_generator_vpop(self));            /* this */
		DO(Dee_function_generator_vassert_type(self, (DeeTypeObject *)attr->ai_decl)); /* this */
		/* XXX: Look at current instruction to see if the result needs to be a reference. */
		return Dee_function_generator_vpush_type_member(self, (DeeTypeObject *)attr->ai_decl, item, true);
	}	break;

	default: {
		/* Fallback: try to load the attribute as-is, and then call it */
		int result;
		DO(Dee_function_generator_vlrot(self, argc + 3)); /* attr, [args...], kw, this */
		DO(Dee_function_generator_vlrot(self, argc + 3)); /* [args...], kw, this, attr */
		result = vopgetattr_constattr(self, attr);
		if (result <= 0) {
			if unlikely(result < 0)
				goto err;                                                  /* [args...], kw, func */
			DO(Dee_function_generator_vrrot(self, argc + 2));              /* func, [args...], kw */
			return Dee_function_generator_vopcallkw_unchecked(self, argc); /* UNCHECKED(result) */
		}
	}	break;

	}
	return 1;
err:
	return -1;
}

/* this, attr, [args...], kw -> UNCHECKED(result) */
INTERN WUNUSED NONNULL((1)) int DCALL
impl_vopcallattrkw_unchecked(struct Dee_function_generator *__restrict self,
                             Dee_vstackaddr_t argc) {
	struct attr_info attr;
	DeeTypeObject *this_type;
	struct Dee_memloc *thisloc;
	struct Dee_memloc *attrloc;
	if unlikely(self->fg_state->ms_stackc < (argc + 3))
		return err_illegal_stack_effect();

	/* Generate code to assert that "attr" is a string. */
	DO(Dee_function_generator_vlrot(self, argc + 2));                     /* this, [args...], kw, attr */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeString_Type)); /* this, [args...], kw, attr */
	DO(Dee_function_generator_vrrot(self, argc + 2));                     /* this, attr, [args...], kw */

	/* Normalize the "this" and "attr" memory locations. */
	DO(Dee_function_generator_state_unshare(self));
	attrloc = Dee_function_generator_vtop(self) - (argc + 1);
	if (!MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph)) {
		DO(Dee_function_generator_gdirect(self, attrloc));
		attrloc = Dee_function_generator_vtop(self) - (argc + 1);
	}
	thisloc = attrloc - 1;
	if (!MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph)) {
		DO(Dee_function_generator_gdirect(self, thisloc));
		attrloc = Dee_function_generator_vtop(self) - (argc + 1);
		thisloc = attrloc - 1;
	}
	ASSERT(MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memloc_typeof(thisloc);
	if (this_type != NULL && attrloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject *this_value_or_null = NULL;
		DeeStringObject *attr_obj = (DeeStringObject *)attrloc->ml_value.v_const;
		ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
		if (thisloc->ml_type == MEMLOC_TYPE_CONST)
			this_value_or_null = thisloc->ml_value.v_const;
		if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, attr_obj, &attr)) {
			int temp = vopcallattrkw_constattr_unchecked(self, argc, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Inline the call to query the attribute */
	if (this_type != NULL && this_type->tp_attr) {
		int temp;
		attr.ai_type = ATTR_TYPE_CUSTOM;
		attr.ai_decl = (DeeObject *)this_type;
		attr.ai_value.v_custom = this_type->tp_attr;
		temp = vopcallattrkw_constattr_unchecked(self, argc, &attr);
		if (temp <= 0)
			return temp; /* Optimization applied, or error */
	}

	/* Fallback: perform a generic CallAttr operation at runtime. */
	DO(Dee_function_generator_vrrot(self, argc + 1));     /* this, attr, kw, [args...] */
	DO(Dee_function_generator_vlinear(self, argc, true)); /* this, attr, kw, [args...], argv */
	DO(Dee_function_generator_vlrot(self, argc + 4));     /* attr, kw, [args...], argv, this */
	DO(Dee_function_generator_vlrot(self, argc + 4));     /* kw, [args...], argv, this, attr */
	DO(Dee_function_generator_vpush_immSIZ(self, argc));  /* kw, [args...], argv, this, attr, argc */
	DO(Dee_function_generator_vlrot(self, 4));            /* kw, [args...], this, attr, argc, argv */
	DO(Dee_function_generator_vlrot(self, argc + 5));     /* [args...], this, attr, argc, argv, kw */
	if (Dee_memloc_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self));            /* [args...], this, attr, argc, argv */
		DO(Dee_function_generator_vcallapi(self, &DeeObject_CallAttr, VCALLOP_CC_RAWINT, 4)); /* [args...], UNCHECKED(result) */
	} else {
		DO(Dee_function_generator_vcallapi(self, &DeeObject_CallAttrKw, VCALLOP_CC_RAWINT, 5)); /* [args...], UNCHECKED(result) */
	}
	return vpop_args_before_unchecked_result(self, argc); /* UNCHECKED(result) */
err:
	return -1;
}

/* func, [args...], kw -> UNCHECKED(result) -- Invoke `DeeObject_CallKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallkw_unchecked(struct Dee_function_generator *__restrict self,
                                           Dee_vstackaddr_t argc) {
	return impl_vopcallkw_unchecked(self, argc, false);
}

/* func, args, kw -> UNCHECKED(result) -- Invoke `DeeObject_CallTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcalltuplekw_unchecked(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimizations for MEMLOC_TYPE_CONST functions with certain types. (e.g. `DeeObjMethodObject') */
	DO(Dee_function_generator_vswap(self));                              /* func, kw, args */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeTuple_Type)); /* func, kw, args */
	DO(Dee_function_generator_vswap(self));                              /* func, args, kw */
	if (Dee_memloc_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self)); /* func, args */
		return Dee_function_generator_vcallapi(self, &DeeObject_CallTuple, VCALLOP_CC_RAWINT, 2);
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_CallTupleKw, VCALLOP_CC_RAWINT, 3);
err:
	return -1;
}

/* func, this, [args...], kw -> UNCHECKED(result) -- Invoke `DeeObject_ThisCallKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscallkw_unchecked(struct Dee_function_generator *__restrict self,
                                               Dee_vstackaddr_t argc) {
	return impl_vopcallkw_unchecked(self, argc + 1, true);
}

/* func, this, args, kw -> UNCHECKED(result) -- Invoke `DeeObject_ThisCallTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscalltuplekw_unchecked(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimizations for MEMLOC_TYPE_CONST functions with certain types. (e.g. `DeeClsMethodObject') */
	/* TODO: Optimizations when `type(func)' is known by skipping operator resolution and directly invoking the call-operator */
	DO(Dee_function_generator_vswap(self));                              /* func, this, kw, args */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeTuple_Type)); /* func, this, kw, args */
	DO(Dee_function_generator_vswap(self));                              /* func, this, args, kw */
	if (Dee_memloc_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self)); /* func, this, args */
		return Dee_function_generator_vcallapi(self, &DeeObject_ThisCallTuple, VCALLOP_CC_RAWINT, 3);
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_ThisCallTupleKw, VCALLOP_CC_RAWINT, 4);
err:
	return -1;
}

/* this, attr, [args...], kw -> UNCHECKED(result) -- Invoke `DeeObject_CallAttrKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrkw_unchecked(struct Dee_function_generator *__restrict self,
                                               Dee_vstackaddr_t argc) {
	return impl_vopcallattrkw_unchecked(self, argc);
}

/* this, attr, args, kw -> UNCHECKED(result) -- Invoke `DeeObject_CallAttrTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrtuplekw_unchecked(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimization or when `attr' and the type of `this' is known:
	 * >> return "a,b,c".split(x); // Inline the actual call to `string_split()',
	 * >>                          // bypassing the complete attribute lookup */
	DO(Dee_function_generator_vlrot(self, 3));                            /* this, args, kw, attr */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeString_Type)); /* this, args, kw, attr */
	DO(Dee_function_generator_vlrot(self, 3));                            /* this, kw, attr, args */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeTuple_Type));  /* this, kw, attr, args */
	DO(Dee_function_generator_vlrot(self, 3));                            /* this, attr, args, kw */
	if (Dee_memloc_isnull(Dee_function_generator_vtop(self))) {
		DO(Dee_function_generator_vpop(self)); /* this, attr, args */
		return Dee_function_generator_vcallapi(self, &DeeObject_CallAttrTuple, VCALLOP_CC_RAWINT, 3);
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_CallAttrTupleKw, VCALLOP_CC_RAWINT, 4);
err:
	return -1;
}



/* func, [args...] -> [args...], UNCHECKED(result) -- Invoke `DeeObject_Call()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcall_unchecked(struct Dee_function_generator *__restrict self,
                                         Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0) /* func, [args...], kw=NULL */
		result = Dee_function_generator_vopcallkw_unchecked(self, argc);
	return result;
}

/* func, args -> UNCHECKED(result) -- Invoke `DeeObject_CallTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcalltuple_unchecked(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0) /* func, args, kw=NULL */
		result = Dee_function_generator_vopcalltuplekw_unchecked(self);
	return result;
}

/* func, this, [args...] -> [args...], UNCHECKED(result) -- Invoke `DeeObject_ThisCall()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscall_unchecked(struct Dee_function_generator *__restrict self,
                                             Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0) /* func, this, [args...], kw=NULL */
		result = Dee_function_generator_vopthiscallkw_unchecked(self, argc);
	return result;
}

/* func, this, args -> UNCHECKED(result) -- Invoke `DeeObject_ThisCallTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscalltuple_unchecked(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0) /* func, this, args, kw=NULL */
		result = Dee_function_generator_vopthiscalltuplekw_unchecked(self);
	return result;
}

/* this, attr, [args...] -> [args...], UNCHECKED(result) -- Invoke `DeeObject_CallAttr()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattr_unchecked(struct Dee_function_generator *__restrict self,
                                             Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0) /* this, attr, [args...], kw=NULL */
		result = Dee_function_generator_vopcallattrkw_unchecked(self, argc);
	return result;
}

/* this, attr, args -> UNCHECKED(result) -- Invoke `DeeObject_CallAttrTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrtuple_unchecked(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0) /* this, attr, args, kw=NULL */
		result = Dee_function_generator_vopcallattrtuplekw_unchecked(self);
	return result;
}

/* func, [args...] -> result -- Invoke `DeeObject_Call()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcall(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vopcall_unchecked(self, argc);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, [args...], kw -> result -- Invoke `DeeObject_CallKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallkw(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vopcallkw_unchecked(self, argc);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, args -> result -- Invoke `DeeObject_CallTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcalltuple(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vopcalltuple_unchecked(self);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, args, kw -> result -- Invoke `DeeObject_CallTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcalltuplekw(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vopcalltuplekw_unchecked(self);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, this, [args...] -> result -- Invoke `DeeObject_ThisCall()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscall(struct Dee_function_generator *__restrict self,
                                   Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vopthiscall_unchecked(self, argc);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, this, [args...], kw -> result -- Invoke `DeeObject_ThisCallKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscallkw(struct Dee_function_generator *__restrict self,
                                     Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vopthiscallkw_unchecked(self, argc);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, this, args -> result -- Invoke `DeeObject_ThisCallTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscalltuple(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vopthiscalltuple_unchecked(self);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* func, this, args, kw -> result -- Invoke `DeeObject_ThisCallTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopthiscalltuplekw(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vopthiscalltuplekw_unchecked(self);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}


/* this, attr, [args...] -> result -- Invoke `DeeObject_CallAttr()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattr(struct Dee_function_generator *__restrict self,
                                   Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vopcallattr_unchecked(self, argc);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* this, attr, [args...], kw -> result -- Invoke `DeeObject_CallAttrKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrkw(struct Dee_function_generator *__restrict self,
                                     Dee_vstackaddr_t argc) {
	int result = Dee_function_generator_vopcallattrkw_unchecked(self, argc);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* this, attr, args -> result -- Invoke `DeeObject_CallAttrTuple()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrtuple(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vopcallattrtuple_unchecked(self);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}

/* this, attr, args, kw -> result -- Invoke `DeeObject_CallAttrTupleKw()' and push the result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopcallattrtuplekw(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vopcallattrtuplekw_unchecked(self);
	if likely(result == 0) /* UNCHECKED(result) */
		result = Dee_function_generator_vcheckobj(self); /* result */
	return result;
}


/* this, attr -> result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopgetattr(struct Dee_function_generator *__restrict self) {
	struct attr_info attr;
	DeeTypeObject *this_type;
	struct Dee_memloc *thisloc;
	struct Dee_memloc *attrloc;
	DO(Dee_function_generator_vdirect(self, 2));                          /* this, attr */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeString_Type)); /* this, attr */
	DO(Dee_function_generator_state_unshare(self));

	/* Load value locations. */
	attrloc = Dee_function_generator_vtop(self);
	thisloc = attrloc - 1;
	ASSERT(MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memloc_typeof(thisloc);
	if (this_type != NULL && attrloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject *this_value_or_null = NULL;
		DeeStringObject *attr_obj = (DeeStringObject *)attrloc->ml_value.v_const;
		ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
		if (thisloc->ml_type == MEMLOC_TYPE_CONST)
			this_value_or_null = thisloc->ml_value.v_const;
		if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, attr_obj, &attr)) {
			int temp = vopgetattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Inline the call to query the attribute */
	if (this_type != NULL && this_type->tp_attr) {
		int temp;
		attr.ai_type = ATTR_TYPE_CUSTOM;
		attr.ai_decl = (DeeObject *)this_type;
		attr.ai_value.v_custom = this_type->tp_attr;
		temp = vopgetattr_constattr(self, &attr);
		if (temp <= 0)
			return temp; /* Optimization applied, or error */
	}

	/* Fallback: emit a runtime attribute lookup. */
	return Dee_function_generator_vcallapi(self, &DeeObject_GetAttr, VCALLOP_CC_OBJECT, 2);
err:
	return -1;
}

/* this, attr -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopboundattr(struct Dee_function_generator *__restrict self) {
	struct attr_info attr;
	DeeTypeObject *this_type;
	struct Dee_memloc *thisloc;
	struct Dee_memloc *attrloc;
	DO(Dee_function_generator_vdirect(self, 2));                          /* this, attr */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeString_Type)); /* this, attr */
	DO(Dee_function_generator_state_unshare(self));

	/* Load value locations. */
	attrloc = Dee_function_generator_vtop(self);
	thisloc = attrloc - 1;
	ASSERT(MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memloc_typeof(thisloc);
	if (this_type != NULL && attrloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject *this_value_or_null = NULL;
		DeeStringObject *attr_obj = (DeeStringObject *)attrloc->ml_value.v_const;
		ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
		if (thisloc->ml_type == MEMLOC_TYPE_CONST)
			this_value_or_null = thisloc->ml_value.v_const;
		if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, attr_obj, &attr)) {
			int temp = vopboundattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Inline the call to query the attribute */
	if (this_type != NULL && this_type->tp_attr) {
		int temp;
		attr.ai_type = ATTR_TYPE_CUSTOM;
		attr.ai_decl = (DeeObject *)this_type;
		attr.ai_value.v_custom = this_type->tp_attr;
		temp = vopboundattr_constattr(self, &attr);
		if (temp <= 0)
			return temp; /* Optimization applied, or error */
	}

	/* Fallback: emit a runtime attribute lookup. */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_BoundAttr, VCALLOP_CC_M1INT, 2));
	DO(Dee_function_generator_vdirect(self, 1));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
	Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* this, attr -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopdelattr(struct Dee_function_generator *__restrict self) {
	struct attr_info attr;
	DeeTypeObject *this_type;
	struct Dee_memloc *thisloc;
	struct Dee_memloc *attrloc;
	DO(Dee_function_generator_vdirect(self, 2));                          /* this, attr */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeString_Type)); /* this, attr */
	DO(Dee_function_generator_state_unshare(self));

	/* Load value locations. */
	attrloc = Dee_function_generator_vtop(self);
	thisloc = attrloc - 1;
	ASSERT(MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memloc_typeof(thisloc);
	if (this_type != NULL && attrloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject *this_value_or_null = NULL;
		DeeStringObject *attr_obj = (DeeStringObject *)attrloc->ml_value.v_const;
		ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
		if (thisloc->ml_type == MEMLOC_TYPE_CONST)
			this_value_or_null = thisloc->ml_value.v_const;
		if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, attr_obj, &attr)) {
			int temp = vopdelattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Inline the call to query the attribute */
	if (this_type != NULL && this_type->tp_attr) {
		int temp;
		attr.ai_type = ATTR_TYPE_CUSTOM;
		attr.ai_decl = (DeeObject *)this_type;
		attr.ai_value.v_custom = this_type->tp_attr;
		temp = vopdelattr_constattr(self, &attr);
		if (temp <= 0)
			return temp; /* Optimization applied, or error */
	}

	/* Fallback: emit a runtime attribute lookup. */
	return Dee_function_generator_vcallapi(self, &DeeObject_DelAttr, VCALLOP_CC_INT, 2);
err:
	return -1;
}

/* this, attr, value -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsetattr(struct Dee_function_generator *__restrict self) {
	struct attr_info attr;
	DeeTypeObject *this_type;
	struct Dee_memloc *thisloc;
	struct Dee_memloc *attrloc;
	if unlikely(self->fg_state->ms_stackc < 3)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_vswap(self));                               /* this, value, attr */
	DO(Dee_function_generator_vassert_type_exact(self, &DeeString_Type)); /* this, value, attr */
	DO(Dee_function_generator_vswap(self));                               /* this, attr, value */
	DO(Dee_function_generator_state_unshare(self));

	/* Normalize the "this" and "attr" memory locations. */
	DO(Dee_function_generator_state_unshare(self));
	attrloc = Dee_function_generator_vtop(self) - 1;
	if (!MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph)) {
		DO(Dee_function_generator_gdirect(self, attrloc));
		attrloc = Dee_function_generator_vtop(self) - 1;
	}
	thisloc = attrloc - 1;
	if (!MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph)) {
		DO(Dee_function_generator_gdirect(self, thisloc));
		attrloc = Dee_function_generator_vtop(self) - 1;
		thisloc = attrloc - 1;
	}
	ASSERT(MEMLOC_VMORPH_ISDIRECT(thisloc->ml_vmorph));
	ASSERT(MEMLOC_VMORPH_ISDIRECT(attrloc->ml_vmorph));

	/* Check if the type of "this", as well as the attribute being accessed is known.
	 * If they are, then we might be able to inline the attribute access! */
	this_type = Dee_memloc_typeof(thisloc);
	if (this_type != NULL && attrloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject *this_value_or_null = NULL;
		DeeStringObject *attr_obj = (DeeStringObject *)attrloc->ml_value.v_const;
		ASSERT_OBJECT_TYPE_EXACT(attr_obj, &DeeString_Type);
		if (thisloc->ml_type == MEMLOC_TYPE_CONST)
			this_value_or_null = thisloc->ml_value.v_const;
		if (DeeObject_TFindAttrInfo(this_type, this_value_or_null, attr_obj, &attr)) {
			int temp = vopsetattr_constattr(self, &attr);
			if (temp <= 0)
				return temp; /* Optimization applied, or error */
		}
	}

	/* Inline the call to query the attribute */
	if (this_type != NULL && this_type->tp_attr) {
		int temp;
		attr.ai_type = ATTR_TYPE_CUSTOM;
		attr.ai_decl = (DeeObject *)this_type;
		attr.ai_value.v_custom = this_type->tp_attr;
		temp = vopsetattr_constattr(self, &attr);
		if (temp <= 0)
			return temp; /* Optimization applied, or error */
	}

	/* Fallback: emit a runtime attribute lookup. */
	return Dee_function_generator_vcallapi(self, &DeeObject_SetAttr, VCALLOP_CC_INT, 3);
err:
	return -1;
}





/* seq -> [elems...] */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopunpack(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t n) {
	struct Dee_memloc *seq;
	Dee_vstackaddr_t i;
	uintptr_t cfa_offset;
	size_t alloc_size;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();

	/* Optimization when "vtop" is always "none" */
	seq = Dee_function_generator_vtop(self);
	if (Dee_memloc_isnone(seq)) {
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		for (i = 0; i < n; ++i) {
			if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
				goto err;
		}
		return 0;
	}

	alloc_size = n * sizeof(DREF DeeObject *);
	cfa_offset = Dee_memstate_hstack_find(self->fg_state, self->fg_state_hstack_res, alloc_size);
	if (cfa_offset == (uintptr_t)-1) {
		cfa_offset = Dee_memstate_hstack_alloca(self->fg_state, alloc_size);
		if unlikely(Dee_function_generator_ghstack_adjust(self, alloc_size))
			goto err;
	}
	if unlikely(Dee_function_generator_vpush_immSIZ(self, n))
		goto err; /* seq, objc */
	if unlikely(Dee_function_generator_vpush_hstack(self, cfa_offset))
		goto err; /* seq, objc, objv */
	if unlikely(Dee_function_generator_vcallapi(self, &DeeObject_Unpack, VCALLOP_CC_INT, 3))
		goto err; /* - */
	for (i = 0; i < n; ++i) {
		uintptr_t n_cfa_offset;
#ifdef HOSTASM_STACK_GROWS_DOWN
		n_cfa_offset = cfa_offset - i * sizeof(DREF DeeObject *);
#else /* HOSTASM_STACK_GROWS_DOWN */
		n_cfa_offset = cfa_offset + i * sizeof(DREF DeeObject *);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		if unlikely(Dee_function_generator_vpush_hstackind(self, n_cfa_offset, 0))
			goto err;
		ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
		Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;
	}
	return 0;
err:
	return -1;
}


/* lhs, rhs -> result */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopconcat(struct Dee_function_generator *__restrict self) {
	/* TODO: Optimizations for known object types (see impl of `DeeObject_ConcatInherited()'). */
	if unlikely (Dee_function_generator_vswap(self))
		goto err; /* rhs, lhs */
	if unlikely (Dee_function_generator_vref(self))
		goto err; /* rhs, ref:lhs */
	if unlikely (Dee_function_generator_vswap(self))
		goto err; /* ref:lhs, rhs */
	if unlikely (Dee_function_generator_vcallapi(self, &DeeObject_ConcatInherited, VCALLOP_CC_RAWINT_KEEPARGS, 2))
		goto err; /* ([valid_if(!result)] ref:lhs), rhs, result */
	if unlikely (Dee_function_generator_gjz_except(self, Dee_function_generator_vtop(self)))
		goto err; /* ([valid_if(false)] ref:lhs), rhs, result */
	if unlikely (Dee_function_generator_vlrot(self, 3))
		goto err; /* rhs, result, ([valid_if(false)] REF:lhs) */
	ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF)); /* ... */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF;           /* rhs, result, ([valid_if(false)] lhs) */
	if unlikely (Dee_function_generator_vpop(self))
		goto err; /* rhs, result */
	if unlikely (Dee_function_generator_vswap(self))
		goto err; /* result, rhs */
	return Dee_function_generator_vpop(self); /* result */
err:
	return -1;
}

/* seq, [elems...] -> seq */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopextend(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	/* TODO: Optimizations for known object types (see impl of `DeeObject_ExtendInherited()'). */
	for (i = 0; i < n; ++i) {
		if unlikely(Dee_function_generator_vref(self))
			goto err;
		if unlikely(Dee_function_generator_vlrot(self, n))
			goto err;
	}
	if unlikely(Dee_function_generator_vlinear(self, n, true))
		goto err; /* seq, [elems...], elemv */
	if unlikely(Dee_function_generator_vlrot(self, n + 2))
		goto err; /* [elems...], elemv, seq */
	if unlikely(Dee_function_generator_vdup(self))
		goto err; /* [elems...], elemv, seq, seq */
	if unlikely(Dee_function_generator_vrrot(self, n + 3))
		goto err; /* seq, [elems...], elemv, seq */
	if unlikely(Dee_function_generator_vpush_immSIZ(self, n))
		goto err; /* seq, [elems...], elemv, seq, elemc */
	if unlikely(Dee_function_generator_vlrot(self, 3))
		goto err; /* seq, [elems...], seq, elemc, elemv */
	if unlikely(Dee_function_generator_vcallapi(self, &DeeObject_ExtendInherited, VCALLOP_CC_INT, 3))
		goto err; /* seq, [elems...] */
	for (i = 0; i < n; ++i) {
		/* In the success-case, references were inherited! */
		ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
		Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF;
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
	}
	return 0;
err:
	return -1;
}

/* ob -> type(ob) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_voptypeof(struct Dee_function_generator *__restrict self, bool ref) {
	struct Dee_memloc *obj;
	DeeTypeObject *known_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;

	/* Check if the object type is known to be a constant. */
	obj = Dee_function_generator_vtop(self);
	known_type = Dee_memloc_typeof(obj);
	if (known_type != NULL) {
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_vpush_const(self, (DeeObject *)known_type);
	}

	/* If we're not holding any kind of reference, or the caller pinky-promises that
	 * they don't need a reference because they know that "obj" stays alive long enough,
	 * then we don't need to give the type a new reference. */
	if (!ref || !Dee_memstate_hasref(self->fg_state, obj))
		return Dee_function_generator_vind(self, offsetof(DeeObject, ob_type));

	/* If the object whose type we're trying to read is a
	 * reference, then we also need a reference to the type! */
	if unlikely(Dee_function_generator_vdup(self))
		goto err; /* obj, obj */
	if unlikely(Dee_function_generator_vind(self, offsetof(DeeObject, ob_type)))
		goto err; /* obj, obj->ob_type */
	if unlikely(Dee_function_generator_vref(self))
		goto err; /* obj, ref:obj->ob_type */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* ref:obj->ob_type, obj */
	return Dee_function_generator_vpop(self); /* ref:obj->ob_type */
err:
	return -1;
}

/* ob -> ob.class */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopclassof(struct Dee_function_generator *__restrict self, bool ref) {
	struct Dee_memloc *obj;
	DeeTypeObject *known_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;

	/* Check if the object type is known to be a constant. */
	obj = Dee_function_generator_vtop(self);
	known_type = Dee_memloc_typeof(obj);
	if (known_type != NULL) {
		/* Special case: for super, we must return the embedded type */
		if (known_type == &DeeSuper_Type)
			return Dee_function_generator_vind(self, offsetof(DeeSuperObject, s_type));
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_vpush_const(self, (DeeObject *)known_type);
	}

	if unlikely(Dee_function_generator_vcallapi(self, &DeeObject_Class, VCALLOP_CC_RAWINT_KEEPARGS, 1))
		goto err; /* obj, obj.class */
	/* If we're not holding any kind of reference, or the caller pinky-promises that
	 * they don't need a reference because they know that "obj" stays alive long enough,
	 * then we don't need to give the type a new reference. */
	if (!ref || Dee_memstate_hasref(self->fg_state, Dee_function_generator_vtop(self) - 1)) {
		if unlikely(Dee_function_generator_vref(self))
			goto err; /* obj, ref:obj.class */
	}
	if unlikely(Dee_function_generator_vswap(self))
		goto err;   /* [ref]:obj.class, obj */
	return Dee_function_generator_vpop(self); /* [ref]:obj.class */
err:
	return -1;
}


/* ob -> ob.super */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsuperof(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc *obj;
	DeeTypeObject *known_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;

	/* Check if the object type is known to be a constant. */
	obj = Dee_function_generator_vtop(self);
	known_type = Dee_memloc_typeof(obj);
	if (known_type != NULL && known_type != &DeeSuper_Type) {
		if unlikely(Dee_function_generator_vpush_const(self, (DeeObject *)DeeType_Base(known_type)))
			goto err;
		return Dee_function_generator_vopsuper(self);
	}

	/* Fallback: do the super operation at runtime. */
	return Dee_function_generator_vcallapi(self, &DeeSuper_Of, VCALLOP_CC_OBJECT, 1);
err:
	return -1;
}

/* ob, type -> ob as type */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopsuper(struct Dee_function_generator *__restrict self) {
	/* XXX: It would be cool to implement this using some `MEMLOC_VMORPH_*'.
	 *      That way, it would be possible to encode `DeeObject_T*' calls in
	 *      order to more efficiently encode "super.foo" in member functions. */
	if unlikely(Dee_function_generator_vswap(self))
		goto err;
	return Dee_function_generator_vcallapi(self, &DeeSuper_New, VCALLOP_CC_OBJECT, 2);
err:
	return -1;
}



/* Helpers for accessing C-level "struct type_member". NOTE: These don't do type assertions! */

/* this -> value */
INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_vpush_type_member(struct Dee_function_generator *__restrict self, DeeTypeObject *type,
                                         struct Dee_type_member const *__restrict desc, bool ref) {
	DO(_Dee_function_generator_vpush_type_member(self, desc, ref));
	if (desc->m_doc != NULL && Dee_memloc_typeof(Dee_function_generator_vtop(self)) == NULL) {
		struct docinfo doc;
		DeeTypeObject *result_type;
		doc.di_doc = desc->m_doc;
		doc.di_typ = type;
		doc.di_mod = NULL;
		result_type = extra_return_type_from_doc(self, 0, &doc);
		if (result_type != NULL) {
			if unlikely(result_type == (DeeTypeObject *)ITER_DONE)
				goto err;
			return Dee_function_generator_vsettyp(self, result_type);
		}
	}
	return 0;
err:
	return -1;
}

/* this -> value  (doesn't look at the doc string to determine typing) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_vpush_type_member(struct Dee_function_generator *__restrict self,
                                          struct Dee_type_member const *__restrict desc, bool ref) {
	/* Behavior here mirrors `Dee_type_member_get()' */
	if (TYPE_MEMBER_ISCONST(desc)) {
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, desc->m_const);
	}

	/* Inline the set operation when possible. */
	switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)

	CASE(STRUCT_NONE):
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, Dee_None);

	CASE(STRUCT_WOBJECT):
	CASE(STRUCT_WOBJECT_OPT): {
		/* XXX: It's possible to inline these! */
		(void)ref;
	}	break;

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_OBJECT_OPT): {
		/* XXX: It's possible to inline these! */
		(void)ref;
	}	break;

	CASE(Dee_STRUCT_BOOL(HOST_SIZEOF_POINTER)): {
		DO(Dee_function_generator_vind(self, desc->m_field.m_offset)); /* FIELD */
		DO(Dee_function_generator_vreg(self, NULL));                   /* reg:FIELD */
		DO(Dee_function_generator_vdirect(self, 1));                   /* reg:FIELD */
		ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
		Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_NZ;
		return 0;
	}	break;

	CASE(Dee_STRUCT_INTEGER(HOST_SIZEOF_POINTER)):
	CASE(Dee_STRUCT_UNSIGNED | Dee_STRUCT_INTEGER(HOST_SIZEOF_POINTER)): {
		DO(Dee_function_generator_vind(self, desc->m_field.m_offset)); /* FIELD */
		DO(Dee_function_generator_vreg(self, NULL));                   /* reg:FIELD */
		DO(Dee_function_generator_vdirect(self, 1));                   /* reg:FIELD */
		ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
		Dee_function_generator_vtop(self)->ml_vmorph = (desc->m_field.m_type & Dee_STRUCT_UNSIGNED)
		                                               ? MEMLOC_VMORPH_UINT
		                                               : MEMLOC_VMORPH_INT;
		return 0;
	}	break;

#undef CASE
	default: break;
	}

	/* Fallback: emit a call to `Dee_type_member_get()' */
/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc)); /* this, desc */
	return Dee_function_generator_vcallapi(self, &Dee_type_member_get, VCALLOP_CC_OBJECT, 2);
err:
	return -1;
}

/* this -> bound */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vbound_type_member(struct Dee_function_generator *__restrict self,
                                          struct Dee_type_member const *__restrict desc) {
	/* Behavior here mirrors `Dee_type_member_bound()' */
	if (TYPE_MEMBER_ISCONST(desc)) {
push_true:
		DO(Dee_function_generator_vpop(self)); /* N/A */
		return Dee_function_generator_vpush_const(self, Dee_True);
	}
	switch (desc->m_field.m_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

#define CASE(x) case (x) & ~(STRUCT_CONST | STRUCT_ATOMIC)
	CASE(STRUCT_NONE):
	CASE(STRUCT_OBJECT_OPT): /* Always bound (because it is `none' when NULL) */
	CASE(STRUCT_CSTR_OPT):
	CASE(STRUCT_CSTR_EMPTY):
	CASE(STRUCT_STRING):
	CASE(STRUCT_CHAR):
	CASE(STRUCT_BOOL8):
	CASE(STRUCT_BOOL16):
	CASE(STRUCT_BOOL32):
	CASE(STRUCT_BOOL64):
	CASE(STRUCT_BOOLBIT0):
	CASE(STRUCT_BOOLBIT1):
	CASE(STRUCT_BOOLBIT2):
	CASE(STRUCT_BOOLBIT3):
	CASE(STRUCT_BOOLBIT4):
	CASE(STRUCT_BOOLBIT5):
	CASE(STRUCT_BOOLBIT6):
	CASE(STRUCT_BOOLBIT7):
	CASE(STRUCT_FLOAT):
	CASE(STRUCT_DOUBLE):
	CASE(STRUCT_LDOUBLE):
	CASE(STRUCT_UNSIGNED|STRUCT_INT8):
	CASE(STRUCT_INT8):
	CASE(STRUCT_UNSIGNED|STRUCT_INT16):
	CASE(STRUCT_INT16):
	CASE(STRUCT_UNSIGNED|STRUCT_INT32):
	CASE(STRUCT_INT32):
	CASE(STRUCT_UNSIGNED|STRUCT_INT64):
	CASE(STRUCT_INT64):
	CASE(STRUCT_UNSIGNED|STRUCT_INT128):
	CASE(STRUCT_INT128):
	CASE(STRUCT_WOBJECT_OPT):
		goto push_true;

	CASE(STRUCT_WOBJECT): {
		/* Check if the reference is bound by generating a call to `Dee_weakref_bound()' */
		DO(Dee_function_generator_vdelta(self, desc->m_field.m_offset)); /* &FIELD */
		return Dee_function_generator_vcallapi(self, &Dee_weakref_bound, VCALLOP_CC_BOOL, 1);
	}	break;

	CASE(STRUCT_OBJECT):
	CASE(STRUCT_CSTR): {
		struct Dee_memloc *vtop;
		DO(Dee_function_generator_vind(self, desc->m_field.m_offset)); /* FIELD */
		DO(Dee_function_generator_vreg(self, NULL));                   /* reg:FIELD */
		DO(Dee_function_generator_vdirect(self, 1));                   /* reg:FIELD */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(MEMLOC_VMORPH_ISDIRECT(vtop->ml_vmorph));
		vtop->ml_vmorph = MEMLOC_VMORPH_TESTNZ(vtop->ml_vmorph);
		return 0;
	}	break;

#undef CASE
	default: break;
	}

	/* Fallback: emit a call to `Dee_type_member_bound()' */
/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc)); /* this, value, desc */
	DO(Dee_function_generator_vswap(self));            /* this, desc, value */
	return Dee_function_generator_vcallapi(self, &Dee_type_member_bound, VCALLOP_CC_BOOL, 3);
err:
	return -1;
}

/* this -> - */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vdel_type_member(struct Dee_function_generator *__restrict self,
                                        struct Dee_type_member const *__restrict desc) {
	/* Behavior here mirrors `Dee_type_member_del()' */
	int result = Dee_function_generator_vpush_const(self, Dee_None);
	if likely(result == 0)
		result = Dee_function_generator_vpop_type_member(self, desc);
	return result;
}

/* this, value -> - */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpop_type_member(struct Dee_function_generator *__restrict self,
                                        struct Dee_type_member const *__restrict desc) {
	/* Behavior here mirrors `Dee_type_member_set()' */
	if unlikely(TYPE_MEMBER_ISCONST(desc))
		goto fallback;
	if unlikely(desc->m_field.m_type & STRUCT_CONST)
		goto fallback;
	/* TODO: Inline the set operation when possible. */

	/* Fallback: emit a call to `Dee_type_member_set()' */
fallback:
	DO(Dee_function_generator_vpush_addr(self, desc)); /* this, value, desc */
	DO(Dee_function_generator_vswap(self));            /* this, desc, value */
	return Dee_function_generator_vcallapi(self, &Dee_type_member_set, VCALLOP_CC_INT, 3);
err:
	return -1;
}

/* Helpers for accessing Class-level "struct class_attribute". NOTE: These don't do type assertions! */

/* this -> value */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vpush_instance_attr(struct Dee_function_generator *__restrict self,
                                           DeeTypeObject *type, struct class_attribute const *attr, bool ref) {
	unsigned int icmember_flags = ref ? DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF
	                                  : DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL;
	uint16_t field_addr;
	/* Behavior here mirrors `DeeInstance_GetAttribute()' */
	field_addr = attr->ca_addr;
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		field_addr += CLASS_GETSET_GET;
#endif /* CLASS_GETSET_GET != 0 */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		/* Member lies in class memory. */                                          /* this */
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD))
			DO(Dee_function_generator_vpop(self));                                  /* N/A */
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)type));            /* [this], type */
		DO(Dee_function_generator_vpush_cmember(self, field_addr, icmember_flags)); /* [this], member */
	} else {
		/* Member lies in instance memory. */                                       /* this */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			DO(Dee_function_generator_vdup(self));                                  /* this, this */
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)type));            /* [this], this, type */
		DO(Dee_function_generator_vpush_imember(self, field_addr, icmember_flags)); /* [this], member */
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {              /* [this], getter */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {          /* this, getter */
			return Dee_function_generator_vopthiscall(self, 0); /* result */
		} else {                                                /* getter */
			return Dee_function_generator_vopcall(self, 0);     /* result */
		}
		__builtin_unreachable();
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {   /* this, func */
		DO(Dee_function_generator_vswap(self));      /* func, this */
		DO(Dee_function_generator_vdirect(self, 2)); /* func, this */
		if (Dee_function_generator_vtop(self)[-0].ml_type == MEMLOC_TYPE_CONST &&
		    Dee_function_generator_vtop(self)[-1].ml_type == MEMLOC_TYPE_CONST) {
			/* Generate compile-time constant */
			DREF DeeObject *meth;
			DeeObject *funcarg = Dee_function_generator_vtop(self)[-1].ml_value.v_const;
			DeeObject *thisarg = Dee_function_generator_vtop(self)[-0].ml_value.v_const;
			meth = DeeInstanceMethod_New(funcarg, thisarg);
			if unlikely(!meth)
				goto err;
			meth = Dee_function_generator_inlineref(self, meth);
			if unlikely(!meth)
				goto err;
			DO(Dee_function_generator_vpop(self)); /* func */
			DO(Dee_function_generator_vpop(self)); /* N/A */
			return Dee_function_generator_vpush_const(self, meth);
		}
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
			/* Inline the behavior of `DeeInstanceMethod_New()' */
			DO(Dee_function_generator_vcall_DeeObject_MALLOC(self, sizeof(DeeInstanceMethodObject))); /* this, func, ref:result */
			DO(Dee_function_generator_vswap(self));                                                   /* this, ref:result, func */
			DO(Dee_function_generator_vref(self));                                                    /* this, ref:result, ref:func */
			DO(Dee_function_generator_vpopind(self, offsetof(DeeInstanceMethodObject, im_func)));     /* this, ref:result */
			DO(Dee_function_generator_vswap(self));                                                   /* ref:result, this */
			DO(Dee_function_generator_vref(self));                                                    /* ref:result, ref:this */
			DO(Dee_function_generator_vpopind(self, offsetof(DeeInstanceMethodObject, im_this)));     /* ref:result */
			DO(Dee_function_generator_vpush_immSIZ(self, 1));                                         /* ref:result, 1 */
			DO(Dee_function_generator_vpopind(self, offsetof(DeeInstanceMethodObject, ob_refcnt)));   /* ref:result */
			DO(Dee_function_generator_vpush_const(self, (DeeObject *)&DeeInstanceMethod_Type));       /* ref:result, DeeFunction_Type */
			DO(Dee_function_generator_vref(self));                                                    /* ref:result, ref:DeeFunction_Type */
			DO(Dee_function_generator_vpopind(self, offsetof(DeeInstanceMethodObject, ob_type)));     /* ref:result */
			return 0;
		}
		return Dee_function_generator_vcallapi(self, &DeeInstanceMethod_New, VCALLOP_CC_OBJECT, 2);
	}
	return 0;
err:
	return -1;
}

/* this -> bound */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vbound_instance_attr(struct Dee_function_generator *__restrict self,
                                            DeeTypeObject *type, struct class_attribute const *attr) {
	struct class_desc *desc;
	/* Behavior here mirrors `DeeInstance_BoundAttribute()' */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		/* When it isn't a get-set, then we can just check if the class/instance member is bound. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			/* Member lies in class memory. */                               /* this */
			DO(Dee_function_generator_vpop(self));                           /* N/A */
			DO(Dee_function_generator_vpush_const(self, (DeeObject *)type)); /* type */
			return Dee_function_generator_vbound_cmember(self, attr->ca_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
		} else {
			/* Member lies in instance memory. */                            /* this */
			DO(Dee_function_generator_vpush_const(self, (DeeObject *)type)); /* this, type */
			return Dee_function_generator_vbound_imember(self, attr->ca_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_NORMAL);
		}
	}
	desc = DeeClass_DESC(type);
	DO(Dee_function_generator_vpush_addr(self, desc));                                           /* this, desc */
	DO(Dee_function_generator_vswap(self));                                                      /* desc, this */
	DO(Dee_function_generator_vdup(self));                                                       /* desc, this, this */
	DO(Dee_function_generator_vdelta(self, desc->cd_offset));                                    /* desc, this, DeeInstance_DESC(desc, this) */
	DO(Dee_function_generator_vswap(self));                                                      /* desc, DeeInstance_DESC(desc, this), this */
	DO(Dee_function_generator_vpush_addr(self, attr));                                           /* desc, DeeInstance_DESC(desc, this), this, attr */
	DO(Dee_function_generator_vcallapi(self, &DeeInstance_BoundAttribute, VCALLOP_CC_M1INT, 4)); /* result */
	DO(Dee_function_generator_vdirect(self, 1));                                                 /* result */
	ASSERT(MEMLOC_VMORPH_ISDIRECT(Dee_function_generator_vtop(self)->ml_vmorph));
	Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
	return 0;
err:
	return -1;
}

/* this -> - */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vdel_instance_attr(struct Dee_function_generator *__restrict self,
                                          DeeTypeObject *type, struct class_attribute const *attr) {
	/* Behavior here mirrors `DeeInstance_DelAttribute()' */
	struct class_desc *desc = DeeClass_DESC(type);
	/* TODO: Inline call to `DeeInstance_DelAttribute()' */

/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc));        /* this, desc */
	DO(Dee_function_generator_vswap(self));                   /* desc, this */
	DO(Dee_function_generator_vdup(self));                    /* desc, this, this */
	DO(Dee_function_generator_vdelta(self, desc->cd_offset)); /* desc, this, DeeInstance_DESC(desc, this) */
	DO(Dee_function_generator_vswap(self));                   /* desc, DeeInstance_DESC(desc, this), this */
	DO(Dee_function_generator_vpush_addr(self, attr));        /* desc, DeeInstance_DESC(desc, this), this, attr */
	return Dee_function_generator_vcallapi(self, &DeeInstance_DelAttribute, VCALLOP_CC_INT, 4);
err:
	return -1;
}

/* this, value -> - */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vpop_instance_attr(struct Dee_function_generator *__restrict self,
                                          DeeTypeObject *type, struct class_attribute const *attr) {
	/* Behavior here mirrors `DeeInstance_SetAttribute()' */
	struct class_desc *desc = DeeClass_DESC(type);
	/* TODO: Inline call to `DeeInstance_SetAttribute()' */

/*fallback:*/
	DO(Dee_function_generator_vpush_addr(self, desc));        /* this, value, desc */
	DO(Dee_function_generator_vlrot(self, 3));                /* value, desc, this */
	DO(Dee_function_generator_vdup(self));                    /* value, desc, this, this */
	DO(Dee_function_generator_vdelta(self, desc->cd_offset)); /* value, desc, this, DeeInstance_DESC(desc, this) */
	DO(Dee_function_generator_vswap(self));                   /* value, desc, DeeInstance_DESC(desc, this), this */
	DO(Dee_function_generator_vpush_addr(self, attr));        /* value, desc, DeeInstance_DESC(desc, this), this, attr */
	DO(Dee_function_generator_vlrot(self, 5));                /* desc, DeeInstance_DESC(desc, this), this, attr, value */
	return Dee_function_generator_vcallapi(self, &DeeInstance_SetAttribute, VCALLOP_CC_INT, 4);
err:
	return -1;
}

/* this, [args...], kw -> UNCHECKED(result) */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_vcall_instance_attrkw_unchecked(struct Dee_function_generator *__restrict self, DeeTypeObject *type,
                                                       struct class_attribute const *attr, Dee_vstackaddr_t argc) {
	uint16_t field_addr;
	/* Behavior here mirrors `DeeInstance_CallAttributeKw()' */
	DO(Dee_function_generator_vlrot(self, argc + 2)); /* [args...], kw, this */
	field_addr = attr->ca_addr;
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		field_addr += CLASS_GETSET_GET;
#endif /* CLASS_GETSET_GET != 0 */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		/* Member lies in class memory. */                               /* [args...], kw, this */
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)type)); /* [args...], kw, this, type */
		DO(Dee_function_generator_vpush_cmember(self, field_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], kw, this, member */
	} else {
		/* Member lies in instance memory. */                            /* [args...], kw, this */
		DO(Dee_function_generator_vdup(self));                           /* [args...], kw, this, this */
		DO(Dee_function_generator_vpush_const(self, (DeeObject *)type)); /* [args...], kw, this, this, type */
		DO(Dee_function_generator_vpush_imember(self, field_addr, DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)); /* [args...], kw, this, member */
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) { /* [args...], kw, this, getter */
		DO(Dee_function_generator_vswap(self));    /* [args...], kw, getter, this */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			DO(Dee_function_generator_vopthiscall(self, 0)); /* [args...], kw, func */
		} else {
			DO(Dee_function_generator_vpop(self));       /* [args...], kw, getter */
			DO(Dee_function_generator_vopcall(self, 0)); /* [args...], kw, func */
		}
		DO(Dee_function_generator_vrrot(self, argc + 2)); /* func, [args...], kw */
		return Dee_function_generator_vopcallkw_unchecked(self, argc);
	} else {
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {        /* [args...], kw, this, func */
			DO(Dee_function_generator_vrrot(self, argc + 3)); /* func, [args...], kw, this */
			DO(Dee_function_generator_vrrot(self, argc + 2)); /* func, this, [args...], kw */
			return Dee_function_generator_vopthiscallkw_unchecked(self, argc);
		} else {                                              /* [args...], kw, this, func */
			DO(Dee_function_generator_vrrot(self, argc + 3)); /* func, [args...], kw, this */
			DO(Dee_function_generator_vpop(self));            /* func, [args...], kw */
			return Dee_function_generator_vopcallkw_unchecked(self, argc);
		}
	}
	__builtin_unreachable();
err:
	return -1;
}




DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_EX_C */
