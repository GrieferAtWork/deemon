/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

/************************************************************************/
/* deemon.Set.operator size()                                           */
/************************************************************************/
__set_size__()->?Dint {
	if (DeeArg_Unpack(argc, argv, ":__set_size__"))
		goto err;
	return DeeSeq_OperatorSizeOb(self);
err:
	return NULL;
}

[[wunused]]
size_t __set_size__.set_operator_size([[nonnull]] DeeObject *self)
%{unsupported(auto("operator size"))}
%{$empty = 0}
%{$with__set_operator_foreach = [[prefix(DEFINE_default_seq_size_with_foreach_cb)]] {
	return (size_t)DeeSet_OperatorForeach(self, &default_seq_size_with_foreach_cb, NULL);
}} {
	DREF DeeObject *sizeob;
	sizeob = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!sizeob)
		goto err;
	return DeeObject_AsDirectSizeInherited(sizeob);
err:
	return (size_t)-1;
}


set_operator_size = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS != Dee_SEQCLASS_NONE && DeeType_RequireSize(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_size;
#endif /* !LOCAL_FOR_OPTIMIZE */
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return $with__set_operator_foreach;
};
