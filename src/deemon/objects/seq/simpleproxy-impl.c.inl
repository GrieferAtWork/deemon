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
#ifdef __INTELLISENSE__
#define DEFINE_Sequence_ids
//#define DEFINE_Sequence_types
//#define DEFINE_Sequence_classes
#endif /* __INTELLISENSE__ */

#include "simpleproxy.c"

#if (defined(DEFINE_Sequence_ids) +   \
     defined(DEFINE_Sequence_types) + \
     defined(DEFINE_Sequence_classes)) != 1
#error "Must #define exactly one of these"
#endif

DECL_BEGIN

#ifdef DEFINE_Sequence_ids
#define LOCAL_simpleproxy_(x)    ids_##x
#define LOCAL_mapitem(item)      DeeInt_NewUIntptr(DeeObject_Id(item))
#define LOCAL_Proxy_Type         SeqIds_Type
#define LOCAL_ProxyIterator_Type SeqIdsIterator_Type
#elif defined(DEFINE_Sequence_types)
#define LOCAL_simpleproxy_(x)     types_##x
#define LOCAL_mapitem(item)       DeeObject_NewRef(Dee_AsObject(Dee_TYPE(item)))
#define LOCAL_mapitem_noref_nofail(item) Dee_AsObject(Dee_TYPE(item))
#define LOCAL_Proxy_Type          SeqTypes_Type
#define LOCAL_ProxyIterator_Type  SeqTypesIterator_Type
#elif defined(DEFINE_Sequence_classes)
#define LOCAL_simpleproxy_(x)     classes_##x
#define LOCAL_mapitem(item)       DeeObject_NewRef(Dee_AsObject(DeeObject_Class(item)))
#define LOCAL_mapitem_noref_nofail(item) Dee_AsObject(DeeObject_Class(item))
#define LOCAL_Proxy_Type          SeqClasses_Type
#define LOCAL_ProxyIterator_Type  SeqClassesIterator_Type
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#if !defined(LOCAL_mapitem_noref) && defined(LOCAL_mapitem_noref_nofail)
#define LOCAL_mapitem_noref(item) LOCAL_mapitem_noref_nofail(item)
#endif /* !LOCAL_mapitem_noref && LOCAL_mapitem_noref_nofail */

#define LOCAL_iter                        LOCAL_simpleproxy_(iter)
#define LOCAL_getitem                     LOCAL_simpleproxy_(getitem)
#define LOCAL_getitem_index               LOCAL_simpleproxy_(getitem_index)
#define LOCAL_trygetitem                  LOCAL_simpleproxy_(trygetitem)
#define LOCAL_trygetitem_index            LOCAL_simpleproxy_(trygetitem_index)
#define LOCAL_contains_foreach_cb         LOCAL_simpleproxy_(contains_foreach_cb)
#define LOCAL_contains                    LOCAL_simpleproxy_(contains)
#define LOCAL_foreach_data                LOCAL_simpleproxy_(foreach_data)
#define LOCAL_foreach_cb                  LOCAL_simpleproxy_(foreach_cb)
#define LOCAL_foreach                     LOCAL_simpleproxy_(foreach)
#define LOCAL_mh_seq_enumerate_data       LOCAL_simpleproxy_(mh_seq_enumerate_data)
#define LOCAL_mh_seq_enumerate_cb         LOCAL_simpleproxy_(mh_seq_enumerate_cb)
#define LOCAL_mh_seq_enumerate            LOCAL_simpleproxy_(mh_seq_enumerate)
#define LOCAL_mh_seq_enumerate_index_data LOCAL_simpleproxy_(mh_seq_enumerate_index_data)
#define LOCAL_mh_seq_enumerate_index_cb   LOCAL_simpleproxy_(mh_seq_enumerate_index_cb)
#define LOCAL_mh_seq_enumerate_index      LOCAL_simpleproxy_(mh_seq_enumerate_index)

PRIVATE WUNUSED NONNULL((1)) DREF SeqSimpleProxyIterator *DCALL
LOCAL_iter(SeqSimpleProxy *__restrict self) {
	DREF SeqSimpleProxyIterator *result;
	result = DeeObject_MALLOC(SeqSimpleProxyIterator);
	if unlikely(!result)
		goto err;
	result->si_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->sp_seq);
	if unlikely(!result->si_iter)
		goto err_r_maybe_nest;
	DeeObject_Init(result, &LOCAL_ProxyIterator_Type);
	return result;
err_r_maybe_nest:
	DeeRT_ErrNestSequenceError(self->sp_seq, self);
/*err_r:*/
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_getitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	elem = DeeObject_InvokeMethodHint(seq_operator_getitem, self->sp_seq, index);
	if unlikely(!elem)
		goto err_maybe_nest;
	result = LOCAL_mapitem(elem);
	Dee_Decref(elem);
	return result;
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	elem = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->sp_seq, index);
	if unlikely(!elem)
		goto err_maybe_nest;
	result = LOCAL_mapitem(elem);
	Dee_Decref(elem);
	return result;
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->sp_seq, index);
	if unlikely(!ITER_ISOK(elem))
		goto err_maybe_nest;
	result = LOCAL_mapitem(elem);
	Dee_Decref(elem);
	return result;
err_maybe_nest:
	if (elem == NULL)
		DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return elem;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->sp_seq, index);
	if unlikely(!ITER_ISOK(elem))
		goto err_maybe_nest;
	result = LOCAL_mapitem(elem);
	Dee_Decref(elem);
	return result;
err_maybe_nest:
	if (elem == NULL)
		DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return elem;
}


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
LOCAL_contains_foreach_cb(void *arg, DeeObject *elem) {
#ifdef DEFINE_Sequence_ids
	return DeeObject_Id(elem) == (uintptr_t)arg ? -2 : 0;
#elif defined(DEFINE_Sequence_types)
	return Dee_TYPE(elem) == (DeeTypeObject *)arg ? -2 : 0;
#elif defined(DEFINE_Sequence_classes)
	return DeeObject_Class(elem) == (DeeTypeObject *)arg ? -2 : 0;
#endif /* ... */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_contains(SeqSimpleProxy *self, DeeObject *value) {
	Dee_ssize_t status;
#ifdef DEFINE_Sequence_ids
	uintptr_t id;
	if (DeeObject_AsUIntptr(value, &id))
		goto err;
	status = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq,
	                                    &LOCAL_contains_foreach_cb, (void *)id);
#elif defined(DEFINE_Sequence_types)
	if unlikely(!DeeType_Check(value))
		return_false;
	status = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq,
	                                    &LOCAL_contains_foreach_cb, value);
#elif defined(DEFINE_Sequence_classes)
	if unlikely(!DeeType_Check(value))
		return_false;
	status = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq,
	                                    &LOCAL_contains_foreach_cb, value);
#endif /* ... */
	if unlikely(status == -1)
		goto err_maybe_nest;
	Dee_ASSERT(status == 0 || status == -2);
	return_bool(status != 0);
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->sp_seq, self);
#ifdef DEFINE_Sequence_ids
err:
#endif /* DEFINE_Sequence_ids */
	return NULL;
}


struct LOCAL_foreach_data {
	Dee_foreach_t pfd_proc; /* [1..1] Underlying callback. */
	void         *pfd_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct LOCAL_foreach_data *data;
	data = (struct LOCAL_foreach_data *)arg;
#ifdef LOCAL_mapitem_noref_nofail
	elem   = LOCAL_mapitem_noref_nofail(elem);
	result = (*data->pfd_proc)(data->pfd_arg, elem);
#elif defined(LOCAL_mapitem_noref)
	elem = LOCAL_mapitem_noref(elem);
	if unlikely(!elem)
		goto err;
	result = (*data->pfd_proc)(data->pfd_arg, elem);
#else /* LOCAL_mapitem_noref */
	elem = LOCAL_mapitem(elem);
	if unlikely(!elem)
		goto err;
	result = (*data->pfd_proc)(data->pfd_arg, elem);
	Dee_Decref(elem);
#endif /* !LOCAL_mapitem_noref */
	return result;
#ifndef LOCAL_mapitem_noref_nofail
err:
	return -1;
#endif /* !LOCAL_mapitem_noref_nofail */
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_foreach(SeqSimpleProxy *self, Dee_foreach_t proc, void *arg) {
	struct LOCAL_foreach_data data;
	Dee_ssize_t result;
	data.pfd_proc = proc;
	data.pfd_arg  = arg;
	result = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq,
	                                    &LOCAL_foreach_cb, &data);
	if unlikely(result == -1)
		DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return result;
}



struct LOCAL_mh_seq_enumerate_data {
	Dee_seq_enumerate_t ped_proc; /* [1..1] Underlying callback. */
	void               *ped_arg;  /* [?..?] Cookie for `ped_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_mh_seq_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	Dee_ssize_t result;
	struct LOCAL_mh_seq_enumerate_data *data;
	data = (struct LOCAL_mh_seq_enumerate_data *)arg;
	if (value) {
#ifdef LOCAL_mapitem_noref_nofail
		value  = LOCAL_mapitem_noref_nofail(value);
		result = (*data->ped_proc)(data->ped_arg, key, value);
#elif defined(LOCAL_mapitem_noref)
		value = LOCAL_mapitem_noref(value);
		if unlikely(!value)
			goto err;
		result = (*data->ped_proc)(data->ped_arg, key, value);
#else /* LOCAL_mapitem_noref */
		value = LOCAL_mapitem(value);
		if unlikely(!value)
			goto err;
		result = (*data->ped_proc)(data->ped_arg, key, value);
		Dee_Decref(value);
#endif /* !LOCAL_mapitem_noref */
	} else {
		result = (*data->ped_proc)(data->ped_arg, key, NULL);
	}
	return result;
#ifndef LOCAL_mapitem_noref_nofail
err:
	return -1;
#endif /* !LOCAL_mapitem_noref_nofail */
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_mh_seq_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	Dee_ssize_t result;
	struct LOCAL_mh_seq_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	result = DeeObject_InvokeMethodHint(seq_enumerate, self->sp_seq, &LOCAL_mh_seq_enumerate_cb, &data);
	if unlikely(result == -1)
		DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return result;
}


struct LOCAL_mh_seq_enumerate_index_data {
	Dee_seq_enumerate_index_t peid_proc; /* [1..1] Underlying callback. */
	void                     *peid_arg;  /* [?..?] Cookie for `peid_proc' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
LOCAL_mh_seq_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	Dee_ssize_t result;
	struct LOCAL_mh_seq_enumerate_index_data *data;
	data = (struct LOCAL_mh_seq_enumerate_index_data *)arg;
	if (value) {
#ifdef LOCAL_mapitem_noref_nofail
		value  = LOCAL_mapitem_noref_nofail(value);
		result = (*data->peid_proc)(data->peid_arg, key, value);
#elif defined(LOCAL_mapitem_noref)
		value = LOCAL_mapitem_noref(value);
		if unlikely(!value)
			goto err;
		result = (*data->peid_proc)(data->peid_arg, key, value);
#else /* LOCAL_mapitem_noref */
		value = LOCAL_mapitem(value);
		if unlikely(!value)
			goto err;
		result = (*data->peid_proc)(data->peid_arg, key, value);
		Dee_Decref(value);
#endif /* !LOCAL_mapitem_noref */
	} else {
		result = (*data->peid_proc)(data->peid_arg, key, NULL);
	}
	return result;
#ifndef LOCAL_mapitem_noref_nofail
err:
	return -1;
#endif /* !LOCAL_mapitem_noref_nofail */
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_mh_seq_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                             void *arg, size_t start, size_t end) {
	Dee_ssize_t result;
	struct LOCAL_mh_seq_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	result = DeeObject_InvokeMethodHint(seq_enumerate_index, self->sp_seq,
	                                    &LOCAL_mh_seq_enumerate_index_cb,
	                                    &data, start, end);
	if unlikely(result == -1)
		DeeRT_ErrNestSequenceError(self->sp_seq, self);
	return result;
}


#undef LOCAL_iter
#undef LOCAL_getitem
#undef LOCAL_getitem_index
#undef LOCAL_trygetitem
#undef LOCAL_trygetitem_index
#undef LOCAL_contains_foreach_cb
#undef LOCAL_contains
#undef LOCAL_foreach_data
#undef LOCAL_foreach_cb
#undef LOCAL_foreach
#undef LOCAL_mh_seq_enumerate_data
#undef LOCAL_mh_seq_enumerate_cb
#undef LOCAL_mh_seq_enumerate
#undef LOCAL_mh_seq_enumerate_index_data
#undef LOCAL_mh_seq_enumerate_index_cb
#undef LOCAL_mh_seq_enumerate_index

#undef LOCAL_simpleproxy_
#undef LOCAL_mapitem
#undef LOCAL_mapitem_noref
#undef LOCAL_mapitem_noref_nofail
#undef LOCAL_Proxy_Type
#undef LOCAL_ProxyIterator_Type

DECL_END

#undef DEFINE_Sequence_classes
#undef DEFINE_Sequence_types
#undef DEFINE_Sequence_ids
