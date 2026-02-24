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
#ifndef GUARD_DEEMON_OBJECTS_MAP_C
#define GUARD_DEEMON_OBJECTS_MAP_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>                /* DeeArg_Unpack* */
#include <deemon/bool.h>               /* return_bool */
#include <deemon/callable.h>           /* DeeCallable_Check */
#include <deemon/computed-operators.h> /* DEFAULT_OPIMP, DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error.h>              /* DeeError_Throwf, DeeError_TypeError */
#include <deemon/format.h>             /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/kwds.h>               /* DeeKwArgs* */
#include <deemon/map.h>                /*  */
#include <deemon/method-hints.h>       /* DeeMA_*, DeeMH_*_t, DeeObject_InvokeMethodHint, DeeType_HasTrait, DeeType_RequireMethodHint, DeeType_TRAIT___map_getitem_always_bound__ */
#include <deemon/none-operator.h>      /* DeeNone_Operator* */
#include <deemon/none.h>               /* DeeNone_Type, Dee_None */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_Decref_likely, Dee_Incref, Dee_TYPE, Dee_formatprinter_t, Dee_ssize_t, ITER_DONE, OBJECT_HEAD_INIT, _Dee_HashSelectC, return_reference_ */
#include <deemon/rodict.h>             /* DeeRoDict_FromSequence, DeeRoDict_Type */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type, Dee_SEQCLASS_MAP */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/type.h>               /* DeeType_Type, Dee_TF_SEQCLASS_SHFT, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, METHOD_F*, OPERATOR_*, TF_NONE, TP_F*, TYPE_*, type_* */

#include "../runtime/kwlist.h"
#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/byattr.h"
#include "seq/default-iterators.h"
#include "seq/default-map-proxy.h"
#include "seq/enumerate-cb.h"
#include "seq/hashfilter.h"
#include "seq/map-fromattr.h"
#include "seq/map-fromkeys.h"
#include "seq/unique-iterator.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_byhash(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("byhash", params: "
	DeeObject *template
", docStringPrefix: "map");]]]*/
#define map_byhash_params "template"
	struct {
		DeeObject *template_;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__template, "o:byhash", &args))
		goto err;
/*[[[end]]]*/
	return DeeMap_HashFilter(self, DeeObject_Hash(args.template_));
err:
	return NULL;
}

DOC_DEF(map_byhash_doc,
        "(" map_byhash_params ")->?S?T2?O?O\n" /* TODO: This should return ?M?O?O */
        "#ptemplate{The object who's hash should be used to search for collisions}"
        "Same as ?Abyhash?DSequence, but rather than comparing the hashes of the "
        /**/ "key-value pairs, search for pairs where the key matches the hash of @template");


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_get(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("get", params: "
	DeeObject *key;
	DeeObject *def = Dee_None;
", docStringPrefix: "map");]]]*/
#define map_get_params "key,def=!N"
	struct {
		DeeObject *key;
		DeeObject *def;
	} args;
	args.def = Dee_None;
	DeeArg_UnpackStruct1Or2(err, argc, argv, "get", &args, &args.key, &args.def);
/*[[[end]]]*/
	result = DeeObject_InvokeMethodHint(map_operator_trygetitem, self, args.key);
	if (result == ITER_DONE) {
		Dee_Incref(args.def);
		result = args.def;
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
	Dee_Decref_likely(result);
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



PRIVATE struct type_method tpconst map_methods[] = {
	/* Default operations for all mappings. */
	TYPE_METHOD(STR_get, &map_get,
	            "(" map_get_params ")->\n"
	            "#r{The value associated with @key or @def when @key has no value associated}"),
	TYPE_KWMETHOD("byhash", &map_byhash, DOC_GET(map_byhash_doc)),

	TYPE_KWMETHOD(STR_enumerate, &map_enumerate,
	              "->?S?T2?O?O\n"
	              "(start,end)->?S?T2?O?O\n"
	              "(cb:?DCallable)->?X2?O?N\n"
	              "(cb:?DCallable,start,end)->?X2?O?N\n"
	              "Enumerate keys and associated values of @this mapping\n"
	              "This function can be used to easily enumerate mapping keys and values, "
	              /**/ "including being able to enumerate keys that are currently unbound"),

	/* TODO: filter(keep:?DCallable)->?DMapping
	 * Returns a mapping representing the sub-set of items where "keep(key)" returns true */

	/* TODO: ubfilter(keep:?DCallable)->?DMapping
	 * Same as filter(keep), but filtered items appear as unbound, rather than absent */

	/* Method hint API functions. */
	TYPE_METHOD(DeeMA_Mapping_equals_name, &DeeMA_Mapping_equals,
	            "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool\n"
	            "Used to implement ?Dequals for Mapping objects. Calling this function "
	            "directly is equivalent to ${deemon.equals(this as Mapping, rhs)}\n"
	            "#T{Requirements|Implementation~"
	            /**/ "TODO|TODO&"
	            /**/ "TODO|TODO"
	            "}"),
	TYPE_METHOD(DeeMA_Mapping_union_name, &DeeMA_Mapping_union,
	            "" DeeMA_Mapping_union_doc "\n"
	            "Same as ${(this as Mapping) | rhs}"),
	TYPE_METHOD(DeeMA_Mapping_difference_name, &DeeMA_Mapping_difference,
	            "" DeeMA_Mapping_difference_doc "\n"
	            "Same as ${(this as Mapping) - keys}"),
	TYPE_METHOD(DeeMA_Mapping_intersection_name, &DeeMA_Mapping_intersection,
	            "" DeeMA_Mapping_intersection_doc "\n"
	            "Same as ${(this as Mapping) & keys}"),
	TYPE_METHOD(DeeMA_Mapping_symmetric_difference_name, &DeeMA_Mapping_symmetric_difference,
	            "" DeeMA_Mapping_symmetric_difference_doc "\n"
	            "Same as ${(this as Mapping) ^ rhs}"),
	TYPE_METHOD(DeeMA_Mapping_setold_name, &DeeMA_Mapping_setold,
	            "" DeeMA_Mapping_setold_doc "\n"
	            "#r{Indicative of @value having been assigned to @key}"
	            "Assign @value to @key, only succeeding when @key already existed to begin with"),
	TYPE_METHOD(DeeMA_Mapping_setold_ex_name, &DeeMA_Mapping_setold_ex,
	            "" DeeMA_Mapping_setold_ex_doc "\n"
	            "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	            "Same as ?#setold but also return the previously assigned value"),
	TYPE_METHOD(DeeMA_Mapping_setnew_name, &DeeMA_Mapping_setnew,
	            "" DeeMA_Mapping_setnew_doc "\n"
	            "#r{Indicative of @value having been assigned to @key}"
	            "Assign @value to @key, only succeeding when @key didn't exist before"),
	TYPE_METHOD(DeeMA_Mapping_setnew_ex_name, &DeeMA_Mapping_setnew_ex,
	            "" DeeMA_Mapping_setnew_ex_doc "\n"
	            "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	            "Same as ?#setnew but return the previously assigned value on failure"),
	TYPE_METHOD(DeeMA_Mapping_setdefault_name, &DeeMA_Mapping_setdefault,
	            "" DeeMA_Mapping_setdefault_doc "\n"
	            "#r{The object currently assigned to @key}"
	            "Lookup @key in @this ?. and return its value if found. "
	            /**/ "Otherwise, assign @def to @key and return it instead"),
	TYPE_METHOD(DeeMA_Mapping_update_name, &DeeMA_Mapping_update,
	            "" DeeMA_Mapping_update_doc "\n"
	            "Iterate @items and unpack each element into 2 others, "
	            /**/ "using them as key and value to insert into @this ?."),
	TYPE_METHOD(DeeMA_Mapping_remove_name, &DeeMA_Mapping_remove,
	            "" DeeMA_Mapping_remove_doc "\n"
	            "Similar to ?#{op:delitem}, but returns indicative of a @key having been removed"),
	TYPE_METHOD(DeeMA_Mapping_removekeys_name, &DeeMA_Mapping_removekeys,
	            "" DeeMA_Mapping_removekeys_doc "\n"
	            "Remove all of the given @keys from @this mapping. "
	            /**/ "Same as ${for (local k: keys) del this[k];}"),
	TYPE_METHOD(DeeMA_Mapping_pop_name, &DeeMA_Mapping_pop,
	            "" DeeMA_Mapping_pop_doc "\n"
	            "#tKeyError{No @def was given and @key was not found}"
	            "Delete @key from @this ?. and return its previously assigned "
	            /**/ "value or @def when @key had no item associated"),
	TYPE_METHOD(DeeMA_Mapping_popitem_name, &DeeMA_Mapping_popitem,
	            "" DeeMA_Mapping_popitem_doc "\n"
	            "#r{A random pair key-value pair that has been removed, or !N if the mapping was empty}"
	            "Remove a random key-value pair from @this mapping, and return it. "
	            /**/ "If the mapping is empty, return !N instead"),

	/* Method hint operator invocation. */
	TYPE_METHOD("__iter__", &DeeMA___map_iter__, "->?#Iterator\nAlias for ${(this as Mapping).operator iter()} (s.a. ?#{op:iter})"),
	TYPE_METHOD("__size__", &DeeMA___map_size__, DeeMA___map_size___doc "\nAlias for ${##(this as Mapping)} (s.a. ?#{op:size})"),
	TYPE_METHOD("__hash__", &DeeMA___map_hash__, DeeMA___map_hash___doc "\nAlias for ${(this as Mapping).operator hash()} (s.a. ?#{op:hash})"),
	TYPE_METHOD("__getitem__", &DeeMA___map_getitem__, DeeMA___map_getitem___doc "\nAlias for ${(this as Mapping)[key]} (s.a. ?#{op:getitem})"),
	TYPE_METHOD("__delitem__", &DeeMA___map_delitem__, DeeMA___map_delitem___doc "\nAlias for ${del (this as Mapping)[key]} (s.a. ?#{op:delitem})"),
	TYPE_METHOD("__setitem__", &DeeMA___map_setitem__, DeeMA___map_setitem___doc "\nAlias for ${(this as Mapping)[key] = value} (s.a. ?#{op:setitem})"),
	TYPE_METHOD("__contains__", &DeeMA___map_contains__, DeeMA___map_contains___doc "\nAlias for ${item in (this as Mapping)} (s.a. ?#{op:contains})"),
	TYPE_METHOD("__enumerate__", &DeeMA___map_enumerate__, DeeMA___map_enumerate___doc "\nAlias for ${(this as Mapping).enumerate(cb[,start,end])} (s.a. ?#enumerate)"),
	TYPE_METHOD("__enumerate_items__", &DeeMA___map_enumerate_items__, DeeMA___map_enumerate_items___doc "\nAlias for ${(this as Mapping).enumerate([start,end])} (s.a. ?#enumerate)"),
	TYPE_METHOD("__compare_eq__", &DeeMA___map_compare_eq__, "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool\nAlias for ${Mapping.equals(this, rhs)} (s.a. ?#equals)"),
	TYPE_METHOD("__eq__", &DeeMA___map_eq__, DeeMA___map_eq___doc "\nAlias for ${(this as Mapping) == (rhs)} (s.a. ?#{op:eq})"),
	TYPE_METHOD("__ne__", &DeeMA___map_ne__, DeeMA___map_ne___doc "\nAlias for ${(this as Mapping) != (rhs)} (s.a. ?#{op:ne})"),
	TYPE_METHOD("__lo__", &DeeMA___map_lo__, DeeMA___map_lo___doc "\nAlias for ${(this as Mapping) < (rhs)} (s.a. ?#{op:lo})"),
	TYPE_METHOD("__le__", &DeeMA___map_le__, DeeMA___map_le___doc "\nAlias for ${(this as Mapping) <= (rhs)} (s.a. ?#{op:le})"),
	TYPE_METHOD("__gr__", &DeeMA___map_gr__, DeeMA___map_gr___doc "\nAlias for ${(this as Mapping) > (rhs)} (s.a. ?#{op:gr})"),
	TYPE_METHOD("__ge__", &DeeMA___map_ge__, DeeMA___map_ge___doc "\nAlias for ${(this as Mapping) >= (rhs)} (s.a. ?#{op:ge})"),
	TYPE_METHOD("__add__", &DeeMA___map_add__, DeeMA___map_add___doc "\nAlias for ${(this as Mapping) + rhs}, ${(this as Mapping) | rhs} (s.a. ?#union, ?#{op:add}, ?#{op:or}, ?#__or__)"),
	TYPE_METHOD("__sub__", &DeeMA___map_sub__, DeeMA___map_sub___doc "\nAlias for ${(this as Mapping) - keys} (s.a. ?#difference, ?#{op:sub})"),
	TYPE_METHOD("__and__", &DeeMA___map_and__, DeeMA___map_and___doc "\nAlias for ${(this as Mapping) & keys} (s.a. ?#intersection, ?#{op:and})"),
	TYPE_METHOD("__xor__", &DeeMA___map_xor__, DeeMA___map_xor___doc "\nAlias for ${(this as Mapping) ^ rhs} (s.a. ?#symmetric_difference, ?#{op:xor})"),
	TYPE_METHOD("__inplace_add__", &DeeMA___map_inplace_add__, DeeMA___map_inplace_add___doc "\nAlias for ${(this as Mapping) += rhs} (s.a. ?#{op:iadd}, ?#{op:ior})"),
	TYPE_METHOD("__inplace_sub__", &DeeMA___map_inplace_sub__, DeeMA___map_inplace_sub___doc "\nAlias for ${(this as Mapping) -= keys} (s.a. ?#{op:isub})"),
	TYPE_METHOD("__inplace_and__", &DeeMA___map_inplace_and__, DeeMA___map_inplace_and___doc "\nAlias for ${(this as Mapping) &= keys} (s.a. ?#{op:iand})"),
	TYPE_METHOD("__inplace_xor__", &DeeMA___map_inplace_xor__, DeeMA___map_inplace_xor___doc "\nAlias for ${(this as Mapping) ^= rhs} (s.a. ?#{op:ixor})"),
	TYPE_METHOD("__or__", &DeeMA___map_add__, DeeMA___map_add___doc "\nAlias for ?#__add__ (s.a. ?#union, ?#{op:add}, ?#{op:or}, ?#__add__)"),
	TYPE_METHOD("__inplace_or__", &DeeMA___map_inplace_add__, DeeMA___map_inplace_add___doc "\nAlias for ?#__inplace_add__ (s.a. ?#{op:iadd}, ?#{op:ior})"),

	/* Old function names. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("insert_all", &DeeMA_Mapping_update,
	            "" DeeMA_Mapping_update_doc "\n"
	            "A deprecated alias for ?#update"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};

struct map_printrepr_foreach_data {
	Dee_formatprinter_t mprf_printer; /* [1..1] Wrapped printer callback */
	void               *mprf_arg;     /* [?..?] Cookie for `mprf_printer' */
	bool                mprf_isfirst; /* True if this is the first item */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
map_printrepr_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	Dee_ssize_t temp, result;
	struct map_printrepr_foreach_data *data;
	data = (struct map_printrepr_foreach_data *)arg;
	if (data->mprf_isfirst) {
		data->mprf_isfirst = false;
		result = 0;
	} else {
		result = DeeFormat_PRINT(data->mprf_printer, data->mprf_arg, ", ");
		if unlikely(result < 0)
			goto done;
	}
#ifdef __OPTIMIZE_SIZE__
	temp = DeeFormat_Printf(data->mprf_printer, data->mprf_arg, "%r: %r", key, value);
#else /* __OPTIMIZE_SIZE__ */
	temp = DeeObject_PrintRepr(key, data->mprf_printer, data->mprf_arg);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(data->mprf_printer, data->mprf_arg, ": ");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeObject_PrintRepr(value, data->mprf_printer, data->mprf_arg);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}


DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_printrepr(DeeObject *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	struct map_printrepr_foreach_data data;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	data.mprf_printer = printer;
	data.mprf_arg     = arg;
	data.mprf_isfirst = true;
	temp = DeeObject_ForeachPair(self, &map_printrepr_foreach_cb, &data);
	if unlikely(temp < 0)
		goto err_temp;
	temp = data.mprf_isfirst ? DeeFormat_PRINT(printer, arg, "}")
	                         : DeeFormat_PRINT(printer, arg, " }");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

#define map_items generic_obj__asset

#define map_asseq generic_obj__asseq
#define map_asset generic_obj__asset
#define map_asmap DeeObject_NewRef

PRIVATE struct type_getset tpconst map_getsets[] = {
	TYPE_GETTER(STR_keys, &default__map_keys,
	            "->?#Keys\n"
	            "Returns a ?DSequence that can be enumerated to view only the keys of @this ?."),
	TYPE_GETTER(STR_values, &default__map_values,
	            "->?#Values\n"
	            "Returns a ?DSequence that can be enumerated to view only the values of @this ?."),
	TYPE_GETTER_AB("items", &map_items,
	               "->?S?T2?O?O\n"
	               "Returns a ?DSet that can be enumerated to view the key-item "
	               /**/ "pairs as 2-element sequences, the same way they could be viewed "
	               /**/ "if @this ?. itself was being iterated\n"
	               "Same as ${this as Set} or ?#asset"),
	TYPE_GETTER(STR_iterkeys, &default__map_iterkeys,
	            "->?#IterKeys\n"
	            "Returns an iterator for ?#{keys}. Same as ${this.keys.operator iter()}"),
	TYPE_GETTER(STR_itervalues, &default__map_itervalues,
	            "->?#IterValues\n"
	            "Returns an iterator for ?#{values}. Same as ${this.values.operator iter()}"),
	TYPE_GETTER("iteritems", &default__map_operator_iter,
	            "->?#Iterator\n"
	            "Returns an iterator for ?#{items}. Same as ${this.operator iter()}"),
	TYPE_GETTER_AB("byattr", &MapByAttr_Of,
	               "->?Ert:MapByAttr\n"
	               "Construct a wrapper for @this mapping that behaves like a generic class object, "
	               /**/ "such that any attribute address ${this.byattr.foo} behaves like ${this[\"foo\"]} "
	               /**/ "(during all of $get, $del and $set).\n"
	               "Note that the returned object doesn't implement the ?DSequence- or ?. "
	               /**/ "interfaces, but instead simply behaves like a completely generic object.\n"
	               "This attribute only makes sense if @this mapping behaves like ${{string: Object}}."),
	TYPE_GETTER(STR_frozen, &default__map_frozen,
	            "->?#Frozen\n"
	            "Returns a read-only (frozen) copy of @this ?."),
	TYPE_GETTER_AB("asseq", &map_asseq, "->?DSequence\nOptimized version of ${this as Sequence}"),
	TYPE_GETTER_AB("asset", &map_asset, "->?DSet\nOptimized version of ${this as Set}"),
	TYPE_GETTER_AB("asmap", &map_asmap, "->?.\nOptimized version of ${this as Mapping}"),
	TYPE_GETTER("length", &default__map_operator_sizeob,
	            "->?Dint\nAlias for ${##(this as Mapping)}"),

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
map_Frozen_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSeq_Type;
	DeeMH_map_frozen_t map_frozen = DeeType_RequireMethodHint(self, map_frozen);
	if (map_frozen == &DeeObject_NewRef) {
		result = self;
	} else if (map_frozen == &DeeRoDict_FromSequence) {
		result = &DeeRoDict_Type;
	} else if (map_frozen == &default__map_frozen__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Iterator_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_map_operator_iter_t map_operator_iter = DeeType_RequireMethodHint(self, map_operator_iter);
	if (map_operator_iter == &default__map_operator_iter__with__seq_operator_iter) {
		result = &DistinctMappingIterator_Type;
	} else if (map_operator_iter == &default__map_operator_iter__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Keys_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSet_Type;
	DeeMH_map_keys_t map_keys = DeeType_RequireMethodHint(self, map_keys);
	if (map_keys == &default__map_keys__with__map_iterkeys) {
		result = &DefaultSequence_MapKeys_Type;
	} else if (map_keys == &default__map_keys__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Values_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeSeq_Type;
	DeeMH_map_values_t map_values = DeeType_RequireMethodHint(self, map_values);
	if (map_values == &default__map_values__with__map_itervalues) {
		result = &DefaultSequence_MapValues_Type;
	} else if (map_values == &default__map_values__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterKeys_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_map_iterkeys_t map_iterkeys = DeeType_RequireMethodHint(self, map_iterkeys);
	if (map_iterkeys == &default__map_iterkeys__with__map_enumerate) {
		/* TODO: Custom iterator type that uses "map_enumerate" */
	} else if (map_iterkeys == &default__map_iterkeys__with__map_operator_iter) {
		result = &DefaultIterator_WithNextKey;
	} else if (map_iterkeys == &default__map_iterkeys__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterValues_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *result = &DeeIterator_Type;
	DeeMH_map_itervalues_t map_itervalues = DeeType_RequireMethodHint(self, map_itervalues);
	if (map_itervalues == &default__map_itervalues__with__map_operator_iter) {
		result = &DefaultIterator_WithNextValue;
	} else if (map_itervalues == &default__map_itervalues__none) {
		result = &DeeNone_Type;
	}
	return_reference_(result);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_fromkeys(DeeTypeObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DREF MapFromKeys *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("fromkeys", params: "
	DeeObject *keys: ?DSet;
	DeeObject *value = Dee_None;
	DeeObject *valuefor:?DCallable = NULL;
", docStringPrefix: "map");]]]*/
#define map_fromkeys_params "keys:?DSet,value=!N,valuefor?:?DCallable"
	struct {
		DeeObject *keys;
		DeeObject *value;
		DeeObject *valuefor;
	} args;
	args.value = Dee_None;
	args.valuefor = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__keys_value_valuefor, "o|oo:fromkeys", &args))
		goto err;
/*[[[end]]]*/
	result = args.valuefor ? MapFromKeysAndCallback_New(args.keys, args.valuefor)
	                       : MapFromKeysAndValue_New(args.keys, args.value);
	if unlikely(!result)
		goto err;

	/* Special case: if the accessed mapping type isn't "Mapping" (iow:
	 * the caller is calling <SubClassOfMapping>.fromkeys()), then cast
	 * the produced wrapper into an instance of `SubClassOfMapping'. */
	if (self != &DeeMapping_Type) {
		DREF DeeObject *instance;
		instance = DeeObject_New(self, 1, (DeeObject *const *)&result);
		Dee_Decref_likely(result);
		return instance;
	}
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromAttr *DCALL
map_fromattr(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("fromattr", params: "
	DeeObject *ob;
", docStringPrefix: "map");]]]*/
#define map_fromattr_params "ob"
	struct {
		DeeObject *ob;
	} args;
	DeeArg_Unpack1(err, argc, argv, "fromattr", &args.ob);
/*[[[end]]]*/
	(void)self;
	return MapFromAttr_New(args.ob);
err:
	return NULL;
}


PRIVATE struct type_method tpconst map_class_methods[] = {
	TYPE_KWMETHOD_F("fromkeys", &map_fromkeys, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                "(" map_fromkeys_params ")->?.\n"
	                "Construct a new ?. from @keys, and @value (or ${valuefor(key)}) as value.\n"
	                "Behavior is similar to the following (although a proper proxy is used, meaning \n"
	                /**/ "that operations like $getitem on the returned mapping don't require "
	                /**/ "enumeration of all keys):\n"
	                "${"
	                /**/ "local result = () -> {\n"
	                /**/ "	for (local k: keys) {\n"
	                /**/ "		local v = valuefor is bound ? valuefor(k) : value;\n"
	                /**/ "		yield (k, v);\n"
	                /**/ "	}\n"
	                /**/ "}() as Mapping;\n"
	                /**/ "if (this !== Mapping)\n"
	                /**/ "	result = this(result); /* When called via a sub-class */\n"
	                /**/ "return result;"
	                "}"),
	TYPE_METHOD_F("fromattr", &map_fromattr, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "(" map_fromattr_params ")->?.\n"
	              "Returns the attributes of a given @ob as a mapping (doing the inverse of ?#byattr)"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_get___map_getitem_always_bound__(DeeTypeObject *__restrict self) {
	bool has = DeeType_HasTrait(self, DeeType_TRAIT___map_getitem_always_bound__);
	return_bool(has);
}

PRIVATE struct type_getset tpconst map_class_getsets[] = {
	TYPE_GETTER(STR_Iterator, &map_Iterator_get,
	            "->?DType\n"
	            "Returns the iterator class used by instances of @this ?. type\n"
	            "This member must be overwritten by sub-classes of ?."),
	TYPE_GETTER(STR_Frozen, &map_Frozen_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#frozen property"),
	TYPE_GETTER(STR_Keys, &map_Keys_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#keys property"),
	TYPE_GETTER(STR_Values, &map_Values_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#values property"),
	TYPE_GETTER(STR_IterKeys, &map_IterKeys_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#iterkeys property"),
	TYPE_GETTER(STR_IterValues, &map_IterValues_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the ?#itervalues property"),
	TYPE_GETTER("__map_getitem_always_bound__", &map_get___map_getitem_always_bound__,
	            "->?Dbool\n"
	            "Evaluates to ?t if ?#{op:getitem} never throws :UnboundItem\n"
	            "\n"
	            "Sub-classes that implement ${Mapping.operator []} (or unrelated classes "
	            /**/ "that define ${__map_getitem__}), such that it never throws "
	            /**/ ":UnboundItem errors should override this property like:\n"
	            /**/ "${public static final __map_getitem_always_bound__ = true;}\n"
	            /**/ "Doing so allows the deemon runtime to implement generated "
	            /**/ "mapping functions more efficiently in some cases. In order "
	            /**/ "for deemon to see and understand the attribute, it #Bmust be "
	            /**/ "written exactly as seen in the example. It may not be a static "
	            /**/ "property, or evaluate to something other than ?t. Otherwise, "
	            /**/ "the hint is ignored and may as well not be present at all."),
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


PRIVATE struct type_math map_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ DEFIMPL(&default__set_operator_inv),
	/* .tp_pos         = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg         = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add         = */ &default__map_operator_add,
	/* .tp_sub         = */ &default__map_operator_sub,
	/* .tp_mul         = */ DEFIMPL_UNSUPPORTED(&default__mul__unsupported),
	/* .tp_div         = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod         = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl         = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr         = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and         = */ &default__map_operator_and,
	/* .tp_or          = */ &default__map_operator_add,
	/* .tp_xor         = */ &default__map_operator_xor,
	/* .tp_pow         = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL_UNSUPPORTED(&default__inc__unsupported),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ &default__map_operator_inplace_add,
	/* .tp_inplace_sub = */ &default__map_operator_inplace_sub,
	/* .tp_inplace_mul = */ DEFIMPL_UNSUPPORTED(&default__inplace_mul__unsupported),
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ &default__map_operator_inplace_and,
	/* .tp_inplace_or  = */ &default__map_operator_inplace_add,
	/* .tp_inplace_xor = */ &default__map_operator_inplace_xor,
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

PRIVATE struct type_cmp map_cmp = {
	/* .tp_hash          = */ &default__map_operator_hash,
	/* .tp_compare_eq    = */ &default__map_operator_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
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
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL_UNSUPPORTED(&default__foreach__unsupported),
	/* .tp_foreach_pair               = */ &default__map_operator_foreach_pair,
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
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
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
"When invoked manually, a general-purpose, empty ?. equivalent "
/**/ "to ?Ert:Mapping_empty is returned\n"
"\n"

"repr->\n"
"Returns the representation of all sequence elements, "
/**/ "using abstract mapping syntax\n"
"e.g.: ${{ \"foo\": 10, \"bar\": \"baz\" }}\n"
"\n"

"[:]->!D\n" /* TODO: Doc could auto-detect this since the impls are *__unsupported, but are implemented by Mapping.__mro__.first */
"[:]=->!D\n"
"del[:]->!D\n"
"\n"

"#->\n"
"The number of keys within this mapping (including unbound keys). Same as ${#this.keys}\n"
"\n"

"[]->\n"
"#tUnboundItem{Specified @key exists (i.e. ${key in this.keys}), but isn't bound. "
/*         */ "Can only happen when ?#__map_getitem_always_bound__ evaluates to ?f}"
"#tUnknownKey{Specified @key does not exists (i.e. ${key !in this.keys})}"
"Lookup and return the value associated with a given @key\n"
"\n"

"iter->\n"
"Returns an iterator for enumerating key-value pairs as 2-elements sequences";

/* TODO: Document all the other operators */
#endif /* !CONFIG_NO_DOC */




/* `Mapping from deemon' */
PUBLIC DeeTypeObject DeeMapping_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Mapping),
	/* .tp_doc      = */ map_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_MAP << Dee_TF_SEQCLASS_SHFT),
	/* .tp_base     = */ &DeeSeq_Type, /* XXX: Shouldn't this be "DeeSet_Type"? */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject, /* Allow default-construction of sequence objects. */
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ &DeeNone_OperatorCopy,
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
		/* .tp_bool      = */ &default__seq_operator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&map_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &map_math,
	/* .tp_cmp           = */ &map_cmp,
	/* .tp_seq           = */ &map_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ map_methods,
	/* .tp_getsets       = */ map_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ map_class_methods,
	/* .tp_class_getsets = */ map_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ map_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(map_operators),
	/* .tp_mhcache       = */ &mh_cache_empty,
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


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_MAP_C */
