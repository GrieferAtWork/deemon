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
#ifndef GUARD_DEEMON_OBJECTS_MAP_C
#define GUARD_DEEMON_OBJECTS_MAP_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "../runtime/kwlist.h"
#include "../runtime/operator-require.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/byattr.h"
#include "seq/default-api.h"
#include "seq/default-iterators.h"
#include "seq/default-map-proxy.h"
#include "seq/each.h"
#include "seq/hashfilter.h"
#include "seq/range.h"

DECL_BEGIN

DOC_DEF(map_byhash_doc,
        "(template:?O)->?S?T2?O?O\n"
        "#ptemplate{The object who's hash should be used to search for collisions}"
        "Same as ?Abyhash?DSequence, but rather than comparing the hashes of the "
        /**/ "key-value pairs, search for pairs where the key matches the hash of @template");
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_byhash(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__template, "o:byhash", &template_))
		goto err;
	return DeeMap_HashFilter(self, DeeObject_Hash(template_));
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_get(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	result = DeeMap_OperatorTryGetItem(self, key);
	if (result == ITER_DONE) {
		Dee_Incref(def);
		result = def;
	}
	return result;
err:
	return NULL;
}



PRIVATE struct type_method tpconst map_methods[] = {
	/* Default operations for all mappings. */
	TYPE_METHOD(STR_get, &map_get,
	            "(key,def=!N)->\n"
	            "#r{The value associated with @key or @def when @key has no value associated}"),
	TYPE_KWMETHOD("byhash", &map_byhash, DOC_GET(map_byhash_doc)),

	/* Default operations for modifiable mappings. */
	TYPE_METHOD(STR_setold, &DeeMH_map_setold,
	            "(key,value)->?Dbool\n"
	            "#r{Indicative of @value having been assigned to @key}"
	            "Assign @value to @key, only succeeding when @key already existed to begin with"),
	TYPE_METHOD(STR_setold_ex, &DeeMH_map_setold_ex,
	            "(key,value)->?T2?Dbool?O\n"
	            "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	            "Same as ?#setold but also return the previously assigned value"),
	TYPE_METHOD(STR_setnew, &DeeMH_map_setnew,
	            "(key,value)->?Dbool\n"
	            "#r{Indicative of @value having been assigned to @key}"
	            "Assign @value to @key, only succeeding when @key didn't exist before"),
	TYPE_METHOD(STR_setnew_ex, &DeeMH_map_setnew_ex,
	            "(key,value)->?T2?Dbool?O\n"
	            "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	            "Same as ?#setnew but return the previously assigned value on failure"),
	TYPE_METHOD(STR_setdefault, &DeeMH_map_setdefault,
	            "(key,def)->\n"
	            "#r{The object currently assigned to @key}"
	            "Lookup @key in @this ?. and return its value if found. "
	            /**/ "Otherwise, assign @def to @key and return it instead"),
	TYPE_METHOD(STR_update, &DeeMH_map_update,
	            "(items:?S?T2?O?O)\n"
	            "Iterate @items and unpack each element into 2 others, "
	            /**/ "using them as key and value to insert into @this ?."),
	TYPE_METHOD(STR_remove, &DeeMH_map_remove, "(key)->?Dbool"),
	TYPE_METHOD(STR_removekeys, &DeeMH_map_removekeys, "(keys:?S?O)"),
	TYPE_METHOD(STR_pop, &DeeMH_map_pop,
	            "(key)->\n"
	            "(key,def)->\n"
	            "#tKeyError{No @def was given and @key was not found}"
	            "Delete @key from @this ?. and return its previously assigned "
	            /**/ "value or @def when @key had no item associated"),
	TYPE_METHOD(STR_popitem, &DeeMH_map_popitem,
	            "->?T2?O?O\n"
	            "#r{A random pair key-value pair that has been removed}"
	            "#tValueError{@this ?. was empty}"),

	TYPE_METHOD("__contains__", &default_map___contains__,
	            "(item)->?Dbool\n"
	            "Alias for ${item in (this as Mapping)}"),
	TYPE_METHOD("__getitem__", &default_map___getitem__,
	            "(key)->\n"
	            "Alias for ${(this as Mapping)[key]}"),
	TYPE_METHOD("__delitem__", &default_map___delitem__,
	            "(key)\n"
	            "Alias for ${del (this as Mapping)[key]}"),
	TYPE_METHOD("__setitem__", &default_map___setitem__,
	            "(key,value)\n"
	            "Alias for ${(this as Mapping)[key] = value}"),
	TYPE_METHOD("__enumerate__", &default_map___enumerate__,
	            "(cb)->\n"
	            "Alias for ${(this as Mapping).enumerate(cb)}"),
	TYPE_KWMETHOD("__enumerate_index__", &default_map___enumerate_index__,
	              "(cb,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->\n"
	              "Alias for ${Mapping.enumerate(this, cb, start, end)}"),
	TYPE_METHOD("__iterkeys__", &default_map___iterkeys__,
	            "->?DIterator\n"
	            "Alias for: ${(this as Mapping).iterkeys}"),
	TYPE_METHOD("__bounditem__", &default_map___bounditem__,
	            "(key,allow_missing=!t)->?Dbool\n"
	            "Alias for ${deemon.bounditem(this as Mapping, key, allow_missing)}"),
	TYPE_METHOD("__hasitem__", &default_map___hasitem__,
	            "(key)->?Dbool\n"
	            "Alias for ${deemon.hasitem(this as Mapping, key)}"),
	TYPE_METHOD("__getitem_index__", &default_map___getitem_index__,
	            "(key:?Dint)->\n"
	            "Alias for ${(this as Mapping)[key]}"),
	TYPE_METHOD("__delitem_index__", &default_map___delitem_index__,
	            "(key:?Dint)\n"
	            "Alias for ${del (this as Mapping)[key]}"),
	TYPE_METHOD("__setitem_index__", &default_map___setitem_index__,
	            "(key:?Dint,value)\n"
	            "Alias for ${(this as Mapping)[key] = value}"),
	TYPE_METHOD("__bounditem_index__", &default_map___bounditem_index__,
	            "(key:?Dint,allow_missing=!t)->?Dbool\n"
	            "Alias for ${deemon.bounditem(this as Mapping, key, allow_missing)}"),
	TYPE_METHOD("__hasitem_index__", &default_map___hasitem_index__,
	            "(key:?Dint)->?Dbool\n"
	            "Alias for ${deemon.hasitem(this as Mapping, key)}"),
	TYPE_METHOD("__trygetitem__", &default_map___trygetitem__,
	            "(key,def=!N)->\n"
	            "Alias for ${(this as Mapping).get(key, def)"),
	TYPE_METHOD("__trygetitem_index__", &default_map___trygetitem_index__,
	            "(key:?Dint,def=!N)->\n"
	            "Alias for ${(this as Mapping).get(key, def)"),
	TYPE_METHOD("__trygetitem_string__", &default_map___trygetitem_string__,
	            "(key:?X2?DBytes?Dstring,def=!N)->\n"
	            "Alias for ${(this as Mapping).get(key, def)"),
	TYPE_METHOD("__getitem_string__", &default_map___getitem_string__,
	            "(key:?X2?DBytes?Dstring)->\n"
	            "Alias for ${(this as Mapping)[key]}"),
	TYPE_METHOD("__delitem_string__", &default_map___delitem_string__,
	            "(key:?X2?DBytes?Dstring)\n"
	            "Alias for ${del (this as Mapping)[key]}"),
	TYPE_METHOD("__setitem_string__", &default_map___setitem_string__,
	            "(key:?X2?DBytes?Dstring,value)\n"
	            "Alias for ${(this as Mapping)[key] = value}"),
	TYPE_METHOD("__bounditem_string__", &default_map___bounditem_string__,
	            "(key:?X2?DBytes?Dstring,allow_missing=!t)->?Dbool\n"
	            "Alias for ${deemon.bounditem(this as Mapping, key, allow_missing)}"),
	TYPE_METHOD("__hasitem_string__", &default_map___hasitem_string__,
	            "(key:?X2?DBytes?Dstring)->?Dbool\n"
	            "Alias for ${deemon.hasitem(this as Mapping, key)}"),
	TYPE_METHOD("__hash__", &default_map___hash__,
	            "->?Dint\n"
	            "Alias for ${(this as Mapping).operator hash()}"),
	TYPE_METHOD("__compare_eq__", &default_map___compare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping).operator == (rhs)}"),
	TYPE_METHOD("__trycompare_eq__", &default_map___trycompare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${deemon.equals(this as Mapping, rhs)}"),
	TYPE_METHOD("__eq__", &default_map___eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping) == (rhs)}"),
	TYPE_METHOD("__ne__", &default_map___ne__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping) != (rhs)}"),
	TYPE_METHOD("__lo__", &default_map___lo__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping) < (rhs)}"),
	TYPE_METHOD("__le__", &default_map___le__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping) <= (rhs)}"),
	TYPE_METHOD("__gr__", &default_map___gr__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping) > (rhs)}"),
	TYPE_METHOD("__ge__", &default_map___ge__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping) >= (rhs)}"),

	TYPE_METHOD("__add__", &default_map___add__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ${(this as Mapping) + rhs}"),
	TYPE_METHOD("__sub__", &default_map___sub__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ${(this as Mapping) - rhs}"),
	TYPE_METHOD("__and__", &default_map___and__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ${(this as Mapping) & rhs}"),
	TYPE_METHOD("__xor__", &default_map___xor__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ${(this as Mapping) ^ rhs}"),
	TYPE_METHOD("__inplace_add__", &default_map___inplace_add__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Mapping) += rhs}"),
	TYPE_METHOD("__inplace_sub__", &default_map___inplace_sub__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Mapping) -= rhs}"),
	TYPE_METHOD("__inplace_and__", &default_map___inplace_and__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Mapping) &= rhs}"),
	TYPE_METHOD("__inplace_xor__", &default_map___inplace_xor__,
	            "(rhs:?S?O)->?.\n"
	            "Alias for ${(this as Mapping) ^= rhs}"),
	TYPE_METHOD("__or__", &default_map___or__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ?#__and__"),
	TYPE_METHOD("__inplace_or__", &default_map___inplace_or__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ?#__inplace_and__"),

	/* Old function names. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("insert_all", &DeeMH_map_update,
	            "(items:?S?T2?O?O)\n"
	            "A deprecated alias for ?#update"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};




PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
map_printrepr(DeeObject *__restrict self,
              dformatprinter printer, void *arg) {
	dssize_t temp, result;
	DREF DeeObject *iterator;
	DREF DeeObject *elem;
	bool is_first = true;
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	iterator = DeeObject_Iter(self); /* TODO: Use DeeObject_ForeachPair */
	if unlikely(!iterator)
		goto err_m1;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		DREF DeeObject *elem_key_and_value[2];
		if (DeeObject_Unpack(elem, 2, elem_key_and_value))
			goto err_m1_iterator_elem;
		Dee_Decref(elem);
		temp = DeeFormat_Printf(printer, arg, "%s%r: %r",
		                        is_first ? "" : ", ",
		                        elem_key_and_value[0],
		                        elem_key_and_value[1]);
		Dee_Decref(elem_key_and_value[1]);
		Dee_Decref(elem_key_and_value[0]);
		if unlikely(temp < 0)
			goto err_iterator;
		result += temp;
		is_first = false;
		if (DeeThread_CheckInterrupt())
			goto err_m1_iterator;
	}
	Dee_Decref(iterator);
	if unlikely(!elem)
		goto err_m1;
	if (is_first) {
		temp = DeeFormat_PRINT(printer, arg, "}");
	} else {
		temp = DeeFormat_PRINT(printer, arg, " }");
	}
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err_m1_iterator_elem:
	temp = -1;
/*err_iterator_elem:*/
	Dee_Decref(elem);
err_iterator:
	Dee_Decref(iterator);
err:
	return temp;
err_m1_iterator:
	temp = -1;
	goto err_iterator;
err_m1:
	temp = -1;
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_items(DeeObject *self) {
	return DeeSuper_New(&DeeSeq_Type, self);
}

PRIVATE struct type_getset tpconst map_getsets[] = {
	TYPE_GETTER("keys", &default_map_keys,
	            "->?#Keys\n"
	            "Returns a ?DSequence that can be enumerated to view only the keys of @this ?."),
	TYPE_GETTER("values", &default_map_values,
	            "->?#Values\n"
	            "Returns a ?DSequence that can be enumerated to view only the values of @this ?."),
	TYPE_GETTER("items", &map_items,
	            "->?S?T2?O?O\n"
	            "Returns a ?DSequence that can be enumerated to view the key-item "
	            /**/ "pairs as 2-element sequences, the same way they could be viewed "
	            /**/ "if @this ?. itself was being iterated\n"
	            "Same as ${this as Sequence}"),
	TYPE_GETTER("iterkeys", &default_map_iterkeys,
	            "->?#IterKeys\n"
	            "Returns an iterator for ?#{keys}. Same as ${this.keys.operator iter()}"),
	TYPE_GETTER("itervalues", &default_map_itervalues,
	            "->?#IterValues\n"
	            "Returns an iterator for ?#{values}. Same as ${this.values.operator iter()}"),
	TYPE_GETTER("iteritems", &DeeObject_Iter,
	            "->?#Iterator\n"
	            "Returns an iterator for ?#{items}. Same as ${this.operator iter()}"),
	TYPE_GETTER("byattr", &MapByAttr_New,
	            "->?Ert:MappingByAttr\n"
	            "Construct a wrapper for @this mapping that behaves like a generic class object, "
	            /**/ "such that any attribute address ${this.byattr.foo} behaves like ${this[\"foo\"]} "
	            /**/ "(during all of $get, $del and $set).\n"
	            "Note that the returned object doesn't implement the ?DSequence- or ?. "
	            /**/ "interfaces, but instead simply behaves like a completely generic object.\n"
	            "This attribute only makes sense if @this mapping behaves like ${{string: Object}}."),
	TYPE_GETTER(STR_frozen, &DeeRoDict_FromSequence,
	            "->?#Frozen\n"
	            "Returns a read-only (frozen) copy of @this ?."),
	/* TODO: KeyType->?DType
	 *       Check if the type of @this overrides the ?#KeyType class attribute.
	 *       If so, return its value; else, return the common base-class of all
	 *       keys in @this ?.. When @this is empty, ?O is returned.
	 * TODO: ValueType->?DType
	 *       Check if the type of @this overrides the ?#ValueType class attribute.
	 *       If so, return its value; else, return the common base-class of all
	 *       values in @this ?.. When @this is empty, ?O is returned. */
	TYPE_GETSET_END
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Iterator_get(DeeTypeObject *__restrict self) {
	if (self == &DeeMapping_Type)
		return_reference_(&DeeIterator_Type);
	/* TODO: Determine based on linked `tp_iter' */
	err_unknown_attribute_string(self, STR_Iterator, ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Keys_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeSet_Type;
	Dee_mh_map_keys_t tsc_map_keys = DeeType_RequireMapKeys(self);
	if (tsc_map_keys == &DeeMap_DefaultKeysWithMapIterKeys)
		result = &DefaultSequence_MapKeys_Type;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Values_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeSeq_Type;
	Dee_mh_map_values_t tsc_map_values = DeeType_RequireMapValues(self);
	if (tsc_map_values == &DeeMap_DefaultValuesWithMapIterValues)
		result = &DefaultSequence_MapValues_Type;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterKeys_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeIterator_Type;
	Dee_mh_map_iterkeys_t tsc_map_iterkeys = DeeType_RequireMapIterKeys(self);
	if (tsc_map_iterkeys == &DeeObject_DefaultIterKeysWithEnumerate) {
		/* TODO: Custom iterator type that uses "tp_enumerate" */
	} else if (tsc_map_iterkeys == &DeeObject_DefaultIterKeysWithEnumerateIndex) {
		/* TODO: Custom iterator type that uses "tp_enumerate_index" */
	} else if (tsc_map_iterkeys == &DeeSeq_DefaultIterKeysWithSize ||
	           tsc_map_iterkeys == &DeeSeq_DefaultIterKeysWithSizeDefault) {
		result = &SeqIntRangeIterator_Type;
	} else if (tsc_map_iterkeys == &DeeSeq_DefaultIterKeysWithSizeOb) {
		result = &SeqRangeIterator_Type;
	} else if (tsc_map_iterkeys == &DeeMap_DefaultIterKeysWithIter) {
		result = &DefaultIterator_WithNextKey;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterValues_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeIterator_Type;
	Dee_mh_map_itervalues_t tsc_map_itervalues = DeeType_RequireMapIterValues(self);
	if (tsc_map_itervalues == &DeeMap_DefaultIterValuesWithIter) {
		result = &DefaultIterator_WithNextValue;
	}
	return_reference_(result);
}



/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("Frozen");
]]]*/
#define Dee_HashStr__Frozen _Dee_HashSelectC(0xa7ed3902, 0x16013e56a91991ea)
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Frozen_get(DeeTypeObject *__restrict self) {
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
		return_reference_(&DeeRoDict_Type);
	if (info.a_attrtype) {
		result = info.a_attrtype;
		Dee_Incref(result);
	} else if (info.a_decl == (DeeObject *)&DeeMapping_Type) {
		result = &DeeRoDict_Type;
		Dee_Incref(&DeeRoDict_Type);
	} else {
		if (info.a_doc) {
			/* TODO: Use doc meta-data to determine the return type! */
		}
		/* Fallback: just tell the caller what they already know: a Mapping will be returned... */
		result = &DeeMapping_Type;
		Dee_Incref(&DeeMapping_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst map_class_getsets[] = {
	TYPE_GETTER(STR_Iterator, &map_Iterator_get,
	            "->?DType\n"
	            "Returns the iterator class used by instances of @this ?. type\n"
	            "This member must be overwritten by sub-classes of ?."),
	TYPE_GETTER("Frozen", &map_Frozen_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#i:frozen property"),
	TYPE_GETTER("Keys", &map_Keys_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#i:keys property"),
	TYPE_GETTER("Values", &map_Values_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#i:values property"),
	TYPE_GETTER("IterKeys", &map_IterKeys_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#i:iterkeys property"),
	TYPE_GETTER("IterValues", &map_IterValues_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#i:itervalues property"),
	TYPE_GETSET_END
};

INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));

PRIVATE struct type_operator const map_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL),
};

/* `Mapping from deemon' */
PUBLIC DeeTypeObject DeeMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Mapping),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any Mapping "
	                         /**/ "type that wishes to implement a key-value protocol\n"
	                         "An object derived from this class must implement ${operator iter}, "
	                         /**/ "and preferrably (but optionally) or ${operator []} (getitem)\n"
	                         "The abstract declaration of a mapping-like sequence is ${{Object: Object}} or ${{(Object, Object)...}}\n"
	                         "\n"

	                         "()\n"
	                         "A no-op default constructor that is implicitly called by sub-classes\n"
	                         "When invoked manually, a general-purpose, empty ?. is returned\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the representation of all sequence elements, "
	                         /**/ "using abstract mapping syntax\n"
	                         "e.g.: ${{ \"foo\": 10, \"bar\": \"baz\" }}\n"
	                         "\n"

	                         "[:]->!D\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating key-value pairs as 2-elements sequences"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_MAP << Dee_TF_SEQCLASS_SHFT),
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
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ &DeeMap_OperatorBool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&map_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &DeeMap_OperatorMath,
	/* .tp_cmp           = */ &DeeMap_OperatorCmp,
	/* .tp_seq           = */ &DeeMap_OperatorSeq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ map_methods,
	/* .tp_getsets       = */ map_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ map_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ map_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(map_operators)
};


/* An empty instance of a generic mapping object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeMapping_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeMapping_Type' should be considered stub/empty. */
PUBLIC DeeObject DeeMapping_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeMapping_Type)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_MAP_C */
