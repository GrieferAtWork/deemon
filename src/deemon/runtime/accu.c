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
#ifndef GUARD_DEEMON_RUNTIME_ACCU_C
#define GUARD_DEEMON_RUNTIME_ACCU_C 1

#include <deemon/api.h>

#include <deemon/accu.h>           /* Dee_ACCU_FLOAT_DEFINED, Dee_ACCU_INT64_DEFINED, Dee_accu */
#include <deemon/bytes.h>          /* DeeBytes*, Dee_bytes_printer_* */
#include <deemon/float.h>          /* DeeFloatObject, DeeFloat_New, DeeFloat_VALUE */
#include <deemon/int.h>            /* DeeInt_* */
#include <deemon/list.h>           /* DeeListObject, DeeList_* */
#include <deemon/none-operator.h>  /* _DeeNone_NewRef2 */
#include <deemon/none.h>           /* return_none */
#include <deemon/object.h>
#include <deemon/operator-hints.h> /* DeeNO_add_t, DeeType_RequireNativeOperator */
#include <deemon/string.h>         /* DeeStringObject, Dee_unicode_printer* */
#include <deemon/tuple.h>          /* DeeTuple*, Dee_tuple_builder* */

#include <hybrid/overflow.h> /* OVERFLOW_SADD, OVERFLOW_UADD */

#include "../objects/int_logic.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* int32_t, int64_t */

#ifndef INT32_MIN
#include <hybrid/limitcore.h> /* __INT32_MAX__, __INT32_MIN__ */
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */

#ifndef INT32_MAX
#include <hybrid/limitcore.h>
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */

DECL_BEGIN

/* Finalize the accumulator and dispose any previously calculated results. */
PUBLIC NONNULL((1)) void DCALL
Dee_accu_fini(struct Dee_accu *__restrict self) {
	switch (self->acu_mode) {
	case Dee_ACCU_FIRST:
	case Dee_ACCU_NONE:
	case Dee_ACCU_INT:
#ifdef Dee_ACCU_INT64_DEFINED
	case Dee_ACCU_INT64:
#endif /* Dee_ACCU_INT64_DEFINED */
#ifdef Dee_ACCU_FLOAT_DEFINED
	case Dee_ACCU_FLOAT:
#endif /* Dee_ACCU_FLOAT_DEFINED */
		break;
	case Dee_ACCU_SECOND:
	case Dee_ACCU_OBJECT:
	case Dee_ACCU_LIST:
		Dee_Decref(self->acu_value.v_object);
		break;
	case Dee_ACCU_STRING:
		Dee_unicode_printer_fini(&self->acu_value.v_string);
		break;
	case Dee_ACCU_BYTES:
		Dee_bytes_printer_fini(&self->acu_value.v_bytes);
		break;
	case Dee_ACCU_TUPLE:
		Dee_tuple_builder_fini(&self->acu_value.v_tuple);
		break;
	default: __builtin_unreachable();
	}
}

PUBLIC NONNULL((1, 2)) void DCALL
Dee_accu_visit(struct Dee_accu *__restrict self,
               Dee_visit_t proc, void *arg) {
	switch (self->acu_mode) {
	case Dee_ACCU_FIRST:
	case Dee_ACCU_NONE:
	case Dee_ACCU_INT:
#ifdef Dee_ACCU_INT64_DEFINED
	case Dee_ACCU_INT64:
#endif /* Dee_ACCU_INT64_DEFINED */
#ifdef Dee_ACCU_FLOAT_DEFINED
	case Dee_ACCU_FLOAT:
#endif /* Dee_ACCU_FLOAT_DEFINED */
	case Dee_ACCU_STRING:
	case Dee_ACCU_BYTES:
		break;
	case Dee_ACCU_SECOND:
	case Dee_ACCU_OBJECT:
	case Dee_ACCU_LIST:
		Dee_Visit(self->acu_value.v_object);
		break;
	case Dee_ACCU_TUPLE:
		Dee_tuple_builder_visit(&self->acu_value.v_tuple);
		break;
	default: __builtin_unreachable();
	}
}


/* Pack the accumulator and return its final result as an object.
 * This function may only be called once, as it does an implicit
 * `Dee_accu_fini()'. Returns `NULL' if an error was thrown.
 *
 * Hint: if you want `self' to remain valid, you can just re-init it
 *       after the call using `Dee_accu_init_with_first_inherited()'
 * @param: empty: What to return if the accumulator is empty. You may
 *                not pass "NULL" here, but you may pass "ITER_DONE" */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_accu_pack(struct Dee_accu *__restrict self, DeeObject *empty) {
	switch (self->acu_mode) {
	case Dee_ACCU_FIRST:
		if (empty != ITER_DONE)
			Dee_Incref(empty);
		return empty;
	case Dee_ACCU_NONE:
		return_none;
	case Dee_ACCU_SECOND:
	case Dee_ACCU_OBJECT:
	case Dee_ACCU_LIST:
		return self->acu_value.v_object;
	case Dee_ACCU_STRING:
		return Dee_unicode_printer_pack(&self->acu_value.v_string);
	case Dee_ACCU_BYTES:
		return Dee_bytes_printer_pack(&self->acu_value.v_bytes);
	case Dee_ACCU_INT:
		return DeeInt_NewSSize(self->acu_value.v_int);
#ifdef Dee_ACCU_INT64_DEFINED
	case Dee_ACCU_INT64:
		return DeeInt_NewInt64(self->acu_value.v_int64);
#endif /* Dee_ACCU_INT64_DEFINED */
#ifdef Dee_ACCU_FLOAT_DEFINED
	case Dee_ACCU_FLOAT:
		return DeeFloat_New(self->acu_value.v_float);
#endif /* Dee_ACCU_FLOAT_DEFINED */
	case Dee_ACCU_TUPLE:
		return Dee_tuple_builder_pack(&self->acu_value.v_tuple);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

INTDEF WUNUSED DREF DeeListObject *DCALL /* From "../objects/list.c" */
DeeList_Copy(DeeListObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeStringObject *DCALL
string_cat(DeeStringObject *__restrict self, DeeObject *__restrict other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeBytesObject *DCALL
bytes_add(DeeBytesObject *self, DeeObject *other);
#ifdef Dee_ACCU_FLOAT_DEFINED
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeFloatObject *DCALL
float_add(DeeFloatObject *__restrict self, DeeObject *__restrict other);
#endif /* Dee_ACCU_FLOAT_DEFINED */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
tuple_concat(DeeTupleObject *self, DeeObject *other);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeListObject *DCALL
list_add(DeeListObject *me, DeeObject *other);


PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
accu_second(struct Dee_accu *__restrict self, DeeObject *second) {
	DeeObject *first = self->acu_value.v_object;
	DeeTypeObject *tp_first = Dee_TYPE(first);
	DeeNO_add_t tp_add = DeeType_RequireNativeOperator(tp_first, add);

	/* Select special (more efficient) implementations for specific types. */
	if (tp_add == (DeeNO_add_t)&string_cat) {
		Dee_ssize_t result;
		Dee_unicode_printer_init(&self->acu_value.v_string);
		self->acu_mode = Dee_ACCU_STRING;
		result = Dee_unicode_printer_printstring(&self->acu_value.v_string, first);
		Dee_Decref(first);
		if unlikely(result < 0)
			goto err;
		return Dee_unicode_printer_printobject(&self->acu_value.v_string, second);
	} else if (tp_add == (DeeNO_add_t)&bytes_add) {
		Dee_ssize_t result;
		Dee_bytes_printer_init(&self->acu_value.v_bytes);
		self->acu_mode = Dee_ACCU_BYTES;
		DeeBytes_IncUse(first);
		result = Dee_bytes_printer_append(&self->acu_value.v_bytes,
		                                  DeeBytes_DATA(first),
		                                  DeeBytes_SIZE(first));
		DeeBytes_DecUse(first);
		Dee_Decref(first);
		if unlikely(result < 0)
			goto err;
		return Dee_bytes_printer_printobject(&self->acu_value.v_bytes, second);
	} else if (tp_add == (DeeNO_add_t)&int_add) {
		int64_t intval1, intval2;
		if unlikely(!DeeInt_Check(second))
			goto fallback;
		if (!DeeInt_TryAsInt64(first, &intval1))
			goto fallback;
		if (!DeeInt_TryAsInt64(second, &intval2))
			goto fallback;
		if (OVERFLOW_SADD(intval1, intval2, &intval1))
			goto fallback;
#ifdef Dee_ACCU_INT64_DEFINED
		if (intval1 >= INT32_MIN && intval1 <= INT32_MAX) {
			self->acu_value.v_int = (int32_t)intval1;
			self->acu_mode = Dee_ACCU_INT;
		} else {
			self->acu_value.v_int64 = intval1;
			self->acu_mode = Dee_ACCU_INT64;
		}
#else /* Dee_ACCU_INT64_DEFINED */
		self->acu_value.v_int = (Dee_ssize_t)intval1;
		self->acu_mode = Dee_ACCU_INT;
#endif /* !Dee_ACCU_INT64_DEFINED */
		Dee_Decref(first);
#ifdef Dee_ACCU_FLOAT_DEFINED
	} else if (tp_add == (DeeNO_add_t)&float_add) {
		double second_value;
		if (DeeObject_AsDouble(second, &second_value))
			goto err;
		self->acu_value.v_float = DeeFloat_VALUE(first) + second_value;
		self->acu_mode = Dee_ACCU_FLOAT;
		Dee_Decref(first);
#endif /* Dee_ACCU_FLOAT_DEFINED */
	} else if (tp_add == (DeeNO_add_t)&tuple_concat) {
		if (!DeeObject_IsShared(first)) {
			/* Can re-use the initial "first" tuple as an in-place buffer. */
			self->acu_value.v_tuple.tb_size  = DeeTuple_SIZE(first);
			self->acu_value.v_tuple.tb_tuple = (DREF DeeTupleObject *)first;
			self->acu_mode = Dee_ACCU_TUPLE;
		} else {
			Dee_ssize_t error;
			size_t total_size = DeeTuple_SIZE(first);
			size_t second_size_fast = DeeObject_SizeFast(second);
			if likely(second_size_fast != (size_t)-1) {
				if (OVERFLOW_UADD(total_size, second_size_fast, &total_size))
					total_size = (size_t)-1;
			}
			Dee_tuple_builder_init_with_hint(&self->acu_value.v_tuple, total_size);
			self->acu_mode = Dee_ACCU_TUPLE;
			error = Dee_tuple_builder_extend(&self->acu_value.v_tuple,
			                                 DeeTuple_SIZE(first),
			                                 DeeTuple_ELEM(first));
			Dee_Decref_unlikely(first);
			if unlikely(error < 0)
				goto err;
		}
		return Dee_tuple_builder_appenditems(&self->acu_value.v_tuple, second);
	} else if (tp_add == (DeeNO_add_t)&list_add) {
		DREF DeeObject *combine;
		combine = DeeList_ConcatInherited(first, second);
		if unlikely(!combine) {
			self->acu_mode = Dee_ACCU_NONE; /* Because reference to "first" was inherited. */
			goto err;
		}
		ASSERT_OBJECT_TYPE_EXACT(combine, &DeeList_Type);
		ASSERT(!DeeObject_IsShared(combine));
		self->acu_value.v_object = combine;
		self->acu_mode = Dee_ACCU_LIST;
	} else if (tp_add == (DeeNO_add_t)&_DeeNone_NewRef2) {
		Dee_Decref_unlikely(first);
		self->acu_mode = Dee_ACCU_NONE;
	} else {
		DREF DeeObject *combine;
fallback:
		combine = (*tp_add)(first, second);
		if unlikely(!combine)
			goto err;
		Dee_Decref(first);
		self->acu_value.v_object = combine;
		self->acu_mode = Dee_ACCU_OBJECT;
	}
	return 0;
err:
	return -1;
}

/* Add `item' into the accumulator.
 * HINT: This function is `Dee_foreach_t'-compatible. */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_accu_add(/*struct Dee_accu*/ void *self, DeeObject *item) {
	struct Dee_accu *me = (struct Dee_accu *)self;
	switch (me->acu_mode) {

	case Dee_ACCU_FIRST: {
		me->acu_mode = Dee_ACCU_SECOND;
		me->acu_value.v_object = item;
		Dee_Incref(item);
	}	break;

	case Dee_ACCU_SECOND:
		return accu_second(me, item);

	case Dee_ACCU_NONE:
		return 0;

	case Dee_ACCU_LIST:
		return DeeList_AppendSequence((DeeObject *)me->acu_value.v_list, item);

	case Dee_ACCU_STRING:
		return Dee_unicode_printer_printobject(&me->acu_value.v_string, item);

	case Dee_ACCU_BYTES:
		return Dee_bytes_printer_printobject(&me->acu_value.v_bytes, item);

#ifdef Dee_ACCU_INT64_DEFINED
	case Dee_ACCU_INT: {
		int32_t addend, result;
		if unlikely(!DeeInt_Check(item)) {
			DREF DeeObject *prev;
convert_int_to_object:
			prev = DeeInt_NewSSize(me->acu_value.v_int);
			if unlikely(!prev)
				goto err;
			me->acu_value.v_object = prev;
			me->acu_mode = Dee_ACCU_OBJECT;
			goto handle_object;
		}
		if unlikely(!DeeInt_TryAsInt32(item, &addend)) {
			int64_t addend64;
			if unlikely(!DeeInt_TryAsInt64(item, &addend64))
				goto convert_int_to_object;
			if (OVERFLOW_SADD(addend64, me->acu_value.v_int, &addend64))
				goto convert_int_to_object;
			me->acu_value.v_int64 = addend64;
			me->acu_mode = Dee_ACCU_INT64;
			break;
		}
		if unlikely(OVERFLOW_SADD(me->acu_value.v_int, addend, &result)) {
			me->acu_value.v_int64 = (int64_t)me->acu_value.v_int + addend;
			me->acu_mode = Dee_ACCU_INT64;
			break;
		}
		me->acu_value.v_int = result;
	}	break;
#endif /* Dee_ACCU_INT64_DEFINED */

#ifdef Dee_ACCU_INT64_DEFINED
	case Dee_ACCU_INT64:
#else /* Dee_ACCU_INT64_DEFINED */
	case Dee_ACCU_INT:
#define v_int64 v_int
#endif /* !Dee_ACCU_INT64_DEFINED */
	{
		int64_t addend;
		if unlikely(!DeeInt_Check(item)) {
			DREF DeeObject *prev;
convert_int64_to_object:
			prev = DeeInt_NewInt64(me->acu_value.v_int64);
			if unlikely(!prev)
				goto err;
			me->acu_value.v_object = prev;
			me->acu_mode = Dee_ACCU_OBJECT;
			goto handle_object;
		}
		if unlikely(!DeeInt_TryAsInt64(item, &addend))
			goto convert_int64_to_object;
		if unlikely(OVERFLOW_SADD(me->acu_value.v_int64, addend, &addend))
			goto convert_int64_to_object;
		me->acu_value.v_int64 = addend;
	}	break;
#undef v_int64

#ifdef Dee_ACCU_FLOAT_DEFINED
	case Dee_ACCU_FLOAT: {
		double item_float;
		if (DeeObject_AsDouble(item, &item_float))
			goto err;
		me->acu_value.v_float += item_float;
	}	break;
#endif /* !Dee_ACCU_FLOAT_DEFINED */

	case Dee_ACCU_TUPLE:
		return Dee_tuple_builder_appenditems(&me->acu_value.v_tuple, item);

	case Dee_ACCU_OBJECT: {
		DeeObject *first;
		DREF DeeObject *combine;
handle_object:
		first = me->acu_value.v_object;
		combine = DeeObject_Add(first, item);
		if unlikely(!combine)
			goto err;
		Dee_Decref(first);
		me->acu_value.v_object = combine;
	}	break;

	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
}

/* Add all elements of `items' into the accumulator. */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_accu_addall(/*struct Dee_accu*/ void *self, DeeObject *items) {
	struct Dee_accu *me = (struct Dee_accu *)self;
	if (me->acu_mode == Dee_ACCU_NONE)
		return 0;
	return DeeObject_Foreach(items, &Dee_accu_add, me);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ACCU_C */
