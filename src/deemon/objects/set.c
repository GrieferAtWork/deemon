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
#ifndef GUARD_DEEMON_OBJECTS_SET_C
#define GUARD_DEEMON_OBJECTS_SET_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
/**/

#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/strings.h"
#include "seq/default-sets.h"
#include "seq/unique-iterator.h"
#include "generic-proxy.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
set_isdisjoint(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("isdisjoint", params: "
	rhs: ?DSet
", docStringPrefix: "set");]]]*/
#define set_isdisjoint_params "rhs:?."
	struct {
		DeeObject *rhs;
	} args;
	DeeArg_Unpack1(err, argc, argv, "isdisjoint", &args.rhs);
/*[[[end]]]*/
	result = SetIntersection_NonEmpty(self, args.rhs);
	if unlikely(result < 0)
		goto err;
	return_bool(!result);
err:
	return NULL;
}

PRIVATE struct type_method tpconst set_methods[] = {
	/* Default operations for all sets. */
	TYPE_METHOD("isdisjoint", &set_isdisjoint,
	            "(" set_isdisjoint_params ")->?Dbool\n"
	            "Returns ?t if ${!((this as Set) & rhs)}\n"
	            "In other words: If @this and @rhs have no items in "
	            /**/ "common (meaning their ?#intersection is empty)"),

	/* Method hint API functions. */
	TYPE_METHOD(DeeMA_Set_equals_name, &DeeMA_Set_equals,
	            "(rhs:?X2?DSet?S?O)->?Dbool\n"
	            "Used to implement ?Dequals for Set objects. Calling this function "
	            "directly is equivalent to ${deemon.equals(this as Set, rhs)}\n"
	            "#T{Requirements|Implementation~"
	            /**/ "TODO|TODO&"
	            /**/ "TODO|TODO"
	            "}"),
	TYPE_METHOD(DeeMA_Set_union_name, &DeeMA_Set_union,
	            "" DeeMA_Set_union_doc "\n"
	            "Same as ${(this as Set) | rhs}"),
	TYPE_METHOD(DeeMA_Set_difference_name, &DeeMA_Set_difference,
	            "" DeeMA_Set_difference_doc "\n"
	            "Same as ${(this as Set) - rhs}"),
	TYPE_METHOD(DeeMA_Set_intersection_name, &DeeMA_Set_intersection,
	            "" DeeMA_Set_intersection_doc "\n"
	            "Same as ${(this as Set) & rhs}"),
	TYPE_METHOD(DeeMA_Set_symmetric_difference_name, &DeeMA_Set_symmetric_difference,
	            "" DeeMA_Set_symmetric_difference_doc "\n"
	            "Same as ${(this as Set) ^ rhs}"),
	TYPE_METHOD(DeeMA_Set_issubset_name, &DeeMA_Set_issubset,
	            "" DeeMA_Set_issubset_doc "\n"
	            "Same as ${(this as Set) <= rhs}"),
	TYPE_METHOD(DeeMA_Set_issuperset_name, &DeeMA_Set_issuperset,
	            "" DeeMA_Set_issuperset_doc "\n"
	            "Same as ${(this as Set) >= rhs}"),

	TYPE_METHOD(DeeMA_Set_unify_name, &DeeMA_Set_unify,
	            "" DeeMA_Set_unify_doc "\n"
	            "Check if @key (or an object with identical #Chash and also compare equal) "
	            /**/ "is already present within @this set. If so, return that already-present "
	            /**/ "object. Otherwise, ?#insert @key into @this set, before re-returning @key"),
	TYPE_METHOD(DeeMA_Set_insert_name, &DeeMA_Set_insert,
	            "" DeeMA_Set_insert_doc "\n"
	            "Insert @key into @this set, returning !t if it was "
	            /**/ "inserted and !f if it was already present"),
	TYPE_METHOD(DeeMA_Set_insertall_name, &DeeMA_Set_insertall,
	            "" DeeMA_Set_insertall_doc "\n"
	            "Insert all elements from @keys into @this set"),
	TYPE_METHOD(DeeMA_Set_remove_name, &DeeMA_Set_remove,
	            "" DeeMA_Set_remove_doc "\n"
	            "Remove @key from @this set, returning !t if it was "
	            /**/ "removed and !f if it wasn't present"),
	TYPE_METHOD(DeeMA_Set_removeall_name, &DeeMA_Set_removeall,
	            "" DeeMA_Set_removeall_doc "\n"
	            "Remove all elements from @keys from @this set"),
	TYPE_METHOD(DeeMA_Set_pop_name, &DeeMA_Set_pop,
	            "" DeeMA_Set_pop_doc "\n"
	            "#tValueError{Set is empty and no @def was given}\n"
	            "Remove and return some random key from @this set. If "
	            /**/ "the set is empty, return @def or throw :ValueError"),

	/* Method hint operator invocation. */
	TYPE_METHOD("__iter__", &DeeMA___set_iter__, "->?#Iterator\nAlias for ${(this as Set).operator iter()} (s.a. ?#{op:iter})"),
	TYPE_METHOD("__size__", &DeeMA___set_size__, DeeMA___set_size___doc "\nAlias for ${##(this as Set)} (s.a. ?#{op:size})"),
	TYPE_METHOD("__bool__", &DeeMA___set_bool__, DeeMA___set_bool___doc "\nAlias for ${!!(this as Set)} (s.a. ?#{op:bool})"),
	TYPE_METHOD("__hash__", &DeeMA___set_hash__, DeeMA___set_hash___doc "\nAlias for ${(this as Set).operator hash()} (s.a. ?#{op:hash})"),
	TYPE_METHOD("__compare_eq__", &DeeMA___set_compare_eq__, "(rhs:?X2?DSet?S?O)->?Dbool\nAlias for ${Set.equals(this, rhs)} (s.a. ?#equals)"),
	TYPE_METHOD("__eq__", &DeeMA___set_eq__, DeeMA___set_eq___doc "\nAlias for ${(this as Set) == rhs} (s.a. ?#{op:eq})"),
	TYPE_METHOD("__ne__", &DeeMA___set_ne__, DeeMA___set_ne___doc "\nAlias for ${(this as Set) != rhs} (s.a. ?#{op:ne})"),
	TYPE_METHOD("__lo__", &DeeMA___set_lo__, DeeMA___set_lo___doc "\nAlias for ${(this as Set) < rhs} (s.a. ?#{op:lo})"),
	TYPE_METHOD("__le__", &DeeMA___set_le__, DeeMA___set_le___doc "\nAlias for ${(this as Set) <= rhs} (s.a. ?#{op:le})"),
	TYPE_METHOD("__gr__", &DeeMA___set_gr__, DeeMA___set_gr___doc "\nAlias for ${(this as Set) > rhs} (s.a. ?#{op:gr})"),
	TYPE_METHOD("__ge__", &DeeMA___set_ge__, DeeMA___set_ge___doc "\nAlias for ${(this as Set) >= rhs} (s.a. ?#{op:ge})"),
	TYPE_METHOD("__inv__", &DeeMA___set_inv__, DeeMA___set_inv___doc "\nAlias for ${~(this as Set)} (s.a. ?#{op:inv})"),
	TYPE_METHOD("__add__", &DeeMA___set_add__, DeeMA___set_add___doc "\nAlias for ${(this as Set) + rhs}, ${(this as Set) | rhs} (s.a. ?#union, ?#{op:add}, ?#{op:or}, ?#__or__)"),
	TYPE_METHOD("__sub__", &DeeMA___set_sub__, DeeMA___set_sub___doc "\nAlias for ${(this as Set) - rhs} (s.a. ?#difference, ?#{op:sub})"),
	TYPE_METHOD("__and__", &DeeMA___set_and__, DeeMA___set_and___doc "\nAlias for ${(this as Set) & rhs} (s.a. ?#intersection, ?#{op:and})"),
	TYPE_METHOD("__xor__", &DeeMA___set_xor__, DeeMA___set_xor___doc "\nAlias for ${(this as Set) ^ rhs} (s.a. ?#symmetric_difference, ?#{op:xor})"),
	TYPE_METHOD("__inplace_add__", &DeeMA___set_inplace_add__, DeeMA___set_inplace_add___doc "\nAlias for ${(this as Set) += rhs} (s.a. ?#{op:iadd}, ?#{op:ior})"),
	TYPE_METHOD("__inplace_sub__", &DeeMA___set_inplace_sub__, DeeMA___set_inplace_sub___doc "\nAlias for ${(this as Set) -= rhs} (s.a. ?#{op:isub})"),
	TYPE_METHOD("__inplace_and__", &DeeMA___set_inplace_and__, DeeMA___set_inplace_and___doc "\nAlias for ${(this as Set) &= rhs} (s.a. ?#{op:iand})"),
	TYPE_METHOD("__inplace_xor__", &DeeMA___set_inplace_xor__, DeeMA___set_inplace_xor___doc "\nAlias for ${(this as Set) ^= rhs} (s.a. ?#{op:ixor})"),
	TYPE_METHOD("__or__", &DeeMA___set_add__, DeeMA___set_add___doc "\nAlias for ?#__add__ (s.a. ?#union, ?#{op:add}, ?#{op:or}, ?#__add__)"),
	TYPE_METHOD("__inplace_or__", &DeeMA___set_inplace_add__, DeeMA___set_inplace_add___doc "\nAlias for ?#__inplace_add__ (s.a. ?#{op:iadd}, ?#{op:ior})"),
	TYPE_METHOD_END
};

#define set_asseq generic_obj__asseq
#define set_asset DeeObject_NewRef
#define set_asmap generic_obj__asmap

INTDEF struct type_getset tpconst set_getsets[];
INTERN_TPCONST struct type_getset tpconst set_getsets[] = {
	TYPE_GETSET_BOUND(STR_first,
	                  &default__set_getfirst,
	                  &default__set_delfirst,
	                  &default__set_setfirst,
	                  &default__set_boundfirst,
	                  "->\n"
	                  "Access the first item of the Set\n"

	                  "When reading the attribute (as in ${x = Set.first(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __set_first__: Object}" /*                                         */ "|${return this.__set_first__;}&"
	                  /**/ "${property __seq_first__: Object} (when ?A__seq_getitem_always_bound__?DSequence)" /**/ "|${return this.__seq_first__;}&"
	                  /**/ "${function first: Object} (?A__seqclass__?DType is ?., ?DSet or ?DMapping)" /**/ "|${return this.first;}&"
	                  /**/ "?A{op:iter}?DSequence" /**/ "|${"
	                  /**/ /**/ "local iter = Sequence.__iter__(this);\n"
	                  /**/ /**/ "foreach (local item: iter)\n"
	                  /**/ /**/ "	return item;\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Set.first);"
	                  /**/ "}"
	                  "}\n"

	                  "When deleting the attribute (as in ${Set.first.delete(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __set_first__: Object}" /*                     */ "|${del this.__set_first__;}&"
	                  /**/ "${property __seq_first__: Object} (when ?A__seq_getitem_always_bound__?DSequence)" /**/ "|${del this.__seq_first__;}&"
	                  /**/ "${function first: Object} (?A__seqclass__?DType is ?.)" /**/ "|${del this.first;}&"
	                  /**/ "?A{op:size}?DSequence, ?A{op:getitem}?DSequence, ?A{op:delitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "for (local i: [:size]) {\n"
	                  /**/ /**/ "	local isBound;\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		isBound = Sequence.__bounditem__(this, i, false);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "	if (isBound) {\n"
	                  /**/ /**/ "		Sequence.__delitem__(this, i);\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}"
	                  /**/ "}&"
	                  /**/ "?A{op:getitem}?DSequence, ?A{op:delitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "for (local i = 0;; ++i) {\n"
	                  /**/ /**/ "	local isBound;\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		isBound = Sequence.__bounditem__(this, i, false);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "	if (isBound) {\n"
	                  /**/ /**/ "		Sequence.__delitem__(this, i);\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}"
	                  /**/ "}"
	                  "}\n"

	                  "When setting the attribute (as in ${Set.first.set(mySeq, value)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __set_first__: Object}" /*                     */ "|${this.__set_first__ = value;}&"
	                  /**/ "${property __seq_first__: Object} (when ?A__seq_getitem_always_bound__?DSequence)" /**/ "|${this.__seq_first__ = value;}&"
	                  /**/ "${function first: Object} (?A__seqclass__?DType is ?.)" /**/ "|${this.first = value;}&"
	                  /**/ "?A{op:size}?DSequence, ?A{op:getitem}?DSequence, ?A{op:setitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "for (local i: [:size]) {\n"
	                  /**/ /**/ "	local isBound;\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		isBound = Sequence.__bounditem__(this, i, false);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "	if (isBound) {\n"
	                  /**/ /**/ "		Sequence.__setitem__(this, i, value);\n"
	                  /**/ /**/ "		return;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw EmptySequence(seq: this);"
	                  /**/ "}&"
	                  /**/ "?A{op:getitem}?DSequence, ?A{op:setitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "for (local i = 0;; ++i) {\n"
	                  /**/ /**/ "	local isBound;\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		isBound = Sequence.__bounditem__(this, i, false);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "	if (isBound) {\n"
	                  /**/ /**/ "		Sequence.__setitem__(this, i, value);\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw EmptySequence(seq: this);"
	                  /**/ "}"
	                  "}"),
	TYPE_GETSET_BOUND(STR_last,
	                  &default__set_getlast,
	                  &default__set_dellast,
	                  &default__set_setlast,
	                  &default__set_boundlast,
	                  "->\n"
	                  "Access the last item of the Set\n"

	                  "When reading the attribute (as in ${x = Set.last(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __set_last__: Object}" /*                     */ "|${return this.__set_last__;}&"
	                  /**/ "${property __seq_last__: Object} (when ?A__seq_getitem_always_bound__?DSequence)" /**/ "|${return this.__seq_last__;}&"
	                  /**/ "${function last: Object} (?A__seqclass__?DType is ?.)" /**/ "|${return this.last;}&"
	                  /**/ "?A{op:size}?DSequence, ?A{op:getitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "while (size--) {\n"
	                  /**/ /**/ "	try {\n"
	                  /**/ /**/ "		return Sequence.__getitem__(this, size);\n"
	                  /**/ /**/ "	} catch (IndexError) {\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	} catch (UnboundItem) {\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Set.last);"
	                  /**/ "}"
	                  /**/ "?A{op:iter}?DSequence" /**/ "|${"
	                  /**/ /**/ "local iter = Sequence.__iter__(this);\n"
	                  /**/ /**/ "local item;\n"
	                  /**/ /**/ "foreach (item: iter) {\n"
	                  /**/ /**/ "	foreach (item: iter)\n"
	                  /**/ /**/ "		;\n"
	                  /**/ /**/ "	return item;\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw UnboundAttribute(ob: this, attr: Set.last);"
	                  /**/ "}"
	                  "}\n"

	                  "When deleting the attribute (as in ${Set.last.delete(mySeq)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __set_last__: Object}" /*                     */ "|${del this.__set_last__;}&"
	                  /**/ "${property __seq_last__: Object} (when ?A__seq_getitem_always_bound__?DSequence)" /**/ "|${del this.__seq_last__;}&"
	                  /**/ "${function last: Object} (?A__seqclass__?DType is ?.)" /**/ "|${del this.last;}&"
	                  /**/ "?A{op:size}?DSequence, ?A{op:getitem}?DSequence, ?A{op:delitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "while (size--) {\n"
	                  /**/ /**/ "	if (Sequence.__bounditem__(this, size, true)) {\n"
	                  /**/ /**/ "		Sequence.__delitem__(this, size);\n"
	                  /**/ /**/ "		break;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}"
	                  /**/ "}"
	                  "}\n"

	                  "When setting the attribute (as in ${Set.last.set(mySeq, value)}):\n"
	                  "#T{Requirements|Implementation~"
	                  /**/ "${property __set_last__: Object}" /*                     */ "|${this.__set_last__ = value;}&"
	                  /**/ "${property __seq_last__: Object} (when ?A__seq_getitem_always_bound__?DSequence)" /**/ "|${this.__seq_last__ = value;}&"
	                  /**/ "${function last: Object} (?A__seqclass__?DType is ?.)" /**/ "|${this.last = value;}&"
	                  /**/ "?A{op:size}?DSequence, ?A{op:getitem}?DSequence, ?A{op:delitem}?DSequence" /**/ "|${"
	                  /**/ /**/ "local size = Sequence.__size__(this);\n"
	                  /**/ /**/ "while (size--) {\n"
	                  /**/ /**/ "	if (Sequence.__bounditem__(this, size, true)) {\n"
	                  /**/ /**/ "		Sequence.__setitem__(this, size, value);\n"
	                  /**/ /**/ "		return;\n"
	                  /**/ /**/ "	}\n"
	                  /**/ /**/ "}\n"
	                  /**/ /**/ "throw EmptySequence(seq: this);"
	                  /**/ "}"
	                  "}"),

	TYPE_GETTER(STR_frozen, &default__set_frozen,
	            "->?#Frozen\n"
	            "Returns a copy of @this ?., with all of its current elements frozen in place, "
	            /**/ "constructing a snapshot of the set's current elements. - The actual type of "
	            /**/ "set returned is implementation- and type- specific, and copying itself may "
	            /**/ "either be done immediately, or as copy-on-write"),
	TYPE_GETTER_AB("asseq", &set_asseq, "->?DSequence\nOptimized version of ${this as Sequence}"),
	TYPE_GETTER_AB("asset", &set_asset, "->?.\nOptimized version of ${this as Set}"),
	TYPE_GETTER_AB("asmap", &set_asmap, "->?DMapping\nOptimized version of ${this as Mapping}"),
	TYPE_GETTER("length", &default__set_operator_sizeob,
	            "->?Dint\nAlias for ${##(this as Set)}"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
set_Frozen_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSet_Type;
	DeeMH_set_frozen_t set_frozen = DeeType_RequireMethodHint(self, set_frozen);
	if (set_frozen == &DeeObject_NewRef) {
		result = self;
	} else if (set_frozen == &DeeRoSet_FromSequence) {
		result = &DeeRoSet_Type;
	} else if (set_frozen == &default__set_frozen__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
set_Iterator_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_set_operator_iter_t set_operator_iter = DeeType_RequireMethodHint(self, set_operator_iter);
	if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter) {
		result = &DistinctIterator_Type;
	} else if (set_operator_iter == &default__set_operator_iter__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}



PRIVATE struct type_getset tpconst set_class_getsets[] = {
	TYPE_GETTER(STR_Frozen, &set_Frozen_get,
	            "->?DType\n"
	            "Returns the type of ?DSet returned by the ?#frozen property"),
	TYPE_GETTER(STR_Iterator, &set_Iterator_get,
	            "->?DType\n"
	            "Returns the type of ?DIterator returned by the ?#{op:iter}"),
	TYPE_GETSET_END
};


INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);

PRIVATE struct type_operator const set_operators[] = {
//	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR), /* TODO: And THISELEM CONST_HASH+CONST_COMPARE_EQ */
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


PRIVATE struct type_math set_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ &default__set_operator_inv,
	/* .tp_pos         = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg         = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add         = */ &default__set_operator_add,
	/* .tp_sub         = */ &default__set_operator_sub,
	/* .tp_mul         = */ DEFIMPL_UNSUPPORTED(&default__mul__unsupported),
	/* .tp_div         = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod         = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl         = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr         = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and         = */ &default__set_operator_and,
	/* .tp_or          = */ &default__set_operator_add,
	/* .tp_xor         = */ &default__set_operator_xor,
	/* .tp_pow         = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL_UNSUPPORTED(&default__inc__unsupported),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ &default__set_operator_inplace_add,
	/* .tp_inplace_sub = */ &default__set_operator_inplace_sub,
	/* .tp_inplace_mul = */ DEFIMPL_UNSUPPORTED(&default__inplace_mul__unsupported),
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ &default__set_operator_inplace_and,
	/* .tp_inplace_or  = */ &default__set_operator_inplace_add,
	/* .tp_inplace_xor = */ &default__set_operator_inplace_xor,
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

PRIVATE struct type_cmp set_cmp = {
	/* .tp_hash          = */ &default__set_operator_hash,
	/* .tp_compare_eq    = */ &default__set_operator_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ &default__set_operator_trycompare_eq,
	/* .tp_eq            = */ &default__set_operator_eq,
	/* .tp_ne            = */ &default__set_operator_ne,
	/* .tp_lo            = */ &default__set_operator_lo,
	/* .tp_le            = */ &default__set_operator_le,
	/* .tp_gr            = */ &default__set_operator_gr,
	/* .tp_ge            = */ &default__set_operator_ge,
};

PRIVATE struct type_seq set_seq = {
	/* .tp_iter                       = */ &default__set_operator_iter,
	/* .tp_sizeob                     = */ &default__set_operator_sizeob,
	/* .tp_contains                   = */ &default__seq_operator_contains,
	/* .tp_getitem                    = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ &default__set_operator_foreach,
	/* .tp_foreach_pair               = */ &default__foreach_pair__with__foreach,
	/* .tp_bounditem                  = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem                    = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size                       = */ &default__set_operator_size,
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
	/* .tp_asvector                   = */ NULL,
	/* .tp_asvector_nothrow           = */ NULL,
};

#ifdef CONFIG_NO_DOC
#define set_doc NULL
#else /* CONFIG_NO_DOC */
PRIVATE char const set_doc[] =
"A recommended abstract base class for any set type "
/**/ "that wishes to implement the Object-Set protocol\n"
"An object derived from this class must implement "
/**/ "${operator contains}, and preferrably ${operator iter}\n"
"\n"

"()\n"
"A no-op default constructor that is implicitly called by sub-classes\n"
"When invoked manually, a general-purpose, empty ?. equivalent "
/**/ "to ?Ert:Set_empty is returned\n"
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
"Returns ?t if @this ?. is non-empty. Note that because sets cannot "
/**/ "contain unbound items, a non-empty ?DSequence consisting of only "
/**/ "unbound items would evaluate to ?t in ?A{op:bool}?DSequence, but "
/**/ "when interpreted as ?., will evaluate to ?f\n"
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
"}";
#endif /* !CONFIG_NO_DOC */

/* `Set from deemon' */
PUBLIC DeeTypeObject DeeSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Set),
	/* .tp_doc      = */ set_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_SET << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor, /* Allow default-construction of sequence objects. */
			/* tp_copy_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_deep_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorSerialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ &default__set_operator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ &default_set_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &set_math,
	/* .tp_cmp           = */ &set_cmp,
	/* .tp_seq           = */ &set_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ set_methods,
	/* .tp_getsets       = */ set_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ set_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ set_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(set_operators),
	/* .tp_mhcache       = */ &mh_cache_empty,
};

PUBLIC DeeObject DeeSet_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeSet_Type)
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SET_C */
