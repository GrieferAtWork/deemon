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
#ifndef GUARD_DEEMON_OBJECTS_SET_C
#define GUARD_DEEMON_OBJECTS_SET_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include "../runtime/operator-require.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/default-api.h"
#include "seq/default-sets.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_difference(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:difference", &other))
		goto err;
	return DeeSet_OperatorSub(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_intersection(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:intersection", &other))
		goto err;
	return DeeSet_OperatorAnd(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_union(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:union", &other))
		goto err;
	return DeeSet_OperatorAdd(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_symmetric_difference(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:symmetric_difference", &other))
		goto err;
	return DeeSet_OperatorXor(self, other);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_isdisjoint(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:isdisjoint", &rhs))
		goto err;
	result = SetIntersection_NonEmpty(self, rhs);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_issubset(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:issubset", &other))
		goto err;
	return DeeSet_OperatorLe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_issuperset(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:issuperset", &other))
		goto err;
	return DeeSet_OperatorGe(self, other);
err:
	return NULL;
}

PRIVATE struct type_method tpconst set_methods[] = {
	TYPE_METHOD("difference", &set_difference,
	            "(to:?.)->?.\n"
	            "Same as ${(this as Set) - to}"),
	TYPE_METHOD("intersection", &set_intersection,
	            "(with_:?.)->?.\n"
	            "Same as ${(this as Set) & with_}"),
	TYPE_METHOD("isdisjoint", &set_isdisjoint,
	            "(with_:?.)->?Dbool\n"
	            "Returns ?t if ${!((this as Set) & with_)}\n"
	            "In other words: If @this and @with_ have no items in common"),
	TYPE_METHOD("union", &set_union,
	            "(with_:?.)->?.\n"
	            "Same as ${(this as Set) | with_}"),
	TYPE_METHOD("symmetric_difference", &set_symmetric_difference,
	            "(with_:?.)->?.\n"
	            "Same as ${(this as Set) ^ with_}"),
	TYPE_METHOD("issubset", &set_issubset,
	            "(of:?.)->?Dbool\n"
	            "Same as ${(this as Set) <= of}"),
	TYPE_METHOD("issuperset", &set_issuperset,
	            "(of:?.)->?Dbool\n"
	            "Same as ${(this as Set) >= of}"),

	/* Default functions for mutable sets */
	TYPE_METHOD(STR_insert, &default_set_insert,
	            "(key)->?Dbool\n"
	            "Insert @key into @this set, returning !t if it was inserted and !f if it was already present"),
	TYPE_METHOD(STR_remove, &default_set_remove,
	            "(key)->?Dbool\n"
	            "Remove @key from @this set, returning !t if it was removed and !f if it wasn't present"),
	TYPE_METHOD(STR_insertall, &default_set_insertall,
	            "(keys:?S?O)\n"
	            "Insert all elements from @keys into @this set"),
	TYPE_METHOD(STR_removeall, &default_set_removeall,
	            "(keys:?S?O)\n"
	            "Remove all elements from @keys from @this set"),
	TYPE_METHOD(STR_unify, &default_set_unify,
	            "(key)->\n"
	            "Insert @key into @this set if it wasn't contained already, and "
	            /**/ "return the (potential) copy of @key that is part of the set"),
	TYPE_METHOD(STR_pop, &default_set_pop,
	            "(def?)->\n"
	            "#tValueError{Set is empty and no @def was given}\n"
	            "Remove and return some random key from @this set. "
	            /**/ "If the set is empty, return @def or throw :ValueError"),

	TYPE_METHOD("__hash__", &default_set___hash__,
	            "->?Dint\n"
	            "Alias for ${(this as Set).operator hash()}"),
	TYPE_METHOD("__compare_eq__", &default_set___compare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set).operator == (rhs)}"),
	TYPE_METHOD("__trycompare_eq__", &default_set___trycompare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${deemon.equals(this as Set, rhs)}"),
	TYPE_METHOD("__eq__", &default_set___eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set) == rhs}"),
	TYPE_METHOD("__ne__", &default_set___ne__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set) != rhs}"),
	TYPE_METHOD("__lo__", &default_set___lo__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set) < rhs}"),
	TYPE_METHOD("__le__", &default_set___le__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set) <= rhs}"),
	TYPE_METHOD("__gr__", &default_set___gr__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set) > rhs}"),
	TYPE_METHOD("__ge__", &default_set___ge__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Set) >= rhs}"),
	TYPE_METHOD("__inv__", &default_set___inv__,
	            "()->?DSet\n"
	            "Alias for ${~(this as Set)}"),
	TYPE_METHOD("__add__", &default_set___add__,
	            "(rhs:?S?O)->?DSet\n"
	            "Alias for ${(this as Set) + rhs}"),
	TYPE_METHOD("__sub__", &default_set___sub__,
	            "(rhs:?S?O)->?DSet\n"
	            "Alias for ${(this as Set) - rhs}"),
	TYPE_METHOD("__and__", &default_set___and__,
	            "(rhs:?S?O)->?DSet\n"
	            "Alias for ${(this as Set) & rhs}"),
	TYPE_METHOD("__xor__", &default_set___xor__,
	            "(rhs:?S?O)->?DSet\n"
	            "Alias for ${(this as Set) ^ rhs}"),
	TYPE_METHOD("__inplace_add__", &default_set___inplace_add__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Set) += rhs}"),
	TYPE_METHOD("__inplace_sub__", &default_set___inplace_sub__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Set) -= rhs}"),
	TYPE_METHOD("__inplace_and__", &default_set___inplace_and__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Set) &= rhs}"),
	TYPE_METHOD("__inplace_xor__", &default_set___inplace_xor__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Set) ^= rhs}"),
	TYPE_METHOD("__or__", &default_set___or__,
	            "(rhs:?S?O)->?DSet\n"
	            "Alias for ?#__and__"),
	TYPE_METHOD("__inplace_or__", &default_set___inplace_or__,
	            "(rhs:?S?O)->?DSet\n"
	            "Alias for ?#__inplace_and__"),

	TYPE_METHOD_END
};

INTDEF struct type_getset tpconst set_getsets[];
INTERN_TPCONST struct type_getset tpconst set_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeRoSet_FromSequence,
	            "->?#Frozen\n"
	            "Returns a copy of @this ?., with all of its current elements frozen in place, "
	            /**/ "constructing a snapshot of the set's current elements. - The actual type of "
	            /**/ "set returned is implementation- and type- specific, and copying itself may "
	            /**/ "either be done immediately, or as copy-on-write"),
	/* TODO: "asseq->?DSequence"  -- alias for `this as Sequence' */
	TYPE_GETSET_END
};


/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("Frozen");
]]]*/
#define Dee_HashStr__Frozen _Dee_HashSelectC(0xa7ed3902, 0x16013e56a91991ea)
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
set_frozen_get(DeeTypeObject *__restrict self) {
	int error;
	DREF DeeTypeObject *result;
	struct attribute_info info;
	struct attribute_lookup_rules rules;
	rules.alr_name       = "Frozen";
	rules.alr_hash       = Dee_HashStr__Frozen;
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = ATTR_PERMGET | ATTR_IMEMBER;
	rules.alr_perm_value = ATTR_PERMGET | ATTR_IMEMBER;
	error = DeeObject_FindAttr(Dee_TYPE(self),
	                           (DeeObject *)self,
	                           &info,
	                           &rules);
	if unlikely(error < 0)
		goto err;
	if (error != 0)
		return_reference_(&DeeRoSet_Type);
	if (info.a_attrtype) {
		result = info.a_attrtype;
		Dee_Incref(result);
	} else if (info.a_decl == (DeeObject *)&DeeSet_Type) {
		result = &DeeRoSet_Type;
		Dee_Incref(&DeeRoSet_Type);
	} else {
		if (info.a_doc) {
			/* TODO: Use doc meta-information to determine the return type! */
		}
		/* Fallback: just tell the caller what they already know: a set will be returned... */
		result = &DeeSet_Type;
		Dee_Incref(&DeeSet_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst set_class_getsets[] = {
	TYPE_GETTER("Frozen", &set_frozen_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#frozen property"),
	TYPE_GETSET_END
};


INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));

PRIVATE struct type_operator const set_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_000D_INV, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0011_SUB, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0017_AND, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0018_OR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0019_XOR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE), /* Deleted */
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),  /* Deleted */
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE), /* Deleted */
};

/* `Set from deemon' */
PUBLIC DeeTypeObject DeeSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Set),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any set type "
	                         /**/ "that wishes to implement the Object-Set protocol\n"
	                         "An object derived from this class must implement "
	                         /**/ "${operator contains}, and preferrably ${operator iter}\n"
	                         "\n"

	                         "()\n"
	                         "A no-op default constructor that is implicitly called by sub-classes\n"
	                         "When invoked manually, a general-purpose, empty ?. is returned\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the representation of all sequence elements, "
	                         /**/ "using abstract sequence syntax\n"
	                         "e.g.: ${{ \"foo\", \"bar\", \"baz\" }}\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t indicative of @item being apart of @this ?.\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. is non-empty\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating all items apart of @this ?.\n"
	                         "Note that some set types do not implement this functionality, most "
	                         /**/ "notably symbolically inversed sets\n"
	                         "\n"

	                         "sub->\n"
	                         "Returns a ?. of all objects from @this, excluding those also found in @other\n"
	                         "\n"

	                         "&->\n"
	                         "Returns the intersection of @this and @other\n"
	                         "\n"

	                         "|->\n"
	                         "add->\n"
	                         "Returns the union of @this and @other\n"
	                         "\n"

	                         "^->\n"
	                         "Returns a ?. containing objects only found in either "
	                         /**/ "@this or @other, but not those found in both\n"
	                         "\n"

	                         "<=->\n"
	                         "Returns ?t if all items found in @this ?. can also be found in @other\n"
	                         "\n"

	                         "==->\n"
	                         "Returns ?t if @this ?. contains the same items as @other, and not any more than that\n"
	                         "\n"

	                         "!=->\n"
	                         "Returns ?t if @this contains different, or less items than @other\n"
	                         "\n"

	                         "<->\n"
	                         "The result of ${this <= other && this != other}\n"
	                         "\n"

	                         ">=->\n"
	                         "Returns ?t if all items found in @other can also be found in @this ?.\n"
	                         "\n"

	                         ">->\n"
	                         "The result of ${this >= other && this != other}\n"
	                         "\n"

	                         "~->\n"
	                         "Returns a symbolic ?. that behaves as though it contained "
	                         /**/ "any feasible object that isn't already apart of @this ?.\n"
	                         "Note however that due to the impossibility of such a ?., you "
	                         /**/ "cannot iterate its elements, and the only ~real~ operator "
	                         /**/ "implemented by it is ?#{op:contains}\n"
	                         "Its main purpose is for being used in conjunction with "
	                         /**/ "?#{op:and} in order to create a sub-set that doesn't "
	                         /**/ "contain a certain set of sub-elements:\n"
	                         "${"
	                         /**/ "import Set from deemon;\n"
	                         /**/ "local x = { 10, 11, 15, 20, 30 };\n"
	                         /**/ "local y = { 11, 15 };\n"
	                         /**/ "print repr((x as Set) & ~(y as Set)); // { 10, 20, 30 }"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_SET << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL, /* TODO: "{ 10, 20, 30 } as Set" */
		/* .tp_bool = */ &DeeSet_OperatorBool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &DeeSet_OperatorMath,
	/* .tp_cmp           = */ &DeeSet_OperatorCmp,
	/* .tp_seq           = */ &DeeSet_OperatorSeq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ set_methods,
	/* .tp_getsets       = */ set_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ set_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ set_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(set_operators)
};

PUBLIC DeeObject DeeSet_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeSet_Type)
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SET_C */
