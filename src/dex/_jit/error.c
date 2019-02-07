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
#ifndef GUARD_DEX_JIT_ERROR_C
#define GUARD_DEX_JIT_ERROR_C 1

#include "libjit.h"
#include <deemon/error.h>

DECL_BEGIN

INTERN ATTR_COLD int DCALL
err_invalid_argc_len(char const *function_name, size_t function_size,
                     size_t argc_cur, size_t argc_min, size_t argc_max) {
 if (argc_min == argc_max) {
  return DeeError_Throwf(&DeeError_TypeError,
                         "function%s%$s expects %Iu arguments when %Iu w%s given",
                         function_size ? " " : "",function_size,function_name,
                         argc_min,argc_cur,argc_cur == 1 ? "as" : "ere");
 } else {
  return DeeError_Throwf(&DeeError_TypeError,
                         "function%s%$s expects between %Iu and %Iu arguments when %Iu w%s given",
                         function_size ? " " : "",function_size,function_name,
                         argc_min,argc_max,argc_cur,argc_cur == 1 ? "as" : "ere");
 }
}

INTERN ATTR_COLD int DCALL err_no_active_exception(void) {
 return DeeError_Throwf(&DeeError_RuntimeError,"No active exception");
}

INTERN ATTR_COLD int DCALL
err_unknown_global(DeeObject *__restrict key) {
 ASSERT_OBJECT(key);
 return DeeError_Throwf(&DeeError_KeyError,
                        "Unknown global `%k'",
                        key);
}
INTERN ATTR_COLD int DCALL
err_unknown_global_str_len(char const *__restrict key, size_t keylen) {
 return DeeError_Throwf(&DeeError_KeyError,
                        "Unknown global `%$s'",
                        keylen,key);
}

INTERN ATTR_COLD int DCALL
err_invalid_unpack_size(DeeObject *__restrict unpack_object,
                        size_t need_size, size_t real_size) {
 ASSERT_OBJECT(unpack_object);
 (void)unpack_object;
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",real_size,
                        real_size == 1 ? "as" : "ere");
}
INTERN ATTR_COLD int DCALL
err_invalid_unpack_iter_size(DeeObject *__restrict unpack_object,
                             DeeObject *__restrict unpack_iterator,
                             size_t need_size) {
 ASSERT_OBJECT(unpack_object);
 ASSERT_OBJECT(unpack_iterator);
 (void)unpack_object;
 (void)unpack_iterator;
 return DeeError_Throwf(&DeeError_UnpackError,
                        "Expected %Iu object%s when at least %Iu w%s given",
                        need_size,need_size > 1 ? "s" : "",need_size+1,
                        need_size == 0 ? "as" : "ere");
}


DECL_END

#endif /* !GUARD_DEX_JIT_ERROR_C */
