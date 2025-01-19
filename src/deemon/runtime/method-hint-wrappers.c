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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>

/**/
#include "kwlist.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMethodAttributeImpls from "..method-hints.method-hints")();]]]*/
PUBLIC_CONST char const DeeMA___seq_bool___name[] = "__seq_bool__";
PUBLIC_CONST char const DeeMA___seq_bool___doc[] = "->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_bool__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	if (DeeArg_Unpack(argc, argv, ":__seq_bool__"))
		goto err;
	result = DeeSeq_OperatorBool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_size___name[] = "__seq_size__";
PUBLIC_CONST char const DeeMA___seq_size___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_size__"))
		goto err;
	return DeeSeq_OperatorSizeOb(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_iter___name[] = "__seq_iter__";
PUBLIC_CONST char const DeeMA___seq_iter___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_iter__"))
		goto err;
	return DeeSeq_OperatorIter(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_iterkeys___name[] = "__seq_iterkeys__";
PUBLIC_CONST char const DeeMA___seq_iterkeys___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iterkeys__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_iterkeys__"))
		goto err;
	return DeeSeq_OperatorIterKeys(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_any___name[] = "__seq_any__";
PUBLIC_CONST char const DeeMA___seq_any___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_any_name[] = "any";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_any__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:any",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAnyWithKey(self, key)
		         : DeeSeq_InvokeAny(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAnyWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeAnyWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C */
