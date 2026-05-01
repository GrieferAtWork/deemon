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
/* deemon.Object.__timeout_nanoseconds__                                */
/************************************************************************/
[[getset]]
__timeout_nanoseconds__->?Dint;


[[wunused, getset_member("get")]] int
__timeout_nanoseconds__.object_as_timeout_nanoseconds([[nonnull]] DeeObject *__restrict self,
                                                      [[nonnull]] uint64_t *__restrict p_timeout_nanoseconds)
%{unsupported(err_obj_unsupportedf(self, "__timeout_nanoseconds__"))}
%{$none = {
	*p_timeout_nanoseconds = 0;
	return 0;
}}
{
	int result;
	DREF DeeObject *attr = LOCAL_GETATTR(self);
	if unlikely(!attr)
		goto err;
	result = DeeObject_AsUInt64M1(attr, p_timeout_nanoseconds);
	Dee_Decref(attr);
	return result;
err:
	return -1;
}
