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

/************************************************************************/
/* deemon.Mapping.keys                                                  */
/************************************************************************/
[[getset, alias(Mapping.keys)]]
__map_keys__->?DSet;



[[wunused, getset_member("get")]] DREF DeeObject *
__map_keys__.map_keys([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$empty = return_reference_(Dee_EmptySet)}
%{$none = return_none}
%{$with__map_iterkeys = DefaultSequence_MapKeys_New(self)}
{
	return LOCAL_GETATTR(self);
}

map_keys = {
	DeeMH_map_iterkeys_t map_iterkeys = REQUIRE(map_iterkeys);
	if (map_iterkeys == &default__map_iterkeys__empty)
		return &$empty;
	if (map_iterkeys)
		return &$with__map_iterkeys;
};
