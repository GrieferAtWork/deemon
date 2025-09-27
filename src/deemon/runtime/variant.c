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
#ifndef GUARD_DEEMON_RUNTIME_VARIANT_C
#define GUARD_DEEMON_RUNTIME_VARIANT_C 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>
#include <deemon/variant.h>

#include <hybrid/sched/yield.h>

#include "runtime_error.h"

/**/
#include <stdint.h> /* uint16_t */

/* Variant deemon/C value wrapper. */

DECL_BEGIN

#if 1
struct alignof_variant {
	char pad;
	struct Dee_variant v;
};
STATIC_ASSERT_MSG(sizeof(struct Dee_variant) == (offsetof(struct Dee_variant, var_data) + 16),
                  "The data-blob of `struct Dee_variant' should be 16 bytes large");
STATIC_ASSERT_MSG(offsetof(struct alignof_variant, v) == __ALIGNOF_POINTER__,
                  "`struct Dee_variant' shouldn't require a "
                  "greater alignment than '__ALIGNOF_POINTER__'");
#endif

#ifdef CONFIG_NO_THREADS
#define Dee_variant_lock(self)         Dee_variant_gettype_nonatomic(self)
#define Dee_variant_unlock(self, type) (void)0
#else /* CONFIG_NO_THREADS */
PRIVATE WUNUSED NONNULL((1)) enum Dee_variant_type DCALL
Dee_variant_lock(struct Dee_variant *__restrict self) {
	enum Dee_variant_type result;
	while ((result = (enum Dee_variant_type)atomic_xch((int *)&self->var_type,
	                                                   (int)Dee_VARIANT_LOCKED)) == Dee_VARIANT_LOCKED)
		SCHED_YIELD();
	return result;
}
#define Dee_variant_unlock(self, type) \
	atomic_write((int *)&(self)->var_type, (int)(type))
#endif /* !CONFIG_NO_THREADS */

/* Initialize "self" as a copy of "other" */
PUBLIC NONNULL((1, 2)) void DCALL
Dee_variant_init_copy(struct Dee_variant *__restrict self,
                      struct Dee_variant *__restrict other) {
	enum Dee_variant_type type = Dee_variant_lock(other);
	memcpy(&self->var_data, &other->var_data, sizeof(self->var_data));
	if (type == Dee_VARIANT_OBJECT)
		Dee_Incref(self->var_data.d_object);
	Dee_variant_unlock(other, type);
	self->var_type = type;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_init_deepcopy(struct Dee_variant *__restrict self,
                          struct Dee_variant *__restrict other) {
	Dee_variant_init_copy(self, other);
	if (self->var_type == Dee_VARIANT_OBJECT) {
		if (DeeObject_InplaceDeepCopy(&self->var_data.d_object))
			goto err_copy;
	}
	return 0;
err_copy:
	Dee_Decref(self->var_data.d_object);
	return -1;
}


/* Get the value of a variant in the form of a deemon object.
 * If the variant's type isn't set to "Dee_VARIANT_OBJECT", the
 * linked object is lazily allocated and assigned
 * @return: ITER_DONE: [Dee_variant_trygetobject] Returned if the variant is unbound
 * @return: NULL:      An error was thrown (`Dee_variant_getobject()' throws an UnboundAttribute error if "self" is unbound) */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_variant_trygetobject(struct Dee_variant *__restrict self) {
	DREF DeeObject *result;
	struct Dee_variant copy;
	copy.var_type = Dee_variant_lock(self);
	switch (copy.var_type) {
	case Dee_VARIANT_UNBOUND:
		Dee_variant_unlock(self, copy.var_type);
		return ITER_DONE;
	case Dee_VARIANT_OBJECT: {
		result = self->var_data.d_object;
		Dee_Incref(result);
		Dee_variant_unlock(self, copy.var_type);
		return result;
	}	break;
	default: break;
	}
	memcpy(&copy.var_data, &self->var_data, sizeof(copy.var_data));
	Dee_variant_unlock(self, copy.var_type);

	switch (copy.var_type) {
	case Dee_VARIANT_INT32:
		result = DeeInt_NewInt32(_Dee_variant_get_int32(&copy));
		break;
	case Dee_VARIANT_UINT32:
		result = DeeInt_NewUInt32(_Dee_variant_get_uint32(&copy));
		break;
	case Dee_VARIANT_INT64:
		result = DeeInt_NewInt64(_Dee_variant_get_int64(&copy));
		break;
	case Dee_VARIANT_UINT64:
		result = DeeInt_NewUInt64(_Dee_variant_get_uint64(&copy));
		break;
	case Dee_VARIANT_INT128:
		result = DeeInt_NewInt128(_Dee_variant_get_int128(&copy));
		break;
	case Dee_VARIANT_UINT128:
		result = DeeInt_NewUInt128(_Dee_variant_get_uint128(&copy));
		break;
#ifndef CONFIG_NO_FPU
	case Dee_VARIANT_FLOAT:
		result = DeeFloat_New(_Dee_variant_get_float(&copy));
		break;
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	if likely(result) {
		/* Try to remember cached object */
		enum Dee_variant_type new_type;
		new_type = Dee_variant_lock(self);
		if likely(new_type == copy.var_type &&
		          bcmp(&copy.var_data, &self->var_data,
		               sizeof(copy.var_data)) == 0) {
			Dee_Incref(result);
			self->var_data.d_object = result;
			Dee_variant_unlock(self, Dee_VARIANT_OBJECT);
		} else {
			Dee_variant_unlock(self, new_type);
		}
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
Dee_variant_getobject(struct Dee_variant *__restrict self,
                      DeeObject *owner, char const *attr) {
	DREF DeeObject *result = Dee_variant_trygetobject(self);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unbound_attribute_string(Dee_TYPE(owner), attr);
	return NULL;
}


#define VARIANT_SETVALUE(self, setter, value, type)                        \
	do {                                                                   \
		DREF DeeObject *_old_object     = NULL;                            \
		enum Dee_variant_type _old_type = Dee_variant_lock(self);          \
		if (_old_type == Dee_VARIANT_OBJECT) {                             \
			_old_object = self->var_data.d_object; /* Inherit reference */ \
			ASSERT(_old_object);                                           \
		}                                                                  \
		setter(self, value);                                               \
		Dee_variant_unlock(self, type);                                    \
		Dee_XDecref(_old_object);                                          \
	}	__WHILE0

/* Set the value of a variant (these can never fail) */
PUBLIC NONNULL((1, 2)) void DCALL
Dee_variant_setobject(struct Dee_variant *__restrict self, DeeObject *value) {
#define _Dee_variant_set_object(self, v) (void)((self)->var_data.d_object = (v))
	Dee_Incref(value);
	VARIANT_SETVALUE(self, _Dee_variant_set_object, value, Dee_VARIANT_OBJECT);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setint32(struct Dee_variant *__restrict self, int32_t value) {
	VARIANT_SETVALUE(self, _Dee_variant_set_int32, value, Dee_VARIANT_INT32);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setuint32(struct Dee_variant *__restrict self, uint32_t value) {
	VARIANT_SETVALUE(self, _Dee_variant_set_uint32, value, Dee_VARIANT_UINT32);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setint64(struct Dee_variant *__restrict self, int64_t value) {
	VARIANT_SETVALUE(self, _Dee_variant_set_int64, value, Dee_VARIANT_INT64);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setuint64(struct Dee_variant *__restrict self, uint64_t value) {
	VARIANT_SETVALUE(self, _Dee_variant_set_uint64, value, Dee_VARIANT_UINT64);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setint128(struct Dee_variant *__restrict self, Dee_int128_t value) {
	VARIANT_SETVALUE(self, _Dee_variant_set_int128, value, Dee_VARIANT_INT128);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setuint128(struct Dee_variant *__restrict self, Dee_uint128_t value) {
	VARIANT_SETVALUE(self, _Dee_variant_set_uint128, value, Dee_VARIANT_UINT128);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_variant_print_impl(struct Dee_variant *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	switch (self->var_type) {
	case Dee_VARIANT_UNBOUND:
		return 0;
	case Dee_VARIANT_OBJECT:
		return DeeObject_Print(self->var_data.d_object, printer, arg);
	case Dee_VARIANT_INT32:
		return DeeFormat_Printf(printer, arg, "%" PRFd32, _Dee_variant_get_int32(self));
	case Dee_VARIANT_UINT32:
		return DeeFormat_Printf(printer, arg, "%" PRFu32, _Dee_variant_get_uint32(self));
	case Dee_VARIANT_INT64:
		return DeeFormat_Printf(printer, arg, "%" PRFd64, _Dee_variant_get_int64(self));
	case Dee_VARIANT_UINT64:
		return DeeFormat_Printf(printer, arg, "%" PRFu64, _Dee_variant_get_uint64(self));
	case Dee_VARIANT_INT128:
		return DeeFormat_Printf(printer, arg, "%" PRFd128, _Dee_variant_get_int128(self));
	case Dee_VARIANT_UINT128:
		return DeeFormat_Printf(printer, arg, "%" PRFu128, _Dee_variant_get_uint128(self));
#ifndef CONFIG_NO_FPU
	case Dee_VARIANT_FLOAT:
		return DeeFormat_Printf(printer, arg, "%f", _Dee_variant_get_float(self));
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

/* Print the DeeObject_Str() or DeeObject_Repr() of the object linked to "self"
 * When the variant "self" is unbound, nothing is printed and "0" is returned.
 * If you want to (safely) implement custom handling for unbound variants, you
 * should wrap these calls like this:
 * >> struct Dee_variant copy;
 * >> Dee_variant_init_copy(&copy, VARIANT_TO_PRINT);
 * >> if (Dee_variant_isbound_nonatomic(&copy)) {
 * >>     result = Dee_variant_print(&copy, printer, arg);
 * >> } else {
 * >>     result = DeeFormat_PRINT(printer, arg, "<UNBOUND>");
 * >> }
 * >> Dee_variant_fini(&copy); */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_variant_print(struct Dee_variant *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	struct Dee_variant copy;
	Dee_variant_init_copy(&copy, self);
	result = Dee_variant_print_impl(&copy, printer, arg);
	Dee_variant_fini(&copy);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_variant_printrepr(struct Dee_variant *__restrict self,
                      Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	struct Dee_variant copy;
	Dee_variant_init_copy(&copy, self);
	/* Only regular objects have code for "repr" */
	if (copy.var_type == Dee_VARIANT_OBJECT) {
		result = DeeObject_PrintRepr(copy.var_data.d_object, printer, arg);
	} else {
		result = Dee_variant_print_impl(&copy, printer, arg);
	}
	Dee_variant_fini(&copy);
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_VARIANT_C */
