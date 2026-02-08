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
/*!export **/
/*!export DeeSet_**/
#ifndef GUARD_DEEMON_SET_H
#define GUARD_DEEMON_SET_H 1 /*!export-*/

#include "api.h"

#include "types.h" /* DeeObject, DeeTypeObject */

#ifndef __INTELLISENSE__
#include "object.h" /* Dee_Incref */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/* Base class for set-style sequence types (e.g. `HashSet')
 * :: Characteristics of a set type::
 *   - `class member Iterator: Iterator from deemon;'
 *   - `operator iter(): Set.Iterator;'
 *     Enumerate set elements in some arbitrary order.
 *   - `operator contains(ob: Object): bool':
 *     Returns ?t if @ob is apart of @this set
 *   - The getitem operator is not implemented.
 *   - The getrange operator is not implemented.
 *
 * Using `Set from deemon' (aka. `DeeSet_Type') as a base class, it will
 * automatically provide for the following member functions and operators:
 *
 * difference(other: Set): Set;
 * operator - (other: Set): Set;
 *     Returns a set of all objects from @this, excluding those also found in @other
 * 
 * intersection(other: Set): Set;
 * operator & (other: Set): Set;
 *     Returns the intersection of @this and @other
 * 
 * isdisjoint(other: Set): bool;
 *     Returns ?t if ${##(this & other) == 0}
 *     In other words: If @this and @other have no items in common.
 *
 * union(other: Set): Set;
 * operator | (other: Set): Set;
 * operator + (other: Set): Set;
 *     Returns the union of @this and @other
 * 
 * symmetric_difference(other: Set): Set;
 * operator ^ (other: Set): Set;
 *     Returns a set containing objects only found in either
 *     @this or @other, but not those found in both.
 * 
 * issubset(other: Set): bool;
 * operator <= (other: Set): bool;
 *     Returns ?t if all items found in @this set can also be found in @other
 * 
 * operator == (other: Set): bool;
 *     Returns ?t if @this set contains the same
 *     items as @other, and not any more than that
 * 
 * operator < (other: Set): bool;
 *     The result of ${this <= other && this != other}
 *
 * issuperset(other: Set): bool;
 * operator >= (other: Set): bool;
 *     Returns ?t if all items found in @other can also be found in @this set
 *
 * operator ~ (): Set;
 *     Returns a symbolic set that behaves as though it contained
 *     any feasible object that isn't already apart of `this' set.
 *     Note however that due to the impossibility of such a set,
 *     you cannot iterate its elements, and the only ~real~ operator
 *     implemented by it is `operator contains'.
 *     Its main purpose is for being used in conjunction with
 *     `operator &' in order to create a sub-set that doesn't
 *     contain a certain set of sub-elements:
 *     >> local items = HashSet({ 10, 11, 15, 20, 30 });
 *     >> print repr(items & ~({ 11, 15 } as Set))
 *
 * NOTE: `DeeSet_Type' itself is derived from `Sequence from deemon' (aka. `DeeSeq_Type')
 * NOTE: Because `DeeSet_Type' inherits from `DeeSeq_Type', all member functions that
 *       it provides, as well as its operators (such as `bool', compare, etc.), are
 *       implicitly inherited, and also provided by objects derived from `DeeSet_Type',
 *       and `HashSet from deemon' itself.
 *       This also means that sub-classes of `Mapping from deemon' should respect the
 *       `Iterator' interface, provided a `class member Iterator: Type' which represents
 *       the iterator type used by the mapping. */
DDATDEF DeeTypeObject DeeSet_Type; /* `Set from deemon' */

/* An empty instance of a generic set object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSet_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSet_Type' should be considered stub/empty. */
DDATDEF DeeObject DeeSet_EmptyInstance;
#define Dee_EmptySet (&DeeSet_EmptyInstance)
#ifdef __INTELLISENSE__
#define DeeSet_NewEmpty() (&DeeSet_EmptyInstance)
#else /* __INTELLISENSE__ */
#define DeeSet_NewEmpty() (Dee_Incref(&DeeSet_EmptyInstance), &DeeSet_EmptyInstance)
#endif /* !__INTELLISENSE__ */

/* A universal instance of a generic set object (i.e. the set of everything in the universe).
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by calling `~Set()'. */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_C
DDATDEF DeeObject DeeSet_UniversalInstance;
#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_C */
#define Dee_UniversalSet (&DeeSet_UniversalInstance)
#ifdef __INTELLISENSE__
#define DeeSet_NewUniversal() (&DeeSet_UniversalInstance)
#else /* __INTELLISENSE__ */
#define DeeSet_NewUniversal() (Dee_Incref(&DeeSet_UniversalInstance), &DeeSet_UniversalInstance)
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_SET_H */
