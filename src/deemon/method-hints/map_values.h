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
/* deemon.Mapping.values                                                */
/************************************************************************/
[[getset, alias(Mapping.values)]]
__map_values__->?DSequence;



[[wunused, getset_member("get")]] DREF DeeObject *
__map_values__.map_values([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return DeeSeq_NewEmpty()}
%{$with__map_itervalues = DefaultSequence_MapValues_New(self)}
{
	return LOCAL_GETATTR(self);
}

map_values = {
	DeeMH_map_itervalues_t map_itervalues = REQUIRE(map_itervalues);
	if (map_itervalues == &default__map_itervalues__empty)
		return &$empty;
	if (map_itervalues)
		return &$with__map_itervalues;
};
