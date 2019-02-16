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
#ifndef GUARD_DEEMON_FLOAT_H
#define GUARD_DEEMON_FLOAT_H 1

#include "api.h"
#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_float_object float_object
#define DEFINE_FLOAT     Dee_DEFINE_FLOAT
#endif /* DEE_SOURCE */

typedef struct Dee_float_object DeeFloatObject;

struct Dee_float_object {
    Dee_OBJECT_HEAD
    double      f_value; /* [const] The value of this float as a C-double. */
};
#define Dee_DEFINE_FLOAT(name,value) \
  DeeFloatObject name = { Dee_OBJECT_HEAD_INIT(&DeeFloat_Type), value }


#define DeeFloat_VALUE(x) ((DeeFloatObject *)Dee_REQUIRES_OBJECT(x))->f_value

#define DeeFloat_Check(x)      DeeObject_InstanceOfExact(x,&DeeFloat_Type) /* `float' is final */
#define DeeFloat_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeFloat_Type)
DDATDEF DeeTypeObject DeeFloat_Type;

/* Create and return a new floating point object. */
DFUNDEF DREF DeeObject *DCALL DeeFloat_New(double value);



/* Print a string representation of the given floating point value.
 * @param: flags: Set of `DEEFLOAT_PRINT_F*' */
DFUNDEF Dee_ssize_t DCALL DeeFloat_Print(double value, Dee_formatprinter_t printer, void *arg,
                                         size_t width, size_t precision, unsigned int flags);
#ifdef __COMPILER_HAVE_LONGDOUBLE
DFUNDEF Dee_ssize_t DCALL DeeFloat_LPrint(long double value, Dee_formatprinter_t printer, void *arg,
                                          size_t width, size_t precision, unsigned int flags);
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#define DEEFLOAT_PRINT_FNORMAL    0x0000 /* Normal printing flags. */
#define DEEFLOAT_PRINT_FLJUST     0x0002 /* Justify the written value to the left. */
#define DEEFLOAT_PRINT_FSIGN      0x0004 /* Always print a sign. */
#define DEEFLOAT_PRINT_FSPACE     0x0008 /* When no sign is printed, put a space character instead. */
#define DEEFLOAT_PRINT_FPADZERO   0x0010 /* Use '0' to pad leading digits to fit `width'. */
#define DEEFLOAT_PRINT_FWIDTH     0x0020 /* The given `width' must be respected. */
#define DEEFLOAT_PRINT_FPRECISION 0x0040 /* The given `precision' must be respected. */



DECL_END

#endif /* !GUARD_DEEMON_FLOAT_H */
