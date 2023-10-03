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
#ifndef GUARD_DEEMON_FLOAT_H
#define GUARD_DEEMON_FLOAT_H 1

#include "api.h"

#include "object.h"
/**/

#include <hybrid/typecore.h>

#ifdef CONFIG_NO_FPU
#undef CONFIG_HAVE_FPU
#undef CONFIG_HAVE_IEEE754
#undef CONFIG_HAVE_IEEE754_LE
#undef CONFIG_HAVE_IEEE754_BE
#else /* CONFIG_NO_FPU */
#ifndef CONFIG_HAVE_FPU
#define CONFIG_HAVE_FPU
#endif /* !CONFIG_HAVE_FPU */
#ifdef CONFIG_NO_IEEE754
#undef CONFIG_HAVE_IEEE754
#undef CONFIG_HAVE_IEEE754_LE
#undef CONFIG_HAVE_IEEE754_BE
#elif defined(__SIZEOF_DOUBLE__) && __SIZEOF_DOUBLE__ != 8
#undef CONFIG_HAVE_IEEE754
#undef CONFIG_HAVE_IEEE754_LE
#undef CONFIG_HAVE_IEEE754_BE
#else /* CONFIG_NO_IEEE754 */
#ifndef CONFIG_HAVE_IEEE754
#define CONFIG_HAVE_IEEE754
#endif /* !CONFIG_HAVE_IEEE754 */
#if !defined(CONFIG_HAVE_IEEE754_LE) && !defined(CONFIG_HAVE_IEEE754_BE)
#ifndef __FLOAT_WORD_ORDER__
#define CONFIG_HAVE_IEEE754_LE /* Fallback... */
#elif __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CONFIG_HAVE_IEEE754_LE
#elif __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define CONFIG_HAVE_IEEE754_BE
#else /* __FLOAT_WORD_ORDER__ == ... */
#undef CONFIG_HAVE_IEEE754
#define CONFIG_NO_IEEE754
#endif /* __FLOAT_WORD_ORDER__ != ... */
#endif /* !CONFIG_HAVE_IEEE754_LE && !CONFIG_HAVE_IEEE754_BE */
#endif /* !CONFIG_NO_IEEE754 */
#endif /* !CONFIG_NO_FPU */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_float_object float_object
#define DEFINE_FLOAT     Dee_DEFINE_FLOAT
#endif /* DEE_SOURCE */

typedef struct Dee_float_object DeeFloatObject;

struct Dee_float_object {
	Dee_OBJECT_HEAD
#ifdef CONFIG_HAVE_FPU
	double      f_value; /* [const] The value of this float as a C-double. */
#endif /* CONFIG_HAVE_FPU */
};

#ifdef CONFIG_HAVE_FPU
#define Dee_DEFINE_FLOAT(name, value) DeeFloatObject name = { Dee_OBJECT_HEAD_INIT(&DeeFloat_Type), value }
#else /* CONFIG_HAVE_FPU */
#define Dee_DEFINE_FLOAT(name, value) DeeFloatObject name = { Dee_OBJECT_HEAD_INIT(&DeeFloat_Type) }
#endif /* !CONFIG_HAVE_FPU */

#ifdef CONFIG_HAVE_FPU
#define DeeFloat_VALUE(x) ((DeeFloatObject *)Dee_REQUIRES_OBJECT(x))->f_value
#endif /* CONFIG_HAVE_FPU */

#define DeeFloat_Check(x)      DeeObject_InstanceOfExact(x, &DeeFloat_Type) /* `float' is final */
#define DeeFloat_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeFloat_Type)
DDATDEF DeeTypeObject DeeFloat_Type;

/* Create and return a new floating point object. */
#ifdef CONFIG_HAVE_FPU
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeFloat_New(double value);

/* Convert a string into a floating-point number. */
DFUNDEF NONNULL((1)) double DCALL Dee_Strtod(char const *str, char **p_endptr);
#ifdef __COMPILER_HAVE_LONGDOUBLE
DFUNDEF NONNULL((1)) __LONGDOUBLE DCALL Dee_Strtold(char const *str, char **p_endptr);
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#endif /* CONFIG_HAVE_FPU */



/* Print a string representation of the given floating point value.
 * @param: flags: Set of `DEEFLOAT_PRINT_F*' */
#ifdef CONFIG_HAVE_FPU
DFUNDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
DeeFloat_Print(double value, Dee_formatprinter_t printer, void *arg,
               size_t width, size_t precision, unsigned int flags);
#ifdef __COMPILER_HAVE_LONGDOUBLE
DFUNDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
DeeFloat_LPrint(__LONGDOUBLE value, Dee_formatprinter_t printer, void *arg,
                size_t width, size_t precision, unsigned int flags);
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#define DeeFloat_PrintRepr(self, printer, arg) \
	DeeFloat_Print(DeeFloat_VALUE(self), printer, arg, 0, 0, DEEFLOAT_PRINT_FNORMAL)
#endif /* CONFIG_HAVE_FPU */
#define DEEFLOAT_PRINT_FNORMAL    0x0000 /* Normal printing flags. */
#define DEEFLOAT_PRINT_FLJUST     0x0002 /* Justify the written value to the left. */
#define DEEFLOAT_PRINT_FSIGN      0x0004 /* Always print a sign. */
#define DEEFLOAT_PRINT_FSPACE     0x0008 /* When no sign is printed, put a space character instead. */
#define DEEFLOAT_PRINT_FPADZERO   0x0010 /* Use '0' to pad leading digits to fit `width'. */
#define DEEFLOAT_PRINT_FWIDTH     0x0020 /* The given `width' must be respected. */
#define DEEFLOAT_PRINT_FPRECISION 0x0040 /* The given `precision' must be respected. */

DECL_END

#endif /* !GUARD_DEEMON_FLOAT_H */
