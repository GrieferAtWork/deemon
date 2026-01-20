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
#ifndef GUARD_DEEMON_RUNTIME_STRUCT_C
#define GUARD_DEEMON_RUNTIME_STRUCT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/kwds.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/serial.h>
#include <deemon/struct.h>
#include <deemon/system-features.h>
#include <deemon/types.h>
#include <deemon/util/atomic.h>
#include <deemon/variant.h>

#include <hybrid/host.h>      /* __pic__ */
#include <hybrid/int128.h>
#include <hybrid/typecore.h>  /* __BYTE_TYPE__, __SIZEOF_DOUBLE__, __SIZEOF_FLOAT__, __SIZEOF_LONG_DOUBLE__, __UINTPTR_HALF_TYPE__ */
#include <hybrid/unaligned.h>

#include "method-hints.h"
#include "runtime_error.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* intN_t, uintN_t */

DECL_BEGIN

#undef uintptr_half_t
#define uintptr_half_t __UINTPTR_HALF_TYPE__
#undef byte_t
#define byte_t __BYTE_TYPE__

/* Check if "type" defines a type_member field at "field_offset" */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
struct_offset_exists(DeeTypeObject const *type,
                     uintptr_half_t field_offset) {
	struct type_member const *member = type->tp_members;
	if (member) {
		for (; member->m_name; ++member) {
			if (!TYPE_MEMBER_ISFIELD(member))
				continue;
			if (member->m_desc.md_field.mdf_offset == field_offset)
				return true;
		}
	}
	return false;
}

/* Check if a field at "field_offset" is defined within [origin,stop) */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
struct_offset_renamed_after(DeeTypeObject const *stop,
                            DeeTypeObject const *origin,
                            uintptr_half_t field_offset) {
	while (origin != stop) {
		if (struct_offset_exists(origin, field_offset))
			return true;
		origin = DeeType_Base(origin);
	}
	return false;
}

/* Check if a field at "field_offset" exists below (or within) "start" */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
struct_offset_exists_r(DeeTypeObject const *start,
                       uintptr_half_t field_offset) {
	while (start != &DeeObject_Type) {
		if (struct_offset_exists(start, field_offset))
			return true;
		start = DeeType_Base(start);
		if unlikely(!start)
			break;
	}
	return false;
}

PRIVATE NONNULL((1, 2, 3)) Dee_ssize_t DCALL
struct_foreach_field_at(DeeTypeObject *type,
                        DeeTypeObject const *origin,
                        Dee_struct_object_foreach_field_cb_t cb,
                        Dee_struct_object_foreach_field_undo_t undo,
                        void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_member const *member = type->tp_members;
	if (member) {
		for (; member->m_name; ++member) {
			if (!TYPE_MEMBER_ISFIELD(member))
				continue;
			/* Skip members that get renamed later on */
			if (struct_offset_renamed_after(type, origin, member->m_desc.md_field.mdf_offset))
				continue;
			temp = (*cb)(arg, type, member);
			if unlikely(temp < 0) {
				if (undo) {
					while (member-- > type->tp_members) {
						if (!TYPE_MEMBER_ISFIELD(member))
							continue;
						if (struct_offset_renamed_after(type, origin, member->m_desc.md_field.mdf_offset))
							continue;
						(*undo)(arg, type, member);
					}
				}
				return temp;
			}
			result += temp;
		}
	}
	return result;
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
struct_foreach_field_undo_at(DeeTypeObject *type,
                             DeeTypeObject const *origin,
                             Dee_struct_object_foreach_field_undo_t undo,
                             void *arg) {
	struct type_member const *member = type->tp_members;
	if (member) {
		while (member->m_name)
			++member;
		while (member-- > type->tp_members) {
			if (!TYPE_MEMBER_ISFIELD(member))
				continue;
			if (struct_offset_renamed_after(type, origin, member->m_desc.md_field.mdf_offset))
				continue;
			(*undo)(arg, type, member);
		}
	}
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
struct_foreach_field_undo_at_step(DeeTypeObject *type,
                                  DeeTypeObject const *origin,
                                  Dee_struct_object_foreach_field_undo_t undo,
                                  void *arg) {
	DeeTypeObject *next;
	struct_foreach_field_undo_at(type, origin, undo, arg);
	next = DeeType_Base(type);
	if (next && next != &DeeObject_Type)
		struct_foreach_field_undo_at_step(next, origin, undo, arg);
}

PRIVATE NONNULL((1, 2, 3)) Dee_ssize_t DCALL
struct_foreach_field_at_step(DeeTypeObject *type,
                             DeeTypeObject const *origin,
                             Dee_struct_object_foreach_field_cb_t cb,
                             Dee_struct_object_foreach_field_undo_t undo,
                             void *arg) {
	Dee_ssize_t temp, result = 0;
	DeeTypeObject *next = DeeType_Base(type);
	if (next && next != &DeeObject_Type) {
		result = struct_foreach_field_at_step(next, origin, cb, undo, arg);
		if unlikely(result < 0)
			return result;
	}
	temp = struct_foreach_field_at(type, origin, cb, undo, arg);
	if unlikely(temp < 0) {
		if (undo && next && next != &DeeObject_Type)
			struct_foreach_field_undo_at_step(next, origin, undo, arg);
		return temp;
	}
	result += temp;
	return result;
}

#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
struct struct_field_cache_builder {
	struct Dee_type_struct_field *sfcb_fieldv; /* [0..sfcb_fieldc|ALLOC(sfcb_fielda)][owned] Vector of fields */
	size_t                        sfcb_fieldc; /* # of fields */
	size_t                        sfcb_fielda; /* Allocated # of fields */
};

PRIVATE NONNULL_T((2, 3)) Dee_ssize_t DCALL
struct_field_cache_builder_append(void *arg, DeeTypeObject *declaring_type,
                                  struct Dee_type_member const *field) {
	struct Dee_type_struct_field *entry;
	struct struct_field_cache_builder *me;
	me = (struct struct_field_cache_builder *)arg;
	ASSERT(me->sfcb_fieldc <= me->sfcb_fielda);
	if (me->sfcb_fieldc >= me->sfcb_fielda) {
		struct Dee_type_struct_field *new_fieldv;
		size_t new_fielda = me->sfcb_fielda * 2;
		if (new_fielda < 8)
			new_fielda = 8;
		new_fieldv = (struct Dee_type_struct_field *)Dee_TryReallocc(me->sfcb_fieldv, new_fielda,
		                                                             sizeof(struct Dee_type_struct_field));
		if unlikely(!new_fieldv) {
			new_fielda = me->sfcb_fieldc + 1;
			new_fieldv = (struct Dee_type_struct_field *)Dee_TryReallocc(me->sfcb_fieldv, new_fielda,
			                                                             sizeof(struct Dee_type_struct_field));
			if unlikely(!new_fieldv)
				goto err;
		}
		me->sfcb_fieldv = new_fieldv;
		me->sfcb_fielda = new_fielda;
	}
	entry = &me->sfcb_fieldv[me->sfcb_fieldc++];
	entry->tsf_member   = field;
	entry->tsf_decltype = declaring_type;
	return 0;
err:
	return -1;
}


/* Allocate a new cache for "struct Dee_type_struct_cache::tsc_allfields" */
PRIVATE WUNUSED NONNULL((1)) struct Dee_type_struct_field *DCALL
Dee_type_struct_getallfields_uncached(DeeTypeObject *__restrict self) {
	Dee_ssize_t status;
	struct Dee_type_struct_field *result;
	struct struct_field_cache_builder builder;
	bzero(&builder, sizeof(builder));
	status = struct_foreach_field_at_step(self, self, &struct_field_cache_builder_append,
	                                      NULL, &builder);
	if unlikely(status < 0)
		goto err;
	result = (struct Dee_type_struct_field *)Dee_TryReallocc(builder.sfcb_fieldv,
	                                                         builder.sfcb_fieldc + 1,
	                                                         sizeof(struct Dee_type_struct_field));
	if unlikely(!result)
		goto err;
	result[builder.sfcb_fieldc].tsf_decltype = NULL;
	result[builder.sfcb_fieldc].tsf_member   = NULL;
	return result;
err:
	Dee_Free(builder.sfcb_fieldv);
	return NULL;
}

/* Allocate a new cache for "struct Dee_type_struct_cache::tsc_locfields" */
PRIVATE WUNUSED NONNULL((1)) struct Dee_type_member const **DCALL
Dee_type_struct_getlocfields_uncached(DeeTypeObject *__restrict self) {
	struct Dee_type_member const **result;
	struct Dee_type_member const **result_v = NULL;
	size_t result_a = 0, result_c = 0;
	struct type_member const *member;
	if ((member = self->tp_members) != NULL) {
		/* Enumerate struct fields that are local, and
		 * relevant (need to be called during visit/fini) */
		DeeTypeObject *tp_base = DeeType_Base(self);
		for (; member->m_name; ++member) {
			if (!TYPE_MEMBER_ISFIELD(member))
				continue;
			if (struct_offset_exists_r(tp_base, member->m_desc.md_field.mdf_offset))
				continue;
			switch (member->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
			case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
			case STRUCT_OBJECT & ~STRUCT_CONST:
			case STRUCT_WOBJECT_OPT:
			case STRUCT_WOBJECT:
			case STRUCT_VARIANT:
				ASSERT(result_c <= result_a);
				if (result_c >= result_a) {
					struct Dee_type_member const * *new_fieldv;
					size_t new_fielda = result_a * 2;
					if (new_fielda < 8)
						new_fielda = 8;
					if unlikely(new_fielda < result_c + 1)
						new_fielda = result_c + 1;
					new_fieldv = (struct Dee_type_member const **)Dee_TryReallocc(result_v, new_fielda,
					                                                              sizeof(struct Dee_type_member const *));
					if unlikely(!new_fieldv) {
						new_fielda = result_c + 1;
						new_fieldv = (struct Dee_type_member const **)Dee_TryReallocc(result_v, new_fielda,
						                                                              sizeof(struct Dee_type_member const *));
						if unlikely(!new_fieldv)
							goto err;
					}
					result_v = new_fieldv;
					result_a = new_fielda;
				}
				result_v[result_c++] = member;
				break;
			default: break;
			}
		}
	}
	result = (struct Dee_type_member const **)Dee_TryReallocc(result_v, result_c + 1,
	                                                          sizeof(struct Dee_type_member const *));
	if unlikely(!result)
		goto err;
	result[result_c] = NULL;
	return result;
err:
	Dee_Free(result_v);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) struct Dee_type_struct_cache *DCALL
Dee_type_struct_cache_of(DeeTypeObject *__restrict self) {
	struct Dee_type_struct_cache *result;
	struct Dee_type_mh_cache *mhc = Dee_type_mh_cache_of(self);
	if unlikely(!mhc)
		goto err;
	result = atomic_read(&mhc->mhc_structcache);
	if unlikely(!result) {
		result = Dee_type_struct_cache_alloc();
		if likely(result) {
			if likely(atomic_cmpxch(&mhc->mhc_structcache, NULL, result)) {
				if (!(self->tp_flags & TP_FHEAP))
					Dee_UntrackAlloc(result);
			} else {
				Dee_type_struct_cache_free(result);
				result = atomic_read(&mhc->mhc_structcache);
				ASSERT(result);
			}
		}
	}
	return result;
err:
	return NULL;
}

/* Return cache for "struct Dee_type_struct_cache::tsc_allfields" */
PRIVATE WUNUSED NONNULL((1)) struct Dee_type_struct_field const *DCALL
Dee_type_struct_getallfields(DeeTypeObject *__restrict self) {
	struct Dee_type_struct_field *result;
	struct Dee_type_struct_cache *tsc = Dee_type_struct_cache_of(self);
	if unlikely(!tsc)
		goto err;
	result = atomic_read(&tsc->tsc_allfields);
	if unlikely(!result) {
		result = Dee_type_struct_getallfields_uncached(self);
		if likely(result) {
			result = (struct Dee_type_struct_field *)Dee_UntrackAlloc(result);
			if likely(atomic_cmpxch(&tsc->tsc_allfields, NULL, result)) {
#ifndef NDEBUG
				if (!(self->tp_flags & TP_FHEAP))
					Dee_UntrackAlloc(result);
#endif /* !NDEBUG */
			} else {
				Dee_type_struct_cache_free(result);
				result = atomic_read(&tsc->tsc_allfields);
				ASSERT(result);
			}
		}
	}
	return result;
err:
	return NULL;
}

/* Return cache for "struct Dee_type_struct_cache::tsc_locfields" */
PRIVATE WUNUSED NONNULL((1)) struct Dee_type_member const *const *DCALL
Dee_type_struct_getlocfields(DeeTypeObject *__restrict self) {
	struct Dee_type_member const **result;
	struct Dee_type_struct_cache *tsc = Dee_type_struct_cache_of(self);
	if unlikely(!tsc)
		goto err;
	result = atomic_read(&tsc->tsc_locfields);
	if unlikely(!result) {
		result = Dee_type_struct_getlocfields_uncached(self);
		if likely(result) {
			result = (struct Dee_type_member const **)Dee_UntrackAlloc(result);
			if likely(atomic_cmpxch(&tsc->tsc_locfields, NULL, result)) {
#ifndef NDEBUG
				if (!(self->tp_flags & TP_FHEAP))
					Dee_UntrackAlloc(result);
#endif /* !NDEBUG */
			} else {
				Dee_type_struct_cache_free(result);
				result = atomic_read(&tsc->tsc_locfields);
				ASSERT(result);
			}
		}
	}
	return result;
err:
	return NULL;
}
#endif /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */



/* Enumerate struct object fields in order as they should be accepted
 * by constructors, and are printed by "DeeStructObject_PrintRepr()".
 *
 * Also does all the handling to prevent fields that have been renamed
 * by sub-classes from being enumerated too early with incorrect names
 *
 * @param: undo: When non-NULL, invoke this for every already-processed
 *               field if `cb' happens to return a negative value.
 *
 * @return: >= 0: Success (return value is the sum of return values from "cb")
 * @return: < 0:  Enumeration stopped prematurely (return value is first
 *                negative return value of "cb" -- this function can't throw
 *                any errors on its own, meaning as long as "cb" doesn't throw
 *                any errors when returning negative, this function returning
 *                negative also means that no exception was thrown) */
PUBLIC NONNULL((1, 2)) Dee_ssize_t DCALL
DeeStructObject_ForeachField(DeeTypeObject *__restrict type,
                             Dee_struct_object_foreach_field_cb_t cb,
                             Dee_struct_object_foreach_field_undo_t undo,
                             void *arg) {
#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
	struct Dee_type_struct_field const *fields;
#endif /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);

	/* Have a field in "type->tp_mhcache" that caches the order of fields here
	 * Reason: enumeration of struct fields is slow because of the field rename rules,
	 *         which results in a **lot** of nested loops slowing stuff down! */
#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
	if likely((fields = Dee_type_struct_getallfields(type)) != NULL) {
		size_t i;
		Dee_ssize_t temp, result = 0;
		for (i = 0; fields[i].tsf_member; ++i) {
			temp = (*cb)(arg, fields[i].tsf_decltype, fields[i].tsf_member);
			if unlikely(temp < 0) {
				if (undo) {
					while (i--)
						(*undo)(arg, fields[i].tsf_decltype, fields[i].tsf_member);
				}
				result = temp;
				break;
			}
			result += temp;
		}
		return result;
	}
#endif /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */

	/* Fallback: enumerate fields on-the-fly (slow) */
	return struct_foreach_field_at_step(type, type, cb, undo, arg);
}


PRIVATE NONNULL((2, 3)) void DCALL
struct_fini_cb(void *arg, DeeTypeObject *declaring_type,
               struct type_member const *field) {
	DeeObject *me = (DeeObject *)arg;
	byte_t *dst = (byte_t *)me + field->m_desc.md_field.mdf_offset;
	(void)declaring_type;
	switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST:
		Dee_XDecref(*(DeeObject **)dst);
		break;
	case STRUCT_WOBJECT_OPT:
	case STRUCT_WOBJECT:
		Dee_weakref_fini((struct weakref *)dst);
		break;
	case STRUCT_VARIANT:
		Dee_variant_fini((struct Dee_variant *)dst);
		break;
	default: break;
	}
}

struct struct_copy_data {
	DeeObject *scd_dst;
	DeeObject *scd_src;
};

PRIVATE NONNULL((2, 3)) void DCALL
struct_copy_undo_cb(void *arg, DeeTypeObject *declaring_type,
                    struct type_member const *field) {
	struct struct_copy_data *data = (struct struct_copy_data *)arg;
	struct_fini_cb(data->scd_dst, declaring_type, field);
}

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_copy_cb(void *arg, DeeTypeObject *declaring_type,
               struct type_member const *field) {
	struct struct_copy_data *data = (struct struct_copy_data *)arg;
	byte_t *dst = (byte_t *)data->scd_dst + field->m_desc.md_field.mdf_offset;
	byte_t *src = (byte_t *)data->scd_src + field->m_desc.md_field.mdf_offset;
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	(void)declaring_type;
	switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST: {
		DeeObject *obj = *(DeeObject *const *)src;
		*(DeeObject **)dst = obj;
		Dee_XIncref(obj);
	}	break;
	case STRUCT_WOBJECT_OPT:
	case STRUCT_WOBJECT:
		Dee_weakref_copy((struct weakref *)dst, (struct weakref *)src);
		break;
	case STRUCT_VARIANT:
		Dee_variant_init_copy((struct Dee_variant *)dst, (struct Dee_variant *)src);
		break;
	case STRUCT_CHAR:
	case STRUCT_BOOLBIT0:
	case STRUCT_BOOLBIT1:
	case STRUCT_BOOLBIT2:
	case STRUCT_BOOLBIT3:
	case STRUCT_BOOLBIT4:
	case STRUCT_BOOLBIT5:
	case STRUCT_BOOLBIT6:
	case STRUCT_BOOLBIT7:
	case STRUCT_BOOL8:
	case STRUCT_INT8:
	case STRUCT_UNSIGNED | STRUCT_INT8:
		*dst = *src;
		break;
	case STRUCT_BOOL16:
	case STRUCT_INT16:
	case STRUCT_UNSIGNED | STRUCT_INT16:
		memcpy(dst, src, 2);
		break;
	case STRUCT_BOOL32:
	case STRUCT_INT32:
	case STRUCT_UNSIGNED | STRUCT_INT32:
		memcpy(dst, src, 4);
		break;
	case STRUCT_BOOL64:
	case STRUCT_INT64:
	case STRUCT_UNSIGNED | STRUCT_INT64:
		memcpy(dst, src, 8);
		break;
	case STRUCT_INT128:
	case STRUCT_UNSIGNED | STRUCT_INT128:
		memcpy(dst, src, 16);
		break;
	case STRUCT_FLOAT:
		memcpy(dst, src, __SIZEOF_FLOAT__);
		break;
	case STRUCT_DOUBLE:
		memcpy(dst, src, __SIZEOF_DOUBLE__);
		break;
	case STRUCT_LDOUBLE:
		memcpy(dst, src, __SIZEOF_LONG_DOUBLE__);
		break;
	case STRUCT_VOID:
		break;
	default: return DeeError_NOTIMPLEMENTED();
	}
	return 0;
}


PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_deepcopy_cb(void *arg, DeeTypeObject *declaring_type,
                   struct type_member const *field) {
	struct struct_copy_data *data = (struct struct_copy_data *)arg;
	byte_t *dst = (byte_t *)data->scd_dst + field->m_desc.md_field.mdf_offset;
	byte_t *src = (byte_t *)data->scd_src + field->m_desc.md_field.mdf_offset;
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	(void)declaring_type;
	switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST: {
		DeeObject *obj = *(DeeObject *const *)src;
		if (obj) {
			obj = DeeObject_DeepCopy(obj);
			if unlikely(!obj)
				return -1;
		}
		*(DREF DeeObject **)dst = obj; /* Inherit reference */
	}	break;
	case STRUCT_VARIANT:
		return Dee_variant_init_deepcopy((struct Dee_variant *)dst, (struct Dee_variant *)src);
	default:
		return struct_copy_cb(arg, declaring_type, field);
	}
	return 0;
}


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeStructObject_Copy(DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	struct struct_copy_data data;
	data.scd_dst = self;
	data.scd_src = other;
	return (int)DeeStructObject_ForeachField(Dee_TYPE(self),
	                                         &struct_copy_cb,
	                                         &struct_copy_undo_cb,
	                                         &data);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeStructObject_Deep(DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	struct struct_copy_data data;
	data.scd_dst = self;
	data.scd_src = other;
	return (int)DeeStructObject_ForeachField(Dee_TYPE(self),
	                                         &struct_deepcopy_cb,
	                                         &struct_copy_undo_cb,
	                                         &data);
}


PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_fini_all_cb(void *arg, DeeTypeObject *declaring_type,
                   struct type_member const *field) {
	struct_fini_cb(arg, declaring_type, field);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
struct_fini_all(DeeObject *__restrict self) {
	DeeStructObject_ForeachField(Dee_TYPE(self), &struct_fini_all_cb, NULL, self);
}


struct struct_init_data {
	DeeObject        *sid_self;    /* [1..1] Object being initialized */
	DeeKwArgs         sid_kwargs;  /* Keyword arguments */
	size_t            sid_posargc; /* (Remaining) positional argument count */
	DeeObject *const *sid_posargv; /* [1..1][0..sid_posargc] (Remaining) positional argument vector */
};

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL /* from "./type_member.c" */
Dee_type_member_set_impl(struct type_member const *desc,
                         DeeObject *self, DeeObject *value);

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_type_member_init(struct type_member const *desc,
                     DeeObject *self, DeeObject *value) {
	byte_t *dst = (byte_t *)self + desc->m_desc.md_field.mdf_offset;
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST:
		Dee_Incref(value);
		*(DeeObject **)dst = value;
		break;
	case STRUCT_WOBJECT_OPT:
		if (DeeNone_Check(value)) {
			Dee_weakref_initempty((struct weakref *)dst);
			return 0;
		}
		ATTR_FALLTHROUGH
	case STRUCT_WOBJECT:
		if unlikely(!Dee_weakref_init((struct weakref *)dst, value, NULL))
			return err_cannot_weak_reference(value);
		break;
	case STRUCT_VARIANT:
		Dee_variant_init_object((struct Dee_variant *)dst, value);
		break;
	default:
		return Dee_type_member_set_impl(desc, self, value);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_type_member_init_unbound(struct type_member const *desc, DeeObject *self) {
	byte_t *dst = (byte_t *)self + desc->m_desc.md_field.mdf_offset;
	switch (desc->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST:
		*(DeeObject **)dst = NULL;
		break;
	case STRUCT_WOBJECT_OPT:
		Dee_weakref_initempty((struct weakref *)dst);
		break;
	case STRUCT_VARIANT:
		Dee_variant_init_unbound((struct Dee_variant *)dst);
		break;
	default:
		return Dee_type_member_set_impl(desc, self, Dee_None);
	}
	return 0;
}

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_init_cb(void *arg, DeeTypeObject *declaring_type,
               struct type_member const *field) {
	DeeObject *initializer;
	struct struct_init_data *data = (struct struct_init_data *)arg;
	(void)declaring_type;
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	/* Load initializer argument (starting with positional arguments,
	 * and when those are exhausted, moving on to keyword arguments)
	 *
	 * -> When there are no more arguments "initializer" is set to ITER_DONE
	 */
	if (data->sid_posargc) {
		initializer = *data->sid_posargv++;
		--data->sid_posargc;
	} else {
		initializer = DeeKwArgs_TryGetItemNRString(&data->sid_kwargs, field->m_name);
		if (!ITER_ISOK(initializer)) {
			if unlikely(!initializer)
				return -1;
			return Dee_type_member_init_unbound(field, data->sid_self);
		}
	}
	ASSERT(ITER_ISOK(initializer));
	return Dee_type_member_init(field, data->sid_self, initializer);
}

PRIVATE NONNULL((2, 3)) void DCALL
struct_init_undo_cb(void *arg, DeeTypeObject *declaring_type,
                    struct type_member const *field) {
	struct struct_init_data *data = (struct struct_init_data *)arg;
	struct_fini_cb(data->sid_self, declaring_type, field);
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeStructObject_InitKw(DeeObject *__restrict self,
                       size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	struct struct_init_data data;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(DeeKwArgs_Init(&data.sid_kwargs, &argc, argv, kw))
		goto err;
	data.sid_self    = self;
	data.sid_posargc = argc;
	data.sid_posargv = argv;
	if unlikely(DeeStructObject_ForeachField(tp_self, &struct_init_cb,
	                                         &struct_init_undo_cb, &data))
		goto err;
	result = DeeKwArgs_Done(&data.sid_kwargs, argc, DeeType_GetName(tp_self));
	if unlikely(result)
		struct_fini_all(self);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeStructObject_Init(DeeObject *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	return DeeStructObject_InitKw(self, argc, argv, NULL);
}

struct struct_serialize_data {
	DeeObject    *scd_src;
	DeeSerial    *scd_dst_writer;
	Dee_seraddr_t scd_dst_addr;
};


PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_serialize_cb(void *arg, DeeTypeObject *declaring_type,
                    struct type_member const *field) {
	struct struct_serialize_data *data = (struct struct_serialize_data *)arg;
	Dee_seraddr_t dst_addr = data->scd_dst_addr + field->m_desc.md_field.mdf_offset;
	byte_t *src = (byte_t *)data->scd_src + field->m_desc.md_field.mdf_offset;
	byte_t *dst = DeeSerial_Addr2Mem(data->scd_dst_writer, dst_addr, byte_t);
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	(void)declaring_type;
	switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST: {
		DeeObject *obj = *(DeeObject *const *)src;
		return DeeSerial_XPutObject(data->scd_dst_writer, dst_addr, obj);
	}	break;
	case STRUCT_WOBJECT_OPT:
	case STRUCT_WOBJECT: {
		struct Dee_weakref *wref = (struct Dee_weakref *)src;
		return DeeSerial_PutWeakref(data->scd_dst_writer, dst_addr, wref);
	}	break;
	case STRUCT_VARIANT:
		return Dee_variant_serialize((struct Dee_variant *)src, data->scd_dst_writer, dst_addr);
	case STRUCT_CHAR:
	case STRUCT_BOOLBIT0:
	case STRUCT_BOOLBIT1:
	case STRUCT_BOOLBIT2:
	case STRUCT_BOOLBIT3:
	case STRUCT_BOOLBIT4:
	case STRUCT_BOOLBIT5:
	case STRUCT_BOOLBIT6:
	case STRUCT_BOOLBIT7:
	case STRUCT_BOOL8:
	case STRUCT_INT8:
	case STRUCT_UNSIGNED | STRUCT_INT8:
		*dst = *src;
		break;
	case STRUCT_BOOL16:
	case STRUCT_INT16:
	case STRUCT_UNSIGNED | STRUCT_INT16:
		memcpy(dst, src, 2);
		break;
	case STRUCT_BOOL32:
	case STRUCT_INT32:
	case STRUCT_UNSIGNED | STRUCT_INT32:
		memcpy(dst, src, 4);
		break;
	case STRUCT_BOOL64:
	case STRUCT_INT64:
	case STRUCT_UNSIGNED | STRUCT_INT64:
		memcpy(dst, src, 8);
		break;
	case STRUCT_INT128:
	case STRUCT_UNSIGNED | STRUCT_INT128:
		memcpy(dst, src, 16);
		break;
	case STRUCT_FLOAT:
		memcpy(dst, src, __SIZEOF_FLOAT__);
		break;
	case STRUCT_DOUBLE:
		memcpy(dst, src, __SIZEOF_DOUBLE__);
		break;
	case STRUCT_LDOUBLE:
		memcpy(dst, src, __SIZEOF_LONG_DOUBLE__);
		break;
	case STRUCT_VOID:
		break;
	default: return DeeError_NOTIMPLEMENTED();
	}
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeStructObject_Serialize(DeeObject *__restrict self,
                          struct Dee_serial *__restrict writer,
                          Dee_seraddr_t addr) {
	struct struct_serialize_data data;
	data.scd_dst_addr   = addr;
	data.scd_dst_writer = writer;
	data.scd_src        = self;
	return (int)DeeStructObject_ForeachField(Dee_TYPE(self),
	                                         &struct_serialize_cb,
	                                         NULL, &data);
}


PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_init_unbound_cb(void *arg, DeeTypeObject *declaring_type,
                       struct type_member const *field) {
	(void)declaring_type;
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	return Dee_type_member_init_unbound(field, (DeeObject *)arg);
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeStructObject_Ctor(DeeObject *__restrict self) {
	return (int)DeeStructObject_ForeachField(Dee_TYPE(self),
	                                         &struct_init_unbound_cb,
	                                         &struct_fini_cb, self);
}

PUBLIC NONNULL((1)) void DCALL /* Remember to set "Dee_TF_TPVISIT" */
DeeStructObject_Visit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                      Dee_visit_t proc, void *arg) {
	struct type_member const *member;
#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
	struct Dee_type_member const *const *fields;
	if likely((fields = Dee_type_struct_getlocfields(tp_self)) != NULL) {
		while ((member = *fields++) != NULL) {
			byte_t *dst = (byte_t *)self + member->m_desc.md_field.mdf_offset;
			switch (member->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
			case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
			case STRUCT_OBJECT & ~STRUCT_CONST:
				Dee_XVisit(*(DREF DeeObject **)dst);
				break;
			case STRUCT_VARIANT:
				Dee_variant_visit((struct Dee_variant *)dst, proc, arg);
				break;
			default: break;
			}
		}
		return;
	}
#endif /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */
	if ((member = tp_self->tp_members) != NULL) {
		DeeTypeObject *tp_base = DeeType_Base(tp_self);
		for (; member->m_name; ++member) {
			byte_t *dst;
			if (!TYPE_MEMBER_ISFIELD(member))
				continue;
			dst = (byte_t *)self + member->m_desc.md_field.mdf_offset;
			switch (member->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
			case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
			case STRUCT_OBJECT & ~STRUCT_CONST:
				if (!struct_offset_exists_r(tp_base, member->m_desc.md_field.mdf_offset))
					Dee_XVisit(*(DREF DeeObject **)dst);
				break;
			case STRUCT_VARIANT:
				if (!struct_offset_exists_r(tp_base, member->m_desc.md_field.mdf_offset))
					Dee_variant_visit((struct Dee_variant *)dst, proc, arg);
				break;
			default: break;
			}
		}
	}
}

PUBLIC NONNULL((1)) void DCALL /* Only finalizes fields defined by "Dee_TYPE(self)" */
DeeStructObject_Fini(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	struct type_member const *member;
#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
	struct Dee_type_member const *const *fields;
	if likely((fields = Dee_type_struct_getlocfields(tp_self)) != NULL) {
		while ((member = *fields++) != NULL)
			struct_fini_cb(self, tp_self, member);
		return;
	}
#endif /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */
	if ((member = tp_self->tp_members) != NULL) {
		DeeTypeObject *tp_base = DeeType_Base(tp_self);
		for (; member->m_name; ++member) {
			if (!TYPE_MEMBER_ISFIELD(member))
				continue;
			if (struct_offset_exists_r(tp_base, member->m_desc.md_field.mdf_offset))
				continue;
			struct_fini_cb(self, tp_self, member);
		}
	}
}


struct struct_printrepr_data {
	DeeObject          *spr_self;    /* [1..1] Object being printed */
	Dee_formatprinter_t spr_printer; /* [1..1] Printer callback */
	void               *spr_arg;     /* [?..?] Cookie for `spr_printer' */
	bool                spr_first;   /* Is this the first argument being printed? */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
struct_printrepr_prefix(struct struct_printrepr_data *data,
                        struct type_member const *field) {
	Dee_ssize_t temp, result;
	if (data->spr_first) {
		data->spr_first = false;
		result = 0;
	} else {
		result = DeeFormat_PRINT(data->spr_printer, data->spr_arg, ", ");
		if unlikely(result < 0)
			goto done;
	}
	temp = DeeFormat_PrintStr(data->spr_printer, data->spr_arg, field->m_name);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(data->spr_printer, data->spr_arg, ": ");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_printrepr_cb(void *arg, DeeTypeObject *declaring_type,
                    struct type_member const *field) {
	Dee_ssize_t result, temp;
	struct struct_printrepr_data *data = (struct struct_printrepr_data *)arg;
	byte_t *src = (byte_t *)data->spr_self + field->m_desc.md_field.mdf_offset;
	(void)declaring_type;
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST: {
		DeeObject *value = *(DeeObject *const *)src;
		if (!value)
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if unlikely(result < 0)
			goto done;
		temp = DeeObject_PrintRepr(value, data->spr_printer, data->spr_arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

	case STRUCT_WOBJECT_OPT:
	case STRUCT_WOBJECT: {
		DREF DeeObject *value;
		value = Dee_weakref_lock((struct weakref *)src);
		if (!value)
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if likely(result >= 0) {
			temp = DeeObject_PrintRepr(value, data->spr_printer, data->spr_arg);
			if unlikely(temp < 0) {
				result = temp;
			} else {
				result += temp;
			}
		}
		Dee_Decref(value);
	}	break;

	case STRUCT_VARIANT: {
		struct Dee_variant value;
		result = 0;
		Dee_variant_init_copy(&value, (struct Dee_variant *)src);
		if (Dee_variant_isbound_nonatomic(&value)) {
			result = struct_printrepr_prefix(data, field);
			if likely(result >= 0) {
				temp = Dee_variant_printrepr(&value, data->spr_printer, data->spr_arg);
				if unlikely(temp < 0) {
					result = temp;
				} else {
					result += temp;
				}
			}
		}
		Dee_variant_fini(&value);
	}	break;

	case STRUCT_CHAR: {
		char value = *(char const *)src;
		if (!value)
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if unlikely(result < 0)
			goto done;
		temp = DeeFormat_Printf(data->spr_printer, data->spr_arg, "%$q", (size_t)1, &value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

	case STRUCT_BOOLBIT0:
	case STRUCT_BOOLBIT1:
	case STRUCT_BOOLBIT2:
	case STRUCT_BOOLBIT3:
	case STRUCT_BOOLBIT4:
	case STRUCT_BOOLBIT5:
	case STRUCT_BOOLBIT6:
	case STRUCT_BOOLBIT7:
	case STRUCT_BOOL8:
	case STRUCT_BOOL16:
	case STRUCT_BOOL32:
	case STRUCT_BOOL64: {
		bool value;
		switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
		case STRUCT_BOOLBIT0:
		case STRUCT_BOOLBIT1:
		case STRUCT_BOOLBIT2:
		case STRUCT_BOOLBIT3:
		case STRUCT_BOOLBIT4:
		case STRUCT_BOOLBIT5:
		case STRUCT_BOOLBIT6:
		case STRUCT_BOOLBIT7: {
			byte_t mask = STRUCT_BOOLBITMASK(field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC));
			value = (*src & mask) != 0;
		}	break;
		case STRUCT_BOOL8:
			value = *(uint8_t const *)src != 0;
			break;
		case STRUCT_BOOL16:
			value = UNALIGNED_GET16((uint16_t const *)src) != 0;
			break;
		case STRUCT_BOOL32:
			value = UNALIGNED_GET32((uint32_t const *)src) != 0;
			break;
		case STRUCT_BOOL64:
			value = UNALIGNED_GET64((uint64_t const *)src) != 0;
			break;
		default: __builtin_unreachable();
		}
		if (!value)
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if unlikely(result < 0)
			goto done;
		temp = DeeFormat_PRINT(data->spr_printer, data->spr_arg, "true");
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

	case STRUCT_INT8:
	case STRUCT_INT16:
	case STRUCT_INT32:
	case STRUCT_INT64:
	case STRUCT_INT128:
	case STRUCT_UNSIGNED | STRUCT_INT8:
	case STRUCT_UNSIGNED | STRUCT_INT16:
	case STRUCT_UNSIGNED | STRUCT_INT32:
	case STRUCT_UNSIGNED | STRUCT_INT64:
	case STRUCT_UNSIGNED | STRUCT_INT128: {
		Dee_uint128_t value;
		switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST |
		                                            STRUCT_ATOMIC |
		                                            STRUCT_UNSIGNED)) {
		case STRUCT_INT8:
			__hybrid_uint128_set8(value, *(uint8_t const *)src);
			break;
		case STRUCT_INT16:
			__hybrid_uint128_set16(value, UNALIGNED_GET16((uint16_t const *)src));
			break;
		case STRUCT_INT32:
			__hybrid_uint128_set32(value, UNALIGNED_GET32((uint32_t const *)src));
			break;
		case STRUCT_INT64:
			__hybrid_uint128_set64(value, UNALIGNED_GET64((uint64_t const *)src));
			break;
		case STRUCT_INT128:
			memcpy(&value, src, sizeof(value));
			break;
		default: __builtin_unreachable();
		}
		if (__hybrid_uint128_iszero(value))
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if unlikely(result < 0)
			goto done;
		temp = DeeFormat_Printf(data->spr_printer, data->spr_arg,
		                        (field->m_desc.md_field.mdf_type & STRUCT_UNSIGNED)
		                        ? "%" PRFu128
		                        : "%" PRFd128,
		                        value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

#ifndef CONFIG_NO_FPU
	case STRUCT_FLOAT:
	case STRUCT_DOUBLE: {
		double value;
		if ((field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) == STRUCT_FLOAT) {
			float fvalue;
			memcpy(&fvalue, src, __SIZEOF_FLOAT__);
			value = (double)fvalue;
		} else {
			memcpy(&value, src, __SIZEOF_DOUBLE__);
		}
		if (value == 0.0)
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if unlikely(result < 0)
			goto done;
		temp = DeeFormat_Printf(data->spr_printer, data->spr_arg, "%f", value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

	case STRUCT_LDOUBLE: {
		__LONGDOUBLE value;
		memcpy(&value, src, __SIZEOF_LONG_DOUBLE__);
		if (value == 0.0L)
			return 0; /* Unbound */
		result = struct_printrepr_prefix(data, field);
		if unlikely(result < 0)
			goto done;
		temp = DeeFormat_Printf(data->spr_printer, data->spr_arg, "%Lf", value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;
#endif /* !CONFIG_NO_FPU */

	case STRUCT_VOID:
		return 0;

	default: {
		/* Generic fallback handling (inefficient, but works for all cases) */
		DREF DeeObject *value;
		value = Dee_type_member_tryget(field, data->spr_self);
		if unlikely(!ITER_ISOK(value)) {
			if (value == ITER_DONE)
				return 0; /* Unbound */
			goto err;
		}
		result = struct_printrepr_prefix(data, field);
		if likely(result >= 0) {
			temp = DeeObject_PrintRepr(value, data->spr_printer, data->spr_arg);
			if unlikely(temp < 0) {
				result = temp;
			} else {
				result += temp;
			}
		}
		Dee_Decref(value);
	}	break;

	}
done:
	return result;
err_temp:
	return temp;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeStructObject_PrintRepr(DeeObject *__restrict self,
                          Dee_formatprinter_t printer,
                          void *arg) {
	Dee_ssize_t temp, result;
	struct struct_printrepr_data data;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	result = DeeFormat_Printf(printer, arg, "%s(", DeeType_GetName(tp_self));
	if unlikely(result < 0)
		goto done;
	data.spr_self    = self;
	data.spr_printer = printer;
	data.spr_arg     = arg;
	data.spr_first   = true;
	temp = DeeStructObject_ForeachField(tp_self, &struct_printrepr_cb, NULL, &data);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, ")");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}



struct struct_hash_data {
	DeeObject *spr_self;   /* [1..1] Object being printed */
	Dee_hash_t spr_result; /* Resulting hash */
	bool       spr_first;  /* At first member? */
};

#ifndef CONFIG_NO_FPU
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL
float_hash(DeeFloatObject *__restrict self);
#endif /* !CONFIG_NO_FPU */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL
struct_hashof_field(DeeObject *self, struct type_member const *field) {
	byte_t *src = (byte_t *)self + field->m_desc.md_field.mdf_offset;
	ASSERT(TYPE_MEMBER_ISFIELD(field));
	switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
	case STRUCT_OBJECT_OPT & ~STRUCT_CONST:
	case STRUCT_OBJECT & ~STRUCT_CONST: {
		DeeObject *obj = *(DeeObject *const *)src;
		if (obj == NULL)
			return DEE_HASHOF_UNBOUND_ITEM;
		return DeeObject_Hash(obj);
	}	break;
	case STRUCT_WOBJECT_OPT:
	case STRUCT_WOBJECT: {
		DREF DeeObject *obj = Dee_weakref_lock((struct weakref *)src);
		if (obj == NULL)
			return DEE_HASHOF_UNBOUND_ITEM;
		return DeeObject_HashInherited(obj);
	}	break;

	case STRUCT_VARIANT:
		return Dee_variant_hash((struct Dee_variant *)src);

	case STRUCT_CHAR:
		return Dee_HashPtr(src, 1);

	case STRUCT_BOOLBIT0:
	case STRUCT_BOOLBIT1:
	case STRUCT_BOOLBIT2:
	case STRUCT_BOOLBIT3:
	case STRUCT_BOOLBIT4:
	case STRUCT_BOOLBIT5:
	case STRUCT_BOOLBIT6:
	case STRUCT_BOOLBIT7:
	case STRUCT_BOOL8:
	case STRUCT_BOOL16:
	case STRUCT_BOOL32:
	case STRUCT_BOOL64: {
		bool value;
		switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
		case STRUCT_BOOLBIT0:
		case STRUCT_BOOLBIT1:
		case STRUCT_BOOLBIT2:
		case STRUCT_BOOLBIT3:
		case STRUCT_BOOLBIT4:
		case STRUCT_BOOLBIT5:
		case STRUCT_BOOLBIT6:
		case STRUCT_BOOLBIT7: {
			byte_t mask = STRUCT_BOOLBITMASK(field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC));
			value = (*src & mask) != 0;
		}	break;
		case STRUCT_BOOL8:
			value = *(uint8_t const *)src != 0;
			break;
		case STRUCT_BOOL16:
			value = UNALIGNED_GET16((uint16_t const *)src) != 0;
			break;
		case STRUCT_BOOL32:
			value = UNALIGNED_GET32((uint32_t const *)src) != 0;
			break;
		case STRUCT_BOOL64:
			value = UNALIGNED_GET64((uint64_t const *)src) != 0;
			break;
		default: __builtin_unreachable();
		}
		return value ? 1 : 0;
	}	break;

	case STRUCT_INT8:
		return (Dee_hash_t)(*(int8_t const *)src);
	case STRUCT_UNSIGNED | STRUCT_INT8:
		return (Dee_hash_t)(*(uint8_t const *)src);
	case STRUCT_INT16:
		return (Dee_hash_t)(*(int16_t const *)src);
	case STRUCT_UNSIGNED | STRUCT_INT16:
		return (Dee_hash_t)(*(uint16_t const *)src);
	case STRUCT_INT32:
		return (Dee_hash_t)(*(int32_t const *)src);
	case STRUCT_UNSIGNED | STRUCT_INT32:
		return (Dee_hash_t)(*(uint32_t const *)src);
	case STRUCT_INT64:
		return (Dee_hash_t)(*(int64_t const *)src);
	case STRUCT_UNSIGNED | STRUCT_INT64:
		return (Dee_hash_t)(*(uint64_t const *)src);

	case STRUCT_INT128:
	case STRUCT_UNSIGNED | STRUCT_INT128: {
		Dee_int128_t value;
		memcpy(&value, src, 16);
#if Dee_SIZEOF_HASH_T <= 4
		return (Dee_hash_t)__hybrid_int128_get32(value);
#else /* Dee_SIZEOF_HASH_T <= 4 */
		return (Dee_hash_t)__hybrid_int128_get64(value);
#endif /* Dee_SIZEOF_HASH_T > 4 */
	}	break;

#ifndef CONFIG_NO_FPU
	case STRUCT_FLOAT:
	case STRUCT_DOUBLE:
	case STRUCT_LDOUBLE: {
		double value;
		DeeFloatObject *obj;
		switch (field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
		case STRUCT_FLOAT: {
			float fvalue;
			memcpy(&fvalue, src, __SIZEOF_FLOAT__);
			value = (double)fvalue;
		}	break;
		case STRUCT_DOUBLE:
			memcpy(&value, src, __SIZEOF_DOUBLE__);
			break;
		case STRUCT_LDOUBLE: {
			__LONGDOUBLE lvalue;
			memcpy(&lvalue, src, __SIZEOF_LONG_DOUBLE__);
			value = (double)lvalue;
		}	break;
		default: __builtin_unreachable();
		}
		obj = COMPILER_CONTAINER_OF(&value, DeeFloatObject, f_value);
		return float_hash(obj);
	}	break;
#endif /* !CONFIG_NO_FPU */

	default: break;
	}
	return DEE_HASHOF_UNBOUND_ITEM;
}

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_hash_cb(void *arg, DeeTypeObject *declaring_type,
               struct type_member const *field) {
	Dee_hash_t hashof_field;
	struct struct_hash_data *data = (struct struct_hash_data *)arg;
	(void)declaring_type;
	hashof_field = struct_hashof_field(data->spr_self, field);
	if (data->spr_first) {
		data->spr_result = hashof_field;
		data->spr_first = false;
	} else {
		data->spr_result = Dee_HashCombine(data->spr_result, hashof_field);
	}
	return 0;
}

PUBLIC WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeStructObject_Hash(DeeObject *__restrict self) {
	struct struct_hash_data data;
	data.spr_self  = self;
	data.spr_first = true;
	DeeStructObject_ForeachField(Dee_TYPE(self), &struct_hash_cb, NULL, &data);
	if unlikely(data.spr_first)
		return DeeObject_HashGeneric(self);
	return data.spr_result;
}


struct struct_compare_data {
	DeeObject *scd_lhs; /* [1..1] Left side */
	DeeObject *scd_rhs; /* [1..1] Right side */
};


/* Fast-pass comparison function that only implements cases that
 * can never throw an exception. Returns "Dee_COMPARE_ERR" if the
 * variant-type case between "lhs" and "rhs" doesn't have a fast-
 * pass, or cannot be done without ever throwing an exception.
 *
 * @return: Dee_COMPARE_LO:  lhs < rhs
 * @return: Dee_COMPARE_EQ:  Equal
 * @return: Dee_COMPARE_GR:  lhs > rhs
 * @return: Dee_COMPARE_ERR: Fast comparison isn't possible */
PRIVATE WUNUSED NONNULL((2, 3)) int DCALL
struct_compare_fast_impl(uintptr_half_t type,
                         byte_t const *lhs,
                         byte_t const *rhs) {
	switch (type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {

	case STRUCT_BOOLBIT0:
	case STRUCT_BOOLBIT1:
	case STRUCT_BOOLBIT2:
	case STRUCT_BOOLBIT3:
	case STRUCT_BOOLBIT4:
	case STRUCT_BOOLBIT5:
	case STRUCT_BOOLBIT6:
	case STRUCT_BOOLBIT7:
	case STRUCT_BOOL8:
	case STRUCT_BOOL16:
	case STRUCT_BOOL32:
	case STRUCT_BOOL64: {
		bool lhs_value;
		bool rhs_value;
		switch (type & ~(STRUCT_CONST | STRUCT_ATOMIC)) {
		case STRUCT_BOOLBIT0:
		case STRUCT_BOOLBIT1:
		case STRUCT_BOOLBIT2:
		case STRUCT_BOOLBIT3:
		case STRUCT_BOOLBIT4:
		case STRUCT_BOOLBIT5:
		case STRUCT_BOOLBIT6:
		case STRUCT_BOOLBIT7: {
			byte_t mask = STRUCT_BOOLBITMASK(type & ~(STRUCT_CONST | STRUCT_ATOMIC));
			lhs_value = (*lhs & mask) != 0;
			rhs_value = (*rhs & mask) != 0;
		}	break;
		case STRUCT_BOOL8:
			lhs_value = *(uint8_t const *)lhs != 0;
			rhs_value = *(uint8_t const *)rhs != 0;
			break;
		case STRUCT_BOOL16:
			lhs_value = UNALIGNED_GET16((uint16_t const *)lhs) != 0;
			rhs_value = UNALIGNED_GET16((uint16_t const *)rhs) != 0;
			break;
		case STRUCT_BOOL32:
			lhs_value = UNALIGNED_GET32((uint32_t const *)lhs) != 0;
			rhs_value = UNALIGNED_GET32((uint32_t const *)rhs) != 0;
			break;
		case STRUCT_BOOL64:
			lhs_value = UNALIGNED_GET64((uint64_t const *)lhs) != 0;
			rhs_value = UNALIGNED_GET64((uint64_t const *)rhs) != 0;
			break;
		default: __builtin_unreachable();
		}
		return Dee_Compare(lhs_value, rhs_value);
	}	break;

#define RETURN_COMPARE(T) \
	Dee_return_compareT(T, *(T const *)lhs, *(T const *)rhs)

	case STRUCT_INT8:
		RETURN_COMPARE(int8_t);
	case STRUCT_CHAR:
	case STRUCT_UNSIGNED | STRUCT_INT8:
		RETURN_COMPARE(uint8_t);
	case STRUCT_INT16:
		RETURN_COMPARE(int16_t);
	case STRUCT_UNSIGNED | STRUCT_INT16:
		RETURN_COMPARE(uint16_t);
	case STRUCT_INT32:
		RETURN_COMPARE(int32_t);
	case STRUCT_UNSIGNED | STRUCT_INT32:
		RETURN_COMPARE(uint32_t);
	case STRUCT_INT64:
		RETURN_COMPARE(int64_t);
	case STRUCT_UNSIGNED | STRUCT_INT64:
		RETURN_COMPARE(uint64_t);

	case STRUCT_INT128: {
		Dee_int128_t lhs_value;
		Dee_int128_t rhs_value;
		memcpy(&lhs_value, lhs, sizeof(lhs_value));
		memcpy(&rhs_value, rhs, sizeof(rhs_value));
		if (__hybrid_int128_lo128(lhs_value, rhs_value))
			return Dee_COMPARE_LO;
		if (__hybrid_int128_gr128(lhs_value, rhs_value))
			return Dee_COMPARE_GR;
		return Dee_COMPARE_EQ;
	}	break;

	case STRUCT_UNSIGNED | STRUCT_INT128: {
		Dee_uint128_t lhs_value;
		Dee_uint128_t rhs_value;
		memcpy(&lhs_value, lhs, sizeof(lhs_value));
		memcpy(&rhs_value, rhs, sizeof(rhs_value));
		if (__hybrid_uint128_lo128(lhs_value, rhs_value))
			return Dee_COMPARE_LO;
		if (__hybrid_uint128_gr128(lhs_value, rhs_value))
			return Dee_COMPARE_GR;
		return Dee_COMPARE_EQ;
	}	break;

#ifndef CONFIG_NO_FPU
	case STRUCT_FLOAT:
		RETURN_COMPARE(float);
	case STRUCT_DOUBLE:
		RETURN_COMPARE(double);
	case STRUCT_LDOUBLE:
		RETURN_COMPARE(__LONGDOUBLE);
#endif /* !CONFIG_NO_FPU */

#undef RETURN_COMPARE

	case STRUCT_VOID:
		return Dee_COMPARE_EQ;

	default: break;
	}
	return Dee_COMPARE_ERR;
}

PRIVATE NONNULL((1, 2)) int DCALL
struct_compare_cb_impl(struct struct_compare_data *data,
                       struct type_member const *field) {
	int result;
	byte_t *lhs = (byte_t *)data->scd_lhs + field->m_desc.md_field.mdf_offset;
	byte_t *rhs = (byte_t *)data->scd_rhs + field->m_desc.md_field.mdf_offset;
	DREF DeeObject *lhs_ob, *rhs_ob;
	result = struct_compare_fast_impl(field->m_desc.md_field.mdf_type, lhs, rhs);
	if (result != Dee_COMPARE_ERR)
		return result;
	if ((field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) == STRUCT_VARIANT)
		return Dee_variant_compare((struct Dee_variant *)lhs, (struct Dee_variant *)rhs);
	lhs_ob = Dee_type_member_tryget(field, data->scd_lhs);
	if unlikely(!lhs_ob)
		goto err;
	rhs_ob = Dee_type_member_tryget(field, data->scd_rhs);
	if unlikely(!rhs_ob)
		goto err_lhs_ob;
	if (lhs_ob == ITER_DONE) {
		if (rhs_ob == ITER_DONE) {
			result = Dee_COMPARE_EQ;
		} else {
			result = Dee_COMPARE_LO; /* UNBOUND < BOUND */
			Dee_Decref(rhs_ob);
		}
	} else if (rhs_ob == ITER_DONE) {
		result = Dee_COMPARE_GR; /* BOUND > UNBOUND */
		Dee_Decref(lhs_ob);
	} else {
		result = DeeObject_Compare(lhs_ob, rhs_ob);
		Dee_Decref(rhs_ob);
		Dee_Decref(lhs_ob);
	}
	return result;
err_lhs_ob:
	if (lhs_ob != ITER_DONE)
		Dee_Decref(lhs_ob);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE NONNULL((1, 2)) int DCALL
struct_compare_eq_cb_impl(struct struct_compare_data *data,
                          struct type_member const *field) {
	int result;
	byte_t *lhs = (byte_t *)data->scd_lhs + field->m_desc.md_field.mdf_offset;
	byte_t *rhs = (byte_t *)data->scd_rhs + field->m_desc.md_field.mdf_offset;
	DREF DeeObject *lhs_ob, *rhs_ob;
	result = struct_compare_fast_impl(field->m_desc.md_field.mdf_type, lhs, rhs);
	if (result != Dee_COMPARE_ERR)
		return result;
	if ((field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) == STRUCT_VARIANT)
		return Dee_variant_compare_eq((struct Dee_variant *)lhs, (struct Dee_variant *)rhs);
	lhs_ob = Dee_type_member_tryget(field, data->scd_lhs);
	if unlikely(!lhs_ob)
		goto err;
	rhs_ob = Dee_type_member_tryget(field, data->scd_rhs);
	if unlikely(!rhs_ob)
		goto err_lhs_ob;
	if (lhs_ob == ITER_DONE) {
		if (rhs_ob == ITER_DONE) {
			result = Dee_COMPARE_EQ;
		} else {
			result = Dee_COMPARE_LO; /* UNBOUND < BOUND */
			Dee_Decref(rhs_ob);
		}
	} else if (rhs_ob == ITER_DONE) {
		result = Dee_COMPARE_GR; /* BOUND > UNBOUND */
		Dee_Decref(lhs_ob);
	} else {
		result = DeeObject_CompareEq(lhs_ob, rhs_ob);
		Dee_Decref(rhs_ob);
		Dee_Decref(lhs_ob);
	}
	return result;
err_lhs_ob:
	if (lhs_ob != ITER_DONE)
		Dee_Decref(lhs_ob);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE NONNULL((1, 2)) int DCALL
struct_trycompare_eq_cb_impl(struct struct_compare_data *data,
                             struct type_member const *field) {
	int result;
	byte_t *lhs = (byte_t *)data->scd_lhs + field->m_desc.md_field.mdf_offset;
	byte_t *rhs = (byte_t *)data->scd_rhs + field->m_desc.md_field.mdf_offset;
	DREF DeeObject *lhs_ob, *rhs_ob;
	result = struct_compare_fast_impl(field->m_desc.md_field.mdf_type, lhs, rhs);
	if (result != Dee_COMPARE_ERR)
		return result;
	if ((field->m_desc.md_field.mdf_type & ~(STRUCT_CONST | STRUCT_ATOMIC)) == STRUCT_VARIANT)
		return Dee_variant_trycompare_eq((struct Dee_variant *)lhs, (struct Dee_variant *)rhs);
	lhs_ob = Dee_type_member_tryget(field, data->scd_lhs);
	if unlikely(!lhs_ob)
		goto err;
	rhs_ob = Dee_type_member_tryget(field, data->scd_rhs);
	if unlikely(!rhs_ob)
		goto err_lhs_ob;
	if (lhs_ob == ITER_DONE) {
		if (rhs_ob == ITER_DONE) {
			result = Dee_COMPARE_EQ;
		} else {
			result = Dee_COMPARE_LO; /* UNBOUND < BOUND */
			Dee_Decref(rhs_ob);
		}
	} else if (rhs_ob == ITER_DONE) {
		result = Dee_COMPARE_GR; /* BOUND > UNBOUND */
		Dee_Decref(lhs_ob);
	} else {
		result = DeeObject_TryCompareEq(lhs_ob, rhs_ob);
		Dee_Decref(rhs_ob);
		Dee_Decref(lhs_ob);
	}
	return result;
err_lhs_ob:
	if (lhs_ob != ITER_DONE)
		Dee_Decref(lhs_ob);
err:
	return Dee_COMPARE_ERR;
}

#define STRUCT_CMP_ENCODE(v) ((Dee_ssize_t)((v) == 0 ? 0 : ((v) - 4)))
#define STRUCT_CMP_DECODE(v) ((int)((v) == 0 ? 0 : ((v) + 4)))
STATIC_ASSERT(STRUCT_CMP_DECODE(0) == Dee_COMPARE_EQ);
STATIC_ASSERT(STRUCT_CMP_ENCODE(Dee_COMPARE_EQ) == 0);
STATIC_ASSERT(STRUCT_CMP_ENCODE(Dee_COMPARE_LO) < 0);
STATIC_ASSERT(STRUCT_CMP_ENCODE(Dee_COMPARE_GR) < 0);
STATIC_ASSERT(STRUCT_CMP_ENCODE(Dee_COMPARE_ERR) < 0);
STATIC_ASSERT(STRUCT_CMP_DECODE(STRUCT_CMP_ENCODE(Dee_COMPARE_EQ)) == Dee_COMPARE_EQ);
STATIC_ASSERT(STRUCT_CMP_DECODE(STRUCT_CMP_ENCODE(Dee_COMPARE_NE)) == Dee_COMPARE_NE);
STATIC_ASSERT(STRUCT_CMP_DECODE(STRUCT_CMP_ENCODE(Dee_COMPARE_GR)) == Dee_COMPARE_GR);
STATIC_ASSERT(STRUCT_CMP_DECODE(STRUCT_CMP_ENCODE(Dee_COMPARE_LO)) == Dee_COMPARE_LO);
STATIC_ASSERT(STRUCT_CMP_DECODE(STRUCT_CMP_ENCODE(Dee_COMPARE_ERR)) == Dee_COMPARE_ERR);

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_compare_cb(void *arg, DeeTypeObject *declaring_type,
                  struct type_member const *field) {
	struct struct_compare_data *data = (struct struct_compare_data *)arg;
	int result = struct_compare_cb_impl(data, field);
	(void)declaring_type;
	return STRUCT_CMP_ENCODE(result);
}

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_compare_eq_cb(void *arg, DeeTypeObject *declaring_type,
                     struct type_member const *field) {
	struct struct_compare_data *data = (struct struct_compare_data *)arg;
	int result = struct_compare_eq_cb_impl(data, field);
	(void)declaring_type;
	return STRUCT_CMP_ENCODE(result);
}

PRIVATE NONNULL((2, 3)) Dee_ssize_t DCALL
struct_trycompare_eq_cb(void *arg, DeeTypeObject *declaring_type,
                        struct type_member const *field) {
	struct struct_compare_data *data = (struct struct_compare_data *)arg;
	int result = struct_trycompare_eq_cb_impl(data, field);
	(void)declaring_type;
	return STRUCT_CMP_ENCODE(result);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeStructObject_Compare(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t result;
	DeeTypeObject *tp_lhs = Dee_TYPE(lhs);
	struct struct_compare_data data;
	if (DeeObject_AssertTypeExact(rhs, tp_lhs))
		goto err;
	data.scd_lhs = lhs;
	data.scd_rhs = rhs;
	result = DeeStructObject_ForeachField(tp_lhs, &struct_compare_cb, NULL, &data);
	return STRUCT_CMP_DECODE(result);
err:
	return Dee_COMPARE_ERR;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeStructObject_CompareEq(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t result;
	DeeTypeObject *tp_lhs = Dee_TYPE(lhs);
	struct struct_compare_data data;
	if (DeeObject_AssertTypeExact(rhs, tp_lhs))
		goto err;
	data.scd_lhs = lhs;
	data.scd_rhs = rhs;
	result = DeeStructObject_ForeachField(tp_lhs, &struct_compare_eq_cb, NULL, &data);
	return STRUCT_CMP_DECODE(result);
err:
	return Dee_COMPARE_ERR;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeStructObject_TryCompareEq(DeeObject *lhs, DeeObject *rhs) {
	Dee_ssize_t result;
	DeeTypeObject *tp_lhs = Dee_TYPE(lhs);
	struct struct_compare_data data;
	if (!DeeObject_InstanceOfExact(rhs, tp_lhs))
		return Dee_COMPARE_NE;
	data.scd_lhs = lhs;
	data.scd_rhs = rhs;
	result = DeeStructObject_ForeachField(tp_lhs, &struct_trycompare_eq_cb, NULL, &data);
	return STRUCT_CMP_DECODE(result);
}



PUBLIC struct type_cmp DeeStructObject_Cmp = {
	/* .tp_hash          = */ &DeeStructObject_Hash,
	/* .tp_compare_eq    = */ &DeeStructObject_CompareEq,
	/* .tp_compare       = */ &DeeStructObject_Compare,
	/* .tp_trycompare_eq = */ &DeeStructObject_TryCompareEq,
#ifndef __pic__ /* These also get auto-filled in, so we don't need relocations */
	/* .tp_eq = */ &default__eq__with__compare_eq,
	/* .tp_ne = */ &default__ne__with__compare_eq,
	/* .tp_lo = */ &default__lo__with__compare,
	/* .tp_le = */ &default__le__with__compare,
	/* .tp_gr = */ &default__gr__with__compare,
	/* .tp_ge = */ &default__ge__with__compare,
#endif /* !__pic__ */
};

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_STRUCT_C */
