/* Copyright (c) 2016 - deemon by Griefer@Work                                    *
 *                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy   *
 * of this software and associated documentation files (the "Software"), to deal  *
 * in the Software without restriction, including without limitation the rights   *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
 * copies of the Software, and to permit persons to whom the Software is          *
 * furnished to do so, subject to the following conditions:                       *
 *                                                                                *
 * The above copyright notice and this permission notice shall be included in all *
 * copies or substantial portions of the Software.                                *
 *                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
 * SOFTWARE.                                                                      *
 */
#pragma once
#ifndef __TPP_VERSION__
#error This header library is only meant to be used with TPP!
#endif
#ifndef __TPP_EVAL
#error This library requires, that tpp was compiled \
       with at least "TPP_CONFIG_HAVE___TPP_EVAL" defined
#endif /* !__TPP_EVAL */
#ifndef __has_feature
#define __has_feature(x) 0
#endif /* !__has_feature */
#define __TPP_BASIC_CAT2(a,b)         a ## b
#define __TPP_BASIC_CAT(a,b)          __TPP_BASIC_CAT2(a,b)
#define __TPP_FORCE_EXPAND(...)       __VA_ARGS__
#define __TPP_BASIC_EXPAND_TUPLE(tpl) __TPP_FORCE_EXPAND tpl
#define __TPP_EAT_ARGS(...)           /* nothing */
#ifndef __pragma
#define __pragma(...) _Pragma(TPP_STR_NOEXPAND(__VA_ARGS__))
#endif /* __pragma */

#if __has_feature(__tpp_pragma_warning__) && __TPP_VERSION__ >= 105
#define __TPP_SUPPRESS_WARNING(id) __pragma(warning(suppress: id))
#else
#define __TPP_SUPPRESS_WARNING(id) /* nothing */
#endif

//////////////////////////////////////////////////////////////////////////
// Returns the string representation of __VA_ARGS__
// >> TPP_STR(foobar) // Expands to ["foobar"]
#define TPP_STR(...)  TPP_STR_NOEXPAND(__VA_ARGS__)
#define TPP_STR_NOEXPAND(...) #__VA_ARGS__
