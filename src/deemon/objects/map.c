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

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_get(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	result = DeeObject_TryGetItem(self, key);
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_setdefault(DeeObject *self, size_t argc, DeeObject *const *argv) {
	struct type_nsi const *nsi;
	DREF DeeObject *result;
	DeeObject *key, *value = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:setdefault", &key, &value))
		goto err;
	nsi = DeeType_NSI(Dee_TYPE(self));
	if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_setdefault)
		return (*nsi->nsi_maplike.nsi_setdefault)(self, key, value);

	/* Fallback: lookup key and override if not already present (thread-unsafe) */
	result = DeeObject_TryGetItem(self, key);
	if (result == ITER_DONE) {
		if unlikely(DeeObject_SetItem(self, key, value))
			goto err;
		result = value;
		Dee_Incref(value);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_pop(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:pop", &key, &def))
		goto err;
	result = DeeObject_TryGetItem(self, key);
	if (result == ITER_DONE) {
		/* Not present -> use default */
		result = def;
		if unlikely(!result) {
			err_unknown_key((DeeObject *)self, key);
		} else {
			Dee_Incref(result);
		}
	} else if (result != NULL) {
		/* Remove `key' from `self' */
		if unlikely(DeeObject_DelItem(self, key))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_popitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int temp;
	DREF DeeObject *result, *iter, *key;
	if (DeeArg_Unpack(argc, argv, ":popitem"))
		goto err;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterNext(iter);
	Dee_Decref_likely(iter);
	if unlikely(!result)
		goto err;
	if (result == ITER_DONE)
		goto err_empty;
	key = DeeObject_GetItemIndex(result, 0);
	if unlikely(!key)
		goto err_r;
	temp = DeeObject_DelItem(self, key);
	Dee_Decref(key);
	if unlikely(temp)
		goto err_r;
	return result;
err_empty:
	err_empty_sequence(self);
	goto err;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_clear(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	for (;;) {
		int temp;
		DREF DeeObject *item, *iter, *key;
		iter = DeeObject_Iter(self);
		if unlikely(!iter)
			goto err;
		item = DeeObject_IterNext(iter);
		Dee_Decref_likely(iter);
		if unlikely(!item)
			goto err;
		if (item == ITER_DONE)
			break;
		key = DeeObject_GetItemIndex(item, 0);
		Dee_Decref(item);
		if unlikely(!key)
			goto err;
		temp = DeeObject_DelItem(self, key);
		Dee_Decref(key);
		if unlikely(temp)
			goto err;
	}
	return_none;
err:
	return NULL;
}


#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define map_update_callback (*(Dee_foreach_pair_t)&DeeObject_SetItem)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
map_update_callback(void *arg, DeeObject *key, DeeObject *value) {
	return DeeObject_SetItem((DeeObject *)arg, key, value);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_update(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:update", &seq))
		goto err;
	if (self != seq) {
		if unlikely(DeeObject_ForeachPair(seq, &map_update_callback, self))
			goto err;
	}
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_setold(DeeObject *self, size_t argc, DeeObject *const *argv) {
	struct type_nsi const *nsi;
	DREF DeeObject *old_value;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setold", &key, &value))
		goto err;
	nsi = DeeType_NSI(Dee_TYPE(self));
	if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_updateold) {
		int error;
		error = (*nsi->nsi_maplike.nsi_updateold)(self, key, value, NULL);
		if unlikely(error < 0)
			goto err;
		return_bool_(error > 0);
	}

	/* Fallback: must use thread-unsafe (multi-step) operations. */
	old_value = DeeObject_TryGetItem(self, key);
	if (ITER_ISOK(old_value)) {
		Dee_Decref(old_value);
		if unlikely(DeeObject_SetItem(self, key, value))
			goto err;
		return_true;
	}
	if unlikely(!old_value)
		goto err;
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_setnew(DeeObject *self, size_t argc, DeeObject *const *argv) {
	struct type_nsi const *nsi;
	DREF DeeObject *old_value;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setnew", &key, &value))
		goto err;
	nsi = DeeType_NSI(Dee_TYPE(self));
	if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_insertnew) {
		int error;
		error = (*nsi->nsi_maplike.nsi_insertnew)(self, key, value, NULL);
		if unlikely(error < 0)
			goto err;
		return_bool_(error == 0);
	}

	/* Fallback: must use thread-unsafe (multi-step) operations. */
	old_value = DeeObject_TryGetItem(self, key);
	if (ITER_ISOK(old_value)) {
		Dee_Decref(old_value);
		return_false;
	}
	if unlikely(!old_value)
		goto err;
	if unlikely(DeeObject_SetItem(self, key, value))
		goto err;
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_setold_ex(DeeObject *self, size_t argc, DeeObject *const *argv) {
	struct type_nsi const *nsi;
	DREF DeeObject *old_value, *result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setold_ex", &key, &value))
		goto err;
	nsi = DeeType_NSI(Dee_TYPE(self));
	if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_updateold) {
		int error;
		error = (*nsi->nsi_maplike.nsi_updateold)(self, key, value, &old_value);
		if unlikely(error < 0)
			goto err;
		if (error == 1) {
			result = DeeTuple_Pack(2, Dee_True, old_value);
			Dee_Decref_unlikely(old_value);
		} else {
			result = DeeTuple_Pack(2, Dee_False, Dee_None);
		}
		return result;
	}

	/* Fallback: must use thread-unsafe (multi-step) operations. */
	old_value = DeeObject_TryGetItem(self, key);
	if (ITER_ISOK(old_value)) {
		if unlikely(DeeObject_SetItem(self, key, value)) {
			Dee_Decref(old_value);
			goto err;
		}
		result = DeeTuple_Pack(2, Dee_True, old_value);
		Dee_Decref_unlikely(old_value);
		return result;
	}
	if unlikely(!old_value)
		goto err;
	return DeeTuple_Pack(2, Dee_False, Dee_None);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_setnew_ex(DeeObject *self, size_t argc, DeeObject *const *argv) {
	struct type_nsi const *nsi;
	DREF DeeObject *result, *old_value;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:setnew_ex", &key, &value))
		goto err;
	nsi = DeeType_NSI(Dee_TYPE(self));
	if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_MAP && nsi->nsi_maplike.nsi_insertnew) {
		int error;
		error = (*nsi->nsi_maplike.nsi_insertnew)(self, key, value, &old_value);
		if unlikely(error < 0)
			goto err;
		if (error == 0) {
			result = DeeTuple_Pack(2, Dee_True, Dee_None);
		} else {
			result = DeeTuple_Pack(2, Dee_False, old_value);
			Dee_Decref_unlikely(old_value);
		}
		return result;
	}

	/* Fallback: must use thread-unsafe (multi-step) operations. */
	old_value = DeeObject_TryGetItem(self, key);
	if (ITER_ISOK(old_value)) {
		result = DeeTuple_Pack(2, Dee_False, old_value);
		Dee_Decref_unlikely(old_value);
		return result;
	}
	if unlikely(!old_value)
		goto err;
	if unlikely(DeeObject_SetItem(self, key, value))
		goto err;
	return DeeTuple_Pack(2, Dee_True, Dee_None);
err:
	return NULL;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_insert_all(DeeObject *self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrString(self, "update", argc, argv);
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */




DOC_DEF(map_get_doc,
        "(key,def=!N)->\n"
        "#r{The value associated with @key or @def when @key has no value associated}");
DOC_DEF(map_byhash_doc,
        "(template:?O)->?S?T2?O?O\n"
        "#ptemplate{The object who's hash should be used to search for collisions}"
        "Same as ?Abyhash?DSequence, but rather than comparing the hashes of the "
        /**/ "key-value pairs, search for pairs where the key matches the hash of @template");
DOC_DEF(map_setdefault_doc,
        "(key,def)->\n"
        "#r{The object currently assigned to @key}"
        "Lookup @key in @this ?. and return its value if found. "
        /**/ "Otherwise, assign @def to @key and return it instead");
DOC_DEF(map_pop_doc,
        "(key)->\n"
        "(key,def)->\n"
        "#tKeyError{No @def was given and @key was not found}"
        "Delete @key from @this ?. and return its previously assigned "
        /**/ "value or @def when @key had no item associated");
DOC_DEF(map_popitem_doc,
        "->?T2?O?O\n"
        "#r{A random pair key-value pair that has been removed}"
        "#tValueError{@this ?. was empty}");
DOC_DEF(map_update_doc,
        "(items:?S?T2?O?O)\n"
        "Iterate @items and unpack each element into 2 others, "
        /**/ "using them as key and value to insert into @this ?.");
DOC_DEF(map_setold_doc,
        "(key,value)->?Dbool\n"
        "#r{Indicative of @value having been assigned to @key}"
        "Assign @value to @key, only succeeding when @key already existed to begin with");
DOC_DEF(map_setnew_doc,
        "(key,value)->?Dbool\n"
        "#r{Indicative of @value having been assigned to @key}"
        "Assign @value to @key, only succeeding when @key didn't exist before");
DOC_DEF(map_setold_ex_doc,
        "(key,value)->?T2?Dbool?O\n"
        "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
        "Same as ?#setold but also return the previously assigned value");
DOC_DEF(map_setnew_ex_doc,
        "(key,value)->?T2?Dbool?O\n"
        "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
        "Same as ?#setnew but return the previously assigned value on failure");

INTDEF struct type_method tpconst map_methods[];
INTERN_TPCONST struct type_method tpconst map_methods[] = {
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	/* Default operations for all mappings. */
	TYPE_METHOD(STR_get, &default_map_get, DOC_GET(map_get_doc)),
	TYPE_KWMETHOD("byhash", &map_byhash, DOC_GET(map_byhash_doc)),

	/* Default operations for modifiable mappings. */
	TYPE_METHOD(STR_setold, &default_map_setold, DOC_GET(map_setold_doc)),
	TYPE_METHOD(STR_setold_ex, &default_map_setold_ex, DOC_GET(map_setold_ex_doc)),
	TYPE_METHOD(STR_setnew, &default_map_setnew, DOC_GET(map_setnew_doc)),
	TYPE_METHOD(STR_setnew_ex, &default_map_setnew_ex, DOC_GET(map_setnew_ex_doc)),
	TYPE_METHOD(STR_setdefault, &default_map_setdefault, DOC_GET(map_setdefault_doc)),
	TYPE_METHOD(STR_update, &default_map_update, DOC_GET(map_update_doc)),
	TYPE_METHOD(STR_remove, &default_map_remove, "(key)->?Dbool"),
	TYPE_METHOD(STR_removekeys, &default_map_removekeys, "(keys:?S?O)"),
	TYPE_METHOD(STR_pop, &default_map_pop, DOC_GET(map_pop_doc)),
	TYPE_METHOD(STR_popitem, &default_map_popitem, DOC_GET(map_popitem_doc)),

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

	/* Old function names. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("insert_all", &default_map_update,
	            "(items:?S?T2?O?O)\n"
	            "A deprecated alias for ?#update"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* Default operations for all mappings. */
	TYPE_METHOD(STR_get, &map_get, DOC_GET(map_get_doc)),
	TYPE_KWMETHOD("byhash", &map_byhash, DOC_GET(map_byhash_doc)),

	/* Default operations for modifiable mappings. */
	TYPE_METHOD("setdefault", &map_setdefault, DOC_GET(map_setdefault_doc)),
	TYPE_METHOD("pop", &map_pop, DOC_GET(map_pop_doc)),
	TYPE_METHOD("popitem", &map_popitem, DOC_GET(map_popitem_doc)),
	TYPE_METHOD("clear", &map_clear,
	            "()\n"
	            "Clear all values from @this ?."),
	TYPE_METHOD("update", &map_update, DOC_GET(map_update_doc)),
	TYPE_METHOD("setold", &map_setold, DOC_GET(map_setold_doc)),
	TYPE_METHOD("setnew", &map_setnew, DOC_GET(map_setnew_doc)),
	TYPE_METHOD("setold_ex", &map_setold_ex, DOC_GET(map_setold_ex_doc)),
	TYPE_METHOD("setnew_ex", &map_setnew_ex, DOC_GET(map_setnew_ex_doc)),

	/* Old function names. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("insert_all", &map_insert_all,
	            "(items:?S?T2?O?O)\n"
	            "A deprecated alias for ?#update"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	TYPE_METHOD_END
};



#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
typedef struct {
	OBJECT_HEAD
	DREF DeeObject        *mpi_iter; /* [1..1][const] The iterator for enumerating `mpi_map'. */
	DREF DeeObject        *mpi_map;  /* [1..1][const] The mapping object for which this is a proxy. */
	struct type_nsi const *mpi_nsi;  /* [0..1][const][->nsi_class == TYPE_SEQX_CLASS_MAP] If available, the NSI interface of `mpi_map'. */
} MapProxyIterator;

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_ctor(MapProxyIterator *__restrict self) {
	self->mpi_iter = DeeObject_Iter(Dee_EmptyMapping);
	if unlikely(!self->mpi_iter)
		goto err;
	self->mpi_map = Dee_EmptyMapping;
	Dee_Incref(self->mpi_map);
	self->mpi_nsi = NULL;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_init(MapProxyIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_mappingproxy.Iterator", &self->mpi_map))
		goto err;
	self->mpi_iter = DeeObject_Iter(self->mpi_map);
	if unlikely(!self->mpi_iter)
		goto err;
	Dee_Incref(self->mpi_map);
	self->mpi_nsi = DeeType_NSI(Dee_TYPE(self->mpi_map));
	if (self->mpi_nsi && self->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
		self->mpi_nsi = NULL;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_iterator_copy(MapProxyIterator *__restrict self,
                    MapProxyIterator *__restrict other) {
	self->mpi_iter = DeeObject_Copy(other->mpi_iter);
	if unlikely(!self->mpi_iter)
		goto err;
	self->mpi_map = other->mpi_map;
	Dee_Incref(self->mpi_map);
	self->mpi_nsi = other->mpi_nsi;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_iterator_deepcopy(MapProxyIterator *__restrict self,
                        MapProxyIterator *__restrict other) {
	self->mpi_iter = DeeObject_DeepCopy(other->mpi_iter);
	if unlikely(!self->mpi_iter)
		goto err;
	self->mpi_map = DeeObject_DeepCopy(other->mpi_map);
	if unlikely(!self->mpi_iter)
		goto err_iter;
	self->mpi_nsi = other->mpi_nsi;
	return 0;
err_iter:
	Dee_Decref(self->mpi_iter);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_iterator_fini(MapProxyIterator *__restrict self) {
	Dee_Decref(self->mpi_iter);
	Dee_Decref(self->mpi_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_iterator_visit(MapProxyIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mpi_iter);
	Dee_Visit(self->mpi_map);
}

PRIVATE NONNULL((1)) int DCALL
proxy_iterator_bool(MapProxyIterator *__restrict self) {
	return DeeObject_Bool(self->mpi_iter);
}

PRIVATE struct type_member tpconst proxy_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapProxyIterator, mpi_map), "->?DMapping"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapProxyIterator, mpi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_iterator_next_key(MapProxyIterator *__restrict self) {
	DREF DeeObject *pair;
	DREF DeeObject *key_and_value[2];
	int error;
	/* Optimize using NSI */
	if (self->mpi_nsi && self->mpi_nsi->nsi_maplike.nsi_nextkey)
		return (*self->mpi_nsi->nsi_maplike.nsi_nextkey)(self->mpi_iter);
	pair = DeeObject_IterNext(self->mpi_iter);
	if (pair == ITER_DONE)
		return ITER_DONE;
	if unlikely(!pair)
		goto err;
	error = DeeObject_Unpack(pair, 2, key_and_value);
	Dee_Decref(pair);
	if unlikely(error)
		goto err;
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_iterator_next_value(MapProxyIterator *__restrict self) {
	DREF DeeObject *pair;
	DREF DeeObject *key_and_value[2];
	int error;
	/* Optimize using NSI */
	if (self->mpi_nsi && self->mpi_nsi->nsi_maplike.nsi_nextvalue)
		return (*self->mpi_nsi->nsi_maplike.nsi_nextvalue)(self->mpi_iter);
	pair = DeeObject_IterNext(self->mpi_iter);
	if (pair == ITER_DONE)
		return ITER_DONE;
	if unlikely(!pair)
		goto err;
	error = DeeObject_Unpack(pair, 2, key_and_value);
	Dee_Decref(pair);
	if unlikely(error)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_iterator_next_item(MapProxyIterator *__restrict self) {
	return DeeObject_IterNext(self->mpi_iter);
}


PRIVATE DeeTypeObject DeeMappingProxyIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingProxyIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_iterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_iterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject DeeMappingKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingKeysIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_key,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject DeeMappingValuesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingValuesIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_value,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE DeeTypeObject DeeMappingItemsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingItemsIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(MapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterator_next_item,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};






typedef struct {
	OBJECT_HEAD
	DREF DeeObject *mp_map; /* [1..1][const] The mapping object for which this is a proxy. */
} MapProxy;

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(MapProxy *__restrict self) {
	self->mp_map = Dee_EmptyMapping;
	Dee_Incref(self->mp_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_init(MapProxy *__restrict self, size_t argc,
           DeeObject *const *argv) {
	self->mp_map = Dee_EmptyMapping;
	if (DeeArg_Unpack(argc, argv, "|o:_MappingProxy", &self->mp_map))
		goto err;
	Dee_Incref(self->mp_map);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_copy(MapProxy *__restrict self,
           MapProxy *__restrict other) {
	self->mp_map = other->mp_map;
	Dee_Incref(self->mp_map);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_deepcopy(MapProxy *__restrict self,
               MapProxy *__restrict other) {
	self->mp_map = DeeObject_DeepCopy(other->mp_map);
	if unlikely(!self->mp_map)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_fini(MapProxy *__restrict self) {
	Dee_Decref(self->mp_map);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_visit(MapProxy *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mp_map);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_sizeob(MapProxy *__restrict self) {
	return DeeObject_SizeOb(self->mp_map);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
proxy_size(MapProxy *__restrict self) {
	return DeeObject_Size(self->mp_map);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
proxy_size_fast(MapProxy *__restrict self) {
	return DeeObject_SizeFast(self->mp_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
proxy_contains_key(MapProxy *self, DeeObject *key) {
	return DeeObject_Contains(self->mp_map, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF MapProxyIterator *DCALL
proxy_iterself(MapProxy *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF MapProxyIterator *result;
	result = DeeObject_MALLOC(MapProxyIterator);
	if unlikely(!result)
		goto done;
	result->mpi_iter = DeeObject_Iter(self->mp_map);
	if unlikely(!result->mpi_iter)
		goto err_r;
	result->mpi_nsi = DeeType_NSI(Dee_TYPE(self->mp_map));
	if (result->mpi_nsi && result->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
		result->mpi_nsi = NULL;
	result->mpi_map = self->mp_map;
	Dee_Incref(result->mpi_map);
	DeeObject_Init(result, result_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
proxy_iterself_keys(MapProxy *__restrict self) {
	return proxy_iterself(self, &DeeMappingKeysIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
proxy_iterself_values(MapProxy *__restrict self) {
	return proxy_iterself(self, &DeeMappingValuesIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
proxy_iterself_items(MapProxy *__restrict self) {
	return proxy_iterself(self, &DeeMappingItemsIterator_Type);
}


PRIVATE struct type_seq proxykeys_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_keys,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&proxy_contains_key,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ NULL, /* TODO: implement using DeeObject_Enumerate() */
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_seq proxyvalues_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_values,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_seq proxyitems_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_items,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_member tpconst proxykeys_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingKeysIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxyvalues_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingValuesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxyitems_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingItemsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeMappingProxyIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxyitems_byhash(MapProxy *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__template, "o:byhash", &template_))
		goto err;
	/* Invoke byhash() on the underlying mapping. */
	return DeeObject_CallAttrStringf(self->mp_map, "byhash", PCKuSIZ,
	                                 DeeObject_Hash(template_));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxykeys_byhash(MapProxy *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	result = proxyitems_byhash(self, argc, argv, kw);
	if likely(result) {
		if (DeeTuple_Check(result)) {
			if (DeeTuple_SIZE(result) == 0)
				goto done;
			if (DeeTuple_SIZE(result) == 1) {
				DREF DeeObject *key_and_value[2];
				/* Unpack the pointed-to pair */
				if (DeeObject_Unpack(DeeTuple_GET(result, 0), 2, key_and_value))
					goto err_r;
				Dee_Decref(result);
				Dee_Decref(key_and_value[1]);
				/* Return the pair's first element (which is the key) */
				result = (DREF DeeObject *)DeeTuple_NewUninitialized(1);
				if unlikely(!result) {
					Dee_Decref(key_and_value[0]);
					goto err;
				}
				DeeTuple_SET(result, 0, key_and_value[0]);
				goto done;
			}
		}
#ifndef __OPTIMIZE_SIZE__
		else if (Dee_TYPE(result) == &MapHashFilter_Type &&
		         ((HashFilter *)result)->f_seq == self->mp_map) {
			/* The mapping doesn't implement a proper byhash() function.
			 * In this case, we might as well just return a regular, old
			 * sequence hash filter object for ourself (the mapping's key
			 * view), rather than going through the each-wrapper below. */
			HashFilter *filter;
			filter = (HashFilter *)result;
			if unlikely(DeeObject_IsShared(filter)) {
				/* The filter is being shared, so we must create a new filter. */
				Dee_hash_t hash;
				hash = filter->f_hash;
				Dee_Decref_unlikely(filter);
				result = DeeSeq_HashFilter((DeeObject *)self, hash);
			} else {
				/* The filter isn't being shared. -> Can modify in-place */
				Dee_DecrefNokill(&MapHashFilter_Type);
				Dee_DecrefNokill(filter->f_seq);
				Dee_Incref(&SeqHashFilter_Type);
				Dee_Incref(self);
				filter->f_seq = (DREF DeeObject *)self;
				filter->ob_type = &SeqHashFilter_Type;
			}
			goto done;
		}
#endif /* !__OPTIMIZE_SIZE__ */

		/* Fallback: byhash() didn't return a tuple, or that tuple's length is >= 2
		 * In this case, we return the equivalent of `return byhash(template).each[0] as Sequence' */
		{
			DREF SeqEachOperator *each;
			each = SeqEachOperator_MALLOC(1);
			if unlikely(!each)
				goto err_r;
			each->so_opname    = OPERATOR_GETITEM;
			each->so_opargc    = 1;
			each->se_seq       = result; /* inherit reference */
			each->so_opargv[0] = DeeInt_Zero;
			Dee_Incref(DeeInt_Zero);
			DeeObject_Init(each, &SeqEachOperator_Type);
			/* Wrap the each-operator in a super-view for sequences, thus preventing
			 * the caller from accidentally extending the each-expression any further. */
			result = DeeSuper_New(&DeeSeq_Type, (DeeObject *)each);
			Dee_Decref_unlikely(each);
		}
	}
done:
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


DOC_REF(seq_byhash_doc);

INTDEF struct type_method tpconst proxykeys_methods[];
INTERN_TPCONST struct type_method tpconst proxykeys_methods[] = {
	TYPE_KWMETHOD("byhash", &proxykeys_byhash, DOC_GET(seq_byhash_doc)),
	TYPE_METHOD_END
};

INTDEF struct type_method tpconst proxyitems_methods[];
INTERN_TPCONST struct type_method tpconst proxyitems_methods[] = {
	TYPE_KWMETHOD("byhash", &proxyitems_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};



/* Base class for proxy views for mapping sequences. */
PRIVATE DeeTypeObject DeeMappingProxy_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingProxy",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxyitems_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_class_members,
};

PRIVATE DeeTypeObject *tpconst mapping_keys_mro[] = {
	&DeeSet_Type,
	&DeeMappingProxy_Type,
	&DeeSeq_Type,
	&DeeObject_Type,
	NULL
};

PRIVATE DeeTypeObject DeeMappingKeys_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingKeys",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxykeys_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxykeys_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxykeys_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ mapping_keys_mro
};

PRIVATE DeeTypeObject DeeMappingValues_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingValues",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxyvalues_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxyvalues_class_members
};

PRIVATE DeeTypeObject DeeMappingItems_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MappingItems",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMappingProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(MapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxyitems_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxyitems_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxyitems_class_members,
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_iterself(DeeObject *__restrict self) {
	if unlikely(Dee_TYPE(self) == &DeeMapping_Type) {
		/* Special case: Create an empty iterator.
		 * >> This can happen when someone tries to iterate a symbolic empty-mapping object. */
		return_empty_iterator;
	}
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_contains(DeeObject *self, DeeObject *key) {
	DREF DeeObject *value;
	value = DeeObject_TryGetItem(self, key);
	if (!ITER_ISOK(value)) {
		if (value == ITER_DONE)
			return_false;
		return NULL;
	}
	Dee_Decref(value);
	return_true;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
map_nsi_getsize(DeeObject *__restrict self) {
	size_t result = 0;
	DREF DeeObject *iter, *item;
	DeeTypeObject *tp_self = Dee_TYPE(self);

	/* Check if a sub-class is overriding `operator iter'. If
	 * not, then the mapping is empty for all we're concerned */
	if (tp_self->tp_seq->tp_iter == &map_iterself)
		goto done;

	/* Very inefficient: iterate the mapping and count items. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		Dee_Decref(item);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
		++result;
	}
	if unlikely(!item)
		goto err_iter;
	Dee_Decref(iter);
done:
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_size(DeeObject *__restrict self) {
	size_t result = map_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_getitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *iter, *item;
	DREF DeeObject *item_key_and_value[2];
	dhash_t key_hash;
	DeeTypeObject *tp_self = Dee_TYPE(self);

	/* Check if a sub-class is overriding `operator iter'. If
	 * not, then the mapping is empty for all we're concerned */
	if (tp_self->tp_seq->tp_iter == &map_iterself) {
		err_unknown_key(self, key);
		goto err;
	}

	/* Very inefficient: iterate the mapping to search for a matching key-item pair. */
	key_hash = DeeObject_Hash(key);
	iter     = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		int unpack_error, temp;
		unpack_error = DeeObject_Unpack(item, 2, item_key_and_value);
		Dee_Decref(item);
		if unlikely(unpack_error)
			goto err_iter;

		/* Check if this is the key we're looking for. */
		if (DeeObject_Hash(item_key_and_value[0]) != key_hash) {
			Dee_Decref(item_key_and_value[0]);
		} else {
			temp = DeeObject_TryCompareEq(key, item_key_and_value[0]);
			Dee_Decref(item_key_and_value[0]);
			if unlikely(temp == Dee_COMPARE_ERR) {
				Dee_Decref(item_key_and_value[1]);
				goto err_iter;
			}
			if (temp == 0) {
				/* Found it! */
				Dee_Decref(iter);
				return item_key_and_value[1];
			}
		}
		Dee_Decref(item_key_and_value[1]);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if (item == ITER_DONE)
		err_unknown_key(self, key);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
map_getitem_def(DeeObject *self, DeeObject *key, DeeObject *defl) {
	DREF DeeObject *iter, *item;
	DREF DeeObject *item_key_and_value[2];
	dhash_t key_hash;
	DeeTypeObject *tp_self = Dee_TYPE(self);

	/* Check if a sub-class is overriding `operator iter'. If
	 * not, then the mapping is empty for all we're concerned */
	if (tp_self->tp_seq->tp_iter == &map_iterself)
		goto return_defl;

	/* Very inefficient: iterate the mapping to search for a matching key-item pair. */
	key_hash = DeeObject_Hash(key);
	iter     = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		int unpack_error, temp;
		unpack_error = DeeObject_Unpack(item, 2, item_key_and_value);
		Dee_Decref(item);
		if unlikely(unpack_error)
			goto err_iter;

		/* Check if this is the key we're looking for. */
		if (DeeObject_Hash(item_key_and_value[0]) != key_hash) {
			Dee_Decref(item_key_and_value[0]);
		} else {
			temp = DeeObject_TryCompareEq(key, item_key_and_value[0]);
			Dee_Decref(item_key_and_value[0]);
			if unlikely(temp == Dee_COMPARE_ERR) {
				Dee_Decref(item_key_and_value[1]);
				goto err_iter;
			}
			if (temp == 0) {
				/* Found it! */
				Dee_Decref(iter);
				return item_key_and_value[1];
			}
		}
		Dee_Decref(item_key_and_value[1]);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if (!item)
		goto err_iter;
	Dee_Decref(iter);
return_defl:
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
map_getrange(DeeObject *self,
             DeeObject *UNUSED(begin),
             DeeObject *UNUSED(end)) {
	/* Override the getrange operator of `sequence' as not-implemented. */
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}

PRIVATE struct type_nsi tpconst map_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&map_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)NULL,
			/* .nsi_nextvalue  = */ (dfunptr_t)NULL,
			/* .nsi_getdefault = */ (dfunptr_t)&map_getitem_def
		}
	}
};

PRIVATE struct type_seq DeeMap_OperatorSeq = {
	/* .tp_iter     = */ &map_iterself,
	/* .tp_sizeob   = */ &map_size,
	/* .tp_contains = */ &map_contains,
	/* .tp_getitem  = */ &map_getitem,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ &map_getrange,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_nsi      = */ &map_nsi
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
map_eq_impl(DeeObject *__restrict self,
            DeeObject *__restrict other) {
	size_t pair_count = 0, other_size;
	int temp;
	DREF DeeObject *iter, *elem, *pair[2];
	DREF DeeObject *other_value;
	/* Check of all keys from `self' have the same value within `other' */
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		if (DeeObject_Unpack(elem, 2, pair))
			goto err_elem;
		Dee_Decref(elem);
		other_value = DeeObject_TryGetItem(other, pair[0]);
		Dee_Decref(pair[0]);
		if (!ITER_ISOK(other_value)) {
			Dee_Decref(pair[1]);
			Dee_Decref(iter);
			if unlikely(!other_value)
				goto err;
			return 0; /* Key wasn't provided by `other' */
		}
		temp = DeeObject_TryCompareEq(pair[1], other_value);
		Dee_Decref(other_value);
		Dee_Decref(pair[1]);
		if (temp != 0) {
			if unlikely(temp == Dee_COMPARE_ERR)
				goto err_iter;
			Dee_Decref(iter);
			return 0;
		}
		/* Track the number of pairs found in `self' */
		++pair_count;
	}
	if unlikely(!elem)
		goto err_iter;
	Dee_Decref(iter);
	/* Make sure that `other' has the same size as `self' */
	other_size = DeeObject_Size(other);
	if unlikely(other_size == (size_t)-1)
		goto err;
	return pair_count == other_size;
err_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
DeeMap_OperatorHash(DeeObject *__restrict self) {
	dhash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	DREF DeeObject *iter, *elem;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		/* Note how we don't use `Dee_HashCombine()' here!
		 * That become order doesn't matter for mappings. */
		result ^= DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err:
	DeeError_Print("Unhandled error in `Mapping.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_OperatorEq(DeeObject *self, DeeObject *other) {
	int error = map_eq_impl(self, other);
	if unlikely(error < 0)
		goto err;
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_OperatorNe(DeeObject *self, DeeObject *other) {
	int error = map_eq_impl(self, other);
	if unlikely(error < 0)
		goto err;
	return_bool_(!error);
err:
	return NULL;
}

PRIVATE struct type_cmp DeeMap_OperatorCmp = {
	/* .tp_hash          = */ &DeeMap_OperatorHash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ &DeeMap_OperatorEq,
	/* .tp_ne            = */ &DeeMap_OperatorNe,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
};
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */



PRIVATE struct type_math map_math = {
	/* .tp_int32  = */ NULL,
	/* .tp_int64  = */ NULL,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ NULL,
	/* .tp_neg    = */ NULL,
	/* .tp_add    = */ NULL, /* TODO: &DeeMap_Union */
	/* .tp_sub    = */ NULL, /* TODO: &DeeMap_Difference */
	/* .tp_mul    = */ NULL,
	/* .tp_div    = */ NULL,
	/* .tp_mod    = */ NULL,
	/* .tp_shl    = */ NULL,
	/* .tp_shr    = */ NULL,
	/* .tp_and    = */ NULL, /* TODO: &DeeMap_Intersection */
	/* .tp_or     = */ NULL, /* TODO: &DeeMap_Union */
	/* .tp_xor    = */ NULL, /* TODO: &DeeMap_SymmetricDifference */
	/* .tp_pow    = */ NULL
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

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
PRIVATE WUNUSED NONNULL((1, 2)) DREF MapProxy *DCALL
map_new_proxy(DeeObject *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF MapProxy *result;
	result = DeeObject_MALLOC(MapProxy);
	if unlikely(!result)
		goto done;
	result->mp_map = self;
	Dee_Incref(self);
	DeeObject_Init(result, result_type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF MapProxyIterator *DCALL
map_new_proxyiter(DeeObject *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF MapProxyIterator *result;
	result = DeeObject_MALLOC(MapProxyIterator);
	if unlikely(!result)
		goto done;
	result->mpi_iter = DeeObject_Iter(self);
	if unlikely(!result->mpi_iter)
		goto err_r;
	result->mpi_map = self;
	Dee_Incref(self);
	if (result_type != &DeeMappingItemsIterator_Type) {
		/* Search for an NSI descriptor defined by the mapping type. */
		result->mpi_nsi = DeeType_NSI(Dee_TYPE(self));
		if (result->mpi_nsi && result->mpi_nsi->nsi_class != TYPE_SEQX_CLASS_MAP)
			result->mpi_nsi = NULL;
	}
	DeeObject_Init(result, result_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxy *DCALL
map_keys(DeeObject *__restrict self) {
	return map_new_proxy(self, &DeeMappingKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxy *DCALL
map_values(DeeObject *__restrict self) {
	return map_new_proxy(self, &DeeMappingValues_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxy *DCALL
map_items(DeeObject *__restrict self) {
	return map_new_proxy(self, &DeeMappingItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
map_iterkeys(DeeObject *__restrict self) {
	return map_new_proxyiter(self, &DeeMappingKeysIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
map_itervalues(DeeObject *__restrict self) {
	return map_new_proxyiter(self, &DeeMappingValuesIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapProxyIterator *DCALL
map_iteritems(DeeObject *__restrict self) {
	return map_new_proxyiter(self, &DeeMappingItemsIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_GetFirst(DeeObject *__restrict self) {
	DREF DeeObject *iter, *result;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (result == ITER_DONE)
		goto err_empty;
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeMap_DelFirst(DeeObject *__restrict self) {
	DREF DeeObject *iter, *key;
	int result;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	key = DeeObject_IterNextKey(iter);
	Dee_Decref(iter);
	if (!ITER_ISOK(key)) {
		if unlikely(!key)
			goto err;
		goto err_empty;
	}
	result = DeeObject_DelItem(self, key);
	Dee_Decref(key);
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_GetLast(DeeObject *__restrict self) {
	DREF DeeObject *iter, *result, *next;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	result = DeeObject_IterNext(iter);
	if (!ITER_ISOK(result)) {
		Dee_Decref(iter);
		if (result == ITER_DONE)
			goto err_empty;
		goto err;
	}
	for (;;) {
		next = DeeObject_IterNext(iter);
		if (!ITER_ISOK(next))
			break;
		Dee_Decref(result);
		result = next;
		if (DeeThread_CheckInterrupt())
			goto err_result;
	}
	Dee_Decref(iter);
	if unlikely(!next)
		Dee_Clear(result);
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return NULL;
err_result:
	Dee_Decref(result);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeMap_DelLast(DeeObject *__restrict self) {
	DREF DeeObject *iter, *key, *next;
	int result;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	key = DeeObject_IterNextKey(iter);
	if (!ITER_ISOK(key)) {
		Dee_Decref(iter);
		if (key == ITER_DONE)
			goto err_empty;
		goto err;
	}
	for (;;) {
		next = DeeObject_IterNextKey(iter);
		if (!ITER_ISOK(next))
			break;
		Dee_Decref(key);
		key = next;
	}
	Dee_Decref(iter);
	if unlikely(!next) {
		Dee_Decref(key);
		goto err;
	}
	result = DeeObject_DelItem(self, key);
	Dee_Decref(key);
	return result;
err_empty:
	err_empty_sequence(self);
err:
	return -1;
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
map_items(DeeObject *self) {
	return DeeSuper_New(&DeeSeq_Type, self);
}
#endif /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

PRIVATE struct type_getset tpconst map_getsets[] = {
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	TYPE_GETTER("keys", &default_map_keys,
	            "->?#Keys\n"
	            "Returns a ?DSequence that can be enumerated to view only the keys of @this ?."),
	TYPE_GETTER("values", &default_map_values,
	            "->?#Values\n"
	            "Returns a ?DSequence that can be enumerated to view only the values of @this ?."),
	TYPE_GETTER("items", &map_items,
	            "->?S?O\n"
	            "Returns a ?DSequence that can be enumerated to view the key-item "
	            /**/ "pairs as 2-element sequences, the same way they could be viewed "
	            /**/ "if @this ?. itself was being iterated\n"
	            "Same as ${this as Sequence}"),
	TYPE_GETTER("iterkeys", &default_map_iterkeys,
	            "->?AIterator?#Keys\n"
	            "Returns an iterator for ?#{keys}. Same as ${this.keys.operator iter()}"),
	TYPE_GETTER("itervalues", &default_map_itervalues,
	            "->?AIterator?#Values\n"
	            "Returns an iterator for ?#{values}. Same as ${this.values.operator iter()}"),
	TYPE_GETTER("iteritems", &DeeObject_Iter,
	            "->?DIterator\n"
	            "Returns an iterator for ?#{items}. Same as ${this.operator iter()}"),
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	TYPE_GETTER("keys", &map_keys,
	            "->?#Keys\n"
	            "Returns a ?DSequence that can be enumerated to view only the keys of @this ?."),
	TYPE_GETTER("values", &map_values,
	            "->?#Values\n"
	            "Returns a ?DSequence that can be enumerated to view only the values of @this ?."),
	TYPE_GETTER("items", &map_items,
	            "->?#Items\n"
	            "Returns a ?DSequence that can be enumerated to view the key-item "
	            /**/ "pairs as 2-element sequences, the same way they could be viewed "
	            /**/ "if @this ?. itself was being iterated\n"
	            "Note however that the returned ?DSequence is pure, meaning that it "
	            /**/ "implements a index-based getitem and getrange operators, the "
	            /**/ "same way one would expect of any regular object implementing "
	            /**/ "the sequence protocol"),
	TYPE_GETTER("iterkeys", &map_iterkeys,
	            "->?AIterator?#Keys\n"
	            "Returns an iterator for ?#{keys}. Same as ${this.keys.operator iter()}"),
	TYPE_GETTER("itervalues", &map_itervalues,
	            "->?AIterator?#Values\n"
	            "Returns an iterator for ?#{values}. Same as ${this.values.operator iter()}"),
	TYPE_GETTER("iteritems", &map_iteritems,
	            "->?AIterator?#Items\n"
	            "Returns an iterator for ?#{items}. Same as ${this.items.operator iter()}"),
	TYPE_GETSET_NODOC(STR_first, &DeeMap_GetFirst, &DeeMap_DelFirst, NULL),
	TYPE_GETSET_NODOC(STR_last, &DeeMap_GetLast, &DeeMap_DelLast, NULL),
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
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

#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Keys_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeSet_Type;
	Dee_tsc_map_keys_t tsc_map_keys = DeeType_RequireMapKeys(self);
	if (tsc_map_keys == &DeeMap_DefaultKeysWithMapIterKeys)
		result = &DefaultSequence_MapKeys_Type;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_Values_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeSeq_Type;
	Dee_tsc_map_values_t tsc_map_values = DeeType_RequireMapValues(self);
	if (tsc_map_values == &DeeMap_DefaultValuesWithMapIterValues)
		result = &DefaultSequence_MapValues_Type;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
map_IterKeys_get(DeeTypeObject *__restrict self) {
	DREF DeeTypeObject *result = &DeeIterator_Type;
	Dee_tsc_map_iterkeys_t tsc_map_iterkeys = DeeType_RequireMapIterKeys(self);
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
	Dee_tsc_map_itervalues_t tsc_map_itervalues = DeeType_RequireMapIterValues(self);
	if (tsc_map_itervalues == &DeeMap_DefaultIterValuesWithIter) {
		result = &DefaultIterator_WithNextValue;
	}
	return_reference_(result);
}
#endif /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */



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
	            "Returns the type of sequence returned by the #i:frozen property"),
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	TYPE_GETTER("Keys", &map_Keys_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the #i:keys property"),
	TYPE_GETTER("Values", &map_Values_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the #i:values property"),
	TYPE_GETTER("IterKeys", &map_IterKeys_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the #i:iterkeys property"),
	TYPE_GETTER("IterValues", &map_IterValues_get,
	            "->?DType\n"
	            "Returns the type of sequence returned by the #i:itervalues property"),
#endif /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	TYPE_GETSET_END
};

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
PRIVATE struct type_member tpconst map_class_members[] = {
	TYPE_MEMBER_CONST_DOC("Proxy", &DeeMappingProxy_Type,
	                      "->?DType\n"
	                      "The common base-class of ?#Keys, ?#Values and ?#Items"),
	TYPE_MEMBER_CONST_DOC("Keys", &DeeMappingKeys_Type,
	                      "->?DType\n"
	                      "The return type of the ?#keys member function"),
	TYPE_MEMBER_CONST_DOC("Values", &DeeMappingValues_Type,
	                      "->?DType\n"
	                      "The return type of the ?#values member function"),
	TYPE_MEMBER_CONST_DOC("Items", &DeeMappingItems_Type,
	                      "->?DType\n"
	                      "The return type of the ?#items member function"),

	/* TODO: KeyType->?DType
	 *       When this type of ?. only allows key of a certain ?DType,
	 *       this class attribute is overwritten with that ?DType. Else,
	 *       it simply evaluates to ?O
	 * TODO: ValueType->?DType
	 *       When this type of ?. only allows values of a certain ?DType,
	 *       this class attribute is overwritten with that ?DType. Else,
	 *       it simply evaluates to ?O */
	TYPE_MEMBER_END
};
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

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
	                         "The abstract declaration of a mapping-like sequence is ${{{object, object}...}}\n"
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
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	/* .tp_features = */ TF_NONE | (Dee_SEQCLASS_MAP << Dee_TF_SEQCLASS_SHFT),
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* .tp_features = */ TF_NONE,
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
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
	/* .tp_math          = */ &map_math,
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
#ifdef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
	/* .tp_class_members = */ NULL,
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
	/* .tp_class_members = */ map_class_members,
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
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
