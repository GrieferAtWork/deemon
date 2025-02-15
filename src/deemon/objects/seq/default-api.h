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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H 1

#ifndef LOCAL_FOR_VARIANTS
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#else /* !LOCAL_FOR_VARIANTS */
#define CONFIG_BUILDING_DEEMON
#define __SIZEOF_POINTER__ 4 /* Doesn't matter */
#define Dee_OPERATOR_USERCOUNT 0x003e /* Doesn't matter */
#define _DEE_WITHOUT_INCLUDES
#define Dee_SEQCLASS_SEQ 0
#define Dee_SEQCLASS_SET 1
#define Dee_SEQCLASS_MAP 2
#include "../../../../include/deemon/class.h"
#endif /* !LOCAL_FOR_VARIANTS */

DECL_BEGIN

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
/* How default API functions from "Sequence", "Set" and "Mapping" are implemented.
 *
 * This structure gets lazily calculated when it is first needed, based on features
 * exhibited by the respective sequence type.
 *
 * Individual function pointers within this structure are all NULL by default, and
 * populated as they are needed (meaning they are `[0..1][lock(WRITE_ONCE)]').
 * Unless otherwise documented, there is always a default available for *all* of
 * these operators.
 *
 *
 * NOTE:
 * - "Default" API functions can be overwritten by sub-classes (the runtime checks
 *   for this the first type a call is made, and will select default implementations
 *   based on which functions are overwritten)
 * - "Generic" API functions can NOT be overwritten (or if they are: the runtime
 *   probably won't actually use them). "Generic" API function will often try to
 *   make use of other APIs (usually Default ones, or operators) for their impl.
 */


union Dee_tsc_uslot {
	DREF DeeObject   *d_function;  /* [1..1][valid_if(:tsc_* == ...)] Thiscall function. */
	Dee_objmethod_t   d_method;    /* [1..1][valid_if(:tsc_* == ...)] Method callback. */
	Dee_kwobjmethod_t d_kwmethod;  /* [1..1][valid_if(:tsc_* == ...)] Method callback. */
};

struct Dee_type_seq_cache {
#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	Dee_mh_##func_name##_t tsc_##func_name;
#include "../../../../include/deemon/method-hints.def"

	/* Extra callbacks for Sequence.first/last */
	Dee_mh_seq_getfirst_t   tsc_seq_getfirst;
	Dee_mh_seq_boundfirst_t tsc_seq_boundfirst;
	Dee_mh_seq_delfirst_t   tsc_seq_delfirst;
	Dee_mh_seq_setfirst_t   tsc_seq_setfirst;
	Dee_mh_seq_getlast_t    tsc_seq_getlast;
	Dee_mh_seq_boundlast_t  tsc_seq_boundlast;
	Dee_mh_seq_dellast_t    tsc_seq_dellast;
	Dee_mh_seq_setlast_t    tsc_seq_setlast;

	/* Extra callbacks for Sequence.cached */
	Dee_mh_seq_cached_t     tsc_seq_cached;

	/* Extra callbacks for Mapping.keys/values/iterkeys/itervalues */
	Dee_mh_map_keys_t       tsc_map_keys;
	Dee_mh_map_values_t     tsc_map_values;
	Dee_mh_map_iterkeys_t   tsc_map_iterkeys;
	Dee_mh_map_itervalues_t tsc_map_itervalues;

	union Dee_tsc_uslot tsc_seq_getfirst_data;
	union Dee_tsc_uslot tsc_seq_delfirst_data;
	union Dee_tsc_uslot tsc_seq_setfirst_data;
	union Dee_tsc_uslot tsc_seq_getlast_data;
	union Dee_tsc_uslot tsc_seq_dellast_data;
	union Dee_tsc_uslot tsc_seq_setlast_data;
	union Dee_tsc_uslot tsc_seq_cached_data;
	union Dee_tsc_uslot tsc_seq_any_data;
	union Dee_tsc_uslot tsc_seq_all_data;
	union Dee_tsc_uslot tsc_seq_parity_data;
	union Dee_tsc_uslot tsc_seq_reduce_data;
	union Dee_tsc_uslot tsc_seq_min_data;
	union Dee_tsc_uslot tsc_seq_max_data;
	union Dee_tsc_uslot tsc_seq_sum_data;
	union Dee_tsc_uslot tsc_seq_count_data;
	union Dee_tsc_uslot tsc_seq_contains_data;
	union Dee_tsc_uslot tsc_seq_locate_data;
	union Dee_tsc_uslot tsc_seq_rlocate_data;
	union Dee_tsc_uslot tsc_seq_startswith_data;
	union Dee_tsc_uslot tsc_seq_endswith_data;
	union Dee_tsc_uslot tsc_seq_find_data;
	union Dee_tsc_uslot tsc_seq_rfind_data;
	union Dee_tsc_uslot tsc_seq_erase_data;
	union Dee_tsc_uslot tsc_seq_insert_data;
	union Dee_tsc_uslot tsc_seq_insertall_data;
	union Dee_tsc_uslot tsc_seq_pushfront_data;
	union Dee_tsc_uslot tsc_seq_append_data;
	union Dee_tsc_uslot tsc_seq_extend_data;
	union Dee_tsc_uslot tsc_seq_xchitem_data;
	union Dee_tsc_uslot tsc_seq_clear_data;
	union Dee_tsc_uslot tsc_seq_pop_data;
	union Dee_tsc_uslot tsc_seq_remove_data;
	union Dee_tsc_uslot tsc_seq_rremove_data;
	union Dee_tsc_uslot tsc_seq_removeall_data;
	union Dee_tsc_uslot tsc_seq_removeif_data;
	union Dee_tsc_uslot tsc_seq_resize_data;
	union Dee_tsc_uslot tsc_seq_fill_data;
	union Dee_tsc_uslot tsc_seq_reverse_data;
	union Dee_tsc_uslot tsc_seq_reversed_data;
	union Dee_tsc_uslot tsc_seq_sort_data;
	union Dee_tsc_uslot tsc_seq_sorted_data;
	union Dee_tsc_uslot tsc_seq_bfind_data;
	union Dee_tsc_uslot tsc_seq_bposition_data;
	union Dee_tsc_uslot tsc_seq_brange_data;
	union Dee_tsc_uslot tsc_set_insert_data;
	union Dee_tsc_uslot tsc_set_remove_data;
	union Dee_tsc_uslot tsc_set_unify_data;
	union Dee_tsc_uslot tsc_set_insertall_data;
	union Dee_tsc_uslot tsc_set_removeall_data;
	union Dee_tsc_uslot tsc_set_pop_data;
	union Dee_tsc_uslot tsc_map_setold_data;
	union Dee_tsc_uslot tsc_map_setold_ex_data;
	union Dee_tsc_uslot tsc_map_setnew_data;
	union Dee_tsc_uslot tsc_map_setnew_ex_data;
	union Dee_tsc_uslot tsc_map_setdefault_data;
	union Dee_tsc_uslot tsc_map_update_data;
	union Dee_tsc_uslot tsc_map_remove_data;
	union Dee_tsc_uslot tsc_map_removekeys_data;
	union Dee_tsc_uslot tsc_map_pop_data;
	union Dee_tsc_uslot tsc_map_popitem_data;
	union Dee_tsc_uslot tsc_map_keys_data;
	union Dee_tsc_uslot tsc_map_values_data;
	union Dee_tsc_uslot tsc_map_iterkeys_data;
	union Dee_tsc_uslot tsc_map_itervalues_data;
};

/* Destroy a lazily allocated sequence operator cache table. */
INTDEF NONNULL((1)) void DCALL
Dee_type_seq_cache_destroy(struct Dee_type_seq_cache *__restrict self);

INTDEF WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self);


/* Helpers for enumerating a sequence by invoking a given callback. */
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL seq_call_enumerate(DeeObject *self, DeeObject *cb);
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL seq_call_enumerate_with_intrange(DeeObject *self, DeeObject *cb, size_t start, size_t end);
INTDEF NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL seq_call_enumerate_with_range(DeeObject *self, DeeObject *cb, DeeObject *start, DeeObject *end);


/*
 * List default implementations returned by `DeeType_RequireSeqOperator*'
 */

/*[[[deemon
import * from deemon;
import ipc;
@@List of (SeqClass, OperatorName)
local operatorNames: {(string, string)...} = [];
local inOperators = false;
for (local l: File.open("../../../../include/deemon/method-hints.h", "rb")) {
	l = l.strip();
	if (!inOperators) {
		if (l == "/" "*[[[begin:seq_operators]]]*" "/")
			inOperators = true;
		continue;
	}
	if (l == "/" "*[[[end:seq_operators]]]*" "/")
		break;
	local START_MARKER = "DeeType_Require";
	local start = l.find(START_MARKER);
	if (start < 0)
		continue;
	start += #START_MARKER;
	local SEQ_CLASS_MARKER = "Operator";
	local seqClassMarker = l.find(SEQ_CLASS_MARKER, start);
	if (seqClassMarker < 0)
		continue;
	local seqClass = l[start:seqClassMarker];
	seqClassMarker += #SEQ_CLASS_MARKER;
	local end = l.find("(", seqClassMarker);
	if (end < 0)
		continue;
	local name = l[seqClassMarker:end].strip();
	operatorNames.append((seqClass, name));
}

for (local seqClass, name: operatorNames) {
	print File.stderr: ("loading: ", seqClass, ".", name);
	local proc = ipc.Process(ipc.Process.current.exe, [
		"deemon", "-E",
		f"-DLOCAL_FOR_VARIANTS",
		f"-DDEFINE_DeeType_Require{seqClass}Operator{name}",
		"default-api-require-operator-impl.c.inl"
	]);
	local r, w = ipc.Pipe.new()...;
	proc.stdout = w;
	proc.start();
	w.close();
	local data = r.readall();
	local e = proc.join();
	if (e != 0)
		throw Error(f"Process failed: {e}");
	data = "\n".join(for (local l: data.splitlines(false).each.strip())
		if (l && !l.startswith("#")) l
	);
	local impls = [f"Dee{seqClass}_Operator{name}"];
	local i = 0, end = #data;
	while (i < end) {
		none, i = data.refind(r"\breturn\s+&", i)...;
		if (i is none)
			break;
		local exprEnd = data.find(";", i);
		if (exprEnd < 0)
			break;
		local expr = data[i:exprEnd].strip().strip("&").strip();
		if (expr !in impls)
			impls.append(expr);
		i = exprEnd + 1;
	}

	local pad = " " * ((operatorNames.each.last.length > ...) - #name);
	print(f"#define Dee{seqClass}_VariantsFor_Operator{name}(cb){pad}"),;
	for (local impl: impls)
		print(f" cb({impl})"),;
	print;
}
]]]*/
#define DeeSeq_VariantsFor_OperatorBool(cb)                    cb(DeeSeq_OperatorBool) cb((*(Dee_mh_seq_operator_bool_t)&_DeeNone_reti0_1)) cb(DeeSeq_DefaultBoolWithSize) cb(DeeSeq_DefaultBoolWithSizeOb) cb(DeeSeq_DefaultBoolWithForeach) cb(DeeSeq_DefaultBoolWithCompareEq) cb(DeeSeq_DefaultBoolWithEq) cb(DeeSeq_DefaultBoolWithNe) cb(DeeSeq_DefaultBoolWithForeachDefault) cb(DeeSeq_DefaultOperatorBoolWithError)
#define DeeSeq_VariantsFor_OperatorIter(cb)                    cb(DeeSeq_OperatorIter) cb(DeeSeq_DefaultOperatorIterWithError)
#define DeeSeq_VariantsFor_OperatorSizeOb(cb)                  cb(DeeSeq_OperatorSizeOb) cb(DeeSeq_DefaultOperatorSizeObWithEmpty) cb(DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize) cb(DeeSeq_DefaultOperatorSizeObWithError)
#define DeeSeq_VariantsFor_OperatorContains(cb)                cb(DeeSeq_OperatorContains) cb(DeeSeq_DefaultOperatorContainsWithMapTryGetItem) cb(DeeSeq_DefaultOperatorContainsWithEmpty) cb(DeeSeq_DefaultContainsWithForeachDefault) cb(DeeSeq_DefaultOperatorContainsWithError)
#define DeeSeq_VariantsFor_OperatorGetItem(cb)                 cb(DeeSeq_OperatorGetItem) cb(DeeSeq_DefaultOperatorGetItemWithEmpty) cb(DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex) cb(DeeSeq_DefaultOperatorGetItemWithError)
#define DeeSeq_VariantsFor_OperatorDelItem(cb)                 cb(DeeSeq_OperatorDelItem) cb(DeeSeq_DefaultOperatorDelItemWithEmpty) cb(DeeSeq_DefaultOperatorDelItemWithSeqDelItemIndex) cb(DeeSeq_DefaultOperatorDelItemWithError)
#define DeeSeq_VariantsFor_OperatorSetItem(cb)                 cb(DeeSeq_OperatorSetItem) cb(DeeSeq_DefaultOperatorSetItemWithEmpty) cb(DeeSeq_DefaultOperatorSetItemWithSeqSetItemIndex) cb(DeeSeq_DefaultOperatorSetItemWithError)
#define DeeSeq_VariantsFor_OperatorGetRange(cb)                cb(DeeSeq_OperatorGetRange) cb(DeeSeq_DefaultOperatorGetRangeWithEmpty) cb(DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndSeqGetRangeIndexN) cb(DeeSeq_DefaultOperatorGetRangeWithError)
#define DeeSeq_VariantsFor_OperatorDelRange(cb)                cb(DeeSeq_OperatorDelRange) cb((*(Dee_mh_seq_operator_delrange_t)&_DeeNone_reti0_3)) cb(DeeSeq_DefaultOperatorDelRangeWithSeqDelRangeIndexAndSeqDelRangeIndexN) cb(DeeSeq_DefaultOperatorDelRangeWithError)
#define DeeSeq_VariantsFor_OperatorSetRange(cb)                cb(DeeSeq_OperatorSetRange) cb((*(Dee_mh_seq_operator_setrange_t)&_DeeNone_reti0_4)) cb(DeeSeq_DefaultOperatorSetRangeWithSeqSetRangeIndexAndSeqSetRangeIndexN) cb(DeeSeq_DefaultOperatorSetRangeWithError)
#define DeeSeq_VariantsFor_OperatorForeach(cb)                 cb(DeeSeq_OperatorForeach) cb(DeeSeq_DefaultOperatorForeachWithError)
#define DeeSeq_VariantsFor_OperatorForeachPair(cb)             cb(DeeSeq_OperatorForeachPair) cb((*(Dee_mh_seq_operator_foreach_pair_t)&DeeSeq_DefaultOperatorForeachWithError))
#define DeeSeq_VariantsFor_OperatorEnumerate(cb)               cb(DeeSeq_OperatorEnumerate) cb((*(Dee_mh_seq_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithEmpty)) cb(DeeSeq_DefaultEnumerateWithCounterAndForeach) cb((*(Dee_mh_seq_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithError))
#define DeeSeq_VariantsFor_OperatorEnumerateIndex(cb)          cb(DeeSeq_OperatorEnumerateIndex) cb(DeeSeq_DefaultOperatorEnumerateIndexWithEmpty) cb(DeeSeq_DefaultEnumerateIndexWithCounterAndIter) cb(DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault) cb(DeeSeq_DefaultOperatorEnumerateIndexWithError)
#define DeeSeq_VariantsFor_OperatorIterKeys(cb)                cb(DeeSeq_OperatorIterKeys) cb(DeeSeq_DefaultOperatorIterWithEmpty) cb(DeeSeq_DefaultOperatorIterKeysWithSeqSize) cb(DeeSeq_DefaultOperatorIterKeysWithError)
#define DeeSeq_VariantsFor_OperatorBoundItem(cb)               cb(DeeSeq_OperatorBoundItem) cb(DeeSeq_DefaultOperatorBoundItemWithEmpty) cb(DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex) cb(DeeSeq_DefaultOperatorBoundItemWithError)
#define DeeSeq_VariantsFor_OperatorHasItem(cb)                 cb(DeeSeq_OperatorHasItem) cb((*(Dee_mh_seq_operator_hasitem_t)&_DeeNone_reti0_2)) cb(DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex) cb(DeeSeq_DefaultOperatorHasItemWithError)
#define DeeSeq_VariantsFor_OperatorSize(cb)                    cb(DeeSeq_OperatorSize) cb(DeeSeq_DefaultOperatorSizeWithEmpty) cb(DeeSeq_DefaultSizeWithForeach) cb(DeeSeq_DefaultSizeWithForeachPair) cb(DeeSeq_DefaultOperatorSizeWithError)
#define DeeSeq_VariantsFor_OperatorSizeFast(cb)                cb(DeeSeq_OperatorSizeFast) cb(DeeObject_DefaultSizeFastWithErrorNotFast)
#define DeeSeq_VariantsFor_OperatorGetItemIndex(cb)            cb(DeeSeq_OperatorGetItemIndex) cb(DeeSeq_DefaultOperatorGetItemIndexWithEmpty) cb(DeeSeq_DefaultGetItemIndexWithForeachDefault) cb(DeeSeq_DefaultOperatorGetItemIndexWithError)
#define DeeSeq_VariantsFor_OperatorDelItemIndex(cb)            cb(DeeSeq_OperatorDelItemIndex) cb(DeeSeq_DefaultOperatorDelItemIndexWithError)
#define DeeSeq_VariantsFor_OperatorSetItemIndex(cb)            cb(DeeSeq_OperatorSetItemIndex) cb(DeeSeq_DefaultOperatorSetItemIndexWithError)
#define DeeSeq_VariantsFor_OperatorBoundItemIndex(cb)          cb(DeeSeq_OperatorBoundItemIndex) cb((*(Dee_mh_seq_operator_bounditem_index_t)&DeeSeq_DefaultOperatorBoundItemWithEmpty)) cb(DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize) cb(DeeSeq_DefaultOperatorBoundItemIndexWithError)
#define DeeSeq_VariantsFor_OperatorHasItemIndex(cb)            cb(DeeSeq_OperatorHasItemIndex) cb((*(Dee_mh_seq_operator_hasitem_index_t)&(*(Dee_mh_seq_operator_hasitem_t)&_DeeNone_reti0_2))) cb(DeeSeq_DefaultOperatorHasItemIndexWithSeqSize) cb(DeeSeq_DefaultOperatorHasItemIndexWithError)
#define DeeSeq_VariantsFor_OperatorGetRangeIndex(cb)           cb(DeeSeq_OperatorGetRangeIndex) cb((*(Dee_mh_seq_operator_getrange_index_t)&DeeSeq_DefaultOperatorGetRangeWithEmpty)) cb(DeeSeq_DefaultOperatorGetRangeIndexWithSeqIterAndSeqSize) cb(DeeSeq_DefaultOperatorGetRangeIndexWithError)
#define DeeSeq_VariantsFor_OperatorDelRangeIndex(cb)           cb(DeeSeq_OperatorDelRangeIndex) cb(DeeSeq_DefaultOperatorDelRangeIndexWithError)
#define DeeSeq_VariantsFor_OperatorSetRangeIndex(cb)           cb(DeeSeq_OperatorSetRangeIndex) cb(DeeSeq_DefaultOperatorSetRangeIndexWithError)
#define DeeSeq_VariantsFor_OperatorGetRangeIndexN(cb)          cb(DeeSeq_OperatorGetRangeIndexN) cb(DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty) cb(DeeSeq_DefaultOperatorGetRangeIndexNWithSeqIterAndSeqSize) cb(DeeSeq_DefaultOperatorGetRangeIndexNWithError)
#define DeeSeq_VariantsFor_OperatorDelRangeIndexN(cb)          cb(DeeSeq_OperatorDelRangeIndexN) cb(DeeSeq_DefaultOperatorDelRangeIndexNWithError)
#define DeeSeq_VariantsFor_OperatorSetRangeIndexN(cb)          cb(DeeSeq_OperatorSetRangeIndexN) cb(DeeSeq_DefaultOperatorSetRangeIndexNWithError)
#define DeeSeq_VariantsFor_OperatorTryGetItem(cb)              cb(DeeSeq_OperatorTryGetItem) cb(DeeSeq_DefaultOperatorTryGetItemWithEmpty) cb(DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex) cb(DeeSeq_DefaultOperatorGetItemWithError)
#define DeeSeq_VariantsFor_OperatorTryGetItemIndex(cb)         cb(DeeSeq_OperatorTryGetItemIndex) cb((*(Dee_mh_seq_operator_trygetitem_index_t)&DeeSeq_DefaultOperatorTryGetItemWithEmpty)) cb(DeeSeq_DefaultTryGetItemIndexWithForeachDefault) cb(DeeSeq_DefaultOperatorGetItemIndexWithError)
#define DeeSeq_VariantsFor_OperatorHash(cb)                    cb(DeeSeq_OperatorHash) cb(DeeSeq_DefaultOperatorHashWithEmpty) cb(DeeSeq_DefaultHashWithForeachDefault) cb(DeeSeq_DefaultOperatorHashWithError)
#define DeeSeq_VariantsFor_OperatorCompareEq(cb)               cb(DeeSeq_OperatorCompareEq) cb(DeeSeq_DefaultOperatorCompareWithEmpty) cb(DeeSeq_DefaultCompareEqWithForeachDefault) cb(DeeSeq_DefaultOperatorCompareEqWithError)
#define DeeSeq_VariantsFor_OperatorCompare(cb)                 cb(DeeSeq_OperatorCompare) cb(DeeSeq_DefaultOperatorCompareWithEmpty) cb(DeeSeq_DefaultCompareWithForeachDefault) cb(DeeSeq_DefaultOperatorCompareWithError)
#define DeeSeq_VariantsFor_OperatorTryCompareEq(cb)            cb(DeeSeq_OperatorTryCompareEq) cb(DeeSeq_DefaultOperatorTryCompareEqWithEmpty) cb(DeeSeq_DefaultCompareEqWithForeachDefault) cb(DeeSeq_DefaultOperatorTryCompareEqWithError)
#define DeeSeq_VariantsFor_OperatorEq(cb)                      cb(DeeSeq_OperatorEq) cb(DeeSeq_DefaultOperatorEqWithEmpty) cb(DeeSeq_DefaultOperatorEqWithSeqCompareEq) cb(DeeSeq_DefaultOperatorEqWithError)
#define DeeSeq_VariantsFor_OperatorNe(cb)                      cb(DeeSeq_OperatorNe) cb(DeeSeq_DefaultOperatorNeWithEmpty) cb(DeeSeq_DefaultOperatorNeWithSeqCompareEq) cb(DeeSeq_DefaultOperatorNeWithError)
#define DeeSeq_VariantsFor_OperatorLo(cb)                      cb(DeeSeq_OperatorLo) cb(DeeSeq_DefaultOperatorLoWithEmpty) cb(DeeSeq_DefaultOperatorLoWithSeqCompare) cb(DeeSeq_DefaultOperatorLoWithError)
#define DeeSeq_VariantsFor_OperatorLe(cb)                      cb(DeeSeq_OperatorLe) cb(DeeSeq_DefaultOperatorLeWithEmpty) cb(DeeSeq_DefaultOperatorLeWithSeqCompare) cb(DeeSeq_DefaultOperatorLeWithError)
#define DeeSeq_VariantsFor_OperatorGr(cb)                      cb(DeeSeq_OperatorGr) cb(DeeSeq_DefaultOperatorGrWithEmpty) cb(DeeSeq_DefaultOperatorGrWithSeqCompare) cb(DeeSeq_DefaultOperatorGrWithError)
#define DeeSeq_VariantsFor_OperatorGe(cb)                      cb(DeeSeq_OperatorGe) cb(DeeSeq_DefaultOperatorGeWithEmpty) cb(DeeSeq_DefaultOperatorGeWithSeqCompare) cb(DeeSeq_DefaultOperatorGeWithError)
#define DeeSeq_VariantsFor_OperatorInplaceAdd(cb)              cb(DeeSeq_OperatorInplaceAdd) cb(DeeSeq_DefaultOperatorInplaceAddWithSeqExtend) cb(DeeObject_DefaultInplaceAddWithAdd)
#define DeeSeq_VariantsFor_OperatorInplaceMul(cb)              cb(DeeSeq_OperatorInplaceMul) cb(DeeSeq_DefaultOperatorInplaceMulWithSeqClearAndSeqExtend) cb(DeeObject_DefaultInplaceMulWithMul)
#define DeeSet_VariantsFor_OperatorIter(cb)                    cb(DeeSet_OperatorIter) cb(DeeSeq_DefaultOperatorIterWithEmpty) cb(DeeSet_DefaultOperatorIterWithDistinctIter) cb(DeeSet_DefaultOperatorIterWithError)
#define DeeSet_VariantsFor_OperatorForeach(cb)                 cb(DeeSet_OperatorForeach) cb(DeeSeq_DefaultOperatorForeachWithEmpty) cb(DeeSet_DefaultOperatorForeachWithDistinctForeach) cb(DeeSet_DefaultOperatorForeachWithError)
#define DeeSet_VariantsFor_OperatorSize(cb)                    cb(DeeSet_OperatorSize) cb(DeeSeq_DefaultOperatorSizeWithEmpty) cb(DeeSet_DefaultOperatorSizeWithSetForeach) cb(DeeSet_DefaultOperatorSizeWithError)
#define DeeSet_VariantsFor_OperatorSizeOb(cb)                  cb(DeeSet_OperatorSizeOb) cb(DeeSeq_DefaultOperatorSizeObWithEmpty) cb(DeeSet_DefaultOperatorSizeObWithSetSize) cb(DeeSet_DefaultOperatorSizeObWithError)
#define DeeSet_VariantsFor_OperatorHash(cb)                    cb(DeeSet_OperatorHash) cb(DeeSeq_DefaultOperatorHashWithEmpty) cb(DeeSet_DefaultHashWithForeachDefault) cb(DeeMap_DefaultHashWithForeachPairDefault) cb(DeeSeq_DefaultOperatorHashWithError)
#define DeeSet_VariantsFor_OperatorCompareEq(cb)               cb(DeeSet_OperatorCompareEq) cb(DeeSeq_DefaultOperatorCompareWithEmpty) cb(DeeSet_DefaultCompareEqWithForeachDefault) cb(DeeSet_DefaultOperatorCompareEqWithError)
#define DeeSet_VariantsFor_OperatorTryCompareEq(cb)            cb(DeeSet_OperatorTryCompareEq) cb(DeeSeq_DefaultOperatorTryCompareEqWithEmpty) cb(DeeSet_DefaultCompareEqWithForeachDefault) cb(DeeSet_DefaultOperatorTryCompareEqWithError)
#define DeeSet_VariantsFor_OperatorEq(cb)                      cb(DeeSet_OperatorEq) cb(DeeSeq_DefaultOperatorEqWithEmpty) cb(DeeSet_DefaultOperatorEqWithSetCompareEq) cb(DeeSet_DefaultOperatorEqWithError)
#define DeeSet_VariantsFor_OperatorNe(cb)                      cb(DeeSet_OperatorNe) cb(DeeSeq_DefaultOperatorNeWithEmpty) cb(DeeSet_DefaultOperatorNeWithSetCompareEq) cb(DeeSet_DefaultOperatorNeWithError)
#define DeeSet_VariantsFor_OperatorLo(cb)                      cb(DeeSet_OperatorLo) cb(DeeSeq_DefaultOperatorNeWithEmpty) cb(DeeSet_DefaultLoWithForeachDefault) cb(DeeSet_DefaultOperatorLoWithError)
#define DeeSet_VariantsFor_OperatorLe(cb)                      cb(DeeSet_OperatorLe) cb(DeeSet_DefaultOperatorLeWithEmpty) cb(DeeSet_DefaultLeWithForeachDefault) cb(DeeSet_DefaultOperatorLeWithError)
#define DeeSet_VariantsFor_OperatorGr(cb)                      cb(DeeSet_OperatorGr) cb(DeeSet_DefaultOperatorGrWithEmpty) cb(DeeSet_DefaultGrWithForeachDefault) cb(DeeSet_DefaultOperatorGrWithError)
#define DeeSet_VariantsFor_OperatorGe(cb)                      cb(DeeSet_OperatorGe) cb(DeeSeq_DefaultOperatorEqWithEmpty) cb(DeeSet_DefaultGeWithForeachDefault) cb(DeeSet_DefaultOperatorGeWithError)
#define DeeSet_VariantsFor_OperatorInv(cb)                     cb(DeeSet_OperatorInv) cb(DeeSet_DefaultOperatorInvWithEmpty) cb(DeeSet_DefaultOperatorInvWithForeach) cb(DeeSet_DefaultOperatorInvWithError)
#define DeeSet_VariantsFor_OperatorAdd(cb)                     cb(DeeSet_OperatorAdd) cb(DeeSet_DefaultOperatorAddWithEmpty) cb(DeeSet_DefaultOperatorAddWithForeach) cb(DeeSet_DefaultOperatorAddWithError)
#define DeeSet_VariantsFor_OperatorSub(cb)                     cb(DeeSet_OperatorSub) cb(DeeSet_DefaultOperatorSubWithEmpty) cb(DeeSet_DefaultOperatorSubWithForeach) cb(DeeSet_DefaultOperatorSubWithError)
#define DeeSet_VariantsFor_OperatorAnd(cb)                     cb(DeeSet_OperatorAnd) cb(DeeSet_DefaultOperatorSubWithEmpty) cb(DeeSet_DefaultOperatorAndWithForeach) cb(DeeSet_DefaultOperatorAndWithError)
#define DeeSet_VariantsFor_OperatorXor(cb)                     cb(DeeSet_OperatorXor) cb(DeeSet_DefaultOperatorAddWithEmpty) cb(DeeSet_DefaultOperatorXorWithForeach) cb(DeeSet_DefaultOperatorXorWithError)
#define DeeSet_VariantsFor_OperatorInplaceAdd(cb)              cb(DeeSet_OperatorInplaceAdd) cb(DeeSet_DefaultOperatorInplaceAddWithSetInsertAll) cb(DeeSet_DefaultOperatorInplaceAddWithError)
#define DeeSet_VariantsFor_OperatorInplaceSub(cb)              cb(DeeSet_OperatorInplaceSub) cb(DeeSet_DefaultOperatorInplaceSubWithSetRemoveAll) cb(DeeSet_DefaultOperatorInplaceSubWithError)
#define DeeSet_VariantsFor_OperatorInplaceAnd(cb)              cb(DeeSet_OperatorInplaceAnd) cb(DeeSet_DefaultOperatorInplaceAndWithForeachAndSetRemoveAll) cb(DeeSet_DefaultOperatorInplaceAndWithError)
#define DeeSet_VariantsFor_OperatorInplaceXor(cb)              cb(DeeSet_OperatorInplaceXor) cb(DeeSet_DefaultOperatorInplaceXorWithForeachAndSetInsertAllAndSetRemoveAll) cb(DeeSet_DefaultOperatorInplaceXorWithError)
#define DeeMap_VariantsFor_OperatorContains(cb)                cb(DeeMap_OperatorContains) cb(DeeSeq_DefaultOperatorContainsWithEmpty) cb(DeeMap_DefaultContainsWithForeachPair) cb(DeeMap_DefaultOperatorContainsWithError)
#define DeeMap_VariantsFor_OperatorGetItem(cb)                 cb(DeeMap_OperatorGetItem) cb(DeeMap_DefaultOperatorGetItemWithEmpty) cb(DeeMap_DefaultGetItemWithForeachPair) cb(DeeMap_DefaultOperatorGetItemWithError)
#define DeeMap_VariantsFor_OperatorDelItem(cb)                 cb(DeeMap_OperatorDelItem) cb(DeeMap_DefaultOperatorDelItemWithError)
#define DeeMap_VariantsFor_OperatorSetItem(cb)                 cb(DeeMap_OperatorSetItem) cb(DeeMap_DefaultOperatorSetItemWithError)
#define DeeMap_VariantsFor_OperatorEnumerate(cb)               cb(DeeMap_OperatorEnumerate) cb((*(Dee_mh_seq_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithEmpty)) cb(DeeMap_DefaultEnumerateWithForeachPairDefault) cb(DeeMap_DefaultOperatorEnumerateWithError)
#define DeeMap_VariantsFor_OperatorEnumerateIndex(cb)          cb(DeeMap_OperatorEnumerateIndex) cb(DeeSeq_DefaultOperatorEnumerateIndexWithEmpty) cb(DeeMap_DefaultEnumerateIndexWithForeachPair) cb(DeeMap_DefaultOperatorEnumerateIndexWithError)
#define DeeMap_VariantsFor_OperatorBoundItem(cb)               cb(DeeMap_OperatorBoundItem) cb(DeeSeq_DefaultOperatorBoundItemWithEmpty) cb(DeeMap_DefaultBoundItemWithForeachPair) cb(DeeMap_DefaultOperatorBoundItemWithError)
#define DeeMap_VariantsFor_OperatorHasItem(cb)                 cb(DeeMap_OperatorHasItem) cb((*(Dee_mh_seq_operator_hasitem_t)&_DeeNone_reti0_2)) cb(DeeMap_DefaultHasItemWithForeachPair) cb(DeeMap_DefaultOperatorHasItemWithError)
#define DeeMap_VariantsFor_OperatorGetItemIndex(cb)            cb(DeeMap_OperatorGetItemIndex) cb(DeeMap_DefaultOperatorGetItemIndexWithEmpty) cb(DeeMap_DefaultGetItemIndexWithForeachPair) cb(DeeMap_DefaultOperatorGetItemIndexWithError)
#define DeeMap_VariantsFor_OperatorDelItemIndex(cb)            cb(DeeMap_OperatorDelItemIndex) cb(DeeMap_DefaultOperatorDelItemIndexWithError)
#define DeeMap_VariantsFor_OperatorSetItemIndex(cb)            cb(DeeMap_OperatorSetItemIndex) cb(DeeMap_DefaultOperatorSetItemIndexWithError)
#define DeeMap_VariantsFor_OperatorBoundItemIndex(cb)          cb(DeeMap_OperatorBoundItemIndex) cb((*(Dee_mh_seq_operator_bounditem_index_t)&DeeSeq_DefaultOperatorBoundItemWithEmpty)) cb(DeeMap_DefaultBoundItemIndexWithForeachPair) cb(DeeMap_DefaultOperatorBoundItemIndexWithError)
#define DeeMap_VariantsFor_OperatorHasItemIndex(cb)            cb(DeeMap_OperatorHasItemIndex) cb((*(Dee_mh_seq_operator_hasitem_index_t)&(*(Dee_mh_seq_operator_hasitem_t)&_DeeNone_reti0_2))) cb(DeeMap_DefaultHasItemIndexWithForeachPair) cb(DeeMap_DefaultOperatorHasItemIndexWithError)
#define DeeMap_VariantsFor_OperatorTryGetItem(cb)              cb(DeeMap_OperatorTryGetItem) cb(DeeSeq_DefaultOperatorTryGetItemWithEmpty) cb(DeeMap_DefaultTryGetItemWithForeachPair) cb(DeeMap_DefaultOperatorGetItemWithError)
#define DeeMap_VariantsFor_OperatorTryGetItemIndex(cb)         cb(DeeMap_OperatorTryGetItemIndex) cb((*(Dee_mh_seq_operator_trygetitem_index_t)&DeeSeq_DefaultOperatorTryGetItemWithEmpty)) cb(DeeMap_DefaultTryGetItemIndexWithForeachPair) cb(DeeMap_DefaultOperatorGetItemIndexWithError)
#define DeeMap_VariantsFor_OperatorTryGetItemStringHash(cb)    cb(DeeMap_OperatorTryGetItemStringHash) cb(DeeMap_DefaultOperatorTryGetItemStringHashWithEmpty) cb(DeeMap_DefaultTryGetItemStringHashWithForeachPair) cb(DeeMap_DefaultOperatorGetItemStringHashWithError)
#define DeeMap_VariantsFor_OperatorGetItemStringHash(cb)       cb(DeeMap_OperatorGetItemStringHash) cb(DeeMap_DefaultOperatorGetItemStringHashWithEmpty) cb(DeeMap_DefaultGetItemStringHashWithForeachPair) cb(DeeMap_DefaultOperatorGetItemStringHashWithError)
#define DeeMap_VariantsFor_OperatorDelItemStringHash(cb)       cb(DeeMap_OperatorDelItemStringHash) cb(DeeMap_DefaultOperatorDelItemStringHashWithError)
#define DeeMap_VariantsFor_OperatorSetItemStringHash(cb)       cb(DeeMap_OperatorSetItemStringHash) cb(DeeMap_DefaultOperatorSetItemStringHashWithError)
#define DeeMap_VariantsFor_OperatorBoundItemStringHash(cb)     cb(DeeMap_OperatorBoundItemStringHash) cb(DeeMap_DefaultOperatorBoundItemStringHashWithEmpty) cb(DeeMap_DefaultBoundItemStringHashWithForeachPair) cb(DeeMap_DefaultOperatorBoundItemStringHashWithError)
#define DeeMap_VariantsFor_OperatorHasItemStringHash(cb)       cb(DeeMap_OperatorHasItemStringHash) cb((*(Dee_mh_map_operator_hasitem_string_hash_t)&_DeeNone_reti0_3)) cb(DeeMap_DefaultHasItemStringHashWithForeachPair) cb(DeeMap_DefaultOperatorHasItemStringHashWithError)
#define DeeMap_VariantsFor_OperatorTryGetItemStringLenHash(cb) cb(DeeMap_OperatorTryGetItemStringLenHash) cb(DeeMap_DefaultOperatorTryGetItemStringLenHashWithEmpty) cb(DeeMap_DefaultTryGetItemStringLenHashWithForeachPair) cb(DeeMap_DefaultOperatorGetItemStringLenHashWithError)
#define DeeMap_VariantsFor_OperatorGetItemStringLenHash(cb)    cb(DeeMap_OperatorGetItemStringLenHash) cb(DeeMap_DefaultOperatorGetItemStringLenHashWithEmpty) cb(DeeMap_DefaultGetItemStringLenHashWithForeachPair) cb(DeeMap_DefaultOperatorGetItemStringLenHashWithError)
#define DeeMap_VariantsFor_OperatorDelItemStringLenHash(cb)    cb(DeeMap_OperatorDelItemStringLenHash) cb(DeeMap_DefaultOperatorDelItemStringLenHashWithError)
#define DeeMap_VariantsFor_OperatorSetItemStringLenHash(cb)    cb(DeeMap_OperatorSetItemStringLenHash) cb(DeeMap_DefaultOperatorSetItemStringLenHashWithError)
#define DeeMap_VariantsFor_OperatorBoundItemStringLenHash(cb)  cb(DeeMap_OperatorBoundItemStringLenHash) cb(DeeMap_DefaultOperatorBoundItemStringLenHashWithEmpty) cb(DeeMap_DefaultBoundItemStringLenHashWithForeachPair) cb(DeeMap_DefaultOperatorBoundItemStringLenHashWithError)
#define DeeMap_VariantsFor_OperatorHasItemStringLenHash(cb)    cb(DeeMap_OperatorHasItemStringLenHash) cb((*(Dee_mh_map_operator_hasitem_string_len_hash_t)&_DeeNone_reti0_4)) cb(DeeMap_DefaultHasItemStringLenHashWithForeachPair) cb(DeeMap_DefaultOperatorHasItemStringLenHashWithError)
#define DeeMap_VariantsFor_OperatorCompareEq(cb)               cb(DeeMap_OperatorCompareEq) cb(DeeSeq_DefaultOperatorCompareWithEmpty) cb(DeeMap_DefaultCompareEqWithForeachPairDefault) cb(DeeMap_DefaultOperatorCompareEqWithError)
#define DeeMap_VariantsFor_OperatorTryCompareEq(cb)            cb(DeeMap_OperatorTryCompareEq) cb(DeeSeq_DefaultOperatorTryCompareEqWithEmpty) cb(DeeMap_DefaultCompareEqWithForeachPairDefault) cb(DeeMap_DefaultOperatorTryCompareEqWithError)
#define DeeMap_VariantsFor_OperatorEq(cb)                      cb(DeeMap_OperatorEq) cb(DeeSeq_DefaultOperatorEqWithEmpty) cb(DeeMap_DefaultOperatorEqWithMapCompareEq) cb(DeeMap_DefaultOperatorEqWithError)
#define DeeMap_VariantsFor_OperatorNe(cb)                      cb(DeeMap_OperatorNe) cb(DeeSeq_DefaultOperatorNeWithEmpty) cb(DeeMap_DefaultOperatorNeWithMapCompareEq) cb(DeeMap_DefaultOperatorNeWithError)
#define DeeMap_VariantsFor_OperatorLo(cb)                      cb(DeeMap_OperatorLo) cb(DeeSeq_DefaultOperatorNeWithEmpty) cb(DeeMap_DefaultLoWithForeachPairDefault) cb(DeeMap_DefaultOperatorLoWithError)
#define DeeMap_VariantsFor_OperatorLe(cb)                      cb(DeeMap_OperatorLe) cb(DeeSet_DefaultOperatorLeWithEmpty) cb(DeeMap_DefaultLeWithForeachPairDefault) cb(DeeMap_DefaultOperatorLeWithError)
#define DeeMap_VariantsFor_OperatorGr(cb)                      cb(DeeMap_OperatorGr) cb(DeeSet_DefaultOperatorGrWithEmpty) cb(DeeMap_DefaultGrWithForeachPairDefault) cb(DeeMap_DefaultOperatorGrWithError)
#define DeeMap_VariantsFor_OperatorGe(cb)                      cb(DeeMap_OperatorGe) cb(DeeSeq_DefaultOperatorEqWithEmpty) cb(DeeMap_DefaultGeWithForeachPairDefault) cb(DeeMap_DefaultOperatorGeWithError)
#define DeeMap_VariantsFor_OperatorAdd(cb)                     cb(DeeMap_OperatorAdd) cb(DeeMap_DefaultOperatorAddWithEmpty) cb(DeeMap_DefaultOperatorAddWithForeach) cb(DeeMap_DefaultOperatorAddWithError)
#define DeeMap_VariantsFor_OperatorSub(cb)                     cb(DeeMap_OperatorSub) cb(DeeSet_DefaultOperatorSubWithEmpty) cb(DeeMap_DefaultOperatorSubWithForeach) cb(DeeMap_DefaultOperatorSubWithError)
#define DeeMap_VariantsFor_OperatorAnd(cb)                     cb(DeeMap_OperatorAnd) cb(DeeSet_DefaultOperatorSubWithEmpty) cb(DeeMap_DefaultOperatorAndWithForeach) cb(DeeMap_DefaultOperatorAndWithError)
#define DeeMap_VariantsFor_OperatorXor(cb)                     cb(DeeMap_OperatorXor) cb(DeeMap_DefaultOperatorAddWithEmpty) cb(DeeMap_DefaultOperatorXorWithForeach) cb(DeeMap_DefaultOperatorXorWithError)
#define DeeMap_VariantsFor_OperatorInplaceAdd(cb)              cb(DeeMap_OperatorInplaceAdd) cb(DeeMap_DefaultOperatorInplaceAddWithMapUpdate) cb(DeeMap_DefaultOperatorInplaceAddWithError)
#define DeeMap_VariantsFor_OperatorInplaceSub(cb)              cb(DeeMap_OperatorInplaceSub) cb(DeeMap_DefaultOperatorInplaceSubWithMapRemoveKeys) cb(DeeMap_DefaultOperatorInplaceSubWithError)
#define DeeMap_VariantsFor_OperatorInplaceAnd(cb)              cb(DeeMap_OperatorInplaceAnd) cb(DeeMap_DefaultOperatorInplaceAndWithForeachAndMapRemoveKeys) cb(DeeMap_DefaultOperatorInplaceAndWithError)
#define DeeMap_VariantsFor_OperatorInplaceXor(cb)              cb(DeeMap_OperatorInplaceXor) cb(DeeMap_DefaultOperatorInplaceXorWithForeachAndMapUpdatAndMapRemoveKeys) cb(DeeMap_DefaultOperatorInplaceXorWithError)
/*[[[end]]]*/


/* Possible implementations for sequence cache operators. */

/************************************************************************/
/* Default Seq Operators                                                */
/************************************************************************/
#define DeeSeq_DefaultOperatorBoolWithEmpty (*(Dee_mh_seq_operator_bool_t)&_DeeNone_reti0_1)
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoolWithEmpty(DeeObject *__restrict self);*/
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoolWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorIterWithEmpty(DeeObject *__restrict self);
INTDEF /*WUNUSED*/ NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorIterWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorSizeObWithEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorSizeObWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorContainsWithMapTryGetItem(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorContainsWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorContainsWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorDelItemWithSeqDelItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorDelItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorDelItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorSetItemWithSeqSetItemIndex(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorSetItemWithEmpty(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorSetItemWithError(DeeObject *self, DeeObject *index, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndSeqGetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeWithEmpty(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end);

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorDelRangeWithSeqDelRangeIndexAndSeqDelRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
#define DeeSeq_DefaultOperatorDelRangeWithEmpty (*(Dee_mh_seq_operator_delrange_t)&_DeeNone_reti0_3)
/*INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorDelRangeWithEmpty(DeeObject *self, DeeObject *start, DeeObject *end);*/
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorDelRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end);

INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeWithSeqSetRangeIndexAndSeqSetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
#define DeeSeq_DefaultOperatorSetRangeWithEmpty (*(Dee_mh_seq_operator_setrange_t)&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorForeachWithEmpty(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorForeachWithError(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);

#define DeeSeq_DefaultOperatorForeachPairWithEmpty (*(Dee_mh_seq_operator_foreach_pair_t)&DeeSeq_DefaultOperatorForeachWithEmpty)
#define DeeSeq_DefaultOperatorForeachPairWithError (*(Dee_mh_seq_operator_foreach_pair_t)&DeeSeq_DefaultOperatorForeachWithError)

#define DeeSeq_DefaultOperatorEnumerateWithEmpty (*(Dee_mh_seq_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithEmpty)
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateWithEmpty(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);*/
#define DeeSeq_DefaultOperatorEnumerateWithError (*(Dee_mh_seq_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithError)
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateWithError(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);*/

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateIndexWithEmpty(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateIndexWithError(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorIterKeysWithSeqSize(DeeObject *__restrict self);
#define DeeSeq_DefaultOperatorIterKeysWithEmpty DeeSeq_DefaultOperatorIterWithEmpty
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorIterKeysWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorBoundItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorBoundItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex(DeeObject *self, DeeObject *index);
/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorHasItemWithEmpty(DeeObject *self, DeeObject *index);*/
#define DeeSeq_DefaultOperatorHasItemWithEmpty (*(Dee_mh_seq_operator_hasitem_t)&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorHasItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultOperatorSizeWithEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultOperatorSizeWithError(DeeObject *__restrict self);

#define DeeSeq_DefaultOperatorSizeFastWithEmpty DeeSeq_DefaultOperatorSizeWithEmpty
#define DeeSeq_DefaultOperatorSizeFastWithError DeeObject_DefaultSizeFastWithErrorNotFast
/*INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultOperatorSizeFastWithError(DeeObject *__restrict self);*/

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemIndexWithEmpty(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelItemIndexWithEmpty(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetItemIndexWithEmpty(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetItemIndexWithError(DeeObject *self, size_t index, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize(DeeObject *self, size_t index);
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoundItemIndexWithEmpty(DeeObject *self, size_t index);*/
#define DeeSeq_DefaultOperatorBoundItemIndexWithEmpty (*(Dee_mh_seq_operator_bounditem_index_t)&DeeSeq_DefaultOperatorBoundItemWithEmpty)
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoundItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorHasItemIndexWithSeqSize(DeeObject *self, size_t index);
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorHasItemIndexWithEmpty(DeeObject *self, size_t index);*/
#define DeeSeq_DefaultOperatorHasItemIndexWithEmpty (*(Dee_mh_seq_operator_hasitem_index_t)&DeeSeq_DefaultOperatorHasItemWithEmpty)
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorHasItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexWithSeqIterAndSeqSize(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
/*INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexWithEmpty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);*/
#define DeeSeq_DefaultOperatorGetRangeIndexWithEmpty (*(Dee_mh_seq_operator_getrange_index_t)&DeeSeq_DefaultOperatorGetRangeWithEmpty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

#define DeeSeq_DefaultOperatorDelRangeIndexWithEmpty (*(Dee_mh_seq_operator_delrange_index_t)&_DeeNone_reti0_3)
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexWithEmpty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);*/
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

#define DeeSeq_DefaultOperatorSetRangeIndexWithEmpty (*(Dee_mh_seq_operator_setrange_index_t)&_DeeNone_reti0_4)
/*INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexWithEmpty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);*/
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexNWithSeqIterAndSeqSize(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexNWithError(DeeObject *self, Dee_ssize_t start);

#define DeeSeq_DefaultOperatorDelRangeIndexNWithEmpty (*(Dee_mh_seq_operator_delrange_index_n_t)&_DeeNone_reti0_2)
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start);*/
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexNWithError(DeeObject *self, Dee_ssize_t start);

#define DeeSeq_DefaultOperatorSetRangeIndexNWithEmpty (*(Dee_mh_seq_operator_setrange_index_n_t)&_DeeNone_reti0_3)
/*INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start, DeeObject *values);*/
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexNWithError(DeeObject *self, Dee_ssize_t start, DeeObject *values);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorTryGetItemWithEmpty(DeeObject *self, DeeObject *index);
#define DeeSeq_DefaultOperatorTryGetItemWithError DeeSeq_DefaultOperatorGetItemWithError

/*INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty(DeeObject *self, size_t index);*/
#define DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty (*(Dee_mh_seq_operator_trygetitem_index_t)&DeeSeq_DefaultOperatorTryGetItemWithEmpty)
#define DeeSeq_DefaultOperatorTryGetItemIndexWithError DeeSeq_DefaultOperatorGetItemIndexWithError

INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultOperatorHashWithEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultOperatorHashWithError(DeeObject *__restrict self);

/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareEqWithEmpty(DeeObject *self, DeeObject *some_object);*/
#define DeeSeq_DefaultOperatorCompareEqWithEmpty DeeSeq_DefaultOperatorCompareWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorTryCompareEqWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorEqWithSeqCompareEq(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorEqWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorNeWithSeqCompareEq(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorNeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLoWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLoWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLeWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGrWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGrWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGeWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorInplaceAddWithSeqExtend(DREF DeeObject **__restrict p_self, DeeObject *some_object);
#define DeeSeq_DefaultOperatorInplaceAddWithEmpty DeeSeq_DefaultOperatorInplaceAddWithError
#define DeeSeq_DefaultOperatorInplaceAddWithError DeeObject_DefaultInplaceAddWithAdd

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorInplaceMulWithSeqClearAndSeqExtend(DREF DeeObject **__restrict p_self, DeeObject *some_object);
#define DeeSeq_DefaultOperatorInplaceMulWithEmpty DeeSeq_DefaultOperatorInplaceMulWithError
#define DeeSeq_DefaultOperatorInplaceMulWithError DeeObject_DefaultInplaceMulWithMul





/************************************************************************/
/* Default Set Operators                                                */
/************************************************************************/
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorIterWithDistinctIter(DeeObject *__restrict self);
#define DeeSet_DefaultOperatorIterWithEmpty DeeSeq_DefaultOperatorIterWithEmpty
INTDEF /*WUNUSED*/ NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorIterWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSet_DefaultOperatorForeachWithDistinctForeach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define DeeSet_DefaultOperatorForeachWithEmpty DeeSeq_DefaultOperatorForeachWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSet_DefaultOperatorForeachWithError(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);

INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSet_DefaultOperatorSizeWithSetForeach(DeeObject *__restrict self);
#define DeeSet_DefaultOperatorSizeWithEmpty DeeSeq_DefaultOperatorSizeWithEmpty
INTDEF /*WUNUSED*/ NONNULL((1)) size_t DCALL DeeSet_DefaultOperatorSizeWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorSizeObWithSetSize(DeeObject *__restrict self);
#define DeeSet_DefaultOperatorSizeObWithEmpty DeeSeq_DefaultOperatorSizeObWithEmpty
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorSizeObWithError(DeeObject *__restrict self);

#define DeeSet_DefaultOperatorHashWithEmpty DeeSeq_DefaultOperatorHashWithEmpty
#define DeeSet_DefaultOperatorHashWithError DeeSeq_DefaultOperatorHashWithError

#define DeeSet_DefaultOperatorCompareEqWithEmpty DeeSeq_DefaultOperatorCompareEqWithEmpty
INTDEF /*WUNUSED*/ NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object);

#define DeeSet_DefaultOperatorTryCompareEqWithEmpty DeeSeq_DefaultOperatorTryCompareEqWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorEqWithSetCompareEq(DeeObject *self, DeeObject *some_object);
#define DeeSet_DefaultOperatorEqWithEmpty DeeSeq_DefaultOperatorEqWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorNeWithSetCompareEq(DeeObject *self, DeeObject *some_object);
#define DeeSet_DefaultOperatorNeWithEmpty DeeSeq_DefaultOperatorNeWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object);

#define DeeSet_DefaultOperatorLoWithEmpty DeeSeq_DefaultOperatorNeWithEmpty /* ({} as Set) < SOME_SEQ -- {} != SOME_SEQ -- SOME_SEQ.nonempty */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorLeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorGrWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object);

#define DeeSet_DefaultOperatorGeWithEmpty DeeSeq_DefaultOperatorEqWithEmpty /* ({} as Set) >= SOME_SEQ -- {} == SOME_SEQ -- SOME_SEQ.empty */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorInvWithEmpty(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorInvWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultOperatorInvWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorAddWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorAddWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorAddWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorSubWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorSubWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorSubWithError(DeeObject *self, DeeObject *some_object);

#define DeeSet_DefaultOperatorAndWithEmpty DeeSet_DefaultOperatorSubWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorAndWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorAndWithError(DeeObject *self, DeeObject *some_object);

#define DeeSet_DefaultOperatorXorWithEmpty DeeSet_DefaultOperatorAddWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorXorWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultOperatorXorWithError(DeeObject *self, DeeObject *some_object);

#define DeeSet_DefaultOperatorInplaceAddWithEmpty DeeSet_DefaultOperatorInplaceAddWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceAddWithSetInsertAll(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceAddWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);

#define DeeSet_DefaultOperatorInplaceSubWithEmpty DeeSet_DefaultOperatorInplaceSubWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceSubWithSetRemoveAll(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceSubWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);

#define DeeSet_DefaultOperatorInplaceAndWithEmpty DeeSet_DefaultOperatorInplaceAndWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceAndWithForeachAndSetRemoveAll(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceAndWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);

#define DeeSet_DefaultOperatorInplaceXorWithEmpty DeeSet_DefaultOperatorInplaceXorWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceXorWithForeachAndSetInsertAllAndSetRemoveAll(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultOperatorInplaceXorWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);






/************************************************************************/
/* Default Map Operators                                                */
/************************************************************************/
#define DeeMap_DefaultOperatorContainsWithEmpty DeeSeq_DefaultOperatorContainsWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorContainsWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemWithError(DeeObject *self, DeeObject *index);

#define DeeMap_DefaultOperatorDelItemWithEmpty (*(Dee_mh_seq_operator_delitem_t)&_DeeNone_reti0_2)
/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorDelItemWithEmpty(DeeObject *self, DeeObject *index);*/
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorDelItemWithError(DeeObject *self, DeeObject *index);

#define DeeMap_DefaultOperatorSetItemWithEmpty DeeMap_DefaultOperatorSetItemWithError
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultOperatorSetItemWithError(DeeObject *self, DeeObject *index, DeeObject *value);

#define DeeMap_DefaultOperatorEnumerateWithEmpty DeeSeq_DefaultOperatorEnumerateWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultOperatorEnumerateWithError(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);

#define DeeMap_DefaultOperatorEnumerateIndexWithEmpty DeeSeq_DefaultOperatorEnumerateIndexWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeMap_DefaultOperatorEnumerateIndexWithError(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);

#define DeeMap_DefaultOperatorBoundItemWithEmpty DeeSeq_DefaultOperatorBoundItemWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorBoundItemWithError(DeeObject *self, DeeObject *index);

#define DeeMap_DefaultOperatorHasItemWithEmpty DeeSeq_DefaultOperatorHasItemWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorHasItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemIndexWithEmpty(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemIndexWithError(DeeObject *self, size_t index);

#define DeeMap_DefaultOperatorDelItemIndexWithEmpty DeeMap_DefaultOperatorDelItemIndexWithError
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultOperatorDelItemIndexWithError(DeeObject *self, size_t index);

#define DeeMap_DefaultOperatorSetItemIndexWithEmpty DeeMap_DefaultOperatorSetItemIndexWithError
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeMap_DefaultOperatorSetItemIndexWithError(DeeObject *self, size_t index, DeeObject *value);

#define DeeMap_DefaultOperatorBoundItemIndexWithEmpty DeeSeq_DefaultOperatorBoundItemIndexWithEmpty
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultOperatorBoundItemIndexWithError(DeeObject *self, size_t index);

#define DeeMap_DefaultOperatorHasItemIndexWithEmpty DeeSeq_DefaultOperatorHasItemIndexWithEmpty
INTDEF WUNUSED NONNULL((1)) int DCALL DeeMap_DefaultOperatorHasItemIndexWithError(DeeObject *self, size_t index);

#define DeeMap_DefaultOperatorTryGetItemWithEmpty DeeSeq_DefaultOperatorTryGetItemWithEmpty
#define DeeMap_DefaultOperatorTryGetItemWithError DeeMap_DefaultOperatorGetItemWithError

#define DeeMap_DefaultOperatorTryGetItemIndexWithEmpty DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty
#define DeeMap_DefaultOperatorTryGetItemIndexWithError DeeMap_DefaultOperatorGetItemIndexWithError

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorTryGetItemStringHashWithEmpty(DeeObject *self, char const *key, Dee_hash_t hash);
#define DeeMap_DefaultOperatorTryGetItemStringHashWithError DeeMap_DefaultOperatorGetItemStringHashWithError

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemStringHashWithEmpty(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash);

#define DeeMap_DefaultOperatorDelItemStringHashWithEmpty DeeMap_DefaultOperatorDelItemStringHashWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorDelItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash);

#define DeeMap_DefaultOperatorSetItemStringHashWithEmpty DeeMap_DefaultOperatorSetItemStringHashWithError
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeMap_DefaultOperatorSetItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorBoundItemStringHashWithEmpty(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorBoundItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash);

#define DeeMap_DefaultOperatorHasItemStringHashWithEmpty (*(Dee_mh_map_operator_hasitem_string_hash_t)&_DeeNone_reti0_3)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorHasItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorTryGetItemStringLenHashWithEmpty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define DeeMap_DefaultOperatorTryGetItemStringLenHashWithError DeeMap_DefaultOperatorGetItemStringLenHashWithError

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemStringLenHashWithEmpty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGetItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

#define DeeMap_DefaultOperatorDelItemStringLenHashWithEmpty DeeMap_DefaultOperatorDelItemStringLenHashWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorDelItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

#define DeeMap_DefaultOperatorSetItemStringLenHashWithEmpty DeeMap_DefaultOperatorSetItemStringLenHashWithError
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeMap_DefaultOperatorSetItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorBoundItemStringLenHashWithEmpty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorBoundItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

#define DeeMap_DefaultOperatorHasItemStringLenHashWithEmpty (*(Dee_mh_map_operator_hasitem_string_len_hash_t)&_DeeNone_reti0_4)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorHasItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

#define DeeMap_DefaultOperatorHashWithEmpty DeeSeq_DefaultOperatorHashWithEmpty
#define DeeMap_DefaultOperatorHashWithError DeeSeq_DefaultOperatorHashWithError

#define DeeMap_DefaultOperatorCompareEqWithEmpty DeeSeq_DefaultOperatorCompareEqWithEmpty
INTDEF /*WUNUSED*/ NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorTryCompareEqWithEmpty DeeSeq_DefaultOperatorTryCompareEqWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorEqWithMapCompareEq(DeeObject *self, DeeObject *some_object);
#define DeeMap_DefaultOperatorEqWithEmpty DeeSeq_DefaultOperatorEqWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorNeWithMapCompareEq(DeeObject *self, DeeObject *some_object);
#define DeeMap_DefaultOperatorNeWithEmpty DeeSeq_DefaultOperatorNeWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorLoWithEmpty DeeSet_DefaultOperatorLoWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorLeWithEmpty DeeSet_DefaultOperatorLeWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorGrWithEmpty DeeSet_DefaultOperatorGrWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorGeWithEmpty DeeSet_DefaultOperatorGeWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorAddWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorAddWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorAddWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorSubWithEmpty DeeSet_DefaultOperatorSubWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorSubWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorSubWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorAndWithEmpty DeeMap_DefaultOperatorSubWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorAndWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorAndWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorXorWithEmpty DeeMap_DefaultOperatorAddWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorXorWithForeach(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultOperatorXorWithError(DeeObject *self, DeeObject *some_object);

#define DeeMap_DefaultOperatorInplaceAddWithEmpty DeeMap_DefaultOperatorInplaceAddWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceAddWithMapUpdate(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceAddWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);

#define DeeMap_DefaultOperatorInplaceSubWithEmpty DeeMap_DefaultOperatorInplaceSubWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceSubWithMapRemoveKeys(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceSubWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);

#define DeeMap_DefaultOperatorInplaceAndWithEmpty DeeMap_DefaultOperatorInplaceAndWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceAndWithForeachAndMapRemoveKeys(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceAndWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);

#define DeeMap_DefaultOperatorInplaceXorWithEmpty DeeMap_DefaultOperatorInplaceXorWithError
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceXorWithForeachAndMapUpdatAndMapRemoveKeys(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultOperatorInplaceXorWithError(DREF DeeObject **__restrict p_self, DeeObject *some_object);
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */







/* Possible implementations for sequence cache functions. */
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSeqGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithTryGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithTryGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSizeAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithForeach(DeeObject *__restrict self);
#define DeeSeq_DefaultTryGetFirstWithError DeeSeq_DefaultGetFirstWithError

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithCallAttrGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithCallAttrExplicitGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithCallGetFirstDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithCallAttrGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithCallAttrExplicitGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithCallGetFirstDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithCallAttrDelFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithCallAttrExplicitDelFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithCallDelFirstDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithSeqGetFirstAndSetRemove(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithSeqGetFirstAndMaplikeDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallAttrSetFirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallAttrExplicitSetFirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallSetFirstDataFunction(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSeqGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSizeAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithForeach(DeeObject *__restrict self);
#define DeeSeq_DefaultTryGetLastWithError DeeSeq_DefaultGetLastWithError

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithCallAttrGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithCallAttrExplicitGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithCallGetLastDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithCallAttrGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithCallAttrExplicitGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithCallGetLastDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeAndBoundItem(DeeObject *__restrict self);
#define DeeSeq_DefaultBoundLastWithForeach DeeSeq_DefaultBoundFirstWithForeach
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithCallAttrDelLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithCallAttrExplicitDelLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithCallDelLastDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSizeAndDelItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSizeAndDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSeqGetLastAndSetRemove(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSeqGetLastAndMaplikeDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithCallAttrSetLast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithCallAttrExplicitSetLast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithCallSetLastDataFunction(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSizeAndSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSizeAndSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithError(DeeObject *self, DeeObject *value);

/* Implementations for `Sequence.cached' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithCallAttrCached(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithCallAttrExplicitCached(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithCallCachedDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithSeqIter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithSeqGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithSeqSizeObAndSeqGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithSeqSizeAndSeqGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultCachedWithError(DeeObject *__restrict self);


/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAttrAny(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAttrExplicitAny(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrExplicitAnyForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrExplicitAnyForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAttrAny(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAttrExplicitAny(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrAny(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrExplicitAny(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAttrAll(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAttrExplicitAll(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrAllForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrExplicitAllForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrAllForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrExplicitAllForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAttrAll(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAttrExplicitAll(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrAll(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrExplicitAll(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallAttrParity(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallAttrExplicitParity(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithSeqCount(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrParityForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrExplicitParityForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrParityForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrExplicitParityForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallAttrParity(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallAttrExplicitParity(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithSeqCountWithRange(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrParity(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrExplicitParity(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallAttrReduce(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallAttrExplicitReduce(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataFunction(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataMethod(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithSeqForeach(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrExplicitReduceForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataMethodForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataKwMethodForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrExplicitReduceForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataMethodForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataKwMethodForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithSeqForeach(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallAttrExplicitReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataFunction(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrExplicitReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataFunction(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithSeqEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallAttrMin(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallAttrExplicitMin(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrMinForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrExplicitMinForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrMinForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrExplicitMinForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallAttrMin(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallAttrExplicitMin(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrMin(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrExplicitMin(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallAttrMax(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallAttrExplicitMax(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrExplicitMaxForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrExplicitMaxForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallAttrMax(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallAttrExplicitMax(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrMax(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrExplicitMax(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallAttrSum(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallAttrExplicitSum(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallAttrSum(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallAttrExplicitSum(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallAttrCount(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallAttrExplicitCount(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithSeqFind(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithSeqForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithSetContains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithMapTryGetItem(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrCountForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrExplicitCountForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrCountForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrExplicitCountForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallAttrExplicitCount(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithSeqFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrExplicitCount(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallAttrContains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallAttrExplicitContains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallContainsDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallContainsDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item);
/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithContains(DeeObject *self, DeeObject *item);*/
#define DeeSeq_DefaultContainsWithContains DeeObject_ContainsAsBool
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithSeqFind(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithMapTryGetItem(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallAttrExplicitContainsForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallAttrExplicitContainsForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallAttrExplicitContains(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithSeqFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrExplicitContains(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithSeqFindWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallAttrLocateForSeq(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallAttrExplicitLocateForSeq(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataFunctionForSeq(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataMethodForSeq(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataKwMethodForSeq(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallAttrLocateForSetOrMap(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallAttrExplicitLocateForSetOrMap(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataFunctionForSetOrMap(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataMethodForSetOrMap(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataKwMethodForSetOrMap(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithSeqForeach(DeeObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallAttrLocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallAttrExplicitLocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataFunction(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataMethod(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataKwMethod(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallAttrRLocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallAttrExplicitRLocate(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataFunction(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataMethod(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataKwMethod(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallAttrStartsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallAttrExplicitStartsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithSeqTryGetFirst(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrExplicitStartsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrExplicitStartsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithSeqTryGetFirst(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallAttrExplicitStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrExplicitStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallAttrEndsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallAttrExplicitEndsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithSeqTryGetLast(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrExplicitEndsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrExplicitEndsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithSeqTryGetLast(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallAttrExplicitEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithSeqSizeAndSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrExplicitEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithSeqSizeAndSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);



/* Mutable sequence functions */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallAttrExplicitFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallAttrExplicitFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallAttrExplicitRFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallAttrExplicitRFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallAttrErase(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallAttrExplicitErase(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataFunction(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataMethod(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataKwMethod(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithDelRangeIndex(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithPop(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithError(DeeObject *self, size_t index, size_t count);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallAttrInsert(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallAttrExplicitInsert(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallInsertDataFunction(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallInsertDataMethod(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallInsertDataKwMethod(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithSeqInsertAll(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithError(DeeObject *self, size_t index, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallAttrExplicitInsertAll(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataFunction(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataMethod(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataKwMethod(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithSetRangeIndex(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithSeqInsert(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithError(DeeObject *self, size_t index, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallAttrPushFront(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallAttrExplicitPushFront(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithSeqInsert(DeeObject *self, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrAppend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrExplicitAppend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrPushBack(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrExplicitPushBack(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithSeqExtend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithSizeAndSeqInsert(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithError(DeeObject *self, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallAttrExtend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallAttrExplicitExtend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithSeqAppend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithSizeAndSeqInsertAll(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithError(DeeObject *self, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallAttrXchItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallAttrExplicitXchItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemDataFunction(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemDataMethod(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemDataKwMethod(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithError(DeeObject *self, size_t index, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallAttrClear(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallAttrExplicitClear(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallClearDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallClearDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallClearDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithDelRangeIndexN(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithSetRangeIndexN(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithSeqErase(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithSeqRemoveAll(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithMapRemoveKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallAttrPop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallAttrExplicitPop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataFunction(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataMethod(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataKwMethod(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndSeqErase(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithError(DeeObject *self, Dee_ssize_t index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallAttrExplicitRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqFindAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallAttrExplicitRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqRemoveAllWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallAttrExplicitRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithTSeqFindAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithSeqEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallAttrExplicitRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithSeqRFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallAttrExplicitRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSeqRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallAttrExplicitRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallAttrExplicitRemoveIf(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataKwMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithError(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallAttrResize(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallAttrExplicitResize(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataFunction(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataMethod(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataKwMethod(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSetRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSeqEraseAndSeqExtend(DeeObject *self, size_t newsize, DeeObject *filler);

INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallAttrFill(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallAttrExplicitFill(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallFillDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallFillDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallFillDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithSizeAndSetRangeIndex(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithError(DeeObject *self, size_t start, size_t end, DeeObject *filler);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallAttrReverse(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallAttrExplicitReverse(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallReverseDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallReverseDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallReverseDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithSeqReversedAndSetRangeIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithError(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallAttrReversed(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallAttrExplicitReversed(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallReversedDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallReversedDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallReversedDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallAttrSort(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallAttrExplicitSort(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallSortDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallSortDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallSortDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithSeqSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithError(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallAttrSort(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallAttrExplicitSort(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithSeqSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithError(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallAttrSorted(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallAttrExplicitSorted(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallSortedDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallSortedDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallSortedDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallAttrSorted(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallAttrExplicitSorted(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallAttrBFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallAttrExplicitBFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallBFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallBFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallBFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithSeqBRange(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallAttrBFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallAttrExplicitBFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithSeqBRangeWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallAttrBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallAttrExplicitBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithSeqBRange(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallAttrExplicitBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithSeqBRangeWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallAttrBRange(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallAttrExplicitBRange(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallBRangeDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallBRangeDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallBRangeDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);

INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallAttrBRange(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallAttrExplicitBRange(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallBRangeDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallBRangeDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallBRangeDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);



/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallAttrInsert(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallAttrExplicitInsert(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithSeqSizeAndSetInsertAll(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithMapSetNew(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithSeqSeqContainsAndSeqAppend(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallAttrExplicitRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithSeqSizeAndSeqRemoveAll(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithMapTryGetItemAndMapDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithSeqRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallAttrUnify(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallAttrExplicitUnify(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithSetInsertAndSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithSeqAppendAndSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallAttrExplicitInsertAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithInplaceAdd(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithInplaceOr(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithSetInsert(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallAttrExplicitRemoveAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithInplaceSub(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithSetRemove(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallAttrPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallAttrExplicitPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithSetFirstAndSetRemove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithMapPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithSeqPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallAttrExplicitPop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataFunction(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataMethod(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataKwMethod(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithSeqTryGetFirstAndSetRemove(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithMapPopItem(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithSeqPop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *default_);



/************************************************************************/
/* For `deemon.Mapping'                                                 */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallAttrSetOld(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallAttrExplicitSetOld(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallSetOldDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallSetOldDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallSetOldDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithMapSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallAttrSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallAttrExplicitSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallSetOldExDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallSetOldExDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallSetOldExDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallAttrSetNew(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallAttrExplicitSetNew(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallSetNewDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallSetNewDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallSetNewDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithMapSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithBoundItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTryGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallAttrSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallAttrExplicitSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithTryGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallAttrSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallAttrExplicitSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithMapSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithMapSetNewAndGetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallAttrUpdate(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallAttrExplicitUpdate(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallUpdateDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallUpdateDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallUpdateDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithInplaceAdd(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithInplaceOr(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithSetItem(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithError(DeeObject *self, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallAttrExplicitRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithBoundItemAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithSizeAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithSizeAndMapRemoveKeys(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallAttrRemoveKeys(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallAttrExplicitRemoveKeys(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithDelItem(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithMapRemove(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallAttrPop(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallAttrExplicitPop(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithGetItemAndMapRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithGetItemAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallAttrExplicitPop(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataFunction(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataMethod(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithTryGetItemAndMapRemove(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithTryGetItemAndDelItem(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *default_);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallAttrPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallAttrExplicitPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithSeqTryGetFirstAndMapRemove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithSeqTryGetFirstAndDelItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallAttrKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallAttrExplicitKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallKeysDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithMapIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallAttrValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallAttrExplicitValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallValuesDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithMapIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallAttrIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallAttrExplicitIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallIterKeysDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithMapKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallAttrIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallAttrExplicitIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallIterValuesDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithMapValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithIter(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithError(DeeObject *self);




/* Extra default functions (for getsets) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_delfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default_seq_setfirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_boundlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_dellast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default_seq_setlast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL default_seq_cached)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL default_map_keys)(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL default_map_values)(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL default_map_iterkeys)(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL default_map_itervalues)(DeeObject *self);
#define default_seq_cached(self)     DeeSeq_InvokeCached(self)
#define default_map_keys(self)       DeeMap_InvokeKeys(self)
#define default_map_values(self)     DeeMap_InvokeValues(self)
#define default_map_iterkeys(self)   DeeMap_InvokeIterKeys(self)
#define default_map_itervalues(self) DeeMap_InvokeIterValues(self)

/* Default operators as type_method-s */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___bool__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___iter__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___size__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___contains__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___getitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___delitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___setitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___getrange__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___delrange__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___setrange__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___foreach__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___foreach_pair__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___enumerate__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___enumerate_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___iterkeys__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___bounditem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___hasitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___size_fast__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___getitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___delitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___setitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___bounditem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___hasitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___getrange_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___delrange_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___setrange_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___trygetitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___trygetitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___hash__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___compare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___compare__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___trycompare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___ne__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___lo__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___le__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___gr__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___ge__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___inplace_add__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq___inplace_mul__(DeeObject *self, size_t argc, DeeObject *const *argv);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___iter__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___size__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___foreach__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___hash__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___compare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___trycompare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___ne__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___lo__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___le__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___gr__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___ge__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___inv__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___add__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a"} + {"b"}         -> {"a","b"} */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___sub__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a","b"} - {"b"}     -> {"a"} */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___and__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a","b"} & {"a"}     -> {"a"} */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___xor__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a","b"} ^ {"a","c"} -> {"b","c"} */
#define default_set___or__ default_set___add__
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___inplace_add__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___inplace_sub__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___inplace_and__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set___inplace_xor__(DeeObject *self, size_t argc, DeeObject *const *argv);
#define default_set___inplace_or__ default_set___inplace_add__

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___contains__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___getitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___delitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___setitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___enumerate__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___enumerate_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___iterkeys__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___bounditem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___hasitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___getitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___delitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___setitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___bounditem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___hasitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___trygetitem__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___trygetitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___trygetitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___getitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv); /* for deemon.string, use GetItemStringHash; for deemon.Bytes, use GetItemStringLenHash */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___delitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___setitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___bounditem_string__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___hasitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv);
#define default_map___hash__ default_set___hash__
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___compare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___trycompare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___eq__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___ne__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___lo__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___le__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___gr__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___ge__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___add__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a"} + {"b"}         -> {"a","b"} */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___sub__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a","b"} - {"b"}     -> {"a"} */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___and__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a","b"} & {"a"}     -> {"a"} */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___xor__(DeeObject *self, size_t argc, DeeObject *const *argv); /* {"a","b"} ^ {"a","c"} -> {"b","c"} */
#define default_map___or__ default_map___add__
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___inplace_add__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___inplace_sub__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___inplace_and__(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map___inplace_xor__(DeeObject *self, size_t argc, DeeObject *const *argv);
#define default_map___inplace_or__ default_map___inplace_add__
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H */
