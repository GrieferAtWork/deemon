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
#ifndef GUARD_DEEMON_OBJECTS_ITERATOR_C
#define GUARD_DEEMON_OBJECTS_ITERATOR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/mro.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq_functions.h"
/**/

#include <stddef.h> /* size_t, offsetof */
#include <stdint.h> /* uintptr_t */

#undef SSIZE_MIN
#undef SSIZE_MAX
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_not_bidirectional(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Iterator instance of `%k' is not bi-directional",
	                       Dee_TYPE(self));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
has_generic_attribute(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr) {
	if (tp_self->tp_attr) {
		if (tp_self->tp_attr->tp_getattr) {
			DREF DeeObject *obj;
			obj = (*maketyped__getattr(tp_self->tp_attr->tp_getattr))(tp_self, self, attr);
			if (obj) {
				Dee_Decref(obj);
				return 1;
			}
			if (DeeError_Catch(&DeeError_NotImplemented) ||
			    DeeError_Catch(&DeeError_AttributeError))
				return 0;
			return -1;
		}
	} else {
		char const *name = DeeString_STR(attr);
		dhash_t hash     = DeeString_Hash(attr);
		/* TODO: Search the type's instance-attribute cache and check
		 *       if the attribute is implemented by the type itself. */
		if (DeeType_IsClass(tp_self))
			return DeeClass_QueryInstanceAttributeStringHash(tp_self, name, hash) != NULL ? 1 : 0;
		if (tp_self->tp_methods &&
		    DeeType_HasMethodAttrStringHash(tp_self, tp_self, name, hash))
			goto yes;
		if (tp_self->tp_getsets &&
		    DeeType_HasGetSetAttrStringHash(tp_self, tp_self, name, hash))
			goto yes;
		if (tp_self->tp_members &&
		    DeeType_HasMemberAttrStringHash(tp_self, tp_self, name, hash))
			goto yes;
	}
	return 0;
yes:
	return 1;
}


INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
get_generic_attribute(DeeTypeObject *tp_self, DeeObject *self, DeeObject *name) {
	DREF DeeObject *result;
	dhash_t hash;
	ASSERT_OBJECT(tp_self);
	ASSERT_OBJECT(self);
	ASSERT(DeeType_Check(tp_self));
	ASSERT(DeeObject_InstanceOf(self, tp_self));
	hash = DeeString_Hash(name);
	/* TODO: Search the type's instance-attribute cache and check
	 *       if the attribute is implemented by the type itself. */
	if (DeeType_IsClass(tp_self)) {
		struct class_attribute *member;
		if ((member = DeeType_QueryAttributeHash(tp_self, tp_self, name, hash)) != NULL) {
			struct class_desc *desc = DeeClass_DESC(tp_self);
			return DeeInstance_GetAttribute(desc, DeeInstance_DESC(desc, self), self, member);
		}
		result = ITER_DONE;
	} else {
		result = ITER_DONE;
		if (tp_self->tp_methods &&
		    (result = DeeType_GetMethodAttrStringHash(tp_self, tp_self, self, DeeString_STR(name), hash)) != ITER_DONE)
			goto done;
		if (tp_self->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, tp_self, self, DeeString_STR(name), hash)) != ITER_DONE)
			goto done;
		if (tp_self->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, tp_self, self, DeeString_STR(name), hash)) != ITER_DONE)
			goto done;
	}
done:
	return result;
}




DEFAULT_OPDEF WUNUSED NONNULL((1)) int DCALL iterator_inc(DeeObject **__restrict p_self);
DEFAULT_OPDEF WUNUSED NONNULL((1)) int DCALL iterator_dec(DeeObject **__restrict p_self);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_inplace_add(DeeObject **__restrict p_self, DeeObject *countob);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_inplace_sub(DeeObject **__restrict p_self, DeeObject *countob);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL iterator_add(DeeObject *self, DeeObject *countob);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL iterator_sub(DeeObject *self, DeeObject *countob);





DEFAULT_OPIMP WUNUSED NONNULL((1)) int DCALL
iterator_bool(DeeObject *__restrict self) {
	DeeObject *elem;
	/* Check if the Iterator has been exhausted
	 * by creating a copy and testing it. */
	if unlikely((self = DeeObject_Copy(self)) == NULL)
		goto err;
	elem = DeeObject_IterNext(self);
	Dee_Decref(self);
	if (elem == ITER_DONE)
		return 0; /* Empty iterator. */
	if unlikely(!elem)
		goto err; /* Error. */
	Dee_Decref(elem);
	return 1; /* Non-empty iterator. */
err:
	return -1;
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
iterator_printrepr(DeeObject *__restrict self,
                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	DREF DeeObject *iterator;
	DREF DeeObject *elem;
	bool is_first;
	/* Create a representation of the Iterator's elements:
	 * >> local x = [10, 20, 30];
	 * >> local y = x.operator iter();
	 * >> y.operator next();
	 * >> print repr y; // { 20, 30 }.operator iter()
	 */
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	iterator = DeeObject_Copy(self);
	if unlikely(!iterator) {
		temp = -1;
		goto err;
	}
	is_first = true;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_iterator_elem;
			result += temp;
		}
		temp = DeeFormat_PrintObjectRepr(printer, arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_iterator;
		if (DeeThread_CheckInterrupt()) {
			temp = -1;
			goto err_iterator;
		}
		is_first = false;
	}
	Dee_Decref(iterator);
	if unlikely(!elem) {
		temp = -1;
		goto err;
	}
	if (!is_first) {
		temp = DeeFormat_PRINT(printer, arg, " }.operator iter()");
	} else {
		temp = DeeFormat_PRINT(printer, arg, "}.operator iter()");
	}
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err_iterator_elem:
	Dee_Decref(elem);
err_iterator:
	Dee_Decref(iterator);
err:
	return temp;
}

/* A default-constructed, raw iterator object behaves as empty. */
STATIC_ASSERT_MSG((size_t)(uintptr_t)ITER_DONE == (size_t)-1, "Assumed by definition of `iterator_iternext'");
#define iterator_iternext (*(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1)

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
get_remaining_iterations(DeeObject *__restrict self) {
	size_t result;
	DREF DeeObject *elem;
	if unlikely((self = DeeObject_Copy(self)) == NULL)
		goto err;
	result = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		++result;
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err_self;
	}
	if unlikely(!elem)
		goto err_self;
	Dee_Decref(self);
	return result;
err_self:
	Dee_Decref(self);
err:
	return (size_t)-1;
}


DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) int DCALL
iterator_compare(DeeObject *self, DeeObject *other) {
	size_t mylen, otlen;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	if (self == other)
		return 0;
	mylen = get_remaining_iterations(self);
	if unlikely(mylen == (size_t)-1)
		goto err;
	otlen = get_remaining_iterations(other);
	if unlikely(otlen == (size_t)-1)
		goto err;
	Dee_return_compare(otlen, mylen); /* Reverse, since "remaining iterations" will reduce over time. */
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp iterator_cmp = {
	/* .tp_hash       = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ &iterator_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_next(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *defl = NULL;
	_DeeArg_Unpack0Or1(err, argc, argv, "next", &defl);
	result = DeeObject_IterNext(self);
	if (result == ITER_DONE) {
		if (defl)
			return_reference_(defl);
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iterator_do_hasprev(DeeObject *__restrict self,
                    struct type_nii const *__restrict nii) {
	DREF DeeObject *temp;
	if (nii->nii_common.nii_hasprev)
		return (*nii->nii_common.nii_hasprev)(self);
	temp = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
	if unlikely(!temp)
		goto err;
	return DeeObject_BoolInherited(temp);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_prev(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	_DeeArg_Unpack0(err, argc, argv, "prev");
	error = DeeIterator_Prev(self);
	if unlikely(error < 0)
		goto err;
	return_bool(!error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_rewind(DeeObject *self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "rewind");
	if (DeeIterator_Rewind(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
iterator_do_revert(DeeObject *__restrict self, size_t count,
                   /*[0..1]*/ DeeObject *count_ob,
                   /*[0..1]*/ DeeObject *minus_count_ob) {
	DREF DeeObject *temp, *temp2;
	DeeTypeObject *tp_self;
	int error;
	DeeTypeMRO mro;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		struct type_math *m;
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_revert)
				return (*nii->nii_common.nii_revert)(self, count);
			if (nii->nii_common.nii_getindex &&
			    nii->nii_common.nii_setindex) {
				size_t index;
				index = (*nii->nii_common.nii_getindex)(self);
				if unlikely(index == (size_t)-1)
					goto err;
				if unlikely(index == (size_t)-2)
					goto done;
				if (index <= count) {
					error = nii->nii_common.nii_rewind
					        ? (*nii->nii_common.nii_rewind)(self)
					        : (*nii->nii_common.nii_setindex)(self, 0);
				} else {
					error = (*nii->nii_common.nii_setindex)(self, index - count);
				}
				if unlikely(error)
					goto err;
				return index <= count ? 2 : 1;
			}
			if (nii->nii_common.nii_prev) {
				for (;;) {
					error = (*nii->nii_common.nii_prev)(self);
					if unlikely(error < 0)
						goto err;
					if (error)
						return 1;
					if (!--count)
						break;
				}
				goto done;
			}
			break;
		}
		m = tp_self->tp_math;
		if (m &&
		    (m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub)) {
			if (count_ob) {
				temp2 = count_ob;
				Dee_Incref(temp2);
			} else {
				temp2 = DeeInt_NewSize(count);
				if unlikely(!temp2)
					goto err;
			}
			temp = self;
			Dee_Incref(self);
			error = (*m->tp_inplace_sub)(&temp, temp2);
			Dee_Decref(temp2);
			if unlikely(error)
				goto err_temp;
			if (temp != self &&
			    DeeObject_MoveAssign(self, temp))
				goto err_temp;
			Dee_Decref(temp);
			goto done;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_revert);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			if (count_ob) {
				temp = count_ob;
				Dee_Incref(temp);
			} else {
				temp = DeeInt_NewSize(count);
				if unlikely(!temp)
					goto err;
			}
			temp2 = DeeObject_CallAttr(self, (DeeObject *)&str_revert, 1, &temp);
			if unlikely(!temp2)
				goto err_temp;
			Dee_Decref(temp2);
			Dee_Decref(temp);
			goto done;
		}
		if (m &&
		    (m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add)) {
			if (minus_count_ob) {
				temp2 = minus_count_ob;
				Dee_Incref(temp2);
			} else {
				temp2 = DeeInt_NewSSize(-(Dee_ssize_t)count);
				if unlikely(!temp2)
					goto err;
			}
			temp = self;
			Dee_Incref(self);
			error = (*m->tp_inplace_add)(&temp, temp2);
			Dee_Decref(temp2);
			if unlikely(error)
				goto err_temp;
			if (temp != self &&
			    DeeObject_MoveAssign(self, temp))
				goto err_temp;
			Dee_Decref(temp);
			goto done;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_advance);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			if (minus_count_ob) {
				temp = minus_count_ob;
				Dee_Incref(temp);
			} else {
				temp = DeeInt_NewSSize(-(Dee_ssize_t)count);
				if unlikely(!temp)
					goto err;
			}
			temp2 = DeeObject_CallAttr(self, (DeeObject *)&str_advance, 1, &temp);
			if unlikely(!temp2)
				goto err_temp;
			Dee_Decref(temp2);
			Dee_Decref(temp);
			goto done;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_index);
		if (temp != ITER_DONE) {
			size_t old_index;
			if unlikely(!temp)
				goto err;
			if (DeeObject_AsSize(temp, &old_index))
				goto err_temp;
			Dee_Decref(temp);
			if (old_index <= count) {
				if unlikely(DeeObject_SetAttr(self, (DeeObject *)&str_index, DeeInt_Zero))
					goto err;
				return 1;
			} else {
				temp = DeeInt_NewSize(old_index - count);
				if unlikely(!temp)
					goto err;
				error = DeeObject_SetAttr(self, (DeeObject *)&str_index, temp);
				Dee_Decref(temp);
			}
			if unlikely(error)
				goto err;
			return 2;
		}
		if (m) {
			if (m->tp_add && m->tp_add != &iterator_add) {
				if (count_ob) {
					temp2 = count_ob;
					Dee_Incref(temp2);
				} else {
					temp2 = DeeInt_NewSize(count);
					if unlikely(!temp2)
						goto err;
				}
				temp = (*m->tp_add)(self, temp2);
				Dee_Decref(temp2);
				if unlikely(!temp)
					goto err;
				if (DeeObject_MoveAssign(self, temp))
					goto err_temp;
				Dee_Decref(temp);
				goto done;
			}
			if (m->tp_sub && m->tp_sub != &iterator_sub) {
				if (minus_count_ob) {
					temp2 = minus_count_ob;
					Dee_Incref(temp2);
				} else {
					temp2 = DeeInt_NewSSize(-(Dee_ssize_t)count);
					if unlikely(!temp2)
						goto err;
				}
				temp = (*m->tp_sub)(self, temp2);
				Dee_Decref(temp2);
				if unlikely(!temp)
					goto err;
				if (DeeObject_MoveAssign(self, temp))
					goto err_temp;
				Dee_Decref(temp);
				goto done;
			}
			if (m->tp_dec && m->tp_dec != &iterator_dec) {
				temp = self;
				Dee_Incref(temp);
				for (;;) {
					if (DeeThread_CheckInterrupt())
						goto err_temp;
					if unlikely((*m->tp_dec)(&temp))
						goto err_temp;
					if (!--count)
						break;
					temp2 = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
					if unlikely(!temp2)
						goto err_temp;
					error = DeeObject_BoolInherited(temp2);
					if (error <= 0) {
						if unlikely(error < 0)
							goto err;
						if (temp != self && DeeObject_MoveAssign(self, temp))
							goto err_temp;
						Dee_Decref(temp);
						return 1;
					}
				}
				if (temp != self && DeeObject_MoveAssign(self, temp))
					goto err_temp;
				Dee_Decref(temp);
				goto done;
			}
			error = has_generic_attribute(tp_self, self, (DeeObject *)&str_seq);
			if (error == 0)
				error = has_generic_attribute(tp_self, self, (DeeObject *)&str_prev);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				for (;;) {
					if (DeeThread_CheckInterrupt())
						goto err;
					temp = DeeObject_CallAttr(self, (DeeObject *)&str_prev, 0, NULL);
					if unlikely(!temp)
						goto err;
					error = DeeObject_BoolInherited(temp);
					if (error <= 0) {
						if unlikely(error < 0)
							goto err;
						return 1;
					}
					if (!--count)
						break;
				}
				goto done;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_not_bidirectional(self);
	goto err;
err_temp:
	Dee_Decref(temp);
err:
	return -1;
done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
iterator_do_advance(DeeObject *__restrict self, size_t count,
                    /*[0..1]*/ DeeObject *count_ob,
                    /*[0..1]*/ DeeObject *minus_count_ob) {
	DeeTypeObject *tp_self;
	DREF DeeObject *temp, *temp2;
	int error;
	DeeTypeMRO mro;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		struct type_math *m;
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_advance)
				return (*nii->nii_common.nii_advance)(self, count);
			if (nii->nii_common.nii_getindex &&
			    nii->nii_common.nii_setindex) {
				size_t index;
				index = (*nii->nii_common.nii_getindex)(self);
				if unlikely(index == (size_t)-1)
					goto err;
				if unlikely(index == (size_t)-2)
					goto done;
				if (OVERFLOW_UADD(index, count, &index))
					index = (size_t)-1;
				error = (*nii->nii_common.nii_setindex)(self, index);
				if unlikely(error)
					goto err;
				goto done;
			}
			if (nii->nii_common.nii_next) {
				for (;;) {
					error = (*nii->nii_common.nii_next)(self);
					if unlikely(error < 0)
						goto err;
					if (error)
						return 1;
					if (!--count)
						break;
				}
				goto done;
			}
			break;
		}
		m = tp_self->tp_math;
		if (m &&
		    (m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add)) {
			if (count_ob) {
				temp2 = count_ob;
				Dee_Incref(temp2);
			} else {
				temp2 = DeeInt_NewSize(count);
				if unlikely(!temp2)
					goto err;
			}
			temp = self;
			Dee_Incref(self);
			error = (*m->tp_inplace_add)(&temp, temp2);
			Dee_Decref(temp2);
			if unlikely(error)
				goto err_temp;
			if (temp != self &&
			    DeeObject_MoveAssign(self, temp))
				goto err_temp;
			Dee_Decref(temp);
			goto done;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_advance);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			if (count_ob) {
				temp = count_ob;
				Dee_Incref(temp);
			} else {
				temp = DeeInt_NewSize(count);
				if unlikely(!temp)
					goto err;
			}
			temp2 = DeeObject_CallAttr(self, (DeeObject *)&str_advance, 1, &temp);
			if unlikely(!temp2)
				goto err_temp;
			Dee_Decref(temp2);
			Dee_Decref(temp);
			goto done;
		}
		if (m &&
		    (m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub)) {
			if (minus_count_ob) {
				temp2 = minus_count_ob;
				Dee_Incref(temp2);
			} else {
				temp2 = DeeInt_NewSSize(-(Dee_ssize_t)count);
				if unlikely(!temp2)
					goto err;
			}
			temp = self;
			Dee_Incref(self);
			error = (*m->tp_inplace_sub)(&temp, temp2);
			Dee_Decref(temp2);
			if unlikely(error)
				goto err_temp;
			if (temp != self &&
			    DeeObject_MoveAssign(self, temp))
				goto err_temp;
			Dee_Decref(temp);
			goto done;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_revert);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			if (minus_count_ob) {
				temp = minus_count_ob;
				Dee_Incref(temp);
			} else {
				temp = DeeInt_NewSSize(-(Dee_ssize_t)count);
				if unlikely(!temp)
					goto err;
			}
			temp2 = DeeObject_CallAttr(self, (DeeObject *)&str_revert, 1, &temp);
			if unlikely(!temp2)
				goto err_temp;
			Dee_Decref(temp2);
			Dee_Decref(temp);
			goto done;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_index);
		if (temp != ITER_DONE) {
			size_t old_index;
			if unlikely(!temp)
				goto err;
			if (DeeObject_AsSize(temp, &old_index))
				goto err_temp;
			Dee_Decref(temp);
			temp = DeeInt_NewSize(old_index + count);
			if unlikely(!temp)
				goto err;
			error = DeeObject_SetAttr(self, (DeeObject *)&str_index, temp);
			Dee_Decref(temp);
			if unlikely(error)
				goto err;
			goto done;
		}
		if (m) {
			if (m->tp_add && m->tp_add != &iterator_add) {
				if (count_ob) {
					temp2 = count_ob;
					Dee_Incref(temp2);
				} else {
					temp2 = DeeInt_NewSize(count);
					if unlikely(!temp2)
						goto err;
				}
				temp = (*m->tp_add)(self, temp2);
				Dee_Decref(temp2);
				if unlikely(!temp)
					goto err;
				if (DeeObject_MoveAssign(self, temp))
					goto err_temp;
				Dee_Decref(temp);
				goto done;
			}
			if (m->tp_sub && m->tp_sub != &iterator_sub) {
				if (minus_count_ob) {
					temp2 = minus_count_ob;
					Dee_Incref(temp2);
				} else {
					temp2 = DeeInt_NewSSize(-(Dee_ssize_t)count);
					if unlikely(!temp2)
						goto err;
				}
				temp = (*m->tp_sub)(self, temp2);
				Dee_Decref(temp2);
				if unlikely(!temp)
					goto err;
				if (DeeObject_MoveAssign(self, temp))
					goto err_temp;
				Dee_Decref(temp);
				goto done;
			}
			if (m->tp_inc && m->tp_inc != &iterator_inc) {
				temp = self;
				Dee_Incref(temp);
				for (;;) {
					if (DeeThread_CheckInterrupt())
						goto err_temp;
					if unlikely((*m->tp_inc)(&temp))
						goto err_temp;
					if (!--count)
						break;
					temp2 = DeeObject_GetAttr(self, (DeeObject *)&str_hasnext);
					if unlikely(!temp2)
						goto err_temp;
					error = DeeObject_BoolInherited(temp2);
					if (error <= 0) {
						if unlikely(error < 0)
							goto err;
						if (temp != self && DeeObject_MoveAssign(self, temp))
							goto err_temp;
						Dee_Decref(temp);
						return 1;
					}
				}
				if (temp != self && DeeObject_MoveAssign(self, temp))
					goto err_temp;
				Dee_Decref(temp);
				goto done;
			}
		}
		if (tp_self->tp_iter_next) {
			for (;;) {
				if (DeeThread_CheckInterrupt())
					goto err;
				temp = (*tp_self->tp_iter_next)(self);
				if (!ITER_ISOK(temp)) {
					if unlikely(!temp)
						goto err;
					return 1;
				}
				Dee_Decref(temp);
				if (!--count)
					break;
			}
			goto done;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
done:
	return 0;
err_temp:
	Dee_Decref(temp);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_revert(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t count;
	int error;
	if (DeeArg_UnpackStruct(argc, argv, UNPdSIZ ":revert", &count))
		goto err;
	if unlikely(count == 0)
		goto done;
	error = unlikely(count < 0)
	        ? iterator_do_advance(self, (size_t)-count, NULL, argv[0])
	        : likely(count > 0)
	          ? iterator_do_revert(self, (size_t)count, argv[0], NULL)
	          : 0;
	if unlikely(error < 0)
		goto err;
done:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_advance(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t count;
	int error;
	if (DeeArg_UnpackStruct(argc, argv, UNPdSIZ ":advance", &count))
		goto err;
	if unlikely(count == 0)
		goto done;
	error = unlikely(count < 0)
	        ? iterator_do_revert(self, (size_t)-count, NULL, argv[0])
	        : likely(count > 0)
	          ? iterator_do_advance(self, (size_t)count, argv[0], NULL)
	          : 0;
	if unlikely(error < 0)
		goto err;
done:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_peek(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *defl = NULL;
	_DeeArg_Unpack0Or1(err, argc, argv, "peek", &defl);
	result = DeeIterator_Peek(self);
	if (result == ITER_DONE) {
		if (!defl)
			goto err_stop;
		result = defl;
		Dee_Incref(defl);
	}
	return result;
err_stop:
	DeeError_Throw(&DeeError_StopIteration_instance);
err:
	return NULL;
}


PRIVATE struct type_method tpconst iterator_methods[] = {
	TYPE_METHOD("next", &iterator_next,
	            "(defl?)->\n"
	            "#tStopIteration{@this Iterator has been exhausted, and no @defl was given}"
	            "Same as ${this.operator next()}\n"
	            "When given, @defl is returned when the Iterator has been "
	            /**/ "exhausted, rather than throwing a :StopIteration error"),
	TYPE_METHOD(STR_peek, &iterator_peek,
	            "(defl?)->\n"
	            "#tStopIteration{@this Iterator has been exhausted, and no @defl was given}"
	            "Peek the next upcoming object, but don't advance to it\n"
	            "${"
	            /**/ "function peek(defl?) {\n"
	            /**/ "	local c = copy this;\n"
	            /**/ "	try {\n"
	            /**/ "		return c.operator next();\n"
	            /**/ "	} catch (StopIteration) {\n"
	            /**/ "		if (defl is bound)\n"
	            /**/ "			return defl;\n"
	            /**/ "		throw;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_prev, &iterator_prev,
	            "->?Dbool\n"
	            "#tNotImplemented{@this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	            "Rewind @this Iterator to the previous item, returning ?f if "
	            /**/ "@this Iterator had already been positioned at the start of its sequence, "
	            /**/ "or ?t otherwise\n"

	            "${"
	            /**/ "function prev(): bool {\n"
	            /**/ "	for (local tp: type(this).__mro__) {\n"
	            /**/ "		if (tp === Iterator)\n"
	            /**/ "			break;\n"
	            /**/ "		if (tp.hasprivateattribute(\"index\")) {\n"
	            /**/ "			local i = this.index;\n"
	            /**/ "			if (!i)\n"
	            /**/ "				return false;\n"
	            /**/ "			this.index = i - 1;\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"dec\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			--this;\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"isub\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			this -= 1;\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"iadd\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			this += -1;\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"sub\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			this.operator move := (this - 1);\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"add\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			this.operator move := (this + (-1));\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"revert\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			this.revert(1);\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"advance\")) {\n"
	            /**/ "			if (!this.hasprev)\n"
	            /**/ "				return false;\n"
	            /**/ "			this.advance(-1);\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"rewind\")) {\n"
	            /**/ "			local c = copy this;\n"
	            /**/ "			c.rewind();\n"
	            /**/ "			if (c == this)\n"
	            /**/ "				return false;\n"
	            /**/ "			for (;;) {\n"
	            /**/ "				local d = copy c;\n"
	            /**/ "				try d.operator next();\n"
	            /**/ "				catch (StopIteration) {\n"
	            /**/ "					return false;\n"
	            /**/ "				}\n"
	            /**/ "				if (d >= this)\n"
	            /**/ "					break;\n"
	            /**/ "				c = d;\n"
	            /**/ "			}\n"
	            /**/ "			this.operator move := (c);\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"seq\")) {\n"
	            /**/ "			local c = this.seq.operator iter();\n"
	            /**/ "			if (c == this)\n"
	            /**/ "				return false;\n"
	            /**/ "			for (;;) {\n"
	            /**/ "				local d = copy c;\n"
	            /**/ "				try d.operator next();\n"
	            /**/ "				catch (Signal.StopIteration) {\n"
	            /**/ "					return false;\n"
	            /**/ "				}\n"
	            /**/ "				if (d >= this)\n"
	            /**/ "					break;\n"
	            /**/ "				c = d;\n"
	            /**/ "			}\n"
	            /**/ "			this.operator move := (c);\n"
	            /**/ "			return true;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	throw NotImplemented(\"...\");\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_rewind, &iterator_rewind,
	            "()\n"
	            "#tNotImplemented{@this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	            "Rewind @this Iterator to the start of its sequence\n"

	            "${"
	            /**/ "function rewind() {\n"
	            /**/ "	for (local tp: type(this).__mro__) {\n"
	            /**/ "		if (tp === Iterator)\n"
	            /**/ "			break;\n"
	            /**/ "		if (tp.hasprivateattribute(\"index\")) {\n"
	            /**/ "			this.index = 0;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"seq\")) {\n"
	            /**/ "			this.operator move := (this.seq.operator iter());\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"prev\")) {\n"
	            /**/ "			while (this.prev())\n"
	            /**/ "				;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"isub\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				this -= int.SIZE_MAX;\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"iadd\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				this += -int.SIZE_MAX;\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"dec\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				--this;\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"revert\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				this.revert(int.SIZE_MAX);\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"advance\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				this.advance(-int.SIZE_MAX);\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"sub\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				this.operator move := (this - int.SIZE_MAX);\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"add\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				this.operator move := (this + (-int.SIZE_MAX));\n"
	            /**/ "			} while (this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	throw NotImplemented(\"...\");\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_revert, &iterator_revert,
	            "(step:?Dint)\n"
	            "#tNotImplemented{@step is positive, and @this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	            "Revert @this Iterator by @step items\n"
	            "${"
	            /**/ "function revert(step: int) {\n"
	            /**/ "	step = step.operator int();\n"
	            /**/ "	if (step == 0)\n"
	            /**/ "		return;\n"
	            /**/ "	if (step < 0)\n"
	            /**/ "		return Iterator.advance(this, -step);\n"
	            /**/ "	for (local tp: type(this).__mro__) {\n"
	            /**/ "		if (tp === Iterator)\n"
	            /**/ "			break;\n"
	            /**/ "		if (tp.hasprivateoperator(\"isub\")) {\n"
	            /**/ "			this -= step;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"iadd\")) {\n"
	            /**/ "			this += -step;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"advance\")) {\n"
	            /**/ "			this.advance(-step);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"index\")) {\n"
	            /**/ "			local new_index = this.index - step;\n"
	            /**/ "			if (new_index < 0)\n"
	            /**/ "				new_index = type(new_index)();\n"
	            /**/ "			this.index = new_index;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"sub\")) {\n"
	            /**/ "			this.operator move := (this - step);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"add\")) {\n"
	            /**/ "			this.operator move := (this + (-step));\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"dec\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				--this;\n"
	            /**/ "			} while (--step && this.hasprev);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"seq\") || tp.hasprivateattribute(\"prev\")) {\n"
	            /**/ "			while (this.prev() && --step)\n"
	            /**/ "				;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	throw NotImplemented(\"...\");\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_advance, &iterator_advance,
	            "(step:?Dint)\n"
	            "#tNotImplemented{@step is negative, and @this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	            "Revert @this Iterator by @step items\n"
	            "${"
	            /**/ "function advance(step: int) {\n"
	            /**/ "	step = step.operator int();\n"
	            /**/ "	if (step == 0)\n"
	            /**/ "		return;\n"
	            /**/ "	if (step < 0)\n"
	            /**/ "		return Iterator.revert(this, -step);\n"
	            /**/ "	for (local tp: type(this).__mro__) {\n"
	            /**/ "		if (tp === Iterator)\n"
	            /**/ "			break;\n"
	            /**/ "		if (tp.hasprivateoperator(\"iadd\")) {\n"
	            /**/ "			this += step;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"isub\")) {\n"
	            /**/ "			this -= -step;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"revert\")) {\n"
	            /**/ "			this.revert(-step);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"index\")) {\n"
	            /**/ "			local new_index = this.index + step;\n"
	            /**/ "			if (new_index < 0)\n"
	            /**/ "				new_index = type(new_index)();\n"
	            /**/ "			this.index = new_index;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"add\")) {\n"
	            /**/ "			this.operator move := (this + step);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"sub\")) {\n"
	            /**/ "			this.operator move := (this - -(step));\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"inc\")) {\n"
	            /**/ "			do {\n"
	            /**/ "				++this;\n"
	            /**/ "			}while (--step && this.hasnext);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivatoperator(\"next\")) {\n"
	            /**/ "			while (--step) {\n"
	            /**/ "				try {\n"
	            /**/ "					this.operator next();\n"
	            /**/ "				} catch (StopIteration) {\n"
	            /**/ "					break;\n"
	            /**/ "				}\n"
	            /**/ "			}\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	return;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD_END
};


/* Get the Iterator's position
 * @return: * :         The iterator's current position, where the a starting position is 0
 * @return: (size_t)-2: The position is indeterminate (the Iterator may have become detached
 *                      from its sequence, as can happen in linked lists when the Iterator's
 *                      link entry gets removed)
 * @return: (size_t)-1: Error */
INTERN WUNUSED NONNULL((1)) size_t DCALL
DeeIterator_GetIndex(DeeObject *__restrict self) {
	DREF DeeObject *copy, *temp;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	DeeTypeMRO mro;
	size_t index;
	int error;
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_getindex)
				return (*nii->nii_common.nii_getindex)(self);
			break;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_rewind);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			copy = DeeObject_Copy(self);
			if unlikely(!copy)
				goto err;
			temp = DeeObject_CallAttr(copy, (DeeObject *)&str_rewind, 0, NULL);
			if unlikely(!temp)
				goto err_copy;
			Dee_Decref(temp);
			goto got_rewound_iter;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_seq);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			copy = DeeObject_Iter(temp);
			Dee_Decref(temp);
			if unlikely(!copy)
				goto err;
			goto got_rewound_iter;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_not_bidirectional(self);
	goto err;
got_rewound_iter:
	index = 0;
	for (;;) {
		if (DeeThread_CheckInterrupt())
			goto err;
		error = DeeObject_CmpLoAsBool(copy, self);
		if (error <= 0) {
			if unlikely(error < 0)
				goto err_copy;
			break;
		}
		temp = DeeObject_IterNext(copy);
		if (!ITER_ISOK(temp)) {
			if unlikely(!temp)
				goto err_copy;
			return (size_t)-2;
		}
		Dee_Decref(temp);
		if unlikely(index >= (size_t)-2)
			goto err_overflow;
		++index;
	}
	Dee_Decref(copy);
	return index;
err_overflow:
	DeeRT_ErrIntegerOverflowU(index, (size_t)-3);
	goto err;
err_copy:
	Dee_Decref(copy);
err:
	return (size_t)-1;
}

/* Set the iterator's position
 * If the given `new_index' is greater than the max allowed index,
 * the iterator is set to an exhausted state (i.e. points at the
 * end of the associated sequence)
 * @return:  0: Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_SetIndex(DeeObject *__restrict self, size_t new_index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *temp;
	DeeTypeMRO mro;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_setindex)
				return (*nii->nii_common.nii_setindex)(self, new_index);
			if (nii->nii_common.nii_rewind) {
				if unlikely((*nii->nii_common.nii_rewind)(self))
					goto err;
				goto after_rewind;
			}
			break;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	temp = DeeObject_CallAttr(self, (DeeObject *)&str_rewind, 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
after_rewind:
	return !new_index ? 0 : iterator_do_advance(self, new_index, NULL, NULL);
err:
	return -1;
}


INTDEF DeeIntObject int_SIZE_MAX;
#if SSIZE_MIN >= INT32_MIN
PRIVATE DEFINE_INT32(int_SIZE_MIN, SSIZE_MIN);
#else /* SSIZE_MIN < INT32_MIN */
PRIVATE DEFINE_INT64(int_SIZE_MIN, SSIZE_MIN);
#endif /* SSIZE_MIN >= INT32_MIN */

/* Rewind the iterator to its starting position
 * @return:  0: Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_Rewind(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	DREF DeeObject *temp, *temp2, *temp3;
	DeeTypeMRO mro;
	int error;
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		struct type_math *m;
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_rewind) {
				if unlikely((*nii->nii_common.nii_rewind)(self))
					goto err;
				goto done;
			}
			if (nii->nii_common.nii_setindex) {
				if unlikely((*nii->nii_common.nii_setindex)(self, 0))
					goto err;
				goto done;
			}
			if (nii->nii_common.nii_revert) {
				for (;;) {
					error = (*nii->nii_common.nii_revert)(self, (size_t)-1);
					if unlikely(error < 0)
						goto err;
					if (error == 1)
						break;
					if (error == 2)
						continue;
					error = iterator_do_hasprev(self, nii);
					if unlikely(error < 0)
						goto err;
					if (!error)
						break;
				}
				goto done;
			}
			if (nii->nii_common.nii_prev) {
				for (;;) {
					error = (*nii->nii_common.nii_prev)(self);
					if unlikely(error < 0)
						goto err;
					if (error)
						break;
				}
				goto done;
			}
			break;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_index);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			error = DeeObject_SetAttr(self, (DeeObject *)&str_index, DeeInt_Zero);
			if unlikely(error < 0)
				goto err;
			goto done;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_seq);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			temp2 = DeeObject_Iter(temp);
			Dee_Decref(temp);
			if unlikely(!temp2)
				goto err;
			error = DeeObject_MoveAssign(self, temp2);
			Dee_Decref(temp2);
			if unlikely(error)
				goto err;
			goto done;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_prev);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			for (;;) {
				if (DeeThread_CheckInterrupt())
					goto err_temp;
				temp2 = DeeObject_Call(temp, 0, NULL);
				if unlikely(!temp2)
					goto err_temp;
				error = DeeObject_BoolInherited(temp2);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err_temp;
					break;
				}
			}
			Dee_Decref(temp);
			goto done;
		}
		m = tp_self->tp_math;
		if (m &&
		    ((m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub) ||
		     (m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add) ||
		     (m->tp_dec && m->tp_dec != &iterator_dec))) {
			temp = self;
			Dee_Incref(temp);
			for (;;) {
				error = (m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub)
				        ? (*m->tp_inplace_sub)(&temp, (DeeObject *)&int_SIZE_MAX)
				        : (m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add)
				          ? (*m->tp_inplace_add)(&temp, (DeeObject *)&int_SIZE_MIN)
				          : (*m->tp_dec)(&temp);
				if unlikely(error)
					goto err_temp;
				temp2 = DeeObject_GetAttr(temp, (DeeObject *)&str_hasprev);
				if unlikely(!temp2)
					goto err_temp;
				error = DeeObject_BoolInherited(temp2);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err_temp;
					break;
				}
			}
			if (temp != self &&
			    DeeObject_MoveAssign(self, temp))
				goto err_temp;
			Dee_Decref(temp);
			goto done;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_revert);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			for (;;) {
				if (DeeThread_CheckInterrupt())
					goto err_temp;
				temp3 = (DREF DeeObject *)&int_SIZE_MAX;
				temp2 = DeeObject_Call(temp, 1, &temp3);
				if unlikely(!temp2)
					goto err_temp;
				Dee_Decref(temp2);
				temp2 = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
				if unlikely(!temp2)
					goto err_temp;
				error = DeeObject_BoolInherited(temp2);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err_temp;
					break;
				}
			}
			Dee_Decref(temp);
			goto done;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_advance);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			for (;;) {
				if (DeeThread_CheckInterrupt())
					goto err_temp;
				temp3 = (DREF DeeObject *)&int_SIZE_MIN;
				temp2 = DeeObject_Call(temp, 1, &temp3);
				if unlikely(!temp2)
					goto err_temp;
				Dee_Decref(temp2);
				temp2 = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
				if unlikely(!temp2)
					goto err_temp;
				error = DeeObject_BoolInherited(temp2);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err_temp;
					break;
				}
			}
			Dee_Decref(temp);
			goto done;
		}
		if (m &&
		    ((m->tp_sub && m->tp_sub != &iterator_sub) ||
		     (m->tp_add && m->tp_add != &iterator_add))) {
			temp = self;
			Dee_Incref(temp);
			for (;;) {
				temp2 = (m->tp_sub && m->tp_sub != &iterator_sub)
				        ? (*m->tp_sub)(temp, (DeeObject *)&int_SIZE_MAX)
				        : (*m->tp_add)(temp, (DeeObject *)&int_SIZE_MIN);
				if unlikely(error)
					goto err_temp;
				Dee_Decref(temp);
				temp  = temp2;
				temp2 = DeeObject_GetAttr(temp, (DeeObject *)&str_hasprev);
				if unlikely(!temp2)
					goto err_temp;
				error = DeeObject_BoolInherited(temp2);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err_temp;
					break;
				}
			}
			if (DeeObject_MoveAssign(self, temp))
				goto err_temp;
			Dee_Decref(temp);
			goto done;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_not_bidirectional(self);
err:
	return -1;
err_temp:
	Dee_Decref(temp);
	goto err;
done:
	return 0;
}

/* Revert the iterator by at most `step' (When `step' is too large, same as `rewind')
 * @return:  0: Success (new relative position wasn't determined)
 * @return:  1: Success (the iterator has reached its starting position)
 * @return:  2: Success (the iterator hasn't reached its starting position)
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_Revert(DeeObject *__restrict self, size_t step) {
	return iterator_do_revert(self, step, NULL, NULL);
}

/* Advance the iterator by at most `step' (When `step' is too large, exhaust the iterator)
 * @return:  0: Success (new relative position wasn't determined)
 * @return:  1: Success (the iterator has become exhausted)
 * @return:  2: Success (the iterator hasn't become exhausted)
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_Advance(DeeObject *__restrict self, size_t step) {
	return iterator_do_advance(self, step, NULL, NULL);
}

/* Decrement the iterator by 1.
 * @return:  0: Success
 * @return:  1: The iterator was already at its starting location
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_Prev(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		DREF DeeObject *temp, *temp2;
		DREF DeeObject *new_self;
		int error;
		struct type_math *m;
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_prev)
				return (*nii->nii_common.nii_prev)(self);
			if (nii->nii_common.nii_revert) {
				error = iterator_do_hasprev(self, nii);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err;
					return 1;
				}
				if unlikely((*nii->nii_common.nii_revert)(self, 1) < 0)
					goto err;
				return 0;
			}
			if (nii->nii_common.nii_getindex &&
			    nii->nii_common.nii_setindex) {
				size_t index;
				index = (*nii->nii_common.nii_getindex)(self);
				if (index == 0 || index == (size_t)-2)
					return 1;
				if unlikely(index == (size_t)-1)
					goto err;
				if unlikely((*nii->nii_common.nii_setindex)(self, index - 1))
					goto err;
				return 0;
			}
			break;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_index);
		if (temp != ITER_DONE) {
			/* >> local i = this.index;
			 * >> if (!i)
			 * >>     return false;
			 * >> this.index = i - 1;
			 * >> return true; */
			if unlikely(!temp)
				goto err;
			error = DeeObject_Bool(temp);
			if (error <= 0) {
				Dee_Decref(temp);
				if unlikely(error < 0)
					goto err;
				return 1;
			}
			temp2 = DeeObject_Sub(temp, DeeInt_One);
			Dee_Decref(temp);
			if unlikely(!temp2)
				goto err;
			error = DeeObject_SetAttr(self, (DeeObject *)&str_index, temp2);
			Dee_Decref(temp2);
			return error;
		}
		if ((m = tp_self->tp_math) != NULL) {
			if ((m->tp_dec && m->tp_dec != &iterator_dec) ||
			    (m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub) ||
			    (m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add)) {
				/* >> if (tp.hasprivateoperator("dec") {
				 * >>     if (!this.hasprev)
				 * >>         return false;
				 * >>     --this;
				 * >>     return true;
				 * >> }
				 */
				temp = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
				if unlikely(!temp)
					goto err;
				error = DeeObject_BoolInherited(temp);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err;
					return 1;
				}
				new_self = self;
				Dee_Incref(new_self);
				if (m->tp_dec && m->tp_dec != &iterator_dec) {
					error = (*m->tp_dec)(&new_self);
				} else if (m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub) {
					error = (*m->tp_inplace_sub)(&new_self, DeeInt_One);
				} else {
					ASSERT(m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add);
					error = (*m->tp_inplace_add)(&new_self, DeeInt_MinusOne);
				}
				if unlikely(error) {
err_new_self:
					Dee_Decref(new_self);
					goto err;
				}
				if (new_self != self &&
				    DeeObject_MoveAssign(self, new_self))
					goto err_new_self;
				Dee_Decref(new_self);
				return 0;
			} else if ((m->tp_sub && m->tp_sub != &iterator_sub) ||
			           (m->tp_add && m->tp_add != &iterator_add)) {
				/* >> if (tp.hasprivateoperator("sub") {
				 * >>     if (!this.hasprev)
				 * >>         return false;
				 * >>     this.operator move := (this - 1);
				 * >>     return true;
				 * >> }
				 */
				temp = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
				if unlikely(!temp)
					goto err;
				error = DeeObject_BoolInherited(temp);
				if (error <= 0) {
					if unlikely(error < 0)
						goto err;
					return 1;
				}
				if (m->tp_sub && m->tp_sub != &iterator_sub) {
					temp = (*m->tp_sub)(self, DeeInt_One);
				} else {
					ASSERT(m->tp_add && m->tp_add != &iterator_add);
					temp = (*m->tp_add)(self, DeeInt_MinusOne);
				}
				if unlikely(!temp)
					goto err;
				error = DeeObject_MoveAssign(self, temp);
				Dee_Decref(temp);
				return error;
			}
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_revert);
		if (error != 0) {
			/* >> if (!this.hasprev)
			 * >>     return false;
			 * >> this.revert(1);
			 * >> return true;
			 */
			if unlikely(error < 0)
				goto err;
			temp = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
			if unlikely(!temp)
				goto err;
			error = DeeObject_BoolInherited(temp);
			if (error <= 0) {
				if unlikely(error < 0)
					goto err;
				return 1;
			}
			new_self = DeeInt_One;
			temp     = DeeObject_CallAttr(self, (DeeObject *)&str_revert, 1, &new_self);
			if unlikely(!temp)
				goto err;
			Dee_Decref(temp);
			return 0;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_advance);
		if (error != 0) {
			/* >> if (!this.hasprev)
			 * >>     return false;
			 * >> this.advance(-1);
			 * >> return true;
			 */
			if unlikely(error < 0)
				goto err;
			temp = DeeObject_GetAttr(self, (DeeObject *)&str_hasprev);
			if unlikely(!temp)
				goto err;
			error = DeeObject_BoolInherited(temp);
			if (error <= 0) {
				if unlikely(error < 0)
					goto err;
				return 1;
			}
			new_self = DeeInt_MinusOne;
			temp     = DeeObject_CallAttr(self, (DeeObject *)&str_advance, 1, &new_self);
			if unlikely(!temp)
				goto err;
			Dee_Decref(temp);
			return 0;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_rewind);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			new_self = DeeObject_Copy(self);
			if unlikely(!new_self)
				goto err;
			temp = DeeObject_CallAttr(new_self, (DeeObject *)&str_rewind, 0, NULL);
			if unlikely(!temp)
				goto err_new_self;
			Dee_Decref(temp);
do_prev_with_rewind_iterator:
			/* >> if (c == this)
			 * >>     return false;
			 * >> for (;;) {
			 * >>     local d = copy c;
			 * >>     try d.operator next();
			 * >>     catch (StopIteration) {
			 * >>         return false;
			 * >>     }
			 * >>     if (d >= this)
			 * >>         break;
			 * >>     c = d;
			 * >> }
			 * >> this.operator move := (c);
			 * >> return true; */
			error = DeeObject_CmpEqAsBool(new_self, self);
			if (error != 0) {
				if unlikely(error < 0)
					goto err_new_self;
				Dee_Decref(new_self);
				return 1;
			}
			for (;;) {
				DREF DeeObject *new_copy;
				if (DeeThread_CheckInterrupt())
					goto err_new_self;
				new_copy = DeeObject_Copy(new_self);
				if unlikely(!new_copy)
					goto err_new_self;
				temp = DeeObject_IterNext(new_copy);
				if unlikely(!ITER_ISOK(temp)) {
					Dee_Decref(new_copy);
					Dee_Decref(new_self);
					if unlikely(!temp)
						goto err;
					return 1;
				}
				error = DeeObject_CmpGeAsBool(new_copy, self);
				if (error != 0) {
					Dee_Decref(new_copy);
					if unlikely(error < 0)
						goto err_new_self;
					break;
				}
				Dee_Decref(new_self);
				new_self = new_copy;
			}
			error = DeeObject_MoveAssign(self, new_self);
			Dee_Decref(new_self);
			return error;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_seq);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			new_self = DeeObject_Iter(temp);
			Dee_Decref(temp);
			if unlikely(!new_self)
				goto err;
			goto do_prev_with_rewind_iterator;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_not_bidirectional(self);
err:
	return -1;
}

/* Increment the iterator, but don't generate a value
 * NOTE: Unlike `tp_iter_next()', this operator shouldn't skip unbound entries,
 *       meaning that (also unlike `tp_iter_next()'), the iterator's index should
 *       only ever be incremented by 1.
 * @return:  0: Success
 * @return:  1: The iterator had already been exhausted
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_Next(DeeObject *__restrict self) {
	return iterator_do_advance(self, 1, DeeInt_One, DeeInt_MinusOne);
}

/* Check if the iterator is at its starting location
 * @return:  0: No, it isn't
 * @return:  1: Yes, it is
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeIterator_HasPrev(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		DREF DeeObject *temp, *temp2;
		int error;
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_hasprev)
				return (*nii->nii_common.nii_hasprev)(self);
			if (nii->nii_common.nii_getindex) {
				size_t index;
				index = (*nii->nii_common.nii_getindex)(self);
				if unlikely(index == (size_t)-1)
					goto err;
				return index != 0;
			}
			break;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_index);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			error = DeeObject_CmpEqAsBool(temp, DeeInt_Zero);
			Dee_Decref(temp);
			if likely(error >= 0)
				error = error ? 0 : 1;
			return error;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_prev);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			temp = DeeObject_Copy(self);
			if unlikely(!temp)
				goto err;
			temp2 = DeeObject_CallAttr(temp, (DeeObject *)&str_prev, 0, NULL);
			Dee_Decref(temp);
			if unlikely(!temp2)
				goto err;
			return DeeObject_BoolInherited(temp2);
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_rewind);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			temp = DeeObject_Copy(self);
			if unlikely(!temp)
				goto err;
			temp2 = DeeObject_CallAttr(temp, (DeeObject *)&str_rewind, 0, NULL);
			if unlikely(!temp2) {
				Dee_Decref(temp);
				goto err;
			}
			Dee_Decref(temp2);
			error = DeeObject_CmpEqAsBool(self, temp);
			Dee_Decref(temp);
			if likely(error >= 0)
				error = error ? 0 : 1;
			return error;
		}
		temp = get_generic_attribute(tp_self, self, (DeeObject *)&str_seq);
		if (temp != ITER_DONE) {
			if unlikely(!temp)
				goto err;
			temp2 = DeeObject_Iter(temp);
			Dee_Decref(temp);
			if unlikely(!temp2)
				goto err;
			error = DeeObject_CmpEqAsBool(self, temp2);
			Dee_Decref(temp2);
			if likely(error >= 0)
				error = error ? 0 : 1;
			return error;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_not_bidirectional(self);
err:
	return -1;
}

/* Peek the next iterator value, but don't actually advance the iterator.
 * @return: ITER_DONE: The iterator has already been exhausted. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeIterator_Peek(DeeObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *temp;
	int error;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	while (tp_self != &DeeIterator_Type) {
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_peek)
				return (*nii->nii_common.nii_peek)(self);
			if (nii->nii_common.nii_prev) {
				result = DeeObject_IterNext(self);
				if (ITER_ISOK(result) &&
				    unlikely((*nii->nii_common.nii_prev)(self)))
					goto err_r;
				return result;
			}
			if (nii->nii_common.nii_revert) {
				result = DeeObject_IterNext(self);
				if (ITER_ISOK(result) &&
				    unlikely((*nii->nii_common.nii_revert)(self, 1)))
					goto err_r;
				return result;
			}
			if (nii->nii_common.nii_getindex &&
			    nii->nii_common.nii_setindex) {
				size_t index = (*nii->nii_common.nii_getindex)(self);
				if unlikely(index == (size_t)-1)
					goto err;
				if unlikely(index == (size_t)-2)
					return ITER_DONE;
				result = DeeObject_IterNext(self);
				if (ITER_ISOK(result) &&
				    unlikely((*nii->nii_common.nii_setindex)(self, index)))
					goto err_r;
				return result;
			}
			break;
		}
		error = has_generic_attribute(tp_self, self, (DeeObject *)&str_peek);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			result = DeeObject_CallAttr(self, (DeeObject *)&str_peek, 0, NULL);
			if (!result && DeeError_Catch(&DeeError_StopIteration))
				result = ITER_DONE;
			return result;
		}
	}
	temp = DeeObject_Copy(self);
	if unlikely(!temp)
		goto err;
	result = DeeObject_IterNext(temp);
	Dee_Decref(temp);
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL IteratorFuture_For(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL IteratorPending_For(DeeObject *__restrict self);

PRIVATE DeeStringObject *tpconst bidirectional_iterator_attributes[] = {
	&str_revert,
	&str_advance,
	&str_index,
	&str_prev,
	&str_rewind,
};


/* Return the sequence associated with the Iterator, or NULL on error.
 * NOTE: Alternatively, a getset/member `seq' may be defined for this. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeIterator_GetSeq(DeeObject *__restrict self) {
	DREF DeeObject *result;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeIterator_Type)
		return DeeSeq_NewEmpty();
	while (tp_self != &DeeIterator_Type) {
		struct type_nii const *nii;
		if (tp_self->tp_cmp &&
		    (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_getseq)
				return (*nii->nii_common.nii_getseq)(self);
		}
		result = get_generic_attribute(tp_self, self, (DeeObject *)&str_seq);
		if (result != ITER_DONE)
			return result;
	}
/*err_noseq:*/
	DeeRT_ErrUnknownAttr(self, &str_seq, DeeRT_ATTRIBUTE_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
iterator_is_bidirectional(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		size_t i;
		struct type_cmp *c;
		struct type_math *m;
		c = tp_self->tp_cmp;
		if (c && c->tp_nii)
			return c->tp_nii->nii_class == TYPE_ITERX_CLASS_BIDIRECTIONAL;
		m = tp_self->tp_math;
		if (m) {
			if (m->tp_dec && m->tp_dec != &iterator_dec)
				goto yes;
			if (m->tp_inplace_sub && m->tp_inplace_sub != &iterator_inplace_sub)
				goto yes;
			if (m->tp_sub && m->tp_sub != &iterator_sub)
				goto yes;
			if (m->tp_add && m->tp_add != &iterator_add)
				goto yes;
			if (m->tp_inplace_add && m->tp_inplace_add != &iterator_inplace_add)
				goto yes;
		}
		for (i = 0; i < COMPILER_LENOF(bidirectional_iterator_attributes); ++i) {
			int temp = has_generic_attribute(tp_self, self, (DeeObject *)bidirectional_iterator_attributes[i]);
			if (temp != 0)
				return temp;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	return 0;
yes:
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_get_isbidirectional(DeeObject *__restrict self) {
	int temp = iterator_is_bidirectional(self);
	if unlikely(temp < 0)
		goto err;
	return_bool(temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_get_hasnext(DeeObject *__restrict self) {
	int temp = DeeObject_Bool(self);
	if unlikely(temp < 0)
		goto err;
	return_bool(temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_get_hasprev(DeeObject *__restrict self) {
	int error = DeeIterator_HasPrev(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_get_index(DeeObject *__restrict self) {
	size_t result = DeeIterator_GetIndex(self);
	if unlikely(result == (size_t)-1)
		goto err;
	if unlikely(result == (size_t)-2)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iterator_set_index(DeeObject *__restrict self,
                   DeeObject *__restrict indexob) {
	DeeTypeObject *tp_self;
	DREF DeeObject *temp;
	size_t newindex;
	DeeTypeMRO mro;
	if (DeeObject_AsSize(indexob, &newindex))
		goto err;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeIterator_Type) {
		struct type_nii const *nii;
		if (tp_self->tp_cmp && (nii = tp_self->tp_cmp->tp_nii) != NULL) {
			if (nii->nii_common.nii_setindex)
				return (*nii->nii_common.nii_setindex)(self, newindex);
			if (nii->nii_common.nii_rewind) {
				if unlikely((*nii->nii_common.nii_rewind)(self))
					goto err;
				goto after_rewind;
			}
			break;
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	temp = DeeObject_CallAttr(self, (DeeObject *)&str_rewind, 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
after_rewind:
	return !newindex ? 0 : iterator_do_advance(self, newindex, indexob, NULL);
err:
	return -1;
}




PRIVATE struct type_getset tpconst iterator_getsets[] = {
	TYPE_GETTER(STR_seq, &DeeIterator_GetSeq,
	            "->?DSequence\n"
	            "Returns the underlying sequence that is being iterated\n"
	            "Since use of this member isn't all too common, sub-classes are allowed "
	            /**/ "to (and sometimes do) not return the exact original sequence, but rather "
	            /**/ "a sequence that is syntactically equivalent (i.e. contains the same items)\n"
	            "Because of this you should not expect their ids to equal, or even their "
	            /**/ "types for that matter. Only expect the contained items, as well as their "
	            /**/ "order to be identical"),
	TYPE_GETTER_AB("future", &IteratorFuture_For,
	               "->?DSequence\n"
	               "Returns an abstract sequence proxy that always refers to the items that are "
	               /**/ "still left to be yielded by @this Iterator. Note that for this to function "
	               /**/ "properly, the Iterator must be copyable.\n"
	               "Also note that as soon as more items are iterated from @this Iterator, those "
	               /**/ "items will disappear from its future sequence immediately.\n"
	               "In the end, this property will simply return a proxy-type derived from :Sequence, "
	               /**/ "who's ${operator iter} is set up to return a copy of the pointed-to Iterator.\n"
	               "${"
	               /**/ "local x = [10, 20, 30];\n"
	               /**/ "local it = x.operator iter();\n"
	               /**/ "print repr it.future;     /* { 10, 20, 30 } */\n"
	               /**/ "print it.operator next(); /* 10 */\n"
	               /**/ "print repr it.future;     /* { 20, 30 } */"
	               "}"),
	TYPE_GETTER_AB("pending", &IteratorPending_For,
	               "->?DSequence\n"
	               "Very similar to ?#future, however the when invoking ${operator iter} on "
	               /**/ "the returned sequence, rather than having it return a copy of @this Iterator, "
	               /**/ "re-return the exact same Iterator, allowing the use of this member for iterators "
	               /**/ "that don't implement a way of being copied\n"

	               "${"
	               /**/ "local x = [10, 20, 30];\n"
	               /**/ "local it = x.operator iter();\n"
	               /**/ "print it.operator next(); /* 10 */\n"
	               /**/ "print repr it.pending;    /* { 20, 30 } */\n"
	               /**/ "/* ERROR: Signal.StopIteration.\n"
	               /**/ " *        The `repr' used the same Iterator,\n"
	               /**/ " *        which consumed all remaining items */\n"
	               /**/ "print it.operator next();"
	               "}"),
	TYPE_GETTER("isbidirectional", &iterator_get_isbidirectional,
	            "->?Dbool\n"
	            "Check if @this Iterator can be used as bi-directional\n"
	            "In order for this to be possible, a sub-class of :Iterator must implement at least "
	            /**/ "one of the following members, at which point the entire bi-directional Iterator "
	            /**/ "function set becomes available. Also note that for full support, an Iterator should "
	            /**/ "also implement ?#seq, ?#{op:assign}, ?#{op:copy} and ?#{op:eq} or ?#{op:ne}\n"
	            "#T{Function|Prototype|Description~"
	            /**/ "?#{op:dec}|${operator -- (): Iterator}|Decrement the Iterator by one&"
	            /**/ "?#{op:isub}|${operator -= (step: int): Iterator}|Revert the Iterator by $step&"
	            /**/ "?#{op:sub}|${operator - (step: int): Iterator}|Create a new iterator reverted by $step&"
	            /**/ "?#{op:iadd}|${operator += (step: int): Iterator}|Advance the Iterator by $step (which may be negative)&"
	            /**/ "?#{op:add}|${operator + (step: int): Iterator}|Create a new iterator advanced by $step (which may be negative)&"
	            /**/ "?#revert|${function revert(step: int)}|Revert the Iterator by $step (same as ?#{op:isub})&"
	            /**/ "?#advance|${function advance(step: int)}|Advance the Iterator by $step (which may be negative) (same as ?#{op:iadd})&"
	            /**/ "?#index|${property index: int = { get(); set(); }}|Get/set the exact index of the Iterator within its sequence&"
	            /**/ "?#prev|${function prev(): bool}|Decrement the Iterator's position, returning ?f if the Iterator had already been fully unwound and ?t otherwise&"
	            /**/ "?#rewind|${function rewind()}|Rewind the Iterator fully"
	            "}\n"
	            "The minimum requirement for access to the entire feature-set is provision of ?#index and ?#{op:next}, "
	            /**/ "at which point all functions will have become unlocked\n"
	            "Hint: If a sub-class wishes to implement any of the above functions, without being considered "
	            /**/ "to be bi-directional, it should override this property and have it evaluate to ?f"),
	TYPE_GETTER(STR_hasprev, &iterator_get_hasprev,
	            "->?Dbool\n"
	            "#tNotImplemented{@this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	            "Returns ?t if @this Iterator has a predecessor\n"
	            "${"
	            /**/ "property hasprev: bool = {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		for (local tp: type(this).__mro__) {\n"
	            /**/ "			if (tp === Iterator)\n"
	            /**/ "				break;\n"
	            /**/ "			if (tp.hasprivateattribute(\"index\"))\n"
	            /**/ "				return this.index != 0;\n"
	            /**/ "			if (tp.hasprivateattribute(\"prev\")) {\n"
	            /**/ "				local c = copy this;\n"
	            /**/ "				return c.prev();\n"
	            /**/ "			}\n"
	            /**/ "			if (tp.hasprivateattribute(\"rewind\")) {\n"
	            /**/ "				local c = copy this;\n"
	            /**/ "				c.rewind();\n"
	            /**/ "				return this != c;\n"
	            /**/ "			}\n"
	            /**/ "			if (tp.hasprivateattribute(\"seq\")) {\n"
	            /**/ "				local c = this.seq.operator iter();\n"
	            /**/ "				return this != c;\n"
	            /**/ "			}\n"
	            /**/ "		}\n"
	            /**/ "		throw NotImplemented(\"...\");\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER(STR_hasnext, &iterator_get_hasnext,
	            "->?Dbool\n"
	            "Returns ?t if @this Iterator has a successor (alias for ?#{op:bool})"),
	TYPE_GETSET(STR_index, &iterator_get_index, NULL, &iterator_set_index,
	            "->?Dint\n"
	            "#tNotImplemented{@this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	            "Get/set the current sequence index of @this Iterator\n"
	            "Note however that depending on the type of sequence, certain indices "
	            /**/ "may not have values bound to them. When invoked, ?#{op:next} usually skips "
	            /**/ "ahead to the first bound index, meaning that ?#{op:next} does not necessarily "
	            /**/ "increment the index counter linearly\n"
	            "${"
	            /**/ "property index: int = {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		local result = 0;\n"
	            /**/ "		local c;\n"
	            /**/ "		for (local tp: type(this).__mro__) {\n"
	            /**/ "			if (tp === Iterator)\n"
	            /**/ "				break;\n"
	            /**/ "			if (tp.hasprivateattribute(\"rewind\")) {\n"
	            /**/ "				c = copy this;\n"
	            /**/ "				c.rewind();\n"
	            /**/ "				goto got_rewound_iter;\n"
	            /**/ "			}\n"
	            /**/ "			if (tp.hasprivateattribute(\"seq\")) {\n"
	            /**/ "				c = this.seq.operator iter();\n"
	            /**/ "				goto got_rewound_iter;\n"
	            /**/ "			}\n"
	            /**/ "		}\n"
	            /**/ "		throw NotImplemented(\"...\");\n"
	            /**/ "got_rewound_iter:\n"
	            /**/ "		while (c < this) {\n"
	            /**/ "			++c;\n"
	            /**/ "			++result;\n"
	            /**/ "		}\n"
	            /**/ "		return result;\n"
	            /**/ "	}\n"
	            /**/ "	set(index: int) {\n"
	            /**/ "		index = index.operator int();\n"
	            /**/ "		this.rewind();\n"
	            /**/ "		this.advance(index);\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETSET_END
};

DEFAULT_OPIMP WUNUSED NONNULL((1)) int DCALL
iterator_inc(DeeObject **__restrict p_self) {
	/* Simply advance the Iterator. */
	return iterator_do_advance(*p_self, 1, DeeInt_One, DeeInt_MinusOne);
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) int DCALL
iterator_dec(DeeObject **__restrict p_self) {
	/* Simply revert the Iterator. */
	return iterator_do_revert(*p_self, 1, DeeInt_One, DeeInt_MinusOne);
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) int DCALL
iterator_inplace_add(DeeObject **__restrict p_self,
                     DeeObject *countob) {
	Dee_ssize_t count;
	/* Increment the Iterator by `count.operator int()' */
	if (DeeObject_AsSSize(countob, &count))
		goto err;
	if unlikely((unlikely(count < 0)
	             ? iterator_do_revert(*p_self, (size_t)-count, NULL, countob)
	             : (count > 0) ? iterator_do_advance(*p_self, (size_t)count, countob, NULL)
	                           : 0) < 0)
		goto err;
	return 0;
err:
	return -1;
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) int DCALL
iterator_inplace_sub(DeeObject **__restrict p_self,
                     DeeObject *countob) {
	Dee_ssize_t count;
	/* Increment the Iterator by `count.operator int()' */
	if (DeeObject_AsSSize(countob, &count))
		goto err;
	if unlikely((unlikely(count < 0)
	             ? iterator_do_advance(*p_self, (size_t)-count, NULL, countob)
	             : (count > 0) ? iterator_do_revert(*p_self, (size_t)count, countob, NULL)
	                           : 0) < 0)
		goto err;
	return 0;
err:
	return -1;
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_add(DeeObject *self, DeeObject *countob) {
	DREF DeeObject *result;
	Dee_ssize_t count;
	/* Increment the Iterator by `count.operator int()' */
	if (DeeObject_AsSSize(countob, &count))
		goto err;
	result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	if unlikely((unlikely(count < 0)
	             ? iterator_do_revert(result, (size_t)-count, NULL, countob)
	             : (count > 0) ? iterator_do_advance(result, (size_t)count, countob, NULL)
	                           : 0) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_sub(DeeObject *self, DeeObject *countob) {
	DREF DeeObject *result;
	Dee_ssize_t count;
	/* Increment the Iterator by `count.operator int()' */
	if (DeeObject_AsSSize(countob, &count))
		goto err;
	result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	if unlikely((unlikely(count < 0)
	             ? iterator_do_advance(result, (size_t)-count, NULL, countob)
	             : (count > 0) ? iterator_do_revert(result, (size_t)count, countob, NULL)
	                           : 0) < 0)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE struct type_math iterator_math = {
	/* .tp_int32       = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64       = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double      = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int         = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv         = */ DEFIMPL_UNSUPPORTED(&default__inv__unsupported),
	/* .tp_pos         = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg         = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add         = */ &iterator_add,
	/* .tp_sub         = */ &iterator_sub,
	/* .tp_mul         = */ DEFIMPL_UNSUPPORTED(&default__mul__unsupported),
	/* .tp_div         = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod         = */ DEFIMPL_UNSUPPORTED(&default__mod__unsupported),
	/* .tp_shl         = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr         = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and         = */ DEFIMPL_UNSUPPORTED(&default__and__unsupported),
	/* .tp_or          = */ DEFIMPL_UNSUPPORTED(&default__or__unsupported),
	/* .tp_xor         = */ DEFIMPL_UNSUPPORTED(&default__xor__unsupported),
	/* .tp_pow         = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ &iterator_inc,
	/* .tp_dec         = */ &iterator_dec,
	/* .tp_inplace_add = */ &iterator_inplace_add,
	/* .tp_inplace_sub = */ &iterator_inplace_sub,
	/* .tp_inplace_mul = */ DEFIMPL_UNSUPPORTED(&default__inplace_mul__unsupported),
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ DEFIMPL_UNSUPPORTED(&default__inplace_and__unsupported),
	/* .tp_inplace_or  = */ DEFIMPL_UNSUPPORTED(&default__inplace_or__unsupported),
	/* .tp_inplace_xor = */ DEFIMPL_UNSUPPORTED(&default__inplace_xor__unsupported),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};



/* TODO: tee(int n=2)->?S?DSequence
 * Return @n independent sequences, each containing a copy of all elements
 * that were pending for @this, with @this Iterator never needing to be copied,
 * and elements only read from that iterator as that element is first accessed by
 * one of the returned iterators, and elements being removed from a pre-cached
 * set of pending elements once all of the iterators have read that item.
 * Meant for use in multi-threaded environments, where one thread is set up as
 * data producer, lazily producing data, and all of the other threads there to
 * lazily consume that data:
 * >> function tee(iter, n = 2) {
 * >>     import Deque from collections;
 * >>     import Signal, Error from deemon;
 * >>     import SharedLock from threading;
 * >>     if (n < 0) throw Error.IntegerOverflow();
 * >>     if (n == 0) return { };
 * >>     if (n == 1) return { iter.pending };
 * >>     local pending = Deque();
 * >>     local offsets = [0] * n;
 * >>     local lock = SharedLock();
 * >>     function gen(i) {
 * >>         for (;;) {
 * >>             local new_item;
 * >>             with (lock) {
 * >>                 local offset = offsets[i];
 * >>                 assert offset <= #pending;
 * >>                 if (offset == #pending) {
 * >>                     try {
 * >>                         new_item = iter.operator next();
 * >>                     } catch (Signal.StopIteration) {
 * >>                         return;
 * >>                     }
 * >>                     pending.pushback((n - 1, new_item));
 * >>                     ++offsets[i]; // offsets[i] = #pending;
 * >>                 } else {
 * >>                     local count;
 * >>                     count, new_item = pending[offset]...;
 * >>                     if (count == 1) {
 * >>                         assert offset == 0;
 * >>                         pending.popfront();
 * >>                         for (local j: [:i])
 * >>                             --offsets[j];
 * >>                         offsets[i] = 0;
 * >>                     } else {
 * >>                         pending[offset] = (count - 1, new_item);
 * >>                         ++offsets[i];
 * >>                     }
 * >>                 }
 * >>             }
 * >>             yield new_item;
 * >>         }
 * >>     }
 * >>     return tuple(for (local i: [:n]) gen(i));
 * >> }
 * Note that tee() should not be used when @this source iterator is copyable,
 * with reading from a copied iterators not having any unwanted side-effects. */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iterator_assign(DeeObject *self, DeeObject *other) {
	size_t index;
	if (DeeObject_AssertImplements(other, Dee_TYPE(self)))
		goto err;
	/* XXX: What about:
	 * >> this.rewind();
	 * >> while (this < other)
	 * >>     ++this; // or `operator next()', `operator ++ ()', `operator += ()' or `advance()'
	 */
	index = DeeIterator_GetIndex(other);
	if unlikely(index == (size_t)-1)
		goto err;
	return DeeIterator_SetIndex(self, index);
err:
	return -1;
}


/* `Iterator from deemon' */
PUBLIC DeeTypeObject DeeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Iterator),
	/* .tp_doc      = */ DOC("The abstract base class implementing helper functions and utility operators "
	                         /**/ "for any iterator-like object derived from it. Sub-classes should always "
	                         /**/ "implement ${operator next} which is what actually determines if a type "
	                         /**/ "qualifies as an Iterator\n"

	                         "Sub-classes are also encouraged to implement a copy-constructor, as well "
	                         /**/ "as a member or property $seq which yields the underlying sequence being "
	                         /**/ "iterated. Note that all builtin iterator types implement the $seq member, "
	                         /**/ "and unless there is a good reason not to, most also implement a copy-constructor\n"
	                         "\n"

	                         "()\n"
	                         "Default-construct an Iterator object\n"
	                         "\n"

	                         "next->\n"
	                         "Default-implemented to always indicate iterator exhaustion\n"
	                         "This function must be overwritten by sub-classes\n"
	                         "\n"

	                         "repr->\n"
	                         "Copies @this Iterator and enumerate all remaining elements, constructing "
	                         /**/ "a representation of all of them using abstract sequence syntax\n"
	                         "${"
	                         /**/ "operator repr(fp: File) {\n"
	                         /**/ "	fp << \"{ [...]\";\n"
	                         /**/ "	local c = copy this;\n"
	                         /**/ "	foreach (local x: c)\n"
	                         /**/ "		fp << \", \" << repr(x);\n"
	                         /**/ "	fp << \" }\";\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?f if @this Iterator has been exhausted, or ?t otherwise.\n"
	                         "${"
	                         /**/ "operator bool() {\n"
	                         /**/ "	local c = copy this;\n"
	                         /**/ "	return try ({\n"
	                         /**/ "		c.operator next();\n"
	                         /**/ "		true;\n"
	                         /**/ "	}) catch (StopIteration from errors) (\n"
	                         /**/ "		false\n"
	                         /**/ "	);\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "+(step:?Dint)->\n"
	                         "#tNotImplemented{@step is negative, and @this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	                         "#tIntegerOverflow{@step is too large}"
	                         "Copies @this Iterator and advance it by yielding @step items from it before returning it\n"
	                         "If the Iterator becomes exhausted before then, stop and return that exhausted iterator\n"
	                         "\n"

	                         "+=(step:?Dint)->\n"
	                         "#tNotImplemented{@step is negative, and @this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	                         "#tIntegerOverflow{@step is too large}"
	                         "Advance @this Iterator by yielding @step items\n"
	                         "If @this Iterator becomes exhausted before then, stop prematurely\n"
	                         "\n"

	                         "++->\n"
	                         "Advance @this Iterator by one. No-op if the Iterator has been exhausted\n"
	                         "Note this is very similar to ?#{op:next}, however in the case of generator-like "
	                         /**/ "iterators, doing this may be faster since no generator value has to be created\n"
	                         "\n"

	                         "call(defl?)->\n"
	                         "Calling an operator as a function will invoke ${operator next}, and return "
	                         /**/ "that value, allowing iterators to be used as function-like producers\n"
	                         "${"
	                         /**/ "operator call(defl?) {\n"
	                         /**/ "	try {\n"
	                         /**/ "		return this.operator next();\n"
	                         /**/ "	} catch (StopIteration) {\n"
	                         /**/ "		if (defl is bound)\n"
	                         /**/ "			return defl;\n"
	                         /**/ "		throw;\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "-(step:?Dint)->\n"
	                         "#tNotImplemented{@step is positive, and @this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	                         "#tIntegerOverflow{@step is too large}"
	                         "Copies @this Iterator and reverts it by @step before returning it\n"
	                         "If the Iterator reaches its starting position before then, stop prematurely\n"
	                         "\n"

	                         "--->\n"
	                         "#tNotImplemented{@this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	                         "Decrement @this operator by one. No-op if the Iterator is already at its starting position\n"
	                         "\n"

	                         "-=(step:?Dint)->\n"
	                         "#tNotImplemented{@step is positive, and @this Iterator isn't bi-directional (s.a. ?#isbidirectional)}"
	                         "#tIntegerOverflow{@step is too large}"
	                         "Revert @this Iterator by @step items\n"
	                         "If @this Iterator reaches its starting position before then, stop prematurely\n"
	                         "\n"

	                         "<->\n"
	                         "<=->\n"
	                         "==->\n"
	                         "!=->\n"
	                         ">->\n"
	                         ">=->\n"
	                         "#tTypeError{The types of @other and @this don't match}"
	                         "Compare @this Iterator with @other, returning ?t/?f "
	                         /**/ "indicate of the remaining number of elements left to be yielded.\n"
	                         "Various iterator sub-classes also override these operators, and their "
	                         /**/ "behavior in respect to the types (and more importantly: the underlying "
	                         /**/ "sequences of) the 2 iterators differs greatly.\n"
	                         "In general though, you should only ever compare 2 iterators "
	                         /**/ "of the same type, and used to iterate the exact same sequence"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&DeeNone_OperatorCtor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ &iterator_assign,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ &iterator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ &iterator_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &iterator_math,
	/* .tp_cmp           = */ &iterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ &iterator_iternext,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ iterator_methods,
	/* .tp_getsets       = */ iterator_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ &iterator_next,
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PUBLIC DeeObject DeeIterator_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeIterator_Type)
};



typedef struct {
	OBJECT_HEAD
	DREF DeeObject *if_iter; /* [1..1][const] The iterator who's future is viewed. */
} IteratorFuture;

INTDEF DeeTypeObject IteratorFuture_Type;
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
IteratorFuture_For(DeeObject *__restrict self) {
	DREF IteratorFuture *result;
	result = DeeObject_MALLOC(IteratorFuture);
	if unlikely(!result)
		goto done;
	result->if_iter = self;
	Dee_Incref(self);
	DeeObject_Init(result, &IteratorFuture_Type);
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
if_ctor(IteratorFuture *__restrict self) {
	self->if_iter = DeeObject_Iter(Dee_EmptySeq);
	return likely(self->if_iter) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
if_copy(IteratorFuture *__restrict self,
        IteratorFuture *__restrict other) {
	self->if_iter = DeeObject_Copy(other->if_iter);
	if unlikely(!self->if_iter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
if_deep(IteratorFuture *__restrict self,
        IteratorFuture *__restrict other) {
	self->if_iter = DeeObject_DeepCopy(other->if_iter);
	if unlikely(!self->if_iter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
if_init(IteratorFuture *__restrict self,
        size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_IteratorFuture", &self->if_iter);
	Dee_Incref(self->if_iter);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
if_bool(IteratorFuture *__restrict self) {
	return DeeObject_Bool(self->if_iter);
}

PRIVATE NONNULL((1)) void DCALL
if_fini(IteratorFuture *__restrict self) {
	Dee_Decref(self->if_iter);
}

PRIVATE NONNULL((1, 2)) void DCALL
if_visit(IteratorFuture *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->if_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
if_iter(IteratorFuture *__restrict self) {
	return DeeObject_Copy(self->if_iter);
}

PRIVATE struct type_seq if_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&if_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst if_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(IteratorFuture, if_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst if_class_members[] = {
	/* Should always be right, because this standard proxy is usually constructed
	 * by the `future' member of `iterator', meaning that the contained iterator
	 * should always be derived from that type.
	 * -> The only time this isn't correct is when the user manually constructs
	 *    instances of this type... */
	TYPE_MEMBER_CONST(STR_Iterator, &DeeIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject IteratorFuture_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IteratorFuture",
	/* .tp_doc      = */ DOC("(iter?:?DIterator)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&if_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&if_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&if_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&if_init,
				TYPE_FIXED_ALLOCATOR(IteratorFuture)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&if_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&if_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&if_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &if_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ if_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ if_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



typedef struct {
	OBJECT_HEAD
	DREF DeeObject *ip_iter; /* [1..1][const] The iterator who's remainder is viewed. */
} IteratorPending;

INTDEF DeeTypeObject IteratorPending_Type;
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
IteratorPending_For(DeeObject *__restrict self) {
	DREF IteratorPending *result;
	result = DeeObject_MALLOC(IteratorPending);
	if unlikely(!result)
		goto done;
	result->ip_iter = self;
	Dee_Incref(self);
	DeeObject_Init(result, &IteratorPending_Type);
done:
	return (DREF DeeObject *)result;
}

STATIC_ASSERT(offsetof(IteratorFuture, if_iter) ==
              offsetof(IteratorPending, ip_iter));
#define ip_ctor if_ctor
#define ip_deep if_deep

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ip_copy(IteratorPending *__restrict self,
        IteratorPending *__restrict other) {
	self->ip_iter = other->ip_iter;
	Dee_Incref(self->ip_iter);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ip_init(IteratorPending *__restrict self,
        size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_IteratorPending", &self->ip_iter);
	Dee_Incref(self->ip_iter);
	return 0;
err:
	return -1;
}

#define ip_bool  if_bool
#define ip_fini  if_fini
#define ip_visit if_visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ip_iter(IteratorPending *__restrict self) {
	return_reference_(self->ip_iter);
}

PRIVATE struct type_seq ip_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ip_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

#define ip_members        if_members
#define ip_class_members  if_class_members

INTERN DeeTypeObject IteratorPending_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IteratorPending",
	/* .tp_doc      = */ DOC("(iter?:?DIterator)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ip_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&ip_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ip_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&ip_init,
				TYPE_FIXED_ALLOCATOR(IteratorPending)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ip_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&ip_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ip_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &ip_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ip_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ip_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeIterator_Foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *elem;
	DeeNO_iter_next_t tp_iter_next = DeeType_RequireNativeOperator(Dee_TYPE(self), iter_next);
	while (ITER_ISOK(elem = (*tp_iter_next)(self))) {
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		/* Must check for interrupts because iterator may produce infinite results. */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeIterator_ForeachPair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	int status;
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *pair[2];
	DeeNO_nextpair_t tp_nextpair = DeeType_RequireNativeOperator(Dee_TYPE(self), nextpair);
	while ((status = (*tp_nextpair)(self, pair)) == 0) {
		temp = (*cb)(arg, pair[0], pair[1]);
		Dee_Decref(pair[1]);
		Dee_Decref(pair[0]);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		/* Must check for interrupts because iterator may produce infinite results. */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(status < 0)
		goto err;
	return result;
err_temp:
	return temp;
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ITERATOR_C */
