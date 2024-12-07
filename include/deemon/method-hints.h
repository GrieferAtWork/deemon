/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_METHOD_HINTS_H
#define GUARD_DEEMON_METHOD_HINTS_H 1

#include "api.h"

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_type_method_hint type_method_hint
#endif /* DEE_SOURCE */

/*
 * Method hints are declaration pairs that come in the form of an entry in a
 * type's `tp_methods' array using a pre-defined function pointer (declared
 * in this header), as well as an entry in a type's `tp_method_hints' array,
 * which then points to the low-level C implementation of the function.
 *
 * These method hints are only available for certain, commonly overwritten,
 * default functions of objects, and are used to:
 * - Make it easier to override default functions without needing to re-write
 *   the same argument processing stub every time the function gets defined.
 * - Allow for fasting dispatching to the actual, underlying function within
 *   optimized _hostasm code.
 *
 *
 * >> PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
 * >> myob_setdefault(MyObject *self, DeeObject *key, DeeObject *value) {
 * >>     // This gets called by:
 * >>     // - `MyObject().setdefault(...)'
 * >>     // - `Mapping.setdefault(MyObject(), ...)'
 * >>     ...
 * >> }
 * >>
 * >> PRIVATE struct type_method_hint myob_method_hints[] = {
 * >>     TYPE_METHOD_HINT(map_setdefault, (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&myob_setdefault),
 * >>     TYPE_METHOD_HINT_END
 * >> };
 * >>
 * >> PRIVATE struct type_method myob_methods[] = {
 * >>     TYPE_METHOD_HINTREF(map_setdefault),
 * >>     TYPE_METHOD_END
 * >> };
 * >>
 * >> PRIVATE DeeTypeObject MyObject_Type = {
 * >>     ...
 * >>     .tp_methods      = myob_methods,
 * >>     .tp_method_hints = myob_method_hints,
 * >> };
 *
 */

enum {
#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	Dee_TMH_##func_name,
#include "method-hints.def"
	Dee_TMH_COUNT
};

enum {
#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	_Dee_TMH_WRAPPER_FLAGS_##attr_name = wrapper_flags,
#include "method-hints.def"
};

#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	typedef Treturn (cc *Dee_mh_##func_name##_t) params;
#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeMH_##attr_name) wrapper_params;           \
	DDATDEF char const DeeMH_##attr_name##_doc[];                                                    \
	DDATDEF char const DeeMH_##attr_name##_name[];
#include "method-hints.def"



/* Used to declare type method hints in C */
struct Dee_type_method_hint {
	uintptr_t    tmh_id;   /* Method hint ID (one of `Dee_TMH_*') */
	Dee_funptr_t tmh_func; /* [1..1] Method hint implementation (custom/type-specific) (NULL marks end-of-list) */
};

#define Dee_TYPE_METHOD_HINT(func_name, func) \
	{ Dee_TMH_##func_name, (Dee_funptr_t)Dee_REQUIRES_TYPE(DeeMH_##func_name##_t, func) }
#define Dee_TYPE_METHOD_HINT_END { 0, NULL }


/* Link a type method in as part of a type's `tp_methods' array. */
#define Dee_TYPE_METHOD_HINTREF(attr_name) \
	{ DeeMH_##attr_name##_name,            \
	  (Dee_objmethod_t)&DeeMH_##attr_name, \
	  DeeMH_##attr_name##_doc,             \
	  _Dee_TMH_WRAPPER_FLAGS_##attr_name }


#ifdef DEE_SOURCE
#define TYPE_METHOD_HINT     Dee_TYPE_METHOD_HINT
#define TYPE_METHOD_HINT_END Dee_TYPE_METHOD_HINT_END
#define TYPE_METHOD_HINTREF  Dee_TYPE_METHOD_HINTREF
#endif /* DEE_SOURCE */

/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
DFUNDEF ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetPrivateMethodHint(DeeTypeObject *__restrict self, uintptr_t id);

/* Same as `DeeType_GetPrivateMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions. */
DFUNDEF ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetMethodHint(DeeTypeObject *__restrict self, uintptr_t id);

DECL_END

#endif /* !GUARD_DEEMON_METHOD_HINTS_H */
