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
#ifndef GUARD_DEEMON_SET_H
#define GUARD_DEEMON_SET_H 1

#include "api.h"
#include "object.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

DECL_BEGIN

/* Base class for set-style sequence types (e.g. `hashset')
 * :: Characteristics of a set type::
 *   - class member iterator -> iterator;
 *   - operator iter():
 *     sequence item through iteration yields another
 *     sequence consisting of 2 elements: `key' = `value'
 *     This sub-sequence is usually often implemented as a tuple,
 *     and code should be optimized for that code, however this
 *     is not a requirement.
 *   - operator contains(object ob) -> bool:
 *     Returns :true if @ob is apart of @this set
 *   - The getitem operator is not implemented.
 *   - The getrange operator is not implemented.
 *
 * Using `set from deemon' (aka. `DeeSet_Type') as a base class, it will
 * automatically provide for the following member functions and operators:
 *
 * difference(set other) -> set;
 * operator - (set other) -> set;
 *     Returns a set of all objects from @this, excluding those also found in @other
 * 
 * intersection(set other) -> set;
 * operator & (set other) -> set;
 *     Returns the intersection of @this and @other
 * 
 * isdisjoint(set other) -> bool;
 *     Returns :true if ${#(this & other) == 0}
 *     In other words: If @this and @other have no items in common.
 *
 * union(set other) -> set;
 * operator | (set other) -> set;
 * operator + (set other) -> set;
 *     Returns the union of @this and @other
 * 
 * symmetric_difference(set other) -> set;
 * operator ^ (set other) -> set;
 *     Returns a set containing objects only found in either
 *     @this or @other, but not those found in both.
 * 
 * issubset(set other) -> bool;
 * operator <= (set other) -> bool;
 *     Returns :true if all items found in @this set can also be found in @other
 * 
 * operator == (set other) -> bool;
 *     Returns :true if @this set contains the same
 *     items as @other, and not any more than that
 * 
 * operator < (set other) -> bool;
 *     The result of ${this <= other && this != other}
 *
 * issuperset(set other) -> bool;
 * operator >= (set other) -> bool;
 *     Returns :true if all items found in @other can also be found in @this set
 *
 * operator ~ () -> set;
 *     Returns a symbolic set that behaves as though it contained
 *     any feasible object that isn't already apart of `this' set.
 *     Note however that due to the impossibility of such a set,
 *     you cannot iterate its elements, and the only ~real~ operator
 *     implemented by it is `operator contains'.
 *     Its main purpose is for being used in conjunction with
 *     `operator &' in order to create a sub-set that doesn't
 *     contain a certain set of sub-elements:
 *     >> local items = hashset({ 10, 11, 15, 20, 30 });
 *     >> print repr(items & ~set({ 11, 15 }))
 *
 * NOTE: `DeeSet_Type' itself is derived from `sequence from deemon' (aka. `DeeSeq_Type')
 * NOTE: Because `DeeSet_Type' inherits from `DeeSeq_Type', all member functions that
 *       it provides, as well as its operators (such as `bool', compare, etc.), are
 *       implicitly inherited, and also provided by objects derived from `DeeSet_Type',
 *       and `mapping from deemon' itself.
 *       This also means that sub-classes of `mapping from deemon' should respect the
 *      `iterator' interface, provided a `class member iterator -> type' which represents
 *       the iterator type used by the mapping. */
DDATDEF DeeTypeObject DeeSet_Type;  /* `set from deemon' */

/* An empty instance of a generic set object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSet_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSet_Type' should be considered stub/empty. */
DDATDEF DeeObject      DeeSet_EmptyInstance;
#define Dee_EmptySet (&DeeSet_EmptyInstance)

/* Check for a symbolic, empty set.
 * NOTE: This function isn't guarantied to capture any kind of empty set,
 *       only sets that are meant to symbolically represent any empty one. */
#define DeeSet_CheckEmpty(x) DeeObject_InstanceOfExact(x,&DeeSet_Type)


#ifdef CONFIG_BUILDING_DEEMON
INTDEF DREF DeeObject *DCALL DeeSet_Invert(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeSet_Union(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF DREF DeeObject *DCALL DeeSet_Difference(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF DREF DeeObject *DCALL DeeSet_Intersection(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF DREF DeeObject *DCALL DeeSet_SymmetricDifference(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF int DCALL DeeSet_IsSubSet(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF int DCALL DeeSet_IsTrueSubSet(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF int DCALL DeeSet_IsSameSet(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
INTDEF int DCALL DeeSet_IsDisjoint(DeeObject *__restrict lhs, DeeObject *__restrict rhs);
#endif


#ifdef DEE_SOURCE
#define Dee_inverse_set_object inverse_set_object
#endif /* DEE_SOURCE */

typedef struct Dee_inverse_set_object DeeInverseSetObject;
struct Dee_inverse_set_object {
    /* An inverse set, that is the symbolic set containing all
     * object, excluding those already contained within `is_set'
     * Since such a set cannot be iterated, working with it
     * requires some special operations, as well as special
     * support in some places, which is why it is exposed here.
     * In user-code, such a set is created through use of `operator ~()' */
    Dee_OBJECT_HEAD
    DREF DeeObject *is_set; /* [1..1][const] The underlying set. */
};
#define DeeInverseSet_SET(ob) (((DeeInverseSetObject *)Dee_REQUIRES_OBJECT(ob))->is_set)

DDATDEF DeeTypeObject DeeInverseSet_Type;
#define DeeInverseSet_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeInverseSet_Type) /* _inverseset is final */
#define DeeInverseSet_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeInverseSet_Type)


DECL_END

#endif /* !GUARD_DEEMON_SET_H */
