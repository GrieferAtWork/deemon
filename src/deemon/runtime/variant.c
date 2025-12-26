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
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/util/atomic.h>
#include <deemon/variant.h>

#include <hybrid/minmax.h>
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

PUBLIC NONNULL((1, 2)) void DCALL
Dee_variant_visit(struct Dee_variant *__restrict self,
                  Dee_visit_t proc, void *arg) {
	enum Dee_variant_type type = Dee_variant_lock(self);
	if (type == Dee_VARIANT_OBJECT)
		(*proc)(self->var_data.d_object, arg);
	Dee_variant_unlock(self, type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_serialize(struct Dee_variant *__restrict self,
                      DeeSerial *__restrict writer,
                      Dee_seraddr_t addr) {
	struct Dee_variant *out = DeeSerial_Addr2Mem(writer, addr, struct Dee_variant);
	enum Dee_variant_type type = Dee_variant_lock(self);
	out->var_type = type;
	switch (type) {

	case Dee_VARIANT_OBJECT: {
		DREF DeeObject *obj = self->var_data.d_object;
		Dee_Incref(obj);
		Dee_variant_unlock(self, type);
		return DeeSerial_PutObjectInherited(writer,
		                                    addr + offsetof(struct Dee_variant, var_data.d_object),
		                                    obj);
	}	break;

	case Dee_VARIANT_CSTRLEN:
		out->var_data.d_cstrlen.sl_len = self->var_data.d_cstrlen.sl_len;
		ATTR_FALLTHROUGH
	case Dee_VARIANT_CSTR: {
		char const *cstr = self->var_data.d_cstr;
		Dee_variant_unlock(self, type);
		return DeeSerial_PutStatic(writer,
		                           addr + offsetof(struct Dee_variant, var_data.d_cstr),
		                           cstr);
	}	break;

	default: break;
	}
	out->var_data = self->var_data;
	Dee_variant_unlock(self, type);
	return 0;
}



/* Get the value of a variant in the form of a deemon object.
 * If the variant's type isn't set to "Dee_VARIANT_OBJECT", the
 * linked object is lazily allocated and assigned
 * @return: ITER_DONE: Variant is unbound
 * @return: NULL:      An error was thrown */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_variant_getobject(struct Dee_variant *__restrict self) {
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
	case Dee_VARIANT_CSTR:
		result = DeeString_New(_Dee_variant_get_cstr(&copy));
		break;
	case Dee_VARIANT_CSTRLEN:
		result = DeeString_NewSized(_Dee_variant_get_cstr(&copy),
		                            _Dee_variant_get_cstrlen(&copy));
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

PRIVATE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
Dee_variant_getobjecttype_impl(struct Dee_variant *__restrict self) {
	switch (self->var_type) {
	case Dee_VARIANT_UNBOUND:
		return NULL;
	case Dee_VARIANT_OBJECT:
		return Dee_TYPE(self->var_data.d_object);
	case Dee_VARIANT_INT32:
	case Dee_VARIANT_UINT32:
	case Dee_VARIANT_INT64:
	case Dee_VARIANT_UINT64:
	case Dee_VARIANT_INT128:
	case Dee_VARIANT_UINT128:
		return &DeeInt_Type;
	case Dee_VARIANT_CSTR:
	case Dee_VARIANT_CSTRLEN:
		return &DeeString_Type;
#ifndef CONFIG_NO_FPU
	case Dee_VARIANT_FLOAT:
		return &DeeFloat_Type;
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

/* Returns the type of the object bound to "self" (or "NULL" if "self" is unbound) */
PUBLIC WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
Dee_variant_getobjecttype(struct Dee_variant *__restrict self) {
	DeeTypeObject *result;
	struct Dee_variant copy;
	Dee_variant_init_copy(&copy, self);
	result = Dee_variant_getobjecttype_impl(&copy);
	Dee_XIncref(result);
	Dee_variant_fini(&copy);
	return result;
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
PUBLIC NONNULL((1)) void DCALL
Dee_variant_setunbound(struct Dee_variant *__restrict self) {
#define _Dee_variant_set_unbound(self, v) (void)0
	VARIANT_SETVALUE(self, _Dee_variant_set_unbound, ~, Dee_VARIANT_UNBOUND);
}

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

PUBLIC NONNULL((1, 2)) void DCALL
Dee_variant_setcstr(struct Dee_variant *__restrict self, char const *str) {
	VARIANT_SETVALUE(self, _Dee_variant_set_cstr, str, Dee_VARIANT_CSTR);
}

PUBLIC NONNULL((1)) void DCALL
Dee_variant_setcstrlen(struct Dee_variant *__restrict self,
                       char const *str, size_t len) {
#define _setpair(a, v) (_Dee_variant_set_cstr(a, str), _Dee_variant_set_cstrlen(a, len))
	VARIANT_SETVALUE(self, _setpair, ~, Dee_VARIANT_CSTRLEN);
#undef _setpair
}


/* Same as `Dee_variant_init_cstr()', but check at runtime if "str" is guarantied
 * to point into statically allocated memory. If it does, use "Dee_VARIANT_CSTR"
 * as variant typing, else use "Dee_VARIANT_OBJECT" and "DeeString_New()".
 *
 * When there is doubt regarding "str" being static, these functions always go the
 * safe route by assuming that it isn't, and turning them into string objects. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_init_cstr_maybe(struct Dee_variant *__restrict self,
                            char const *str) {
	if (!DeeSystem_IsStaticPointer(str)) {
		DREF DeeObject *obj = DeeString_New(str);
		if unlikely(!obj)
			goto err;
		Dee_variant_init_object_inherited(self, obj);
		return 0;
	}
	Dee_variant_init_cstr(self, str);
	return 0;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1)) ATTR_INS(2, 3) int DCALL
Dee_variant_init_cstrlen_maybe(struct Dee_variant *__restrict self,
                               char const *str, size_t len) {
	if (!DeeSystem_IsStaticPointer(str)) {
		DREF DeeObject *obj = DeeString_NewSized(str, len);
		if unlikely(!obj)
			goto err;
		Dee_variant_init_object_inherited(self, obj);
		return 0;
	}
	Dee_variant_init_cstrlen(self, str, len);
	return 0;
err:
	return -1;
}


PUBLIC NONNULL((1, 2)) bool DCALL
Dee_variant_setobject_if_unbound(struct Dee_variant *__restrict self, DeeObject *value) {
	enum Dee_variant_type old_type = Dee_variant_lock(self);
	if (old_type != Dee_VARIANT_UNBOUND) {
		Dee_variant_unlock(self, old_type);
		return false;
	}
	Dee_Incref(value);
	self->var_data.d_object = value;
	Dee_variant_unlock(self, Dee_VARIANT_OBJECT);
	return true;
}

#define VARIANT_SETVALUE_IF_UNBOUND(self, setter, value, type)   \
	do {                                                         \
		enum Dee_variant_type old_type = Dee_variant_lock(self); \
		if (old_type != Dee_VARIANT_UNBOUND) {                   \
			Dee_variant_unlock(self, old_type);                  \
			return false;                                        \
		}                                                        \
		setter(self, value);                                     \
		Dee_variant_unlock(self, type);                          \
		return true;                                             \
	}	__WHILE0

PUBLIC NONNULL((1)) bool DCALL
Dee_variant_setint32_if_unbound(struct Dee_variant *__restrict self, int32_t value) {
	VARIANT_SETVALUE_IF_UNBOUND(self, _Dee_variant_set_int32, value, Dee_VARIANT_INT32);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_variant_setuint32_if_unbound(struct Dee_variant *__restrict self, uint32_t value) {
	VARIANT_SETVALUE_IF_UNBOUND(self, _Dee_variant_set_uint32, value, Dee_VARIANT_UINT32);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_variant_setint64_if_unbound(struct Dee_variant *__restrict self, int64_t value) {
	VARIANT_SETVALUE_IF_UNBOUND(self, _Dee_variant_set_int64, value, Dee_VARIANT_INT64);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_variant_setuint64_if_unbound(struct Dee_variant *__restrict self, uint64_t value) {
	VARIANT_SETVALUE_IF_UNBOUND(self, _Dee_variant_set_uint64, value, Dee_VARIANT_UINT64);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_variant_setint128_if_unbound(struct Dee_variant *__restrict self, Dee_int128_t value) {
	VARIANT_SETVALUE_IF_UNBOUND(self, _Dee_variant_set_int128, value, Dee_VARIANT_INT128);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_variant_setuint128_if_unbound(struct Dee_variant *__restrict self, Dee_uint128_t value) {
	VARIANT_SETVALUE_IF_UNBOUND(self, _Dee_variant_set_uint128, value, Dee_VARIANT_UINT128);
}


/* Check if "var_data" of "a" and "b" are identical */
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_variant_samedata(struct Dee_variant const *__restrict a,
                     struct Dee_variant const *__restrict b,
                     enum Dee_variant_type type) {
	switch (type) {
	case Dee_VARIANT_UNBOUND:
		return true;
	case Dee_VARIANT_OBJECT:
		return a->var_data.d_object == b->var_data.d_object;
	case Dee_VARIANT_INT32:
	case Dee_VARIANT_UINT32:
		return _Dee_variant_get_uint32(a) == _Dee_variant_get_uint32(b);
	case Dee_VARIANT_INT64:
	case Dee_VARIANT_UINT64:
		return _Dee_variant_get_uint64(a) == _Dee_variant_get_uint64(b);
	case Dee_VARIANT_INT128:
	case Dee_VARIANT_UINT128: {
		Dee_uint128_t aval = _Dee_variant_get_uint128(a);
		Dee_uint128_t bval = _Dee_variant_get_uint128(b);
		return __hybrid_uint128_eq128(aval, bval);
	}	break;
	case Dee_VARIANT_CSTRLEN:
		if (_Dee_variant_get_cstrlen(a) != _Dee_variant_get_cstrlen(b))
			return false;
		ATTR_FALLTHROUGH
	case Dee_VARIANT_CSTR:
		return _Dee_variant_get_cstr(a) == _Dee_variant_get_cstr(b);
#ifndef CONFIG_NO_FPU
	case Dee_VARIANT_FLOAT:
		return _Dee_variant_get_float(a) == _Dee_variant_get_float(b);
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}


/* Compare "self" with "oldval" (asserting identical types and memcmp()'ing "var_data").
 * If this compare indicates equality, atomically assign "newval" to "self" and return
 * "true". Else, do nothing and return "false".
 *
 * For this purpose, it is assumed that "oldval" and "newval" will not be changed by
 * another thread (you can easily assert this by simply ensuring that both "oldval" and
 * "newval" are allocated on your stack)
 *
 * Example usage:
 * >> struct Dee_variant oldval;
 * >> struct Dee_variant newval;
 * >> for (;;) {
 * >>     Dee_variant_init_copy(&oldval, &VARIANT);
 * >>     Dee_variant_init_uint32(&newval, 42);
 * >>     if (Dee_variant_cmpxch(&VARIANT, &oldval, &newval))
 * >>         break;
 * >>     Dee_variant_fini(&oldval);
 * >> }
 * >> Dee_variant_fini(&newval);
 * >>
 * >> // At this point, "self" is known to be "uint32:42"
 * >> // and "oldval" is whatever it was before
 * >> ...
 * >>
 * >> Dee_variant_fini(&oldval);
 *
 * @param: self:   The variant whose value to change
 * @param: oldval: The expected old value of "self"
 * @param: newval: The new value to assign when "self" still equals "oldval" */
PUBLIC WUNUSED NONNULL((1, 2, 3)) bool DCALL
Dee_variant_cmpxch(struct Dee_variant *__restrict self,
                   struct Dee_variant const *__restrict oldval,
                   struct Dee_variant const *__restrict newval) {
	bool result;
	enum Dee_variant_type old_type = Dee_variant_lock(self);
	ASSERT(self != oldval);
	ASSERT(self != newval);
	result = old_type == oldval->var_type &&
	         Dee_variant_samedata(self, oldval, old_type);
	if (result) {
		memcpy(&self->var_data, &newval->var_data, sizeof(self->var_data));
		Dee_variant_unlock(self, newval->var_type);
		if (old_type == Dee_VARIANT_OBJECT)
			Dee_Decref(oldval->var_data.d_object);
	} else {
		Dee_variant_unlock(self, old_type);
	}
	return result;
}



PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_variant_print_impl(struct Dee_variant *__restrict self,
                       Dee_formatprinter_t printer, void *arg,
                       bool repr) {
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
	case Dee_VARIANT_CSTR: {
		char const *cstr = _Dee_variant_get_cstr(self);
		if (repr)
			return DeeFormat_Printf(printer, arg, "%q", cstr);
		return (*printer)(arg, cstr, strlen(cstr));
	}	break;
	case Dee_VARIANT_CSTRLEN:
		if (repr) {
			return DeeFormat_Printf(printer, arg, "%$q",
			                        _Dee_variant_get_cstrlen(self),
			                        _Dee_variant_get_cstr(self));
		}
		return (*printer)(arg, _Dee_variant_get_cstr(self), _Dee_variant_get_cstrlen(self));
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
	result = Dee_variant_print_impl(&copy, printer, arg, false);
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
		result = Dee_variant_print_impl(&copy, printer, arg, true);
	}
	Dee_variant_fini(&copy);
	return result;
}



#ifndef CONFIG_NO_FPU
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL
float_hash(DeeFloatObject *__restrict self);
#endif /* !CONFIG_NO_FPU */

/* Compare variants with each other. */
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
Dee_variant_hash_impl(struct Dee_variant const *__restrict self) {
	switch (self->var_type) {
	case Dee_VARIANT_UNBOUND:
		return DEE_HASHOF_UNBOUND_ITEM;
	case Dee_VARIANT_OBJECT:
		return DeeObject_Hash(self->var_data.d_object);
	case Dee_VARIANT_INT32:
		return (Dee_hash_t)_Dee_variant_get_int32(self);
	case Dee_VARIANT_UINT32:
		return (Dee_hash_t)_Dee_variant_get_uint32(self);
	case Dee_VARIANT_INT64:
		return (Dee_hash_t)_Dee_variant_get_int64(self);
	case Dee_VARIANT_UINT64:
		return (Dee_hash_t)_Dee_variant_get_uint64(self);
	case Dee_VARIANT_INT128:
	case Dee_VARIANT_UINT128: {
		Dee_uint128_t value = _Dee_variant_get_uint128(self);
#if Dee_SIZEOF_HASH_T <= 4
		return (Dee_hash_t)__hybrid_uint128_get32(value);
#else /* Dee_SIZEOF_HASH_T <= 4 */
		return (Dee_hash_t)__hybrid_uint128_get64(value);
#endif /* Dee_SIZEOF_HASH_T > 4 */
	}	break;
	case Dee_VARIANT_CSTR:
		return Dee_HashStr(_Dee_variant_get_cstr(self));
	case Dee_VARIANT_CSTRLEN:
		return Dee_HashPtr(_Dee_variant_get_cstr(self), _Dee_variant_get_cstrlen(self));
#ifndef CONFIG_NO_FPU
	case Dee_VARIANT_FLOAT: {
		double value = _Dee_variant_get_float(self);
		DeeFloatObject *obj = COMPILER_CONTAINER_OF(&value, DeeFloatObject, f_value);
		return float_hash(obj);
	}	break;
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
dee_memcmp2(void const *lhs, size_t lhs_size,
            void const *rhs, size_t rhs_size) {
	size_t common = MIN(lhs_size, rhs_size);
	int result = memcmp(lhs, rhs, common * sizeof(char));
	if (result < 0) {
		result = Dee_COMPARE_LO;
	} else if (result > 0) {
		result = Dee_COMPARE_GR;
	} else /*if (result == 0)*/ {
		if (lhs_size < rhs_size) {
			result = Dee_COMPARE_LO;
		} else if (lhs_size > rhs_size) {
			result = Dee_COMPARE_GR;
		}
	}
	return result;
}

/* Fast-pass comparison function that only implements cases that
 * can never throw an exception. Returns "Dee_COMPARE_ERR" if the
 * variant-type case between "lhs" and "rhs" doesn't have a fast-
 * pass, or cannot be done without ever throwing an exception.
 *
 * @return: -1: lhs < rhs
 * @return: 0 : Equal
 * @return: 1 : lhs > rhs
 * @return: Dee_COMPARE_ERR: Fast comparison isn't possible */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_fast_compare_impl(struct Dee_variant const *__restrict lhs,
                              struct Dee_variant const *__restrict rhs) {
	switch (lhs->var_type) {

	case Dee_VARIANT_UNBOUND:
		return rhs->var_type == Dee_VARIANT_UNBOUND
		       ? Dee_COMPARE_EQ  /* UNBOUND == UNBOUND */
		       : Dee_COMPARE_LO; /* UNBOUND < BOUND */

	case Dee_VARIANT_INT32: {
		int32_t lhs_value = _Dee_variant_get_int32(lhs);
		switch (rhs->var_type) {
		case Dee_VARIANT_INT32:
			Dee_return_compareT(int32_t, lhs_value, _Dee_variant_get_int32(rhs));
		case Dee_VARIANT_UINT32:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_uint32(rhs));
		case Dee_VARIANT_INT64:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_int64(rhs));
		default: break;
		}
	}	break;

	case Dee_VARIANT_UINT32: {
		uint32_t lhs_value = _Dee_variant_get_uint32(lhs);
		switch (rhs->var_type) {
		case Dee_VARIANT_INT32:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_int32(rhs));
		case Dee_VARIANT_UINT32:
			Dee_return_compareT(uint32_t, lhs_value, _Dee_variant_get_uint32(rhs));
		case Dee_VARIANT_INT64:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_int64(rhs));
		case Dee_VARIANT_UINT64:
			Dee_return_compareT(uint64_t, lhs_value, _Dee_variant_get_uint64(rhs));
		default: break;
		}
	}	break;

	case Dee_VARIANT_INT64: {
		int64_t lhs_value = _Dee_variant_get_int64(lhs);
		switch (rhs->var_type) {
		case Dee_VARIANT_INT32:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_int32(rhs));
		case Dee_VARIANT_UINT32:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_uint32(rhs));
		case Dee_VARIANT_INT64:
			Dee_return_compareT(int64_t, lhs_value, _Dee_variant_get_int64(rhs));
		default: break;
		}
	}	break;

	case Dee_VARIANT_UINT64: {
		int64_t lhs_value = _Dee_variant_get_int64(lhs);
		switch (rhs->var_type) {
		case Dee_VARIANT_UINT32:
			Dee_return_compareT(uint64_t, lhs_value, _Dee_variant_get_uint32(rhs));
		case Dee_VARIANT_UINT64:
			Dee_return_compareT(uint64_t, lhs_value, _Dee_variant_get_uint64(rhs));
		default: break;
		}
	}	break;

	case Dee_VARIANT_INT128: {
		if (rhs->var_type == Dee_VARIANT_INT128) {
			Dee_int128_t lhs_value = _Dee_variant_get_int128(lhs);
			Dee_int128_t rhs_value = _Dee_variant_get_int128(rhs);
			if (__hybrid_int128_lo128(lhs_value, rhs_value))
				return Dee_COMPARE_LO;
			if (__hybrid_int128_gr128(lhs_value, rhs_value))
				return Dee_COMPARE_GR;
			return Dee_COMPARE_EQ;
		}
	}	break;

	case Dee_VARIANT_UINT128: {
		if (rhs->var_type == Dee_VARIANT_UINT128) {
			Dee_uint128_t lhs_value = _Dee_variant_get_uint128(lhs);
			Dee_uint128_t rhs_value = _Dee_variant_get_uint128(rhs);
			if (__hybrid_uint128_lo128(lhs_value, rhs_value))
				return Dee_COMPARE_LO;
			if (__hybrid_uint128_gr128(lhs_value, rhs_value))
				return Dee_COMPARE_GR;
			return Dee_COMPARE_EQ;
		}
	}	break;

	case Dee_VARIANT_CSTR: {
		char const *lhs_str = _Dee_variant_get_cstr(lhs);
		if (rhs->var_type == Dee_VARIANT_CSTR) {
			char const *rhs_str = _Dee_variant_get_cstr(rhs);
			int result = strcmp(lhs_str, rhs_str);
			return Dee_CompareFromDiff(result);
		} else if (rhs->var_type == Dee_VARIANT_CSTRLEN) {
			char const *rhs_str = _Dee_variant_get_cstr(rhs);
			size_t lhs_len = strlen(lhs_str);
			size_t rhs_len = _Dee_variant_get_cstrlen(rhs);
			return dee_memcmp2(lhs_str, lhs_len, rhs_str, rhs_len);
		}
	}	break;

	case Dee_VARIANT_CSTRLEN: {
		char const *lhs_str = _Dee_variant_get_cstr(lhs);
		size_t lhs_len = _Dee_variant_get_cstrlen(lhs);
		if (rhs->var_type == Dee_VARIANT_CSTR) {
			char const *rhs_str = _Dee_variant_get_cstr(rhs);
			size_t rhs_len = strlen(rhs_str);
			return dee_memcmp2(lhs_str, lhs_len, rhs_str, rhs_len);
		} else if (rhs->var_type == Dee_VARIANT_CSTRLEN) {
			char const *rhs_str = _Dee_variant_get_cstr(rhs);
			size_t rhs_len = _Dee_variant_get_cstrlen(rhs);
			return dee_memcmp2(lhs_str, lhs_len, rhs_str, rhs_len);
		}
	}	break;

#ifndef CONFIG_NO_FPU
	case Dee_VARIANT_FLOAT: {
		if (rhs->var_type == Dee_VARIANT_FLOAT) {
			Dee_return_compareT(double,
			                    _Dee_variant_get_float(lhs),
			                    _Dee_variant_get_float(rhs));
		}
	}	break;
#endif /* !CONFIG_NO_FPU */

	default: break;
	}
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_compare_impl(struct Dee_variant *__restrict lhs,
                         struct Dee_variant *__restrict rhs) {
	DREF DeeObject *lhs_ob;
	DREF DeeObject *rhs_ob;
	int result = Dee_variant_fast_compare_impl(lhs, rhs);
	if (result != Dee_COMPARE_ERR)
		return result;
	lhs_ob = Dee_variant_getobject(lhs);
	if unlikely(!ITER_ISOK(lhs_ob)) {
		if unlikely(!lhs_ob)
			goto err;
		return Dee_variant_isbound_nonatomic(rhs)
		       ? Dee_COMPARE_LO  /* UNBOUND < BOUND */
		       : Dee_COMPARE_EQ; /* UNBOUND == UNBOUND */
	}
	rhs_ob = Dee_variant_getobject(rhs);
	if unlikely(!ITER_ISOK(rhs_ob)) {
		Dee_Decref(lhs_ob);
		if unlikely(!rhs_ob)
			goto err;
		return Dee_COMPARE_GR; /* BOUND > UNBOUND */
	}
	result = DeeObject_Compare(lhs_ob, rhs_ob);
	Dee_Decref_unlikely(rhs_ob);
	Dee_Decref_unlikely(lhs_ob);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_compare_eq_impl(struct Dee_variant *__restrict lhs,
                            struct Dee_variant *__restrict rhs) {
	DREF DeeObject *lhs_ob;
	DREF DeeObject *rhs_ob;
	int result = Dee_variant_fast_compare_impl(lhs, rhs);
	if (result != Dee_COMPARE_ERR)
		return result;
	lhs_ob = Dee_variant_getobject(lhs);
	if unlikely(!ITER_ISOK(lhs_ob)) {
		if unlikely(!lhs_ob)
			goto err;
		return Dee_variant_isbound_nonatomic(rhs)
		       ? Dee_COMPARE_LO  /* UNBOUND < BOUND */
		       : Dee_COMPARE_EQ; /* UNBOUND == UNBOUND */
	}
	rhs_ob = Dee_variant_getobject(rhs);
	if unlikely(!ITER_ISOK(rhs_ob)) {
		Dee_Decref(lhs_ob);
		if unlikely(!rhs_ob)
			goto err;
		return Dee_COMPARE_GR; /* BOUND > UNBOUND */
	}
	result = DeeObject_CompareEq(lhs_ob, rhs_ob);
	Dee_Decref_unlikely(rhs_ob);
	Dee_Decref_unlikely(lhs_ob);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_trycompare_eq_impl(struct Dee_variant *__restrict lhs,
                               struct Dee_variant *__restrict rhs) {
	DREF DeeObject *lhs_ob;
	DREF DeeObject *rhs_ob;
	int result = Dee_variant_fast_compare_impl(lhs, rhs);
	if (result != Dee_COMPARE_ERR)
		return result;
	lhs_ob = Dee_variant_getobject(lhs);
	if unlikely(!ITER_ISOK(lhs_ob)) {
		if unlikely(!lhs_ob)
			goto err;
		return Dee_variant_isbound_nonatomic(rhs)
		       ? Dee_COMPARE_LO  /* UNBOUND < BOUND */
		       : Dee_COMPARE_EQ; /* UNBOUND == UNBOUND */
	}
	rhs_ob = Dee_variant_getobject(rhs);
	if unlikely(!ITER_ISOK(rhs_ob)) {
		Dee_Decref(lhs_ob);
		if unlikely(!rhs_ob)
			goto err;
		return Dee_COMPARE_GR; /* BOUND > UNBOUND */
	}
	result = DeeObject_TryCompareEq(lhs_ob, rhs_ob);
	Dee_Decref_unlikely(rhs_ob);
	Dee_Decref_unlikely(lhs_ob);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PUBLIC WUNUSED NONNULL((1)) Dee_hash_t DCALL
Dee_variant_hash(struct Dee_variant *__restrict self) {
	Dee_hash_t result;
	struct Dee_variant copy;
	Dee_variant_init_copy(&copy, self);
	result = Dee_variant_hash_impl(&copy);
	Dee_variant_fini(&copy);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_compare(struct Dee_variant *lhs, struct Dee_variant *rhs) {
	int result;
	struct Dee_variant lhs_copy;
	struct Dee_variant rhs_copy;
	Dee_variant_init_copy(&lhs_copy, lhs);
	Dee_variant_init_copy(&rhs_copy, rhs);
	result = Dee_variant_compare_impl(&lhs_copy, &rhs_copy);
	Dee_variant_fini(&rhs_copy);
	Dee_variant_fini(&lhs_copy);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_compare_eq(struct Dee_variant *lhs, struct Dee_variant *rhs) {
	int result;
	struct Dee_variant lhs_copy;
	struct Dee_variant rhs_copy;
	Dee_variant_init_copy(&lhs_copy, lhs);
	Dee_variant_init_copy(&rhs_copy, rhs);
	result = Dee_variant_compare_eq_impl(&lhs_copy, &rhs_copy);
	Dee_variant_fini(&rhs_copy);
	Dee_variant_fini(&lhs_copy);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_variant_trycompare_eq(struct Dee_variant *lhs, struct Dee_variant *rhs) {
	int result;
	struct Dee_variant lhs_copy;
	struct Dee_variant rhs_copy;
	Dee_variant_init_copy(&lhs_copy, lhs);
	Dee_variant_init_copy(&rhs_copy, rhs);
	result = Dee_variant_trycompare_eq_impl(&lhs_copy, &rhs_copy);
	Dee_variant_fini(&rhs_copy);
	Dee_variant_fini(&lhs_copy);
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_VARIANT_C */
