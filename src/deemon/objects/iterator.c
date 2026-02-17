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
#ifndef GUARD_DEEMON_OBJECTS_ITERATOR_C
#define GUARD_DEEMON_OBJECTS_ITERATOR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>                /* DeeArg_Unpack0Or1 */
#include <deemon/bool.h>               /* return_bool */
#include <deemon/computed-operators.h> /* DEFAULT_OPDEF, DEFAULT_OPIMP, DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error.h>              /* DeeError_StopIteration_instance, DeeError_Throw */
#include <deemon/format.h>             /* DeeFormat_PRINT, DeeFormat_PrintObjectRepr */
#include <deemon/method-hints.h>       /* DeeMA_*, DeeObject_InvokeMethodHint */
#include <deemon/none-operator.h>      /* DeeNone_Operator*, _DeeNone_retsm1_1 */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Decref, Dee_TYPE, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_return_compare, Dee_ssize_t, Dee_visit_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeNO_iter_next_t, DeeNO_nextpair_t, DeeType_RequireNativeOperator */
#include <deemon/seq.h>                /* DeeSeq_Type, Dee_EmptySeq */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/thread.h>             /* DeeThread_CheckInterrupt */
#include <deemon/type.h>               /* DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_*, type_* */

#include "../runtime/method-hint-defaults.h"
#include "../runtime/method-hints.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

DEFAULT_OPDEF WUNUSED NONNULL((1)) int DCALL iterator_inc(DeeObject **__restrict p_self);
DEFAULT_OPDEF WUNUSED NONNULL((1)) int DCALL iterator_dec(DeeObject **__restrict p_self);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_inplace_add(DeeObject **__restrict p_self, DeeObject *countob);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_inplace_sub(DeeObject **__restrict p_self, DeeObject *countob);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL iterator_add(DeeObject *self, DeeObject *countob);
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL iterator_sub(DeeObject *self, DeeObject *countob);

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
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),          /* TODO: method-hint */
	/* .tp_compare    = */ &iterator_compare,                                     /* TODO: method-hint */
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq), /* TODO: method-hint */
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_next(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *def = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "next", &def);
	result = DeeObject_IterNext(self);
	if (result == ITER_DONE) {
		if (def)
			return_reference_(def);
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_nextkey(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *def = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "nextkey", &def);
	result = DeeObject_IterNextKey(self);
	if (result == ITER_DONE) {
		if (def)
			return_reference_(def);
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_nextvalue(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *def = NULL;
	DeeArg_Unpack0Or1(err, argc, argv, "nextvalue", &def);
	result = DeeObject_IterNextValue(self);
	if (result == ITER_DONE) {
		if (def)
			return_reference_(def);
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

PRIVATE struct type_method tpconst iterator_methods[] = {
	TYPE_METHOD_F(DeeMA_Iterator_advance_name, &DeeMA_Iterator_advance,
	              DeeMA_Iterator_advance_flags,
	              DeeMA_Iterator_advance_doc "\n"
	              "TODO"),
	TYPE_METHOD_F(DeeMA_Iterator_revert_name, &DeeMA_Iterator_revert,
	              DeeMA_Iterator_revert_flags,
	              DeeMA_Iterator_revert_doc "\n"
	              "TODO"),
	TYPE_METHOD_F(DeeMA_Iterator_peek_name, &DeeMA_Iterator_peek,
	              DeeMA_Iterator_peek_flags,
	              DeeMA_Iterator_peek_doc "\n"
	              "#tStopIteration{@this Iterator has been exhausted, and no @def was given}"
	              "Peek the next upcoming object, but don't consume it\n"
	              "${"
	              /**/ "function peek(def?) {\n"
	              /**/ "	local c = copy this;\n"
	              /**/ "	foreach (local next: c)\n"
	              /**/ "		return next;\n"
	              /**/ "	if (def is bound)\n"
	              /**/ "		return def;\n"
	              /**/ "	throw StopIteration();\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD_F(DeeMA_Iterator_prev_name, &DeeMA_Iterator_prev,
	              DeeMA_Iterator_prev_flags,
	              DeeMA_Iterator_prev_doc "\n"
	              "#tStopIteration{@this Iterator has no predecessor}"
	              "Revert the iterator by 1 element, then peek its new head-element. "
	              /**/ "If you're a C developer, you can think of this as #C{*--this}, "
	              /**/ "where-as the regular ${operator next} behaves like #C{*this++}"),
	TYPE_METHOD_F(DeeMA_Iterator_rewind_name, &DeeMA_Iterator_rewind,
	              DeeMA_Iterator_rewind_flags,
	              DeeMA_Iterator_rewind_doc "\n"
	              "Same as ${del this.index} (s.a. ?#index)"),
	TYPE_METHOD("next", &iterator_next,
	            "(def?)->\n"
	            "#tStopIteration{@this Iterator has been exhausted, and no @def was given}"
	            "Same as ${this.operator next()}\n"
	            "When given, @def is returned when the Iterator has been "
	            /**/ "exhausted, rather than throwing a :StopIteration error"),
	TYPE_METHOD("nextkey", &iterator_nextkey,
	            "(def?)->\n"
	            "#tStopIteration{@this Iterator has been exhausted, and no @def was given}"
	            "Consume the next item, unpack it into a 2-element "
	            /**/ "sequence, and return that sequence's first item\n"
	            "${"
	            /**/ "function nextkey(def?) {\n"
	            /**/ "	foreach (local next: c) {\n"
	            /**/ "		local a, none = next...;\n"
	            /**/ "		return a;\n"
	            /**/ "	}\n"
	            /**/ "	if (def is bound)\n"
	            /**/ "		return def;\n"
	            /**/ "	throw StopIteration();\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("nextvalue", &iterator_nextvalue,
	            "(def?)->\n"
	            "#tStopIteration{@this Iterator has been exhausted, and no @def was given}"
	            "Consume the next item, unpack it into a 2-element "
	            /**/ "sequence, and return that sequence's second item\n"
	            "${"
	            /**/ "function nextvalue(def?) {\n"
	            /**/ "	foreach (local next: c) {\n"
	            /**/ "		local none, b = next...;\n"
	            /**/ "		return b;\n"
	            /**/ "	}\n"
	            /**/ "	if (def is bound)\n"
	            /**/ "		return def;\n"
	            /**/ "	throw StopIteration();\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD_END
};

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL IteratorFuture_For(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL IteratorPending_For(DeeObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_get_hasnext(DeeObject *__restrict self) {
	int temp = DeeObject_InvokeMethodHint(iter_operator_bool, self);
	if unlikely(temp < 0)
		goto err;
	return_bool(temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iterator_get_hasprev(DeeObject *__restrict self) {
	size_t index = DeeObject_InvokeMethodHint(iter_getindex, self);
	if unlikely(index == (size_t)-1)
		goto err;
	return_bool(index == 0);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst iterator_getsets[] = {
	TYPE_GETTER(STR_seq, &default__iter_getseq,
	            "->?DSequence\n"
	            "Returns the underlying sequence that is being iterated\n"
	            "Since use of this member isn't all too common, sub-classes are allowed "
	            /**/ "to (and sometimes do) not return the exact original sequence, but rather "
	            /**/ "a sequence that is syntactically equivalent (i.e. contains the same items)\n"
	            "Because of this you should not expect their ids to equal, or even their "
	            /**/ "types for that matter. Only expect the contained items, as well as their "
	            /**/ "order to be identical"),
	TYPE_GETSET_F(DeeMA_Iterator_index_name,
	              &DeeMA_Iterator_index_get,
	              &DeeMA_Iterator_index_del,
	              &DeeMA_Iterator_index_set,
	              DeeMA_Iterator_index_flags,
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

	TYPE_GETTER_AB(STR_hasprev, &iterator_get_hasprev,
	               "->?Dbool\n"
	               "Returns ?t if @this Iterator has a predecessor (alias for ${Iterator.index(this) == 0})"),
	TYPE_GETTER_AB(STR_hasnext, &iterator_get_hasnext,
	               "->?Dbool\n"
	               "Returns ?t if @this Iterator has a successor (alias for ?#{op:bool})"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iterator_seek(DeeObject *self, DeeObject *delta_obj, bool invert_direction) {
	Dee_ssize_t delta;
	size_t moved;
	if (DeeObject_AsSSize(delta_obj, &delta))
		goto err;
	if (delta > 0) {
		if (invert_direction) {
			moved = DeeObject_InvokeMethodHint(iter_revert, self, (size_t)(delta));
		} else {
			moved = DeeObject_InvokeMethodHint(iter_advance, self, (size_t)(delta));
		}
	} else if (delta < 0) {
		if (invert_direction) {
			moved = DeeObject_InvokeMethodHint(iter_advance, self, (size_t)(-delta));
		} else {
			moved = DeeObject_InvokeMethodHint(iter_revert, self, (size_t)(-delta));
		}
	} else {
		moved = 0;
	}
	if unlikely(moved == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) int DCALL
iterator_inplace_add(DeeObject **__restrict p_self, DeeObject *delta_obj) {
	return iterator_seek(*p_self, delta_obj, false);
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) int DCALL
iterator_inplace_sub(DeeObject **__restrict p_self, DeeObject *delta_obj) {
	return iterator_seek(*p_self, delta_obj, true);
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_add(DeeObject *self, DeeObject *delta_obj) {
	DREF DeeObject *result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	if (iterator_seek(result, delta_obj, false))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_sub(DeeObject *self, DeeObject *delta_obj) {
	DREF DeeObject *result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	if (iterator_seek(result, delta_obj, true))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) int DCALL
iterator_inc(DeeObject **__restrict p_self) {
	size_t moved = DeeObject_InvokeMethodHint(iter_advance, *p_self, 1);
	if unlikely(moved == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) int DCALL
iterator_dec(DeeObject **__restrict p_self) {
	size_t moved = DeeObject_InvokeMethodHint(iter_revert, *p_self, 1);
	if unlikely(moved == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
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
	size_t index = DeeObject_InvokeMethodHint(iter_getindex, other);
	if unlikely(index == (size_t)-1)
		goto err;
	return DeeObject_InvokeMethodHint(iter_setindex, self, index);
err:
	return -1;
}


#ifdef CONFIG_NO_DOC
#define iter_doc NULL
#else /* CONFIG_NO_DOC */
PRIVATE char const iter_doc[] =
"The abstract base class implementing helper functions and utility operators "
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

"call(def?)->\n"
"Calling an operator as a function will invoke ${operator next}, and return "
/**/ "that value, allowing iterators to be used as function-like producers\n"
"${"
/**/ "operator call(def?) {\n"
/**/ "	try {\n"
/**/ "		return this.operator next();\n"
/**/ "	} catch (StopIteration) {\n"
/**/ "		if (def is bound)\n"
/**/ "			return def;\n"
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
/**/ "of the same type, and used to iterate the exact same sequence";
#endif

/* `Iterator from deemon' */
PUBLIC DeeTypeObject DeeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Iterator),
	/* .tp_doc      = */ iter_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorSerialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ &iterator_assign, /* TODO: method-hint */
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ &default__iter_operator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ &iterator_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &iterator_math,
	/* .tp_cmp           = */ &iterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ &iterator_iternext,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
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
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ NULL,
	/* .tp_operators_size= */ 0,
	/* .tp_mhcache       = */ &mh_cache_empty,
};


PUBLIC DeeObject DeeIterator_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeIterator_Type)
};









typedef struct {
	PROXY_OBJECT_HEAD(if_iter) /* [1..1][const] The iterator who's future is viewed. */
} IteratorFuture;

#define IteratorFuture_New(ob)                   ((DREF IteratorFuture *)ProxyObject_New(&IteratorFuture_Type, Dee_AsObject(ob)))
#define IteratorFuture_NewInherited(ob)          ((DREF IteratorFuture *)ProxyObject_NewInherited(&IteratorFuture_Type, Dee_AsObject(ob)))
#define IteratorFuture_NewInheritedOnSuccess(ob) ((DREF IteratorFuture *)ProxyObject_NewInheritedOnSuccess(&IteratorFuture_Type, Dee_AsObject(ob)))

INTDEF DeeTypeObject IteratorFuture_Type;
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
IteratorFuture_For(DeeObject *__restrict self) {
	return Dee_AsObject(IteratorFuture_New(self));
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
if_ctor(IteratorFuture *__restrict self) {
	self->if_iter = DeeObject_Iter(Dee_EmptySeq);
	return likely(self->if_iter) ? 0 : -1;
}

STATIC_ASSERT(offsetof(IteratorFuture, if_iter) == offsetof(ProxyObject, po_obj));
#define if_copy      generic_proxy__copy_recursive
#define if_init      generic_proxy__init
#define if_bool      generic_proxy__bool
#define if_fini      generic_proxy__fini
#define if_visit     generic_proxy__visit
#define if_serialize generic_proxy__serialize

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
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT_AB, offsetof(IteratorFuture, if_iter), "->?DIterator"),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ IteratorFuture,
			/* tp_ctor:        */ &if_ctor,
			/* tp_copy_ctor:   */ &if_copy,
			/* tp_any_ctor:    */ &if_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &if_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&if_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &if_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
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
	PROXY_OBJECT_HEAD(ip_iter) /* [1..1][const] The iterator who's remainder is viewed. */
} IteratorPending;

#define IteratorPending_New(ob)                   ((DREF IteratorPending *)ProxyObject_New(&IteratorPending_Type, Dee_AsObject(ob)))
#define IteratorPending_NewInherited(ob)          ((DREF IteratorPending *)ProxyObject_NewInherited(&IteratorPending_Type, Dee_AsObject(ob)))
#define IteratorPending_NewInheritedOnSuccess(ob) ((DREF IteratorPending *)ProxyObject_NewInheritedOnSuccess(&IteratorPending_Type, Dee_AsObject(ob)))

INTDEF DeeTypeObject IteratorPending_Type;
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
IteratorPending_For(DeeObject *__restrict self) {
	return Dee_AsObject(IteratorPending_New(self));
}

STATIC_ASSERT(offsetof(IteratorPending, ip_iter) == offsetof(IteratorFuture, if_iter));
#define ip_ctor if_ctor
#define ip_bool if_bool

STATIC_ASSERT(offsetof(IteratorPending, ip_iter) == offsetof(ProxyObject, po_obj));
#define ip_copy      generic_proxy__copy_alias
#define ip_init      generic_proxy__init
#define ip_fini      generic_proxy__fini
#define ip_visit     generic_proxy__visit
#define ip_serialize generic_proxy__serialize
#define ip_iter      generic_proxy__getobj

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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ IteratorPending,
			/* tp_ctor:        */ &ip_ctor,
			/* tp_copy_ctor:   */ &ip_copy,
			/* tp_any_ctor:    */ &ip_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ip_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ip_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &ip_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
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
