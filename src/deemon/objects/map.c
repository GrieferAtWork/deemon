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
#ifndef GUARD_DEEMON_OBJECTS_MAP_C
#define GUARD_DEEMON_OBJECTS_MAP_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/class.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
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
#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/operator-require.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/byattr.h"
#include "seq/default-api.h"
#include "seq/default-iterators.h"
#include "seq/default-map-proxy.h"
#include "seq/each.h"
#include "seq/enumerate-cb.h"
#include "seq/hashfilter.h"
#include "seq/range.h"
#include "seq/unique-iterator.h"

DECL_BEGIN

DOC_DEF(map_byhash_doc,
        "(template:?O)->?S?T2?O?O\n" /* TODO: This should return ?M?O?O */
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

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("cb");
print define_Dee_HashStr("start");
print define_Dee_HashStr("end");
]]]*/
#define Dee_HashStr__cb _Dee_HashSelectC(0x75ffadba, 0x2501dbb50208b92e)
#define Dee_HashStr__start _Dee_HashSelectC(0xa2ed6890, 0x80b621ce3c3982d5)
#define Dee_HashStr__end _Dee_HashSelectC(0x37fb4a05, 0x6de935c204dc3d01)
/*[[[end]]]*/

#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
PRIVATE ATTR_COLD int DCALL
err_map_enumerate_start_but_no_end(void) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Mapping.enumerate: `start' given, but no `end'");
}

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
do_map_enumerate_with_kw(DeeObject *self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeObject *cb, *start, *end;
	DeeKwArgs kwds;
	if (DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
	switch (argc) {

	case 0: {
		if unlikely((cb = DeeKwArgs_TryGetItemNRStringHash(&kwds, "cb", Dee_HashStr__cb)) == NULL)
			goto err;
		if unlikely((start = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
			goto err;
		if unlikely((end = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (cb != ITER_DONE) {
handle_with_cb:
			if (end != ITER_DONE) {
				if (start != ITER_DONE) {
					result = map_call_enumerate_with_range(self, cb, start, end);
				} else {
					start = DeeObject_NewDefault(Dee_TYPE(end));
					if unlikely(!start)
						goto err;
					result = map_call_enumerate_with_range(self, cb, start, end);
					Dee_Decref(start);
				}
			} else if (start == ITER_DONE) {
				result = map_call_enumerate(self, cb);
			} else {
				goto err_start_but_no_end;
			}
		} else {
			if (end != ITER_DONE) {
				if (start != ITER_DONE) {
					result = DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, start, end);
				} else {
					start = DeeObject_NewDefault(Dee_TYPE(end));
					if unlikely(!start)
						goto err;
					result = DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, start, end);
					Dee_Decref(start);
				}
			} else if (start == ITER_DONE) {
				result = DeeObject_InvokeMethodHint(map_makeenumeration, self);
			} else {
				goto err_start_but_no_end;
			}
		}
	}	break;

	case 1: {
		cb = argv[0];
		if unlikely((end = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (DeeCallable_Check(cb)) {
			if unlikely((start = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
				goto err;
			goto handle_with_cb;
		}
		start = cb;
		if (end != ITER_DONE) {
			result = DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, start, end);
		} else {
			goto err_start_but_no_end;
		}
	}	break;

	case 2: {
		cb = argv[0];
		if (DeeCallable_Check(cb)) {
			if unlikely((end = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
				goto err;
			start = argv[1];
			goto handle_with_cb;
		}
		start = argv[0];
		end   = argv[1];
		result = DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, start, end);
	}	break;

	case 3: {
		cb     = argv[0];
		start  = argv[1];
		end    = argv[2];
		result = map_call_enumerate_with_range(self, cb, start, end);
	}	break;

	default:
		goto err_bad_args;
	}
	if unlikely(DeeKwArgs_Done(&kwds, argc, "enumerate"))
		goto err_r;
	return result;
err_start_but_no_end:
	err_map_enumerate_start_but_no_end();
	goto err;
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
	goto err;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_enumerate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if unlikely(kw)
		return do_map_enumerate_with_kw(self, argc, argv, kw);
	if likely(argc == 0)
		return DeeObject_InvokeMethodHint(map_makeenumeration, self);
	if (DeeCallable_Check(argv[0])) {
		if (argc == 1)
			return map_call_enumerate(self, argv[0]);
		if unlikely(argc == 2)
			goto err_start_but_no_end;
		if (argc != 3)
			goto err_bad_args;
		return map_call_enumerate_with_range(self, argv[0], argv[1], argv[2]);
	} else {
		if unlikely(argc == 1)
			goto err_start_but_no_end;
		if (argc != 2)
			goto err_bad_args;
		return DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, argv[0], argv[1]);
	}
	__builtin_unreachable();
err_start_but_no_end:
	err_map_enumerate_start_but_no_end();
	goto err;
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
err:
	return NULL;
}
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */



PRIVATE struct type_method tpconst map_methods[] = {
	/* Default operations for all mappings. */
	TYPE_METHOD(STR_get, &map_get,
	            "(key,def=!N)->\n"
	            "#r{The value associated with @key or @def when @key has no value associated}"),
	TYPE_KWMETHOD("byhash", &map_byhash, DOC_GET(map_byhash_doc)),

#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	TYPE_KWMETHOD(STR_enumerate, &map_enumerate,
	              "->?S?T2?O?O\n"
	              "(start,end)->?S?T2?O?O\n"
	              "(cb:?DCallable)->?X2?O?N\n"
	              "(cb:?DCallable,start,end)->?X2?O?N\n"
	              "Enumerate keys and associated values of @this mapping\n"
	              "This function can be used to easily enumerate mapping keys and values, "
	              /**/ "including being able to enumerate keys that are currently unbound"),
	TYPE_METHOD(STR_union, &DeeMA_Mapping_union,
	            "(rhs:?X3?.?M?O?O?S?T2?O?O)->?.\n"
	            "Same as ${(this as Mapping) | rhs}"),
	TYPE_METHOD(STR_difference, &DeeMA_Mapping_difference,
	            "(keys:?X2?DSet?S?O)->?.\n"
	            "Same as ${(this as Mapping) - keys}"),
	TYPE_METHOD(STR_intersection, &DeeMA_Mapping_intersection,
	            "(keys:?X2?DSet?S?O)->?.\n"
	            "Same as ${(this as Mapping) & keys}"),
	TYPE_METHOD(STR_symmetric_difference, &DeeMA_Mapping_symmetric_difference,
	            "(rhs:?X2?M?O?O?S?T2?O?O)->?.\n"
	            "Same as ${(this as Mapping) ^ rhs}"),
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

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
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	TYPE_METHOD("__enumerate__", &default_map___enumerate__,
	            "(cb)->?X2?O?N\n"
	            "(cb,start,end)->?X2?O?N\n"
	            "Alias for ${(this as Mapping).enumerate(cb[,start,end])}"),
	TYPE_METHOD("__enumerate_items__", &DeeMA___map_enumerate_items__,
	            "()->?T2?O?O\n"
	            "(start,end)->?T2?O?O\n"
	            "Alias for ${(this as Mapping).enumerate([start,end])}"),
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
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
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_METHOD("__compare_eq__", &default_map___compare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${(this as Mapping).operator == (rhs)}"),
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	TYPE_METHOD("__trycompare_eq__", &default_map___trycompare_eq__,
	            "(rhs:?S?O)->?Dbool\n"
	            "Alias for ${deemon.equals(this as Mapping, rhs)}"),
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
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
	            "Alias for ?#__add__"),
	TYPE_METHOD("__inplace_or__", &default_map___inplace_or__,
	            "(rhs:?S?O)->?DMapping\n"
	            "Alias for ?#__inplace_add__"),

	/* Old function names. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("insert_all", &DeeMH_map_update,
	            "(items:?S?T2?O?O)\n"
	            "A deprecated alias for ?#update"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	TYPE_METHOD_HINTREF(explicit_seq_any),
	TYPE_METHOD_HINTREF(explicit_seq_all),
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_METHOD_END
};

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
PRIVATE WUNUSED NONNULL((1)) int DCALL
map_mh_seq_any_with_range(DeeObject *self, size_t start, size_t end) {
	size_t map_size;
	if (start <= end)
		return 0;
	if (start == 0)
		return DeeMap_OperatorBool(self);
	map_size = DeeMap_OperatorSize(self);
	if unlikely(map_size == (size_t)-1)
		goto err;
	return start < map_size;
err:
	return -1;
}

PRIVATE struct type_method_hint tpconst map_method_hints[] = {
	/* Mappings are made up of non-empty tuples, so there is never an sequence-like elem that is "false"
	 * In other words: Mapping.all() is always true. */
	TYPE_METHOD_HINT_F(seq_all, &_DeeNone_reti1_1, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &DeeSeq_DefaultAllWithKeyWithSeqForeach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, (int (DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_reti1_3, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &DeeSeq_DefaultAllWithRangeAndKeyWithSeqEnumerateIndex, METHOD_FNOREFESCAPE),

	/* Mappings to "seq_all", all sequence-like map items are "true",
	 * so "Mapping.any()" is true if the map is non-empty. */
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	TYPE_METHOD_HINT_F(seq_any, &default__seq_operator_bool, METHOD_FNOREFESCAPE),
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_METHOD_HINT_F(seq_any, &DeeMap_OperatorBool, METHOD_FNOREFESCAPE),
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_METHOD_HINT_F(seq_any_with_key, &DeeSeq_DefaultAnyWithKeyWithSeqForeach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &map_mh_seq_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &DeeSeq_DefaultAnyWithRangeAndKeyWithSeqEnumerateIndex, METHOD_FNOREFESCAPE),

	TYPE_METHOD_HINT_END
};
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */




DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_printrepr(DeeObject *__restrict self,
              dformatprinter printer, void *arg) {
	Dee_ssize_t temp, result;
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
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	return DeeSuper_New(&DeeSet_Type, self);
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	return DeeSuper_New(&DeeSeq_Type, self);
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
}

PRIVATE struct type_getset tpconst map_getsets[] = {
	TYPE_GETTER("keys", &default_map_keys,
	            "->?#Keys\n"
	            "Returns a ?DSequence that can be enumerated to view only the keys of @this ?."),
	TYPE_GETTER("values", &default_map_values,
	            "->?#Values\n"
	            "Returns a ?DSequence that can be enumerated to view only the values of @this ?."),
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
	TYPE_GETTER_AB("items", &map_items,
	               "->?S?T2?O?O\n"
	               "Returns a ?DSet that can be enumerated to view the key-item "
	               /**/ "pairs as 2-element sequences, the same way they could be viewed "
	               /**/ "if @this ?. itself was being iterated\n"
	               "Same as ${this as Sequence}"),
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_GETTER("items", &map_items,
	            "->?S?T2?O?O\n"
	            "Returns a ?DSequence that can be enumerated to view the key-item "
	            /**/ "pairs as 2-element sequences, the same way they could be viewed "
	            /**/ "if @this ?. itself was being iterated\n"
	            "Same as ${this as Sequence}"),
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_GETTER("iterkeys", &default_map_iterkeys,
	            "->?#IterKeys\n"
	            "Returns an iterator for ?#{keys}. Same as ${this.keys.operator iter()}"),
	TYPE_GETTER("itervalues", &default_map_itervalues,
	            "->?#IterValues\n"
	            "Returns an iterator for ?#{values}. Same as ${this.values.operator iter()}"),
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#define default__map_operator_iter DeeObject_Iter
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_GETTER("iteritems", &default__map_operator_iter,
	            "->?#Iterator\n"
	            "Returns an iterator for ?#{items}. Same as ${this.operator iter()}"),
	TYPE_GETTER_AB("byattr", &MapByAttr_New,
	               "->?Ert:MapByAttr\n"
	               "Construct a wrapper for @this mapping that behaves like a generic class object, "
	               /**/ "such that any attribute address ${this.byattr.foo} behaves like ${this[\"foo\"]} "
	               /**/ "(during all of $get, $del and $set).\n"
	               "Note that the returned object doesn't implement the ?DSequence- or ?. "
	               /**/ "interfaces, but instead simply behaves like a completely generic object.\n"
	               "This attribute only makes sense if @this mapping behaves like ${{string: Object}}."),
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#define default__map_frozen DeeRoDict_FromSequence
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
	TYPE_GETTER(STR_frozen, &default__map_frozen,
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



#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Frozen_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSeq_Type;
	DeeMH_map_frozen_t map_frozen = DeeType_RequireMethodHint(self, map_frozen);
	if (map_frozen == &DeeObject_NewRef) {
		result = self;
	} else if (map_frozen == &DeeRoDict_FromSequence) {
		result = &DeeRoDict_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Iterator_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_map_operator_iter_t map_operator_iter = DeeType_RequireMethodHint(self, map_operator_iter);
	if (map_operator_iter == &default__map_operator_iter__with__seq_operator_iter) {
		result = &DistinctMappingIterator_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Keys_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSet_Type;
	DeeMH_map_keys_t map_keys = DeeType_RequireMethodHint(self, map_keys);
	if (map_keys == &default__map_keys__with__map_iterkeys) {
		result = &DefaultSequence_MapKeys_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Values_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSeq_Type;
	DeeMH_map_values_t map_values = DeeType_RequireMethodHint(self, map_values);
	if (map_values == &default__map_values__with__map_itervalues) {
		result = &DefaultSequence_MapValues_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterKeys_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_map_iterkeys_t map_iterkeys = DeeType_RequireMethodHint(self, map_iterkeys);
	if (map_iterkeys == &default__map_iterkeys__with__map_enumerate) {
		/* TODO: Custom iterator type that uses "tp_enumerate" */
	} else if (map_iterkeys == &default__map_iterkeys__with__map_operator_iter) {
		result = &DefaultIterator_WithNextKey;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterValues_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_map_itervalues_t map_itervalues = DeeType_RequireMethodHint(self, map_itervalues);
	if (map_itervalues == &default__map_itervalues__with__map_operator_iter) {
		result = &DefaultIterator_WithNextValue;
	}
	return_reference_(result);
}
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
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
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


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


#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
PRIVATE struct type_math map_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ DEFIMPL(&default__set_operator_inv),
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &default__map_operator_add,
	/* .tp_sub         = */ &default__map_operator_sub,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ &default__map_operator_and,
	/* .tp_or          = */ &default__map_operator_add,
	/* .tp_xor         = */ &default__map_operator_xor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ &default__map_operator_inplace_add,
	/* .tp_inplace_sub = */ &default__map_operator_inplace_sub,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ &default__map_operator_inplace_and,
	/* .tp_inplace_or  = */ &default__map_operator_inplace_add,
	/* .tp_inplace_xor = */ &default__map_operator_inplace_xor,
	/* .tp_inplace_pow = */ NULL,
};

PRIVATE struct type_cmp map_cmp = {
	/* .tp_hash          = */ &default__set_operator_hash,
	/* .tp_compare_eq    = */ &default__map_operator_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &default__map_operator_trycompare_eq,
	/* .tp_eq            = */ &default__map_operator_eq,
	/* .tp_ne            = */ &default__map_operator_ne,
	/* .tp_lo            = */ &default__map_operator_lo,
	/* .tp_le            = */ &default__map_operator_le,
	/* .tp_gr            = */ &default__map_operator_gr,
	/* .tp_ge            = */ &default__map_operator_ge,
};

PRIVATE struct type_seq map_seq = {
	/* .tp_iter                       = */ &default__map_operator_iter,
	/* .tp_sizeob                     = */ &default__map_operator_sizeob,
	/* .tp_contains                   = */ &default__map_operator_contains,
	/* .tp_getitem                    = */ &default__map_operator_getitem,
	/* .tp_delitem                    = */ &default__map_operator_delitem,
	/* .tp_setitem                    = */ &default__map_operator_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ &default__map_operator_foreach_pair,
	/* ._deprecated_tp_enumerate      = */ NULL,
	/* ._deprecated_tp_enumerate_index= */ NULL,
	/* ._deprecated_tp_iterkeys       = */ NULL,
	/* .tp_bounditem                  = */ &default__map_operator_bounditem,
	/* .tp_hasitem                    = */ &default__map_operator_hasitem,
	/* .tp_size                       = */ &default__map_operator_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ &default__map_operator_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ &default__map_operator_delitem_index,
	/* .tp_setitem_index              = */ &default__map_operator_setitem_index,
	/* .tp_bounditem_index            = */ &default__map_operator_bounditem_index,
	/* .tp_hasitem_index              = */ &default__map_operator_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ &default__map_operator_trygetitem,
	/* .tp_trygetitem_index           = */ &default__map_operator_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ &default__map_operator_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ &default__map_operator_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ &default__map_operator_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ &default__map_operator_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ &default__map_operator_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ &default__map_operator_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ &default__map_operator_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ &default__map_operator_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ &default__map_operator_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ &default__map_operator_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ &default__map_operator_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ &default__map_operator_hasitem_string_len_hash,
	/* .tp_asvector                   = */ NULL,
	/* .tp_asvector_nothrow           = */ NULL,
};
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#ifdef CONFIG_NO_DOC
#define map_doc NULL
#else /* CONFIG_NO_DOC */
PRIVATE char const map_doc[] =
"A recommended abstract base class for any Mapping "
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
"Returns an iterator for enumerating key-value pairs as 2-elements sequences";
#endif /* !CONFIG_NO_DOC */




/* `Mapping from deemon' */
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
PUBLIC DeeTypeObject DeeMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Mapping),
	/* .tp_doc      = */ map_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_MAP << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&DeeNone_OperatorCtor, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ &default__seq_operator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&map_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &map_math,
	/* .tp_cmp           = */ &map_cmp,
	/* .tp_seq           = */ &map_seq,
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
	/* .tp_operators_size= */ COMPILER_LENOF(map_operators),
	/* .tp_mhcache       = */ &mh_cache_empty,
};
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
/* Prevent the computed-operator system from seeing this one */
#define _old_DeeMapping_Type DeeMapping_Type
#define _old_DeeMapping_name DeeString_STR(&str_Mapping)
PUBLIC DeeTypeObject _old_DeeMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ _old_DeeMapping_name,
	/* .tp_doc      = */ map_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_MAP << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&DeeNone_OperatorCtor, /* Allow default-construction of sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
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
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&map_printrepr
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
	/* .tp_method_hints  = */ map_method_hints,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ map_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(map_operators)
};
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


/* An empty instance of a generic mapping object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeMapping_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeMapping_Type' should be considered stub/empty. */
PUBLIC DeeObject DeeMapping_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeMapping_Type)
};


#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
/* Wrapper for `DeeObject_BoolInherited(DeeObject_InvokeMethodHint(map_operator_contains, self, key))' */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeMap_OperatorContainsAsBool(DeeObject *self,
                              DeeObject *key) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(map_operator_contains, self, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_MAP_C */
