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
#ifdef __INTELLISENSE__
//#define DEFINE_DeeArg_Unpack
#define DEFINE_DeeArg_UnpackStruct
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/arg.h>
#include <deemon/error-rt.h>
#include <deemon/kwds.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>

#include <hybrid/host.h>
#include <hybrid/int128.h>
#include <hybrid/typecore.h>

#include "runtime_error.h"

#include <stdarg.h> /* va_arg, va_end, va_list, va_start */
#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* INTn_MAX, INTn_MIN, UINTn_MAX, intN_t, uintN_t, uintptr_t */

#if (defined(DEFINE_DeeArg_Unpack) + defined(DEFINE_DeeArg_UnpackStruct)) != 1
#error "Must #define exactly one of these macros"
#endif

#ifndef __SIZEOF_BOOL__
#define __SIZEOF_BOOL__ __SIZEOF_CHAR__
#endif /* !__SIZEOF_BOOL__ */

#ifndef __ALIGNOF_BOOL__
#define __ALIGNOF_BOOL__ __SIZEOF_BOOL__
#endif /* !__ALIGNOF_BOOL__ */

#ifndef VALIST_ADDR
#ifdef CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY
#define VALIST_ADDR(x) (&(x))
#else /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
#define VALIST_ADDR(x) (&(x)[0])
#endif /* !CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
#endif /* !VALIST_ADDR */

#ifndef INT8_MIN
#define INT8_MIN __INT8_MIN__
#endif /* !INT8_MIN */
#ifndef INT8_MAX
#define INT8_MAX __INT8_MAX__
#endif /* !INT8_MAX */
#ifndef UINT8_MAX
#define UINT8_MAX __UINT8_MAX__
#endif /* !UINT8_MAX */
#ifndef INT16_MIN
#define INT16_MIN __INT16_MIN__
#endif /* !INT16_MIN */
#ifndef INT16_MAX
#define INT16_MAX __INT16_MAX__
#endif /* !INT16_MAX */
#ifndef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX */
#ifndef INT32_MIN
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */
#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#ifndef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */
#ifndef INT64_MIN
#define INT64_MIN __INT64_MIN__
#endif /* !INT64_MIN */
#ifndef INT64_MAX
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX */
#ifndef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX */

/* MSCV for 32-bit is wholly broken in terms of stack alignment:
 *
 * >> PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
 * >> file_seek(DeeObject *self, size_t argc,
 * >>           DeeObject *const *argv, DeeObject *kw) {
 * >>	Dee_pos_t result;
 * >>	int whence = Dee_SEEK_SET;
 * >>	struct {
 * >>		Dee_off_t off;
 * >>		DeeObject *whence;
 * >>	} args;
 * >>	args.whence = NULL;
 * >>	ASSERT(IS_ALIGNED((uintptr_t)&args, 8));
 * >>	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__off_whence,
 * >>	                          UNPdN(Dee_SIZEOF_OFF_T) "|o:seek", &args))
 * >>		goto err;
 * >>	...
 * >> }
 *
 * Without special handling here, the above breaks on MSVC+32-bit. Not because
 * the code above is buggy, but because MSVC is:
 * - Even though MSVC reports "alignof(Dee_off_t) == 8", it just simply disregards
 *   that fact when it comes to allocating the containing structure on-stack. As
 *   such, the "ASSERT" in the above code will fail (since "args" only gets aligned
 *   on a 4-byte boundary)
 * - As such, without some special handling here, we would RIGHTFULLY try to align
 *   the initial "args" pointer to its nearest 8-byte address (which actually ends
 *   up being the high 32-bit of "args.off" and "args.whence")
 *
 * >> I've never seen such immsense BS, so please excuse the upcoming language. <<
 *
 * ==== CAUTION ==== RETARDED MSVC NONSENSE AHEAD ====
 * Type                                                            sizeof  alignof stack align (32-bit)
 * __int8, unsigned __int8, char, bool                               1        1      1
 * __int16, unsigned __int16, short, unsigned short, wchar_t         2        2      2
 * __int32, unsigned __int32, int, unsigned int, long, unsigned long 4        4      4
 * float                                                             4        4      4
 * double                                                            8        8      4
 * __int64, unsigned __int64, long long, unsigned long long          8        8      4
 * __ptr32                                                           4        4      4
 * __ptr64                                                           8        8      4
 * __m64                                                             8        8      8
 * __m128, __m128d, __m128i                                          16       16     16
 * src: https://github.com/rust-lang/rust/issues/112480#issuecomment-2411262503
 * ==== CAUTION ==== RETARDED MSVC NONSENSE ABOVE ====
 *
 * The FUCKING BS TLDR of the above is, that we essentially have to align
 * a proxy "offset" variable that starts at "0" instead of looking at the
 * actual pointer that's supposed to be getting aligned:
 * >> OUT(char)  ->  (CEIL_ALIGN(offset, 1) - offset=0) == 0  ->  don't align  (new offset=1)
 * >> OUT(int)   ->  (CEIL_ALIGN(offset, 4) - offset=1) == 3  ->  out+=3       (new offset=8)
 *
 * !HOWEVER! this only needs to happen under _MSC_VER + __i386__
 *
 * NOTE: The real solution ms should've gone for here would have been to
 *       just have `alignof(int64_t) == 4', like gcc does and actually
 *       makes a lot of sense when you realize that int64_t on a 32-bit
 *       machine is actually just int32_t[2] with a bunch of compiler
 *       magic. */
#undef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
#if defined(_MSC_VER) && defined(__i386__) && !defined(__x86_64__) && __ALIGNOF_INT64__ == 8
#define COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
#endif /* _MSC_VER && __i386__ && !__x86_64__ && __ALIGNOF_INT64__ == 8 */


#ifdef DEFINE_DeeArg_Unpack
#define LOCAL_Dee_VPUnpackf               Dee_VPUnpackf
#define LOCAL_Dee_VPUnpackfSkip           Dee_VPUnpackfSkip
#define LOCAL_DeeArg_VUnpack              DeeArg_VUnpack
#define LOCAL_DeeArg_VUnpackKw            DeeArg_VUnpackKw
#define LOCAL_Dee_VUnpackf                Dee_VUnpackf
#define LOCAL_PARAM_P_OUT                 struct va_list_struct *__restrict p_args
#define LOCAL_PARAM_OUT                   va_list args
#define LOCAL_ARG_P_OUT                   p_args
#define LOCAL_ARG_OUT                     args
#define LOCAL_OUTPUT(T, alignof_T, value) (void)(*va_arg(p_args->vl_ap, T *) = (value))
#define LOCAL_OUTPUT_PTR_PREALIGN(T, _)   (void)0
#define LOCAL_OUTPUT_PTR(T)               va_arg(p_args->vl_ap, T *)
#define LOCAL_SKIP(T, alignof_T)          (void)va_arg(p_args->vl_ap, T *)
#else /* DEFINE_DeeArg_Unpack */

#ifdef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
#define LOCAL_Dee_VPUnpackf     pab_Dee_PUnpackStruct /* pab == PissAssBuggy */
#define LOCAL_Dee_VPUnpackfSkip pab_Dee_PUnpackStructSkip
#define LOCAL_PARAM_P_OUT       void **p_out, uintptr_t *p_out_align
#define LOCAL_ARG_P_OUT         p_out, p_out_align
#else /* COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#define LOCAL_Dee_VPUnpackf     Dee_PUnpackStruct
#define LOCAL_Dee_VPUnpackfSkip Dee_PUnpackStructSkip
#define LOCAL_PARAM_P_OUT       void **p_out
#define LOCAL_ARG_P_OUT         p_out
#endif /* !COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#define LOCAL_DeeArg_VUnpack    DeeArg_UnpackStruct
#define LOCAL_DeeArg_VUnpackKw  DeeArg_UnpackStructKw
#define LOCAL_Dee_VUnpackf      Dee_UnpackStruct
#define LOCAL_PARAM_OUT         void *out
#define LOCAL_ARG_OUT           out
#ifdef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
#define LOCAL_OUTPUT(T, alignof_T, value)                                                \
	do {                                                                                 \
		uintptr_t new_out_align = (*p_out_align + (alignof_T) - 1) & ~((alignof_T) - 1); \
		size_t delta = new_out_align - *p_out_align;                                     \
		*p_out = (void *)((uintptr_t)*p_out + delta);                                    \
		*p_out_align += delta;                                                           \
		*(T *)*p_out = (value);                                                          \
		*p_out = (void *)((uintptr_t)*p_out + sizeof(T));                                \
		*p_out_align += sizeof(T);                                                       \
	}	__WHILE0
#define LOCAL_OUTPUT_PTR_PREALIGN(T, alignof_T) \
	do {                                                                                 \
		uintptr_t new_out_align = (*p_out_align + (alignof_T) - 1) & ~((alignof_T) - 1); \
		size_t delta = (new_out_align - *p_out_align) + sizeof(T);                       \
		*p_out = (void *)((uintptr_t)*p_out + delta);                                    \
		*p_out_align += delta;                                                           \
	}	__WHILE0
#define LOCAL_OUTPUT_PTR(T) ((T *)(void *)((uintptr_t)*p_out - sizeof(T)))
#define LOCAL_SKIP(T, alignof_T) \
	(void)(*p_out = (void *)((((uintptr_t)*p_out + (alignof_T) - 1) & ~((alignof_T) - 1)) + sizeof(T)))
#else /* COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#define LOCAL_OUTPUT(T, alignof_T, value)                                                 \
	(void)(*p_out = (void *)(((uintptr_t)*p_out + (alignof_T) - 1) & ~((alignof_T) - 1)), \
	       *(T *)*p_out = (value), *p_out = (void *)((uintptr_t)*p_out + sizeof(T)))
#define LOCAL_OUTPUT_PTR_PREALIGN(T, alignof_T) \
	(void)(*p_out = (void *)((((uintptr_t)*p_out + (alignof_T) - 1) & ~((alignof_T) - 1)) + sizeof(T)))
#define LOCAL_OUTPUT_PTR(T) ((T *)(void *)((uintptr_t)*p_out - sizeof(T)))
#define LOCAL_SKIP(T, alignof_T) \
	(void)(*p_out = (void *)((((uintptr_t)*p_out + (alignof_T) - 1) & ~((alignof_T) - 1)) + sizeof(T)))
#endif /* !COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#endif /* !DEFINE_DeeArg_Unpack */


DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#ifndef DEFINED_count_unpack_args
#define DEFINED_count_unpack_args
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
count_unpack_args(char const **__restrict p_format) {
	size_t result      = 0;
	char const *format = *p_format;
	while (*format) {
		switch (*format++) {

			/* Length modifiers / ignored. */
		case ',':
		case 'h':
		case 'l':
		case 'I':
		case 'L':
		case 'U':
			break;

		case 'n':
		case '-': /* none */
		case 'o': /* Object */
		case 's': /* string */
		case 'f':
		case 'D': /* float */
		case 'd':
		case 'i': /* signed int */
		case 'u':
		case 'x': /* unsigned int */
			++result;
			break;

		case '(': {
			unsigned int recursion;
			recursion = 1;
			for (; *format; ++format) {
				if (*format == '(') {
					++recursion;
				} else if (*format == ')') {
					if (!--recursion) {
						++format;
						break;
					}
				} else if (!*format) {
					break;
				}
			}
			++result;
		}	break;

		default:
			if (format[-1] >= '0' && format[-1] <= '9')
				break;
			ATTR_FALLTHROUGH
		case '\0':
			--format;
			goto done;
		}
	}
done:
	*p_format = format;
	return result;
}
#endif /* !DEFINED_count_unpack_args */

#if defined(DEFINE_DeeArg_UnpackStruct) && defined(COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT)
PRIVATE
#else /* DEFINE_DeeArg_UnpackStruct && COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
PUBLIC
#endif /* !DEFINE_DeeArg_UnpackStruct || !COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
WUNUSED NONNULL((1, 2, 3)) int
(DCALL LOCAL_Dee_VPUnpackf)(DeeObject *__restrict self,
                            char const **__restrict p_format,
                            LOCAL_PARAM_P_OUT) {
	char const *format = *p_format;
again:
	switch (*format++) {

	case ',':
		goto again;

	case '(': {
		DREF DeeObject *iterator, *elem;
		size_t argc;
		bool is_optional;
		char const *fmt_start;
		int temp;
		/* Unpack a sequence. */
		iterator = DeeObject_Iter(self);
		if unlikely(!iterator)
			goto err;
		is_optional = false, fmt_start = format, argc = 0;
		while (*format) {
			if (*format == '|') {
				is_optional = true;
				++format;
			}
			elem = DeeObject_IterNext(iterator);
			if unlikely(!elem)
				goto err;
			if (elem == ITER_DONE) {
				if (!is_optional && *format && *format != ')')
					goto invalid_argc2;
				break;
			} else if (*format == ')')
				goto invalid_argc;
			if (!*format || *format == ')') {
				if (!is_optional)
					goto invalid_argc;
				break;
			}
			/* Recursively unpacked contained objects. */
			temp = LOCAL_Dee_VPUnpackf(elem, &format, LOCAL_ARG_P_OUT);
			Dee_Decref(elem);
			if unlikely(temp)
				return temp;
			++argc;
		}
		Dee_Decref(iterator);
		if (*format == ')')
			++format;
		break;
invalid_argc:
		/* Count the remaining arguments for the error message. */
		Dee_Decref(elem);
		++argc;
		{
			size_t remaining;
			remaining = DeeObject_IterAdvance(iterator, (size_t)-1);
			if unlikely(remaining == (size_t)-1)
				goto err_iter;
			argc += remaining;
		}
invalid_argc2:
		Dee_Decref(iterator);
		{
			size_t argc_min;
			size_t argc_max;
			format   = fmt_start;
			argc_min = argc_max = count_unpack_args(&format);
			if (*format == '|') {
				++format;
				argc_max += count_unpack_args(&format);
			}
			return DeeRT_ErrUnpackErrorEx(self, argc_min, argc_max, argc);
		}
err_iter:
		Dee_Decref(iterator);
		return -1;
	}	break;

		/* Ignore this object. */
	case 'n':
	case '-':
		break;

	case 'o': /* Store the object as-is. */
		LOCAL_OUTPUT(DeeObject *, __ALIGNOF_POINTER__, self);
		break;

	case 'S': { /* Store a fixed-width string. */
		void const *str;
		if (DeeObject_AssertTypeExact(self, &DeeString_Type))
			goto err;
		if likely(format[0] == 's') {
			++format;
			str = DeeString_As1Byte(self);
		} else if (format[0] == '1') {
			ASSERTF(format[1] == '6' && format[2] == 's',
			        "Invalid format: `%s' (`%s')",
			        format, *p_format);
			format += 3;
			str = DeeString_As2Byte(self);
		} else {
			ASSERTF(format[0] == '3' && format[1] == '2' && format[2] == 's',
			        "Invalid format: `%s' (`%s')",
			        format, *p_format);
			format += 3;
			str = DeeString_As4Byte(self);
		}
		if unlikely(!str)
			goto err;
		LOCAL_OUTPUT(void const *, __ALIGNOF_POINTER__, str);
	}	break;

	case 'U': { /* Store a unicode string. */
		void const *str;
		if (DeeObject_AssertTypeExact(self, &DeeString_Type))
			goto err;
		if likely(format[0] == 's') {
			++format;
			str = DeeString_AsUtf8(self);
		} else if (format[0] == '1') {
			ASSERTF(format[1] == '6' && format[2] == 's',
			        "Invalid format: `%s' (`%s')",
			        format, *p_format);
			format += 3;
			str = DeeString_AsUtf16(self, Dee_STRING_ERROR_FSTRICT);
		} else {
			ASSERTF(format[0] == '3' && format[1] == '2' && format[2] == 's',
			        "Invalid format: `%s' (`%s')",
			        format, *p_format);
			format += 3;
			str = DeeString_AsUtf32(self);
		}
		if unlikely(!str)
			goto err;
		LOCAL_OUTPUT(void const *, __ALIGNOF_POINTER__, str);
	}	break;

	case 's': /* Store a string. */
		if (DeeObject_AssertTypeExact(self, &DeeString_Type))
			goto err;
		LOCAL_OUTPUT(char const *, __ALIGNOF_POINTER__, DeeString_STR(self));
		break;

	case '$': { /* Store a string, including its length. */
		void const *str;
		if (DeeObject_AssertTypeExact(self, &DeeString_Type))
			goto err;
		if (*format == 'U') {
			++format;
			if likely(format[0] == 's') {
				++format;
				str = DeeString_AsUtf8(self);
			} else if (format[0] == '1') {
				ASSERTF(format[1] == '6' && format[2] == 's',
				        "Invalid format: `%s' (`%s')",
				        format, *p_format);
				format += 3;
				str = DeeString_AsUtf16(self, Dee_STRING_ERROR_FSTRICT);
			} else {
				ASSERTF(format[0] == '3' && format[1] == '2' && format[2] == 's',
				        "Invalid format: `%s' (`%s')",
				        format, *p_format);
				format += 3;
				str = DeeString_AsUtf32(self);
			}
			if unlikely(!str)
				goto err;
		} else if (*format == 'S') {
			++format;
			if likely(format[0] == 's') {
				++format;
				str = DeeString_As1Byte(self);
			} else if (format[0] == '1') {
				ASSERTF(format[1] == '6' && format[2] == 's',
				        "Invalid format: `%s' (`%s')",
				        format, *p_format);
				format += 3;
				str = DeeString_As2Byte(self);
			} else {
				ASSERTF(format[0] == '3' && format[1] == '2' && format[2] == 's',
				        "Invalid format: `%s' (`%s')",
				        format, *p_format);
				format += 3;
				str = DeeString_As4Byte(self);
			}
			if unlikely(!str)
				goto err;
		} else if (*format == 'l') {
			ASSERTF(format[1] == 's', "Invalid format: `%s' (`%s')", format, *p_format);
			format += 2;
			str = DeeString_AsWide(self);
			if unlikely(!str)
				goto err;
		} else {
			ASSERTF(*format == 's', "Invalid format: `%s' (`%s')", format, *p_format);
			++format;
			str = DeeString_STR(self);
		}
		LOCAL_OUTPUT(size_t, __ALIGNOF_SIZE_T__, WSTR_LENGTH(str));
		LOCAL_OUTPUT(void const *, __ALIGNOF_POINTER__, str);
	}	break;

#ifdef __LONGDOUBLE
	case 'L':
#endif /* __LONGDOUBLE */
	{
		double value;
#ifdef __LONGDOUBLE
		ASSERTF(*format == 'D', "Invalid format: `%s' (`%s')", format, *p_format);
		++format;
		ATTR_FALLTHROUGH
#endif /* __LONGDOUBLE */
	case 'f':
	case 'D':
		if (DeeObject_AsDouble(self, &value))
			goto err;
		if (format[-1] == 'f') {
			LOCAL_OUTPUT(float, __ALIGNOF_FLOAT__, (float)value);
		} else
#ifdef __LONGDOUBLE
		if (format[-2] == 'L') {
			LOCAL_OUTPUT(__LONGDOUBLE, __ALIGNOF_LONG_DOUBLE__, (__LONGDOUBLE)value);
		} else
#endif /* __LONGDOUBLE */
		{
			LOCAL_OUTPUT(double, __ALIGNOF_DOUBLE__, (double)value);
		}
	}	break;

	case 'b': {
		int temp; /* Bool */
		temp = DeeObject_Bool(self);
		if unlikely(temp < 0)
			return temp;
		LOCAL_OUTPUT(bool, __ALIGNOF_BOOL__, !!temp);
	}	break;

	/* Int */
	{
		int length, temp;
#ifndef LEN_INT_IB1
#define LEN_INT_IB1  0
#define LEN_INT_IB2  1
#define LEN_INT_IB4  2
#define LEN_INT_IB8  3
#define LEN_INT_IB16 4
#define LEN_INT2(n)  LEN_INT_IB##n
#define LEN_INT(n)   LEN_INT2(n)
#endif /* !LEN_INT_IB1 */
	case 'c':
		length = LEN_INT_IB1;
		goto do_integer_format;
	case 'l':
		if (*format == 's') {
			void const *str;
			/* Wide-character string. */
			if (DeeObject_AssertTypeExact(self, &DeeString_Type))
				goto err;
			str = DeeString_AsWide(self);
			if unlikely(!str)
				goto err;
			LOCAL_OUTPUT(void const *, __ALIGNOF_POINTER__, str);
			break;
		}
		ATTR_FALLTHROUGH
	case 'I':
	case 'h':
	case 'd':
	case 'u':
	case 'i':
	case 'x':
		length = LEN_INT(__SIZEOF_INT__);
do_integer_format:
		if (format[-1] == 'I') {
			if (*format == '8') {
				length = LEN_INT(1);
				format += 2;
			} else if (*format == '1') {
				if (format[1] == '2') {
					ASSERTF(format[2] == '8', "Invalid format: `%s' (`%s')", format, *p_format);
					length = LEN_INT(16);
					format += 4;
				} else {
					ASSERTF(format[1] == '6', "Invalid format: `%s' (`%s')", format, *p_format);
					length = LEN_INT(2);
					format += 3;
				}
			} else if (*format == '3') {
				ASSERTF(format[1] == '2', "Invalid format: `%s' (`%s')", format, *p_format);
				length = LEN_INT(4);
				format += 3;
			} else if (*format == '6') {
				ASSERTF(format[1] == '4', "Invalid format: `%s' (`%s')", format, *p_format);
				length = LEN_INT(8);
				format += 3;
			} else {
				length = LEN_INT(__SIZEOF_SIZE_T__);
				format += 1;
			}
		} else if (format[-1] == 'h') {
			length = LEN_INT(__SIZEOF_SHORT__);
			if (*format == 'h') {
				++format;
				length = LEN_INT(__SIZEOF_CHAR__);
			}
			++format;
		} else if (format[-1] == 'l') {
			length = LEN_INT(__SIZEOF_LONG__);
			if (*format == 'l') {
				++format;
#ifdef __SIZEOF_LONG_LONG__
				length = LEN_INT(__SIZEOF_LONG_LONG__);
#else /* __SIZEOF_LONG_LONG__ */
				length = LEN_INT(8);
#endif /* !__SIZEOF_LONG_LONG__ */
			}
			++format;
		}
		if (format[-1] == 'd' || format[-1] == 'i') {
			switch (length) { /* signed int */
			case LEN_INT_IB1:
				LOCAL_OUTPUT_PTR_PREALIGN(int8_t, __ALIGNOF_INT8__);
				temp = DeeObject_AsInt8(self, LOCAL_OUTPUT_PTR(int8_t));
				break;
			case LEN_INT_IB2:
				LOCAL_OUTPUT_PTR_PREALIGN(int16_t, __ALIGNOF_INT16__);
				temp = DeeObject_AsInt16(self, LOCAL_OUTPUT_PTR(int16_t));
				break;
			case LEN_INT_IB4:
				LOCAL_OUTPUT_PTR_PREALIGN(int32_t, __ALIGNOF_INT32__);
				temp = DeeObject_AsInt32(self, LOCAL_OUTPUT_PTR(int32_t));
				break;
			case LEN_INT_IB8:
				LOCAL_OUTPUT_PTR_PREALIGN(int64_t, __ALIGNOF_INT64__);
				temp = DeeObject_AsInt64(self, LOCAL_OUTPUT_PTR(int64_t));
				break;
			case LEN_INT_IB16:
				LOCAL_OUTPUT_PTR_PREALIGN(Dee_int128_t, __ALIGNOF_INT128__);
				temp = DeeObject_AsInt128(self, LOCAL_OUTPUT_PTR(Dee_int128_t));
				break;
			default: __builtin_unreachable();
			}
		} else if (format[-1] == 'u') {
parse_unsigned_int:
			switch (length) { /* unsigned int */
			case LEN_INT_IB1:
				LOCAL_OUTPUT_PTR_PREALIGN(uint8_t, __ALIGNOF_INT8__);
				temp = DeeObject_AsUInt8(self, LOCAL_OUTPUT_PTR(uint8_t));
				break;
			case LEN_INT_IB2:
				LOCAL_OUTPUT_PTR_PREALIGN(uint16_t, __ALIGNOF_INT16__);
				temp = DeeObject_AsUInt16(self, LOCAL_OUTPUT_PTR(uint16_t));
				break;
			case LEN_INT_IB4:
				LOCAL_OUTPUT_PTR_PREALIGN(uint32_t, __ALIGNOF_INT32__);
				temp = DeeObject_AsUInt32(self, LOCAL_OUTPUT_PTR(uint32_t));
				break;
			case LEN_INT_IB8:
				LOCAL_OUTPUT_PTR_PREALIGN(uint64_t, __ALIGNOF_INT64__);
				temp = DeeObject_AsUInt64(self, LOCAL_OUTPUT_PTR(uint64_t));
				break;
			case LEN_INT_IB16:
				LOCAL_OUTPUT_PTR_PREALIGN(Dee_uint128_t, __ALIGNOF_INT128__);
				temp = DeeObject_AsUInt128(self, LOCAL_OUTPUT_PTR(Dee_uint128_t));
				break;
			default: __builtin_unreachable();
			}
		} else if (format[-1] == 'x') {
			switch (length) { /* unsigned int */
			case LEN_INT_IB1:
				LOCAL_OUTPUT_PTR_PREALIGN(uint8_t, __ALIGNOF_INT8__);
				temp = DeeObject_AsUInt8M1(self, LOCAL_OUTPUT_PTR(uint8_t));
				break;
			case LEN_INT_IB2:
				LOCAL_OUTPUT_PTR_PREALIGN(uint16_t, __ALIGNOF_INT16__);
				temp = DeeObject_AsUInt16M1(self, LOCAL_OUTPUT_PTR(uint16_t));
				break;
			case LEN_INT_IB4:
				LOCAL_OUTPUT_PTR_PREALIGN(uint32_t, __ALIGNOF_INT32__);
				temp = DeeObject_AsUInt32M1(self, LOCAL_OUTPUT_PTR(uint32_t));
				break;
			case LEN_INT_IB8:
				LOCAL_OUTPUT_PTR_PREALIGN(uint64_t, __ALIGNOF_INT64__);
				temp = DeeObject_AsUInt64M1(self, LOCAL_OUTPUT_PTR(uint64_t));
				break;
			case LEN_INT_IB16:
				LOCAL_OUTPUT_PTR_PREALIGN(Dee_uint128_t, __ALIGNOF_INT128__);
				temp = DeeObject_AsUInt128M1(self, LOCAL_OUTPUT_PTR(Dee_uint128_t));
				break;
			default: __builtin_unreachable();
			}
		} else {
			ASSERTF(format[-1] == 'c', "Invalid format: `%s' (`%s')", format, *p_format);
			/* Unicode character (either a single-character string/Bytes object, or an integer). */
			if (DeeString_Check(self)) {
				uint32_t ch;
				if (DeeString_WLEN(self) != 1)
					err_expected_single_character_string(self);
				ch = DeeString_GetChar(self, 0);
				switch (length) { /* unsigned int */

				case LEN_INT_IB1:
					if unlikely(ch > UINT8_MAX) {
						DeeRT_ErrIntegerOverflowU32(ch, UINT8_MAX);
						goto err;
					}
					LOCAL_OUTPUT(uint8_t, __ALIGNOF_INT8__, (uint8_t)ch);
					break;

				case LEN_INT_IB2:
					if unlikely(ch > UINT16_MAX) {
						DeeRT_ErrIntegerOverflowU32(ch, UINT16_MAX);
						goto err;
					}
					LOCAL_OUTPUT(uint16_t, __ALIGNOF_INT16__, (uint16_t)ch);
					break;

				case LEN_INT_IB4:
					LOCAL_OUTPUT(uint32_t, __ALIGNOF_INT32__, (uint32_t)ch);
					break;

				case LEN_INT_IB8:
					LOCAL_OUTPUT(uint64_t, __ALIGNOF_INT64__, (uint64_t)ch);
					break;

				case LEN_INT_IB16: {
					Dee_uint128_t *p;
					LOCAL_OUTPUT_PTR_PREALIGN(Dee_uint128_t, __ALIGNOF_INT128__);
					p = LOCAL_OUTPUT_PTR(Dee_uint128_t);
					__hybrid_uint128_set32(*p, ch);
				}	break;

				default: __builtin_unreachable();
				}
				temp = 0;
			} else {
				goto parse_unsigned_int;
			}
		}
		if unlikely(temp < 0)
			return temp;
	}	break;

	default:
		ASSERTF(!*format, "Invalid format: `%s' (`%s')", format, *p_format);
		break;
	}
	*p_format = format;
	return 0;
err:
	return -1;
}


PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) char const *DCALL
LOCAL_Dee_VPUnpackfSkip(char const *__restrict format,
                        LOCAL_PARAM_P_OUT) {
again:
	switch (*format++) {

	case ',':
		goto again;

	case '(':
		/* Unpack a sequence. */
		while (*format && *format != ')')
			format = LOCAL_Dee_VPUnpackfSkip(format, LOCAL_ARG_P_OUT);
		break;

		/* Ignore this object. */
	case 'n':
	case '-':
		break;

#ifdef __LONGDOUBLE
	case 'L':
		ASSERTF(*format == 'D', "Invalid format: `%s'", format);
		++format;
#endif /* __LONGDOUBLE */
#ifdef DEFINE_DeeArg_Unpack
#ifdef __LONGDOUBLE
		ATTR_FALLTHROUGH
#endif /* __LONGDOUBLE */
	case 'D':
	case 'f':
	case 'b':
		LOCAL_SKIP(void, ~);
		break;
#else /* DEFINE_DeeArg_Unpack */
#ifdef __LONGDOUBLE
		LOCAL_SKIP(long double, __ALIGNOF_LONG_DOUBLE__);
		break;
#endif /* __LONGDOUBLE */
	case 'D':
		LOCAL_SKIP(double, __ALIGNOF_DOUBLE__);
		break;
	case 'f':
		LOCAL_SKIP(float, __ALIGNOF_FLOAT__);
		break;
	case 'b':
		LOCAL_SKIP(bool, __ALIGNOF_BOOL__);
		break;
#endif /* !DEFINE_DeeArg_Unpack */

	case 'U': /* Store a unicode string. */
	case 'S': /* Store a fixed-width string. */
		if likely(format[0] == 's') {
			++format;
		} else {
			ASSERTF((format[0] == '1' && format[1] == '6' && format[2] == 's') ||
			        (format[0] == '3' && format[1] == '2' && format[2] == 's'),
			        "Invalid format: `%s'", format);
			format += 3;
		}
		ATTR_FALLTHROUGH
	case 'o':
	case 's':
		LOCAL_SKIP(void *, __ALIGNOF_POINTER__);
		break;

	case '$': /* Store a string, including its length. */
		if (*format == 'S' || *format == 'U') {
			++format;
			if likely(format[0] == 's') {
				++format;
			} else {
				ASSERTF((format[0] == '1' && format[1] == '6' && format[2] == 's') ||
				        (format[0] == '3' && format[1] == '2' && format[2] == 's'),
				        "Invalid format: `%s'", format);
				format += 3;
			}
		} else if (*format == 'l') {
			ASSERTF(format[1] == 's', "Invalid format: `%s'", format);
			format += 2;
		} else {
			ASSERTF(*format == 's', "Invalid format: `%s'", format);
			++format;
		}
		LOCAL_SKIP(size_t, __ALIGNOF_SIZE_T__);
		LOCAL_SKIP(void *, __ALIGNOF_POINTER__);
		break;

	/* Int */
	case 'l':
		if (*format == 's') {
			LOCAL_SKIP(Dee_wchar_t *, __ALIGNOF_POINTER__);
			break;
		}
		ATTR_FALLTHROUGH
	case 'I':
	case 'h':
	case 'd':
	case 'u':
	case 'i':
	case 'x': {
#ifdef DEFINE_DeeArg_Unpack
	case 'c':
#define LOCAL_SETLENGTH(x) (void)0
#else /* DEFINE_DeeArg_Unpack */
#define LOCAL_SETLENGTH(x) (void)(length = (x))
		int length;
		length = LEN_INT(__SIZEOF_INT__);
		__IF0 {
	case 'c':
			length = LEN_INT_IB1;
		}
#endif /* !DEFINE_DeeArg_Unpack */
		if (format[-1] == 'I') {
			if (*format == '8') {
				LOCAL_SETLENGTH(LEN_INT(1));
				format += 2;
			} else if (*format == '1') {
				if (format[1] == '2') {
					ASSERTF(format[2] == '8', "Invalid format: `%s'", format);
					LOCAL_SETLENGTH(LEN_INT(16));
					format += 4;
				} else {
					ASSERTF(format[1] == '6', "Invalid format: `%s'", format);
					LOCAL_SETLENGTH(LEN_INT(2));
					format += 3;
				}
			} else if (*format == '3') {
				ASSERTF(format[1] == '2', "Invalid format: `%s'", format);
				LOCAL_SETLENGTH(LEN_INT(4));
				format += 3;
			} else if (*format == '6') {
				ASSERTF(format[1] == '4', "Invalid format: `%s'", format);
				LOCAL_SETLENGTH(LEN_INT(8));
				format += 3;
			} else {
				LOCAL_SETLENGTH(LEN_INT(__SIZEOF_SIZE_T__));
				format += 1;
			}
		} else if (format[-1] == 'h') {
			LOCAL_SETLENGTH(LEN_INT(__SIZEOF_SHORT__));
			if (*format == 'h') {
				++format;
				LOCAL_SETLENGTH(LEN_INT(__SIZEOF_CHAR__));
			}
			++format;
		} else if (format[-1] == 'l') {
			LOCAL_SETLENGTH(LEN_INT(__SIZEOF_LONG__));
			if (*format == 'l') {
				++format;
#ifdef __SIZEOF_LONG_LONG__
				LOCAL_SETLENGTH(LEN_INT(__SIZEOF_LONG_LONG__));
#else /* __SIZEOF_LONG_LONG__ */
				LOCAL_SETLENGTH(LEN_INT(8));
#endif /* !__SIZEOF_LONG_LONG__ */
			}
			++format;
		}
#undef LOCAL_SETLENGTH
#ifdef DEFINE_DeeArg_Unpack
		LOCAL_SKIP(void *, __ALIGNOF_POINTER__);
#else /* DEFINE_DeeArg_Unpack */
		switch (length) { /* unsigned int */
		case LEN_INT_IB1: LOCAL_SKIP(uint8_t, __ALIGNOF_INT8__); break;
		case LEN_INT_IB2: LOCAL_SKIP(uint16_t, __ALIGNOF_INT16__); break;
		case LEN_INT_IB4: LOCAL_SKIP(uint32_t, __ALIGNOF_INT32__); break;
		case LEN_INT_IB8: LOCAL_SKIP(uint64_t, __ALIGNOF_INT64__); break;
		case LEN_INT_IB16: LOCAL_SKIP(Dee_uint128_t, __ALIGNOF_INT128__); break;
		default: __builtin_unreachable();
		}
#endif /* !DEFINE_DeeArg_Unpack */
	}	break;

	default:
		ASSERTF(!*format, "Invalid format: `%s'", format);
		break;
	}
	return format;
}



/* An extension to `Dee_Unpackf', explicitly for unpacking elements from function arguments.
 * Format language syntax:
 *     using Dee_Unpackf::object;
 *     __main__   ::= [(object  // Process regular objects, writing values to pointers passed through varargs.
 *                    | '|'     // Marker: The remainder of the format is optional.
 *                      )...]
 *                    [':' <function_name>] // Optional, trailing function name (Used in error messages)
 *     ;
 * Example usage:
 * >> // function my_function(int a, int b, int c = 5) -> int;
 * >> // @return: * : The sum of `a', `b' and `c'
 * >> PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
 * >> my_function(DeeObject *UNUSED(self),
 * >>             size_t argc, DeeObject *const *argv) {
 * >>     int a, b, c = 5;
 * >>     if (DeeArg_Unpack(argc, argv, "dd|d:my_function", &a, &b, &c))
 * >>         return NULL;
 * >>     return DeeInt_NewInt(a + b + c);
 * >> }
 */
PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((3)) int
(DCALL LOCAL_DeeArg_VUnpack)(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv,
                             char const *__restrict format, LOCAL_PARAM_OUT) {
	int temp;
#ifdef DEFINE_DeeArg_Unpack
	struct va_list_struct *p_args = (struct va_list_struct *)VALIST_ADDR(args);
#else /* DEFINE_DeeArg_Unpack */
	void **p_out = &out;
#ifdef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
	uintptr_t out_align = 0;
#define p_out_align &out_align
#endif /* COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#endif /* !DEFINE_DeeArg_Unpack */
	char const *fmt_start = format;
	bool is_optional = false;
	DeeObject *const *iter = argv;
	DeeObject *const *end = argv + argc;
	for (;;) {
		if (*format == '|') {
			is_optional = true;
			++format;
		}
		if (iter >= end) {
			if (!is_optional && *format && *format != ':')
				goto invalid_argc;
			break;
		}
		if (!*format || *format == ':')
			goto invalid_argc;
		temp = LOCAL_Dee_VPUnpackf(*iter++, (char const **)&format, LOCAL_ARG_P_OUT);
		if unlikely(temp)
			return temp;
	}
	return 0;
invalid_argc:
	{
		size_t argc_min, argc_max;
		format   = fmt_start;
		argc_min = argc_max = count_unpack_args((char const **)&format);
		if (*format == '|') {
			++format;
			argc_max += count_unpack_args((char const **)&format);
		}
		if (*format == ':') {
			++format;
		} else {
			format = NULL;
		}
		err_invalid_argc(format, argc, argc_min, argc_max);
	}
	return -1;
#undef p_out_align
}



#ifndef DEFINED_kwds_findstr
#define DEFINED_kwds_findstr
PRIVATE NONNULL((1, 2)) size_t DCALL
kwds_findstr(DeeKwdsObject *__restrict self,
             char const *__restrict name,
             Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	perturb = i = hash & self->kw_mask;
	for (;; i = (i << 2) + i + perturb + 1, perturb >>= 5) {
		struct kwds_entry *entry;
		entry = &self->kw_map[i & self->kw_mask];
		if (!entry->ke_name)
			break;
		if (entry->ke_hash != hash)
			continue;
		if (strcmp(DeeString_STR(entry->ke_name), name) != 0)
			continue;
		return entry->ke_index;
	}
	return (size_t)-1;
}
#endif /* !DEFINED_kwds_findstr */

PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) int
(DCALL LOCAL_DeeArg_VUnpackKw)(size_t argc, DeeObject *const *argv,
                               DeeObject *kw, struct Dee_keyword *__restrict kwlist,
                               char const *__restrict format, LOCAL_PARAM_OUT) {
	char const *fmt_start;
	size_t kw_argc;
	bool is_optional;
	int temp;
#ifdef DEFINE_DeeArg_Unpack
	struct va_list_struct *p_args;
#else /* DEFINE_DeeArg_Unpack */
	void **p_out;
#ifdef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
	uintptr_t out_align;
#define p_out_align &out_align
#endif /* COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#endif /* !DEFINE_DeeArg_Unpack */
	if (!kw) /* Without arguments, do a regular unpack. */
		return LOCAL_DeeArg_VUnpack(argc, argv, format, LOCAL_ARG_OUT);
	fmt_start   = format;
	is_optional = false;
#ifdef DEFINE_DeeArg_Unpack
	p_args = (struct va_list_struct *)VALIST_ADDR(args);
#else /* DEFINE_DeeArg_Unpack */
	p_out = &out;
#ifdef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
	out_align = 0;
#endif /* COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#endif /* !DEFINE_DeeArg_Unpack */
	if (DeeKwds_Check(kw)) {
		/* Indirect keyword list. */
		size_t pos_argc;
		kw_argc = ((DeeKwdsObject *)kw)->kw_size;
		if unlikely(kw_argc > argc)
			return err_keywords_bad_for_argc((DeeKwdsObject *)kw, argc, argv);

		/* Parse all argument not passed through keywords. */
		pos_argc = argc - kw_argc;
		while (pos_argc--) {
			if (*format == '|') {
				is_optional = true;
				++format;
			}
			if (!*format || *format == ':') {
				ASSERTF(!kwlist->k_name, "Keyword list too long");
				goto err_invalid_argc; /* Too many arguments. */
			}
			temp = LOCAL_Dee_VPUnpackf(*argv++, (char const **)&format, LOCAL_ARG_P_OUT);
			if unlikely(temp)
				return temp;
			ASSERTF(kwlist->k_name, "Keyword list too short");
			++kwlist;
		}

		/* All remaining arguments are passed through
		 * keywords found in `kwlist .. kwlist + x'. */
		for (;;) {
			Dee_hash_t keyword_hash;
			size_t kwd_index;
			if (*format == '|') {
				is_optional = true;
				++format;
			}
			if (!*format || *format == ':') {
				ASSERTF(!kwlist->k_name, "Keyword list too long");
				if (kw_argc) {
					/* TODO: This can also happen when:
					 * >> function foo(x, bar = none);
					 * >> foo(x: 10, baz: 20);
					 * In this case we should do a fuzzy match and
					 * warn the caller that instead of `baz', they
					 * probably meant `bar' */
					goto err_invalid_argc; /* Too many arguments. */
				}
				break;
			}

			/* Find the matching positional argument. */
			ASSERTF(kwlist->k_name, "Keyword list too short");
			keyword_hash = kwlist->k_hash;
			if (keyword_hash == (Dee_hash_t)-1) {
				/* Lazily calculate the hash the first time around. */
				keyword_hash   = Dee_HashStr(kwlist->k_name);
				kwlist->k_hash = keyword_hash;
			}
			kwd_index = kwds_findstr((DeeKwdsObject *)kw,
			                         kwlist->k_name,
			                         keyword_hash);
			if unlikely(kwd_index == (size_t)-1) {
				/* Argument not given. */
				if (!is_optional) {
					size_t argc_min, argc_max;
					argc_min = argc_max = count_unpack_args((char const **)&format);
					if (*format == '|') {
						++format;
						argc_max += count_unpack_args((char const **)&format);
					}
					if (*format == ':') {
						++format;
					} else {
						format = NULL;
					}
					return err_invalid_argc_missing_kw(kwlist->k_name,
					                                   format,
					                                   argc,
					                                   argc_min,
					                                   argc_max);
				}

				/* The argument is optional, but not given. -> So just skip this one! */
				format = LOCAL_Dee_VPUnpackfSkip(format, LOCAL_ARG_P_OUT);
			} else {
				/* All right! we've got the argument! */
				ASSERT(kwd_index < ((DeeKwdsObject *)kw)->kw_size);
				temp = LOCAL_Dee_VPUnpackf(argv[kwd_index], (char const **)&format, LOCAL_ARG_P_OUT);
				if unlikely(temp)
					return temp;
				ASSERT(kw_argc != 0);
				if (!--kw_argc) {
					if (is_optional ||
					    (*format == '|' || *format == ':' || *format == '\0'))
						break;

					/* TODO: This can also happen when:
					 * >> function foo(x, bar);
					 * >> foo(x: 10, baz: 20);
					 * In this case we should do a fuzzy match and
					 * warn the caller that instead of `baz', they
					 * probably meant `bar' */
					goto err_invalid_argc; /* Too few arguments. */
				}
			}
			++kwlist;
		}
		return 0; /* Done! */
	}

	/* Keyword arguments are given, but aren't a `DeeKwds_Type' object.
	 * In this situation, we're supposed to interpret them as a mapping-like object!
	 * But first off: parse all the positional argument! */
	while (argc--) {
		if (*format == '|') {
			is_optional = true;
			++format;
		}
		if (!*format || *format == ':') {
			ASSERTF(!kwlist->k_name, "Keyword list too long");
			goto err_invalid_argc; /* Too many arguments. */
		}
		temp = LOCAL_Dee_VPUnpackf(*argv++, (char const **)&format, LOCAL_ARG_P_OUT);
		if unlikely(temp)
			return temp;
		ASSERTF(kwlist->k_name, "Keyword list too short");
		++kwlist;
	}

	/* Now with positional arguments out of the way, move on to the named arguments. */
	kw_argc = 0;
	for (;;) {
		Dee_hash_t keyword_hash;
		DeeObject *keyword_value;
		if (*format == '|') {
			is_optional = true;
			++format;
		}
		if (!*format || *format == ':') {
			ASSERTF(!kwlist->k_name, "Keyword list too long");
			break; /* End of argument list. */
		}

		/* Find the matching positional argument. */
		ASSERTF(kwlist->k_name, "Keyword list too short");
		keyword_hash = kwlist->k_hash;
		if (keyword_hash == (Dee_hash_t)-1) {
			/* Lazily calculate the hash the first time around. */
			keyword_hash   = Dee_HashStr(kwlist->k_name);
			kwlist->k_hash = keyword_hash;
		}
		keyword_value = DeeKw_TryGetItemNRStringHash(kw, kwlist->k_name, keyword_hash);
		if unlikely(keyword_value == ITER_DONE) {
			/* Argument not given. */
			if (!is_optional) {
				size_t argc_min, argc_max;
				argc_min = argc_max = count_unpack_args((char const **)&format);
				if (*format == '|') {
					++format;
					argc_max += count_unpack_args((char const **)&format);
				}
				if (*format == ':') {
					++format;
				} else {
					format = NULL;
				}
				return err_invalid_argc_missing_kw(kwlist->k_name,
				                                   format,
				                                   argc,
				                                   argc_min,
				                                   argc_max);
			}

			/* The argument is optional, but not given. -> So just skip this one! */
			format = LOCAL_Dee_VPUnpackfSkip(format, LOCAL_ARG_P_OUT);
		} else {
			/* All right! we've got the argument! */
			temp = LOCAL_Dee_VPUnpackf(keyword_value, (char const **)&format, LOCAL_ARG_P_OUT);
			if unlikely(temp)
				return temp;
			++kw_argc;
		}
		++kwlist;
	}

	/* Make sure that the argument list doesn't contain argument that went unused. */
	{
		size_t kw_size = DeeObject_Size(kw);
		if unlikely(kw_size == (size_t)-1)
			goto err;
		if (kw_argc != kw_size)
			goto err_invalid_argc;
	}
	return 0;
	{
		size_t argc_min, argc_max;
err_invalid_argc:
		format   = fmt_start;
		argc_min = argc_max = count_unpack_args((char const **)&format);
		if (*format == '|') {
			++format;
			argc_max += count_unpack_args((char const **)&format);
		}
		if (*format == ':') {
			++format;
		} else {
			format = NULL;
		}
		err_invalid_argc(format, argc, argc_min, argc_max);
	}
err:
	return -1;
#undef p_out_align
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL LOCAL_Dee_VUnpackf)(DeeObject *__restrict self,
                           char const *__restrict format,
                           LOCAL_PARAM_OUT) {
#ifdef DEFINE_DeeArg_Unpack
	return Dee_VPUnpackf(self, (char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
#else /* DEFINE_DeeArg_Unpack */
	return Dee_PUnpackStruct(self, (char const **)&format, &out);
#endif /* !DEFINE_DeeArg_Unpack */
}


#ifdef DEFINE_DeeArg_UnpackStruct
#ifdef COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL Dee_PUnpackStruct)(DeeObject *__restrict self,
                          char const **__restrict p_format,
                          void **p_out) {
	uintptr_t out_align = (uintptr_t)*p_out;
	return pab_Dee_PUnpackStruct(self, p_format, p_out, &out_align);
}
#endif /* COMPILER_HAVE_PISSASS_BUGGY_STACK_ALIGNMENT */
#endif /* DEFINE_DeeArg_UnpackStruct */


#ifdef DEFINE_DeeArg_Unpack
PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((3)) int
(DeeArg_Unpack)(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv,
                char const *__restrict format, ...) {
	int result;
	va_list args;
	va_start(args, format);
	result = DeeArg_VUnpack(argc, argv, format, args);
	va_end(args);
	return result;
}


PUBLIC WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) int
(DeeArg_UnpackKw)(size_t argc, DeeObject *const *argv,
                  DeeObject *kw, struct Dee_keyword *__restrict kwlist,
                  char const *__restrict format, ...) {
	int result;
	va_list args;
	va_start(args, format);
	result = DeeArg_VUnpackKw(argc, argv, kw, kwlist, format, args);
	va_end(args);
	return result;
}


/* Unpack values from an object.
 * Format language syntax:
 *     __main__   ::= object;
 *     object     ::= ('n' | '-')          // Ignore / skip this object. (Do not advance `va_arg')
 *                  | ref_object           // `va_arg(DeeObject **)'
 *                  | ref_int              //
 *                  | ref_float            //
 *                  | ref_bool             //
 *                  | ref_str              // `char const **'
 *                  | '(' [objects...] ')' // Enumerate elements of a sequence
 *     ;
 *     objects    ::= [(object   // Parse some object from the sequence.
 *                    | ','      // `,' is simply ignored, but can be used to prevent ambiguity.
 *                    | '|'      // Any following objects are optional (va_arg() is still invoked, but non-present elements are skipped)
 *                      )...];
 *     ref_object ::= 'o'; // `va_arg(DeeObject **)'
 *     ref_int    ::= [ref_intlen] ('d' | 'u' | 'i' | 'x'); // `u' and `x' read unsigned integers ("x" uses *M1)
 *     ref_str    ::= ['$']      // `va_arg(size_t *)' (Store the length of the string)
 *                  | 'l' 's'    // `va_arg(wchar_t const **)'
 *                  | 'U16' 's'  // `va_arg(uint16_t const **)'
 *                  | 'U32' 's'  // `va_arg(uint32_t const **)'
 *                  | 's'        // `va_arg(char const **)'
 *                  ;
 *     ref_intlen ::= 'I' ['8' | '16' | '32' | '64'] // Fixed-length / sizeof(size_t)
 *                  | 'hh' // `va_arg(char *)'
 *                  | 'h'  // `va_arg(short *)'
 *                  | ''   // `va_arg(int *)' (Default when nothing else was given)
 *                  | 'l'  // `va_arg(long *)'
 *                  | 'll' // `va_arg(long long *)' (__(U)LONGLONG *)
 *     ;
 *     ref_float  ::= 'f'  // `va_arg(float *)'
 *                  | 'D'  // `va_arg(double *)'
 *                  | 'LD' // `va_arg(long double *)'
 *     ;
 *     ref_bool   ::= 'b'; // `va_arg(bool)'
 */
PUBLIC WUNUSED NONNULL((1, 2)) int
(Dee_Unpackf)(DeeObject *__restrict self,
              char const *__restrict format, ...) {
	int result;
	struct va_list_struct args;
	va_start(args.vl_ap, format);
#ifdef CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY
	/* Compile-time assert that our config is correct.
	 * -> If it isn't then this breaks, since you can't
	 *    assign arrays to each other in C. */
	args.vl_ap = args.vl_ap;
#endif /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
	result = Dee_VPUnpackf(self, (char const **)&format, &args);
	va_end(args.vl_ap);
	return result;
}
#endif /* DEFINE_DeeArg_Unpack */

#undef LOCAL_Dee_VPUnpackf
#undef LOCAL_Dee_VPUnpackfSkip
#undef LOCAL_DeeArg_VUnpack
#undef LOCAL_DeeArg_VUnpackKw
#undef LOCAL_Dee_VUnpackf
#undef LOCAL_PARAM_P_OUT
#undef LOCAL_PARAM_OUT
#undef LOCAL_ARG_P_OUT
#undef LOCAL_ARG_OUT
#undef LOCAL_VAL_P_OUT
#undef LOCAL_OUTPUT
#undef LOCAL_OUTPUT_PTR
#undef LOCAL_OUTPUT_PTR_PREALIGN
#undef LOCAL_SKIP

DECL_END

#undef DEFINE_DeeArg_Unpack
#undef DEFINE_DeeArg_UnpackStruct
