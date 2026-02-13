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
#ifndef GUARD_DEEMON_OBJECTS_TYPE_C
#define GUARD_DEEMON_OBJECTS_TYPE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeSlab_ENUMERATE, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>                /* DeeArg_Unpack*, UNPuSIZ */
#include <deemon/bool.h>               /* return_bool, return_bool01, return_false, return_true */
#include <deemon/callable.h>           /* DeeCallable_Type */
#include <deemon/class.h>              /* DeeClassDesc_QueryInstanceAttributeStringHash, DeeClassDescriptorObject, DeeClass_DESC, DeeInstance_DESC, Dee_CLASS_*, Dee_class_attribute, Dee_class_desc, Dee_class_desc_lock_endread, Dee_class_desc_lock_read, Dee_instance_desc, Dee_instance_desc_lock_endwrite, Dee_instance_desc_lock_write */
#include <deemon/code.h>               /* DeeCodeObject, DeeFunctionObject, DeeFunction_Check, DeeFunction_Type, DeeYieldFunctionObject, DeeYieldFunction_Type */
#include <deemon/computed-operators.h>
#include <deemon/dict.h>               /* Dee_dict_item */
#include <deemon/error-rt.h>           /* DeeRT_ATTRIBUTE_ACCESS_BOUND, DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_*, ERROR_PRINT_DOHANDLE */
#include <deemon/file.h>               /* DeeFileType_Type */
#include <deemon/format.h>             /* DeeFormat_PrintStr, DeeFormat_Printf */
#include <deemon/gc.h>                 /* DeeGC_Track */
#include <deemon/int.h>                /* DeeInt_NewSize, DeeInt_NewUInt */
#include <deemon/kwds.h>               /* DeeKwds*, Dee_kwds_entry */
#include <deemon/map.h>                /* DeeMapping_Type */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint */
#include <deemon/module.h>             /* DeeModuleObject, DeeModule_OfPointer, Dee_module_object */
#include <deemon/mro.h>                /* DeeType_Bound*Attr*, DeeType_Call*Attr*, DeeType_Del*Attr*, DeeType_FindAttr, DeeType_FindAttrInfoStringLenHash, DeeType_Get*Attr*, DeeType_Has*Attr*, DeeType_IterAttr, DeeType_QueryAttributeHash, DeeType_QueryInstanceAttributeHash, DeeType_Set*Attr*, DeeType_VCallAttrStringHashf, DeeType_VCallAttrf, Dee_attrdesc, Dee_attrhint, Dee_attrinfo, Dee_attriter, Dee_attrspec, Dee_membercache_fini, Dee_membercache_init */
#include <deemon/none.h>               /* DeeNone_Check, DeeNone_Type, Dee_None, return_none */
#include <deemon/object.h>             /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_COMPARE_ERR, Dee_Decref*, Dee_Incref, Dee_TYPE, Dee_WEAKREF_SUPPORT_ADDR, Dee_XDecref, Dee_XDecref_unlikely, Dee_formatprinter_t, Dee_funptr_t, Dee_hash_t, Dee_ssize_t, Dee_visit_t, Dee_weakref_support_fini, Dee_weakref_support_init, ITER_DONE, OBJECT_HEAD, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeType_GetNativeOperator, DeeType_GetNativeOperatorWithoutUnsupported, Dee_tno_id */
#include <deemon/property.h>           /* DeePropertyObject, DeeProperty_Type */
#include <deemon/rodict.h>             /* DeeRoDictObject, DeeRoDict_Type */
#include <deemon/roset.h>              /* DeeRoSetObject, DeeRoSet_Type, Dee_roset_item */
#include <deemon/seq.h>                /* DeeSeq_Type, DeeType_GetSeqClass, Dee_SEQCLASS_* */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/super.h>              /* DeeSuper* */
#include <deemon/system-features.h>    /* bzero*, memcpyc */
#include <deemon/tuple.h>              /* DeeNullableTuple_Type, DeeTuple* */
#include <deemon/type.h>               /* DeeObject_Init, DeeObject_UndoConstruction, DeeTypeMRO, DeeTypeMRO_Init, DeeTypeMRO_Next, DeeTypeType_GetOperatorByName, DeeType_*, Dee_GC_PRIORITY_CLASS, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_Visit, Dee_XVisit, Dee_operator_t, Dee_opinfo, Dee_type_*, METHOD_FCONSTCALL, METHOD_FNOREFESCAPE, STRUCT_*, TF_*, TP_F*, TYPE_*, type_* */
#include <deemon/util/hash-io.h>       /* Dee_hash_vidx_t */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */
#include <deemon/util/weakref.h>       /* Dee_weakref_* */

#include "../runtime/kwlist.h"
#include "../runtime/method-hints.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/typemro.h"
#include "type-operators.h"

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint16_t */

#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

#ifndef GenericObject_DEFINED
#define GenericObject_DEFINED
typedef struct {
	OBJECT_HEAD
} GenericObject;
#endif /* !GenericObject_DEFINED */


/* Assert the typing of an object (raising an `Error.TypeError' if the type wasn't expected)
 * HINT: When `required_type' isn't a type-object, these functions throw an error!
 * @return: -1: The object doesn't match the required typing.
 * @return:  0: The object matches the required typing. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertType)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeObject_InstanceOf(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertTypeOrAbstract)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeType_IsAbstract(required_type))
		return 0;
	if likely(DeeObject_InstanceOf(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertImplements)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeObject_Implements(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertTypeExact)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeObject_InstanceOfExact(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}


/* Returns the class of `self', automatically
 * dereferencing super-objects and other wrappers.
 * Beyond that, this function returns the same as `Dee_TYPE()' */
PUBLIC WUNUSED ATTR_RETNONNULL NONNULL((1)) DeeTypeObject *DCALL
DeeObject_Class(DeeObject *__restrict self) {
	DeeTypeObject *result;
	ASSERT_OBJECT(self);
	result = Dee_TYPE(self);
	if (result == &DeeSuper_Type)
		result = DeeSuper_TYPE(self);
	return result;
}


/* Return true if `test_type' is equal to, or extends `extended_type'
 * NOTE: When `extended_type' is not a type, this function simply returns `false'
 * >> return test_type.extends(extended_type);
 *
 * HINT: Always returns either `0' or `1'!
 * @return: 0 : "test_type" does not inherit from `extended_type', or `extended_type' isn't a type
 * @return: 1 : "test_type" does inherit from `extended_type' */
PUBLIC WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_Extends(DeeTypeObject const *test_type,
                DeeTypeObject const *extended_type) {
	do {
		if (test_type == extended_type)
			return 1;
	} while ((test_type = DeeType_Base(test_type)) != NULL);
	return 0;
}

/* Same as `DeeType_Extends()', but also check `tp_mro' for matches.
 * This function should be used when `implemented_type' is an abstract type.
 * >> return test_type.implements(implemented_type);
 *
 * HINT: Always returns either `0' or `1'!
 * @return: 0 : "test_type" does not implement "implemented_type"
 * @return: 1 : "test_type" does implement "implemented_type" */
PUBLIC WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_Implements(DeeTypeObject const *test_type,
                   DeeTypeObject const *implemented_type) {
	DeeTypeMRO mro;
	test_type = DeeTypeMRO_Init(&mro, test_type);
	do {
		if (test_type == implemented_type)
			return 1;
	} while ((test_type = DeeTypeMRO_Next(&mro, test_type)) != NULL);
	return 0;
}




INTERN WUNUSED NONNULL((1)) int DCALL
type_ctor(DeeTypeObject *__restrict self) {
	/* Simply initialize everything to ZERO and set the HEAP flag. */
	bzero((void *)&self->tp_name,
	      sizeof(DeeTypeObject) -
	      offsetof(DeeTypeObject, tp_name));
	self->tp_flags |= TP_FHEAP;
	return 0;
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_str(DeeObject *__restrict self) {
	DeeTypeObject *me = (DeeTypeObject *)self;
	char const *name;
	if (me->tp_flags & TP_FNAMEOBJECT) {
		DREF DeeStringObject *result;
		result = container_of(me->tp_name, DeeStringObject, s_str);
		Dee_Incref(result);
		return Dee_AsObject(result);
	}
	name = DeeType_GetName(me);
	return DeeString_New(name);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
type_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DeeTypeObject *me = (DeeTypeObject *)self;
	char const *name;
	if (me->tp_flags & TP_FNAMEOBJECT) {
		DREF DeeStringObject *nameob;
		nameob = container_of(me->tp_name, DeeStringObject, s_str);
		return DeeString_PrintUtf8((DeeObject *)nameob, printer, arg);
	}
	name = DeeType_GetName(me);
	return DeeFormat_PrintStr(printer, arg, name);
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_repr(DeeObject *__restrict self) {
	DeeTypeObject *me = (DeeTypeObject *)self;
	DREF DeeModuleObject *mod;
	DREF DeeStringObject *result;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DeeStringObject *modname;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	char const *name;
	mod = DeeType_GetModule(me);
	if (!mod)
		goto fallback;
	name = DeeType_GetName(me);
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	result  = (DREF DeeStringObject *)DeeString_Newf("%r.%s", mod, name);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	modname = mod->mo_name;
	result  = (DREF DeeStringObject *)DeeString_Newf("%k.%s", modname, name);
	Dee_Decref(mod);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return Dee_AsObject(result);
fallback:
	return type_str(self);
}

DEFAULT_OPIMP WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
type_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	char const *name;
	DeeTypeObject *me = (DeeTypeObject *)self;
	DREF DeeModuleObject *mod = DeeType_GetModule(me);
	if (!mod)
		goto fallback;
	name = DeeType_GetName(me);
	return DeeFormat_Printf(printer, arg, "%R.%s", mod, name);
fallback:
	return type_print(self, printer, arg);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	Dee_ssize_t result, temp;
	char const *name;
	DeeTypeObject *me = (DeeTypeObject *)self;
	DREF DeeModuleObject *mod = DeeType_GetModule(me);
	if (!mod)
		goto fallback;
	result = DeeString_PrintUtf8((DeeObject *)mod->mo_name, printer, arg);
	Dee_Decref(mod);
	if unlikely(result < 0)
		goto done;
	name = DeeType_GetName(me);
	temp = DeeFormat_Printf(printer, arg, ".%s", name);
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
fallback:
	return type_print(self, printer, arg);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}


INTDEF NONNULL((1)) void DCALL class_fini(DeeTypeObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL class_visit(DeeTypeObject *__restrict self, Dee_visit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL class_clear(DeeTypeObject *__restrict self);
INTDEF NONNULL((1)) void DCALL class_pclear(DeeTypeObject *__restrict self, unsigned int gc_priority);

PRIVATE NONNULL((1)) void DCALL
type_fini(DeeTypeObject *__restrict self) {
	/* Clear weak references and check for revival. */
	Dee_weakref_fini(&self->tp_module);
	Dee_weakref_support_fini(self);
	ASSERTF(DeeType_IsHeapType(self),
	        "Non heap-allocated type %k is being destroyed (This shouldn't happen)",
	        self);

	/* Free the method hint cache */
	if (self->tp_mhcache)
		Dee_type_mh_cache_destroy(self->tp_mhcache);

	/* Finalize class data if the type is actually a user-defined class. */
	if (DeeType_IsClass(self)) {
		class_fini(self);
		Dee_Free(self->tp_class);
	}

	/* clang-format off */
/*[[[deemon (printFreeAllocatedOperatorTables from "..method-hints.method-hints")("self");]]]*/
	Dee_Free(self->tp_callable);
	Dee_Free(self->tp_iterator);
	Dee_Free(self->tp_math);
	Dee_Free(self->tp_cmp);
	Dee_Free(self->tp_seq);
	Dee_Free(self->tp_with);
	Dee_Free(self->tp_attr);
/*[[[end]]]*/
	/* clang-format on */

	/* Finalize the type's member caches. */
	Dee_membercache_fini(&self->tp_cache);
	Dee_membercache_fini(&self->tp_class_cache);

	/* Cleanup extra MRO types */
	if (self->tp_mro != NULL) {
		size_t i;
		for (i = 0; self->tp_mro[i] != NULL; ++i)
			Dee_Decref_unlikely(self->tp_mro[i]);
		Dee_Free((void *)self->tp_mro);
	}

	/* Cleanup name & doc objects should those have been used. */
	if (self->tp_flags & TP_FNAMEOBJECT)
		Dee_Decref_likely(container_of(self->tp_name, DeeStringObject, s_str));
	if (self->tp_flags & TP_FDOCOBJECT)
		Dee_Decref_likely(container_of(self->tp_doc, DeeStringObject, s_str));
	Dee_XDecref_unlikely(self->tp_base);
}


#ifdef CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
#define DeeType_GetAbiOperator(self, id) DeeType_GetNativeOperator(self, id)
#else /* CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
#define DeeType_GetAbiOperator(self, id) DeeType_GetNativeOperatorWithoutUnsupported(self, id)
#endif /* !CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */

#define Dee_TNO_UNDEF Dee_TNO_COUNT

#define VTABLE_DESC__tp_math(cb, _)                  \
	cb(_, 0x00, tp_int32, Dee_TNO_int32)             \
	cb(_, 0x01, tp_int64, Dee_TNO_int64)             \
	cb(_, 0x02, tp_double, Dee_TNO_double)           \
	cb(_, 0x03, tp_int, Dee_TNO_int)                 \
	cb(_, 0x04, tp_inv, Dee_TNO_inv)                 \
	cb(_, 0x05, tp_pos, Dee_TNO_pos)                 \
	cb(_, 0x06, tp_neg, Dee_TNO_neg)                 \
	cb(_, 0x07, tp_add, Dee_TNO_add)                 \
	cb(_, 0x08, tp_sub, Dee_TNO_sub)                 \
	cb(_, 0x09, tp_mul, Dee_TNO_mul)                 \
	cb(_, 0x0a, tp_div, Dee_TNO_div)                 \
	cb(_, 0x0b, tp_mod, Dee_TNO_mod)                 \
	cb(_, 0x0c, tp_shl, Dee_TNO_shl)                 \
	cb(_, 0x0d, tp_shr, Dee_TNO_shr)                 \
	cb(_, 0x0e, tp_and, Dee_TNO_and)                 \
	cb(_, 0x0f, tp_or, Dee_TNO_or)                   \
	cb(_, 0x10, tp_xor, Dee_TNO_xor)                 \
	cb(_, 0x11, tp_pow, Dee_TNO_pow)                 \
	cb(_, 0x12, tp_inc, Dee_TNO_inc)                 \
	cb(_, 0x13, tp_dec, Dee_TNO_dec)                 \
	cb(_, 0x14, tp_inplace_add, Dee_TNO_inplace_add) \
	cb(_, 0x15, tp_inplace_sub, Dee_TNO_inplace_sub) \
	cb(_, 0x16, tp_inplace_mul, Dee_TNO_inplace_mul) \
	cb(_, 0x17, tp_inplace_div, Dee_TNO_inplace_div) \
	cb(_, 0x18, tp_inplace_mod, Dee_TNO_inplace_mod) \
	cb(_, 0x19, tp_inplace_shl, Dee_TNO_inplace_shl) \
	cb(_, 0x1a, tp_inplace_shr, Dee_TNO_inplace_shr) \
	cb(_, 0x1b, tp_inplace_and, Dee_TNO_inplace_and) \
	cb(_, 0x1c, tp_inplace_or, Dee_TNO_inplace_or)   \
	cb(_, 0x1d, tp_inplace_xor, Dee_TNO_inplace_xor) \
	cb(_, 0x1e, tp_inplace_pow, Dee_TNO_inplace_pow)

#define VTABLE_DESC__tp_cmp(cb, _)                       \
	cb(_, 0x00, tp_hash, Dee_TNO_hash)                   \
	cb(_, 0x01, tp_compare_eq, Dee_TNO_compare_eq)       \
	cb(_, 0x02, tp_compare, Dee_TNO_compare)             \
	cb(_, 0x03, tp_trycompare_eq, Dee_TNO_trycompare_eq) \
	cb(_, 0x04, tp_eq, Dee_TNO_eq)                       \
	cb(_, 0x05, tp_ne, Dee_TNO_ne)                       \
	cb(_, 0x06, tp_lo, Dee_TNO_lo)                       \
	cb(_, 0x07, tp_le, Dee_TNO_le)                       \
	cb(_, 0x08, tp_gr, Dee_TNO_gr)                       \
	cb(_, 0x09, tp_ge, Dee_TNO_ge)                       \
	cb(_, 0x0a, tp_nii, Dee_TNO_UNDEF)

#define VTABLE_DESC__tp_seq(cb, _)                                                 \
	cb(_, 0x00, tp_iter, Dee_TNO_iter)                                             \
	cb(_, 0x01, tp_sizeob, Dee_TNO_sizeob)                                         \
	cb(_, 0x02, tp_contains, Dee_TNO_contains)                                     \
	cb(_, 0x03, tp_getitem, Dee_TNO_getitem)                                       \
	cb(_, 0x04, tp_delitem, Dee_TNO_delitem)                                       \
	cb(_, 0x05, tp_setitem, Dee_TNO_setitem)                                       \
	cb(_, 0x06, tp_getrange, Dee_TNO_getrange)                                     \
	cb(_, 0x07, tp_delrange, Dee_TNO_delrange)                                     \
	cb(_, 0x08, tp_setrange, Dee_TNO_setrange)                                     \
	cb(_, 0x09, tp_foreach, Dee_TNO_foreach)                                       \
	cb(_, 0x0a, tp_foreach_pair, Dee_TNO_foreach_pair)                             \
	cb(_, 0x0b, tp_bounditem, Dee_TNO_bounditem)                                   \
	cb(_, 0x0c, tp_hasitem, Dee_TNO_hasitem)                                       \
	cb(_, 0x0d, tp_size, Dee_TNO_size)                                             \
	cb(_, 0x0e, tp_size_fast, Dee_TNO_size_fast)                                   \
	cb(_, 0x0f, tp_getitem_index, Dee_TNO_getitem_index)                           \
	cb(_, 0x10, tp_getitem_index_fast, Dee_TNO_getitem_index_fast)                 \
	cb(_, 0x11, tp_delitem_index, Dee_TNO_delitem_index)                           \
	cb(_, 0x12, tp_setitem_index, Dee_TNO_setitem_index)                           \
	cb(_, 0x13, tp_bounditem_index, Dee_TNO_bounditem_index)                       \
	cb(_, 0x14, tp_hasitem_index, Dee_TNO_hasitem_index)                           \
	cb(_, 0x15, tp_getrange_index, Dee_TNO_getrange_index)                         \
	cb(_, 0x16, tp_delrange_index, Dee_TNO_delrange_index)                         \
	cb(_, 0x17, tp_setrange_index, Dee_TNO_setrange_index)                         \
	cb(_, 0x18, tp_getrange_index_n, Dee_TNO_getrange_index_n)                     \
	cb(_, 0x19, tp_delrange_index_n, Dee_TNO_delrange_index_n)                     \
	cb(_, 0x1a, tp_setrange_index_n, Dee_TNO_setrange_index_n)                     \
	cb(_, 0x1b, tp_trygetitem, Dee_TNO_trygetitem)                                 \
	cb(_, 0x1c, tp_trygetitem_index, Dee_TNO_trygetitem_index)                     \
	cb(_, 0x1d, tp_trygetitem_string_hash, Dee_TNO_trygetitem_string_hash)         \
	cb(_, 0x1e, tp_getitem_string_hash, Dee_TNO_getitem_string_hash)               \
	cb(_, 0x1f, tp_delitem_string_hash, Dee_TNO_delitem_string_hash)               \
	cb(_, 0x20, tp_setitem_string_hash, Dee_TNO_setitem_string_hash)               \
	cb(_, 0x21, tp_bounditem_string_hash, Dee_TNO_bounditem_string_hash)           \
	cb(_, 0x22, tp_hasitem_string_hash, Dee_TNO_hasitem_string_hash)               \
	cb(_, 0x23, tp_trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash) \
	cb(_, 0x24, tp_getitem_string_len_hash, Dee_TNO_getitem_string_len_hash)       \
	cb(_, 0x25, tp_delitem_string_len_hash, Dee_TNO_delitem_string_len_hash)       \
	cb(_, 0x26, tp_setitem_string_len_hash, Dee_TNO_setitem_string_len_hash)       \
	cb(_, 0x27, tp_bounditem_string_len_hash, Dee_TNO_bounditem_string_len_hash)   \
	cb(_, 0x28, tp_hasitem_string_len_hash, Dee_TNO_hasitem_string_len_hash)       \
	cb(_, 0x29, tp_asvector, Dee_TNO_UNDEF)                                        \
	cb(_, 0x2a, tp_asvector_nothrow, Dee_TNO_UNDEF)                                \
	cb(_, 0x2b, tp_trygetitemnr, Dee_TNO_UNDEF)                                    \
	cb(_, 0x2c, tp_trygetitemnr_string_hash, Dee_TNO_UNDEF)                        \
	cb(_, 0x2d, tp_trygetitemnr_string_len_hash, Dee_TNO_UNDEF)

#define VTABLE_DESC__tp_iterator(cb, _)          \
	cb(_, 0x00, tp_nextpair, Dee_TNO_nextpair)   \
	cb(_, 0x01, tp_nextkey, Dee_TNO_nextkey)     \
	cb(_, 0x02, tp_nextvalue, Dee_TNO_nextvalue) \
	cb(_, 0x03, tp_advance, Dee_TNO_advance)

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define IF_CONFIG_CALLTUPLE_OPTIMIZATIONS(x) x
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define IF_CONFIG_CALLTUPLE_OPTIMIZATIONS(x) /* nothing */
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define VTABLE_DESC__tp_attr(cb, _)                                              \
	cb(_, 0x00, tp_getattr, Dee_TNO_getattr)                                     \
	cb(_, 0x01, tp_delattr, Dee_TNO_delattr)                                     \
	cb(_, 0x02, tp_setattr, Dee_TNO_setattr)                                     \
	cb(_, 0x03, tp_iterattr, Dee_TNO_UNDEF)                                      \
	cb(_, 0x04, tp_findattr, Dee_TNO_UNDEF)                                      \
	cb(_, 0x05, tp_hasattr, Dee_TNO_hasattr)                                     \
	cb(_, 0x06, tp_boundattr, Dee_TNO_boundattr)                                 \
	cb(_, 0x07, tp_callattr, Dee_TNO_UNDEF)                                      \
	cb(_, 0x08, tp_callattr_kw, Dee_TNO_UNDEF)                                   \
	cb(_, 0x09, tp_vcallattrf, Dee_TNO_UNDEF)                                    \
	cb(_, 0x0a, tp_getattr_string_hash, Dee_TNO_getattr_string_hash)             \
	cb(_, 0x0b, tp_delattr_string_hash, Dee_TNO_delattr_string_hash)             \
	cb(_, 0x0c, tp_setattr_string_hash, Dee_TNO_setattr_string_hash)             \
	cb(_, 0x0d, tp_hasattr_string_hash, Dee_TNO_hasattr_string_hash)             \
	cb(_, 0x0e, tp_boundattr_string_hash, Dee_TNO_boundattr_string_hash)         \
	cb(_, 0x0f, tp_callattr_string_hash, Dee_TNO_UNDEF)                          \
	cb(_, 0x10, tp_callattr_string_hash_kw, Dee_TNO_UNDEF)                       \
	cb(_, 0x11, tp_vcallattr_string_hashf, Dee_TNO_UNDEF)                        \
	cb(_, 0x12, tp_getattr_string_len_hash, Dee_TNO_getattr_string_len_hash)     \
	cb(_, 0x13, tp_delattr_string_len_hash, Dee_TNO_delattr_string_len_hash)     \
	cb(_, 0x14, tp_setattr_string_len_hash, Dee_TNO_setattr_string_len_hash)     \
	cb(_, 0x15, tp_hasattr_string_len_hash, Dee_TNO_hasattr_string_len_hash)     \
	cb(_, 0x16, tp_boundattr_string_len_hash, Dee_TNO_boundattr_string_len_hash) \
	cb(_, 0x17, tp_callattr_string_len_hash, Dee_TNO_UNDEF)                      \
	cb(_, 0x18, tp_callattr_string_len_hash_kw, Dee_TNO_UNDEF)                   \
	cb(_, 0x19, tp_findattr_info_string_len_hash, Dee_TNO_UNDEF)                 \
	IF_CONFIG_CALLTUPLE_OPTIMIZATIONS(                                           \
	cb(_, 0x1a, tp_callattr_tuple, Dee_TNO_UNDEF)                                \
	cb(_, 0x1b, tp_callattr_tuple_kw, Dee_TNO_UNDEF))

#define VTABLE_DESC__tp_with(cb, _)      \
	cb(_, 0x00, tp_enter, Dee_TNO_enter) \
	cb(_, 0x01, tp_leave, Dee_TNO_leave)

#define VTABLE_DESC__tp_callable(cb, _)                    \
	cb(_, 0x00, tp_call_kw, Dee_TNO_call_kw)               \
	cb(_, 0x01, tp_thiscall, Dee_TNO_thiscall)             \
	cb(_, 0x02, tp_thiscall_kw, Dee_TNO_thiscall_kw)       \
	IF_CONFIG_CALLTUPLE_OPTIMIZATIONS(                     \
	cb(_, 0x03, tp_call_tuple, Dee_TNO_call_tuple)         \
	cb(_, 0x04, tp_call_tuple_kw, Dee_TNO_call_tuple_kw)   \
	cb(_, 0x05, tp_thiscall_tuple, Dee_TNO_thiscall_tuple) \
	cb(_, 0x06, tp_thiscall_tuple_kw, Dee_TNO_thiscall_tuple_kw))

#define _ASSERT_DESC_CB(struct_type, id, field, tno_id) \
	STATIC_ASSERT(offsetof(struct struct_type, field) == (id) * sizeof(Dee_funptr_t));
#define _ENUM_TNO_CB(struct_type, id, field, tno_id) tno_id,
#define DEFINE_DESC(struct_type, desc)                            \
	desc(_ASSERT_DESC_CB, struct_type)                            \
	PRIVATE enum Dee_tno_id const struct_type##_vtable_desc[] = { \
		desc(_ENUM_TNO_CB, ~)                                     \
	};                                                            \
	STATIC_ASSERT(sizeof(struct struct_type) ==                   \
	              COMPILER_LENOF(struct_type##_vtable_desc) *     \
	              sizeof(Dee_funptr_t))
DEFINE_DESC(type_math, VTABLE_DESC__tp_math);
DEFINE_DESC(type_cmp, VTABLE_DESC__tp_cmp);
DEFINE_DESC(type_seq, VTABLE_DESC__tp_seq);
DEFINE_DESC(type_iterator, VTABLE_DESC__tp_iterator);
DEFINE_DESC(type_attr, VTABLE_DESC__tp_attr);
DEFINE_DESC(type_with, VTABLE_DESC__tp_with);
DEFINE_DESC(type_callable, VTABLE_DESC__tp_callable);
#undef DEFINE_DESC
#undef _ENUM_TNO_CB
#undef _ASSERT_DESC_CB

PRIVATE WUNUSED NONNULL((1, 3, 5, 6)) int DCALL
DeeSerial_PutVTable(DeeSerial *__restrict writer, Dee_seraddr_t addr,
                    enum Dee_tno_id const *__restrict desc, size_t num_functions,
                    Dee_funptr_t const **p_vtable, DeeTypeObject *__restrict tp) {
	size_t i;
	Dee_seraddr_t out_vtable_addr;
	bool has_any = false;
	Dee_funptr_t const *in_vtable;
	/* Pre-load all function pointers. */
	for (i = 0; i < num_functions; ++i) {
		if (desc[i] != Dee_TNO_UNDEF) {
			Dee_funptr_t ptr = DeeType_GetAbiOperator(tp, desc[i]);
			if (ptr)
				has_any = true;
		}
	}
	if unlikely(!has_any) {
		*DeeSerial_Addr2Mem(writer, addr, void *) = NULL;
		return 0;
	}
	in_vtable = *p_vtable;
	out_vtable_addr = DeeSerial_Malloc(writer, num_functions * sizeof(Dee_funptr_t), in_vtable);
	if (!Dee_SERADDR_ISOK(out_vtable_addr))
		goto err;
	if (DeeSerial_PutAddr(writer, addr, out_vtable_addr))
		goto err;
	for (i = 0; i < num_functions; ++i) {
		Dee_seraddr_t addrof_funptr = out_vtable_addr + i * sizeof(Dee_funptr_t);
		Dee_funptr_t ptr = NULL;
		if (desc[i] != Dee_TNO_UNDEF) {
			ptr = DeeType_GetAbiOperator(tp, desc[i]);
		} else if (in_vtable) {
			ptr = in_vtable[i];
		}
		if (DeeSerial_XPutFuncPtr(writer, addrof_funptr, ptr))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
type_serialize_custom_operators(DeeSerial *__restrict writer,
                                struct Dee_type_operator const *ops,
                                size_t count) {
	size_t i;
	Dee_seraddr_t out_addr = DeeSerial_Malloc(writer, count * sizeof(struct Dee_type_operator), ops);
	if unlikely(!Dee_SERADDR_ISOK(out_addr))
		goto err;
	memcpyc(DeeSerial_Addr2Mem(writer, out_addr, void),
	        ops, count, sizeof(struct Dee_type_operator));
	for (i = 0; i < count; ++i) {
		struct Dee_type_operator const *op = &ops[i];
		Dee_seraddr_t addrof_op = out_addr + i * sizeof(struct Dee_type_operator);
#define ADDROF(field) (addrof_op + offsetof(struct Dee_type_operator, field))
		if (Dee_type_operator_isdecl(op)) {
			if (DeeSerial_PutPointer(writer, ADDROF(to_decl.oi_invoke), op->to_decl.oi_invoke))
				goto err;
		} else {
			ASSERT(Dee_type_operator_iscustom(op));
			if (DeeSerial_XPutFuncPtr(writer, ADDROF(to_custom.s_invoke), op->to_custom.s_invoke))
				goto err;
		}
#undef ADDROF
	}
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
}


INTDEF WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
class_desc_serialize(struct Dee_class_desc *__restrict self,
                     DeeSerial *__restrict writer);

INTDEF struct type_cmp instance_builtin_cmp;
INTDEF struct type_callable instance_user_callable;

INTERN WUNUSED NONNULL((1, 2)) int DCALL
type_serialize(DeeTypeObject *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeTypeObject, field))
	DeeTypeObject *out;
	if (DeeSerial_XPutObject(writer, ADDROF(tp_base), self->tp_base))
		goto err;
	if (DeeSerial_PutWeakref(writer, ADDROF(tp_module), &self->tp_module))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, DeeTypeObject);
	Dee_weakref_support_init(out);
	out->tp_name     = NULL;
	out->tp_doc      = NULL;
	out->tp_flags    = self->tp_flags | TP_FHEAP;
	out->tp_weakrefs = self->tp_weakrefs;
	out->tp_features = self->tp_features;
	out->tp_init.tp_alloc.tp_instance_size = self->tp_init.tp_alloc.tp_instance_size;
	out->tp_operators      = NULL;
	out->tp_operators_size = self->tp_operators_size;
	out->tp_mhcache  = NULL; /* Not serialized (lazily allocated again if needed) */
	out->tp_mro      = NULL;
	Dee_membercache_init(&out->tp_cache);
	Dee_membercache_init(&out->tp_class_cache);
	out->tp_class = NULL;

	/* Encode name */
	if (self->tp_name) {
		if (self->tp_flags & TP_FNAMEOBJECT) {
			DeeStringObject *name = container_of(self->tp_name, DeeStringObject, s_str);
			if (DeeSerial_PutObjectEx(writer, ADDROF(tp_name), name, offsetof(DeeStringObject, s_str)))
				goto err;
		} else {
			if (DeeSerial_PutPointer(writer, ADDROF(tp_name), self->tp_name))
				goto err;
		}
	}

	/* Encode doc-string */
	if (self->tp_doc) {
		if (self->tp_flags & TP_FDOCOBJECT) {
			DeeStringObject *doc = container_of(self->tp_doc, DeeStringObject, s_str);
			if (DeeSerial_PutObjectEx(writer, ADDROF(tp_doc), doc, offsetof(DeeStringObject, s_str)))
				goto err;
		} else {
			if (DeeSerial_PutPointer(writer, ADDROF(tp_doc), self->tp_doc))
				goto err;
		}
	}

	/* Serialize extra MRO types */
	if (self->tp_mro != NULL) {
		size_t count = 0;
		while (self->tp_mro[count])
			++count;
		/* Technically, all elements but the last (sentinal) are
		 * non-NULL, but just re-juse this function for simplicity */
		if (DeeSerial_XPutMemdupObjectv(writer, ADDROF(tp_mro),
		                                (DeeObject *const *)self->tp_mro,
		                                count + 1))
			goto err;
	}

	/* Heap-allocated V-tables */
#define PUT_VTABLE(field, desc)                                          \
	if (DeeSerial_PutVTable(writer, ADDROF(field), desc,                 \
	                        sizeof(*self->field) / sizeof(Dee_funptr_t), \
	                        (Dee_funptr_t const **)&self->field, self))  \
		goto err
	PUT_VTABLE(tp_math, type_math_vtable_desc);
	if (self->tp_cmp == &instance_builtin_cmp) {
		/* Special handling for built-in operator table */
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(tp_cmp), &instance_builtin_cmp))
			goto err;
	} else {
		PUT_VTABLE(tp_cmp, type_cmp_vtable_desc);
	}
	PUT_VTABLE(tp_seq, type_seq_vtable_desc);
	PUT_VTABLE(tp_iterator, type_iterator_vtable_desc);
	PUT_VTABLE(tp_attr, type_attr_vtable_desc);
	PUT_VTABLE(tp_with, type_with_vtable_desc);
	if (self->tp_callable == &instance_user_callable) {
		/* Special handling for built-in operator table */
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(tp_callable), &instance_user_callable))
			goto err;
	} else {
		PUT_VTABLE(tp_callable, type_callable_vtable_desc);
	}
#undef PUT_VTABLE

	/* Directly embedded function pointers */
#define PUT_FUNPTR(field)                                          \
	if (DeeSerial_XPutFuncPtr(writer, ADDROF(field), self->field)) \
		goto err
	DeeType_InheritConstructors(self);
	PUT_FUNPTR(tp_init.tp_alloc.tp_ctor);
	PUT_FUNPTR(tp_init.tp_alloc.tp_copy_ctor);
	PUT_FUNPTR(tp_init.tp_alloc.tp_any_ctor);
	PUT_FUNPTR(tp_init.tp_alloc.tp_any_ctor_kw);
	PUT_FUNPTR(tp_init.tp_alloc.tp_serialize);
	PUT_FUNPTR(tp_init.tp_alloc.tp_free);
	if (self->tp_init.tp_alloc.tp_free)
		PUT_FUNPTR(tp_init.tp_alloc.tp_alloc);
	PUT_FUNPTR(tp_init.tp_dtor);
	if (DeeSerial_PutFuncPtr(writer, ADDROF(tp_init.tp_destroy), DeeType_RequireDestroy(self)))
		goto err;
	PUT_FUNPTR(tp_visit);
#undef PUT_FUNPTR

	/* Non-owned structure pointers */
#define PUT_STRUCTPTR(field)                                       \
	if (DeeSerial_XPutPointer(writer, ADDROF(field), self->field)) \
		goto err
	PUT_STRUCTPTR(tp_gc);
	PUT_STRUCTPTR(tp_buffer);
	PUT_STRUCTPTR(tp_methods);
	PUT_STRUCTPTR(tp_getsets);
	PUT_STRUCTPTR(tp_members);
	PUT_STRUCTPTR(tp_class_methods);
	PUT_STRUCTPTR(tp_class_getsets);
	PUT_STRUCTPTR(tp_class_members);
	PUT_STRUCTPTR(tp_method_hints);
#undef PUT_STRUCTPTR

	/* Directly embedded operators */
#define PUT_TNO(field, id)                                                              \
	if (DeeSerial_XPutFuncPtr(writer, ADDROF(field), DeeType_GetAbiOperator(self, id))) \
		goto err
	PUT_TNO(tp_init.tp_assign, Dee_TNO_assign);
	PUT_TNO(tp_init.tp_move_assign, Dee_TNO_move_assign);
	PUT_TNO(tp_cast.tp_str, Dee_TNO_str);
	PUT_TNO(tp_cast.tp_repr, Dee_TNO_repr);
	PUT_TNO(tp_cast.tp_bool, Dee_TNO_bool);
	PUT_TNO(tp_cast.tp_print, Dee_TNO_print);
	PUT_TNO(tp_cast.tp_printrepr, Dee_TNO_printrepr);
	PUT_TNO(tp_iter_next, Dee_TNO_iter_next);
	PUT_TNO(tp_call, Dee_TNO_call);
#undef PUT_TNO
	if (self->tp_class) {
		Dee_seraddr_t cdesc_addr;
		if (self->tp_operators_size) {
			Dee_seraddr_t ops_addr;
			ops_addr = type_serialize_custom_operators(writer, self->tp_operators,
			                                           self->tp_operators_size);
			if unlikely(!Dee_SERADDR_ISOK(ops_addr))
				goto err;
			if (DeeSerial_PutAddr(writer, ADDROF(tp_operators), ops_addr))
				goto err;
		}
		cdesc_addr = class_desc_serialize(self->tp_class, writer);
		if unlikely(!Dee_SERADDR_ISOK(cdesc_addr))
			goto err;
		return DeeSerial_PutAddr(writer, ADDROF(tp_class), cdesc_addr);
	} else if (self->tp_operators_size) {
		ASSERT(self->tp_operators);
		return DeeSerial_PutPointer(writer, ADDROF(tp_operators), self->tp_operators);
	}
	return 0;
err:
	return -1;
#undef ADDROF
}


PRIVATE NONNULL((1, 2)) void DCALL
type_visit(DeeTypeObject *__restrict self,
           Dee_visit_t proc, void *arg) {
	if (DeeType_IsClass(self))
		class_visit(self, proc, arg);
	if (self->tp_mro != NULL) {
		size_t i;
		for (i = 0; self->tp_mro[i] != NULL; ++i)
			Dee_Visit(self->tp_mro[i]);
	}
	Dee_XVisit(self->tp_base);
}

PRIVATE NONNULL((1)) void DCALL
type_clear(DeeTypeObject *__restrict self) {
	if (DeeType_IsClass(self))
		class_clear(self);
}

PRIVATE NONNULL((1)) void DCALL
type_pclear(DeeTypeObject *__restrict self,
            unsigned int gc_priority) {
	if (DeeType_IsClass(self))
		class_pclear(self, gc_priority);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_baseof(DeeTypeObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("baseof", params: "
	DeeTypeObject *other;
", docStringPrefix: "type");]]]*/
#define type_baseof_params "other:?."
	struct {
		DeeTypeObject *other;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__other, "o:baseof", &args))
		goto err;
/*[[[end]]]*/
	if (!DeeType_Check(args.other))
		return_false;
	return_bool01(DeeType_Extends(args.other, self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_extends(DeeTypeObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("extends", params: "
	DeeTypeObject *other;
", docStringPrefix: "type");]]]*/
#define type_extends_params "other:?."
	struct {
		DeeTypeObject *other;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__other, "o:extends", &args))
		goto err;
/*[[[end]]]*/
	return_bool01(DeeType_Extends(self, args.other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_implements(DeeTypeObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("implements", params: "
	DeeTypeObject *other;
", docStringPrefix: "type");]]]*/
#define type_implements_params "other:?."
	struct {
		DeeTypeObject *other;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__other, "o:implements", &args))
		goto err;
/*[[[end]]]*/
	return_bool01(DeeType_Implements(self, args.other));
err:
	return NULL;
}


PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_init_var_type(DeeTypeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot instantiate variable-length type %k",
	                       self);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_missing_mandatory_init(DeeTypeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Missing initializer for mandatory base-type %k",
	                       self);
}

PRIVATE char const str_shared_ctor_failed[] = "Constructor of shared object failed";
INTDEF NONNULL((1)) void DCALL
instance_clear_members(struct Dee_instance_desc *__restrict self, uint16_t size);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_new_raw(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	DeeTypeObject *first_base;
	if unlikely(DeeType_IsVariable(self)) {
		err_init_var_type(self);
		goto err;
	}
	result = DeeType_AllocInstance(self);
	if unlikely(!result)
		goto err;
	DeeObject_Init((GenericObject *)result, self);

	/* Search for the first non-class base. */
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct Dee_class_desc *desc = DeeClass_DESC(first_base);
		struct Dee_instance_desc *instance = DeeInstance_DESC(desc, result);
		Dee_atomic_rwlock_init(&instance->id_lock);
		bzeroc(instance->id_vtab,
		       desc->cd_desc->cd_imemb_size,
		       sizeof(DREF DeeObject *));
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}

	/* Instantiate non-base types. */
	if (!first_base || first_base == &DeeObject_Type) {
done:
		if (DeeType_IsGC(self))
			result = DeeGC_Track(result);
		return result;
	}
	if (first_base->tp_init.tp_alloc.tp_ctor) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_ctor:
		if unlikely((*first_base->tp_init.tp_alloc.tp_ctor)(result))
			goto err_r;
		goto done;
	}
	if (first_base->tp_init.tp_alloc.tp_any_ctor) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_any_ctor:
		if unlikely((*first_base->tp_init.tp_alloc.tp_any_ctor)(result, 0, NULL))
			goto err_r;
		goto done;
	}
	if (first_base->tp_init.tp_alloc.tp_any_ctor_kw) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_any_ctor_kw:
		if unlikely((*first_base->tp_init.tp_alloc.tp_any_ctor_kw)(result, 0, NULL, NULL))
			goto err_r;
		goto done;
	}
	if (DeeType_InheritConstructors(first_base)) {
		if (first_base->tp_init.tp_alloc.tp_ctor)
			goto invoke_base_ctor;
		if (first_base->tp_init.tp_alloc.tp_any_ctor)
			goto invoke_base_any_ctor;
		if (first_base->tp_init.tp_alloc.tp_any_ctor_kw)
			goto invoke_base_any_ctor_kw;
	}
	err_missing_mandatory_init(first_base);
	goto err_r;
err_r:
	if (!DeeObject_UndoConstruction(first_base, result)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return result;
	}
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct Dee_class_desc *desc = DeeClass_DESC(first_base);
		struct Dee_instance_desc *instance = DeeInstance_DESC(desc, result);
		instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}
	Dee_DecrefNokill(self);
	DeeType_FreeInstance(self, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
set_basic_member(DeeTypeObject *tp_self, DeeObject *self,
                 DeeStringObject *member_name, DeeObject *value) {
	int temp;
	DeeTypeObject *iter   = tp_self;
	char const *attr_name = DeeString_STR(member_name);
	Dee_hash_t attr_hash  = DeeString_Hash(member_name);
	if ((temp = DeeType_SetBasicCachedAttrStringHash(tp_self, self, attr_name, attr_hash, value)) <= 0)
		goto done_temp;
	do {
		if (DeeType_IsClass(iter)) {
			struct Dee_class_attribute *attr;
			struct Dee_instance_desc *instance;
			struct Dee_class_desc *desc;
			DREF DeeObject *old_value;
			attr = DeeType_QueryAttributeHash(tp_self, iter,
			                                  (DeeObject *)member_name,
			                                  attr_hash);
			if (!attr)
				goto next_base;
			if (attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FCLASSMEM |
			                     Dee_CLASS_ATTRIBUTE_FGETSET))
				goto next_base;
			desc     = DeeClass_DESC(iter);
			instance = DeeInstance_DESC(desc, self);
			Dee_Incref(value);
			Dee_instance_desc_lock_write(instance);
			old_value = instance->id_vtab[attr->ca_addr];
			instance->id_vtab[attr->ca_addr] = value;
			Dee_instance_desc_lock_endwrite(instance);
			if unlikely(old_value)
				Dee_Decref(old_value);
			return 0;
		}
		if (iter->tp_members &&
		    (temp = DeeType_SetMemberAttrStringHash(tp_self, iter, self, attr_name, attr_hash, value)) <= 0)
			goto done_temp;
next_base:
		;
	} while ((iter = DeeType_Base(iter)) != NULL);
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Could not find member %k in %k, or its bases",
	                       member_name, tp_self);
done_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
set_private_basic_member(DeeTypeObject *tp_self, DeeObject *self,
                         DeeStringObject *member_name, DeeObject *value) {
	int temp;
	char const *attr_name = DeeString_STR(member_name);
	Dee_hash_t attr_hash  = DeeString_Hash(member_name);
	if (DeeType_IsClass(tp_self)) {
		struct Dee_class_attribute *attr;
		struct Dee_instance_desc *instance;
		struct Dee_class_desc *desc = DeeClass_DESC(tp_self);
		DREF DeeObject *old_value;
		attr = DeeClassDesc_QueryInstanceAttributeStringHash(desc,
		                                                     attr_name,
		                                                     attr_hash);
		if (!attr)
			goto not_found;
		if (attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FCLASSMEM |
		                     Dee_CLASS_ATTRIBUTE_FGETSET))
			goto not_found;
		instance = DeeInstance_DESC(desc, self);
		Dee_Incref(value);
		Dee_instance_desc_lock_write(instance);
		old_value = instance->id_vtab[attr->ca_addr];
		instance->id_vtab[attr->ca_addr] = value;
		Dee_instance_desc_lock_endwrite(instance);
		if unlikely(old_value)
			Dee_Decref(old_value);
		return 0;
	}
	if (tp_self->tp_members &&
	    (temp = DeeType_SetMemberAttrStringHash(tp_self, tp_self, self, attr_name, attr_hash, value)) <= 0)
		goto done_temp;
not_found:
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Could not find member %k in %k",
	                       member_name, tp_self);
done_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
unpack_init_info(DeeObject *__restrict info,
                 DREF DeeObject **__restrict p_init_fields,
                 DREF DeeObject **__restrict p_init_args,
                 DREF DeeObject **__restrict p_init_kw) {
	DREF DeeObject *fields_args_kw[3];
	size_t n_args = DeeObject_InvokeMethodHint(seq_unpack_ex, info, 1, 3, fields_args_kw);
	if unlikely(n_args == (size_t)-1)
		goto err;
	ASSERT(n_args >= 1 && n_args <= 3);
	if (n_args < 3) {
		fields_args_kw[2] = NULL;
	} else if (DeeNone_Check(fields_args_kw[2])) {
		Dee_DecrefNokill(fields_args_kw[2]);
		fields_args_kw[2] = NULL;
	}
	if (n_args < 2) {
		fields_args_kw[1] = DeeTuple_NewEmpty();
	} else if (DeeNone_Check(fields_args_kw[1])) {
		Dee_DecrefNokill(fields_args_kw[1]);
		fields_args_kw[1] = DeeTuple_NewEmpty();
	}
	if (DeeNone_Check(fields_args_kw[0])) {
		Dee_DecrefNokill(fields_args_kw[0]);
		fields_args_kw[0] = NULL;
	}
	*p_init_fields = fields_args_kw[0];
	*p_init_args   = fields_args_kw[1];
	*p_init_kw     = fields_args_kw[2];
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unpack_init_info1(DeeObject *__restrict info) {
	DREF DeeObject *init_fields;
	DREF DeeObject *init_argv;
	DREF DeeObject *init_kw;
	if unlikely(unpack_init_info(info, &init_fields, &init_argv, &init_kw))
		goto err;
	Dee_XDecref(init_kw);
	Dee_Decref(init_argv);
	if (!init_fields)
		init_fields = ITER_DONE;
	return init_fields;
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
type_invoke_base_constructor(DeeTypeObject *tp_self, DeeObject *self,
                             size_t argc, DeeObject *const *argv,
                             DeeObject *kw);

struct assign_init_fields_data {
	DeeTypeObject *aifd_tp_self;
	DeeObject     *aifd_self;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
assign_init_fields_foreach(void *arg, DeeObject *key, DeeObject *value) {
	struct assign_init_fields_data *me = (struct assign_init_fields_data *)arg;
	Dee_ssize_t result = DeeObject_AssertTypeExact(key, &DeeString_Type);
	if likely(result == 0) {
		result = set_basic_member(me->aifd_tp_self, me->aifd_self,
		                          (DeeStringObject *)key, value);
	}
	return result;
}
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
assign_init_fields(DeeTypeObject *tp_self, DeeObject *self, DeeObject *fields) {
	struct assign_init_fields_data data;
	data.aifd_tp_self = tp_self;
	data.aifd_self    = self;
	return (int)DeeObject_ForeachPair(fields, &assign_init_fields_foreach, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_new_extended(DeeTypeObject *self, DeeObject *initializer) {
	DREF DeeObject *result, *init_info;
	int temp;
	DREF DeeObject *init_fields, *init_args, *init_kw;
	DeeTypeObject *first_base, *iter;
	if unlikely(DeeType_IsVariable(self)) {
		err_init_var_type(self);
		goto err;
	}
	result = DeeType_AllocInstance(self);
	if unlikely(!result)
		goto err;
	DeeObject_Init((GenericObject *)result, self);

	/* Search for the first non-class base. */
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct Dee_class_desc *desc = DeeClass_DESC(first_base);
		struct Dee_instance_desc *instance = DeeInstance_DESC(desc, result);
		Dee_atomic_rwlock_init(&instance->id_lock);
		bzeroc(instance->id_vtab,
		       desc->cd_desc->cd_imemb_size,
		       sizeof(DREF DeeObject *));
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}

	/* Instantiate non-base types. */
	if (!first_base || first_base == &DeeObject_Type)
		goto done_fields;

	/* {(Type, ({(string, Object)...}, Tuple))...} */
	/* {(Type, ({(string, Object)...}, Tuple, Mapping))...} */
	init_info = DeeObject_TryGetItem(initializer, (DeeObject *)first_base);
	if unlikely(!init_info)
		goto err_r;
	if (init_info == ITER_DONE) {
		init_args   = DeeTuple_NewEmpty();
		init_fields = NULL;
		init_kw     = NULL;
	} else {
		temp = unpack_init_info(init_info, &init_fields, &init_args, &init_kw);
		Dee_Decref(init_info);
		if unlikely(temp)
			goto err_r;
	}

	/* Invoke the mandatory base-type constructor. */
	temp = type_invoke_base_constructor(first_base, result,
	                                    DeeTuple_SIZE(init_args),
	                                    DeeTuple_ELEM(init_args),
	                                    init_kw);
	Dee_XDecref(init_kw);
	Dee_Decref(init_args);
	if likely(!temp && init_fields)
		temp = assign_init_fields(first_base, result, init_fields);
	Dee_XDecref(init_fields);
	if unlikely(temp)
		goto err_r_firstbase;
done_fields:

	/* Fill in all of the fields of non-first-base types. */
	iter = self;
	do {
		if (iter == first_base)
			continue;
		init_info = DeeObject_TryGetItem(initializer, (DeeObject *)iter);
		if unlikely(!init_info)
			goto err_r_firstbase;
		if (init_info == ITER_DONE)
			continue;
		if (DeeNone_Check(init_info)) {
			Dee_DecrefNokill(init_info);
			continue;
		}
		init_fields = unpack_init_info1(init_info);
		Dee_Decref(init_info);
		if (init_fields == ITER_DONE)
			continue;
		if unlikely(!init_fields)
			goto err_r_firstbase;
		temp = assign_init_fields(iter, result, init_fields);
		Dee_Decref(init_fields);
		if unlikely(temp)
			goto err_r_firstbase;
	} while ((iter = DeeType_Base(iter)) != NULL);
	if (DeeType_IsGC(self))
		result = DeeGC_Track(result);
	return result;
err_r_firstbase:
	if (!DeeObject_UndoConstruction(first_base, result)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return result;
	}
err_r:
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct Dee_class_desc *desc = DeeClass_DESC(first_base);
		struct Dee_instance_desc *instance = DeeInstance_DESC(desc, result);
		instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}
	Dee_DecrefNokill(self);
	DeeType_FreeInstance(self, result);
err:
	return NULL;
}

struct assign_private_basic_members_data {
	DeeTypeObject *apbmd_tp_self;
	DeeObject     *apbmd_self;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
assign_private_basic_members_foreach(void *arg, DeeObject *key, DeeObject *value) {
	struct assign_private_basic_members_data *me = (struct assign_private_basic_members_data *)arg;
	Dee_ssize_t result = DeeObject_AssertTypeExact(key, &DeeString_Type);
	if likely(result == 0) {
		result = set_private_basic_member(me->apbmd_tp_self, me->apbmd_self,
		                                  (DeeStringObject *)key, value);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
assign_private_basic_members(DeeTypeObject *tp_self, DeeObject *self, DeeObject *fields) {
	struct assign_private_basic_members_data data;
	data.apbmd_tp_self = tp_self;
	data.apbmd_self    = self;
	return (int)DeeObject_ForeachPair(fields, &assign_private_basic_members_foreach, &data);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_newinstance(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	if (self == &DeeNone_Type)
		return_none; /* Allow `none' to be instantiated with whatever you throw at it! */
	if (kw && (DeeKwds_Check(kw) ? (argc == DeeKwds_SIZE(kw)) : (argc == 0))) {
		/* Instantiate using keyword arguments. */
		result = type_new_raw(self);
		/* Fill in values for provided fields. */
		if (DeeKwds_Check(kw)) {
			size_t i;
			DeeKwdsObject *kwds = (DeeKwdsObject *)kw;
			for (i = 0; i <= kwds->kw_mask; ++i) {
				struct Dee_kwds_entry *ke = &kwds->kw_map[i];
				if (!ke->ke_name)
					continue;
				ASSERT(ke->ke_index < argc);
				if unlikely(set_private_basic_member(self, result,
				                                     ke->ke_name,
				                                     argv[ke->ke_index]))
					goto err_r;
			}
		} else {
			if unlikely(assign_private_basic_members(self, result, kw))
				goto err_r;
		}
		return result;
	}

	/* Without any arguments, simply construct an
	 * empty instance (with all members unbound) */
	if (!argc)
		return type_new_raw(self);
	if (argc != 1) {
		err_invalid_argc("newinstance", argc, 0, 1);
		goto err;
	}

	/* Extended constructors! */
	return type_new_extended(self, argv[0]);
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_getinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("getinstanceattr", params: "
	DeeStringObject *attr;
", docStringPrefix: "type");]]]*/
#define type_getinstanceattr_params "attr:?Dstring"
	struct {
		DeeStringObject *attr;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__attr, "o:getinstanceattr", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;
	return DeeType_GetInstanceAttr(self, (DeeObject *)args.attr);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_callinstanceattr(DeeTypeObject *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	if unlikely(!argc) {
		err_invalid_argc_va("callinstanceattr", argc, 1);
		goto err;
	}
	if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		goto err;
	return DeeType_CallInstanceAttrKw(self, argv[0], argc - 1, argv + 1, kw);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	int result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("hasinstanceattr", params: "
	DeeStringObject *attr;
", docStringPrefix: "type");]]]*/
#define type_hasinstanceattr_params "attr:?Dstring"
	struct {
		DeeStringObject *attr;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__attr, "o:hasinstanceattr", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;
	result = DeeType_HasInstanceAttr(self, (DeeObject *)args.attr);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_boundinstanceattr(DeeTypeObject *self, size_t argc,
                       DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("boundinstanceattr", params: "
	DeeStringObject *attr;
	bool allow_missing = true;
", docStringPrefix: "type");]]]*/
#define type_boundinstanceattr_params "attr:?Dstring,allow_missing=!t"
	struct {
		DeeStringObject *attr;
		bool allow_missing;
	} args;
	args.allow_missing = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__attr_allow_missing, "o|b:boundinstanceattr", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;

	/* Instance attributes of types are always bound (because they're all wrappers) */
	switch (DeeType_BoundInstanceAttr(self, (DeeObject *)args.attr)) {
	default:
		if unlikely(!args.allow_missing) {
			DeeRT_ErrUnknownTypeInstanceAttr(self, args.attr, DeeRT_ATTRIBUTE_ACCESS_BOUND);
			goto err;
		}
		ATTR_FALLTHROUGH
	case Dee_BOUND_NO:
		return_false;
	case Dee_BOUND_YES:
		return_true;
	case Dee_BOUND_ERR:
		goto err;
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_delinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("delinstanceattr", params: "
	DeeStringObject *attr;
", docStringPrefix: "type");]]]*/
#define type_delinstanceattr_params "attr:?Dstring"
	struct {
		DeeStringObject *attr;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__attr, "o:delinstanceattr", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;
	if (DeeType_DelInstanceAttr(self, (DeeObject *)args.attr))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_setinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("setinstanceattr", params: "
	DeeStringObject *attr;
	DeeObject *value;
", docStringPrefix: "type");]]]*/
#define type_setinstanceattr_params "attr:?Dstring,value"
	struct {
		DeeStringObject *attr;
		DeeObject *value;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__attr_value, "oo:setinstanceattr", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;
	if (DeeType_SetInstanceAttr(self, (DeeObject *)args.attr, args.value))
		goto err;
	return_reference_(args.value);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
impl_type_hasprivateattribute_string_hash(DeeTypeObject *__restrict self,
                                          char const *name_str,
                                          Dee_hash_t name_hash) {
	/* TODO: Lookup the attribute in the member cache, and
	 *       see which type is set as the declaring type! */
	if (DeeType_IsClass(self)) {
		struct Dee_class_desc *desc = DeeClass_DESC(self);
		if (DeeClassDesc_QueryInstanceAttributeStringHash(desc, name_str, name_hash) != NULL)
			goto found;
	} else {
		if (self->tp_methods &&
		    DeeType_HasMethodAttrStringHash(self, self, name_str, name_hash))
			goto found;
		if (self->tp_getsets &&
		    DeeType_HasGetSetAttrStringHash(self, self, name_str, name_hash))
			goto found;
		if (self->tp_members &&
		    DeeType_HasMemberAttrStringHash(self, self, name_str, name_hash))
			goto found;
	}
	return false;
found:
	return true;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasattribute(DeeTypeObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	char const *name_str;
	Dee_hash_t name_hash;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr, "o:hasattribute", &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	name_str  = DeeString_STR(name);
	name_hash = DeeString_Hash(name);
	if (!self->tp_attr) {
		DeeTypeMRO mro;
		DeeTypeObject *iter;
		if (DeeType_HasCachedAttrStringHash(self, name_str, name_hash))
			goto found;
		iter = self;
		DeeTypeMRO_Init(&mro, iter);
		for (;;) {
			if (DeeType_IsClass(iter)) {
				if (DeeType_QueryInstanceAttributeHash(self, iter, name, name_hash) != NULL)
					goto found;
			} else {
				if (iter->tp_methods &&
				    DeeType_HasMethodAttrStringHash(self, iter, name_str, name_hash))
					goto found;
				if (iter->tp_getsets &&
				    DeeType_HasGetSetAttrStringHash(self, iter, name_str, name_hash))
					goto found;
				if (iter->tp_members &&
				    DeeType_HasMemberAttrStringHash(self, iter, name_str, name_hash))
					goto found;
			}
			iter = DeeTypeMRO_Next(&mro, iter);
			if (!iter)
				break;
			if (iter->tp_attr)
				break;
		}
	}
	return_false;
found:
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasprivateattribute(DeeTypeObject *self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr,
	                    "o:hasprivateattribute", &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return_bool(!self->tp_attr &&
	            impl_type_hasprivateattribute_string_hash(self,
	                                                      DeeString_STR(name),
	                                                      DeeString_Hash(name)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasoperator(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	Dee_operator_t opid;
	size_t op_argc = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__name_argc,
	                    "o|" UNPuSIZ ":hasoperator",
	                    &name, &op_argc))
		goto err;
	if (DeeString_Check(name)) {
		struct Dee_opinfo const *info;
		info = DeeTypeType_GetOperatorByName(Dee_TYPE(self),
		                                     DeeString_STR(name),
		                                     op_argc);
		if (info == NULL)
			goto nope;
		opid = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(name, &opid))
			goto err;
	}
	if (DeeType_HasOperator(self, opid))
		return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasprivateoperator(DeeTypeObject *self, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	Dee_operator_t opid;
	size_t op_argc = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__name_argc,
	                    "o|" UNPuSIZ ":hasprivateoperator",
	                    &name, &op_argc))
		goto err;
	if (DeeString_Check(name)) {
		struct Dee_opinfo const *info;
		info = DeeTypeType_GetOperatorByName(Dee_TYPE(self),
		                                     DeeString_STR(name),
		                                     op_argc);
		if (info == NULL)
			goto nope;
		opid = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(name, &opid))
			goto err;
	}
	if (DeeType_HasPrivateOperator(self, opid))
		return_true;
nope:
	return_false;
err:
	return NULL;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED DREF DeeObject *DCALL
type_extends_not_same(DeeTypeObject *self, size_t argc,
                          DeeObject *const *argv) {
	DeeTypeObject *other;
	DeeArg_Unpack1(err, argc, argv, "derived_from", &other);
	return_bool(self != other && DeeType_Extends(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_vartype(DeeTypeObject *self, size_t argc,
                DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_vartype");
	return_bool(DeeType_IsVariable(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_heaptype(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_heaptype");
	return_bool(DeeType_IsHeapType(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_gctype(DeeTypeObject *self, size_t argc,
               DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_gctype");
	return_bool(DeeType_IsGC(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_final(DeeTypeObject *self, size_t argc,
              DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_final");
	return_bool(DeeType_IsFinal(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_class(DeeTypeObject *self, size_t argc,
              DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_class");
	return_bool(DeeType_IsClass(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_complete(DeeTypeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_complete");
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_classtype(DeeTypeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_class_type");
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_is_ctypes_class(DeeTypeObject *__restrict self,
                     char const *__restrict name) {
	DREF DeeObject *temp;
	int error;
	temp = DeeObject_GetAttrString(Dee_AsObject(self), "isstructured");
	if unlikely(!temp)
		goto err;
	error = DeeObject_BoolInherited(temp);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto nope;
	temp = DeeObject_GetAttrString(Dee_AsObject(self), name);
	if unlikely(!temp)
		goto err;
	error = DeeObject_BoolInherited(temp);
	if unlikely(error < 0)
		goto err;
	if (error)
		return_true;
nope:
	return_false;
err:
	if (DeeError_Catch(&DeeError_AttributeError))
		goto nope;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_pointer(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_pointer");
	return type_is_ctypes_class(self, "ispointer");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_lvalue(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_pointer");
	return type_is_ctypes_class(self, "islvalue");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_structured(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_structured");
	return DeeObject_GetAttrString(Dee_AsObject(self), "isstructured");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_struct(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_struct");
	return type_is_ctypes_class(self, "isstruct");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_array(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_array");
	return type_is_ctypes_class(self, "isarray");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_foreign_function(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_foreign_function");
	return type_is_ctypes_class(self, "isfunction");
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_filetype(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_file");
	return_bool(Dee_TYPE(self) == &DeeFileType_Type);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_superbase(DeeTypeObject *self, size_t argc,
                  DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "is_super_base");
	return_bool(DeeType_Base(self) == NULL);
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_method tpconst type_methods[] = {
	TYPE_KWMETHOD_F("baseof", &type_baseof, METHOD_FNOREFESCAPE,
	                "(" type_baseof_params ")->?Dbool\n"
	                "Returns ?t if @this ?. is equal to, or a base of @other.\n"
	                "If @other isn't a ?., ?f is returned.\n"
	                "Using baseof, the behavior of ${x is y} can be approximated as:\n"
	                "${"
	                /**/ "print y.baseof(type(x)); /* aka: `print x is y;' */"
	                "}"),
	TYPE_KWMETHOD_F("extends", &type_extends, METHOD_FNOREFESCAPE,
	                "(" type_extends_params ")->?Dbool\n"
	                "Returns ?t if @this ?. is equal to, or has been derived from @other.\n"
	                "If @other isn't a ?., ?f is returned."),
	TYPE_KWMETHOD_F("implements", &type_implements, METHOD_FNOREFESCAPE,
	                "(" type_implements_params ")->?Dbool\n"
	                "Check if @other appears in ?#__mro__"),
	TYPE_KWMETHOD("newinstance", &type_newinstance,
	              "(fields!!)->\n"
	              "Allocate a new instance of @this ?. and initialize members in accordance to @fields.\n"
	              "${"
	              /**/ "class MyClass {\n"
	              /**/ "	member foo;\n"
	              /**/ "	this = del; /* Delete the regular constructor. */\n"
	              /**/ "}\n"
	              /**/ "local x = MyClass.newinstance(foo: 42);\n"
	              /**/ "print x.foo;\n"
	              "}"
	              "\n"

	              "(initializer:?S?T2?.?T1?S?T2?Dstring?O=!N)->\n"                 /* {(Type, ({(string,Object)...},)...} */
	              "(initializer:?S?T2?.?T2?S?T2?Dstring?O?N=!N)->\n"               /* {(Type, ({(string,Object)...}, none)...} */
	              "(initializer:?S?T2?.?T2?S?T2?Dstring?O?DTuple=!N)->\n"          /* {(Type, ({(string,Object)...}, Tuple)...} */
	              "(initializer:?S?T2?.?T3?S?T2?Dstring?O?DTuple?N=!N)->\n"        /* {(Type, ({(string,Object)...}, Tuple, none)...} */
	              "(initializer:?S?T2?.?T3?S?T2?Dstring?O?DTuple?DMapping=!N)->\n" /* {(Type, ({(string,Object)...}, Tuple, Mapping)...} */
	              "(initializer:?S?T2?.?T2?N?DTuple=!N)->\n"                       /* {(Type, (none, Tuple)...} */
	              "(initializer:?S?T2?.?T3?N?DTuple?N=!N)->\n"                     /* {(Type, (none, Tuple, none)...} */
	              "(initializer:?S?T2?.?T3?N?DTuple?DMapping=!N)->\n"              /* {(Type, (none, Tuple, Mapping)...} */
	              "#tTypeError{No superargs tuple was provided for one of the type's bases, when that base "
	              /*            */ "has a mandatory constructor that can't be invoked without any arguments. "
	              /*            */ "Note that a user-defined class never has a mandatory constructor, with this "
	              /*            */ "only affecting builtin types such as ?DInstanceMethod or ?DProperty}"
	              "A extended way of constructing and initializing a ?., that involves providing explicit "
	              /**/ "member initializers on a per-Type bases, as well as argument tuples and optional keyword "
	              /**/ "mappings to-be used for construction of one of the type's super-classes (allowing to provide "
	              /**/ "for explicit argument lists when one of the type's bases has a mandatory constructor)\n"
	              "${"
	              /**/ "import List from deemon;\n"
	              /**/ "class MyList: List {\n"
	              /**/ "	this = del; /* Delete the regular constructor. */\n"
	              /**/ "	member mylist_member;\n"
	              /**/ "	function appendmember() {\n"
	              /**/ "		this.append(mylist_member);\n"
	              /**/ "	}\n"
	              /**/ "}\n"
	              /**/ "local x = MyList.newinstance({\n"
	              /**/ "	MyList: ({ \"mylist_member\" : \"abc\" }, none),\n"
	              /**/ "	List:   ({ }, pack([10, 20, 30])),\n"
	              /**/ "});\n"
	              /**/ "print repr x;          /* [10, 20, 30] */\n"
	              /**/ "print x.mylist_member; /* \"abc\" */\n"
	              /**/ "x.appendmember();\n"
	              /**/ "print repr x;          /* [10, 20, 30, \"abc\"] */"
	              "}"),
	TYPE_KWMETHOD_F("hasattribute", &type_hasattribute, METHOD_FNOREFESCAPE,
	                "(name:?Dstring)->?Dbool\n"
	                "Returns ?t if this type, or one of its super-classes defines an "
	                /**/ "instance-attribute @name, and doesn't define any attribute-operators. "
	                /**/ "Otherwise, return ?f:\n"
	                "${"
	                /**/ "function hasattribute(name: string): bool {\n"
	                /**/ "	import attribute from deemon;\n"
	                /**/ "	return attribute.exists(this, name, \"ic\", \"ic\")\n"
	                /**/ "}"
	                "}\n"
	                "Note that this function only searches instance-attributes, meaning that class/static "
	                /**/ "attributes/members such as ?AIterator?Dstring. are not matched, whereas something \n"
	                /**/ "like ?Afind?Dstring is.\n"
	                "Note that this function is quite similar to #hasinstanceattr, however unlike "
	                /**/ "that function, this function will stop searching the base-classes of @this ?. "
	                /**/ "when one of that types implements one of the attribute operators."),
	TYPE_KWMETHOD_F("hasprivateattribute", &type_hasprivateattribute, METHOD_FNOREFESCAPE,
	                "(name:?Dstring)->?Dbool\n"
	                "Similar to #hasattribute, but only looks at attributes declared "
	                /**/ "by @this ?., excluding any defined by a super-class.\n"
	                "${"
	                /**/ "function hasprivateattribute(name: string): bool {\n"
	                /**/ "	import attribute from deemon;\n"
	                /**/ "	return attribute.exists(this, name, \"ic\", \"ic\", this)\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("hasoperator", &type_hasoperator, METHOD_FNOREFESCAPE,
	                "(name:?Dint)->?Dbool\n"
	                "(name:?Dstring)->?Dbool\n"
	                "(name:?Dstring,argc:?Dint)->?Dbool\n"
	                "Returns ?t if instances of @this ?. implement an operator @name, "
	                /**/ "or ?f if not, or if @name is not recognized as an operator "
	                /**/ "available for the Type-Type that is ${type this}\n"
	                "Note that this function also looks at the operators of "
	                /**/ "base-classes, as well as that a user-defined class that has "
	                /**/ "explicitly deleted an operator will cause this function to "
	                /**/ "return true, indicative of that operator being implemented "
	                /**/ "to cause an error to be thrown when invoked.\n"
	                "The given @name is the so-called real operator name, "
	                /**/ "as listed under Name in the following table:\n"
	                "#T{Name|Symbolical name|Prototype~"
	                /**/ "$\"constructor\"|$\"this\"|${this(args..., **kwds)}&"
	                /**/ "$\"copy\"|$\"copy\"|${copy()}&"
	                /**/ "$\"deepcopy\"|$\"deepcopy\"|${deepcopy()}&"
	                /**/ "$\"destructor\"|$\"#~this\"|${##~this()}&"
	                /**/ "$\"assign\"|$\":=\"|${operator := (other)}&"
	                /**/ "$\"str\"|$\"str\"|${operator str(): string}&"
	                /**/ "$\"repr\"|$\"repr\"|${operator repr(): string}&"
	                /**/ "$\"bool\"|$\"bool\"|${operator bool(): bool}&"
	                /**/ "$\"call\"|$\"()\"|${operator ()(args!): Object}&"
	                /**/ "$\"next\"|$\"next\"|${operator next(): Object}&"
	                /**/ "$\"int\"|$\"int\"|${operator int(): int}&"
	                /**/ "$\"float\"|$\"float\"|${operator float(): float}&"
	                /**/ "$\"inv\"|$\"#~\"|${operator #~ (): Object}&"
	                /**/ "$\"pos\"|$\"+\"|${operator + (): Object}&"
	                /**/ "$\"neg\"|$\"-\"|${operator - (): Object}&"
	                /**/ "$\"add\"|$\"+\"|${operator + (other): Object}&"
	                /**/ "$\"sub\"|$\"-\"|${operator - (other): Object}&"
	                /**/ "$\"mul\"|$\"*\"|${operator * (other): Object}&"
	                /**/ "$\"div\"|$\"/\"|${operator / (other): Object}&"
	                /**/ "$\"mod\"|$\"%\"|${operator % (other): Object}&"
	                /**/ "$\"shl\"|$\"<<\"|${operator << (other): Object}&"
	                /**/ "$\"shr\"|$\">>\"|${operator >> (other): Object}&"
	                /**/ "$\"and\"|$\"#&\"|${operator #& (other): Object}&"
	                /**/ "$\"or\"|$\"#|\"|${operator #| (other): Object}&"
	                /**/ "$\"xor\"|$\"^\"|${operator ^ (other): Object}&"
	                /**/ "$\"pow\"|$\"**\"|${operator ** (other): Object}&"
	                /**/ "$\"inc\"|$\"++\"|${operator ++ (): Object}&"
	                /**/ "$\"dec\"|$\"--\"|${operator -- (): Object}&"
	                /**/ "$\"iadd\"|$\"+=\"|${operator += (other): Object}&"
	                /**/ "$\"isub\"|$\"-=\"|${operator -= (other): Object}&"
	                /**/ "$\"imul\"|$\"*=\"|${operator *= (other): Object}&"
	                /**/ "$\"idiv\"|$\"/=\"|${operator /= (other): Object}&"
	                /**/ "$\"imod\"|$\"%=\"|${operator %= (other): Object}&"
	                /**/ "$\"ishl\"|$\"<<=\"|${operator <<= (other): Object}&"
	                /**/ "$\"ishr\"|$\">>=\"|${operator >>= (other): Object}&"
	                /**/ "$\"iand\"|$\"#&=\"|${operator #&= (other): Object}&"
	                /**/ "$\"ior\"|$\"#|=\"|${operator #|= (other): Object}&"
	                /**/ "$\"ixor\"|$\"^=\"|${operator ^= (other): Object}&"
	                /**/ "$\"ipow\"|$\"**=\"|${operator **= (other): Object}&"
	                /**/ "$\"hash\"|$\"hash\"|${operator hash(): int}&"
	                /**/ "$\"eq\"|$\"==\"|${operator == (other): Object}&"
	                /**/ "$\"ne\"|$\"!=\"|${operator != (other): Object}&"
	                /**/ "$\"lo\"|$\"<\"|${operator < (other): Object}&"
	                /**/ "$\"le\"|$\"<=\"|${operator <= (other): Object}&"
	                /**/ "$\"gr\"|$\">\"|${operator > (other): Object}&"
	                /**/ "$\"ge\"|$\">=\"|${operator >= (other): Object}&"
	                /**/ "$\"iter\"|$\"iter\"|${operator iter(): Object}&"
	                /**/ "$\"size\"|$\"##\"|${operator ## (): Object}&"
	                /**/ "$\"contains\"|$\"contains\"|${operator contains(item): Object}&"
	                /**/ "$\"getitem\"|$\"[]\"|${operator [] (index): Object}&"
	                /**/ "$\"delitem\"|$\"del[]\"|${operator del[] (index): none}&"
	                /**/ "$\"setitem\"|$\"[]=\"|${operator []= (index, value): none}&"
	                /**/ "$\"getrange\"|$\"[:]\"|${operator [:] (start, end): Object}&"
	                /**/ "$\"delrange\"|$\"del[:]\"|${operator del[:] (start, end): none}&"
	                /**/ "$\"setrange\"|$\"[:]=\"|${operator [:]= (start, end, value): none}&"
	                /**/ "$\"getattr\"|$\".\"|${operator . (name: string): Object}&"
	                /**/ "$\"delattr\"|$\"del.\"|${operator del. (name: string): none}&"
	                /**/ "$\"setattr\"|$\".=\"|${operator .= (name: string, value): none}&"
	                /**/ "$\"enumattr\"|$\"enumattr\"|${operator enumattr(): {attribute...}}&"
	                /**/ "$\"enter\"|$\"enter\"|${operator enter(): none}&"
	                /**/ "$\"leave\"|$\"leave\"|${operator leave(): none}"
	                "}"),
	TYPE_KWMETHOD_F("hasprivateoperator", &type_hasprivateoperator, METHOD_FNOREFESCAPE,
	                "(name:?Dint)->?Dbool\n"
	                "(name:?Dstring)->?Dbool\n"
	                "(name:?Dstring,argc:?Dint)->?Dbool\n"
	                "Returns ?t if instances of @this ?. implement an operator @name, "
	                /**/ "or ?f if not, or if @name is not recognized as an operator provided "
	                /**/ "available for the Type-Type that is ${type this}.\n"
	                "Note that this function intentionally don't look at operators of "
	                /**/ "base-classes (which is instead done by #hasoperator), meaning that "
	                /**/ "inherited operators are not included, with the exception of explicitly "
	                /**/ "inherited constructors.\n"
	                "For a list of operator names, see #hasoperator."),
	TYPE_KWMETHOD("getinstanceattr", &type_getinstanceattr,
	              "(" type_getinstanceattr_params ")->\n"
	              "Lookup an attribute @name that is implemented by instances of @this ?.\n"
	              "Normally, such attributes can also be accessed using regular attribute lookup, "
	              /**/ "however in ambiguous cases where both the type, as well as instances implement an "
	              /**/ "attribute of the same name (s.a. ?A{i:isdir}?Eposix:stat vs. ?A{c:isdir}?Eposix:stat), "
	              /**/ "using regular attribute lookup on the type (as in ${posix.stat.isdir}) will always "
	              /**/ "return the type-attribute, rather than a wrapper around the instance attribute.\n"
	              "In such cases, this function may be used to explicitly lookup the instance variant:\n"
	              "${"
	              /**/ "import stat from posix;\n"
	              /**/ "local statIsDirProperty = stat.getinstanceattr(\"isdir\");\n"
	              /**/ "local myStatInstance = stat(\".\");\n"
	              /**/ "// Same as `myStatInstance.isdir' -- true\n"
	              /**/ "print repr statIsDirProperty(myStatInstance);"
	              "}\n"
	              "Note that one minor exception exists to the default lookup rule, and it relates to how "
	              /**/ "attributes of ?. itself are queried (such as in the expression ${(Type from deemon).baseof}).\n"
	              "In this case, access is always made as an instance-bound, meaning that for this purpose, "
	              /**/ "?. is considered an instance of ?. (typetype), rather than the type of ?. (typetype) "
	              /**/ "(I know that sounds complicated, but without this rule, ${(Type from deemon).baseof} would "
	              /**/ "return a class method object taking 2 arguments, rather than the intended single argument).\n"
	              "Also note that the `*instanceattr' functions will not check for types that have overwritten "
	              /**/ "one of the attribute-operators, but will continue search for matching attribute names, even "
	              /**/ "if those attributes would normally have been overshadowed by attribute callbacks."),
	TYPE_KWMETHOD("callinstanceattr", &type_callinstanceattr,
	              "(attr:?Dstring,args!,kwds!!)->\n"
	              "s.a. ?#getinstanceattr"),
	TYPE_KWMETHOD("hasinstanceattr", &type_hasinstanceattr,
	              "(" type_hasinstanceattr_params ")->?Dbool\n"
	              "s.a. ?#getinstanceattr"),
	TYPE_KWMETHOD("boundinstanceattr", &type_boundinstanceattr,
	              "(" type_boundinstanceattr_params ")->?Dbool\n"
	              "s.a. ?#getinstanceattr"),
	TYPE_KWMETHOD("delinstanceattr", &type_delinstanceattr,
	              "(" type_delinstanceattr_params ")\n"
	              "s.a. ?#getinstanceattr"),
	TYPE_KWMETHOD("setinstanceattr", &type_setinstanceattr,
	              "(" type_setinstanceattr_params ")->\n"
	              "s.a. ?#getinstanceattr (always re-returns @value)"),

	TYPE_KWMETHOD_F("derivedfrom", &type_extends, METHOD_FNOREFESCAPE,
	                "(other:?.)->?Dbool\n"
	                "Deprecated alias for ?#extends"),

#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Deprecated functions */
	TYPE_KWMETHOD_F("same_or_derived_from", &type_extends, METHOD_FNOREFESCAPE, "(other:?.)->?Dbool\nDeprecated alias for ?#derivedfrom"),
	TYPE_METHOD_F("derived_from", &type_extends_not_same, METHOD_FNOREFESCAPE, "(other:?.)->?Dbool\nDeprecated alias for ${this !== other && this.derivedfrom(other)}"),
	TYPE_METHOD_F("is_vartype", &type_is_vartype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__isvariable__"),
	TYPE_METHOD_F("is_heaptype", &type_is_heaptype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__iscustom__"),
	TYPE_METHOD_F("is_gctype", &type_is_gctype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__isgc__"),
	TYPE_METHOD_F("is_final", &type_is_final, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#isfinal"),
	TYPE_METHOD_F("is_class", &type_is_class, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__isclass__"),
	TYPE_METHOD_F("is_complete", &type_is_complete, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated (always returns ?t)"),
	TYPE_METHOD_F("is_classtype", &type_is_classtype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated (always returns ?f)"),
	TYPE_METHOD("is_pointer", &type_is_pointer, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.ispointer catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_lvalue", &type_is_lvalue, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.islvalue catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_structured", &type_is_structured, "->?Dbool\nDeprecated alias for ${try this.isstructured catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_struct", &type_is_struct, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.isstruct catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_array", &type_is_array, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.isarray catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_foreign_function", &type_is_foreign_function, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.isfunction catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD_F("is_file", &type_is_filetype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ${this is type(File from deemon)}"),
	TYPE_METHOD_F("is_super_base", &type_is_superbase, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ${this.__base__ is none}"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
type_getname_impl(DeeTypeObject *__restrict self) {
	DREF DeeStringObject *result;
	ASSERT(self->tp_name);
	if (self->tp_flags & TP_FNAMEOBJECT) {
		result = container_of(self->tp_name, DeeStringObject, s_str);
		Dee_Incref(result);
	} else {
		result = (DREF DeeStringObject *)DeeString_New(self->tp_name);
	}
	return result;
}

PRIVATE DEFINE_STRING(str_anonymous_type, "<anonymous type>");

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_getname(DeeTypeObject *__restrict self) {
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	if (self->tp_name == NULL)
		return_reference(&str_anonymous_type);
	return Dee_AsObject(type_getname_impl(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get__name__(DeeTypeObject *__restrict self) {
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	if unlikely(self->tp_name == NULL)
		return DeeRT_ErrTUnboundAttrCStr(&DeeType_Type, self, STR___name__);
	return Dee_AsObject(type_getname_impl(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isbuffer(DeeTypeObject *__restrict self) {
	do {
		if (self->tp_buffer && self->tp_buffer->tp_getbuf)
			return_true;
	} while ((self = self->tp_base) != NULL);
	return_false;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_classdesc(DeeTypeObject *__restrict self) {
	if (!DeeType_IsClass(self))
		return DeeRT_ErrTUnboundAttrCStr(&DeeType_Type, self, "__class__");
	return_reference_(Dee_AsObject(self->tp_class->cd_desc));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_seqclass(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	unsigned int cls = DeeType_GetSeqClass(self);
	switch (cls) {
	case Dee_SEQCLASS_NONE:
		result = Dee_None;
		break;
	case Dee_SEQCLASS_SEQ:
		result = Dee_AsObject(&DeeSeq_Type);
		break;
	case Dee_SEQCLASS_SET:
		result = Dee_AsObject(&DeeSet_Type);
		break;
	case Dee_SEQCLASS_MAP:
		result = Dee_AsObject(&DeeMapping_Type);
		break;
	default: __builtin_unreachable();
	}
	Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_issingleton(DeeTypeObject *__restrict self) {
	if (self->tp_features & TF_SINGLETON)
		return_true; /* Alternative means of creation. */
	if (DeeType_IsClass(self)) {
		/* Special handling for user-defined classes. */
		if (!self->tp_init.tp_alloc.tp_ctor &&
		    !self->tp_init.tp_alloc.tp_any_ctor &&
		    !self->tp_init.tp_alloc.tp_any_ctor_kw)
			return_true; /* The type isn't constructible. */
	}
	return_false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
get_module_from_addr(struct Dee_class_desc *__restrict my_class, uint16_t addr) {
	DeeObject *slot;
	DREF DeeModuleObject *result = NULL;
	Dee_class_desc_lock_read(my_class);
	slot = my_class->cd_members[addr];
	if (slot && DeeFunction_Check(slot)) {
		DeeFunctionObject *slot_function;
		slot_function = (DeeFunctionObject *)slot;
		result        = slot_function->fo_code->co_module;
		Dee_Incref(result);
	}
	Dee_class_desc_lock_endread(my_class);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeClass_GetModule(DeeTypeObject *__restrict self) {
	struct Dee_class_desc *my_class = self->tp_class;
	DeeClassDescriptorObject *desc = my_class->cd_desc;
	DREF DeeModuleObject *result;
	size_t i;

	/* Search through the operator bindings table. */
	for (i = 0; i <= desc->cd_clsop_mask; ++i) {
		if (desc->cd_clsop_list[i].co_name == (Dee_operator_t)-1)
			continue;
		result = get_module_from_addr(my_class,
		                              desc->cd_clsop_list[i].co_addr);
		if (result)
			goto done;
	}

	/* Search through class attribute table. */
	for (i = 0; i <= desc->cd_cattr_mask; ++i) {
		if (!desc->cd_cattr_list[i].ca_name)
			continue;
		result = get_module_from_addr(my_class,
		                              desc->cd_cattr_list[i].ca_addr);
		if (result)
			goto done;
		if ((desc->cd_cattr_list[i].ca_flag &
		     (Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FGETSET)) ==
		    (Dee_CLASS_ATTRIBUTE_FGETSET)) {
			result = get_module_from_addr(my_class,
			                              desc->cd_cattr_list[i].ca_addr + Dee_CLASS_GETSET_DEL);
			if (result)
				goto done;
			result = get_module_from_addr(my_class,
			                              desc->cd_cattr_list[i].ca_addr + Dee_CLASS_GETSET_SET);
			if (result)
				goto done;
		}
	}
	for (i = 0; i <= desc->cd_iattr_mask; ++i) {
		if (!desc->cd_iattr_list[i].ca_name)
			continue;
		if (!(desc->cd_iattr_list[i].ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM))
			continue;
#if Dee_CLASS_GETSET_GET == 0
		result = get_module_from_addr(my_class,
		                              desc->cd_iattr_list[i].ca_addr);
		if (result)
			goto done;
#endif /* Dee_CLASS_GETSET_GET == 0 */
		if ((desc->cd_iattr_list[i].ca_flag &
		     (Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FGETSET)) ==
		    (Dee_CLASS_ATTRIBUTE_FGETSET)) {
#if Dee_CLASS_GETSET_GET != 0
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + Dee_CLASS_GETSET_GET);
			if (result)
				goto done;
#endif /* Dee_CLASS_GETSET_GET != 0 */
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + Dee_CLASS_GETSET_DEL);
			if (result)
				goto done;
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + Dee_CLASS_GETSET_SET);
			if (result)
				goto done;
		}
#if Dee_CLASS_GETSET_GET != 0
		else {
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr);
			if (result)
				goto done;
		}
#endif /* Dee_CLASS_GETSET_GET != 0 */
	}
	return NULL;
done:
	return result;
}

/* Return the module used to define a given type `self',
 * or `NULL' if that module could not be determined.
 * NOTE: When `NULL' is returned, _NO_ error is thrown! */
PUBLIC WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeType_GetModule(DeeTypeObject *__restrict self) {
	DREF DeeModuleObject *result;
	/* - For user-defined classes: search though all the operator/method bindings
	 *   described for the class member table, testing them for functions and
	 *   returning the module that they are bound to.
	 * - For types loaded by dex modules, do some platform-specific trickery to
	 *   determine the address space bounds within which the module was loaded,
	 *   then simply compare the type pointer against those bounds.
	 * - All other types are defined as part of the builtin `deemon' module. */
again:
	result = (DREF DeeModuleObject *)Dee_weakref_lock(&self->tp_module);
	if (result != NULL)
		return result;
	if (DeeType_IsClass(self)) {
		result = DeeClass_GetModule(self);
		if (result != NULL)
			Dee_weakref_set(&self->tp_module, Dee_AsObject(result));
		return result;
	}
#ifndef CONFIG_EXPERIMENTAL_MMAP_DEC
	if (!DeeType_IsHeapType(self))
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
	{
		/* Lookup the originating module of a statically allocated C-type. */
		result = DeeModule_OfPointer(self);
		if (result) {
			Dee_weakref_set(&self->tp_module, Dee_AsObject(result));
			return result;
		}
	}

	/* Special case for custom type-types (such
	 * as those provided by the `ctypes' module)
	 *  -> In this case, we simply return the module associated with the
	 *     typetype, thus allowing custom types to be resolved as well. */
	if (self != Dee_TYPE(self)) {
		self = Dee_TYPE(self);
		if (self != &DeeType_Type &&
		    self != &DeeFileType_Type)
			goto again;
	}
	return NULL;
}

/* Returns the `tp_name' of `self', or the string
 * "<anonymous type>" when `self' doesn't have a
 * type name set. */
PUBLIC ATTR_RETNONNULL ATTR_PURE WUNUSED NONNULL((1)) char const *DCALL
DeeType_GetName(DeeTypeObject const *__restrict self) {
	char const *result;
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	result = self->tp_name;
	if (result == NULL)
		result = DeeString_STR(&str_anonymous_type);
	return result;
}


/* Returns the "tp_serialize" operator for "self". If possible, inherit from base class. */
INTERN WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetTpSerialize)(DeeTypeObject *__restrict self) {
	Dee_funptr_t result = self->tp_init._tp_init_._tp_init5_;
	if (!result && DeeType_IsSuperConstructible(self) && self->tp_base) {
		result = DeeType_GetTpSerialize(self->tp_base);
		if (result)
			self->tp_init._tp_init_._tp_init5_ = result;
	}
	return result;
}


/* Returns the "instance-size" of a given object "self",
 * whilst trying to resolve known standard allocators.
 * The caller must ensure that `!DeeType_IsVariable(Dee_TYPE(self))'
 * @return: * : The instance size of "self"
 * @return: 0 : Instance size is unknown (non-standard allocator used) */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) size_t
(DCALL DeeType_GetInstanceSize)(DeeTypeObject const *__restrict self) {
	void (DCALL *tp_free)(void *__restrict ob);
	ASSERT(!DeeType_IsVariable(Dee_TYPE(self)));
	if ((tp_free = self->tp_init.tp_alloc.tp_free) == NULL)
		return self->tp_init.tp_alloc.tp_instance_size;
	/* FIXME: This right here doesn't work in DEX modules when PLT symbols are used.
	 *
	 * This currently breaks serialization for stuff like "collections.UniqueSet".
	 *
	 * The only solution I can see here is to just always disable slab-based alloc
	 * functions in DEX modules, but then the deemon core should be able to lazily
	 * assign slab allocators for DEX types, as those types are used. */
#ifndef CONFIG_NO_OBJECT_SLABS
	if (DeeType_IsGC(self)) {
#define CHECK_ALLOCATOR(index, size)                  \
		if (tp_free == &DeeGCObject_SlabFree##size) { \
			return size * sizeof(void *);             \
		} else
		DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
		{}
	} else {
#define CHECK_ALLOCATOR(index, size)                \
		if (tp_free == &DeeObject_SlabFree##size) { \
			return size * sizeof(void *);           \
		} else
		DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
		{}
	}
#endif /* !CONFIG_NO_OBJECT_SLABS */
	return 0;
}


/* Check if "self" may be considered as deep-immutable.
 * This is a (somewhat smarter) helper-wrapper around "DeeType_IsDeepImmutable()"
 * that does a couple of extra checks for certain types of shallow-immutable
 * objects, like "Tuple" of "_SeqOne". */
PUBLIC ATTR_PURE WUNUSED NONNULL((1)) bool
(DCALL DeeObject_IsDeepImmutable)(DeeObject const *__restrict self) {
	DeeTypeObject *tp;
again:
	tp = Dee_TYPE(self);
	if (DeeType_IsDeepImmutable(tp))
		return true;
	ASSERTF(!DeeType_IsTypeType(tp), "All type-types must be marked with 'TP_FDEEPIMMUTABLE'");

	/* Special handling for certain types... */
	if (tp == &DeeTuple_Type) {
		size_t i;
		DeeTupleObject *me = (DeeTupleObject *)self;
		for (i = 0; i < me->t_size; ++i) {
			if (!DeeObject_IsDeepImmutable(me->t_elem[i]))
				return false;
		}
		return true;
	} else if (tp == &DeeNullableTuple_Type) {
		size_t i;
		DeeTupleObject *me = (DeeTupleObject *)self;
		for (i = 0; i < me->t_size; ++i) {
			if (me->t_elem[i] && !DeeObject_IsDeepImmutable(me->t_elem[i]))
				return false;
		}
		return true;
	} else if (tp == &DeeRoDict_Type) {
		DeeRoDictObject *me = (DeeRoDictObject *)self;
		Dee_hash_vidx_t i;
		for (i = 0; i < me->rd_vsize; ++i) {
			struct Dee_dict_item *it = &me->rd_vtab[i];
			if (it->di_key) {
				if (!DeeObject_IsDeepImmutable(it->di_key))
					return false;
				if (!DeeObject_IsDeepImmutable(it->di_value))
					return false;
			}
		}
		return true;
	} else if (tp == &DeeRoSet_Type) {
		DeeRoSetObject *me = (DeeRoSetObject *)self;
		size_t i;
		for (i = 0; i <= me->rs_mask; ++i) {
			struct Dee_roset_item *it = &me->rs_elem[i];
			if (it->rsi_key && !DeeObject_IsDeepImmutable(it->rsi_key))
				return false;
		}
		return true;
#if 0
	} else if (tp == &DeeSeqOne_Type) { /* Already handled by "ProxyObject" below */
		self = DeeSeqOne_GET(self);
		goto again;
	} else if (tp == &DeeSeqPair_Type) { /* Already handled by "ProxyObject2" below */
		if (!DeeObject_IsDeepImmutable(DeeSeqPair_ELEM(self)[0]))
			return false;
		self = DeeSeqPair_ELEM(self)[1];
		goto again;
#endif
	} else if (tp == &DeeFunction_Type) {
		/* Functions are deep immutable if they do not contain "static" variables */
		DeeFunctionObject *me = (DeeFunctionObject *)self;
		DeeCodeObject *code = me->fo_code;
		return code->co_refc <= code->co_refstaticc;
	} else if (tp == &DeeYieldFunction_Type) {
		/* Yield functions invocations are immutable if their function does not contain "static" variables */
		DeeYieldFunctionObject *me = (DeeYieldFunctionObject *)self;
		DeeCodeObject *code = me->yf_func->fo_code;
		return code->co_refc <= code->co_refstaticc;
	} else if (tp == &DeeProperty_Type) {
		DeePropertyObject *me = (DeePropertyObject *)self;
		return (!me->p_get || DeeObject_IsDeepImmutable(me->p_get)) &&
		       (!me->p_del || DeeObject_IsDeepImmutable(me->p_del)) &&
		       (!me->p_set || DeeObject_IsDeepImmutable(me->p_set));
	} else if (tp == &DeeSuper_Type) {
		self = DeeSuper_SELF(self);
		goto again;
	} else if (!DeeType_IsVariable(tp)) {
		/* Check if "tp" uses special, known copy-operator (this catches a bunch of sequence proxy types) */
		if (tp->tp_init.tp_alloc.tp_copy_ctor == (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&generic_proxy__copy_alias) {
			ProxyObject *me = (ProxyObject *)self;
			self = me->po_obj;
			goto again;
		} else if (tp->tp_init.tp_alloc.tp_copy_ctor == (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&generic_proxy2__copy_alias12) {
			ProxyObject2 *me = (ProxyObject2 *)self;
			if (!DeeObject_IsDeepImmutable(me->po_obj1))
				return false;
			self = me->po_obj2;
			goto again;
		} else if (tp->tp_init.tp_alloc.tp_copy_ctor == (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&generic_proxy3__copy_alias123) {
			ProxyObject3 *me = (ProxyObject3 *)self;
			if (!DeeObject_IsDeepImmutable(me->po_obj1))
				return false;
			if (!DeeObject_IsDeepImmutable(me->po_obj2))
				return false;
			self = me->po_obj3;
			goto again;
		}
	}
	return false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
type_get_module(DeeTypeObject *__restrict self) {
	DREF DeeModuleObject *result;
	result = DeeType_GetModule(self);
	if likely(result)
		return result;
	DeeRT_ErrTUnboundAttr(&DeeType_Type, self, &str___module__);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_bound_module(DeeTypeObject *__restrict self) {
	DREF DeeModuleObject *result;
	result = DeeType_GetModule(self);
	Dee_XDecref_unlikely(result);
	return Dee_BOUND_FROMBOOL(result != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_instancesize(DeeTypeObject *__restrict self) {
	size_t instance_size;
	if (DeeType_IsVariable(self))
		goto unknown;
	if (!self->tp_init.tp_alloc.tp_ctor &&
	    !self->tp_init.tp_alloc.tp_copy_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor_kw)
		goto unknown;
	instance_size = DeeType_GetInstanceSize(self);
	if unlikely(!instance_size)
		goto unknown;
	return DeeInt_NewSize(instance_size);
unknown:
	DeeRT_ErrTUnboundAttrCStr(&DeeType_Type, Dee_AsObject(self), "__instancesize__");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_bound_instancesize(DeeTypeObject *__restrict self) {
	if (DeeType_IsVariable(self))
		goto unknown;
	if (!self->tp_init.tp_alloc.tp_ctor &&
	    !self->tp_init.tp_alloc.tp_copy_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor_kw)
		goto unknown;
	if (DeeType_GetInstanceSize(self) == 0)
		goto unknown;
	return Dee_BOUND_YES;
unknown:
	return Dee_BOUND_NO;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_operators(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_operatorids(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_ctable(DeeTypeObject *__restrict self);


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_istypetype(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsTypeType(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isvarargconstructible(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsVarArgConstructible(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isconstructible(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsConstructible(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_iscopyable(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsCopyable(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isnamespace(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsNamespace(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isconstcastable(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsConstCastable(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_gcpriority(DeeTypeObject *__restrict self) {
	return DeeInt_NewUInt(DeeType_GCPriority(self));
}

PRIVATE struct type_member tpconst type_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___doc__, STRUCT_CONST | STRUCT_CSTR_OPT, offsetof(DeeTypeObject, tp_doc),
	                      "->?X2?Dstring?N\n"
	                      "Doc string for this type, including documentation on operators, or ?N if it has none"),
	TYPE_MEMBER_FIELD_DOC("__base__", STRUCT_OBJECT, offsetof(DeeTypeObject, tp_base),
	                      "->?.\n"
	                      "#t{UnboundAttribute}"
	                      "The immediate/primary base of this type"),
	TYPE_MEMBER_BITFIELD_DOC("isfinal", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FFINAL,
	                         "True if this type cannot be sub-classed"),
	TYPE_MEMBER_BITFIELD_DOC("isinterrupt", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FINTERRUPT,
	                         "True if instances of this type, when thrown, can only be caught by "
	                         "an ${@[interrupt] catch (...)} catch-all expression, or by a catch "
	                         "expression that explicitly names this type"),
	TYPE_MEMBER_BITFIELD_DOC("isabstract", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FABSTRACT,
	                         "True if this type is #Iabstract, meaning it can be used as an interface"),
	TYPE_MEMBER_BITFIELD_DOC("__isvariable__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FVARIABLE,
	                         "True if instances of this type have a variable size (s.a. ?#__instancesize__)"),
	TYPE_MEMBER_BITFIELD_DOC("__deep_immutable__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FDEEPIMMUTABLE,
	                         "True if instances of this type are deep immutable (meaning "
	                         /**/ "that $deepcopy always re-returns the original object)"),
	TYPE_MEMBER_BITFIELD("__isgc__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FGC),
	TYPE_MEMBER_FIELD_DOC("__isclass__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeTypeObject, tp_class),
	                      "True if this type is a user-defined class (s.a. ?#__class__)"),
	TYPE_MEMBER_BITFIELD("__isinttruncated__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FTRUNCATE),
	TYPE_MEMBER_BITFIELD("__hasmoveany__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FMOVEANY),
	TYPE_MEMBER_BITFIELD_DOC("__iscustom__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FHEAP,
	                         "True if this type was dynamically allocated on the heap"),
	TYPE_MEMBER_BITFIELD("__issuperconstructible__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FINHERITCTOR),
	TYPE_MEMBER_BITFIELD_DOC("__iskw__", STRUCT_CONST, DeeTypeObject, tp_features, TF_KW,
	                         "True if instances of this type can be used as keywords argument in the C API. "
	                         /**/ "When this is not true, and an instance is used as a keywords argument, the "
	                         /**/ "compiler must generate code to wrap instance of this type as a mapping that "
	                         /**/ "#Idoes support ?#__iskw__ (s.a. ?Ert:kw)"),
	TYPE_MEMBER_FIELD("__isnoargconstructible__", STRUCT_CONST | STRUCT_BOOLPTR,
	                  offsetof(DeeTypeObject, tp_init.tp_alloc.tp_ctor)),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst type_getsets[] = {
	TYPE_GETTER_AB_F("name", &type_getname,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Same as ?#__name__, but returns $\"<anonymous type>\" if the type is unnamed"),
	TYPE_GETTER_F(STR___name__, &type_get__name__,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "#t{UnboundAttribute}"
	              "Name of this type"),
	TYPE_GETTER_AB_F("isbuffer", &type_isbuffer,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this Type implements the buffer interface\n"
	                 "The most prominent Type to which this applies is ?DBytes, however other types also support this"),
	TYPE_GETTER_AB_F("__bases__", &TypeBases_New,
	                 METHOD_FCONSTCALL,
	                 "->?Ert:TypeBases\n"
	                 "Returns a sequence of ?. that represents all immediate bases of @this ?."),
	TYPE_GETTER_AB_F("__mro__", &TypeMRO_New,
	                 METHOD_FCONSTCALL,
	                 "->?Ert:TypeMRO\n"
	                 "Returns a sequence of ?. that represents the MethodResolutionOrder of @this ?."),
	TYPE_GETTER_AB_F("__class__", &type_get_classdesc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Ert:ClassDescriptor\n"
	                 "#tUnboundAttribute{@this typeType is a user-defined class (s.a. ?#__isclass__)}"
	                 "Returns the internal class-descriptor descriptor for a user-defined class"),
	TYPE_GETTER_AB_F("__seqclass__", &type_get_seqclass,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?.?N\n"
	                 "Returns the #C{sequence classification} of this type, which is the first of the "
	                 /**/ "following types to appear in ?#__mro__:"
	                 "#L-{"
	                 /**/ "?DSequence|"
	                 /**/ "?DSet|"
	                 /**/ "?DMapping"
	                 "}"),
	TYPE_GETTER_AB_F("__issingleton__", &type_issingleton,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Check if @this Type describes a singleton object, requiring that @this type not be "
	                 /**/ "implementing a constructor (or be deleting its constructor), as well as not be one "
	                 /**/ "of the special internal types used to represent implementation-specific wrapper "
	                 /**/ "objects for C attributes, or be generated by the compiler, such as code objects, "
	                 /**/ "class descriptors or DDI information providers"),
	TYPE_GETTER_BOUND_F(STR___module__, &type_get_module, &type_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Return the module used to define @this Type, or throw :UnboundAttribute if the module "
	                    /**/ "cannot be determined, which may be the case if the type doesn't have any defining "
	                    /**/ "features such as operators, or class/instance member functions"),
	TYPE_GETTER_AB_F("__ctable__", &type_get_ctable,
	                 METHOD_FCONSTCALL,
	                 "->?X2?S?O?AObjectTable?Ert:ClassDescriptor\n"
	                 "Returns an indexable sequence describing the class object table, "
	                 /**/ "as referenced by ?Aaddr?AAttribute?Ert:ClassDescriptor\n"
	                 "For non-user-defined classes (aka. ?#__isclass__ is ?f), an empty sequence is returned\n"
	                 "The instance-attribute table can be accessed through ?A__itable__?DObject"),
	TYPE_GETTER_AB_F("__operators__", &type_get_operators,
	                 METHOD_FCONSTCALL,
	                 "->?S?X2?Dstring?Dint\n"
	                 "Enumerate the names of all the operators overwritten by @this Type as a set-like sequence\n"
	                 "This member functions such that the member function ?#hasprivateoperator can be implemented as:\n"
	                 "${"
	                 /**/ "function hasprivateoperator(name: string | int): bool {\n"
	                 /**/ "	return name in this.__operators__;\n"
	                 /**/ "}"
	                 "}\n"
	                 "Also note that this set doesn't differentiate between overwritten and deleted operators, "
	                 /**/ "as for this purpose any deleted operator is considered to be implemented as throwing a "
	                 /**/ ":NotImplemented exception\n"
	                 "Additionally, this set also includes automatically generated operators for user-classes, "
	                 /**/ "meaning that pretty much any user-class will always have its compare, assignment, as well "
	                 /**/ "as constructor and destructor operators overwritten, even when the user didn't actually "
	                 /**/ "define any of them\n"
	                 "For the purposes of human-readable information, is is recommended to use ?Aoperators?#__class__ "
	                 /**/ "when @this Type is a user-defined class (aka. ?#__isclass__ is ?t), and only use ?#__operators__ "
	                 /**/ "for all other types that this doesn't apply to"),
	TYPE_GETTER_AB_F("__operatorids__", &type_get_operatorids,
	                 METHOD_FCONSTCALL,
	                 "->?S?Dint\n"
	                 "Enumerate the ids of all the operators overwritten by @this Type as a set-like sequence\n"
	                 "This is the same as ?#__operators__, but the runtime will not attempt to translate known "
	                 /**/ "operator ids to their user-friendly name, as described in ?#hasoperator"),
	TYPE_GETTER_BOUND_F("__instancesize__", &type_get_instancesize, &type_bound_instancesize,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the heap allocation size of instances of @this Type, or throw :UnboundAttribute "
	                    /**/ "when @this Type cannot be instantiated, is a singleton (such as ?N), or has variable-"
	                    /**/ "length instances (?#__isvariable__)"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETTER_BOUND_F("__instance_size__", &type_get_instancesize, &type_bound_instancesize,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Deprecated alias for ?#__instancesize__"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER_AB_F("__istypetype__", &type_istypetype, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_AB_F("__isvarargconstructible__", &type_isvarargconstructible, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_AB_F("__isconstructible__", &type_isconstructible, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_AB_F("__iscopyable__", &type_iscopyable, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_AB_F("__isnamespace__", &type_isnamespace, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Instance methods/getsets of this type never look at the $this argument "
	                 /**/ "(allowing for optimizations in ?M_hostasm by passing undefined values for it)"),
	TYPE_GETTER_AB_F("__isconstcastable__", &type_isconstcastable, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Allow constant propagation when instances of this type as used as arguments "
	                 /**/ "to functions marked as #C{METHOD_FCONSTCALL_IF_ARGS_CONSTCAST}."),
	TYPE_GETTER_AB_F("__gcpriority__", &type_gcpriority, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};



INTDEF Dee_hash_t DCALL default__hash__unsupported(DeeObject *__restrict self);

DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL
generic_object_compare_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, Dee_TYPE(self)))
		goto err;
	return self == some_object ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL
generic_object_trycompare_eq(DeeObject *self, DeeObject *some_object) {
	return self == some_object ? 0 : 1;
}

DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_object_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, Dee_TYPE(self)))
		goto err;
	return_bool(self == some_object);
err:
	return NULL;
}

DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_object_ne(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, Dee_TYPE(self)))
		goto err;
	return_bool(self != some_object);
err:
	return NULL;
}

/* Generic operators that implement equals using `===' and hash using `Object.id()'
 * Use this instead of re-inventing the wheel in order to allow for special optimization
 * to be possible when your type appears in compare operations. */
PUBLIC struct Dee_type_cmp DeeObject_GenericCmpByAddr = {
	/* .tp_hash          = */ &default__hash__unsupported,
	/* .tp_compare_eq    = */ &generic_object_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &generic_object_trycompare_eq,
	/* .tp_eq            = */ &generic_object_eq,
	/* .tp_ne            = */ &generic_object_ne,
};




#define type_hash default__hash__unsupported
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) int DCALL
type_compare_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		goto err;
	return self == some_object ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

#define type_trycompare_eq generic_object_trycompare_eq
DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		goto err;
	return_bool(self == some_object);
err:
	return NULL;
}

DEFAULT_OPDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_ne(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		goto err;
	return_bool(self != some_object);
err:
	return NULL;
}

PRIVATE struct type_cmp type_cmp_ = {
	/* .tp_hash          = */ &type_hash,
	/* .tp_compare_eq    = */ &type_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ &type_trycompare_eq,
	/* .tp_eq            = */ &type_eq,
	/* .tp_ne            = */ &type_ne,
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_getattr(DeeObject *self, DeeObject *name) {
	return DeeType_GetAttr((DeeTypeObject *)self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_delattr(DeeObject *self, DeeObject *name) {
	return DeeType_DelAttr((DeeTypeObject *)self, name);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
type_setattr(DeeObject *self, DeeObject *name, DeeObject *value) {
	return DeeType_SetAttr((DeeTypeObject *)self, name, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) Dee_ssize_t DCALL
type_enumattr(DeeTypeObject *UNUSED(tp_self), DeeTypeObject *self,
              struct Dee_attriter *iterbuf, size_t bufsize,
              struct Dee_attrhint const *__restrict hint) {
	return DeeType_IterAttr(self, iterbuf, bufsize, hint);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
type_findattr(DeeTypeObject *UNUSED(tp_self), DeeTypeObject *self,
              struct Dee_attrspec const *__restrict specs,
              struct Dee_attrdesc *__restrict result) {
	return DeeType_FindAttr(self, specs, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_hasattr(DeeTypeObject *self, DeeObject *name) {
	return DeeType_HasAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_boundattr(DeeTypeObject *self, DeeObject *name) {
	return DeeType_BoundAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_callattr(DeeTypeObject *self, DeeObject *name,
              size_t argc, DeeObject *const *argv) {
	return DeeType_CallAttr(self, name, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_callattr_kw(DeeTypeObject *self, DeeObject *name,
                 size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeType_CallAttrKw(self, name, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_vcallattrf(DeeTypeObject *self, DeeObject *name,
                char const *format, va_list args) {
	return DeeType_VCallAttrf(self, name, format, args);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 6)) bool DCALL
type_findattr_info_string_len_hash(DeeTypeObject *tp_self, DeeTypeObject *self,
                                   char const *__restrict attr, size_t attrlen, Dee_hash_t hash,
                                   struct Dee_attrinfo *__restrict retinfo) {
	(void)tp_self;
	return DeeType_FindAttrInfoStringLenHash(self, attr, attrlen, hash, retinfo);
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_callattr_tuple(DeeTypeObject *self, /*String*/ DeeObject *name, DeeObject *args) {
	return DeeType_CallAttrTuple(self, name, args);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_callattr_tuple_kw(DeeTypeObject *self, /*String*/ DeeObject *name,
                       DeeObject *args, DeeObject *kw) {
	return DeeType_CallAttrTupleKw(self, name, args, kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

PRIVATE struct type_attr type_attr_data = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&type_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&type_delattr,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&type_setattr,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&type_enumattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&type_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&type_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&type_boundattr,
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&type_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&type_callattr_kw,
	/* .tp_vcallattrf                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, char const *, va_list))&type_vcallattrf,
	/* .tp_getattr_string_hash           = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_GetAttrStringHash,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_DelAttrStringHash,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&DeeType_SetAttrStringHash,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_HasAttrStringHash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_BoundAttrStringHash,
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&DeeType_CallAttrStringHash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&DeeType_CallAttrStringHashKw,
	/* .tp_vcallattr_string_hashf        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, char const *, va_list))&DeeType_VCallAttrStringHashf,
	/* .tp_getattr_string_len_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_GetAttrStringLenHash,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_DelAttrStringLenHash,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&DeeType_SetAttrStringLenHash,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_HasAttrStringLenHash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_BoundAttrStringLenHash,
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&DeeType_CallAttrStringLenHash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&DeeType_CallAttrStringLenHashKw,
	/* .tp_findattr_info_string_len_hash = */ (bool (DCALL *)(DeeTypeObject *, DeeObject *, char const *__restrict, size_t, Dee_hash_t, struct Dee_attrinfo *__restrict))&type_findattr_info_string_len_hash,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_callattr_tuple                = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&type_callattr_tuple,
	/* .tp_callattr_tuple_kw             = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&type_callattr_tuple_kw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PRIVATE struct type_gc tpconst type_gc_data = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&type_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&type_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_CLASS
};

PRIVATE DeeTypeObject *tpconst type_mro[] = {
	&DeeObject_Type,
	&DeeCallable_Type, /* Types can be called to invoke their constructor, so have them implement deemon.Callable. */
	NULL,
};

PRIVATE struct type_callable type_callable = {
	/* .tp_call_kw = */ (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&DeeObject_NewKw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type), /* The type of Type is Type :D */
	/* .tp_name     = */ DeeString_STR(&str_Type),
	/* .tp_doc      = */ DOC("The so-called Type-Type, that is the type of anything that is "
	                         /**/ "also a Type, such as ?Dint or ?DList, or even ?. itself"),
	/* NOTE: The "TP_FDEEPIMMUTABLE" flag here is actually kind-of wrong: Types can have
	 *       static members that might in turn not be immutable. However, it is expected
	 *       behavior that "deepcopy MyClass()" will just return another instance of
	 *       `MyClass', and not an instance of a new class that is a copy of MyClass! */
	/* .tp_flags    = */ TP_FGC | TP_FNAMEOBJECT | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(DeeTypeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* class Type: Object { ... } */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeTypeObject,
			/* tp_ctor:        */ &type_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &type_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&type_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ &type_str,
		/* .tp_repr      = */ &type_repr,
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ &type_print,
		/* .tp_printrepr = */ &type_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&type_visit,
	/* .tp_gc            = */ &type_gc_data,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &type_cmp_,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ &type_attr_data,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ type_methods,
	/* .tp_getsets       = */ type_getsets,
	/* .tp_members       = */ type_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&DeeObject_New,
	/* .tp_callable      = */ &type_callable,
	/* .tp_mro           = */ type_mro,
	/* .tp_operators     = */ type_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(type_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TYPE_C */
