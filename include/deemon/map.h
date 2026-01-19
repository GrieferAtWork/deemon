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
#ifndef GUARD_DEEMON_MAP_H
#define GUARD_DEEMON_MAP_H 1

#include "api.h"

#include "types.h"

#include <stddef.h> /* size_t */

#ifndef __INTELLISENSE__
#include "object.h"
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/* Base class for mapping-style sequence types (e.g. `Dict')
 * :: Characteristics of a mapping type::
 *   - operator iter():
 *     sequence item through iteration yields another
 *     sequence consisting of 2 elements: `key' = `value'
 *     This sub-sequence is usually often implemented as a tuple,
 *     and code should be optimized for that code, however this
 *     is not a requirement.
 *   - The getitem operator is implemented as a lookup-key
 *     function that returns the associated value.
 *     A missing key causes a `KeyError' to be thrown.
 *   - The getrange operator is not implemented.
 *
 * Using `Mapping from deemon' (aka. `DeeMapping_Type') as a base class, it will
 * automatically provide for the following member functions and operators:
 *   - `operator getrange()'
 *      - Always thrown a NotImplement error, overriding `sequence.operator getrange'.
 *   - `operator | (other: Mapping): Mapping'
 *   - `operator & (other: Mapping): Mapping'
 *      - Returns a mapping consisting of the union, or intersection of keys from `this' and `other'
 *   - `operator contains(key: Object): bool'
 *      - Returns `true' / `false' indicative of there being a key-item pair for `key'
 *   - `operator tp_repr() -> string'
 *      - Returns a representation of the contents of the mapping,
 *        following the generic-mapping syntax of `{ key: item, ... }'
 *   - `class member Proxy: Type'
 *      - Returns the common base-class of `keys', `values' and `items'
 *        By default, this is `DeeMappingProxy_Type'
 *   - `class member Keys: Type'
 *      - Returns the return type of the `keys' member property
 *        By default, this is `DeeMappingKeys_Type'
 *   - `class member Values: Type'
 *      - Returns the return type of the `values' member property
 *        By default, this is `DeeMappingValues_Type'
 *   - `class member Items: Type'
 *      - Returns the return type of the `items' member property
 *        By default, this is `DeeMappingItems_Type'
 *   - `property keys: Mapping.Keys'
 *      - Returns a sequence that can be enumerated to view only the keys of a mapping
 *   - `property values: Mapping.Values'
 *      - Returns a sequence that can be enumerated to view only the values of a mapping
 *   - `property items: Mapping.Items'
 *      - Returns a sequence that can be enumerated to view the key-item pairs as
 *        2-element sequences, the same way they could be viewed if the mapping itself
 *        was being iterated.
 *        Note however that the returned object is a pure sequence, meaning that it
 *        implements an index-based getitem operator, as well as a getrange operator.
 *   - `function get(key: Object, def: Object = none): Object'
 *      - Same as `operator []', but if `key' doesn't exist, `def' is returned instead.
 * NOTE: `DeeMapping_Type' itself is derived from `Sequence from deemon' (aka. `DeeSeq_Type')
 * NOTE: Because `DeeMapping_Type' inherits from `DeeSeq_Type', all member functions that
 *       it provides, as well as its operators (such as `bool', compare, etc.), are
 *       implicitly inherited, and also provided by objects derived from `DeeMapping_Type',
 *       and `Mapping from deemon' itself.
 *       This also means that sub-classes of `Mapping from deemon' should respect the
 *       iterator interface, provided a `class member Iterator: Type' which represents
 *       the iterator type used by the mapping. */
DDATDEF DeeTypeObject DeeMapping_Type; /* `Mapping from deemon' */
#define DeeMapping_Check(ob) DeeObject_Implements(ob, &DeeMapping_Type)

/* An empty instance of a generic mapping object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeMapping_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeMapping_Type' should be considered stub/empty. */
DDATDEF DeeObject DeeMapping_EmptyInstance;
#define Dee_EmptyMapping (&DeeMapping_EmptyInstance)
#ifdef __INTELLISENSE__
#define DeeMapping_NewEmpty() (&DeeMapping_EmptyInstance)
#else /* __INTELLISENSE__ */
#define DeeMapping_NewEmpty() (Dee_Incref(&DeeMapping_EmptyInstance), &DeeMapping_EmptyInstance)
#endif /* !__INTELLISENSE__ */



/* Wrapper for `DeeObject_BoolInherited(DeeObject_InvokeMethodHint(map_operator_contains, self, key))' */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeMap_OperatorContainsAsBool(DeeObject *self,
                              DeeObject *key);


#undef si_key
#undef si_value
typedef struct {
	DREF DeeObject *si_key;   /* [1..1][const] The key of this shared item. */
	DREF DeeObject *si_value; /* [1..1][const] The value of this shared item. */
} DeeSharedItem;

/* Type of object returned by `DeeSharedMap_NewShared()' */
DDATDEF DeeTypeObject DeeSharedMap_Type;

/* Create a new shared map that will inherit elements from
 * the given vector once `DeeSharedMap_Decref()' is called.
 * NOTE: This function can implicitly inherit a reference to each item of the
 *       given vector, though does not actually inherit the vector itself:
 *       - DeeSharedMap_Decref:            The `vector' arg here is `DREF DeeSharedItem *const *'
 *       - DeeSharedMap_DecrefNoGiftItems: The `vector' arg here is `DeeSharedItem *const *'
 * NOTE: Do NOT free the given `vector' before calling `DeeSharedMap_Decref'
 *       on the returned object, as `vector' will be shared with it until
 *       that point in time! */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeSharedMap_NewShared(size_t length, DREF DeeSharedItem const *vector);

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `vector',
 * as passed to `DeeSharedMap_NewShared()', but still decref()
 * all contained object.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sskv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedMap object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_MAP' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedMap_Decref(DREF DeeObject *__restrict self);

/* Same as `DeeSharedMap_Decref()', but should be used if the caller
 * does *not* want to gift the vector references to all of its items. */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedMap_DecrefNoGiftItems(DREF DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_MAP_H */
