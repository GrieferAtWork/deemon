/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SEQ_H
#define GUARD_DEEMON_SEQ_H 1

#include "api.h"

#include "alloc.h" /* Dee_MallocUsableSize */
#include "object.h"
/**/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define return_empty_seq                Dee_return_empty_seq
#define return_empty_iterator           Dee_return_empty_iterator
#define Dee_type_nii                    type_nii
#define Dee_type_nsi                    type_nsi
#define TYPE_ITERX_CLASS_UNIDIRECTIONAL Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL
#define TYPE_ITERX_CLASS_BIDIRECTIONAL  Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL
#define TYPE_ITERX_FNORMAL              Dee_TYPE_ITERX_FNORMAL
#define TYPE_SEQX_CLASS_SEQ             Dee_TYPE_SEQX_CLASS_SEQ
#define TYPE_SEQX_CLASS_MAP             Dee_TYPE_SEQX_CLASS_MAP
#define TYPE_SEQX_CLASS_SET             Dee_TYPE_SEQX_CLASS_SET
#define TYPE_SEQX_FNORMAL               Dee_TYPE_SEQX_FNORMAL
#define TYPE_SEQX_FMUTABLE              Dee_TYPE_SEQX_FMUTABLE
#define TYPE_SEQX_FRESIZABLE            Dee_TYPE_SEQX_FRESIZABLE
#endif /* DEE_SOURCE */



/* TODO: Once `CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS' is done, re-write this documentation! */
/* NOTE: There are no `DeeSeq_Check()' macros because they wouldn't make sense.
 *       Being derived from `DeeSeq_Type' is _NOT_ mandatory when writing a
 *       sequence class. The only thing that it does do is allow usercode
 *       to safely query whether or not an object implements all of the standard
 *       sequence functions.
 * Instead, a sequence object `ob' should be
 * detected using `DeeType_IsSequence(Dee_TYPE(ob))'.
 * The following things are required from sub-class of `Sequence':
 *     - Must either implement `tp_iter' or `tp_sizeob' + `tp_getitem'
 * The following things are implemented by `Sequence':
 *     - Abstraction that automatically defines the following operators:
 *        - tp_iter
 *        - tp_sizeob
 *        - tp_contains
 *        - tp_getitem  (for integer-index argument only)
 *        - tp_getrange  (for integer-index argument only)
 *        - tp_eq  (Lexicographic element-wise compare with other iterables)
 *        - tp_ne  (...)
 *        - tp_lo
 *        - tp_le
 *        - tp_gr
 *        - tp_ge
 *        - tp_add  (Concat 2 sequences)
 *        - tp_mul  (Repeat a sequence some number of times)
 *        - tp_or   (Repeat the left sequence, then only yield elements from right not found in the left one as well)
 *        - tp_and  (Convert the left sequence into a set, then only yield elements from the right found in that set)
 *        - tp_repr (Surrounded by `{ ... }', print a comma-separated list of all elements)
 *        - tp_bool (Indicates if the sequence is non-empty)
 *     - Abstraction that automatically defines the following getsets:
 *        - `isempty: bool'       (Read-only; same as `tp_bool', but negated)
 *        - `isnonempty: bool'    (Read-only; same as `tp_bool')
 *        - `length: int'         (Read-only; same as `tp_sizeob')
 *        - `first: Object'       (Read-write; same as `tp_getitem(0)' / `tp_setitem(0)')
 *        - `last: Object'        (Read-write; same as `tp_getitem(length - 1)' / `tp_setitem(length - 1)')
 *        - `each: Sequence'      (Read-only; Proxy sequence for construction expressions to-be applied to each element)
 *        - `ids: {int...}'       (Read-only; Proxy sequence for object IDs for elements)
 *        - `types: {Type...}'    (Read-only; Proxy sequence for element types)
 *        - `classes: {Type...}'  (Read-only; Proxy sequence for element classes)
 *        - `isfrozen: bool'      (Read-only; returns true if the sequence is frozen)
 *        - `frozen: Sequence'    (Read-only; returns a frozen copy of the sequence)
 *     - Abstraction that automatically defines the following class getsets:
 *        - `Iterator: Type'
 *          Evaluates to the internally used iterator type when `DeeObject_Iter()' would
 *          return it. Otherwise, accessing this field raises an `Error.AttributeError'.
 *          The intention here is that a sub-class defining its own iterator should override
 *          this field in order to return its own type.
 *     - Abstraction that automatically defines the following methods:
 *        - `front(): Object'
 *        - `back(): Object'
 *        - `reduce(combine: Callable, init?): Object'
 *           - Call `combine()' on two consecutive objects, reusing the result as
 *             the first argument in the next call; return the final sum-style value.
 *             NOTE: When the sequence is empty, return `none'
 *        - `filter(keep: Callable): Sequence'
 *           - Same as `(for (local x: this) if (keep(x)) x)'
 *        - `sum(): Object'
 *           - Same as `reduce((a, b) -> a + b);' or `this + ...'
 *           - Preferred way to concat sequences containing strings:
 *              - `print ["foo", "bar", "foobar"].sum(); // "foobarfoobar"'
 *        - `any(): bool'
 *           - Same as `reduce((a, b) -> a || b, false);', but stop on the first `true'.
 *           - Same as `this || ...'
 *        - `all(): bool'
 *           - Same as `reduce((a, b) -> a && b, true);', but stop on the first `false'.
 *           - Same as `this && ...'
 *        - `parity(): bool'
 *           - Same as `((#this.filter(x -> !!x)) % 2) != 0'
 *        - `min(key: Callable = none): Object'
 *        - `max(key: Callable = none): Object'
 *        - `count(ob: Object, key: Callable = none): int'
 *        - `locate(ob: Object, key: Callable = none): Object'
 *        - `rlocate(ob: Object, key: Callable = none): Object'
 *        - `locateall(ob: Object, key: Callable = none): Sequence'
 *        - `transform(callable transformation): Sequence'
 *           - Invoke `transformation()' on all items and return a sequence of all the results.
 *           - Same as `(for (local x: this) transformation(x));'
 *        - `contains(ob: Object, key: Callable = none): bool'
 *           - Same as the `tp_contains' operator, but allows for a key function to be used.
 *        - `partition(ob: Object, key: Callable = none): (Sequence, (ob), Sequence)'
 *        - `rpartition(ob: Object, key: Callable = none): (Sequence, (ob), Sequence)'
 *        - `startswith(ob: Object, key: Callable = none): bool'
 *        - `endswith(ob: Object, key: Callable = none): bool'
 *        - `find(ob: Object, key: Callable = none): int'
 *        - `rfind(ob: Object, key: Callable = none): int'
 *        - `index(ob: Object, key: Callable = none): int'
 *        - `rindex(ob: Object, key: Callable = none): int'
 *        - `join(items: Sequence): Sequence'
 *        - `strip(ob: Object, key: Callable = none): Sequence'
 *        - `lstrip(ob: Object, key: Callable = none): Sequence'
 *        - `rstrip(ob: Object, key: Callable = none): Sequence'
 *        - `split(sep: Object, key: Callable = none): Sequence'
 *        - `reversed(): Sequence'
 *        - `sorted(key: Callable = none): Sequence'
 *        - `segments(segsize: int): Sequence'
 *        - `countseq(seq: Sequence, key: Callable = none): int'
 *        - `containsseq(seq: Sequence, key: Callable = none): bool'
 *        - `partitionseq(seq: Sequence, key: Callable = none): (Sequence, Sequence, Sequence)'
 *        - `rpartitionseq(seq: Sequence, key: Callable = none): (Sequence, Sequence, Sequence)'
 *        - `startswithseq(seq: Sequence, key: Callable = none): bool'
 *        - `endswithseq(seq: Sequence, key: Callable = none): bool'
 *        - `findseq(seq: Sequence, key: Callable = none): int'
 *        - `rfindseq(seq: Sequence, key: Callable = none): int'
 *        - `indexseq(seq: Sequence, key: Callable = none): int'
 *        - `rindexseq(seq: Sequence, key: Callable = none): int'
 *        - `stripseq(items: Sequence, key: Callable = none): Sequence'
 *        - `lstripseq(items: Sequence, key: Callable = none): Sequence'
 *        - `rstripseq(items: Sequence, key: Callable = none): Sequence'
 *        - `splitseq(sep_seq: Sequence, key: Callable = none): Sequence'
 *        - ... // More exist; please consult http://localhost:8080/modules/deemon/Sequence
 * Some operations (Such as `tp_add') will create instances of special objects
 * that will only start invoking underlying operators when worked with:
 * >> function foo() {
 * >>     print "In foo()";
 * >>     yield 10;
 * >> }
 * >> function bar() {
 * >>     print "In bar()";
 * >>     yield 20;
 * >> }
 * >> 
 * >> local combine = foo()+bar(); // Create a merged-sequence object
 * >> // At this point, neither function has started executing, yet.
 * >> for (local x: combine) {
 * >>      print x; // "In foo()", "10", "In bar()", "20"
 * >> }
 * Other operators/functions may also invoke iteration more than once.
 * Which operators/functions do this is intentionally
 * not revealed and is subject to change in the future.
 * Code expecting certain types of sequences (or mutable sequence for that)
 * should always perform an explicit cast to the desired sequence type:
 * >> local a = [5, 10, 20, 30, 5];
 * >> local b = a.strip(5);
 * >> for (local x: b)
 * >>     print x; // 10 20 30
 * >> print type b; // Undefined and subject to change
 * >> b = [b...]; // Explicit cast to list; same as `b = list(b);'
 * HINT: Instantiating `seq' as-is will yield `Dee_EmptySeq',
 *       which in return is re-used internally as a placeholder
 *       to represent an empty, general-purpose sequence. */
DDATDEF DeeTypeObject DeeSeq_Type; /* `Sequence from deemon' */
#define DeeSeq_Check(ob) DeeObject_Implements(ob, &DeeSeq_Type)

/* Similar to what `DeeSeq_Type' is for all sequence-style types,
 * `DeeIterator_Type' is the the same for all iterator-type types.
 * The following things are implemented by `Iterator':
 *     - Abstraction that automatically defines the following operators:
 *        - tp_bool
 *           - Copy the iterator and return true if invoking tp_iter_next() yields a value.
 *        - tp_repr
 *           - Copy the iterator and print its remaining elements.
 *        - tp_add
 *           - Create a copy of the iterator and invoke `tp_inplace_add' on it.
 *        - tp_inc
 *           - Same as `tp_inplace_add' when the second operand is `1'.
 *        - tp_inplace_add
 *           - Advance the iterator by the integer representation of the second operand.
 *             When the second operand is negative, throw an `Error.ValueError.ArithmeticError.IntegerOverflow'
 *     - Abstraction that automatically defines the following methods:
 *        - `next(): Object'
 *           - Literally the same as invoking `operator __next__()', but has a more userfriendly name.
 *             This function will throw a `Signal.StopIteration' when the iterator has been exhausted.
 * As should be apparent, in order to use the full auto-API provided for
 * iterators, any iterator sub-class should implement a copy-constructor
 * in addition to the `tp_iter_next()' operator, although it should be
 * noted that implementation of the copy-constructor is _NOT_ mandatory,
 * as in some cases it is even impossible to pull off (such as for yield-
 * functions not marked as copyable or using non-copyable local variables).
 *
 * TODO: The "seq" field of iterators is deprecated (for the sake of performance in order to
 *       allow forwarding of iterators objects from underlying sequences, without needing to
 *       wrap them just so that "seq" returns the correct object type). */
DDATDEF DeeTypeObject DeeIterator_Type; /* `Iterator from deemon' */

/* An empty instance of a generic sequence object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSeq_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSeq_Type' should be considered stub/empty,
 *       but obviously something like an empty tuple is also an empty sequence. */
DDATDEF DeeObject      DeeSeq_EmptyInstance;
#define Dee_EmptySeq (&DeeSeq_EmptyInstance)
#define Dee_return_empty_seq  Dee_return_reference_(Dee_EmptySeq)

DDATDEF DeeObject           DeeIterator_EmptyInstance;
#define Dee_EmptyIterator (&DeeIterator_EmptyInstance)
#define Dee_return_empty_iterator Dee_return_reference_(Dee_EmptyIterator)



/*******************************************************************************************/
/*                                                                                         */
/* !!! BEGIN: DEPRECATED; PENDING REMOVAL AFTER CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
/*                                                                                         */
/*******************************************************************************************/

/* ==== NATIVE ITERATOR INTERFACE EXTENSIONS FOR TYPES ==== */
struct Dee_type_nii {
	/* Native Iterator Interface for types. */
#define Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL 0x0000 /* uni-directional iterator */
#define Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL  0x0001 /* bi-directional iterator */
#define Dee_TYPE_ITERX_FNORMAL              0x0000 /* Normal iterator flags. */
	__UINTPTR_HALF_TYPE__   nii_class; /* Iterator class (One of `TYPE_ITERX_CLASS_*') */
	__UINTPTR_HALF_TYPE__   nii_flags; /* Iterator flags (Set of `TYPE_ITERX_F*') */
	union {
		Dee_funptr_t       _nii_class_functions[10];

		struct {
			/* [0..1] Return the sequence associated with the iterator, or NULL on error.
			 * NOTE: Alternatively, a getset/member `seq' may be defined for this. */
			WUNUSED_T NONNULL_T((1))
			DREF DeeObject *(DCALL *nii_getseq)(DeeObject *__restrict self);

			/* [0..1] Get the iterator's position
			 * NOTE: Unbound sequence indices also count for this operation
			 * @return: * :         The iterator's current position, where the a starting position is 0
			 * @return: (size_t)-2: The position is indeterminate (the iterator may have become detached
			 *                      from its sequence, as can happen in linked lists when the iterator's
			 *                      link entry gets removed)
			 * @return: (size_t)-1: Error */
			WUNUSED_T NONNULL_T((1))
			size_t          (DCALL *nii_getindex)(DeeObject *__restrict self);

			/* [0..1] Set the iterator's position
			 * If the given `new_index' is greater than the max allowed index,
			 * the iterator is set to an exhausted state (i.e. points at the
			 * end of the associated sequence)
			 * NOTE: Unbound sequence indices also count for this operation
			 * @return:  0: Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_setindex)(DeeObject *__restrict self, size_t new_index);

			/* [0..1] Rewind the iterator to its starting position
			 * @return:  0: Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_rewind)(DeeObject *__restrict self);

			/* [0..1] Revert the iterator by at most `step' (When `step' is too large, same as `rewind')
			 * @return:  0: Success (new relative position couldn't be determined)
			 * @return:  1: Success (the iterator has reached its starting position)
			 * @return:  2: Success (the iterator hasn't reached its starting position)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_revert)(DeeObject *__restrict self, size_t step);

			/* [0..1] Advance the iterator by at most `step' (When `step' is too large, exhaust the iterator)
			 * @return:  0: Success (new relative position couldn't be determined)
			 * @return:  1: Success (the iterator has become exhausted)
			 * @return:  2: Success (the iterator hasn't become exhausted)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_advance)(DeeObject *__restrict self, size_t step);

			/* [0..1] Decrement the iterator by 1.
			 * @return:  0: Success
			 * @return:  1: The iterator was already at its starting location,
			 *              or the position couldn't be determined
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_prev)(DeeObject *__restrict self);

			/* [0..1] Increment the iterator, but don't generate a value
			 * NOTE: Unlike `tp_iter_next()', this operator shouldn't skip unbound entries,
			 *       meaning that (also unlike `tp_iter_next()'), the iterator's index should
			 *       only ever be incremented by 1.
			 * @return:  0: Success
			 * @return:  1: The iterator had already been exhausted,
			 *              or the position couldn't be determined
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_next)(DeeObject *__restrict self);

			/* [0..1] Check if the iterator has a predecessor
			 * @return:  0: No, it doesn't have one (index == 0)
			 * @return:  1: Yes, it does have one (index != 0)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_hasprev)(DeeObject *__restrict self);

			/* NOTE: `nii_hasnext' should be provided through `tp_bool' (`operator bool()') */

			/* [0..1] Peek the next iterator value, but don't actually advance the iterator.
			 * @return: ITER_DONE: The iterator has already been exhausted. */
			WUNUSED_T NONNULL_T((1))
			DREF DeeObject *(DCALL *nii_peek)(DeeObject *__restrict self);
		}                   nii_common;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define nii_common _dee_aunion.nii_common
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};


/* ==== NATIVE SEQUENCE INTERFACE EXTENSIONS FOR TYPES ==== */

struct Dee_type_nsi {
	/* Native Sequence Interface for types. */
#define Dee_TYPE_SEQX_CLASS_SEQ  0x0000 /* Sequence-like */
#define Dee_TYPE_SEQX_CLASS_MAP  0x0001 /* Mapping-like */
#define Dee_TYPE_SEQX_CLASS_SET  0x0002 /* Set-like */
#define Dee_TYPE_SEQX_FNORMAL    0x0000 /* Normal sequence flags. */
#define Dee_TYPE_SEQX_FMUTABLE   0x0001 /* The sequence is mutable. */
#define Dee_TYPE_SEQX_FRESIZABLE 0x0002 /* The sequence is resizable. */
	__UINTPTR_HALF_TYPE__   nsi_class; /* Sequence class (One of `TYPE_SEQX_CLASS_*') */
	__UINTPTR_HALF_TYPE__   nsi_flags; /* Sequence flags (Set of `TYPE_SEQX_F*') */
	union {
		Dee_funptr_t       _nsi_class_functions[24];

		struct {
			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1)) size_t (DCALL *nsi_getsize)(DeeObject *__restrict self);
		}                   nsi_common;

		struct { /* TYPE_SEQX_CLASS_SEQ */
#if 0 // def CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS // TODO
			Dee_funptr_t _unused_nsi_getsize;
			Dee_funptr_t _unused_nsi_getsize_fast;
			Dee_funptr_t _unused_nsi_getitem;
			Dee_funptr_t _unused_nsi_delitem;
			Dee_funptr_t _unused_nsi_setitem;
			Dee_funptr_t _unused_nsi_getitem_fast;
			Dee_funptr_t _unused_nsi_getrange;
			Dee_funptr_t _unused_nsi_getrange_n;
			Dee_funptr_t _unused_nsi_delrange;
			Dee_funptr_t _unused_nsi_delrange_n;
			Dee_funptr_t _unused_nsi_setrange;
			Dee_funptr_t _unused_nsi_setrange_n;
#else /* CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
			/* NOTE: If provided, these functions are only ever called as extensions to the
			 *       regular sequence operators, meaning that if you implement `nsi_getsize',
			 *       you are also _required_ to implement `tp_sizeob'
			 * NOTE: Any object implementing sequence extensions _must_ at
			 *       the very least provide the operators for `nsi_getsize'!
			 * NOTE: The `*_fast' variants are allowed to assume:
			 *       `index < ANY_PREVIOUS(nsi_getsize(ob)))' */

			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1))    size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self);

			/* Same as `nsi_getsize', but never throw any errors, and simply return (size_t)-1 to indicate failure.
			 * HINT: This callback is used to implement `DeeFastSeq_GetSize_deprecated()', with
			 *       either `nsi_getitem_fast()' or `nsi_getitem()' then being used
			 *       to implement the item lookup itself.
			 * WARNING: When implementing this operator, you must also implement at
			 *          least one of `nsi_getitem' or `nsi_getitem_fast' */
			WUNUSED_T NONNULL_T((1))    size_t          (DCALL *nsi_getsize_fast)(DeeObject *__restrict self);
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getitem)(DeeObject *__restrict self, size_t index);
			WUNUSED_T NONNULL_T((1))    int             (DCALL *nsi_delitem)(DeeObject *__restrict self, size_t index);
			WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *nsi_setitem)(DeeObject *self, size_t index, DeeObject *value);

			/* When `nsi_getitem_fast()' returns NULL, no error is thrown, and it means that the item is unbound.
			 * @param: index: Always `< SOME_PAST_CALL((*nsi_getsize_fast)(self))'
			 * @return: * :   A reference to the item at `index'
			 * @return: NULL: The item at `index' isn't bound (NO ERROR IS THROWN IN THIS CASE) */
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getitem_fast)(DeeObject *__restrict self, size_t index);

			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getrange)(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getrange_n)(DeeObject *__restrict self, Dee_ssize_t start); /* end: Dee_None */
			WUNUSED_T NONNULL_T((1))    int             (DCALL *nsi_delrange)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
			WUNUSED_T NONNULL_T((1))    int             (DCALL *nsi_delrange_n)(DeeObject *self, Dee_ssize_t start); /* end: Dee_None */
			WUNUSED_T NONNULL_T((1, 4)) int             (DCALL *nsi_setrange)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
			WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *nsi_setrange_n)(DeeObject *self, Dee_ssize_t start, DeeObject *values); /* end: Dee_None */
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
		}                   nsi_seqlike;

		struct { /* TYPE_SEQX_CLASS_MAP */
			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1))       size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self);

			/* Same as `mapping.Iterator.operator next()' of the mapping's core iterator,
			 * however only return the key / value, rather than a key-value tuple.
			 * @param: iterator: An iterator object, as returned by `mapping.operator iter()' */
			WUNUSED_T NONNULL_T((1))       DREF DeeObject *(DCALL *nsi_nextkey)(DeeObject *__restrict iterator);
			WUNUSED_T NONNULL_T((1))       DREF DeeObject *(DCALL *nsi_nextvalue)(DeeObject *__restrict iterator);

			/* Lookup the given `key' and return its association, or `defl' if it doesn't yet exist.
			 * WARNING: `defl' may be ITER_DONE, in which case you really shouldn't incref() it! */
			WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *nsi_getdefault)(DeeObject *self, DeeObject *key, DeeObject *defl);

			/* Check if the mapping contains a element for `key' and return that element's value, or
			 * insert a new element for `key', setting its value to `defl', then returning `defl'. */
			WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *nsi_setdefault)(DeeObject *self, DeeObject *key, DeeObject *defl);

			/* Update an existing mapping element
			 * @param: p_oldvalue: When non-NULL, store a reference to the old item here.
			 * @return: 1:  The existing key was updated (*p_oldvalue is set to the previous value)
			 * @return: 0:  `key' doesn't exist. (*p_oldvalue is left unchanged)
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 2, 3)) int             (DCALL *nsi_updateold)(DeeObject *self, DeeObject *key, DeeObject *value, DREF DeeObject **p_oldvalue);

			/* Insert a new mapping element, but don't change a pre-existing one
			 * @param: p_oldvalue: When non-NULL, store a reference to the old item here.
			 * @return: 1:  `key' already exists (*p_oldvalue is set to its associated value)
			 * @return: 0:  `key' didn't exist and was thus added (*p_oldvalue is left unchanged)
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 2, 3)) int             (DCALL *nsi_insertnew)(DeeObject *self, DeeObject *key, DeeObject *value, DREF DeeObject **p_oldvalue);
		}                   nsi_maplike;

		struct { /* TYPE_SEQX_CLASS_SET */
			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1))    size_t (DCALL *nsi_getsize)(DeeObject *__restrict self);
		}                   nsi_setlike;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define nsi_common  _dee_aunion.nsi_common
#define nsi_seqlike _dee_aunion.nsi_seqlike
#define nsi_maplike _dee_aunion.nsi_maplike
#define nsi_setlike _dee_aunion.nsi_setlike
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};


/*****************************************************************************************/
/*                                                                                       */
/* !!! END: DEPRECATED; PENDING REMOVAL AFTER CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
/*                                                                                       */
/*****************************************************************************************/



struct Dee_seq_range {
	union {
#undef sr_start
#undef sr_istart
		size_t      sr_start;  /* Start index (clamped) */
		Dee_ssize_t sr_istart; /* Start index (input) */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion_s
#define sr_start  _dee_aunion_s.sr_start
#define sr_istart _dee_aunion_s.sr_istart
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	union {
#undef sr_end
#undef sr_iend
		size_t      sr_end;    /* End index (clamped) */
		Dee_ssize_t sr_iend;   /* End index (input) */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion_e
#define sr_end  _dee_aunion_e.sr_end
#define sr_iend _dee_aunion_e.sr_iend
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};


/* Clamp a range, as given to `operator [:]' & friends to the bounds
 * accepted by the associated sequence. This handles stuff like negative
 * index over-roll and past-the-end truncation. */
#define DeeSeqRange_Clamp(prange, istart, iend, size) \
	((prange)->sr_istart = (istart),                  \
	 (prange)->sr_iend   = (iend),                    \
	 _DeeSeqRange_Clamp(prange, size))
#define _DeeSeqRange_Clamp(prange, size)              \
	(void)(((prange)->sr_start <= (prange)->sr_end && \
	        (prange)->sr_end <= (size)) ||            \
	       (DeeSeqRange_DoClamp(prange, size), 0))
DFUNDEF ATTR_INOUT(1) void DCALL
DeeSeqRange_DoClamp(struct Dee_seq_range *__restrict self,
                    size_t size);

/* Specialized version of `DeeSeqRange_DoClamp()' for `[istart:none]' range expressions. */
#define DeeSeqRange_Clamp_n(istart, size) \
	((size_t)(istart) <= (size) ? (size_t)(istart) : DeeSeqRange_DoClamp_n(istart, size))
DFUNDEF ATTR_CONST WUNUSED size_t DCALL
DeeSeqRange_DoClamp_n(Dee_ssize_t start, size_t size);


/* Lookup the closest NSI descriptor for `tp', or return `NULL'
 * if the top-most type implementing any sequence operator doesn't
 * expose NSI functionality. */
DFUNDEF WUNUSED NONNULL((1)) struct Dee_type_nsi const *DCALL
DeeType_NSI(DeeTypeObject *__restrict tp);

/* Create new range sequence objects. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRange_New(DeeObject *begin, DeeObject *end, DeeObject *step);
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeRange_NewInt(Dee_ssize_t begin, Dee_ssize_t end, Dee_ssize_t step);

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Sum(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Any(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_All(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Min(DeeObject *self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Max(DeeObject *self);


#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS

/* @return: == -2: An error occurred.
 * @return: == -1: `self < some_object'
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `self > some_object' */
INTDEF WUNUSED NONNULL((1))    int DCALL DeeSeq_CompareIV(DeeObject *lhs, DeeObject *const *rhsv, size_t rhsc);                      /* ITERATOR <=> VECTOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_CompareII(DeeObject *lhs, DeeObject *rhs);                                           /* ITERATOR <=> ITERATOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_CompareIS(DeeObject *lhs, DeeObject *rhs);                                           /* ITERATOR <=> SEQUENCE */

/* @return: == -1: An error occurred.
 * @return: == 0:  Sequences differ
 * @return: == 1:  Sequences are equal */
INTDEF WUNUSED NONNULL((1))    int DCALL DeeSeq_EqIV(DeeObject *lhs, DeeObject *const *rhsv, size_t rhsc);          /* ITERATOR == VECTOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EqII(DeeObject *lhs, DeeObject *rhs);                               /* ITERATOR == ITERATOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EqIS(DeeObject *lhs, DeeObject *rhs);                               /* ITERATOR == SEQUENCE */
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
#endif /* CONFIG_BUILDING_DEEMON */


/* Possible return values for `DeeType_GetSeqClass()' */
#define Dee_SEQCLASS_UNKNOWN 0 /* Never returned by `DeeType_GetSeqClass()' (used internally) */
#define Dee_SEQCLASS_NONE    1 /* Type does not inherit from "Sequence" */
#define Dee_SEQCLASS_SEQ     2 /* Type inherits from "Sequence" */
#define Dee_SEQCLASS_SET     3 /* Type inherits from "Sequence" and "Set" */
#define Dee_SEQCLASS_MAP     4 /* Type inherits from "Sequence" and "Mapping" */
#define Dee_SEQCLASS_COUNT   5

/* Sequence type classification
 * @return: * : One of `Dee_SEQCLASS_*' */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetSeqClass(DeeTypeObject const *__restrict self);


/* Construct a new reference-vector object that can be iterated
 * and used to potentially modify the elements of a given `vector'.
 * NOTE: When write-access is granted, `vector' should be `[0..1][0..length]',
 *       whereas when write-access is not possible, then the disposition of
 *       elements of `vector' doesn't matter and can either be `[0..1]' or `[1..1]'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *owner, size_t length,
                 DeeObject **vector,
#ifndef CONFIG_NO_THREADS
                 Dee_atomic_rwlock_t *plock
#else /* !CONFIG_NO_THREADS */
                 bool writable
#endif /* CONFIG_NO_THREADS */
                 );

#ifdef __INTELLISENSE__
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeRefVector_NewReadonly(DeeObject *__restrict owner, size_t length,
                         DeeObject *const *__restrict vector);
#elif defined(CONFIG_NO_THREADS)
#define DeeRefVector_NewReadonly(owner, length, vector) \
	DeeRefVector_New(owner, length, (DeeObject **)(vector), false)
#else /* CONFIG_NO_THREADS */
#define DeeRefVector_NewReadonly(owner, length, vector) \
	DeeRefVector_New(owner, length, (DeeObject **)(vector), NULL)
#endif /* !CONFIG_NO_THREADS */


/* Type of object returned by `DeeSharedVector_NewShared()' */
DDATDEF DeeTypeObject DeeSharedVector_Type;

/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedVector_Decref()' is called.
 * NOTE: This function can implicitly inherit a reference to each item of the
 *       given vector, though does not actually inherit the vector itself:
 *       - DeeSharedVector_Decref:            The `vector' arg here is `DREF DeeObject *const *'
 *       - DeeSharedVector_DecrefNoGiftItems: The `vector' arg here is `DeeObject *const *'
 * NOTE: The returned object cannot be used to change out the elements
 *       of the given `vector', meaning that _it_ can still be [const] */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeSharedVector_NewShared(size_t length, /*maybe*/ DREF DeeObject *const *vector);

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained objects.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedVector_Decref(DREF DeeObject *__restrict self);

/* Same as `DeeSharedVector_Decref()', but should be used if the caller
 * does *not* want to gift the vector references to all of its items.
 * s.a.: the "maybe DREF" annotated on the `vector' arcument of
 *       `DeeSharedVector_NewShared()' */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedVector_DecrefNoGiftItems(DREF DeeObject *__restrict self);



/* Check if `self' is a fast-sequence object, and return its (current)
 * length if it is, or return `DEE_FASTSEQ_NOTFAST_DEPRECATED' if it isn't.
 * A fast-sequence object is a vector-based object implemented by the
 * deemon C-core, meaning that its size can quickly be determined,
 * and items can quickly be accessed, given their index.
 * The following types function as fast-sequence-compatible:
 *  - Tuple
 *  - List
 *  - _SharedVector      (Created by a `ASM_CALL_SEQ' instruction -- `call top, [#X]')
 *  - _SeqSubRange       (Only if the sub-ranged sequence is a fast-sequence)
 *  - _SeqSubRangeN      (*ditto*)
 *  - _SeqTransformation (Only if the sequence being transformed is a fast-sequence)
 *  - _SeqIntRange
 *  - string
 *  - Bytes
 * Sub-classes of these types are not fast-sequence-compatible. */
DFUNDEF WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSize_deprecated(DeeObject *__restrict self); /* Deprecated */
#define DEE_FASTSEQ_NOTFAST_DEPRECATED ((size_t)-1)

/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize_deprecated(self)' when
 * `self' is an immutable sequence (anything other than `List' and `_SharedVector').
 * WARNING: This function may _ONLY_ be used if `DeeFastSeq_GetSize_deprecated(self)'
 *          returned something other than `DEE_FASTSEQ_NOTFAST_DEPRECATED'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItem_deprecated(DeeObject *__restrict self, size_t index); /* Deprecated */





/* New fast-sequence interface */
typedef struct {
	DeeObject *fsq_self; /* [1..1] The sequence being enumerated. */
	size_t     fsq_size; /* The # of items in `fsq_self' */
	/* [1..1] Callback to get the index'th element of "fsq_self".
	 * NOTE: The caller must ensure that `index < fsq_size'
	 * HINT: This is the relevant "tp_getitem_index_fast" operator.
	 * @return: * :   A reference to the index'th element of "fsq_self".
	 * @return: NULL: The index'th element of "fsq_self" isn't bound (no error was thrown) */
	DREF DeeObject *(DCALL *fsq_getitem_index_fast)(DeeObject *__restrict self, size_t index);
} DeeFastSeq;

#define DeeFastSeq_GetSize(self)        ((self)->fsq_size)

/* Return the index'th element of "self".
 * @return: * :   A reference to the index'th element of "self".
 * @return: NULL: The index'th element of "self" isn't bound (no error was thrown) */
#define DeeFastSeq_GetItem(self, index) ((*(self)->fsq_getitem_index_fast)((self)->fsq_self, index))

/* Try to load index-based fast sequence controls for "seq".
 * @return: true:  Success. You may use other `DeeFastSeq_*' to access sequence elements.
 * @return: false: Failure. Given `seq' does not implement `tp_getitem_index_fast' */
#ifndef __OPTIMIZE_SIZE__
#define DeeFastSeq_Init(self, seq) (((self)->fsq_size = DeeFastSeq_Init_impl(self, seq)) != (size_t)-1)
#endif /* !__OPTIMIZE_SIZE__ */
DFUNDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeFastSeq_Init)(DeeFastSeq *__restrict self, DeeObject *__restrict seq);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t (DCALL DeeFastSeq_Init_impl)(DeeFastSeq *__restrict self, DeeObject *__restrict seq);



/* Allocate a suitable heap-vector for all the elements of a given sequence,
 * before returning that vector (then populated by [1..1] references), which
 * the caller must inherit upon success.
 * @return: * :   A vector of objects (with a length of `*p_length'),
 *                that must be freed using `Dee_Free', before inheriting
 *                a reference to each of its elements.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    /*[out]*/ size_t *__restrict p_length);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc(DeeObject *__restrict self,
                             /*[out]*/ size_t *__restrict p_length,
                             /*[out]*/ size_t *__restrict p_allocated);
#ifdef Dee_MallocUsableSize
DFUNDEF WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc2(DeeObject *__restrict self,
                              /*[out]*/ size_t *__restrict p_length);
#endif /* Dee_MallocUsableSize */

/* Same as `DeeSeq_AsHeapVectorWithAlloc()', however also inherit
 * a pre-allocated heap-vector `*p_vector' with an allocated size
 * of `IN(*p_allocated) * sizeof(DeeObject *)', which is updated
 * as more memory needs to be allocated.
 * NOTE: `*p_vector' may be updated to point to a new vector, even
 *       when the function fails (i.e. (size_t)-1 is returned)
 * @param: p_vector:    A pointer to a preallocated object-vector `[0..IN(*p_allocated)]'
 *                      May only point to a `NULL' vector when `IN(*p_allocated)' is ZERO(0).
 *                      Upon return, this pointer may have been updated to point to a
 *                      realloc()-ated vector, should the need to allocate more memory
 *                      have arisen.
 * @param: p_allocated: A pointer to an information field describing how much pointers
 *                      are allocated upon entry / how much are allocated upon exit.
 *                      Just as `p_vector', this pointer may be updated, even upon error.
 * @return: * :         The amount of filled in objects in `*p_vector'
 * @return: (size_t)-1: An error occurred. Note that both `*p_vector' and `*p_allocated'
 *                      may have been modified since entry, with their original values
 *                      no longer being valid! */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                  /*in-out*/ size_t *__restrict p_allocated);
#ifdef Dee_MallocUsableSize
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse2(DeeObject *__restrict self,
                                   /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector);
#endif /* Dee_MallocUsableSize */

/* Same as `DeeSeq_AsHeapVectorWithAllocReuse()', but assume
 * that `IN(*p_allocated) >= offset', while also leaving the first
 * `offset' vector entries untouched and inserting the first enumerated
 * sequence element at `(*p_vector)[offset]', rather than `(*p_vector)[0]'
 * -> This function can be used to efficiently append elements to a
 *    vector which may already contain other objects upon entry. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                        /*in-out*/ size_t *__restrict p_allocated,
                                        /*in*/ size_t offset);
#ifdef Dee_MallocUsableSize
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset2(DeeObject *__restrict self,
                                         /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                         /*in*/ size_t offset);
#endif /* Dee_MallocUsableSize */

DECL_END

#endif /* !GUARD_DEEMON_SEQ_H */
