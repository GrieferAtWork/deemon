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


[[kw, alias(Sequence.bfind)]]
__seq_bfind__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?X2?Dint?N {
	size_t result = !DeeNone_Check(key)
	                ? CALL_DEPENDENCY(seq_bfind_with_key, self, item, start, end, key)
	                : CALL_DEPENDENCY(seq_bfind, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


[[kw, alias(Sequence.bposition)]]
__seq_bposition__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dint {
	size_t result = !DeeNone_Check(key)
	                ? CALL_DEPENDENCY(seq_bposition_with_key, self, item, start, end, key)
	                : CALL_DEPENDENCY(seq_bposition, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

[[kw, alias(Sequence.brange)]]
__seq_brange__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?T2?Dint?Dint {
	size_t result_range[2];
	if (!DeeNone_Check(key)
	    ? CALL_DEPENDENCY(seq_brange_with_key, self, item, start, end, key, result_range)
	    : CALL_DEPENDENCY(seq_brange, self, item, start, end, result_range))
		goto err;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, result_range[0], result_range[1]);
err:
	return NULL;
}

%[define(DEFINE_overflowsafe_mid =
#ifndef DEFINED_overflowsafe_mid
#define DEFINED_overflowsafe_mid
LOCAL ATTR_CONST size_t overflowsafe_mid(size_t a, size_t b) {
	size_t result;
	if unlikely(OVERFLOW_UADD(a, b, &result)) {
		size_t a_div2 = a >> 1;
		size_t b_div2 = b >> 1;
		result = (a_div2 + b_div2);
		if ((a & 1) && (b & 1))
			++result;
		return result;
	}
	return result >> 1;
}
#endif /* !DEFINED_overflowsafe_mid */
)]


/*[[[deemon
import * from deemon;

function printBody(name: string,
                   isFind: bool = false,
                   isPosition: bool = false,
                   isRange: bool = false,
                   hasKey: bool = false) {
	local LOCAL_return_ERR = isRange ? '-1' : '(size_t)Dee_COMPARE_ERR';
	local NEED_err_item_overflow = false;

	print('%{unsupported({');
if (isFind) {
	print('	err_seq_unsupportedf(self, "__seq_bfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);');
	print('	return (size_t)Dee_COMPARE_ERR;');
} else if (isPosition) {
	print('	err_seq_unsupportedf(self, "__seq_bposition__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);');
	print('	return (size_t)Dee_COMPARE_ERR;');
} else if (isRange) {
	print('	return err_seq_unsupportedf(self, "__seq_brange__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);');
}
	print('})}');
	print('%{$empty = {');
if (isFind) {
	print('	return (size_t)-1;');
} else if (isPosition) {
	print('	return 0;');
} else if (isRange) {
	print('	result_range[0] = 0;');
	print('	result_range[1] = 0;');
	print('	return 0;');
}
	print('}}');
	print('%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {');
	print('	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)');
	print('	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);');
	print('	if unlikely(selfsize == (size_t)-1)');
	print('		goto err;');
	print('	if (end > selfsize)');
	print('		end = selfsize;');
	print('	if likely(start < end) {');
	local err_item = "err";
if (hasKey) {
	err_item = "err_item";
	print('		item = DeeObject_Call(key, 1, &item);');
	print('		if unlikely(!item)');
	print('			goto err;');
}
	print('		do {');
	print('			int cmp_result;');
	print('			size_t mid = overflowsafe_mid(start, end);');
	print('			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);');
	print('			if unlikely(!ITER_ISOK(seq_item)) {');
	print('				if unlikely(!seq_item)');
	print('					goto ', err_item, ';');
	print('				cmp_result = 1; /' '* item > <unbound> *' '/');
	print('			} else {');
if (hasKey) {
	print('				cmp_result = DeeObject_CompareKey(item, seq_item, key);');
} else {
	print('				cmp_result = DeeObject_Compare(item, seq_item);');
}
	print('				Dee_Decref(seq_item);');
	print('				if unlikely(cmp_result == Dee_COMPARE_ERR)');
	print('					goto ', err_item, ';');
	print('			}');
	print('			if (cmp_result < 0) {');
	print('				end = mid;');
	print('			} else if (cmp_result > 0) {');
	print('				start = mid + 1;');
	print('			} else {');
	print('				/' '* Found it! (at "mid") *' '/');
if (isFind || isPosition) {
	print('				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end\', it can\'t be SIZE_MAX!");');
	print('				if unlikely(mid == (size_t)Dee_COMPARE_ERR) {');
	print('					end = mid;');
	print('					goto err_item_overflow;');
	print('				}');
	NEED_err_item_overflow = true;
if (hasKey)
	print('				Dee_Decref(item);');
	print('				return mid;');
} else if (isRange) {
	print('				size_t result_range_start = mid;');
	print('				size_t result_range_end   = mid + 1;');
	print;
	print('				/' '* Widen the result range\'s lower bound *' '/');
	print('				while (result_range_start > start) {');
	print('					mid = overflowsafe_mid(start, result_range_start);');
	print('					seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);');
	print('					if unlikely(!ITER_ISOK(seq_item)) {');
	print('						if unlikely(!seq_item)');
	print('							goto ', err_item, ';');
	print('						cmp_result = 1; /' '* item > <unbound> *' '/');
	print('					} else {');
if (hasKey) {
	print('						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);');
} else {
	print('						cmp_result = DeeObject_TryCompareEq(item, seq_item);');
}
	print('						Dee_Decref(seq_item);');
	print('						if unlikely(cmp_result == Dee_COMPARE_ERR)');
	print('							goto ', err_item, ';');
	print('					}');
	print('					if (cmp_result == 0) {');
	print('						/' '* Still part of returned range! *' '/');
	print('						result_range_start = mid;');
	print('					} else {');
	print('						/' '* No longer part of returned range! *' '/');
	print('						start = mid + 1;');
	print('					}');
	print('					/' '* Since this runs in O(log(N)), there\'s no need to check for interrupts! *' '/');
	print('				}');
	print;
	print('				/' '* Widen the result range\'s upper bound *' '/');
	print('				while (result_range_end < end) {');
	print('					mid = overflowsafe_mid(result_range_end, end);');
	print('					seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);');
	print('					if unlikely(!ITER_ISOK(seq_item)) {');
	print('						if unlikely(!seq_item)');
	print('							goto ', err_item, ';');
	print('						cmp_result = 1; /' '* item > <unbound> *' '/');
	print('					} else {');
if (hasKey) {
	print('						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);');
} else {
	print('						cmp_result = DeeObject_TryCompareEq(item, seq_item);');
}
	print('						Dee_Decref(seq_item);');
	print('						if unlikely(cmp_result == Dee_COMPARE_ERR)');
	print('							goto ', err_item, ';');
	print('					}');
	print('					if (cmp_result == 0) {');
	print('						/' '* Still part of returned range! *' '/');
	print('						result_range_end = mid + 1;');
	print('					} else {');
	print('						/' '* No longer part of returned range! *' '/');
	print('						end = mid;');
	print('					}');
	print('					/' '* Since this runs in O(log(N)), there\'s no need to check for interrupts! *' '/');
	print('				}');
	print('');
	print('				/' '* Write-back the result range bounds *' '/');
	print('				result_range[0] = result_range_start;');
	print('				result_range[1] = result_range_end;');
if (hasKey)
	print('				Dee_Decref(item);');
	print('				return 0;');
} // isRange
	print('			}');
	print('			/' '* Since this runs in O(log(N)), there\'s no need to check for interrupts! *' '/');
	print('		} while (start < end);');
	print('	}');
	print('	ASSERT(start >= end);');
if (hasKey)
	print('	Dee_Decref(item);');
if (isFind) {
	print('	return (size_t)-1;');
} else if (isPosition) {
	print('	if unlikely(end == (size_t)-1 || end == (size_t)Dee_COMPARE_ERR)');
	print('		goto err_item_overflow;');
	NEED_err_item_overflow = true;
	print('	return end;');
} else if (isRange) {
	print('	result_range[0] = start;');
	print('	result_range[1] = end;');
	print('	return 0;');
}
if (NEED_err_item_overflow) {
	print('err_item_overflow:');
	print('	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR - 1);');
}
if (hasKey) {
	print('err_item:');
	print('	Dee_Decref(item);');
}
	print('err:');
	print('	return ', LOCAL_return_ERR, ';');
	print('}} {');
	if (isRange) {
		print('	DREF DeeObject *result_start_and_end[2];');
	} else {
		print('	size_t result;');
	}
	print('	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ',
		hasKey ? ' "o"' : '', ', item, start, end', hasKey ? ', key' : '', ');');
	print('	if unlikely(!resultob)');
	print('		goto err;');
	if (isRange) {
		print('	if (DeeSeq_Unpack(resultob, 2, result_start_and_end))');
		print('		goto err_r;');
		print('	Dee_Decref(resultob);');
		print('	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))');
		print('		goto err_result_start_and_end;');
		print('	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))');
		print('		goto err_result_start_and_end;');
		print('	Dee_Decref(result_start_and_end[1]);');
		print('	Dee_Decref(result_start_and_end[0]);');
		print('	return 0;');
		print('err_result_start_and_end:');
		print('	Dee_Decref(result_start_and_end[1]);');
		print('	Dee_Decref(result_start_and_end[0]);');
		print('	goto err;');
	} else {
		print('	if (DeeNone_Check(resultob)) {');
		print('		Dee_DecrefNokill(resultob);');
		print('		return (size_t)-1;');
		print('	}');
		print('	if (DeeObject_AsSize(resultob, &result))');
		print('		goto err_r;');
		print('	Dee_Decref(resultob);');
		print('	return result;');
	}
	print('err_r:');
	print('	Dee_Decref(resultob);');
	print('err:');
	print('	return ', LOCAL_return_ERR, ';');
	print('}');
	print;
	print;
	print(name, ' = {');
	print('	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);');
	print('	if (seq_operator_size != &default__seq_operator_size__unsupported) {');
	print('		if (seq_operator_size == &default__seq_operator_size__empty)');
	print('			return &$empty;');
	print('		if (REQUIRE(seq_operator_trygetitem_index))');
	print('			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;');
	print('	}');
	print('};');
	print;
	print;
}

print("[[wunused]] size_t");
print("__seq_bfind__.seq_bfind([[nonnull]] DeeObject *self,");
print("                        [[nonnull]] DeeObject *item,");
print("                        size_t start, size_t end)");
printBody("seq_bfind", isFind: true);

print("[[wunused]] size_t");
print("__seq_bfind__.seq_bfind_with_key([[nonnull]] DeeObject *self,");
print("                                 [[nonnull]] DeeObject *item,");
print("                                 size_t start, size_t end,");
print("                                 [[nonnull]] DeeObject *key)");
printBody("seq_bfind_with_key", isFind: true, hasKey: true);



print("[[wunused]] size_t");
print("__seq_bposition__.seq_bposition([[nonnull]] DeeObject *self,");
print("                                [[nonnull]] DeeObject *item,");
print("                                size_t start, size_t end)");
printBody("seq_bposition", isPosition: true);

print("[[wunused]] size_t");
print("__seq_bposition__.seq_bposition_with_key([[nonnull]] DeeObject *self,");
print("                                         [[nonnull]] DeeObject *item,");
print("                                         size_t start, size_t end,");
print("                                         [[nonnull]] DeeObject *key)");
printBody("seq_bposition_with_key", isPosition: true, hasKey: true);



print("[[wunused]] int");
print("__seq_brange__.seq_brange([[nonnull]] DeeObject *self,");
print("                          [[nonnull]] DeeObject *item,");
print("                          size_t start, size_t end,");
print("                          [[nonnull]] size_t result_range[2])");
printBody("seq_brange", isRange: true);

print("[[wunused]] int");
print("__seq_brange__.seq_brange_with_key([[nonnull]] DeeObject *self,");
print("                                   [[nonnull]] DeeObject *item,");
print("                                   size_t start, size_t end,");
print("                                   [[nonnull]] DeeObject *key,");
print("                                   [[nonnull]] size_t result_range[2])");
printBody("seq_brange_with_key", isRange: true, hasKey: true);
]]]*/
[[wunused]] size_t
__seq_bfind__.seq_bfind([[nonnull]] DeeObject *self,
                        [[nonnull]] DeeObject *item,
                        size_t start, size_t end)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_bfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = {
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_Compare(item, seq_item);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR) {
					end = mid;
					goto err_item_overflow;
				}
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	return (size_t)-1;
err_item_overflow:
	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR - 1);
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	size_t result;
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}


seq_bfind = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_trygetitem_index))
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
};


[[wunused]] size_t
__seq_bfind__.seq_bfind_with_key([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *item,
                                 size_t start, size_t end,
                                 [[nonnull]] DeeObject *key)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_bfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = {
	return (size_t)-1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR) {
					end = mid;
					goto err_item_overflow;
				}
				Dee_Decref(item);
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	Dee_Decref(item);
	return (size_t)-1;
err_item_overflow:
	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR - 1);
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	size_t result;
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}


seq_bfind_with_key = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_trygetitem_index))
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
};


[[wunused]] size_t
__seq_bposition__.seq_bposition([[nonnull]] DeeObject *self,
                                [[nonnull]] DeeObject *item,
                                size_t start, size_t end)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_bposition__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = {
	return 0;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_Compare(item, seq_item);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR) {
					end = mid;
					goto err_item_overflow;
				}
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	if unlikely(end == (size_t)-1 || end == (size_t)Dee_COMPARE_ERR)
		goto err_item_overflow;
	return end;
err_item_overflow:
	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR - 1);
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	size_t result;
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}


seq_bposition = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_trygetitem_index))
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
};


[[wunused]] size_t
__seq_bposition__.seq_bposition_with_key([[nonnull]] DeeObject *self,
                                         [[nonnull]] DeeObject *item,
                                         size_t start, size_t end,
                                         [[nonnull]] DeeObject *key)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_bposition__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = {
	return 0;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR) {
					end = mid;
					goto err_item_overflow;
				}
				Dee_Decref(item);
				return mid;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	Dee_Decref(item);
	if unlikely(end == (size_t)-1 || end == (size_t)Dee_COMPARE_ERR)
		goto err_item_overflow;
	return end;
err_item_overflow:
	DeeRT_ErrIntegerOverflowU(end, (size_t)Dee_COMPARE_ERR - 1);
err_item:
	Dee_Decref(item);
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	size_t result;
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeNone_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return (size_t)-1;
	}
	if (DeeObject_AsSize(resultob, &result))
		goto err_r;
	Dee_Decref(resultob);
	return result;
err_r:
	Dee_Decref(resultob);
err:
	return (size_t)Dee_COMPARE_ERR;
}


seq_bposition_with_key = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_trygetitem_index))
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
};


[[wunused]] int
__seq_brange__.seq_brange([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *item,
                          size_t start, size_t end,
                          [[nonnull]] size_t result_range[2])
%{unsupported({
	return err_seq_unsupportedf(self, "__seq_brange__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
})}
%{$empty = {
	result_range[0] = 0;
	result_range[1] = 0;
	return 0;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_Compare(item, seq_item);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				size_t result_range_start = mid;
				size_t result_range_end   = mid + 1;

				/* Widen the result range's lower bound */
				while (result_range_start > start) {
					mid = overflowsafe_mid(start, result_range_start);
					seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareEq(item, seq_item);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_start = mid;
					} else {
						/* No longer part of returned range! */
						start = mid + 1;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Widen the result range's upper bound */
				while (result_range_end < end) {
					mid = overflowsafe_mid(result_range_end, end);
					seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareEq(item, seq_item);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_end = mid + 1;
					} else {
						/* No longer part of returned range! */
						end = mid;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Write-back the result range bounds */
				result_range[0] = result_range_start;
				result_range[1] = result_range_end;
				return 0;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	result_range[0] = start;
	result_range[1] = end;
	return 0;
err:
	return -1;
}} {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!resultob)
		goto err;
	if (DeeSeq_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}


seq_brange = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_trygetitem_index))
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
};


[[wunused]] int
__seq_brange__.seq_brange_with_key([[nonnull]] DeeObject *self,
                                   [[nonnull]] DeeObject *item,
                                   size_t start, size_t end,
                                   [[nonnull]] DeeObject *key,
                                   [[nonnull]] size_t result_range[2])
%{unsupported({
	return err_seq_unsupportedf(self, "__seq_brange__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
})}
%{$empty = {
	result_range[0] = 0;
	result_range[1] = 0;
	return 0;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = [[prefix(DEFINE_overflowsafe_mid)]] {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */
				size_t result_range_start = mid;
				size_t result_range_end   = mid + 1;

				/* Widen the result range's lower bound */
				while (result_range_start > start) {
					mid = overflowsafe_mid(start, result_range_start);
					seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err_item;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err_item;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_start = mid;
					} else {
						/* No longer part of returned range! */
						start = mid + 1;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Widen the result range's upper bound */
				while (result_range_end < end) {
					mid = overflowsafe_mid(result_range_end, end);
					seq_item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err_item;
						cmp_result = 1; /* item > <unbound> */
					} else {
						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err_item;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_end = mid + 1;
					} else {
						/* No longer part of returned range! */
						end = mid;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Write-back the result range bounds */
				result_range[0] = result_range_start;
				result_range[1] = result_range_end;
				Dee_Decref(item);
				return 0;
			}
			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);
	}
	ASSERT(start >= end);
	Dee_Decref(item);
	result_range[0] = start;
	result_range[1] = end;
	return 0;
err_item:
	Dee_Decref(item);
err:
	return -1;
}} {
	DREF DeeObject *result_start_and_end[2];
	DREF DeeObject *resultob = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!resultob)
		goto err;
	if (DeeSeq_Unpack(resultob, 2, result_start_and_end))
		goto err_r;
	Dee_Decref(resultob);
	if (DeeObject_AsSize(result_start_and_end[0], &result_range[0]))
		goto err_result_start_and_end;
	if (DeeObject_AsSize(result_start_and_end[1], &result_range[1]))
		goto err_result_start_and_end;
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	return 0;
err_result_start_and_end:
	Dee_Decref(result_start_and_end[1]);
	Dee_Decref(result_start_and_end[0]);
	goto err;
err_r:
	Dee_Decref(resultob);
err:
	return -1;
}


seq_brange_with_key = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_trygetitem_index))
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
};
/*[[[end]]]*/


