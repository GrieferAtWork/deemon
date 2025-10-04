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
#ifdef __INTELLISENSE__
#include "dict.c"
#define DEFINE_dict_first
//#define DEFINE_dict_firstkey
//#define DEFINE_dict_firstvalue
//#define DEFINE_dict_last
//#define DEFINE_dict_lastkey
//#define DEFINE_dict_lastvalue
#endif /* __INTELLISENSE__ */

#include <deemon/seq.h>

#if (defined(DEFINE_dict_first) +      \
     defined(DEFINE_dict_firstkey) +   \
     defined(DEFINE_dict_firstvalue) + \
     defined(DEFINE_dict_last) +       \
     defined(DEFINE_dict_lastkey) +    \
     defined(DEFINE_dict_lastvalue)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_dict_first
#define LOCAL_dict_trygetspecial            dict_trygetfirst
#define LOCAL_dict_getspecial               dict_getfirst
#define LOCAL_dict_delspecial               dict_delfirst
#define LOCAL_dict_setspecial_pair          dict_setfirst_pair
#define LOCAL_dict_setspecial_pair_getindex dict_setfirst_pair_getindex
#define LOCAL_dict_setspecial               dict_setfirst
#define LOCAL_IS_FIRST
#define LOCAL_IS_PAIR
#elif defined(DEFINE_dict_firstkey)
#define LOCAL_dict_getspecial              dict_getfirstkey
#define LOCAL_dict_setspecial              dict_setfirstkey
#define LOCAL_dict_setspecial_key_getindex dict_setfirstkey_getindex
#define LOCAL_IS_FIRST
#define LOCAL_IS_KEY
#elif defined(DEFINE_dict_firstvalue)
#define LOCAL_dict_getspecial dict_getfirstvalue
#define LOCAL_dict_setspecial dict_setfirstvalue
#define LOCAL_IS_FIRST
#define LOCAL_IS_VALUE
#elif defined(DEFINE_dict_last)
#define LOCAL_dict_trygetspecial       dict_trygetlast
#define LOCAL_dict_getspecial          dict_getlast
#define LOCAL_dict_delspecial          dict_dellast
#define LOCAL_dict_setspecial          dict_setlast
#define LOCAL_dict_setspecial_pair     dict_setlast_pair
#define LOCAL_dict_setspecial_pair_getindex dict_setlast_pair_getindex
#define LOCAL_IS_LAST
#define LOCAL_IS_PAIR
#elif defined(DEFINE_dict_lastkey)
#define LOCAL_dict_getspecial              dict_getlastkey
#define LOCAL_dict_setspecial              dict_setlastkey
#define LOCAL_dict_setspecial_key_getindex dict_setlastkey_getindex
#define LOCAL_IS_LAST
#define LOCAL_IS_KEY
#elif defined(DEFINE_dict_lastvalue)
#define LOCAL_dict_getspecial dict_getlastvalue
#define LOCAL_dict_setspecial dict_setlastvalue
#define LOCAL_IS_LAST
#define LOCAL_IS_VALUE
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#if (defined(LOCAL_IS_FIRST) + defined(LOCAL_IS_LAST)) != 1
#error "Exactly one of these should have been defined!"
#endif /* ... */
#if (defined(LOCAL_IS_PAIR) + defined(LOCAL_IS_KEY) + defined(LOCAL_IS_VALUE)) != 1
#error "Exactly one of these should have been defined!"
#endif /* ... */


#ifdef LOCAL_IS_FIRST
#define LOCAL_STR_firstlast STR_first
#define LOCAL_str_firstlast "first"
#else /* LOCAL_IS_FIRST */
#define LOCAL_STR_firstlast STR_last
#define LOCAL_str_firstlast "last"
#endif /* !LOCAL_IS_FIRST */

#ifdef LOCAL_IS_KEY
#define LOCAL_str_subtype "key"
#elif defined(LOCAL_IS_VALUE)
#define LOCAL_str_subtype "value"
#else /* ... */
#define LOCAL_str_getset LOCAL_str_firstlast
#define LOCAL_STR_getset LOCAL_STR_firstlast
#endif /* !... */
#ifndef LOCAL_str_getset
#define LOCAL_str_getset LOCAL_str_firstlast LOCAL_str_subtype
#endif /* !LOCAL_str_getset */
#ifndef LOCAL_STR_getset
#define LOCAL_STR_getset LOCAL_str_getset
#endif /* !LOCAL_STR_getset */

DECL_BEGIN

#ifdef LOCAL_IS_KEY
#define LOCAL_Dee_dict_item_getspecial(item) ((item)->di_key)
#elif defined(LOCAL_IS_VALUE)
#define LOCAL_Dee_dict_item_getspecial(item) ((item)->di_value)
#endif /* ... */

#ifdef LOCAL_IS_FIRST
#define LOCAL_dict_loaditem(self, item)      \
	do {                                     \
		(item) = _DeeDict_GetRealVTab(self); \
		while (!(item)->di_key)              \
			++(item);                        \
	}	__WHILE0
#else /* LOCAL_IS_FIRST */
#define LOCAL_dict_loaditem(self, item)      \
	do {                                     \
		(item) = _DeeDict_GetRealVTab(self); \
		(item) += (self)->d_vsize;           \
		while (!(--(item))->di_key)          \
			;                                \
	}	__WHILE0
#endif /* !LOCAL_IS_FIRST */


/************************************************************************/
/* dict_getfirst, dict_getfirstkey, dict_getfirstvalue                  */
/* dict_getlast, dict_getlastkey, dict_getlastvalue                     */
/************************************************************************/
#ifdef LOCAL_IS_PAIR
#define LOCAL_value_type DeeTupleObject
#else /* LOCAL_IS_PAIR */
#define LOCAL_value_type DeeObject
#endif /* !LOCAL_IS_PAIR */

#ifdef LOCAL_dict_getspecial
PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_value_type *DCALL
LOCAL_dict_getspecial(Dict *__restrict self) {
	DREF LOCAL_value_type *result;
	struct Dee_dict_item *item;
#ifdef LOCAL_IS_PAIR
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
#endif /* LOCAL_IS_PAIR */
	DeeDict_LockRead(self);
	if unlikely(!self->d_vused)
		goto err_maybe_r_unbound_unlock;
	LOCAL_dict_loaditem(self, item);
#ifdef LOCAL_IS_PAIR
	result->t_elem[0] = item->di_key;
	result->t_elem[1] = item->di_value;
	Dee_Incref(result->t_elem[0]);
	Dee_Incref(result->t_elem[1]);
#else /* LOCAL_IS_PAIR */
	result = LOCAL_Dee_dict_item_getspecial(item);
	Dee_Incref(result);
#endif /* !LOCAL_IS_PAIR */
	DeeDict_LockEndRead(self);
	return result;
err_maybe_r_unbound_unlock:
	DeeDict_LockEndRead(self);
	DeeRT_ErrTUnboundAttrCStr(&DeeDict_Type, (DeeObject *)self, LOCAL_STR_getset);
#ifdef LOCAL_IS_PAIR
	DeeTuple_FreeUninitializedPair(result);
err:
#endif /* LOCAL_IS_PAIR */
	return NULL;
}
#endif /* LOCAL_dict_getspecial */

#ifdef LOCAL_dict_trygetspecial
PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_value_type *DCALL
LOCAL_dict_trygetspecial(Dict *__restrict self) {
	DREF LOCAL_value_type *result;
	struct Dee_dict_item *item;
#ifdef LOCAL_IS_PAIR
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
#endif /* LOCAL_IS_PAIR */
	DeeDict_LockRead(self);
	if unlikely(!self->d_vused)
		goto err_maybe_r_unbound_unlock;
	LOCAL_dict_loaditem(self, item);
#ifdef LOCAL_IS_PAIR
	result->t_elem[0] = item->di_key;
	result->t_elem[1] = item->di_value;
	Dee_Incref(result->t_elem[0]);
	Dee_Incref(result->t_elem[1]);
#else /* LOCAL_IS_PAIR */
	result = LOCAL_Dee_dict_item_getspecial(item);
	Dee_Incref(result);
#endif /* !LOCAL_IS_PAIR */
	DeeDict_LockEndRead(self);
	return result;
err_maybe_r_unbound_unlock:
	DeeDict_LockEndRead(self);
#ifdef LOCAL_IS_PAIR
	DeeTuple_FreeUninitializedPair(result);
#endif /* LOCAL_IS_PAIR */
	return (DREF LOCAL_value_type *)ITER_DONE;
#ifdef LOCAL_IS_PAIR
err:
	return NULL;
#endif /* LOCAL_IS_PAIR */
}
#endif /* LOCAL_dict_trygetspecial */

#undef LOCAL_value_type

/************************************************************************/
/* dict_delfirst                                                        */
/* dict_dellast                                                         */
/************************************************************************/
#ifdef LOCAL_dict_delspecial
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_delspecial(Dict *__restrict self) {
	DREF DeeObject *old_key;
	DREF DeeObject *old_value;
	struct Dee_dict_item *item;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		return 0;
	}
	LOCAL_dict_loaditem(self, item);
	old_key   = item->di_key;
	old_value = item->di_value;
	item->di_key = NULL;
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	Dee_Decref(old_key);
	Dee_Decref(old_value);
	return 0;
}
#endif /* LOCAL_dict_delspecial */



/************************************************************************/
/* dict_setfirst, dict_setfirstkey, dict_setfirstvalue                  */
/* dict_setlast, dict_setlastkey, dict_setlastvalue                     */
/************************************************************************/
#ifdef LOCAL_dict_setspecial

#ifdef LOCAL_dict_setspecial_pair
PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_dict_vidx_t DCALL
LOCAL_dict_setspecial_pair_getindex(void *arg, Dict *self,
                                    /*virt*/ Dee_dict_vidx_t overwrite_index,
                                    DeeObject **p_value) {
	struct dict_mh_seq_setitem_index_impl_data *data;
	data = (struct dict_mh_seq_setitem_index_impl_data *)arg;
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		err_empty_sequence((DeeObject *)self);
		return Dee_DICT_HTAB_EOF;
	}
#ifdef LOCAL_IS_FIRST
	data->dsqsii_index = 0;
#else /* LOCAL_IS_FIRST */
	data->dsqsii_index = self->d_vused - 1;
#endif /* !LOCAL_IS_FIRST */
	return dict_mh_seq_setitem_index_impl_cb(data, self, overwrite_index, p_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_dict_setspecial_pair(Dict *self, DeeObject *key, DeeObject *value) {
	int result;
	struct dict_mh_seq_setitem_index_impl_data data;
	data.dsqsii_deleted_key = NULL;
	result = dict_setitem_at(self, key, value, &LOCAL_dict_setspecial_pair_getindex, &data);
	if (data.dsqsii_deleted_key) {
		Dee_Decref(data.dsqsii_deleted_key);
		Dee_Decref(data.dsqsii_deleted_value);
	}
	return result;
}
#endif /* LOCAL_dict_setspecial_pair */

#ifdef LOCAL_dict_setspecial_key_getindex
#ifndef DICT_SETSPECIAL_PAIR_OLDKEY_DATA_DEFINED
#define DICT_SETSPECIAL_PAIR_OLDKEY_DATA_DEFINED
struct dict_setspecial_key_old_data {
	DREF DeeObject *sskod_key;   /* out[0..1] Old key */
	DREF DeeObject *sskod_value; /* out[0..1] Old value */
};
#endif /* !DICT_SETSPECIAL_PAIR_OLDKEY_DATA_DEFINED */

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_dict_vidx_t DCALL
LOCAL_dict_setspecial_key_getindex(void *arg, Dict *self,
                                   /*virt*/ Dee_dict_vidx_t overwrite_index,
                                   DeeObject **p_value) {
	struct dict_setspecial_key_old_data *data;
	/*real*/ Dee_dict_vidx_t result;
	struct Dee_dict_item *item;
	(void)overwrite_index;
	data = (struct dict_setspecial_key_old_data *)arg;
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		err_empty_sequence((DeeObject *)self);
		return Dee_DICT_HTAB_EOF;
	}
	LOCAL_dict_loaditem(self, item);
	result = (Dee_dict_vidx_t)(item - _DeeDict_GetRealVTab(self));
	Dee_dict_vidx_real2virt(&result);
	if (overwrite_index == result) {
		data->sskod_key   = NULL;
		data->sskod_value = item->di_value;
		Dee_Incref(data->sskod_value);
	} else {
		/* Delete existing item. */
		data->sskod_key   = item->di_key;   /* Inherit reference */
		data->sskod_value = item->di_value; /* Inherit reference */
		item->di_key = NULL;
		--self->d_vused;
	}
	*p_value = data->sskod_value;
	return result;
}
#endif /* LOCAL_dict_setspecial_key_getindex */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_setspecial(Dict *self, DeeObject *value) {
#ifdef LOCAL_IS_KEY
	int result;
	struct dict_setspecial_key_old_data data;
	data.sskod_key   = NULL;
	data.sskod_value = NULL;
	result = dict_setitem_at(self, value, Dee_None, &LOCAL_dict_setspecial_key_getindex, &data);
	Dee_XDecref(data.sskod_key);
	Dee_XDecref(data.sskod_value);
	return result;
#elif defined(LOCAL_IS_VALUE)
	DREF DeeObject *old_value;
	struct Dee_dict_item *item;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_vused)
		goto err_empty;
	LOCAL_dict_loaditem(self, item);
	Dee_Incref(value);
	old_value = item->di_value;
	item->di_value = value;
	DeeDict_LockEndWrite(self);
	Dee_Decref(old_value);
	return 0;
err_empty:
	DeeDict_LockEndWrite(self);
	return err_empty_sequence((DeeObject *)self);
#else /* ... */
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeSeq_Unpack(value, 2, key_and_value))
		goto err;
	result = LOCAL_dict_setspecial_pair(self, key_and_value[0], key_and_value[1]);
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	return result;
err:
	return -1;
#endif /* !... */
}
#endif /* LOCAL_dict_setspecial */


#undef LOCAL_dict_loaditem
#undef LOCAL_Dee_dict_item_getspecial

DECL_END

#undef LOCAL_STR_firstlast
#undef LOCAL_str_firstlast
#undef LOCAL_str_subtype
#undef LOCAL_str_getset
#undef LOCAL_STR_getset

#undef LOCAL_dict_getspecial
#undef LOCAL_dict_trygetspecial
#undef LOCAL_dict_delspecial
#undef LOCAL_dict_setspecial
#undef LOCAL_dict_setspecial_pair
#undef LOCAL_dict_setspecial_pair_getindex
#undef LOCAL_dict_setspecial_key_getindex
#undef LOCAL_IS_FIRST
#undef LOCAL_IS_LAST
#undef LOCAL_IS_PAIR
#undef LOCAL_IS_KEY
#undef LOCAL_IS_VALUE


#undef DEFINE_dict_first
#undef DEFINE_dict_firstkey
#undef DEFINE_dict_firstvalue
#undef DEFINE_dict_last
#undef DEFINE_dict_lastkey
#undef DEFINE_dict_lastvalue
