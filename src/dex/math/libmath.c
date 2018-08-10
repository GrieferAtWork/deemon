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
#ifndef GUARD_DEX_FILES_LIBMATH_C
#define GUARD_DEX_FILES_LIBMATH_C 1
#define _KOS_SOURCE 1
#define _USE_MATH_DEFINES 1

#include "libmath.h"

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/objmethod.h>
#include <deemon/arg.h>
#include <deemon/float.h>

#include <math.h>

DECL_BEGIN

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


PRIVATE DREF DeeObject *DCALL
f_math_copysign(size_t argc, DeeObject **__restrict argv) {
 double x,y;
 if (DeeArg_Unpack(argc,argv,"DD:copysign",&x,&y))
     return NULL;
 M_PI;
 return DeeFloat_New(copysign(x,y));
}

PRIVATE DEFINE_CMETHOD(math_copysign,f_math_copysign);
PRIVATE DEFINE_FLOAT(math_pi,M_PI);


PRIVATE struct dex_symbol symbols[] = {
    { "copysign", (DeeObject *)&math_copysign, MODSYM_FNORMAL,
      DOC("(float x,float y)->float\n"
          "Return @x with the sign copied from @y") },
    /* TODO */
    { "pi", (DeeObject *)&math_pi, MODSYM_FNORMAL, DOC("The mathematical constant PI") },
    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols
};

DECL_END


#endif /* !GUARD_DEX_FILES_LIBMATH_C */
