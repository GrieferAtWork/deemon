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
#ifndef GUARD_DEX_CTYPES_C_MALLOC_C
#define GUARD_DEX_CTYPES_C_MALLOC_C 1

#include "libctypes.h"
#include "c_api.h"

#include <deemon/arg.h>
#include <deemon/object.h>
#include <deemon/error.h>
#include <deemon/none.h>

#include <stdlib.h>

DECL_BEGIN

INTERN DREF DeeObject *DCALL
capi_free(size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *uptr;
 union pointer ptr;
 if (DeeArg_Unpack(argc,argv,"o:free",&uptr) ||
     DeeObject_AsPointer(uptr,&DeeCVoid_Type,&ptr))
     goto err;
 CTYPES_RECURSIVE_FAULTPROTECT(Dee_Free(ptr.ptr),goto err);
 return_none;
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
capi_malloc(size_t argc, DeeObject **__restrict argv) {
 void *ptr; DREF DeeObject *result; size_t num_bytes;
 if (DeeArg_Unpack(argc,argv,DEE_FMT_SIZE_T ":malloc",&num_bytes))
     goto err;
 ptr = Dee_Malloc(num_bytes);
 if unlikely(!ptr) goto err;
 result = DeePointer_NewVoid(ptr);
 if unlikely(!result) Dee_Free(ptr);
 return result;
err:
 return NULL;
}


INTERN DREF DeeObject *DCALL
capi_realloc(size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *uptr; DREF DeeObject *result;
 union pointer ptr; size_t new_size;
 if (DeeArg_Unpack(argc,argv,"oIu:realloc",&uptr,&new_size) ||
     DeeObject_AsPointer(uptr,&DeeCVoid_Type,&ptr))
     goto err;
 /* Allocate the resulting pointer _before_ doing the realloc().
  * This way, we don't run the chance to cause an exception
  * after we've already successfully reallocted the user-pointer. */
 result = DeePointer_NewVoid(0);
 if unlikely(!result) goto err;
 CTYPES_RECURSIVE_FAULTPROTECT(
     ptr.ptr = Dee_Realloc(ptr.ptr,new_size),
     goto err_r);
 if unlikely(!ptr.ptr) goto err_r;
 /* Update the resulting pointer. */
 ((struct pointer_object *)result)->p_ptr.ptr = ptr.ptr;
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
capi_calloc(size_t argc, DeeObject **__restrict argv) {
 void *ptr; DREF DeeObject *result;
 size_t count,num_bytes = 1,total;
 if (DeeArg_Unpack(argc,argv,DEE_FMT_SIZE_T "|" DEE_FMT_SIZE_T ":calloc",&count,&num_bytes))
     goto err;
 total = count * num_bytes;
 /* Check for allocation overflow. */
 if unlikely((total < count || total < num_bytes) &&
              count && num_bytes) {
  DeeError_Throwf(&DeeError_IntegerOverflow,
                  "calloc allocation total of `%Iu * %Iu' is overflowing",
                  count,num_bytes);
  goto err;
 }
 ptr = Dee_Calloc(total);
 if unlikely(!ptr) goto err;
 result = DeePointer_NewVoid(ptr);
 if unlikely(!result) Dee_Free(ptr);
 return result;
err:
 return NULL;
}



INTERN DREF DeeObject *DCALL
capi_trymalloc(size_t argc, DeeObject **__restrict argv) {
 void *ptr; DREF DeeObject *result; size_t num_bytes;
 if (DeeArg_Unpack(argc,argv,DEE_FMT_SIZE_T ":trymalloc",&num_bytes))
     goto err;
 ptr = Dee_TryMalloc(num_bytes);
 result = DeePointer_NewVoid(ptr);
 if unlikely(!result) Dee_Free(ptr);
 return result;
err:
 return NULL;
}


INTERN DREF DeeObject *DCALL
capi_tryrealloc(size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *uptr; DREF DeeObject *result;
 union pointer ptr; size_t new_size;
 if (DeeArg_Unpack(argc,argv,"oIu:tryrealloc",&uptr,&new_size) ||
     DeeObject_AsPointer(uptr,&DeeCVoid_Type,&ptr))
     goto err;
 /* Allocate the resulting pointer _before_ doing the realloc().
  * This way, we don't run the chance to cause an exception
  * after we've already successfully reallocted the user-pointer. */
 result = DeePointer_NewVoid(0);
 if unlikely(!result) goto err;
 CTYPES_RECURSIVE_FAULTPROTECT(
     ptr.ptr = Dee_TryRealloc(ptr.ptr,new_size),
     goto err_r);
 /* Update the resulting pointer. */
 ((struct pointer_object *)result)->p_ptr.ptr = ptr.ptr;
 return result;
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
err_r:
 Dee_Decref(result);
#endif /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
capi_trycalloc(size_t argc, DeeObject **__restrict argv) {
 void *ptr; DREF DeeObject *result;
 size_t count,num_bytes = 1,total;
 if (DeeArg_Unpack(argc,argv,DEE_FMT_SIZE_T "|" DEE_FMT_SIZE_T ":trycalloc",&count,&num_bytes))
     goto err;
 total = count * num_bytes;
 /* Check for allocation overflow. */
 if unlikely((total < count || total < num_bytes) &&
              count && num_bytes) {
  ptr = NULL;
 } else {
  ptr = Dee_TryCalloc(total);
 }
 if unlikely(!ptr) goto err;
 result = DeePointer_NewVoid(ptr);
 if unlikely(!result) Dee_Free(ptr);
 return result;
err:
 return NULL;
}





DECL_END


#endif /* !GUARD_DEX_CTYPES_C_MALLOC_C */
