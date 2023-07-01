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
#ifndef GUARD_DEEMON_MAP_H
#define GUARD_DEEMON_MAP_H 1

#include "api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "object.h"

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
DDATDEF DeeObject          DeeMapping_EmptyInstance;
#define Dee_EmptyMapping (&DeeMapping_EmptyInstance)

DECL_END

#endif /* !GUARD_DEEMON_MAP_H */
