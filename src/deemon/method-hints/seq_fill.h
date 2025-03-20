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
/* deemon.Sequence.fill()                                               */
/************************************************************************/
[[kw, alias(Sequence.fill)]]
__seq_fill__(size_t start = 0, size_t end = (size_t)-1, filler=!N) {
	if unlikely(CALL_DEPENDENCY(seq_fill, self, start, end, filler))
		goto err;
	return_none;
err:
	return NULL;
}



%[define(DEFINE_default_fill_with_enumerate_index_and_setitem_index_cb =
#ifndef DEFINED_default_fill_with_enumerate_index_and_setitem_index_cb
#define DEFINED_default_fill_with_enumerate_index_and_setitem_index_cb
struct default_fill_with_enumerate_index_and_setitem_index_data {
	DeeObject *dfweiasiid_seq;    /* [1..1] Sequence whose items to set. */
	DeeObject *dfweiasiid_filler; /* [1..1] Value to assign to indices. */
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *dfweiasiid_setitem_index)(DeeObject *self, size_t index, DeeObject *value);
};

PRIVATE WUNUSED Dee_ssize_t DCALL
default_fill_with_enumerate_index_and_setitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	struct default_fill_with_enumerate_index_and_setitem_index_data *data;
	(void)value;
	data = (struct default_fill_with_enumerate_index_and_setitem_index_data *)arg;
	return (*data->dfweiasiid_setitem_index)(data->dfweiasiid_seq, index, data->dfweiasiid_filler);
}
#endif /* !DEFINED_default_fill_with_enumerate_index_and_setitem_index_cb */
)]


[[wunused]] int
__seq_fill__.seq_fill([[nonnull]] DeeObject *self,
                      size_t start, size_t end,
                      [[nonnull]] DeeObject *filler)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_operator_size__and__seq_operator_setrange_index = {
	int result;
	DREF DeeObject *repeat;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	repeat = DeeSeq_RepeatItem(filler, end - start);
	if unlikely(!repeat)
		goto err;
	result = CALL_DEPENDENCY(seq_operator_setrange_index, self, (Dee_ssize_t)start, (Dee_ssize_t)end, repeat);
	Dee_Decref(repeat);
	return result;
err:
	return -1;
}}
%{$with__seq_enumerate_index__and__seq_operator_setitem_index =
[[prefix(DEFINE_default_fill_with_enumerate_index_and_setitem_index_cb)]] {
	struct default_fill_with_enumerate_index_and_setitem_index_data data;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	data.dfweiasiid_seq    = self;
	data.dfweiasiid_filler = filler;
	data.dfweiasiid_setitem_index = seq->tp_setitem_index;
	return (int)CALL_DEPENDENCY(seq_enumerate_index, self,
	                            &default_fill_with_enumerate_index_and_setitem_index_cb,
	                            &data, start, end);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}


seq_fill = {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index = REQUIRE(seq_operator_setrange_index);
	if (seq_operator_setrange_index != NULL &&
	    seq_operator_setrange_index != &default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall) {
		DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
		if (seq_operator_size != &default__seq_operator_size__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &$empty;
			if (seq_operator_size == &default__seq_operator_size__with__seq_operator_iter ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair ||
			    seq_operator_size == &default__seq_operator_size__with__map_enumerate) {
				if (REQUIRE_NODEFAULT(seq_operator_setitem_index))
					return &$with__seq_enumerate_index__and__seq_operator_setitem_index;
			}
			return &$with__seq_operator_size__and__seq_operator_setrange_index;
		}
	}

	seq_operator_setitem_index = REQUIRE(seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_enumerate_index_t seq_enumerate_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &$empty;
		seq_enumerate_index = REQUIRE_ANY(seq_enumerate_index);
		if (seq_enumerate_index != &default__seq_enumerate_index__unsupported) {
			if (seq_enumerate_index == &default__seq_enumerate_index__empty)
				return &$empty;
			return &$with__seq_enumerate_index__and__seq_operator_setitem_index;
		}
	}
};
