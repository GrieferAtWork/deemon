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
#ifndef GUARD_DEEMON_RUNTIME_NEW_C
#define GUARD_DEEMON_RUNTIME_NEW_C 1

#include <deemon/api.h>

#include <deemon/kwds.h>   /* DeeKwds_Check, DeeKwds_SIZE */
#include <deemon/object.h> /* DREF, DeeObject, DeeObject_Size, DeeTypeObject, Dee_Decref, Dee_TYPE */
#include <deemon/tuple.h>  /* DeeTuple* */
#include <deemon/type.h>   /* DeeType_*, Dee_tp_new_copy_t, Dee_tp_new_kw_t, Dee_tp_new_t, OPERATOR_COPY, TP_FGC, TP_FHEAP */

#include <hybrid/host.h> /* __ARCH_VA_LIST_IS_STACK_POINTER */

#include "runtime_error.h"

#include <stdarg.h> /* va_end, va_list, va_start */
#include <stddef.h> /* NULL, size_t */

/* Use wildcard impls instead of materialized impls when optimizing for size */
#if defined(__OPTIMIZE_SIZE__) || 0
#define USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#endif /* __OPTIMIZE_SIZE__ */

DECL_BEGIN

/* Common new-function-selection feature flags */
#define COMMON_NEW_F_NORMAL 0
#define COMMON_NEW_F_GC1    1
#define COMMON_NEW_F_Heap1  2
#define COMMON_NEW_F_Free1  4
#define COMMON_NEW_F(GC, Heap, Free)     \
	(((GC) ? COMMON_NEW_F_GC1 : 0) |     \
	 ((Heap) ? COMMON_NEW_F_Heap1 : 0) | \
	 ((Free) ? COMMON_NEW_F_Free1 : 0))

LOCAL ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetCommonNewFeatures(DeeTypeObject *__restrict self) {
	unsigned int result;
#define DEFAULT_COPY_SHIFT 13
#if ((TP_FGC >> DEFAULT_COPY_SHIFT) == COMMON_NEW_F_GC1 && \
     (TP_FHEAP >> DEFAULT_COPY_SHIFT) == COMMON_NEW_F_Heap1)
	result = (self->tp_flags & (TP_FGC | TP_FHEAP)) >> DEFAULT_COPY_SHIFT;
#else /* ... */
	result = COMMON_NEW_F_NORMAL;
	if (DeeType_IsGC(self))
		result |= COMMON_NEW_F_GC1;
	if (DeeType_IsHeapType(self))
		result |= COMMON_NEW_F_Heap1;
#endif /* !... */
	if (self->tp_init.tp_alloc.tp_free)
		result |= COMMON_NEW_F_Free1;
	return result;
}



/* ============================ DeeObject_New() / DeeObject_NewKw() ============================ */

#define DEFAULT_NEW_F_NORMAL COMMON_NEW_F_NORMAL
#define DEFAULT_NEW_F_GC1    COMMON_NEW_F_GC1
#define DEFAULT_NEW_F_Heap1  COMMON_NEW_F_Heap1
#define DEFAULT_NEW_F_Free1  COMMON_NEW_F_Free1
#define DEFAULT_NEW_F_Arg0   0x08 /* tp_ctor */
#define DEFAULT_NEW_F_ArgN   0x10 /* tp_any_ctor */
#define DEFAULT_NEW_F_ArgN0  0x18 /* tp_any_ctor + tp_ctor */
#define DEFAULT_NEW_F_ArgK   0x20 /* tp_any_ctor_kw */
#define DEFAULT_NEW_F_ArgK0  0x28 /* tp_any_ctor_kw + tp_ctor */

LOCAL ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetDefaultNewFeatures(DeeTypeObject *__restrict self) {
	unsigned int result = DeeType_GetCommonNewFeatures(self);
	if (self->tp_init.tp_alloc.tp_any_ctor) {
		result |= DEFAULT_NEW_F_ArgN; /* Prefer non-*_kw constructor */
	} else if (self->tp_init.tp_alloc.tp_any_ctor_kw) {
		result |= DEFAULT_NEW_F_ArgK;
	}
	if (self->tp_init.tp_alloc.tp_ctor)
		result |= DEFAULT_NEW_F_Arg0;
	return result;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetDefaultNewKwFeatures(DeeTypeObject *__restrict self) {
	unsigned int result = DeeType_GetCommonNewFeatures(self);
	if (self->tp_init.tp_alloc.tp_any_ctor_kw) {
		result |= DEFAULT_NEW_F_ArgK; /* Prefer *_kw constructor */
	} else if (self->tp_init.tp_alloc.tp_any_ctor) {
		result |= DEFAULT_NEW_F_ArgN;
	}
	if (self->tp_init.tp_alloc.tp_ctor)
		result |= DEFAULT_NEW_F_Arg0;
	return result;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNew_Variable_Arg0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv) {
	if likely(argc == 0)
		return (*tp_self->tp_init.tp_var.tp_ctor)();
	err_unimplemented_constructor(tp_self, argc, argv);
	return NULL;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNew_Variable_ArgN(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv) {
	return (*tp_self->tp_init.tp_var.tp_any_ctor)(argc, argv);
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNew_Variable_ArgN0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv) {
	if (argc == 0)
		return (*tp_self->tp_init.tp_var.tp_ctor)();
	return (*tp_self->tp_init.tp_var.tp_any_ctor)(argc, argv);
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNew_Variable_ArgK(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv) {
	return (*tp_self->tp_init.tp_var.tp_any_ctor_kw)(argc, argv, NULL);
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNew_Variable_ArgK0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv) {
	if (argc == 0)
		return (*tp_self->tp_init.tp_var.tp_ctor)();
	return (*tp_self->tp_init.tp_var.tp_any_ctor_kw)(argc, argv, NULL);
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNew_Unsupported(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv) {
	err_unimplemented_constructor(tp_self, argc, argv);
	return NULL;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNewKw_Variable_Arg0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if likely(argc == 0) {
		if unlikely(kw) {
			if (DeeKwds_Check(kw)) {
				if (DeeKwds_SIZE(kw) != 0)
					goto err_no_keywords;
			} else {
				size_t kw_size = DeeObject_Size(kw);
				if unlikely(kw_size == (size_t)-1)
					goto err;
				if (kw_size != 0)
					goto err_no_keywords;
			}
		}
		return (*tp_self->tp_init.tp_var.tp_ctor)();
	}
	err_unimplemented_constructor_kw(tp_self, argc, argv, kw);
err:
	return NULL;
err_no_keywords:
	err_keywords_ctor_not_accepted(tp_self, kw);
	goto err;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNewKw_Variable_ArgN(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if unlikely(kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_size = DeeObject_Size(kw);
			if unlikely(kw_size == (size_t)-1)
				goto err;
			if (kw_size != 0)
				goto err_no_keywords;
		}
	}
	return (*tp_self->tp_init.tp_var.tp_any_ctor)(argc, argv);
err_no_keywords:
	err_keywords_ctor_not_accepted(tp_self, kw);
err:
	return NULL;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNewKw_Variable_ArgN0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if unlikely(kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_size = DeeObject_Size(kw);
			if unlikely(kw_size == (size_t)-1)
				goto err;
			if (kw_size != 0)
				goto err_no_keywords;
		}
	}
	if (argc == 0)
		return (*tp_self->tp_init.tp_var.tp_ctor)();
	return (*tp_self->tp_init.tp_var.tp_any_ctor)(argc, argv);
err_no_keywords:
	err_keywords_ctor_not_accepted(tp_self, kw);
err:
	return NULL;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNewKw_Variable_ArgK(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return (*tp_self->tp_init.tp_var.tp_any_ctor_kw)(argc, argv, kw);
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNewKw_Variable_ArgK0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if (argc == 0) {
		if (kw) {
			if (DeeKwds_Check(kw)) {
				if (DeeKwds_SIZE(kw) != 0)
					goto do_invoke_any_ctor_kw;
			} else {
				size_t kw_size = DeeObject_Size(kw);
				if unlikely(kw_size == (size_t)-1)
					goto err;
				if (kw_size != 0)
					goto do_invoke_any_ctor_kw;
			}
		}
		return (*tp_self->tp_init.tp_var.tp_ctor)();
	}
do_invoke_any_ctor_kw:
	return (*tp_self->tp_init.tp_var.tp_any_ctor_kw)(argc, argv, kw);
err:
	return NULL;
}

PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
DeeObject_DefaultNewKw_Unsupported(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	err_unimplemented_constructor_kw(tp_self, argc, argv, kw);
	return NULL;
}

/*[[[deemon
import * from deemon;

#include "new-new.c.inl"

print("#ifdef __INTELLISENSE__");
for (local params: getPermutations()) {
	print("PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL ",
	      "_".join(params), "(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv",
	      params.some.endswith("Kw") ? ", DeeObject *kw" : "", ");");
}
print("#else /" "* __INTELLISENSE__ *" "/");
print("DECL_END");
function materializeWildcard(params: {string...}): {{string...}...} {
	local indices = [({ 0 } * #params)...];
	for (;;) {
		yield (
			for (local i, v : indices.enumerate())
				if (params[i].endswith("X"))
					params[i][:-1] + v
				else
					params[i]
		).frozen;
		for (local i = #indices - 1;; --i) {
			if (i < 0)
				return;
			if (!params[i].endswith("X"))
				continue;
			local c = indices[i] + 1;
			local m = #params[i];
			if (c >= 2) {
				indices[i] = 0;
			} else {
				indices[i] = c;
				break;
			}
		}
	}
}

// Bind wildcard implementations when those should be used
for (local params: getPermutations()) {
	local key = "_".join(params);
	if (params.some.endswith("X")) {
		print("#ifdef USE_", key);
		print("#define DEFINE_", key);
		print("#include \"new-new.c.inl\"");
		for (local optionParams: materializeWildcard(params)) {
			print("#define ", "_".join(optionParams), " ", key);
		}
		print("#endif /" "* USE_", key, " *" "/");
	}
}

// Bind all missing implementations to individual materializations
for (local params: getPermutations()) {
	local key = "_".join(params);
	if (!params.some.endswith("X")) {
		print("#ifndef ", key);
		print("#define DEFINE_", key);
		print("#include \"new-new.c.inl\"");
		print("#endif /" "* !", key, " *" "/");
	}
}

print("DECL_BEGIN");
print("#endif /" "* !__INTELLISENSE__ *" "/");
]]]*/
#ifdef __INTELLISENSE__
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX(DeeTypeObject *tp_self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#else /* __INTELLISENSE__ */
DECL_END
#ifdef USE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1 */
#ifdef USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#include "new-new.c.inl"
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#endif /* USE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX */
#ifndef DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0 */
#ifndef DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1
#include "new-new.c.inl"
#endif /* !DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1 */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */
/*[[[end]]]*/


#define DEFAULT_NEW_F(Arg, GC, Heap, Free) \
	((DEFAULT_NEW_F_Arg##Arg - DEFAULT_NEW_F_Arg0) | COMMON_NEW_F(GC, Heap, Free))
/*[[[deemon
#include "new-new.c.inl"
for (local kw: {false, true}) {
	print("PRIVATE Dee_tp_new", kw ? "_kw" : "", "_t tpconst default_new", kw ? "_kw" : "", "_impls[40] = {");
	for (local params: getPermutations()) {
		if (params.some.endswith("X"))
			continue;
		if (kw != params.some.endswith("Kw"))
			continue;
		local key  = "_".join(params);
		local args = (
			for (local param: params)
				if (param.startswith("Arg"))
					param[3:].ljust(2)
				else if (param.endswith("0") || param.endswith("1"))
					param[-1:]
		).frozen;
		print("	/" "* [DEFAULT_NEW_F(", ", ".join(args), ")] = *" "/ &", key, ",");
	}
	print("};");
}
]]]*/
PRIVATE Dee_tp_new_t tpconst default_new_impls[40] = {
	/* [DEFAULT_NEW_F(0 , 0, 0, 0)] = */ &DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(0 , 0, 0, 1)] = */ &DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(0 , 0, 1, 0)] = */ &DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(0 , 0, 1, 1)] = */ &DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(0 , 1, 0, 0)] = */ &DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(0 , 1, 0, 1)] = */ &DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(0 , 1, 1, 0)] = */ &DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(0 , 1, 1, 1)] = */ &DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N , 0, 0, 0)] = */ &DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N , 0, 0, 1)] = */ &DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N , 0, 1, 0)] = */ &DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N , 0, 1, 1)] = */ &DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N , 1, 0, 0)] = */ &DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N , 1, 0, 1)] = */ &DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N , 1, 1, 0)] = */ &DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N , 1, 1, 1)] = */ &DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N0, 0, 0, 0)] = */ &DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N0, 0, 0, 1)] = */ &DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N0, 0, 1, 0)] = */ &DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N0, 0, 1, 1)] = */ &DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N0, 1, 0, 0)] = */ &DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N0, 1, 0, 1)] = */ &DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N0, 1, 1, 0)] = */ &DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N0, 1, 1, 1)] = */ &DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K , 0, 0, 0)] = */ &DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K , 0, 0, 1)] = */ &DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K , 0, 1, 0)] = */ &DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K , 0, 1, 1)] = */ &DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K , 1, 0, 0)] = */ &DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K , 1, 0, 1)] = */ &DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K , 1, 1, 0)] = */ &DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K , 1, 1, 1)] = */ &DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K0, 0, 0, 0)] = */ &DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K0, 0, 0, 1)] = */ &DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K0, 0, 1, 0)] = */ &DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K0, 0, 1, 1)] = */ &DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K0, 1, 0, 0)] = */ &DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K0, 1, 0, 1)] = */ &DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K0, 1, 1, 0)] = */ &DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K0, 1, 1, 1)] = */ &DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1,
};
PRIVATE Dee_tp_new_kw_t tpconst default_new_kw_impls[40] = {
	/* [DEFAULT_NEW_F(0 , 0, 0, 0)] = */ &DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(0 , 0, 0, 1)] = */ &DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(0 , 0, 1, 0)] = */ &DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(0 , 0, 1, 1)] = */ &DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(0 , 1, 0, 0)] = */ &DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(0 , 1, 0, 1)] = */ &DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(0 , 1, 1, 0)] = */ &DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(0 , 1, 1, 1)] = */ &DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N , 0, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N , 0, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N , 0, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N , 0, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N , 1, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N , 1, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N , 1, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N , 1, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N0, 0, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N0, 0, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N0, 0, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N0, 0, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(N0, 1, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(N0, 1, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(N0, 1, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(N0, 1, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K , 0, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K , 0, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K , 0, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K , 0, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K , 1, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K , 1, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K , 1, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K , 1, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K0, 0, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K0, 0, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K0, 0, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K0, 0, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1,
	/* [DEFAULT_NEW_F(K0, 1, 0, 0)] = */ &DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0,
	/* [DEFAULT_NEW_F(K0, 1, 0, 1)] = */ &DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1,
	/* [DEFAULT_NEW_F(K0, 1, 1, 0)] = */ &DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0,
	/* [DEFAULT_NEW_F(K0, 1, 1, 1)] = */ &DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1,
};
/*[[[end]]]*/


LOCAL ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_t DCALL
DeeType_RequireNew_uncached_impl(DeeTypeObject *__restrict self) {
	unsigned int feat;

	/* Try to inherit constructors if none are loaded */
	if (self->tp_init.tp_var.tp_ctor == NULL &&
	    self->tp_init.tp_var.tp_any_ctor == NULL &&
	    self->tp_init.tp_var.tp_any_ctor_kw == NULL)
		DeeType_InheritConstructors(self);

	/* Deal with variable types */
	if (DeeType_IsVariable(self)) {
		bool has0 = self->tp_init.tp_var.tp_ctor != NULL;
		if (self->tp_init.tp_var.tp_any_ctor != NULL) {
			return has0 ? &DeeObject_DefaultNew_Variable_ArgN0
			            : &DeeObject_DefaultNew_Variable_ArgN;
		} else if (self->tp_init.tp_var.tp_any_ctor_kw != NULL) {
			return has0 ? &DeeObject_DefaultNew_Variable_ArgK0
			            : &DeeObject_DefaultNew_Variable_ArgK;
		} else {
			return has0 ? &DeeObject_DefaultNew_Variable_Arg0
			            : &DeeObject_DefaultNew_Unsupported;
		}
		__builtin_unreachable();
	}

	/* Load tp_new features */
	feat = DeeType_GetDefaultNewFeatures(self);
	if (feat < DEFAULT_NEW_F_Arg0)
		return &DeeObject_DefaultNew_Unsupported; /* No actual constructors defined */
	feat -= DEFAULT_NEW_F_Arg0;
	ASSERT(feat < COMPILER_LENOF(default_new_impls));
	return default_new_impls[feat];
}

LOCAL ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_kw_t DCALL
DeeType_RequireNewKw_uncached_impl(DeeTypeObject *__restrict self) {
	unsigned int feat;

	/* Try to inherit constructors if none are loaded */
	if (self->tp_init.tp_var.tp_ctor == NULL &&
	    self->tp_init.tp_var.tp_any_ctor == NULL &&
	    self->tp_init.tp_var.tp_any_ctor_kw == NULL)
		DeeType_InheritConstructors(self);

	/* Deal with variable types */
	if (DeeType_IsVariable(self)) {
		bool has0 = self->tp_init.tp_var.tp_ctor != NULL;
		if (self->tp_init.tp_var.tp_any_ctor_kw != NULL) {
			return has0 ? &DeeObject_DefaultNewKw_Variable_ArgK0
			            : &DeeObject_DefaultNewKw_Variable_ArgK;
		} else if (self->tp_init.tp_var.tp_any_ctor != NULL) {
			return has0 ? &DeeObject_DefaultNewKw_Variable_ArgN0
			            : &DeeObject_DefaultNewKw_Variable_ArgN;
		} else {
			return has0 ? &DeeObject_DefaultNewKw_Variable_Arg0
			            : &DeeObject_DefaultNewKw_Unsupported;
		}
		__builtin_unreachable();
	}

	/* Load tp_new features */
	feat = DeeType_GetDefaultNewKwFeatures(self);
	if (feat < DEFAULT_NEW_F_Arg0)
		return &DeeObject_DefaultNewKw_Unsupported; /* No actual constructors defined */
	feat -= DEFAULT_NEW_F_Arg0;
	ASSERT(feat < COMPILER_LENOF(default_new_kw_impls));
	return default_new_kw_impls[feat];
}




/* ============================ DeeObject_Copy() ============================ */

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
DeeObject_DefaultCopy_Variable(DeeTypeObject *tp_self, DeeObject *self) {
	return (*tp_self->tp_init.tp_var.tp_copy_ctor)(self);
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
DeeObject_DefaultCopy_Unsupported(DeeTypeObject *tp_self, DeeObject *self) {
	(void)self;
	err_unimplemented_operator(tp_self, OPERATOR_COPY);
	return NULL;
}

#ifdef __INTELLISENSE__
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free0_HeapType0_GC0(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free0_HeapType0_GC1(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free0_HeapType1_GC0(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free0_HeapType1_GC1(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free1_HeapType0_GC0(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free1_HeapType0_GC1(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free1_HeapType1_GC0(DeeTypeObject *tp_self, DeeObject *self);
PRIVATE NONNULL((1)) DREF DeeObject *DCALL DeeObject_DefaultCopy_Free1_HeapType1_GC1(DeeTypeObject *tp_self, DeeObject *self);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC0
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC1
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC0
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC1
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC0
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC1
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC0
#include "new-copy.c.inl"
#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC1
#include "new-copy.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

PRIVATE Dee_tp_new_copy_t tpconst default_copy_impls[8] = {
	/* [COMMON_NEW_F(0, 0, 0)] = */ &DeeObject_DefaultCopy_Free0_HeapType0_GC0,
	/* [COMMON_NEW_F(0, 0, 1)] = */ &DeeObject_DefaultCopy_Free0_HeapType0_GC1,
	/* [COMMON_NEW_F(0, 1, 0)] = */ &DeeObject_DefaultCopy_Free0_HeapType1_GC0,
	/* [COMMON_NEW_F(0, 1, 1)] = */ &DeeObject_DefaultCopy_Free0_HeapType1_GC1,
	/* [COMMON_NEW_F(1, 0, 0)] = */ &DeeObject_DefaultCopy_Free1_HeapType0_GC0,
	/* [COMMON_NEW_F(1, 0, 1)] = */ &DeeObject_DefaultCopy_Free1_HeapType0_GC1,
	/* [COMMON_NEW_F(1, 1, 0)] = */ &DeeObject_DefaultCopy_Free1_HeapType1_GC0,
	/* [COMMON_NEW_F(1, 1, 1)] = */ &DeeObject_DefaultCopy_Free1_HeapType1_GC1,
};

LOCAL ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_copy_t DCALL
DeeType_RequireNewCopy_uncached_impl(DeeTypeObject *__restrict self) {
	unsigned int feat;
	if (self->tp_init.tp_var.tp_copy_ctor == NULL)
		DeeType_InheritConstructors(self);
	if (self->tp_init.tp_var.tp_copy_ctor == NULL)
		return &DeeObject_DefaultCopy_Unsupported;
	if (DeeType_IsVariable(self))
		return &DeeObject_DefaultCopy_Variable;
	feat = DeeType_GetCommonNewFeatures(self);
	return default_copy_impls[feat];
}





PRIVATE ATTR_NOINLINE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_t DCALL
DeeType_RequireNew_uncached(DeeTypeObject *__restrict self) {
	Dee_tp_new_t result;
	result = DeeType_RequireNew_uncached_impl(self);
	self->tp_init.tp_new = result;
	COMPILER_WRITE_BARRIER();
	return result;
}

PRIVATE ATTR_NOINLINE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_kw_t DCALL
DeeType_RequireNewKw_uncached(DeeTypeObject *__restrict self) {
	Dee_tp_new_kw_t result;
	result = DeeType_RequireNewKw_uncached_impl(self);
	self->tp_init.tp_new_kw = result;
	COMPILER_WRITE_BARRIER();
	return result;
}

PRIVATE ATTR_NOINLINE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_copy_t DCALL
DeeType_RequireNewCopy_uncached(DeeTypeObject *__restrict self) {
	Dee_tp_new_copy_t result;
	result = DeeType_RequireNewCopy_uncached_impl(self);
	self->tp_init.tp_new_copy = result;
	COMPILER_WRITE_BARRIER();
	return result;
}


/* Return the relevant implementations of DeeObject_New() / DeeObject_NewKw() / DeeObject_Copy() */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_t DCALL
DeeType_RequireNew(DeeTypeObject *__restrict self) {
	if likely(self->tp_init.tp_new)
		return self->tp_init.tp_new;
	return DeeType_RequireNew_uncached(self);
}

PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_kw_t DCALL
DeeType_RequireNewKw(DeeTypeObject *__restrict self) {
	if likely(self->tp_init.tp_new_kw)
		return self->tp_init.tp_new_kw;
	return DeeType_RequireNewKw_uncached(self);
}

PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_new_copy_t DCALL
DeeType_RequireNewCopy(DeeTypeObject *__restrict self) {
	if likely(self->tp_init.tp_new_copy)
		return self->tp_init.tp_new_copy;
	return DeeType_RequireNewCopy_uncached(self);
}



PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_New(DeeTypeObject *object_type, size_t argc, DeeObject *const *argv) {
#ifdef __OPTIMIZE_SIZE__
	return (*DeeType_RequireNew(object_type))(object_type, argc, argv);
#else /* __OPTIMIZE_SIZE__ */
	if likely(object_type->tp_init.tp_new)
		return (*object_type->tp_init.tp_new)(object_type, argc, argv);
	return (*DeeType_RequireNew_uncached(object_type))(object_type, argc, argv);
#endif /* !__OPTIMIZE_SIZE__ */
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_NewKw(DeeTypeObject *object_type, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
#ifdef __OPTIMIZE_SIZE__
	return (*DeeType_RequireNewKw(object_type))(object_type, argc, argv, kw);
#else /* __OPTIMIZE_SIZE__ */
	if likely(object_type->tp_init.tp_new_kw)
		return (*object_type->tp_init.tp_new_kw)(object_type, argc, argv, kw);
	return (*DeeType_RequireNewKw_uncached(object_type))(object_type, argc, argv, kw);
#endif /* !__OPTIMIZE_SIZE__ */
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Copy(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
#ifdef __OPTIMIZE_SIZE__
	return (*DeeType_RequireNewCopy(tp_self))(tp_self, self);
#else /* __OPTIMIZE_SIZE__ */
	if likely(tp_self->tp_init.tp_new_copy)
		return (*tp_self->tp_init.tp_new_copy)(tp_self, self);
	return (*DeeType_RequireNewCopy_uncached(tp_self))(tp_self, self);
#endif /* !__OPTIMIZE_SIZE__ */
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_TCopy(DeeTypeObject *tp_self, DeeObject *self) {
#ifdef __OPTIMIZE_SIZE__
	return (*DeeType_RequireNewCopy(tp_self))(tp_self, self);
#else /* __OPTIMIZE_SIZE__ */
	if likely(tp_self->tp_init.tp_new_copy)
		return (*tp_self->tp_init.tp_new_copy)(tp_self, self);
	return (*DeeType_RequireNewCopy_uncached(tp_self))(tp_self, self);
#endif /* !__OPTIMIZE_SIZE__ */
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewDefault)(DeeTypeObject *__restrict object_type) {
	return DeeObject_New(object_type, 0, NULL);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_NewTuple)(DeeTypeObject *object_type, DeeObject *args) {
	return DeeObject_New(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_NewTupleKw)(DeeTypeObject *object_type, DeeObject *args, DeeObject *kw) {
	return DeeObject_NewKw(object_type, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}


#ifdef __ARCH_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VNewPack, 12),
                    DCALL_ASSEMBLY_NAME(DeeObject_New, 12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *object_type,
                   size_t argc, va_list args) {
	return DeeObject_New(object_type, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* __ARCH_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VNewPack(DeeTypeObject *object_type,
                   size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_New(object_type, argc, DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !__ARCH_VA_LIST_IS_STACK_POINTER */

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VNewf(DeeTypeObject *object_type,
                char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_New(object_type,
	                       DeeTuple_SIZE(args_tuple),
	                       DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *
DeeObject_NewPack(DeeTypeObject *object_type, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VNewPack(object_type, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_Newf(DeeTypeObject *object_type,
               char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VNewf(object_type, format, args);
	va_end(args);
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_NEW_C */
