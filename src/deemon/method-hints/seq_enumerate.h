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

/************************************************************************/
/* deemon.Sequence.__seq_enumerate__()                                  */
/************************************************************************/
__seq_enumerate__(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	size_t start = 0;
	size_t end = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":__seq_enumerate__", &data.sed_cb, &start, &end))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		foreach_status = CALL_DEPENDENCY(seq_enumerate, self, &seq_enumerate_cb, &data);
	} else {
		foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_enumerate_index_cb, &data, start, end);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}





%[define(DEFINE_default_seq_enumerate_with_counter__and__seq_foreach_cb =
#ifndef DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb
#define DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb
struct default_seq_enumerate_with_counter__and__seq_foreach_data {
	Dee_seq_enumerate_t dewcaf_cb;      /* [1..1] Wrapped callback */
	void               *dewcaf_arg;     /* [?..?] Cookie for `dewcaf_cb' */
	size_t              dewcaf_counter; /* Index of the next element that will be enumerated */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_enumerate_with_counter__and__seq_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_seq_enumerate_with_counter__and__seq_foreach_data *data;
	data = (struct default_seq_enumerate_with_counter__and__seq_foreach_data *)arg;
	indexob = DeeInt_NewSize(data->dewcaf_counter);
	if unlikely(!indexob)
		goto err;
	++data->dewcaf_counter;
	result = (*data->dewcaf_cb)(data->dewcaf_arg, indexob, elem);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_enumerate_with_counter__and__seq_foreach_cb */
)]

%[define(DEFINE_default_enumerate_with_enumerate_index_cb =
#ifndef DEFINED_default_enumerate_with_enumerate_index_cb
#define DEFINED_default_enumerate_with_enumerate_index_cb
struct default_enumerate_with_enumerate_index_data {
	Dee_seq_enumerate_t dewei_cb;  /* [1..1] Wrapped callback. */
	void               *dewei_arg; /* [?..?] Cookie for `dewei_cb' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_with_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	DREF DeeObject *indexob;
	struct default_enumerate_with_enumerate_index_data *data;
	data = (struct default_enumerate_with_enumerate_index_data *)arg;
	indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = (*data->dewei_cb)(data->dewei_arg, indexob, value);
	Dee_Decref(indexob);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_enumerate_with_enumerate_index_cb */
)]





/* Enumerate valid keys/indices of "self", as well as their current value.
 * @return: * : Sum of return values of `*cb'
 * @return: -1: An error occurred during iteration (or potentially inside of `*cb') */
[[wunused]] Dee_ssize_t
__seq_enumerate__.seq_enumerate([[nonnull]] DeeObject *__restrict self,
                                [[nonnull]] Dee_seq_enumerate_t cb,
                                void *arg)
%{unsupported({ return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); })}
%{$empty = 0}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_ssize_t temp, result = 0;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *indexob, *index_value;
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = (*tp_getitem_index_fast)(self, i);
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_indexob;
			}
		}
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
		if unlikely(!index_value)
			goto err_indexob;
		if (index_value == ITER_DONE) {
			temp = (*cb)(arg, indexob, NULL);
		} else {
			temp = (*cb)(arg, indexob, index_value);
			Dee_Decref(index_value);
		}
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value, *sizeob;
	sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	for (;;) {
		int index_is_less_than_size = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (index_is_less_than_size <= 0) {
			if unlikely(index_is_less_than_size < 0)
				goto err_sizeob_indexob;
			break;
		}
		index_value = CALL_DEPENDENCY(seq_operator_getitem, self, indexob);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_sizeob_indexob;
			}
		}
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_sizeob_indexob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
		if unlikely(DeeObject_Inc(&indexob))
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_indexob:
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return temp;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}}
%{$with__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value;
	size_t i;
	for (i = 0;; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_indexob;
			}
		}
		temp = (*cb)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}}
%{$with__seq_operator_getitem = {
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	Dee_ssize_t temp, result = 0;
	DREF DeeIntObject *indexob;
	Dee_Incref(DeeInt_Zero);
	indexob = (DREF DeeIntObject *)DeeInt_Zero;
	for (;;) {
		DREF DeeObject *index_value;
		index_value = CALL_DEPENDENCY(seq_operator_getitem, self, (DeeObject *)indexob);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_indexob;
			}
		}
		temp = (*cb)(arg, (DeeObject *)indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_indexob;
		result += temp;
		if unlikely(DeeThread_CheckInterrupt())
			goto err_indexob;
		if unlikely(int_inc(&indexob))
			goto err_indexob;
	}
	Dee_Decref(indexob);
	return result;
err_temp_indexob:
	Dee_Decref(indexob);
	return temp;
err_indexob:
	Dee_Decref(indexob);
/*err:*/
	return -1;
}}
%{using seq_enumerate_index: [[prefix(DEFINE_default_enumerate_with_enumerate_index_cb)]] {
	struct default_enumerate_with_enumerate_index_data data;
	data.dewei_cb  = cb;
	data.dewei_arg = arg;
	return CALL_DEPENDENCY(seq_enumerate_index, self, &default_enumerate_with_enumerate_index_cb, &data, 0, (size_t)-1);
}}
%{$with__seq_operator_foreach__and__counter =
[[prefix(DEFINE_default_seq_enumerate_with_counter__and__seq_foreach_cb)]] {
	struct default_seq_enumerate_with_counter__and__seq_foreach_data data;
	data.dewcaf_cb      = cb;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_enumerate_with_counter__and__seq_foreach_cb, &data);
}} {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = LOCAL_CALLATTR(self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}





%[define(DEFINE_default_seq_enumerate_index_with_counter__and__seq_foreach_cb =
#ifndef DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb
#define DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb
#define default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */

struct default_seq_enumerate_index_with_counter__and__seq_foreach_data {
	Dee_seq_enumerate_index_t deiwcaf_cb;    /* [1..1] Wrapped callback */
	void                     *deiwcaf_arg;   /* [?..?] Cookie for `deiwcaf_cb' */
	size_t                    deiwcaf_index; /* Index of the next element that will be enumerate_indexd */
	size_t                    deiwcaf_start; /* Enumeration start index */
	size_t                    deiwcaf_end;   /* Enumeration end index */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_enumerate_index_with_counter__and__seq_foreach_cb(void *arg, DeeObject *elem) {
	size_t index;
	struct default_seq_enumerate_index_with_counter__and__seq_foreach_data *data;
	data = (struct default_seq_enumerate_index_with_counter__and__seq_foreach_data *)arg;
	if (data->deiwcaf_index >= data->deiwcaf_end)
		return default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP;
	index = data->deiwcaf_index++;
	if (index < data->deiwcaf_start)
		return 0; /* Skipped... */
	return (*data->deiwcaf_cb)(data->deiwcaf_arg, index, elem);
}
#endif /* !DEFINED_default_seq_enumerate_index_with_counter__and__seq_foreach_cb */
)]



%[define(DEFINE_default_enumerate_index_with_enumerate_cb =
#ifndef DEFINED_default_enumerate_index_with_enumerate_cb
#define DEFINED_default_enumerate_index_with_enumerate_cb
struct default_enumerate_index_with_enumerate_data {
	Dee_seq_enumerate_index_t deiwe_cb;    /* [1..1] Underlying callback. */
	void                     *deiwe_arg;   /* [?..?] Cookie for `deiwe_cb' */
	size_t                    deiwe_start; /* Enumeration start index */
	size_t                    deiwe_end;   /* Enumeration end index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_enumerate_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	size_t index;
	struct default_enumerate_index_with_enumerate_data *data;
	data = (struct default_enumerate_index_with_enumerate_data *)arg;
	if (DeeObject_AsSize(key, &index)) /* TODO: Handle overflow (by skipping this item) */
		goto err;
	if (index >= data->deiwe_start && index < data->deiwe_end)
		return (*data->deiwe_cb)(data->deiwe_arg, index, value);
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_enumerate_index_with_enumerate_cb */
)]



/* Same as `seq_enumerate()', but only valid when "self" uses integers for indices
 * or is a mapping where all keys are integers. In the former case, [start,end)
 * can be given in order to allow the implementation to only enumerate indices that
 * fall within that range (though an implementation is allowed to simply ignore these
 * arguments)
 * If you want to always enumerate all indices (like is also done by `seq_enumerate',
 * then simply pass `start = 0, end = (size_t)-1')
 * @return: * : Sum of return values of `*cb'
 * @return: -1: An error occurred during iteration (or potentially inside of `*cb') */
[[wunused]] Dee_ssize_t
__seq_enumerate__.seq_enumerate_index([[nonnull]] DeeObject *__restrict self,
                                      [[nonnull]] Dee_seq_enumerate_index_t cb,
                                      void *arg, size_t start, size_t end)
%{unsupported({ return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); })}
%{$empty = 0}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_ssize_t temp, result = 0;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = (*tp_getitem_index_fast)(self, i);
		temp = (*cb)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err;
			}
		}
		temp = (*cb)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
		if unlikely(!index_value)
			goto err;
		if (index_value == ITER_DONE) {
			temp = (*cb)(arg, i, NULL);
		} else {
			temp = (*cb)(arg, i, index_value);
			Dee_Decref(index_value);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!index_value) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err;
			}
		}
		temp = (*cb)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{using seq_enumerate: [[prefix(DEFINE_default_enumerate_index_with_enumerate_cb)]] {
	struct default_enumerate_index_with_enumerate_data data;
	data.deiwe_cb    = cb;
	data.deiwe_arg   = arg;
	data.deiwe_start = start;
	data.deiwe_end   = end;
	return CALL_DEPENDENCY(seq_enumerate, self, &default_enumerate_index_with_enumerate_cb, &data);
}}
%{$with__seq_operator_foreach__and__counter =
[[prefix(DEFINE_default_seq_enumerate_index_with_counter__and__seq_foreach_cb)]] {
	struct default_seq_enumerate_index_with_counter__and__seq_foreach_data data;
	Dee_ssize_t result;
	data.deiwcaf_cb    = cb;
	data.deiwcaf_arg   = arg;
	data.deiwcaf_index = 0;
	data.deiwcaf_start = start;
	data.deiwcaf_end   = end;
	result = CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_enumerate_index_with_counter__and__seq_foreach_cb, &data);
	if unlikely(result == default_seq_enumerate_index_with_counter__and__seq_foreach_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
}} {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}


seq_enumerate = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_seq_operator_size_t seq_operator_size;
	/*if (REQUIRE_NODEFAULT(seq_enumerate_index))
		return &$with__seq_enumerate_index;*/
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_getitem = REQUIRE(seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
			if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
				goto use_seq_operator_foreach;
			if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair)
				goto use_seq_operator_foreach;
			seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto use_seq_operator_foreach;
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		} else if (seq_operator_getitem) {
			return &$with__seq_operator_sizeob__and__seq_operator_getitem;
		}
	}
use_seq_operator_foreach:
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach) {
		if (seq_operator_foreach == &default__seq_operator_foreach__empty)
			return &$empty;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem)
			return &$with__seq_operator_sizeob__and__seq_operator_getitem;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem_index)
			return &$with__seq_operator_getitem_index;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem)
			return &$with__seq_operator_getitem;
		return &$with__seq_operator_foreach__and__counter;
	}
};

seq_enumerate_index = {
	DeeMH_seq_enumerate_t seq_enumerate = REQUIRE(seq_enumerate);
	if (seq_enumerate == &default__seq_enumerate__empty)
		return &$empty;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem)
		return &$with__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter)
		return &$with__seq_operator_foreach__and__counter;
	if (seq_enumerate)
		return &$with__seq_enumerate;
};
