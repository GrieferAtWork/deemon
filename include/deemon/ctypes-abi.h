/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_CTYPES_ABI_H
#define GUARD_DEEMON_CTYPES_ABI_H 1

#include "api.h"

#include "object.h"
#include "module.h"

DECL_BEGIN

/* Helpers for interfacing with pointer-like objects, as defined by the ctypes ABI */


/* Return the void type object. */
LOCAL DREF DeeObject *DCALL DeeCTypes_GetVoid(void) {
	return DeeModule_GetExtern("ctypes", "void");
}

/* Construct a new void-pointer pointing to the given `address'. */
LOCAL DREF DeeObject *DCALL DeeCTypes_CreateVoidPointer(void *address) {
	DREF DeeObject *result, *ptr;
	result = DeeCTypes_GetVoid();
	if likely(result) {
		ptr = DeeObject_CallAttrStringf(result, "ptr", "Iu", address);
		Dee_Decref(result);
		result = ptr;
	}
	return result;
}

/* Extract the address of a given pointer-like object.
 * @return:  0: Success.
 * @return: -1: Error. */
LOCAL int DCALL DeeCTypes_GetPointer(DeeObject *__restrict self, void **paddress) {
	int result;
	DREF DeeObject *ptr;
	ptr = DeeObject_GetAttrString(self, "__ptr__");
	if unlikely(!ptr)
		goto err;
	result = DeeObject_AsUIntptr(ptr, (uintptr_t *)paddress);
	Dee_Decref(ptr);
	return result;
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_CTYPES_ABI_H */