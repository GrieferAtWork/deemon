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
#define DEFINE_dict_first
//#define DEFINE_dict_firstkey
//#define DEFINE_dict_firstvalue
//#define DEFINE_dict_last
//#define DEFINE_dict_lastkey
//#define DEFINE_dict_lastvalue
//#define DEFINE_hashset_first
//#define DEFINE_hashset_last
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/dict.h>         /* DeeDict_*, Dee_dict_item, _DeeDict_GetRealVTab */
#include <deemon/error-rt.h>     /* DeeRT_ErrEmptySequence, DeeRT_ErrTUnboundAttrCStr */
#include <deemon/hashset.h>      /* DeeHashSet_*, Dee_hashset_item, _DeeHashSet_GetRealVTab */
#include <deemon/none.h>         /* Dee_None */
#include <deemon/object.h>       /* Dee_Decref, Dee_Incref, Dee_XDecref */
#include <deemon/pair.h>         /* DeeSeqPairObject, DeeSeq_* */
#include <deemon/seq.h>          /* DeeSeq_Unpack */
#include <deemon/types.h>        /* DREF, DeeObject, Dee_AsObject, ITER_DONE */
#include <deemon/util/hash-io.h> /* Dee_HASH_HTAB_EOF, Dee_hash_vidx_real2virt, Dee_hash_vidx_t */

#include "dict-utils.h"

#include <stddef.h> /* NULL */

#if (defined(DEFINE_dict_first) +      \
     defined(DEFINE_dict_firstkey) +   \
     defined(DEFINE_dict_firstvalue) + \
     defined(DEFINE_dict_last) +       \
     defined(DEFINE_dict_lastkey) +    \
     defined(DEFINE_dict_lastvalue) +  \
     defined(DEFINE_hashset_first) +   \
     defined(DEFINE_hashset_last)) != 1
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
#elif defined(DEFINE_hashset_first)
#define LOCAL_dict_trygetspecial           hashset_trygetfirst
#define LOCAL_dict_getspecial              hashset_getfirst
#define LOCAL_dict_delspecial              hashset_delfirst
#define LOCAL_dict_setspecial              hashset_setfirst
#define LOCAL_dict_setspecial_key_getindex hashset_setfirst_getindex
#define LOCAL_IS_FIRST
#define LOCAL_IS_KEY
#define LOCAL_IS_HASHSET
#elif defined(DEFINE_hashset_last)
#define LOCAL_dict_trygetspecial           hashset_trygetlast
#define LOCAL_dict_getspecial              hashset_getlast
#define LOCAL_dict_delspecial              hashset_dellast
#define LOCAL_dict_setspecial              hashset_setlast
#define LOCAL_dict_setspecial_key_getindex hashset_setlast_getindex
#define LOCAL_IS_LAST
#define LOCAL_IS_KEY
#define LOCAL_IS_HASHSET
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

#ifdef LOCAL_IS_HASHSET
#ifdef __INTELLISENSE__
#include "hashset.c"
#endif /* __INTELLISENSE__ */

#define LOCAL_Dict                         HashSet
#define LOCAL_Dee_dict_item                Dee_hashset_item
#define LOCAL_DeeDict_LockTryRead(self)    DeeHashSet_LockTryRead(self)
#define LOCAL_DeeDict_LockTryWrite(self)   DeeHashSet_LockTryWrite(self)
#define LOCAL_DeeDict_LockRead(self)       DeeHashSet_LockRead(self)
#define LOCAL_DeeDict_LockWrite(self)      DeeHashSet_LockWrite(self)
#define LOCAL_DeeDict_LockTryUpgrade(self) DeeHashSet_LockTryUpgrade(self)
#define LOCAL_DeeDict_LockUpgrade(self)    DeeHashSet_LockUpgrade(self)
#define LOCAL_DeeDict_LockDowngrade(self)  DeeHashSet_LockDowngrade(self)
#define LOCAL_DeeDict_LockEndWrite(self)   DeeHashSet_LockEndWrite(self)
#define LOCAL_DeeDict_LockEndRead(self)    DeeHashSet_LockEndRead(self)
#define LOCAL_DeeDict_LockEnd(self)        DeeHashSet_LockEnd(self)
#define LOCAL__DeeDict_GetRealVTab         _DeeHashSet_GetRealVTab
#define LOCAL_DeeDict_Type                 DeeHashSet_Type
#define LOCAL_dict_autoshrink              hashset_autoshrink

/* Map dict fields to hashsets */
#define d_valloc  hs_valloc
#define d_vsize   hs_vsize
#define d_vused   hs_vused
#define d_vtab    hs_vtab
#define d_hmask   hs_hmask
#define d_hidxops hs_hidxops
#define d_htab    hs_htab
#define d_lock    hs_lock
#define di_hash   hsi_hash
#define di_key    hsi_key
#else /* LOCAL_IS_HASHSET */
#ifdef __INTELLISENSE__
#include "dict.c"
#endif /* __INTELLISENSE__ */

#define LOCAL_Dict                         Dict
#define LOCAL_Dee_dict_item                Dee_dict_item
#define LOCAL_DeeDict_LockTryRead(self)    DeeDict_LockTryRead(self)
#define LOCAL_DeeDict_LockTryWrite(self)   DeeDict_LockTryWrite(self)
#define LOCAL_DeeDict_LockRead(self)       DeeDict_LockRead(self)
#define LOCAL_DeeDict_LockWrite(self)      DeeDict_LockWrite(self)
#define LOCAL_DeeDict_LockTryUpgrade(self) DeeDict_LockTryUpgrade(self)
#define LOCAL_DeeDict_LockUpgrade(self)    DeeDict_LockUpgrade(self)
#define LOCAL_DeeDict_LockDowngrade(self)  DeeDict_LockDowngrade(self)
#define LOCAL_DeeDict_LockEndWrite(self)   DeeDict_LockEndWrite(self)
#define LOCAL_DeeDict_LockEndRead(self)    DeeDict_LockEndRead(self)
#define LOCAL_DeeDict_LockEnd(self)        DeeDict_LockEnd(self)
#define LOCAL__DeeDict_GetRealVTab         _DeeDict_GetRealVTab
#define LOCAL_DeeDict_Type                 DeeDict_Type
#define LOCAL_dict_autoshrink              dict_autoshrink
#endif /* !LOCAL_IS_HASHSET */

DECL_BEGIN

#ifdef LOCAL_IS_KEY
#define LOCAL_Dee_dict_item_getspecial(item) ((item)->di_key)
#elif defined(LOCAL_IS_VALUE)
#define LOCAL_Dee_dict_item_getspecial(item) ((item)->di_value)
#endif /* ... */

#ifdef LOCAL_IS_FIRST
#define LOCAL_dict_loaditem(self, item)            \
	do {                                           \
		(item) = LOCAL__DeeDict_GetRealVTab(self); \
		while (!(item)->di_key)                    \
			++(item);                              \
	}	__WHILE0
#else /* LOCAL_IS_FIRST */
#define LOCAL_dict_loaditem(self, item)            \
	do {                                           \
		(item) = LOCAL__DeeDict_GetRealVTab(self); \
		(item) += (self)->d_vsize;                 \
		while (!(--(item))->di_key)                \
			;                                      \
	}	__WHILE0
#endif /* !LOCAL_IS_FIRST */


/************************************************************************/
/* dict_getfirst, dict_getfirstkey, dict_getfirstvalue                  */
/* dict_getlast, dict_getlastkey, dict_getlastvalue                     */
/************************************************************************/
#ifdef LOCAL_IS_PAIR
#define LOCAL_value_type DeeSeqPairObject
#else /* LOCAL_IS_PAIR */
#define LOCAL_value_type DeeObject
#endif /* !LOCAL_IS_PAIR */

#ifdef LOCAL_dict_getspecial
PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_value_type *DCALL
LOCAL_dict_getspecial(LOCAL_Dict *__restrict self) {
	DREF LOCAL_value_type *result;
	struct LOCAL_Dee_dict_item *item;
#ifdef LOCAL_IS_PAIR
	result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
#endif /* LOCAL_IS_PAIR */
	DeeDict_LockRead(self);
	if unlikely(!self->d_vused)
		goto err_maybe_r_unbound_unlock;
	LOCAL_dict_loaditem(self, item);
#ifdef LOCAL_IS_PAIR
	DeeSeq_InitPairv(result, item->di_key_and_value);
#else /* LOCAL_IS_PAIR */
	result = LOCAL_Dee_dict_item_getspecial(item);
	Dee_Incref(result);
#endif /* !LOCAL_IS_PAIR */
	DeeDict_LockEndRead(self);
	return result;
err_maybe_r_unbound_unlock:
	DeeDict_LockEndRead(self);
	DeeRT_ErrTUnboundAttrCStr(&LOCAL_DeeDict_Type,
	                          Dee_AsObject(self),
	                          LOCAL_STR_getset);
#ifdef LOCAL_IS_PAIR
	DeeSeq_FreePairUninitialized(result);
err:
#endif /* LOCAL_IS_PAIR */
	return NULL;
}
#endif /* LOCAL_dict_getspecial */

#ifdef LOCAL_dict_trygetspecial
PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_value_type *DCALL
LOCAL_dict_trygetspecial(LOCAL_Dict *__restrict self) {
	DREF LOCAL_value_type *result;
	struct LOCAL_Dee_dict_item *item;
#ifdef LOCAL_IS_PAIR
	result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
#endif /* LOCAL_IS_PAIR */
	DeeDict_LockRead(self);
	if unlikely(!self->d_vused)
		goto err_maybe_r_unbound_unlock;
	LOCAL_dict_loaditem(self, item);
#ifdef LOCAL_IS_PAIR
	DeeSeq_InitPairv(result, item->di_key_and_value);
#else /* LOCAL_IS_PAIR */
	result = LOCAL_Dee_dict_item_getspecial(item);
	Dee_Incref(result);
#endif /* !LOCAL_IS_PAIR */
	DeeDict_LockEndRead(self);
	return result;
err_maybe_r_unbound_unlock:
	DeeDict_LockEndRead(self);
#ifdef LOCAL_IS_PAIR
	DeeSeq_FreePairUninitialized(result);
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
LOCAL_dict_delspecial(LOCAL_Dict *__restrict self) {
	DREF DeeObject *old_key;
#ifndef LOCAL_IS_HASHSET
	DREF DeeObject *old_value;
#endif /* !LOCAL_IS_HASHSET */
	struct LOCAL_Dee_dict_item *item;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		return 0;
	}
	LOCAL_dict_loaditem(self, item);
	old_key = item->di_key;
	item->di_key = NULL;
#ifndef LOCAL_IS_HASHSET
	old_value = item->di_value;
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
#endif /* !LOCAL_IS_HASHSET */
	--self->d_vused;
	LOCAL_dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	Dee_Decref(old_key);
#ifndef LOCAL_IS_HASHSET
	Dee_Decref(old_value);
#endif /* !LOCAL_IS_HASHSET */
	return 0;
}
#endif /* LOCAL_dict_delspecial */



/************************************************************************/
/* dict_setfirst, dict_setfirstkey, dict_setfirstvalue                  */
/* dict_setlast, dict_setlastkey, dict_setlastvalue                     */
/************************************************************************/
#ifdef LOCAL_dict_setspecial

#ifdef LOCAL_dict_setspecial_pair
PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
LOCAL_dict_setspecial_pair_getindex(void *arg, LOCAL_Dict *self,
                                    /*virt*/ Dee_hash_vidx_t overwrite_index,
                                    DeeObject **p_value) {
	struct dict_mh_seq_setitem_index_impl_data *data;
	data = (struct dict_mh_seq_setitem_index_impl_data *)arg;
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		DeeRT_ErrEmptySequence(self);
		return Dee_HASH_HTAB_EOF;
	}
#ifdef LOCAL_IS_FIRST
	data->dsqsii_index = 0;
#else /* LOCAL_IS_FIRST */
	data->dsqsii_index = self->d_vused - 1;
#endif /* !LOCAL_IS_FIRST */
	return dict_mh_seq_setitem_index_impl_cb(data, self, overwrite_index, p_value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_dict_setspecial_pair(LOCAL_Dict *self, DeeObject *key, DeeObject *value) {
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
#ifndef LOCAL_IS_HASHSET
	DREF DeeObject *sskod_value; /* out[0..1] Old value */
#endif /* !LOCAL_IS_HASHSET */
};
#endif /* !DICT_SETSPECIAL_PAIR_OLDKEY_DATA_DEFINED */

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
LOCAL_dict_setspecial_key_getindex(void *arg, LOCAL_Dict *self,
                                   /*virt*/ Dee_hash_vidx_t overwrite_index
#ifndef LOCAL_IS_HASHSET
                                   , DeeObject **p_value
#endif /* !LOCAL_IS_HASHSET */
                                   ) {
	struct dict_setspecial_key_old_data *data;
	/*real*/ Dee_hash_vidx_t result;
	struct LOCAL_Dee_dict_item *item;
	(void)overwrite_index;
	data = (struct dict_setspecial_key_old_data *)arg;
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		DeeRT_ErrEmptySequence(self);
		return Dee_HASH_HTAB_EOF;
	}
	LOCAL_dict_loaditem(self, item);
	result = (Dee_hash_vidx_t)(item - LOCAL__DeeDict_GetRealVTab(self));
	Dee_hash_vidx_real2virt(&result);
	if (overwrite_index == result) {
		data->sskod_key = NULL;
#ifndef LOCAL_IS_HASHSET
		data->sskod_value = item->di_value;
		Dee_Incref(data->sskod_value);
#endif /* !LOCAL_IS_HASHSET */
	} else {
		/* Delete existing item. */
		data->sskod_key   = item->di_key;   /* Inherit reference */
#ifndef LOCAL_IS_HASHSET
		data->sskod_value = item->di_value; /* Inherit reference */
#endif /* !LOCAL_IS_HASHSET */
		item->di_key = NULL;
		--self->d_vused;
	}
#ifndef LOCAL_IS_HASHSET
	*p_value = data->sskod_value;
#endif /* !LOCAL_IS_HASHSET */
	return result;
}
#endif /* LOCAL_dict_setspecial_key_getindex */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_setspecial(LOCAL_Dict *self, DeeObject *value) {
#ifdef LOCAL_IS_KEY
	int result;
	struct dict_setspecial_key_old_data data;
	data.sskod_key = NULL;
#ifndef LOCAL_IS_HASHSET
	data.sskod_value = NULL;
#endif /* !LOCAL_IS_HASHSET */
#ifdef LOCAL_IS_HASHSET
	result = hashset_insert_at(self, value, &LOCAL_dict_setspecial_key_getindex, &data);
#else /* LOCAL_IS_HASHSET */
	result = dict_setitem_at(self, value, Dee_None, &LOCAL_dict_setspecial_key_getindex, &data);
#endif /* !LOCAL_IS_HASHSET */
	Dee_XDecref(data.sskod_key);
#ifndef LOCAL_IS_HASHSET
	Dee_XDecref(data.sskod_value);
#endif /* !LOCAL_IS_HASHSET */
	return result;
#elif defined(LOCAL_IS_VALUE)
	DREF DeeObject *old_value;
	struct LOCAL_Dee_dict_item *item;
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
	return DeeRT_ErrEmptySequence(self);
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

#undef LOCAL_Dict
#undef LOCAL_Dee_dict_item
#undef LOCAL_DeeDict_LockTryRead
#undef LOCAL_DeeDict_LockTryWrite
#undef LOCAL_DeeDict_LockRead
#undef LOCAL_DeeDict_LockWrite
#undef LOCAL_DeeDict_LockTryUpgrade
#undef LOCAL_DeeDict_LockUpgrade
#undef LOCAL_DeeDict_LockDowngrade
#undef LOCAL_DeeDict_LockEndWrite
#undef LOCAL_DeeDict_LockEndRead
#undef LOCAL_DeeDict_LockEnd
#undef LOCAL__DeeDict_GetRealVTab
#undef LOCAL_DeeDict_Type
#undef LOCAL_dict_autoshrink
#ifdef LOCAL_IS_HASHSET
#undef d_valloc
#undef d_vsize
#undef d_vused
#undef d_vtab
#undef d_hmask
#undef d_hidxops
#undef d_htab
#undef d_lock
#undef di_hash
#undef di_key
#endif /* LOCAL_IS_HASHSET */
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
#undef LOCAL_IS_HASHSET
#undef LOCAL_IS_KEY
#undef LOCAL_IS_VALUE


#undef DEFINE_dict_first
#undef DEFINE_dict_firstkey
#undef DEFINE_dict_firstvalue
#undef DEFINE_dict_last
#undef DEFINE_dict_lastkey
#undef DEFINE_dict_lastvalue
#undef DEFINE_hashset_first
#undef DEFINE_hashset_last
