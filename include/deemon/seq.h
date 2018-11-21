/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SEQ_H
#define GUARD_DEEMON_SEQ_H 1

#include "api.h"
#include "object.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

DECL_BEGIN

/* NOTE: These are no `DeeSeq_Check()' macros because they wouldn't make sense.
 *       Being derived from `DeeSeq_Type' is _NOT_ mandatory when writing a
 *       sequence class. The only thing that it does do is allow userspace
 *       to safely query whether or not an object implement all of the standard
 *       sequence functions.
 * Instead, a sequence object `ob' should be
 * detected using `DeeType_IsSequence(Dee_TYPE(ob))'.
 * The following things are required from sub-class of `sequence':
 *     - Must either implement `tp_iter_self' or `tp_size' + `tp_get'
 * The following things are implemented by `sequence':
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
 *        - `length'  (Read-only; same as `tp_size')
 *     - Abstraction that automatically defines the following class getsets:
 *        - `iterator'
 *           Evaluates to the internally used iterator type when `DeeObject_IterSelf()' would return it.
 *           Otherwise accessing this field raises an `Error.AttributeError'.
 *           The intention here is that a sub-class defining its own iterator
 *           should override this field in order to return its own type.
 *     - Abstraction that automatically defines the following methods:
 *        - `empty() -> bool'
 *        - `non_empty() -> bool'
 *        - `front() -> object'
 *        - `back() -> object'
 *        - `filter(callable func) -> sequence'
 *           - Same as `(for (local x: this) if (func(x)) x)'
 *        - `reduce(callable combine) -> object'
 *           - Call `combine()' on two consecutive objects, reusing the result as
 *             the first argument in the next call; return the final sum-style value.
 *             NOTE: When the sequence is empty, return `none'
 *        - `sum() -> object'
 *           - Same as `reduce([](a,b) -> a+b);'
 *           - Preferred way to concat sequences containing strings:
 *              - `print ["foo","bar","foobar"].sum(); // "foobarfoobar"'
 *        - `any() -> bool'
 *           - Same as `reduce([](a,b) -> a || b);', but stop on the first `true' and return `false' when empty.
 *        - `all() -> bool'
 *           - Same as `reduce([](a,b) -> a && b);', but stop on the first `false' and return `true' when empty.
 *        - `non() -> bool'
 *           - Returns true if all elements of the sequence equate to `false' and return `true' when empty.
 *        - `parity() -> object'
 *           - Same as `reduce([](a,b) -> !!a ^ !!b);'
 *        - `min(callable key = none) -> object'
 *           - Same as `reduce([](a,b) -> key(a,b) ? a : b);'
 *        - `max(callable key = none) -> object'
 *           - Same as `reduce([](a,b) -> key(a,b) ? b : a);'
 *        - `count(object ob, callable key = none) -> int'
 *        - `locate(object ob, callable key = none) -> object'
 *        - `rlocate(object ob, callable key = none) -> object'
 *        - `locateall(object ob, callable key = none) -> sequence'
 *        - `transform(callable transformation) -> sequence'
 *           - Invoke `transformation()' on all items and return a sequence of all the results.
 *           - Same as `(for (local x: this) transformation(x));'
 *        - `contains(object ob, callable key = none) -> bool'
 *           - Same as the `tp_contains' operator, but allows for a key function to be used.
 *        - `partition(object ob, callable key = none) -> (sequence,(ob),sequence)'
 *        - `rpartition(object ob, callable key = none) -> (sequence,(ob),sequence)'
 *        - `startswith(object ob, callable key = none) -> bool'
 *        - `endswith(object ob, callable key = none) -> bool'
 *        - `find(object ob, callable key = none) -> int'
 *        - `rfind(object ob, callable key = none) -> int'
 *        - `index(object ob, callable key = none) -> int'
 *        - `rindex(object ob, callable key = none) -> int'
 *        - `join(sequence items) -> sequence'
 *        - `strip(object ob, callable key = none) -> sequence'
 *        - `lstrip(object ob, callable key = none) -> sequence'
 *        - `rstrip(object ob, callable key = none) -> sequence'
 *        - `split(object sep, callable key = none) -> sequence'
 *        - `reversed() -> sequence'
 *        - `sorted(callable key = none) -> sequence'
 *        - `segments(size_t segsize) -> sequence'
 *        - `countseq(sequence seq, callable key = none) -> int'
 *        - `containsseq(sequence seq, callable key = none) -> bool'
 *        - `partitionseq(sequence seq, callable key = none) -> (sequence,seq,sequence)'
 *        - `rpartitionseq(sequence seq, callable key = none) -> (sequence,seq,sequence)'
 *        - `startswithseq(sequence seq, callable key = none) -> bool'
 *        - `endswithseq(sequence seq, callable key = none) -> bool'
 *        - `findseq(sequence seq, callable key = none) -> int'
 *        - `rfindseq(sequence seq, callable key = none) -> int'
 *        - `indexseq(sequence seq, callable key = none) -> int'
 *        - `rindexseq(sequence seq, callable key = none) -> int'
 *        - `stripseq(sequence items, callable key = none) -> sequence'
 *        - `lstripseq(sequence items, callable key = none) -> sequence'
 *        - `rstripseq(sequence items, callable key = none) -> sequence'
 *        - `splitseq(sequence sep_seq, callable key = none) -> sequence'
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
 * >> local a = [5,10,20,30,5];
 * >> local b = a.strip(5);
 * >> for (local x: b) print x; // 10 20 30
 * >> print type b; // Undefined and subject to change
 * >> b = [b...]; // Explicit cast to list; same as `b = list(b);'
 * HINT: Instantiating `seq' as-is will yield `Dee_EmptySeq',
 *       which in return is re-used internally as a placeholder
 *       to represent an empty, general-purpose sequence. */
DDATDEF DeeTypeObject DeeSeq_Type;  /* `sequence from deemon' */

/* Similar to what `DeeSeq_Type' is for all sequence-style types,
 * `DeeIterator_Type' is the the same for all iterator-type types.
 * The following things are implemented by `iterator':
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
 *             When the second operand is negative, throw an `Error.ValueError.Arithmetic.IntegerOverflow'
 *     - Abstraction that automatically defines the following methods:
 *        - `next() -> object'
 *           - Literally the same as invoking `operator __next__()', but has a more userfriendly name.
 *             This function will throw a `Signal.StopIteration' when the iterator has been exhausted.
 * As should be apparent, in order to use the full auto-API provided for
 * iterators, any iterator sub-class should implement a copy-constructor
 * in addition to the `tp_iter_next()' operator, although it should be
 * noted that implementation of the copy-constructor is _NOT_ mandatory,
 * as in some cases it even is impossible to pull off (such as for yield-
 * functions not marked as copyable or using non-copyable local variables).
 */
DDATDEF DeeTypeObject DeeIterator_Type; /* `iterator from deemon' */

/* An empty instance of a generic sequence object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSeq_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSeq_Type' should be considered stub/empty.
 */
DDATDEF DeeObject      DeeSeq_EmptyInstance;
#define Dee_EmptySeq (&DeeSeq_EmptyInstance)
#define return_empty_seq  return_reference_(Dee_EmptySeq)




/* ==== NATIVE SEQUENCE INTERFACE EXTENSIONS FOR TYPES ==== */

struct type_nsi {
    /* Native Sequence Interface for types. */
#define TYPE_SEQX_CLASS_SEQ  0x0000 /* Sequence-like */
#define TYPE_SEQX_CLASS_MAP  0x0001 /* Mapping-like */
#define TYPE_SEQX_CLASS_SET  0x0002 /* Set-like */
#define TYPE_SEQX_FNORMAL    0x0000 /* Normal sequence flags. */
#define TYPE_SEQX_FMUTABLE   0x0001 /* The sequence is mutable. */
#define TYPE_SEQX_FRESIZABLE 0x0002 /* The sequence is resizable. */
#if __SIZEOF_POINTER__ == 4
    uint16_t                nsi_class; /* Sequence class (One of `TYPE_SEQX_CLASS_*') */
    uint16_t                nsi_flags; /* Sequence flags (Set of `TYPE_SEQX_F*') */
#elif __SIZEOF_POINTER__ == 8
    uint32_t                nsi_class; /* Sequence class (One of `TYPE_SEQX_CLASS_*') */
    uint32_t                nsi_flags; /* Sequence class (Set of `TYPE_SEQX_F*') */
#else
#error "Unsupported __SIZEOF_POINTER__"
#endif
    union {
        void              *_nsi_class_functions[22];
        struct {
            size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self); /* [1..1] ERROR: (size_t)-1 */
        }                   nsi_common;
        struct { /* TYPE_SEQX_CLASS_SEQ */
           /* NOTE: If provided, these functions are only ever called as extensions to the
            *       regular sequence operators, meaning that if you implement `nsi_getsize',
            *       you are also _required_ to implement `tp_size'
            * NOTE: Any object implementing sequence extensions _must_ at
            *       the very least provide the operators for `nsi_getsize'!
            * NOTE: The `*_fast' variants are allowed to assume:
            *      `index < ANY_PREVIOUS(nsi_getsize(ob)))' */
            size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self); /* [1..1] ERROR: (size_t)-1 */
            /* Same as `nsi_getsize', but never throw any errors, and simply return (size_t)-1 to indicate failure.
             * HINT: This callback is used to implement `DeeFastSeq_GetSize()', with
             *       either `nsi_getitem_fast()' or `nsi_getitem()' then being used
             *       to implement the item lookup itself.
             * WARNING: When implementing this operator, you must also implement at
             *          least one of `nsi_getitem' ro `nsi_getitem_fast' */
            size_t          (DCALL *nsi_getsize_fast)(DeeObject *__restrict self);
            DREF DeeObject *(DCALL *nsi_getitem)(DeeObject *__restrict self, size_t index);
            int             (DCALL *nsi_delitem)(DeeObject *__restrict self, size_t index);
            int             (DCALL *nsi_setitem)(DeeObject *__restrict self, size_t index, DeeObject *__restrict value);
            /* When `nsi_getitem_fast()' returns NULL, no error is thrown, and it means that the item is unbound. */
            DREF DeeObject *(DCALL *nsi_getitem_fast)(DeeObject *__restrict self, size_t index);
            DREF DeeObject *(DCALL *nsi_getrange)(DeeObject *__restrict self, dssize_t start, dssize_t end);
            DREF DeeObject *(DCALL *nsi_getrange_n)(DeeObject *__restrict self, dssize_t start); /* end: Dee_None */
            int             (DCALL *nsi_setrange)(DeeObject *__restrict self, dssize_t start, dssize_t end, DeeObject *__restrict values);
            int             (DCALL *nsi_setrange_n)(DeeObject *__restrict self, dssize_t start, DeeObject *__restrict values); /* end: Dee_None */
            /* NOTE: start/end in here operate differently (and simpler) than in ranges:
             *       If either value is `>= nsi_getsize()', truncate it to that length.
             * NOTE: Comparisons should be performed as `keyed_search_item == key(this[?])'
             * @return: * : Index of the matching item
             * @return: (size_t)-1: Index not found.
             * @return: (size_t)-2: Error. */
            size_t          (DCALL *nsi_find)(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key);
            size_t          (DCALL *nsi_rfind)(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key);
            DREF DeeObject *(DCALL *nsi_xch)(DeeObject *__restrict self, size_t index, DeeObject *__restrict value);
            int             (DCALL *nsi_insert)(DeeObject *__restrict self, size_t index, DeeObject *__restrict value);
            int             (DCALL *nsi_insertall)(DeeObject *__restrict self, size_t index, DeeObject *__restrict values);
            int             (DCALL *nsi_insertvec)(DeeObject *__restrict self, size_t index, size_t insertc, DeeObject **__restrict insertv);
            /* NOTE: When `index' is lower than ZERO(0), the length of the sequence `self' must be added
             *       first, such that `nsi_pop(self,-1)' is equivalent to a `popback()' function call. */
            DREF DeeObject *(DCALL *nsi_pop)(DeeObject *__restrict self, dssize_t index);
            /* NOTE: erase differs from delrange, in that erase _always_ removes the indices,
             *       while delrange is allowed to leave the index range as unbound.
             * @return: * : Number or erased items.
             * @return: (size_t)-1: Error. */
            size_t          (DCALL *nsi_erase)(DeeObject *__restrict self, size_t index, size_t count);
            /* Remove or unbind the first/last/all instance(s) of `elem'
             * NOTE: Comparisons should be performed as `keyed_search_item == key(this[?])'
             * @return: 0 : Element not found.
             * @return: 1 : Element was unbound.
             * @return: -1 : error. */
            int             (DCALL *nsi_remove)(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key);
            int             (DCALL *nsi_rremove)(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key);
            /* NOTE: Comparisons should be performed as `keyed_search_item == key(this[?])'
             * @return: * : The number of removed items.
             * @return: (size_t)-1: Error. */
            size_t          (DCALL *nsi_removeall)(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key);
            size_t          (DCALL *nsi_removeif)(DeeObject *__restrict self, DeeObject *__restrict should, size_t start, size_t end);
        }                   nsi_seqlike;
        struct { /* TYPE_SEQX_CLASS_MAP */
            size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self); /* [1..1] ERROR: (size_t)-1 */
            /* Same as `mapping.iterator.operator next()' of the mapping's core iterator,
             * however only return the key / value, rather than a key-value tuple.
             * @param: iterator: An iterator object, as returned by `mapping.operator iter()' */
            DREF DeeObject *(DCALL *nsi_nextkey)(DeeObject *__restrict iterator);
            DREF DeeObject *(DCALL *nsi_nextvalue)(DeeObject *__restrict iterator);
            /* Lookup the given `key' and return its association, or `defl' if it doesn't yet exist.
             * WARNING: `defl' may be ITER_DONE, in which case you really shouldn't incref() it! */
            DREF DeeObject *(DCALL *nsi_getdefault)(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict defl);
            /* Check if the mapping contains a element for `key' and return that element's value, or
             * insert a new element for `key', setting its value to `defl', then returning `defl'. */
            DREF DeeObject *(DCALL *nsi_setdefault)(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict defl);
            /* Update an existing mapping element
             * @param: poldvalue: When non-NULL, store a reference to the old item here.
             * @return: 1:  The existing key was updated.
             * @return: 0: `key' doesn't exist. (*poldvalue is left unchanged)
             * @return: -1: Error. */
            int             (DCALL *nsi_updateold)(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict value, DREF DeeObject **poldvalue);
            /* Insert a new mapping element, but don't change a pre-existing one
             * @param: poldvalue: When non-NULL, store a reference to the old item here.
             * @return: 1:  A new element was inserted.
             * @return: 0: `key' doesn't exist. (*poldvalue is left unchanged)
             * @return: -1: Error. */
            int             (DCALL *nsi_insertnew)(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict value, DREF DeeObject **poldvalue);
        }                   nsi_maplike;
        struct { /* TYPE_SEQX_CLASS_SET */
            size_t          (DCALL *nsi_getsize)(DeeObject *__restrict self); /* [1..1] ERROR: (size_t)-1 */
            /* Insert a new `key' into the set
             * @return: 1:  The given `key' was inserted.
             * @return: 0:  A identical key was already apart of the set.
             * @return: -1: Error. */
            int             (DCALL *nsi_insert)(DeeObject *__restrict self, DeeObject *__restrict key);
            /* Remove a given `key' from the set
             * @return: 1:  The given `key' was removed.
             * @return: 0:  The given `key' could not be found within the set.
             * @return: -1: Error. */
            int             (DCALL *nsi_remove)(DeeObject *__restrict self, DeeObject *__restrict key);
        }                   nsi_setlike;
    };
};



/* Lookup the closes NSI descriptor for `tp', or return `NULL'
 * if the top-most type implementing any sequence operator doesn't
 * expose NSI functionality. */
DFUNDEF struct type_nsi *DCALL DeeType_NSI(DeeTypeObject *__restrict tp);

/* Create new range sequence objects. */
DFUNDEF DREF DeeObject *DCALL DeeRange_New(DeeObject *__restrict begin, DeeObject *__restrict end, DeeObject *step);
DFUNDEF DREF DeeObject *DCALL DeeRange_NewInt(dssize_t begin, dssize_t end, dssize_t step);

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
DFUNDEF DREF DeeObject *DCALL DeeSeq_Sum(DeeObject *__restrict self);
DFUNDEF int DCALL DeeSeq_Any(DeeObject *__restrict self);
DFUNDEF int DCALL DeeSeq_All(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeSeq_Min(DeeObject *__restrict self, DeeObject *key);
DFUNDEF DREF DeeObject *DCALL DeeSeq_Max(DeeObject *__restrict self, DeeObject *key);

#ifdef CONFIG_BUILDING_DEEMON
/* Mutable-sequence API */
INTDEF int DCALL DeeSeq_DelItem(DeeObject *__restrict self, size_t index);
INTDEF int DCALL DeeSeq_SetItem(DeeObject *__restrict self, size_t index, DeeObject *__restrict value);
INTDEF DREF DeeObject *DCALL DeeSeq_XchItem(DeeObject *__restrict self, size_t index, DeeObject *__restrict value);
INTDEF int DCALL DeeSeq_DelRange(DeeObject *__restrict self, size_t start, size_t end);
INTDEF int DCALL DeeSeq_SetRange(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict values);
INTDEF int DCALL DeeSeq_DelRangeN(DeeObject *__restrict self, size_t start);
INTDEF int DCALL DeeSeq_SetRangeN(DeeObject *__restrict self, size_t start, DeeObject *__restrict values);
INTDEF int DCALL DeeSeq_Insert(DeeObject *__restrict self, size_t index, DeeObject *__restrict value);
INTDEF int DCALL DeeSeq_InsertAll(DeeObject *__restrict self, size_t index, DeeObject *__restrict values);
INTDEF int DCALL DeeSeq_Append(DeeObject *__restrict self, DeeObject *__restrict value);
INTDEF int DCALL DeeSeq_Extend(DeeObject *__restrict self, DeeObject *__restrict values);
INTDEF int DCALL DeeSeq_InplaceExtend(DREF DeeObject **__restrict pself, DeeObject *__restrict values);
INTDEF int DCALL DeeSeq_InplaceRepeat(DREF DeeObject **__restrict pself, DeeObject *__restrict count);
INTDEF size_t DCALL DeeSeq_Erase(DeeObject *__restrict self, size_t index, size_t count);
INTDEF DREF DeeObject *DCALL DeeSeq_PopItem(DeeObject *__restrict self, dssize_t index);
INTDEF int DCALL DeeSeq_Remove(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict elem, DeeObject *key);
INTDEF int DCALL DeeSeq_RRemove(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict elem, DeeObject *key);
INTDEF size_t DCALL DeeSeq_RemoveAll(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict elem, DeeObject *key);
INTDEF size_t DCALL DeeSeq_RemoveIf(DeeObject *__restrict self, DeeObject *__restrict should, size_t start, size_t end);
INTDEF size_t DCALL DeeSeq_Fill(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict value);
INTDEF int DCALL DeeSeq_Reverse(DeeObject *__restrict self);
INTDEF int DCALL DeeSeq_Sort(DeeObject *__restrict self, DeeObject *key);

/* Determine if a given sequence is mutable or resizable.
 * @return: 1:  The sequence is mutable or resizable.
 * @return: 0:  The sequence isn't mutable or resizable.
 * @return: -1: An error occurred. */
INTDEF int DCALL DeeSeq_IsMutable(DeeObject *__restrict self);
INTDEF int DCALL DeeSeq_IsResizable(DeeObject *__restrict self);

/* NOTE: Technically, all of these functions can be used on any type of object,
 *       but all objects derived from `DeeSeq_Type' automatically implement
 *       all of them as member functions.
 *       With that in mind, any type implementing the `tp_seq' interface
 *       with the intention of behaving as an iterable, should probably
 *       be derived from `DeeSeq_Type' as this allows userspace to query
 *       for a general purpose sequence by writing `x is sequence from deemon' */
INTDEF size_t DCALL DeeSeq_Size(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSeq_GetItem(DeeObject *__restrict self, size_t index);
INTDEF DREF DeeObject *DCALL DeeSeq_GetRange(DeeObject *__restrict self, size_t begin, size_t end);
INTDEF DREF DeeObject *DCALL DeeSeq_GetRangeN(DeeObject *__restrict self, size_t begin);

/* General-purpose iterator/sequence compare functions.
 * NOTE: The iterator-compare functions compare the _ELEMENTS_
 *       they yield, not the abstract iterator positions. */
INTDEF int DCALL DeeIter_Eq(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL DeeIter_Lo(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL DeeIter_Le(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL DeeSeq_Eq(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL DeeSeq_Lo(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL DeeSeq_Le(DeeObject *__restrict self, DeeObject *__restrict other);

INTDEF int DCALL DeeSeq_EqVV(DeeObject **__restrict lhsv, DeeObject **__restrict rhsv, size_t elemc);             /* VECTOR == VECTOR */
INTDEF int DCALL DeeSeq_EqVF(DeeObject **__restrict lhsv, DeeObject *__restrict rhs, size_t elemc);               /* VECTOR == DeeFastSeq */
INTDEF int DCALL DeeSeq_EqVI(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict rhs);                /* VECTOR == ITERATOR */
INTDEF int DCALL DeeSeq_EqVS(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict seq);                /* VECTOR == SEQUENCE */
INTDEF int DCALL DeeSeq_LoVV(DeeObject **__restrict lhsv, size_t lhsc, DeeObject **__restrict rhsv, size_t rhsc); /* VECTOR < VECTOR */
INTDEF int DCALL DeeSeq_LoVF(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict rhs, size_t rhsc);   /* VECTOR < DeeFastSeq */
INTDEF int DCALL DeeSeq_LoVI(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict rhs);                /* VECTOR < ITERATOR */
INTDEF int DCALL DeeSeq_LoVS(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict seq);                /* VECTOR < SEQUENCE */
INTDEF int DCALL DeeSeq_LeVV(DeeObject **__restrict lhsv, size_t lhsc, DeeObject **__restrict rhsv, size_t rhsc); /* VECTOR <= VECTOR */
INTDEF int DCALL DeeSeq_LeVF(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict rhs, size_t rhsc);   /* VECTOR <= DeeFastSeq */
INTDEF int DCALL DeeSeq_LeVI(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict rhs);                /* VECTOR <= ITERATOR */
INTDEF int DCALL DeeSeq_LeVS(DeeObject **__restrict lhsv, size_t lhsc, DeeObject *__restrict seq);                /* VECTOR <= SEQUENCE */

INTDEF int DCALL DeeSeq_EqIV(DeeObject *__restrict lhs, DeeObject **__restrict rhsv, size_t rhsc);  /* ITERATOR == VECTOR */
INTDEF int DCALL DeeSeq_EqIF(DeeObject *__restrict lhs, DeeObject *__restrict rhs, size_t rhsc);    /* ITERATOR == DeeFastSeq */
INTDEF int DCALL DeeSeq_LoIV(DeeObject *__restrict lhs, DeeObject **__restrict rhsv, size_t rhsc);  /* ITERATOR < VECTOR */
INTDEF int DCALL DeeSeq_LoIF(DeeObject *__restrict lhs, DeeObject *__restrict rhs, size_t rhsc);    /* ITERATOR < DeeFastSeq */
INTDEF int DCALL DeeSeq_LeIV(DeeObject *__restrict lhs, DeeObject **__restrict rhsv, size_t rhsc);  /* ITERATOR <= VECTOR */
INTDEF int DCALL DeeSeq_LeIF(DeeObject *__restrict lhs, DeeObject *__restrict rhs, size_t rhsc);    /* ITERATOR <= DeeFastSeq */

/* Construct new concat-proxy-sequence objects. */
INTDEF DREF DeeObject *DCALL DeeSeq_Concat(DeeObject *__restrict self, DeeObject *__restrict other);

/* Construct new repetition-proxy-sequence objects. */
INTDEF DREF DeeObject *DCALL DeeSeq_Repeat(DeeObject *__restrict self, size_t count);
INTDEF DREF DeeObject *DCALL DeeSeq_RepeatItem(DeeObject *__restrict item, size_t count);

INTDEF int DCALL DeeSeq_NonEmpty(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSeq_Front(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSeq_Back(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSeq_Reduce(DeeObject *__restrict self, DeeObject *__restrict combine, DeeObject *__restrict init);
INTDEF int DCALL DeeSeq_Parity(DeeObject *__restrict self);
INTDEF size_t DCALL DeeSeq_Count(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_Locate(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_RLocate(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_LocateAll(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_Transform(DeeObject *__restrict self, DeeObject *__restrict transformation);
INTDEF int DCALL DeeSeq_Contains(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF int DCALL DeeSeq_StartsWith(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF int DCALL DeeSeq_EndsWith(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF size_t DCALL DeeSeq_Find(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF size_t DCALL DeeSeq_RFind(DeeObject *__restrict self, size_t start, size_t end, DeeObject *__restrict keyed_search_item, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF DREF DeeObject *DCALL DeeSeq_Join(DeeObject *__restrict self, DeeObject *__restrict items);
INTDEF DREF DeeObject *DCALL DeeSeq_Strip(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_LStrip(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_RStrip(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_Split(DeeObject *__restrict self, DeeObject *__restrict sep, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_Reversed(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSeq_Filter(DeeObject *__restrict self, DeeObject *__restrict pred_keep);
INTDEF DREF DeeObject *DCALL DeeSeq_Sorted(DeeObject *__restrict self, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_Segments(DeeObject *__restrict self, size_t segsize);
INTDEF DREF DeeObject *DCALL DeeSeq_Combinations(DeeObject *__restrict self, size_t r);
INTDEF DREF DeeObject *DCALL DeeSeq_RepeatCombinations(DeeObject *__restrict self, size_t r);
INTDEF DREF DeeObject *DCALL DeeSeq_Permutations(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSeq_Permutations2(DeeObject *__restrict self, size_t r);

/* Sequence functions. */
INTDEF size_t DCALL DeeSeq_CountSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key); /* @return: -1: Error. */
INTDEF int DCALL DeeSeq_ContainsSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_Partition(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_RPartition(DeeObject *__restrict self, DeeObject *__restrict keyed_search_item, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_PartitionSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_RPartitionSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF int DCALL DeeSeq_StartsWithSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF int DCALL DeeSeq_EndsWithSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF size_t DCALL DeeSeq_FindSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF size_t DCALL DeeSeq_RFindSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF DREF DeeObject *DCALL DeeSeq_StripSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_LStripSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_RStripSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTDEF DREF DeeObject *DCALL DeeSeq_SplitSeq(DeeObject *__restrict self, DeeObject *__restrict sep_seq, DeeObject *key);

/* Vector-sorting functions. */
INTDEF int DCALL DeeSeq_MergeSort(DREF DeeObject **__restrict dst,
                                  DREF DeeObject *const *__restrict src,
                                  size_t objc, DeeObject *key);
INTDEF int DCALL DeeSeq_InsertionSort(DREF DeeObject **__restrict dst,
                                      DREF DeeObject *const *__restrict src,
                                      size_t objc, DeeObject *key);

/* Mutable-set operators. */

/* @return:  1: New item inserted.
 * @return:  0: Pre-existing item was not inserted.
 * @return: -1: Error. */
INTDEF int DCALL DeeSet_Insert(DeeObject *__restrict self, DeeObject *__restrict item);

/* @return:  1: Pre-existing item was removed.
 * @return:  0: No pre-existing item found.
 * @return: -1: Error. */
INTDEF int DCALL DeeSet_Remove(DeeObject *__restrict self, DeeObject *__restrict item);

/* @return: * : Number of inserted/removed items (that didn't already exist / weren't apart of @self)
 * @return: (size_t)-1: Error. */
INTDEF size_t DCALL DeeSet_InsertAll(DeeObject *__restrict self, DeeObject *__restrict items);
INTDEF size_t DCALL DeeSet_RemoveAll(DeeObject *__restrict self, DeeObject *__restrict items);

#endif /* CONFIG_BUILDING_DEEMON */


/* Construct a new reference-vector object that can be iterated
 * and be used to potentially modify the elements of a given `vector'.
 * NOTE: When write-access is granted, `vector' should be `[0..1][0..length]',
 *       whereas when write-access is not possible, then the disposition of
 *       elements of `vector' doesn't matter and can either be `[0..1]' or `[1..1]'. */
DFUNDEF DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *__restrict owner, size_t length,
                 DeeObject **__restrict vector,
#ifndef CONFIG_NO_THREADS
                 rwlock_t *plock
#else
                 bool writable
#endif
                 );

#ifdef __INTELLISENSE__
DFUNDEF DREF DeeObject *DCALL
DeeRefVector_NewReadonly(DeeObject *__restrict owner, size_t length,
                         DeeObject *const *__restrict vector);
#elif defined(CONFIG_NO_THREADS)
#define DeeRefVector_NewReadonly(owner,length,vector) DeeRefVector_New(owner,length,(DeeObject **)(vector),false)
#else
#define DeeRefVector_NewReadonly(owner,length,vector) DeeRefVector_New(owner,length,(DeeObject **)(vector),NULL)
#endif




/* Check if `self' is a fast-sequence object, and return its (current)
 * length if it is, or return `DEE_FASTSEQ_NOTFAST' if it isn't.
 * A fast-sequence object is a vector-based object implemented by the
 * deemon C-core, meaning that its size can quickly be determined,
 * and items can quickly be accessed, given their index.
 * The following types function as fast-sequence-compatible:
 *  - tuple
 *  - list
 *  - _sharedvector   (Created by a `ASM_CALL_SEQ' instruction -- `call top, {#X}')
 *  - _subrange       (Only if the sub-ranged sequence is a fast-sequence)
 *  - _transformation (Only if the sequence being transformed is a fast-sequence)
 *  - _intrange
 *  - string
 *  - bytes
 * Sub-classes of these types are not fast-sequence-compatible. */
DFUNDEF size_t DCALL DeeFastSeq_GetSize(DeeObject *__restrict self);
#define DEE_FASTSEQ_NOTFAST  ((size_t)-1)

/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize(self)' when
 * `self' is an immutable sequence (anything other than `list' and `_sharedvector').
 * WARNING: This function may _ONLY_ be used if `DeeFastSeq_GetSize(self)'
 *          returned something other than `DEE_FASTSEQ_NOTFAST'. */
DFUNDEF DREF DeeObject *DCALL DeeFastSeq_GetItem(DeeObject *__restrict self, size_t index);

/* An alternative (and more restrictive) variant of the FastSeq-interface:
 *  - Semantically, these functions are used the same way as the regular interface
 *  - Unlike the functions above, these are guarantied to be non-blocking
 *    -> However, an atomic lock doesn't count as something that would block,
 *       yet because this means that `DeeFastSeq_GetItemNB()' can never throw
 *       an exception, it also means that any sequence who's size could change
 *       at any time (such as `list') cannot be used here.
 * The following types function as fast-sequence-compatible-nb:
 *  - tuple
 *  - _sharedvector   (If the sequence is cleared while being used here, `none' will be returned)
 *  - _subrange       (Only if the sub-ranged sequence is a fast-sequence-nb) */
DFUNDEF size_t DCALL DeeFastSeq_GetSizeNB(DeeObject *__restrict self);
DFUNDEF ATTR_RETNONNULL DREF DeeObject *DCALL DeeFastSeq_GetItemNB(DeeObject *__restrict self, size_t index);


/* Allocate a suitable heap-vector for all the elements of a given sequence,
 * before returning that vector (then populated by [1..1] references), which
 * the caller must inherit upon success.
 * @return: * :   A vector of objects (with a length of `*plength'),
 *                that must be freed using `Dee_Free', before inheriting
 *                a reference to each of its elements.
 * @return: NULL: An error occurred. */
DFUNDEF /*owned(Dee_Free)*/DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    size_t *__restrict plength);

DFUNDEF /*owned(Dee_Free)*/DREF DeeObject **DCALL
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
DFUNDEF size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out,owned(Dee_Free)*/DeeObject ***__restrict pvector,
                                  /*in-out*/size_t *__restrict pallocated);

/* Same as `DeeSeq_AsHeapVectorWithAllocReuse()', but assume
 * that `IN(*pallocated) >= offset', while also leaving the first
 * `offset' vector entries untouched and inserting the first enumerated
 * sequence element at `(*pvector)[offset]', rather than `(*pvector)[0]'
 * -> This function can be used to efficiently append elements to a
 *    vector which may already contain other objects upon entry. */
DFUNDEF size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out,owned(Dee_Free)*/DeeObject ***__restrict pvector,
                                        /*in-out*/size_t *__restrict pallocated,
                                        /*in*/size_t offset);


#ifdef GUARD_DEEMON_OBJMETHOD_H
#ifdef CONFIG_BUILDING_DEEMON
#define DeeSeq_KeyIsID(key) ((DeeObject *)REQUIRES_OBJECT(key) == (DeeObject *)&_DeeObject_IdObjMethod)
INTDEF DeeClsMethodObject _DeeObject_IdObjMethod;
#else /* CONFIG_BUILDING_DEEMON */
#define DeeSeq_KeyIsID(key) ((key) && DeeClsMethod_Check(key) && DeeClsMethod_FUNC(key) == &_DeeObject_IdFunc)
DFUNDEF DREF DeeObject *DCALL
_DeeObject_IdFunc(DeeObject *__restrict self, size_t argc,
                  DeeObject **__restrict argv);
#endif /* !CONFIG_BUILDING_DEEMON */
#endif /* GUARD_DEEMON_OBJMETHOD_H */


DECL_END

#endif /* !GUARD_DEEMON_SEQ_H */
