/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SEQ_H
#define GUARD_DEEMON_SEQ_H 1

#include "api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define return_empty_seq                Dee_return_empty_seq
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




/* NOTE: There are no `DeeSeq_Check()' macros because they wouldn't make sense.
 *       Being derived from `DeeSeq_Type' is _NOT_ mandatory when writing a
 *       sequence class. The only thing that it does do is allow usercode
 *       to safely query whether or not an object implements all of the standard
 *       sequence functions.
 * Instead, a sequence object `ob' should be
 * detected using `DeeType_IsSequence(Dee_TYPE(ob))'.
 * The following things are required from sub-class of `Sequence':
 *     - Must either implement `tp_iter_self' or `tp_size' + `tp_get'
 * The following things are implemented by `Sequence':
 *     - Abstraction that automatically defines the following operators:
 *        - tp_iter_self
 *        - tp_size
 *        - tp_contains
 *        - tp_get  (for integer-index argument only)
 *        - tp_range_get  (for integer-index argument only)
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
 *        - `length: int'         (Read-only; same as `tp_size')
 *        - `first: Object'       (Read-write; same as `tp_get(0)' / `tp_set(0)')
 *        - `last: Object'        (Read-write; same as `tp_get(length - 1)' / `tp_set(length - 1)')
 *        - `ismutable: bool'     (Read-only; s.a. `DeeSeq_IsMutable()')
 *        - `isresizable: bool'   (Read-only; s.a. `DeeSeq_IsResizable()')
 *        - `each: Sequence'      (Read-only; Proxy sequence for construction expressions to-be applied to each element)
 *        - `ids: {int...}'       (Read-only; Proxy sequence for object IDs for elements)
 *        - `types: {Type...}'    (Read-only; Proxy sequence for element types)
 *        - `classes: {Type...}'  (Read-only; Proxy sequence for element classes)
 *        - `isfrozen: bool'      (Read-only; returns true if the sequence is frozen)
 *        - `frozen: Sequence'    (Read-only; returns a frozen copy of the sequence)
 *     - Abstraction that automatically defines the following class getsets:
 *        - `Iterator: Type'
 *          Evaluates to the internally used iterator type when `DeeObject_IterSelf()' would
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
 */
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


/* ==== NATIVE ITERATOR INTERFACE EXTENSIONS FOR TYPES ==== */
struct Dee_type_nii {
	/* Native Iterator Interface for types. */
#define Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL 0x0000 /* uni-directional iterator */
#define Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL  0x0001 /* bi-directional iterator */
#define Dee_TYPE_ITERX_FNORMAL              0x0000 /* Normal iterator flags. */
#if __SIZEOF_POINTER__ == 4
	uint16_t                nii_class; /* Iterator class (One of `TYPE_ITERX_CLASS_*') */
	uint16_t                nii_flags; /* Iterator flags (Set of `TYPE_ITERX_F*') */
#elif __SIZEOF_POINTER__ == 8
	uint32_t                nii_class; /* Iterator class (One of `TYPE_ITERX_CLASS_*') */
	uint32_t                nii_flags; /* Iterator class (Set of `TYPE_ITERX_F*') */
#else /* __SIZEOF_POINTER__ == ... */
#error "Unsupported __SIZEOF_POINTER__"
#endif /* __SIZEOF_POINTER__ != ... */
	union {
		Dee_funptr_t       _nii_class_functions[10];

		struct {
			/* Return the sequence associated with the iterator, or NULL on error.
			 * NOTE: Alternatively, a getset/member `seq' may be defined for this. */
			WUNUSED_T NONNULL_T((1))
			DREF DeeObject *(DCALL *nii_getseq)(DeeObject *__restrict self);

			/* Get the iterator's position
			 * NOTE: Unbound sequence indices also count for this operation
			 * @return: * :         The iterator's current position, where the a starting position is 0
			 * @return: (size_t)-2: The position is indeterminate (the iterator may have become detached
			 *                      from its sequence, as can happen in linked lists when the iterator's
			 *                      link entry gets removed)
			 * @return: (size_t)-1: Error */
			WUNUSED_T NONNULL_T((1))
			size_t          (DCALL *nii_getindex)(DeeObject *__restrict self);

			/* Set the iterator's position
			 * If the given `new_index' is greater than the max allowed index,
			 * the iterator is set to an exhausted state (i.e. points at the
			 * end of the associated sequence)
			 * NOTE: Unbound sequence indices also count for this operation
			 * @return:  0: Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_setindex)(DeeObject *__restrict self, size_t new_index);

			/* Rewind the iterator to its starting position
			 * @return:  0: Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_rewind)(DeeObject *__restrict self);

			/* Revert the iterator by at most `step' (When `step' is too large, same as `rewind')
			 * @return:  0: Success (new relative position couldn't be determined)
			 * @return:  1: Success (the iterator has reached its starting position)
			 * @return:  2: Success (the iterator hasn't reached its starting position)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_revert)(DeeObject *__restrict self, size_t step);

			/* Advance the iterator by at most `step' (When `step' is too large, exhaust the iterator)
			 * @return:  0: Success (new relative position couldn't be determined)
			 * @return:  1: Success (the iterator has become exhausted)
			 * @return:  2: Success (the iterator hasn't become exhausted)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_advance)(DeeObject *__restrict self, size_t step);

			/* Decrement the iterator by 1.
			 * @return:  0: Success
			 * @return:  1: The iterator was already at its starting location,
			 *              or the position couldn't be determined
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_prev)(DeeObject *__restrict self);

			/* Increment the iterator, but don't generate a value
			 * NOTE: Unlike `tp_iter_next()', this operator shouldn't skip unbound entries,
			 *       meaning that (also unlike `tp_iter_next()'), the iterator's index should
			 *       only ever be incremented by 1.
			 * @return:  0: Success
			 * @return:  1: The iterator had already been exhausted,
			 *              or the position couldn't be determined
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_next)(DeeObject *__restrict self);

			/* Check if the iterator has a predecessor
			 * @return:  0: No, it doesn't have one (index == 0)
			 * @return:  1: Yes, it does have one (index != 0)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_hasprev)(DeeObject *__restrict self);

			/* NOTE: `nii_hasnext' should be provided through `tp_bool' (`operator bool()') */
			/* Peek the next iterator value, but don't actually advance the iterator.
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
#if __SIZEOF_POINTER__ == 4
	uint16_t                nsi_class; /* Sequence class (One of `TYPE_SEQX_CLASS_*') */
	uint16_t                nsi_flags; /* Sequence flags (Set of `TYPE_SEQX_F*') */
#elif __SIZEOF_POINTER__ == 8
	uint32_t                nsi_class; /* Sequence class (One of `TYPE_SEQX_CLASS_*') */
	uint32_t                nsi_flags; /* Sequence class (Set of `TYPE_SEQX_F*') */
#else /* __SIZEOF_POINTER__ == ... */
#error "Unsupported __SIZEOF_POINTER__"
#endif /* __SIZEOF_POINTER__ != ... */
	union {
		Dee_funptr_t       _nsi_class_functions[22];

		struct {
			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1)) size_t (DCALL *nsi_getsize)(DeeObject *__restrict self);
		}                   nsi_common;

		struct { /* TYPE_SEQX_CLASS_SEQ */
			/* NOTE: If provided, these functions are only ever called as extensions to the
			 *       regular sequence operators, meaning that if you implement `nsi_getsize',
			 *       you are also _required_ to implement `tp_size'
			 * NOTE: Any object implementing sequence extensions _must_ at
			 *       the very least provide the operators for `nsi_getsize'!
			 * NOTE: The `*_fast' variants are allowed to assume:
			 *      `index < ANY_PREVIOUS(nsi_getsize(ob)))' */

			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1))    size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self);

			/* Same as `nsi_getsize', but never throw any errors, and simply return (size_t)-1 to indicate failure.
			 * HINT: This callback is used to implement `DeeFastSeq_GetSize()', with
			 *       either `nsi_getitem_fast()' or `nsi_getitem()' then being used
			 *       to implement the item lookup itself.
			 * WARNING: When implementing this operator, you must also implement at
			 *          least one of `nsi_getitem' or `nsi_getitem_fast' */
			WUNUSED_T NONNULL_T((1))    size_t          (DCALL *nsi_getsize_fast)(DeeObject *__restrict self);
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getitem)(DeeObject *__restrict self, size_t index);
			WUNUSED_T NONNULL_T((1))    int             (DCALL *nsi_delitem)(DeeObject *__restrict self, size_t index);
			WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *nsi_setitem)(DeeObject *self, size_t index, DeeObject *value);

			/* When `nsi_getitem_fast()' returns NULL, no error is thrown, and it means that the item is unbound. */
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getitem_fast)(DeeObject *__restrict self, size_t index);

			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getrange)(DeeObject *__restrict self, Dee_ssize_t start, Dee_ssize_t end);
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_getrange_n)(DeeObject *__restrict self, Dee_ssize_t start); /* end: Dee_None */
			WUNUSED_T NONNULL_T((1, 4)) int             (DCALL *nsi_setrange)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
			WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *nsi_setrange_n)(DeeObject *self, Dee_ssize_t start, DeeObject *values); /* end: Dee_None */

			/* NOTE: start/end in here operate differently (and simpler) than in ranges:
			 *       If either value is `>= nsi_getsize()', truncate it to that length.
			 * NOTE: Comparisons should be performed as `keyed_search_item == key(this[?])'
			 * @return: * : Index of the matching item
			 * @return: (size_t)-1: Index not found.
			 * @return: (size_t)-2: Error. */
			WUNUSED_T NONNULL_T((1, 4)) size_t          (DCALL *nsi_find)(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key);
			WUNUSED_T NONNULL_T((1, 4)) size_t          (DCALL *nsi_rfind)(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key);

			WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *nsi_xch)(DeeObject *self, size_t index, DeeObject *value);
			WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *nsi_insert)(DeeObject *self, size_t index, DeeObject *value);
			WUNUSED_T NONNULL_T((1, 3)) int             (DCALL *nsi_insertall)(DeeObject *self, size_t index, DeeObject *values);
			WUNUSED_T NONNULL_T((1))    int             (DCALL *nsi_insertvec)(DeeObject *self, size_t index, size_t insertc, DeeObject *const *insertv);

			/* NOTE: When `index' is lower than ZERO(0), the length of the sequence `self' must be added
			 *       first, such that `nsi_pop(self, -1)' is equivalent to a `popback()' function call. */
			WUNUSED_T NONNULL_T((1))    DREF DeeObject *(DCALL *nsi_pop)(DeeObject *__restrict self, Dee_ssize_t index);

			/* NOTE: erase differs from delrange, in that erase _always_ removes the indices,
			 *       while delrange is allowed to leave the index range as unbound.
			 * @return: * : Number or erased items.
			 * @return: (size_t)-1: Error. */
			WUNUSED_T NONNULL_T((1))    size_t          (DCALL *nsi_erase)(DeeObject *__restrict self, size_t index, size_t count);

			/* Remove or unbind the first/last/all instance(s) of `elem'
			 * NOTE: Comparisons should be performed as `keyed_search_item == key(this[?])'
			 * @return: 0 : Element not found.
			 * @return: 1 : Element was unbound.
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 4)) int             (DCALL *nsi_remove)(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key);
			WUNUSED_T NONNULL_T((1, 4)) int             (DCALL *nsi_rremove)(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key);

			/* NOTE: Comparisons should be performed as `keyed_search_item == key(this[?])'
			 * @return: * : The number of removed items.
			 * @return: (size_t)-1: Error. */
			WUNUSED_T NONNULL_T((1, 4)) size_t          (DCALL *nsi_removeall)(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key);
			WUNUSED_T NONNULL_T((1, 4)) size_t          (DCALL *nsi_removeif)(DeeObject *self, size_t start, size_t end, DeeObject *should);
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
			 * @param: poldvalue: When non-NULL, store a reference to the old item here.
			 * @return: 1:  The existing key was updated.
			 * @return: 0: `key' doesn't exist. (*poldvalue is left unchanged)
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 2, 3)) int             (DCALL *nsi_updateold)(DeeObject *self, DeeObject *key, DeeObject *value, DREF DeeObject **poldvalue);

			/* Insert a new mapping element, but don't change a pre-existing one
			 * @param: poldvalue: When non-NULL, store a reference to the old item here.
			 * @return: 1:  A new element was inserted.
			 * @return: 0: `key' doesn't exist. (*poldvalue is left unchanged)
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 2, 3)) int             (DCALL *nsi_insertnew)(DeeObject *self, DeeObject *key, DeeObject *value, DREF DeeObject **poldvalue);
		}                   nsi_maplike;

		struct { /* TYPE_SEQX_CLASS_SET */
			/* [1..1] ERROR: (size_t)-1 */
			WUNUSED_T NONNULL_T((1))    size_t (DCALL *nsi_getsize)(DeeObject *__restrict self);

			/* Insert a new `key' into the set
			 * @return: 1:  The given `key' was inserted.
			 * @return: 0:  A identical key was already apart of the set.
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 2)) int    (DCALL *nsi_insert)(DeeObject *self, DeeObject *key);

			/* Remove a given `key' from the set
			 * @return: 1:  The given `key' was removed.
			 * @return: 0:  The given `key' could not be found within the set.
			 * @return: -1: Error. */
			WUNUSED_T NONNULL_T((1, 2)) int    (DCALL *nsi_remove)(DeeObject *self, DeeObject *key);
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
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Min(DeeObject *self, DeeObject *key);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Max(DeeObject *self, DeeObject *key);

#ifdef CONFIG_BUILDING_DEEMON
/* Mutable-sequence API */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DelItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_SetItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_XchItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DelRange(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_SetRange(DeeObject *self, size_t start, size_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DelRangeN(DeeObject *__restrict self, size_t start);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_SetRangeN(DeeObject *self, size_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_Insert(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_InsertAll(DeeObject *self, size_t index, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_Append(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_Extend(DeeObject *self, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_InplaceExtend(DREF DeeObject **__restrict pself, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_InplaceRepeat(DREF DeeObject **__restrict pself, DeeObject *count);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Erase(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_PopItem(DeeObject *__restrict self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_Remove(DeeObject *self, size_t start, size_t end, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_RRemove(DeeObject *self, size_t start, size_t end, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_RemoveAll(DeeObject *self, size_t start, size_t end, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_RemoveIf(DeeObject *self, size_t start, size_t end, DeeObject *should);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_Fill(DeeObject *self, size_t start, size_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Reverse(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Sort(DeeObject *self, DeeObject *key);

/* Determine if a given sequence is mutable or resizable.
 * @return: 1:  The sequence is mutable or resizable.
 * @return: 0:  The sequence isn't mutable or resizable.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_IsMutable(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_IsResizable(DeeObject *__restrict self);

/* NOTE: Technically, all of these functions can be used on any type of object,
 *       but all objects derived from `DeeSeq_Type' automatically implement
 *       all of them as member functions.
 *       With that in mind, any type implementing the `tp_seq' interface
 *       with the intention of behaving as an Iterable, should probably
 *       be derived from `DeeSeq_Type' as this allows usercode to query
 *       for a general purpose sequence by writing `x is Sequence from deemon' */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Size(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_GetItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_GetRange(DeeObject *__restrict self, size_t begin, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_GetRangeN(DeeObject *__restrict self, size_t begin);

/* @return: == -2: An error occurred.
 * @return: == -1: `self < some_object'
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `self > some_object' */
INTDEF WUNUSED                 int DCALL DeeSeq_CompareVV(DeeObject *const *lhsv, size_t lhsc, DeeObject *const *rhsv, size_t rhsc); /* VECTOR <=> VECTOR */
INTDEF WUNUSED NONNULL((3))    int DCALL DeeSeq_CompareVF(DeeObject *const *lhsv, size_t lhsc, DeeObject *rhs, size_t rhsc);         /* VECTOR <=> DeeFastSeq */
INTDEF WUNUSED NONNULL((3))    int DCALL DeeSeq_CompareVI(DeeObject *const *lhsv, size_t lhsc, DeeObject *rhs);                      /* VECTOR <=> ITERATOR */
INTDEF WUNUSED NONNULL((3))    int DCALL DeeSeq_CompareVS(DeeObject *const *lhsv, size_t lhsc, DeeObject *rhs);                      /* VECTOR <=> SEQUENCE */
INTDEF WUNUSED NONNULL((1))    int DCALL DeeSeq_CompareFV(DeeObject *lhs, size_t lhsc, DeeObject *const *rhsv, size_t rhsc);         /* DeeFastSeq <=> VECTOR */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_CompareFF(DeeObject *lhs, size_t lhsc, DeeObject *rhs, size_t rhsc);                 /* DeeFastSeq <=> DeeFastSeq */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_CompareFI(DeeObject *lhs, size_t lhsc, DeeObject *rhs);                              /* DeeFastSeq <=> ITERATOR */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_CompareFS(DeeObject *lhs, size_t lhsc, DeeObject *rhs);                              /* DeeFastSeq <=> SEQUENCE */
INTDEF WUNUSED NONNULL((1))    int DCALL DeeSeq_CompareIV(DeeObject *lhs, DeeObject *const *rhsv, size_t rhsc);                      /* ITERATOR <=> VECTOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_CompareIF(DeeObject *lhs, DeeObject *rhs, size_t rhsc);                              /* ITERATOR <=> DeeFastSeq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_CompareII(DeeObject *lhs, DeeObject *rhs);                                           /* ITERATOR <=> ITERATOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_CompareIS(DeeObject *lhs, DeeObject *rhs);                                           /* ITERATOR <=> SEQUENCE */

/* @return: == -1: An error occurred.
 * @return: == 0:  Sequences differ
 * @return: == 1:  Sequences are equal */
INTDEF WUNUSED                 int DCALL DeeSeq_EqVV(DeeObject *const *lhsv, DeeObject *const *rhsv, size_t elemc); /* VECTOR == VECTOR */
INTDEF WUNUSED NONNULL((2))    int DCALL DeeSeq_EqVF(DeeObject *const *lhsv, DeeObject *rhs, size_t elemc);         /* VECTOR == DeeFastSeq */
INTDEF WUNUSED NONNULL((3))    int DCALL DeeSeq_EqVI(DeeObject *const *lhsv, size_t lhsc, DeeObject *rhs);          /* VECTOR == ITERATOR */
INTDEF WUNUSED NONNULL((3))    int DCALL DeeSeq_EqVS(DeeObject *const *lhsv, size_t lhsc, DeeObject *rhs);          /* VECTOR == SEQUENCE */
INTDEF WUNUSED NONNULL((1))    int DCALL DeeSeq_EqFV(DeeObject *lhs, DeeObject *const *rhsv, size_t elemc);         /* DeeFastSeq == VECTOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EqFF(DeeObject *lhs, DeeObject *rhs, size_t elemc);                 /* DeeFastSeq == DeeFastSeq */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_EqFI(DeeObject *lhs, size_t lhsc, DeeObject *rhs);                  /* DeeFastSeq == ITERATOR */
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_EqFS(DeeObject *lhs, size_t lhsc, DeeObject *rhs);                  /* DeeFastSeq == SEQUENCE */
INTDEF WUNUSED NONNULL((1))    int DCALL DeeSeq_EqIV(DeeObject *lhs, DeeObject *const *rhsv, size_t rhsc);          /* ITERATOR == VECTOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EqIF(DeeObject *lhs, DeeObject *rhs, size_t rhsc);                  /* ITERATOR == DeeFastSeq */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EqII(DeeObject *lhs, DeeObject *rhs);                               /* ITERATOR == ITERATOR */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EqIS(DeeObject *lhs, DeeObject *rhs);                               /* ITERATOR == SEQUENCE */

/* Construct new concat-proxy-sequence objects. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Concat(DeeObject *self, DeeObject *other);

/* Construct new repetition-proxy-sequence objects. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Repeat(DeeObject *__restrict self, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatItem(DeeObject *__restrict item, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatItemForever(DeeObject *__restrict item);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_NonEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Front(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Back(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_Reduce(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Parity(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_Count(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Locate(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RLocate(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_LocateAll(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Transform(DeeObject *self, DeeObject *transformation);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_Contains(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_StartsWith(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EndsWith(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_Find(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_RFind(DeeObject *self, size_t start, size_t end, DeeObject *keyed_search_item, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Join(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Strip(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_LStrip(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RStrip(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Split(DeeObject *self, DeeObject *sep, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Reversed(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Filter(DeeObject *self, DeeObject *pred_keep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_HashFilter(DeeObject *self, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_HashFilter(DeeObject *self, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Sorted(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Segments(DeeObject *__restrict self, size_t segsize);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Combinations(DeeObject *__restrict self, size_t r);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatCombinations(DeeObject *__restrict self, size_t r);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Permutations(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Permutations2(DeeObject *__restrict self, size_t r);

/* Binary search for `keyed_search_item'
 * In case multiple elements match `keyed_search_item', the returned index
 * will be that for one of them, though it is undefined which one specifically.
 * @return: (size_t)-1: Not found.
 * @return: (size_t)-2: Error. */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_BFind(DeeObject *self, size_t start, size_t end,
             DeeObject *keyed_search_item, DeeObject *key);

/* Find the index-range of all items, such that:
 * >> for (elem: self[*p_startindex:*p_endindex])
 * >>     assert keyed_search_item == key(elem);
 * @return: 0: Success
 * @return: -1: Error. */
INTDEF WUNUSED NONNULL((1, 4, 6, 7)) int DCALL
DeeSeq_BFindRange(DeeObject *self, size_t start, size_t end,
                  DeeObject *keyed_search_item, DeeObject *key,
                  size_t *__restrict p_startindex,
                  size_t *__restrict p_endindex);

/* Same as `DeeSeq_BFind()', but return index where `keyed_search_item'
 * should go in case no matching item already exists in `self'
 * @return: (size_t)-1: Error. */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_BFindPosition(DeeObject *self, size_t start, size_t end,
                     DeeObject *keyed_search_item, DeeObject *key);

/* Returns `self[DeeSeq_BFind(self, keyed_search_item, key)]'
 * In case multiple elements match `keyed_search_item', the
 * returned item will be one of them, though which one is
 * undefined.
 * @return: NULL: Error, or not found (and `defl' is NULL). */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_BLocate(DeeObject *self, size_t start, size_t end,
               DeeObject *keyed_search_item, DeeObject *key,
               DeeObject *defl);

/* Sequence functions. */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_CountSeq(DeeObject *self, DeeObject *seq, DeeObject *key); /* @return: -1: Error. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_ContainsSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Partition(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RPartition(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_PartitionSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RPartitionSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_StartsWithSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EndsWithSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_FindSeq(DeeObject *self, DeeObject *seq, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_RFindSeq(DeeObject *self, DeeObject *seq, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_StripSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_LStripSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RStripSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_SplitSeq(DeeObject *self, DeeObject *sep_seq, DeeObject *key);

/* Vector-sorting functions. */
INTDEF WUNUSED int DCALL
DeeSeq_MergeSort(DREF DeeObject **__restrict dst,
                 DREF DeeObject *const *__restrict src,
                 size_t objc, DeeObject *key);
INTDEF int DCALL
DeeSeq_InsertionSort(DREF DeeObject **__restrict dst,
                     DREF DeeObject *const *__restrict src,
                     size_t objc, DeeObject *key);

/* Mutable-set operators. */

/* @return:  1: New item inserted.
 * @return:  0: Pre-existing item was not inserted.
 * @return: -1: Error. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_Insert(DeeObject *self, DeeObject *item);

/* @return:  1: Pre-existing item was removed.
 * @return:  0: No pre-existing item found.
 * @return: -1: Error. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_Remove(DeeObject *self, DeeObject *item);

/* @return: * : Number of inserted/removed items (that didn't already exist / weren't apart of @self)
 * @return: (size_t)-1: Error. */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSet_InsertAll(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSet_RemoveAll(DeeObject *self, DeeObject *items);


/* Return the sequence associated with the iterator, or NULL on error.
 * NOTE: Alternatively, a getset/member `seq' may be defined for this. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeIterator_GetSeq(DeeObject *__restrict self);

/* Get the iterator's position
 * @return: * :         The iterator's current position, where the a starting position is 0
 * @return: (size_t)-2: The position is indeterminate (the iterator may have become detached
 *                      from its sequence, as can happen in linked lists when the iterator's
 *                      link entry gets removed)
 * @return: (size_t)-1: Error */
INTDEF WUNUSED NONNULL((1)) size_t DCALL
DeeIterator_GetIndex(DeeObject *__restrict self);

/* Set the iterator's position
 * If the given `new_index' is greater than the max allowed index,
 * the iterator is set to an exhausted state (i.e. points at the
 * end of the associated sequence)
 * @return:  0: Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_SetIndex(DeeObject *__restrict self, size_t new_index);

/* Rewind the iterator to its starting position
 * @return:  0: Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Rewind(DeeObject *__restrict self);

/* Revert the iterator by at most `step' (When `step' is too large, same as `rewind')
 * @return:  0: Success (new relative position wasn't determined)
 * @return:  1: Success (the iterator has reached its starting position)
 * @return:  2: Success (the iterator hasn't reached its starting position)
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Revert(DeeObject *__restrict self, size_t step);

/* Advance the iterator by at most `step' (When `step' is too large, exhaust the iterator)
 * @return:  0: Success (new relative position wasn't determined)
 * @return:  1: Success (the iterator has become exhausted)
 * @return:  2: Success (the iterator hasn't become exhausted)
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Advance(DeeObject *__restrict self, size_t step);

/* Decrement the iterator by 1.
 * @return:  0: Success
 * @return:  1: The iterator was already at its starting location
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Prev(DeeObject *__restrict self);

/* Increment the iterator, but don't generate a value
 * NOTE: Unlike `tp_iter_next()', this operator shouldn't skip unbound entries,
 *       meaning that (also unlike `tp_iter_next()'), the iterator's index should
 *       only ever be incremented by 1.
 * @return:  0: Success
 * @return:  1: The iterator had already been exhausted
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Next(DeeObject *__restrict self);

/* Check if the iterator is at its starting location
 * @return:  0: No, it isn't
 * @return:  1: Yes, it is
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_HasPrev(DeeObject *__restrict self);

/* Peek the next iterator value, but don't actually advance the iterator.
 * @return: ITER_DONE: The iterator has already been exhausted. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeIterator_Peek(DeeObject *__restrict self);

#endif /* CONFIG_BUILDING_DEEMON */


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



/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedVector_Decref()' is called.
 * NOTE: This function implicitly inherits a reference to each item
 *       of the given vector, though does not actually inherit the
 *       vector itself!
 * NOTE: The returned object cannot be used to change out the elements
 *       of the given `vector', meaning that _it_ can still be [const] */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeSharedVector_NewShared(size_t length, DREF DeeObject *const *vector);

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



#undef si_key
#undef si_value
typedef struct {
	DREF DeeObject  *si_key;    /* [1..1][const] The key of this shared item. */
	DREF DeeObject  *si_value;  /* [1..1][const] The value of this shared item. */
} DeeSharedItem;

/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedMap_Decref()' is called.
 * NOTE: This function implicitly inherits a reference to each item
 *       of the given vector, though does not actually inherit the
 *       vector itself! */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeSharedMap_NewShared(size_t length, DREF DeeSharedItem *vector);

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `skv_vector',
 * but still decref() all contained object.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sskv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_MAP' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedMap_Decref(DREF DeeObject *__restrict self);






/* Check if `self' is a fast-sequence object, and return its (current)
 * length if it is, or return `DEE_FASTSEQ_NOTFAST' if it isn't.
 * A fast-sequence object is a vector-based object implemented by the
 * deemon C-core, meaning that its size can quickly be determined,
 * and items can quickly be accessed, given their index.
 * The following types function as fast-sequence-compatible:
 *  - Tuple
 *  - List
 *  - _SharedVector      (Created by a `ASM_CALL_SEQ' instruction -- `call top, {#X}')
 *  - _SeqSubRange       (Only if the sub-ranged sequence is a fast-sequence)
 *  - _SeqSubRangeN      (*ditto*)
 *  - _SeqTransformation (Only if the sequence being transformed is a fast-sequence)
 *  - _SeqIntRange
 *  - string
 *  - Bytes
 * Sub-classes of these types are not fast-sequence-compatible. */
DFUNDEF WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSize(DeeObject *__restrict self);
#define DEE_FASTSEQ_NOTFAST  ((size_t)-1)

/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize(self)' when
 * `self' is an immutable sequence (anything other than `List' and `_SharedVector').
 * WARNING: This function may _ONLY_ be used if `DeeFastSeq_GetSize(self)'
 *          returned something other than `DEE_FASTSEQ_NOTFAST'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItem(DeeObject *__restrict self, size_t index);

/* Same as `DeeFastSeq_GetItem()', but returns ITER_DONE if an error
 * occurred, and `NULL' if the item has been marked as unbound. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItemUnbound(DeeObject *__restrict self, size_t index);

/* An alternative (and more restrictive) variant of the FastSeq-interface:
 *  - Semantically, these functions are used the same way as the regular interface
 *  - Unlike the functions above, these are guarantied to be non-blocking
 *    -> However, an atomic lock doesn't count as something that would block,
 *       yet because this means that `DeeFastSeq_GetItemNB()' can never throw
 *       an exception, it also means that any sequence who's size could change
 *       at any time (such as `List') cannot be used here.
 * The following types function as fast-sequence-compatible-nb:
 *  - Tuple
 *  - _SharedVector   (If the sequence is cleared while being used here, `none' will be returned)
 *  - _SeqSubRange    (Only if the sub-ranged sequence is a fast-sequence-nb) */
DFUNDEF WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSizeNB(DeeObject *__restrict self);
DFUNDEF ATTR_RETNONNULL DREF DeeObject *DCALL
DeeFastSeq_GetItemNB(DeeObject *__restrict self, size_t index);


/* Allocate a suitable heap-vector for all the elements of a given sequence,
 * before returning that vector (then populated by [1..1] references), which
 * the caller must inherit upon success.
 * @return: * :   A vector of objects (with a length of `*plength'),
 *                that must be freed using `Dee_Free', before inheriting
 *                a reference to each of its elements.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    size_t *__restrict plength);

DFUNDEF WUNUSED NONNULL((1, 2, 3)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc(DeeObject *__restrict self,
                             size_t *__restrict plength,
                             size_t *__restrict pallocated);

/* Same as `DeeSeq_AsHeapVectorWithAlloc()', however also inherit
 * a pre-allocated heap-vector `*pvector' with an allocated size
 * of `IN(*pallocated) * sizeof(DeeObject *)', which is updated
 * as more memory needs to be allocated.
 * NOTE: `*pvector' may be updated to point to a new vector, even
 *       when the function fails (i.e. (size_t)-1 is returned)
 * @param: pvector:     A pointer to a preallocated object-vector `[0..IN(*pallocated)]'
 *                      May only point to a `NULL' vector when `IN(*pallocated)' is ZERO(0).
 *                      Upon return, this pointer may have been updated to point to a
 *                      realloc()-ated vector, should the need to allocate more memory
 *                      have arisen.
 * @param: pallocated:  A pointer to an information field describing how much pointers
 *                      are allocated upon entry / how much are allocated upon exit.
 *                      Just as `pvector', this pointer may be updated, even upon error.
 * @return: * :         The amount of filled in objects in `*pvector'
 * @return: (size_t)-1: An error occurred. Note that both `*pvector' and `*pallocated'
 *                      may have been modified since entry, with their original values
 *                      no longer being valid! */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out, owned(Dee_Free)*/ DeeObject ***__restrict pvector,
                                  /*in-out*/ size_t *__restrict pallocated);

/* Same as `DeeSeq_AsHeapVectorWithAllocReuse()', but assume
 * that `IN(*pallocated) >= offset', while also leaving the first
 * `offset' vector entries untouched and inserting the first enumerated
 * sequence element at `(*pvector)[offset]', rather than `(*pvector)[0]'
 * -> This function can be used to efficiently append elements to a
 *    vector which may already contain other objects upon entry. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out, owned(Dee_Free)*/ DeeObject ***__restrict pvector,
                                        /*in-out*/ size_t *__restrict pallocated,
                                        /*in*/ size_t offset);

/* Same as `DeeObject_Unpack()', but handle `DeeError_UnboundItem'
 * by filling in the resp. element from `objv[*]' with `NULL'.
 * This function is implemented to try the following things with `self' (in order):
 *  - Use `DeeFastSeq_GetSize()' + `DeeFastSeq_GetItemUnbound()'
 *    Try next when `DeeFastSeq_GetSize() == DEE_FASTSEQ_NOTFAST'
 *  - Use `DeeObject_Size()' + `DeeObject_GetItemIndex()'
 *    Try next when `DeeObject_Size()' throws `DeeError_NotImplemented', or
 *    `DeeObject_GetItemIndex()' (first call only) throws `DeeError_NotImplemented'
 *    or `DeeError_TypeError'.
 *  - Use `DeeObject_Unpack()' (meaning that all elements written to `objv' will be non-NULL)
 * @return: 0 : Success
 * @return: -1: Error */
DFUNDEF WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_UnpackWithUnbound)(DeeObject *__restrict self, size_t objc,
                                    /*out*/ DREF DeeObject **__restrict objv);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeObject_UnpackWithUnbound(self, objc, objv) __builtin_expect(DeeObject_UnpackWithUnbound(self, objc, objv), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_SEQ_H */
