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
#ifndef GUARD_DEX_RT_LIBRT_C
#define GUARD_DEX_RT_LIBRT_C 1
#define DEE_SOURCE

#include "librt.h"

#include <deemon/compiler/compiler.h>

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/cached-dict.h>
#include <deemon/callable.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dex.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/mapfile.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/property.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/weakref.h>

DECL_BEGIN

/* !!! THIS MODULE IS NON-STANDARD AND MEANT TO EXPOSE IMPLEMENTATION-INTERNALS !!!
 * --------------------------------------------------------------------------------
 * Any compiler/interpreter that wishes to comply with the deemon standard is
 * not required to implement this module, or if it chooses to implement it, it
 * is not required to provide exports identical, or compatible with those seen
 * here.
 * --------------------------------------------------------------------------------
 * If an implementation should choose to provide this module, the purpose of it
 * is to expose implementation-specific internals, such as controls for max
 * stack recursion, objects and types used to implement the compiler, as well
 * as other internal types.
 * --------------------------------------------------------------------------------
 * Since the `rt' module as a whole is non-portable, exported symbols are not
 * required to include the `_np' suffix normally required to indicate non-portable. */




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_getstacklimit_f(size_t argc, DeeObject *const *argv) {
	uint16_t result;
	if (DeeArg_Unpack(argc, argv, ":getstacklimit"))
		goto err;
	result = atomic_read(&DeeExec_StackLimit);
	return DeeInt_NewUInt16(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_setstacklimit_f(size_t argc, DeeObject *const *argv) {
	uint16_t result, newval = DEE_CONFIG_DEFAULT_STACK_LIMIT;
	if (DeeArg_Unpack(argc, argv, "|" UNPu16 ":setstacklimit", &newval))
		goto err;
	result = atomic_xch(&DeeExec_StackLimit, newval);
	return DeeInt_NewUInt16(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_getcalloptimizethreshold_f(size_t argc, DeeObject *const *argv) {
	size_t result;
	if (DeeArg_Unpack(argc, argv, ":getcalloptimizethreshold"))
		goto err;
	result = DeeCode_GetOptimizeCallThreshold();
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_setcalloptimizethreshold_f(size_t argc, DeeObject *const *argv) {
	size_t result, newval;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":setcalloptimizethreshold", &newval))
		goto err;
	result = DeeCode_SetOptimizeCallThreshold(newval);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeTypeObject *DCALL
librt_makeclass_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *base, *descriptor;
	DeeModuleObject *declaring_module = (DeeModuleObject *)Dee_None;
	PRIVATE DEFINE_KWLIST(kwlist, { K(base), K(descriptor), K(module), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "oo|o:makeclass",
	                    &base, &descriptor, &declaring_module))
		goto err;
	if (DeeObject_AssertTypeExact(descriptor, &DeeClassDescriptor_Type))
		goto err;
	if (DeeNone_Check(declaring_module)) {
		/* Special case: no declaring module given. */
		declaring_module = NULL;
	} else {
		if (DeeObject_AssertTypeExact(declaring_module, &DeeModule_Type))
			goto err;
	}
	return DeeClass_New(base, descriptor, declaring_module);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(librt_getstacklimit, &librt_getstacklimit_f, METHOD_FPURECALL);
PRIVATE DEFINE_CMETHOD(librt_setstacklimit, &librt_setstacklimit_f, METHOD_FNORMAL);

PRIVATE DEFINE_CMETHOD(librt_getcalloptimizethreshold, &librt_getcalloptimizethreshold_f, METHOD_FPURECALL);
PRIVATE DEFINE_CMETHOD(librt_setcalloptimizethreshold, &librt_setcalloptimizethreshold_f, METHOD_FNORMAL);
PRIVATE DEFINE_KWCMETHOD(librt_makeclass, &librt_makeclass_f, METHOD_FNORMAL);

#if 1
#define str_Iterator (*COMPILER_CONTAINER_OF(DeeIterator_Type.tp_name, DeeStringObject, s_str))
#else
/*[[[deemon (PRIVATE_DEFINE_STRING from rt.gen.string)("str_Iterator", "Iterator");]]]*/
PRIVATE DEFINE_STRING_EX(str_Iterator, "Iterator", 0xfce46883, 0x3c33c9d5c64ebfff);
/*[[[end]]]*/
#endif

LOCAL WUNUSED DREF DeeObject *DCALL
get_type_of(DREF DeeObject *ob) {
	DREF DeeObject *result = NULL;
	if likely(ob) {
		result = (DeeObject *)Dee_TYPE(ob);
		Dee_Incref(result);
		Dee_Decref_unlikely(ob);
	}
	return result;
}

LOCAL WUNUSED DREF DeeObject *DCALL
get_iterator_of(DREF DeeObject *ob) {
	DREF DeeObject *result = NULL;
	if likely(ob) {
		result = DeeObject_GetAttr((DeeObject *)ob,
		                           (DeeObject *)&str_Iterator);
		Dee_Decref_unlikely(ob);
	}
	return result;
}


LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ClassOperatorTable_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type, "OperatorTable");
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttribute_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type, "Attribute");
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttributeTable_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type, "AttributeTable");
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ObjectTable_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type, "ObjectTable");
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_TypeMRO_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)&DeeInt_Type, "__mro__"));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_TypeBases_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)&DeeInt_Type, "__bases__"));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_TypeMROIterator_impl_f(void) {
	return get_iterator_of(librt_get_TypeMRO_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_TypeBasesIterator_impl_f(void) {
	return get_iterator_of(librt_get_TypeBases_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassOperatorTable_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ClassOperatorTable_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassOperatorTableIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ClassOperatorTable_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttribute_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ClassAttribute_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttributeTable_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ClassAttributeTable_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttributeTableIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ClassAttributeTable_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ObjectTable_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ObjectTable_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeMRO_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_TypeMRO_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeMROIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_TypeMROIterator_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeBases_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_TypeBases_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeBasesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_TypeBasesIterator_impl_f();
}



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RoDictIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeRoDict_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RoSetIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeRoSet_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_KwdsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeKwds_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_KwdsMappingIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeKwdsMapping_Type,
	                         (DeeObject *)&str_Iterator);
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GenericIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	/* The internal `_ObjectTable' type doesn't implement its own iterator type,
	 * but instead uses the generic iterator. - By requesting access to the iterator
	 * that's being used, we can thereby gain backdoor access to the internal, generic
	 * iterator implementation type. */
	return get_iterator_of(librt_get_ObjectTable_impl_f());
}


LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
get_generic_iterator_member_type(char const *__restrict name) {
	/* return type(iterator().operator . (name)); */
	DREF DeeObject *result, *stub_iterator, *member;
	stub_iterator = DeeObject_NewDefault(&DeeIterator_Type);
	if unlikely(!stub_iterator)
		goto err;
	member = DeeObject_GetAttrString(stub_iterator, name);
	Dee_Decref_likely(stub_iterator);
	if unlikely(!member)
		goto err;
	result = (DREF DeeObject *)Dee_TYPE(member);
	Dee_Incref(result);
	Dee_Decref_likely(member);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IteratorPending_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_generic_iterator_member_type("pending");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IteratorFuture_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_generic_iterator_member_type("future");
}




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeString_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeBytes_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ListIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeList_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TupleIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeTuple_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_HashSetIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeHashSet_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DictIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeDict_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DictProxyIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeDictProxy_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DictKeysIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeDictKeys_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DictItemsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeDictItems_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DictValuesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeDictValues_Type,
	                         (DeeObject *)&str_Iterator);
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_TracebackIterator_impl_f(void) {
	return DeeObject_GetAttr((DeeObject *)&DeeTraceback_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TracebackIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_TracebackIterator_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCEnum_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return_reference((DREF DeeObject *)Dee_TYPE(&DeeGCEnumTracked_Singleton));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Traceback_empty_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	DREF DeeObject *iter, *result = NULL;
	iter = librt_get_TracebackIterator_impl_f();
	if likely(iter) {
		result = DeeObject_GetAttrString(iter, "seq");
		Dee_Decref_likely(iter);
	}
	return result;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_GCSet_empty_impl_f(void) {
	DeeObject *argv[] = { Dee_None };
	return DeeObject_CallAttrString(&DeeGCEnumTracked_Singleton, "reachable", 1, argv);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSet_empty_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_GCSet_empty_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSet_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(librt_get_GCSet_empty_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSetIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(get_type_of(librt_get_GCSet_empty_impl_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Code_empty_impl_f(void) {
	/* The empty-code object is set when `Code()' is called without any arguments. */
	return DeeObject_NewDefault(&DeeCode_Type);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Code_empty_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_Code_empty_impl_f();
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_Module_empty_f_impl(void) {
	DREF DeeObject *empty_code;
	DREF DeeModuleObject *result;
	empty_code = librt_get_Code_empty_impl_f();
	if unlikely(!empty_code)
		goto err;
	if (DeeObject_AssertTypeExact(empty_code, &DeeCode_Type))
		goto err_empty_code;
	result = ((DeeCodeObject *)empty_code)->co_module;
	Dee_Incref(result);
	Dee_Decref_unlikely(empty_code);
	return (DREF DeeObject *)result;
err_empty_code:
	Dee_Decref(empty_code);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Module_empty_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_Module_empty_f_impl();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BlackListKwdsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeBlackListKwds_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BlackListKwIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeBlackListKw_Type,
	                         (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_CachedDictIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeCachedDict_Type,
	                         (DeeObject *)&str_Iterator);
}




LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_DocKwds_impl_f(void) {
	/* To implement this, we need to get access to an instance of it,
	 * which we are doing via `type(("import" from deemon).__kwds__)'.
	 * Because the `import()' function is known to implement keyword
	 * support, we can use it as a reference point for a C-level function
	 * with a non-empty keyword list, without having to create such an
	 * object ourself. */
	DREF DeeObject *result, *kwds;
	kwds = DeeObject_GetAttrString((DeeObject *)&DeeBuiltin_Import, "__kwds__");
	if unlikely(!kwds)
		goto err;
	result = (DREF DeeObject *)Dee_TYPE(kwds);
	Dee_Incref(result);
	Dee_Decref_likely(kwds);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DocKwds_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_DocKwds_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DocKwdsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_DocKwds_impl_f());
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingProxy_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type, "Proxy");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingKeys_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type, "Proxy");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingValues_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type, "Values");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingItems_impl_f(void) {
	return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type, "Items");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingHashFilter_impl_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyMapping, "byhash", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingByAttr_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString(Dee_EmptyMapping, "byattr"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingProxy_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_MappingProxy_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingKeys_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_MappingKeys_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingValues_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_MappingValues_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingItems_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_MappingItems_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingProxyIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_MappingProxy_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingKeysIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_MappingKeys_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingValuesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_MappingValues_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingItemsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_MappingItems_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingHashFilter_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_MappingHashFilter_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingHashFilterIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_MappingHashFilter_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MappingByAttr_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_MappingByAttr_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeOperators_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)&DeeString_Type, "__operators__"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeOperators_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_TypeOperators_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeOperatorsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_TypeOperators_impl_f());
}


LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
librt_get_sequence_mutation_type(char const *__restrict name) {
	DREF DeeObject *ob, *result = NULL;
	ob = DeeTuple_Pack(2, Dee_None, Dee_None);
	if likely(ob) {
		DeeObject *argv[] = { DeeInt_One };
		result            = get_type_of(DeeObject_CallAttrString(ob, name, 1, argv));
		Dee_Decref(ob);
	}
	return result;
}

LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
librt_get_string_mutation_type(char const *__restrict name) {
	DeeObject *argv[] = { DeeInt_One };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, name, 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqCombinations_impl_f(void) {
	return librt_get_string_mutation_type("combinations");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqCombinations_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqCombinations_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqCombinationsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqCombinations_impl_f());
}


LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatCombinations_impl_f(void) {
	return librt_get_string_mutation_type("repeatcombinations");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatCombinations_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqRepeatCombinations_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatCombinationsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqRepeatCombinations_impl_f());
}


LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqPermutations_impl_f(void) {
	return librt_get_string_mutation_type("permutations");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqPermutations_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqPermutations_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqPermutationsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqPermutations_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqSegments_impl_f(void) {
	/* Since string overrides `segments', we must use a true sequence here! */
	return librt_get_sequence_mutation_type("segments");
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSegments_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqSegments_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSegmentsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqSegments_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqConcat_impl_f(void) {
	return get_type_of(DeeObject_Add(Dee_EmptySeq, Dee_EmptySeq));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqConcat_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqConcat_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqConcatIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqConcat_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilter_impl_f(void) {
	DeeObject *argv[] = { Dee_None };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq, "filter", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqHashFilter_impl_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq, "byhash", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilter_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqFilter_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilterIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqFilter_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqHashFilter_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqHashFilter_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqHashFilterIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqHashFilter_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqLocator_impl_f(void) {
	DeeObject *argv[] = { Dee_None };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq, "locateall", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqLocator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqLocator_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqLocatorIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqLocator_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqSubRange_impl_f(void) {
	return get_type_of(DeeObject_GetRange(Dee_EmptySeq, DeeInt_Zero, DeeInt_One));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSubRange_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqSubRange_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSubRangeIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqSubRange_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSubRangeN_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(DeeObject_GetRange(Dee_EmptySeq, DeeInt_One, Dee_None));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqTransformation_impl_f(void) {
	DeeObject *argv[] = { Dee_None };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq, "transform", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqTransformation_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqTransformation_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqTransformationIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqTransformation_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqRange_impl_f(void) {
	return get_type_of(DeeRange_New(Dee_None, Dee_None, NULL));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRange_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqRange_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRangeIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqRange_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqIntRange_impl_f(void) {
	return get_type_of(DeeRange_NewInt(0, 20, 1));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIntRange_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqIntRange_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIntRangeIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqIntRange_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeat_impl_f(void) {
	DeeObject *argv[] = { Dee_EmptySeq, DeeInt_One };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "repeatseq", 2, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeat_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqRepeat_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqRepeat_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqItemRepeat_impl_f(void) {
	DeeObject *argv[] = { Dee_None, DeeInt_One };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type, "repeat", 2, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqItemRepeat_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqItemRepeat_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqItemRepeatIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqItemRepeat_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqIds_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)Dee_EmptySeq, "ids"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIds_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqIds_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIdsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqIds_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqTypes_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)Dee_EmptySeq, "types"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqTypes_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqTypes_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqTypesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqTypes_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqClasses_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)Dee_EmptySeq, "classes"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqClasses_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SeqClasses_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqClassesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SeqClasses_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqEach_stub_instance(void) {
	return DeeObject_GetAttrString(Dee_EmptySeq, "each");
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachOperator_stub_instance(void) {
	DREF DeeObject *result;
	result = librt_get_SeqEach_stub_instance();
	if likely(result) {
		DREF DeeObject *temp;
		temp = DeeObject_Pos(result);
		Dee_Decref(result);
		result = temp;
	}
	return result;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachGetAttr_stub_instance(void) {
	DREF DeeObject *result;
	result = librt_get_SeqEach_stub_instance();
	if likely(result) {
		DREF DeeObject *temp;
		temp = DeeObject_GetAttr(result, (DeeObject *)&str_Iterator);
		Dee_Decref(result);
		result = temp;
	}
	return result;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttr_stub_instance(void) {
	DREF DeeObject *result;
	result = librt_get_SeqEach_stub_instance();
	if likely(result) {
		DREF DeeObject *temp;
		temp = DeeObject_CallAttr(result, (DeeObject *)&str_Iterator, 0, NULL);
		Dee_Decref(result);
		result = temp;
	}
	return result;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrKw_stub_instance(void) {
	DREF DeeObject *result;
	result = librt_get_SeqEach_stub_instance();
	if likely(result) {
		DREF DeeObject *temp;
		temp = DeeObject_CallAttrKw(result, (DeeObject *)&str_Iterator,
		                            0,
		                            NULL,
		                            Dee_EmptyMapping);
		Dee_Decref(result);
		result = temp;
	}
	return result;
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEach_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(librt_get_SeqEach_stub_instance());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachOperator_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(librt_get_SeqEachOperator_stub_instance());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachOperatorIterator_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(get_type_of(librt_get_SeqEachOperator_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachGetAttr_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(librt_get_SeqEachGetAttr_stub_instance());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachGetAttrIterator_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(get_type_of(librt_get_SeqEachGetAttr_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttr_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(librt_get_SeqEachCallAttr_stub_instance());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrIterator_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(get_type_of(librt_get_SeqEachCallAttr_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrKw_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_type_of(librt_get_SeqEachCallAttrKw_stub_instance());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrKwIterator_Type_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(get_type_of(librt_get_SeqEachCallAttrKw_stub_instance()));
}



LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_nonempty_stub_set(void) {
	DREF DeeRoSetObject *result;
	result = DeeRoSet_NewWithHint(1);
	if (result && DeeRoSet_Insert(&result, Dee_None))
		Dee_Clear(result);
	return (DREF DeeObject *)result;
}


LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SetUnion_impl_f(void) {
	DREF DeeObject *a, *b, *c;
	a = librt_get_nonempty_stub_set();
	if unlikely(!a)
		goto err;
	b = librt_get_nonempty_stub_set();
	if unlikely(!b)
		goto err_a;
	c = DeeObject_Or(a, b);
	Dee_Decref(b);
	Dee_Decref(a);
	return get_type_of(c);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SetSymmetricDifference_impl_f(void) {
	DREF DeeObject *a, *b, *c;
	a = librt_get_nonempty_stub_set();
	if unlikely(!a)
		goto err;
	b = librt_get_nonempty_stub_set();
	if unlikely(!b)
		goto err_a;
	c = DeeObject_Xor(a, b);
	Dee_Decref(b);
	Dee_Decref(a);
	return get_type_of(c);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SetIntersection_impl_f(void) {
	DREF DeeObject *a, *b, *c;
	a = librt_get_nonempty_stub_set();
	if unlikely(!a)
		goto err;
	b = librt_get_nonempty_stub_set();
	if unlikely(!b)
		goto err_a;
	c = DeeObject_And(a, b);
	Dee_Decref(b);
	Dee_Decref(a);
	return get_type_of(c);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_SetDifference_impl_f(void) {
	DREF DeeObject *a, *b, *c;
	a = librt_get_nonempty_stub_set();
	if unlikely(!a)
		goto err;
	b = librt_get_nonempty_stub_set();
	if unlikely(!b)
		goto err_a;
	c = DeeObject_Sub(a, b);
	Dee_Decref(b);
	Dee_Decref(a);
	return get_type_of(c);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetUnion_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SetUnion_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetUnionIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SetUnion_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetSymmetricDifference_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SetSymmetricDifference_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetSymmetricDifferenceIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SetSymmetricDifference_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetIntersection_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SetIntersection_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetIntersectionIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SetIntersection_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetDifference_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_SetDifference_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetDifferenceIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_SetDifference_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SharedVectorIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeSharedVector_Type, (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SharedMapIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return DeeObject_GetAttr((DeeObject *)&DeeSharedMap_Type, (DeeObject *)&str_Iterator);
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RefVector_impl_f(void) {
	return get_type_of(DeeRefVector_NewReadonly(Dee_None, 0, NULL));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RefVector_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_RefVector_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RefVectorIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_RefVector_impl_f());
}




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleExports_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(), "__exports__"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleExports_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ModuleExports_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleExportsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ModuleExports_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleGlobals_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(), "__globals__"));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleGlobals_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ModuleGlobals_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleGlobalsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ModuleGlobals_impl_f());
}



PRIVATE DEFINE_BYTES(small_bytes, Dee_BUFFER_FREADONLY, 1, { 0 });
LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_BytesFind_impl_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes, "findall", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesFind_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_BytesFind_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesFindIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_BytesFind_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseFind_impl_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes, "casefindall", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseFind_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_BytesCaseFind_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseFindIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_BytesCaseFind_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_BytesSegments_impl_f(void) {
	DeeObject *argv[] = { DeeInt_One };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes, "segments", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSegments_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_BytesSegments_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSegmentsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_BytesSegments_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_BytesSplit_impl_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes, "split", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_BytesSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_BytesSplit_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseSplit_impl_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes, "casesplit", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_BytesCaseSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_BytesCaseSplit_impl_f());
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_BytesLineSplit_impl_f(void) {
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes, "splitlines", 0, NULL));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesLineSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_BytesLineSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesLineSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_BytesLineSplit_impl_f());
}






LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringSegments_impl_f(void) {
	return librt_get_string_mutation_type("segments");
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringFind_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, "findall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseFind_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, "casefindall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringSplit_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, "split", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseSplit_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, "casesplit", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringLineSplit_impl_f(void) {
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, "splitlines", 0, NULL));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringScan_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrString((DeeObject *)&str_Iterator, "scanf", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_StringOrdinals_impl_f(void) {
	return get_type_of(DeeObject_GetAttrString((DeeObject *)&str_Iterator, "ordinals"));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSegments_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringSegments_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSegmentsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringSegments_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringFind_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringFind_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringFindIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringFind_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseFind_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringCaseFind_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseFindIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringCaseFind_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringSplit_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringCaseSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringCaseSplit_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringLineSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringLineSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringLineSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringLineSplit_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringScan_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringScan_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringScanIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringScan_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringOrdinals_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_StringOrdinals_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringOrdinalsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_StringOrdinals_impl_f());
}



LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReFindAll_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyString, "refindall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_RegFindAll_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyString, "regfindall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReLocateAll_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyString, "relocateall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReSplit_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyString, "resplit", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReGroups_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyString, "regmatch", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReSubStrings_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyString, "rescanf", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesFindAll_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyBytes, "refindall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesFindAll_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyBytes, "regfindall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesLocateAll_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyBytes, "relocateall", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesSplit_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyBytes, "resplit", 1, argv));
}

LOCAL WUNUSED DREF DeeObject *DCALL
librt_get_ReSubBytes_impl_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrString(Dee_EmptyBytes, "rescanf", 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReFindAll_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReFindAll_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReFindAllIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReFindAll_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegFindAll_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_RegFindAll_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegFindAllIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_RegFindAll_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReLocateAll_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReLocateAll_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReLocateAllIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReLocateAll_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReSplit_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReGroups_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReGroups_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReGroupsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReGroups_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubStrings_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReSubStrings_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubStringsIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReSubStrings_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubBytes_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReSubBytes_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubBytesIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReSubBytes_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesFindAll_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReBytesFindAll_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesFindAllIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReBytesFindAll_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesFindAll_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_RegBytesFindAll_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesFindAllIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_RegBytesFindAll_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesLocateAll_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReBytesLocateAll_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesLocateAllIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReBytesLocateAll_impl_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesSplit_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return librt_get_ReBytesSplit_impl_f();
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesSplitIterator_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {
	return get_iterator_of(librt_get_ReBytesSplit_impl_f());
}




PRIVATE DEFINE_CMETHOD(librt_get_SeqCombinations, &librt_get_SeqCombinations_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqCombinationsIterator, &librt_get_SeqCombinationsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeatCombinations, &librt_get_SeqRepeatCombinations_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeatCombinationsIterator, &librt_get_SeqRepeatCombinationsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqPermutations, &librt_get_SeqPermutations_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqPermutationsIterator, &librt_get_SeqPermutationsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSegments, &librt_get_SeqSegments_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSegmentsIterator, &librt_get_SeqSegmentsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqConcat, &librt_get_SeqConcat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqConcatIterator, &librt_get_SeqConcatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqFilter, &librt_get_SeqFilter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqFilterIterator, &librt_get_SeqFilterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqHashFilter, &librt_get_SeqHashFilter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqHashFilterIterator, &librt_get_SeqHashFilterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqLocator, &librt_get_SeqLocator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqLocatorIterator, &librt_get_SeqLocatorIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSubRange, &librt_get_SeqSubRange_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSubRangeIterator, &librt_get_SeqSubRangeIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSubRangeN, &librt_get_SeqSubRangeN_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqTransformation, &librt_get_SeqTransformation_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqTransformationIterator, &librt_get_SeqTransformationIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRange, &librt_get_SeqRange_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRangeIterator, &librt_get_SeqRangeIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqIntRange, &librt_get_SeqIntRange_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqIntRangeIterator, &librt_get_SeqIntRangeIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeat, &librt_get_SeqRepeat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeatIterator, &librt_get_SeqRepeatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqItemRepeat, &librt_get_SeqItemRepeat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqItemRepeatIterator, &librt_get_SeqItemRepeatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqIds, &librt_get_SeqIds_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqIdsIterator, &librt_get_SeqIdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqTypes, &librt_get_SeqTypes_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqTypesIterator, &librt_get_SeqTypesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqClasses, &librt_get_SeqClasses_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqClassesIterator, &librt_get_SeqClassesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEach, &librt_get_SeqEach_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachOperator, &librt_get_SeqEachOperator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachOperatorIterator, &librt_get_SeqEachOperatorIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachGetAttr, &librt_get_SeqEachGetAttr_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachGetAttrIterator, &librt_get_SeqEachGetAttrIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachCallAttr, &librt_get_SeqEachCallAttr_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachCallAttrIterator, &librt_get_SeqEachCallAttrIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachCallAttrKw, &librt_get_SeqEachCallAttrKw_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SeqEachCallAttrKwIterator, &librt_get_SeqEachCallAttrKwIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetUnion, &librt_get_SetUnion_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetUnionIterator, &librt_get_SetUnionIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetSymmetricDifference, &librt_get_SetSymmetricDifference_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetSymmetricDifferenceIterator, &librt_get_SetSymmetricDifferenceIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetIntersection, &librt_get_SetIntersection_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetIntersectionIterator, &librt_get_SetIntersectionIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetDifference, &librt_get_SetDifference_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SetDifferenceIterator, &librt_get_SetDifferenceIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ClassOperatorTable, &librt_get_ClassOperatorTable_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ClassOperatorTableIterator, &librt_get_ClassOperatorTableIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ClassAttribute, &librt_get_ClassAttribute_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ClassAttributeTable, &librt_get_ClassAttributeTable_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ClassAttributeTableIterator, &librt_get_ClassAttributeTableIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ObjectTable, &librt_get_ObjectTable_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TypeMRO, &librt_get_TypeMRO_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TypeMROIterator, &librt_get_TypeMROIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TypeBases, &librt_get_TypeBases_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TypeBasesIterator, &librt_get_TypeBasesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RoDictIterator, &librt_get_RoDictIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RoSetIterator, &librt_get_RoSetIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_KwdsIterator, &librt_get_KwdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_KwdsMappingIterator, &librt_get_KwdsMappingIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_GenericIterator, &librt_get_GenericIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_IteratorPending, &librt_get_IteratorPending_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_IteratorFuture, &librt_get_IteratorFuture_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringIterator, &librt_get_StringIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesIterator, &librt_get_BytesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ListIterator, &librt_get_ListIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TupleIterator, &librt_get_TupleIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_HashSetIterator, &librt_get_HashSetIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DictIterator, &librt_get_DictIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DictProxyIterator, &librt_get_DictProxyIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DictKeysIterator, &librt_get_DictKeysIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DictItemsIterator, &librt_get_DictItemsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DictValuesIterator, &librt_get_DictValuesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TracebackIterator, &librt_get_TracebackIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_GCSet, &librt_get_GCSet_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_GCSetIterator, &librt_get_GCSetIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_GCSet_empty, &librt_get_GCSet_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_Code_empty, &librt_get_Code_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BlackListKwdsIterator, &librt_get_BlackListKwdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BlackListKwIterator, &librt_get_BlackListKwIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_CachedDictIterator, &librt_get_CachedDictIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_GCEnum, &librt_get_GCEnum_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_Traceback_empty, &librt_get_Traceback_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_Module_empty, &librt_get_Module_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DocKwds, &librt_get_DocKwds_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_DocKwdsIterator, &librt_get_DocKwdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingProxy, &librt_get_MappingProxy_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingProxyIterator, &librt_get_MappingProxyIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingKeys, &librt_get_MappingKeys_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingKeysIterator, &librt_get_MappingKeysIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingValues, &librt_get_MappingValues_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingValuesIterator, &librt_get_MappingValuesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingItems, &librt_get_MappingItems_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingItemsIterator, &librt_get_MappingItemsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingHashFilter, &librt_get_MappingHashFilter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingHashFilterIterator, &librt_get_MappingHashFilterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_MappingByAttr, &librt_get_MappingByAttr_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SharedVectorIterator, &librt_get_SharedVectorIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_SharedMapIterator, &librt_get_SharedMapIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RefVector, &librt_get_RefVector_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RefVectorIterator, &librt_get_RefVectorIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TypeOperators, &librt_get_TypeOperators_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_TypeOperatorsIterator, &librt_get_TypeOperatorsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleExports, &librt_get_ModuleExports_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleExportsIterator, &librt_get_ModuleExportsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleGlobals, &librt_get_ModuleGlobals_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleGlobalsIterator, &librt_get_ModuleGlobalsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesFind, &librt_get_BytesFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesFindIterator, &librt_get_BytesFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseFind, &librt_get_BytesCaseFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseFindIterator, &librt_get_BytesCaseFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSegments, &librt_get_BytesSegments_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSegmentsIterator, &librt_get_BytesSegmentsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSplit, &librt_get_BytesSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSplitIterator, &librt_get_BytesSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseSplit, &librt_get_BytesCaseSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseSplitIterator, &librt_get_BytesCaseSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesLineSplit, &librt_get_BytesLineSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_BytesLineSplitIterator, &librt_get_BytesLineSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringFind, &librt_get_StringFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringFindIterator, &librt_get_StringFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseFind, &librt_get_StringCaseFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseFindIterator, &librt_get_StringCaseFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringSegments, &librt_get_StringSegments_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringSegmentsIterator, &librt_get_StringSegmentsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringSplit, &librt_get_StringSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringSplitIterator, &librt_get_StringSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseSplit, &librt_get_StringCaseSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseSplitIterator, &librt_get_StringCaseSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringLineSplit, &librt_get_StringLineSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringLineSplitIterator, &librt_get_StringLineSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringScan, &librt_get_StringScan_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringScanIterator, &librt_get_StringScanIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringOrdinals, &librt_get_StringOrdinals_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_StringOrdinalsIterator, &librt_get_StringOrdinalsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReFindAll, &librt_get_ReFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReFindAllIterator, &librt_get_ReFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RegFindAll, &librt_get_RegFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RegFindAllIterator, &librt_get_RegFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReLocateAll, &librt_get_ReLocateAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReLocateAllIterator, &librt_get_ReLocateAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReSplit, &librt_get_ReSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReSplitIterator, &librt_get_ReSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReGroups, &librt_get_ReGroups_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReGroupsIterator, &librt_get_ReGroupsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReSubStrings, &librt_get_ReSubStrings_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReSubStringsIterator, &librt_get_ReSubStringsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReSubBytes, &librt_get_ReSubBytes_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReSubBytesIterator, &librt_get_ReSubBytesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReBytesFindAll, &librt_get_ReBytesFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReBytesFindAllIterator, &librt_get_ReBytesFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RegBytesFindAll, &librt_get_RegBytesFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_RegBytesFindAllIterator, &librt_get_RegBytesFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReBytesLocateAll, &librt_get_ReBytesLocateAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReBytesLocateAllIterator, &librt_get_ReBytesLocateAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReBytesSplit, &librt_get_ReBytesSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(librt_get_ReBytesSplitIterator, &librt_get_ReBytesSplitIterator_f, METHOD_FCONSTCALL);



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_argv_get_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":argv.getter"))
		goto err;
	return Dee_GetArgv();
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_argv_set_f(size_t argc, DeeObject *const *argv) {
	DeeObject *new_tuple;
	size_t i;
	if (DeeArg_Unpack(argc, argv, "o:argv.setter", &new_tuple))
		goto err;
	if (DeeObject_AssertTypeExact(new_tuple, &DeeTuple_Type))
		goto err;
	for (i = 0; i < DeeTuple_SIZE(new_tuple); ++i) {
		if (DeeObject_AssertTypeExact(DeeTuple_GET(new_tuple, i), &DeeString_Type))
			goto err;
	}
	Dee_SetArgv(new_tuple);
	return_none;
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(librt_argv_get, &librt_argv_get_f, METHOD_FPURECALL);
PRIVATE DEFINE_CMETHOD(librt_argv_set, &librt_argv_set_f, METHOD_FNORMAL);



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_kw_f(size_t argc, DeeObject *const *argv) {
	DeeObject *kw;
	if (DeeArg_Unpack(argc, argv, "o:kw", &kw))
		goto err;
	return DeeKw_Wrap(kw);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(librt_kw, &librt_kw_f, METHOD_FCONSTCALL);




/* NOTE: At first glance, the combination of `MODSYM_FPROPERTY|MODSYM_FCONSTEXPR' may
 *       not look like it would make sense, but by using this combination, we prevent
 *       the symbols to be considered properties during enumeration (`ATTR_PROPERTY'
 *       doesn't get set), thus allowing the doc server to browse them unrestricted. */
PRIVATE struct dex_symbol symbols[] = {
	{ "getstacklimit", (DeeObject *)&librt_getstacklimit, MODSYM_FREADONLY, /* varying */
	  DOC("->?Dint\n"
	      "Returns the current stack limit, that is the max number of "
	      /**/ "user-code functions that may be executed consecutively before "
	      /**/ "a :StackOverflow error is thrown\n"
	      "The default stack limit is $" PP_STR(DEE_CONFIG_DEFAULT_STACK_LIMIT)) },
	{ "setstacklimit", (DeeObject *)&librt_setstacklimit, MODSYM_FREADONLY, /* varying */
	  DOC("(new_limit=!" PP_STR(DEE_CONFIG_DEFAULT_STACK_LIMIT) ")->?Dint\n"
	      "#tIntegerOverflow{@new_limit is negative, or greater than $0xffff}"
	      "Set the new stack limit to @new_limit and return the old limit\n"
	      "The stack limit is checked every time a user-code function is "
	      /**/ "entered, at which point a :StackOverflow error is thrown if "
	      /**/ "the currently set limit is exceeded") },
	{ "stacklimitunlimited",
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
	  Dee_True
#else /* CONFIG_HAVE_EXEC_ALTSTACK */
	  Dee_False
#endif /* !CONFIG_HAVE_EXEC_ALTSTACK */
	  ,
	  MODSYM_FREADONLY | MODSYM_FCONSTEXPR, /* varying */
	  DOC("->?Dbool\n"
	      "A boolean that is ?t if the deemon interpreter supports "
	      /**/ "an unlimited stack limit, meaning that #setstacklimit can "
	      /**/ "be used to set a stack limit of to up $0xffff ($65535), which "
	      /**/ "is the hard limit.\n"
	      "When ?f, setting the stack limit higher than the default "
	      /**/ "may lead to hard crashes of the deemon interpreter, causing "
	      /**/ "the user-script and hosting application to crash in an undefined "
	      /**/ "manner.\n"
	      "Unlimited stack limit support requires a special arch-specific "
	      /**/ "sub-routine within the deemon core, which may not be implemented "
	      /**/ "for an arbitrary architecture.") },

	{ "getcalloptimizethreshold", (DeeObject *)&librt_getcalloptimizethreshold, MODSYM_FREADONLY,
	  DOC("->?Dint\n"
	      "Get the threshold specifying how often a ?DFunction or ?DCode object "
	      /**/ "needs to be called before deemon will automatically try to optimize it.") },
	{ "setcalloptimizethreshold", (DeeObject *)&librt_setcalloptimizethreshold, MODSYM_FREADONLY,
	  DOC("(newThreshold:?Dint)->?Dint\n"
	      "#r{The old threshold}"
	      "Set the threshold specifying how often a ?DFunction or ?DCode object "
	      /**/ "needs to be called before deemon will automatically try to optimize it.") },

	{ "SlabStat", (DeeObject *)&SlabStat_Type, MODSYM_FREADONLY }, /* Access to slab allocator statistics. */
	{ "makeclass", (DeeObject *)&librt_makeclass, MODSYM_FREADONLY,
	  DOC("(base:?X3?N?DType?S?DType,descriptor:?GClassDescriptor,module:?X2?DModule?N=!N)->?DType\n"
	      "#pmodule{The module that is declaring the class (and returned by ${return.__module__}). "
	      /*     */ "When not given (or given as ?N), the type is not linked to a module.}"
	      "Construct a new class from a given @base type, as well as class @descriptor") },

	/* Access of the arguments passed to the __MAIN__ module. */
	{ "argv", (DeeObject *)&librt_argv_get, MODSYM_FPROPERTY,
	  DOC("->?DTuple\n"
	      "The arguments that are passed to the $__MAIN__ user-code "
	      /**/ "module, where they are available as ${[...]}.\n"
	      "Since these arguments aren't accessible from any other module if not explicitly "
	      /**/ "passed by the $__MAIN__ module itself (similar to how you'd have to forward "
	      /**/ "#Cargc + #Cargv in C), this rt-specific extension property allows you to get "
	      /**/ "and set that tuple of arguments.\n"
	      "Note however that setting a new argument tuple will not change the tuple "
	      /**/ "which the $__MAIN__ module has access to.") },
	{ NULL, NULL, MODSYM_FNORMAL },
	{ NULL, (DeeObject *)&librt_argv_set, MODSYM_FNORMAL },


	/* Internal types used to drive sequence proxies */
	{ "SeqCombinations", (DeeObject *)&librt_get_SeqCombinations, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqCombinations_Type */
	{ "SeqCombinationsIterator", (DeeObject *)&librt_get_SeqCombinationsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* SeqCombinationsIterator_Type */
	{ "SeqRepeatCombinations", (DeeObject *)&librt_get_SeqRepeatCombinations, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* SeqRepeatCombinations_Type */
	{ "SeqRepeatCombinationsIterator", (DeeObject *)&librt_get_SeqRepeatCombinationsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* SeqRepeatCombinationsIterator_Type */
	{ "SeqPermutations", (DeeObject *)&librt_get_SeqPermutations, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqPermutations_Type */
	{ "SeqPermutationsIterator", (DeeObject *)&librt_get_SeqPermutationsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* SeqPermutationsIterator_Type */
	{ "SeqSegments", (DeeObject *)&librt_get_SeqSegments, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* SeqSegments_Type */
	{ "SeqSegmentsIterator", (DeeObject *)&librt_get_SeqSegmentsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* SeqSegmentsIterator_Type */
	{ "SeqConcat", (DeeObject *)&librt_get_SeqConcat, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* SeqConcat_Type */
	{ "SeqConcatIterator", (DeeObject *)&librt_get_SeqConcatIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqConcatIterator_Type */
	{ "SeqFilter", (DeeObject *)&librt_get_SeqFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* SeqFilter_Type */
	{ "SeqFilterIterator", (DeeObject *)&librt_get_SeqFilterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqFilterIterator_Type */
	{ "SeqHashFilter", (DeeObject *)&librt_get_SeqHashFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* SeqHashFilter_Type */
	{ "SeqHashFilterIterator", (DeeObject *)&librt_get_SeqHashFilterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* SeqHashFilterIterator_Type */
	{ "SeqLocator", (DeeObject *)&librt_get_SeqLocator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                       /* SeqLocator_Type */
	{ "SeqLocatorIterator", (DeeObject *)&librt_get_SeqLocatorIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* SeqLocatorIterator_Type */
	{ "SeqSubRange", (DeeObject *)&librt_get_SeqSubRange, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* SeqSubRange_Type */
	{ "SeqSubRangeIterator", (DeeObject *)&librt_get_SeqSubRangeIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* SeqSubRangeIterator_Type */
	{ "SeqSubRangeN", (DeeObject *)&librt_get_SeqSubRangeN, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* SeqSubRangeN_Type */
	{ "SeqTransformation", (DeeObject *)&librt_get_SeqTransformation, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqTransformation_Type */
	{ "SeqTransformationIterator", (DeeObject *)&librt_get_SeqTransformationIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* SeqTransformationIterator_Type */
	{ "SeqRange", (DeeObject *)&librt_get_SeqRange, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* SeqRange_Type */
	{ "SeqRangeIterator", (DeeObject *)&librt_get_SeqRangeIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* SeqRangeIterator_Type */
	{ "SeqIntRange", (DeeObject *)&librt_get_SeqIntRange, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* SeqIntRange_Type */
	{ "SeqIntRangeIterator", (DeeObject *)&librt_get_SeqIntRangeIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* SeqIntRangeIterator_Type */
	{ "SeqRepeat", (DeeObject *)&librt_get_SeqRepeat, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* SeqRepeat_Type */
	{ "SeqRepeatIterator", (DeeObject *)&librt_get_SeqRepeatIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqRepeatIterator_Type */
	{ "SeqItemRepeat", (DeeObject *)&librt_get_SeqItemRepeat, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* SeqItemRepeat_Type */
	{ "SeqItemRepeatIterator", (DeeObject *)&librt_get_SeqItemRepeatIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* SeqItemRepeatIterator_Type */
	{ "SeqIds", (DeeObject *)&librt_get_SeqIds, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                               /* SeqIds_Type */
	{ "SeqIdsIterator", (DeeObject *)&librt_get_SeqIdsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* SeqIdsIterator_Type */
	{ "SeqTypes", (DeeObject *)&librt_get_SeqTypes, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* SeqTypes_Type */
	{ "SeqTypesIterator", (DeeObject *)&librt_get_SeqTypesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* SeqTypesIterator_Type */
	{ "SeqClasses", (DeeObject *)&librt_get_SeqClasses, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                       /* SeqClasses_Type */
	{ "SeqClassesIterator", (DeeObject *)&librt_get_SeqClassesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* SeqClassesIterator_Type */
	/* Seq-each wrapper types. */
	{ "SeqEach", (DeeObject *)&librt_get_SeqEach, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* SeqEach_Type */
	{ "SeqEachOperator", (DeeObject *)&librt_get_SeqEachOperator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqEachOperator_Type */
	{ "SeqEachOperatorIterator", (DeeObject *)&librt_get_SeqEachOperatorIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* SeqEachOperatorIterator_Type */
	{ "SeqEachGetAttr", (DeeObject *)&librt_get_SeqEachGetAttr, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* SeqEachGetAttr_Type */
	{ "SeqEachGetAttrIterator", (DeeObject *)&librt_get_SeqEachGetAttrIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* SeqEachGetAttrIterator_Type */
	{ "SeqEachCallAttr", (DeeObject *)&librt_get_SeqEachCallAttr, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqEachCallAttr_Type */
	{ "SeqEachCallAttrIterator", (DeeObject *)&librt_get_SeqEachCallAttrIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* SeqEachCallAttrIterator_Type */
	{ "SeqEachCallAttrKw", (DeeObject *)&librt_get_SeqEachCallAttrKw, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqEachCallAttrKw_Type */
	{ "SeqEachCallAttrKwIterator", (DeeObject *)&librt_get_SeqEachCallAttrKwIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* SeqEachCallAttrKwIterator_Type */
	/* TODO: SeqRemoveIfAllWrapper_Type */

	/* Internal types used to drive set proxies */
	{ "SetUnion", (DeeObject *)&librt_get_SetUnion, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* SetUnion_Type */
	{ "SetUnionIterator", (DeeObject *)&librt_get_SetUnionIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SetUnionIterator_Type */
	{ "SetSymmetricDifference", (DeeObject *)&librt_get_SetSymmetricDifference, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* SetSymmetricDifference_Type */
	{ "SetSymmetricDifferenceIterator", (DeeObject *)&librt_get_SetSymmetricDifferenceIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* SetSymmetricDifferenceIterator_Type */
	{ "SetIntersection", (DeeObject *)&librt_get_SetIntersection, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* SetIntersection_Type */
	{ "SetIntersectionIterator", (DeeObject *)&librt_get_SetIntersectionIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* SetIntersectionIterator_Type */
	{ "SetDifference", (DeeObject *)&librt_get_SetDifference, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* SetDifference_Type */
	{ "SetDifferenceIterator", (DeeObject *)&librt_get_SetDifferenceIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* SetDifferenceIterator_Type */
	{ "SetInversion", (DeeObject *)&DeeSetInversion_Type, MODSYM_FREADONLY },

	/* Internal types used to drive mapping proxies */
	{ "MappingProxy", (DeeObject *)&librt_get_MappingProxy, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("Base class for ?GMappingKeys, ?GMappingValues and ?GMappingItems") }, /* DeeMappingProxy_Type */
	{ "MappingProxyIterator", (DeeObject *)&librt_get_MappingProxyIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("Iterator class for ?GMappingProxy, and Base class for ?GMappingKeysIterator, ?GMappingValuesIterator and ?GMappingItemsIterator") }, /* DeeMappingProxyIterator_Type */
	{ "MappingKeys", (DeeObject *)&librt_get_MappingKeys, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("General purpose, sequence proxy type for viewing the keys of an abstract mapping object\n"
	      "When not overwritten by the mapping type itself, this is the type of sequence that's returned "
	      /**/ "by :Mapping.keys") }, /* DeeMappingKeys_Type */
	{ "MappingKeysIterator", (DeeObject *)&librt_get_MappingKeysIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeMappingKeysIterator_Type */
	{ "MappingValues", (DeeObject *)&librt_get_MappingValues, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("General purpose, sequence proxy type for viewing the values of an abstract mapping object\n"
	      "When not overwritten by the mapping type itself, this is the type of sequence that's returned "
	      /**/ "by :Mapping.values") }, /* DeeMappingValues_Type */
	{ "MappingValuesIterator", (DeeObject *)&librt_get_MappingValuesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeMappingValuesIterator_Type */
	{ "MappingItems", (DeeObject *)&librt_get_MappingItems, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("General purpose, sequence proxy type for viewing the items (key-value pairs) of an abstract mapping object\n"
	      "When not overwritten by the mapping type itself, this is the type of sequence that's returned "
	      /**/ "by :Mapping.items") }, /* DeeMappingItems_Type */
	{ "MappingItemsIterator", (DeeObject *)&librt_get_MappingItemsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DeeMappingProxyIterator_Type */
	{ "MappingHashFilter", (DeeObject *)&librt_get_MappingHashFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* MapHashFilter_Type */
	{ "MappingHashFilterIterator", (DeeObject *)&librt_get_MappingHashFilterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* MapHashFilterIterator_Type */
	{ "MappingByAttr", (DeeObject *)&librt_get_MappingByAttr, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* MapByAttr_Type */

	/* Internal types used for safe & fast passing of temporary sequences */
	{ "SharedVector", (DeeObject *)&DeeSharedVector_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },                                      /* DeeSharedVector_Type */
	{ "SharedVectorIterator", (DeeObject *)&librt_get_SharedVectorIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* SharedVectorIterator_Type */
	{ "SharedMap", (DeeObject *)&DeeSharedMap_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },                                            /* DeeSharedMap_Type */
	{ "SharedMapIterator", (DeeObject *)&librt_get_SharedMapIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* SharedMapIterator_Type */
	{ "RefVector", (DeeObject *)&librt_get_RefVector, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* RefVector_Type */
	{ "RefVectorIterator", (DeeObject *)&librt_get_RefVectorIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* RefVectorIterator_Type */

	/* Internal types used to drive sequence operations on `Bytes' */
	{ "BytesFind", (DeeObject *)&librt_get_BytesFind, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* BytesFind_Type */
	{ "BytesFindIterator", (DeeObject *)&librt_get_BytesFindIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* BytesFindIterator_Type */
	{ "BytesCaseFind", (DeeObject *)&librt_get_BytesCaseFind, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* BytesCaseFind_Type */
	{ "BytesCaseFindIterator", (DeeObject *)&librt_get_BytesCaseFindIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* BytesCaseFindIterator_Type */
	{ "BytesSegments", (DeeObject *)&librt_get_BytesSegments, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* BytesSegments_Type */
	{ "BytesSegmentsIterator", (DeeObject *)&librt_get_BytesSegmentsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* BytesSegmentsIterator_Type */
	{ "BytesSplit", (DeeObject *)&librt_get_BytesSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* BytesSplit_Type */
	{ "BytesSplitIterator", (DeeObject *)&librt_get_BytesSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* BytesSplitIterator_Type */
	{ "BytesCaseSplit", (DeeObject *)&librt_get_BytesCaseSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* BytesCaseSplit_Type */
	{ "BytesCaseSplitIterator", (DeeObject *)&librt_get_BytesCaseSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* BytesCaseSplitIterator_Type */
	{ "BytesLineSplit", (DeeObject *)&librt_get_BytesLineSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* BytesLineSplit_Type */
	{ "BytesLineSplitIterator", (DeeObject *)&librt_get_BytesLineSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* BytesLineSplitIterator_Type */

	/* Internal types used to drive sequence operations on `string' */
	{ "StringScan", (DeeObject *)&librt_get_StringScan, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* StringScan_Type */
	{ "StringScanIterator", (DeeObject *)&librt_get_StringScanIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* StringScanIterator_Type */
	{ "StringFind", (DeeObject *)&librt_get_StringFind, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* StringFind_Type */
	{ "StringFindIterator", (DeeObject *)&librt_get_StringFindIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* StringFindIterator_Type */
	{ "StringCaseFind", (DeeObject *)&librt_get_StringCaseFind, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* StringCaseFind_Type */
	{ "StringCaseFindIterator", (DeeObject *)&librt_get_StringCaseFindIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* StringCaseFindIterator_Type */
	{ "StringOrdinals", (DeeObject *)&librt_get_StringOrdinals, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* StringOrdinals_Type */
	{ "StringOrdinalsIterator", (DeeObject *)&librt_get_StringOrdinalsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* StringOrdinalsIterator_Type */
	{ "StringSegments", (DeeObject *)&librt_get_StringSegments, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* StringSegments_Type */
	{ "StringSegmentsIterator", (DeeObject *)&librt_get_StringSegmentsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* StringSegmentsIterator_Type */
	{ "StringSplit", (DeeObject *)&librt_get_StringSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* StringSplit_Type */
	{ "StringSplitIterator", (DeeObject *)&librt_get_StringSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* StringSplitIterator_Type */
	{ "StringCaseSplit", (DeeObject *)&librt_get_StringCaseSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* StringCaseSplit_Type */
	{ "StringCaseSplitIterator", (DeeObject *)&librt_get_StringCaseSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* StringCaseSplitIterator_Type */
	{ "StringLineSplit", (DeeObject *)&librt_get_StringLineSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* StringLineSplit_Type */
	{ "StringLineSplitIterator", (DeeObject *)&librt_get_StringLineSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* StringLineSplitIterator_Type */

	/* Internal types used to drive sequence operations with regular expressions */
	{ "ReFindAll", (DeeObject *)&librt_get_ReFindAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* ReFindAll_Type */
	{ "ReFindAllIterator", (DeeObject *)&librt_get_ReFindAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* ReFindAllIterator_Type */
	{ "RegFindAll", (DeeObject *)&librt_get_RegFindAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* RegFindAll_Type */
	{ "RegFindAllIterator", (DeeObject *)&librt_get_RegFindAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* RegFindAllIterator_Type */
	{ "ReLocateAll", (DeeObject *)&librt_get_ReLocateAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* ReLocateAll_Type */
	{ "ReLocateAllIterator", (DeeObject *)&librt_get_ReLocateAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* ReLocateAllIterator_Type */
	{ "ReSplit", (DeeObject *)&librt_get_ReSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* ReSplit_Type */
	{ "ReSplitIterator", (DeeObject *)&librt_get_ReSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* ReSplitIterator_Type */
	{ "ReGroups", (DeeObject *)&librt_get_ReGroups, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* ReGroups_Type */
	{ "ReGroupsIterator", (DeeObject *)&librt_get_ReGroupsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ReGroupsIterator_Type */
	{ "ReSubStrings", (DeeObject *)&librt_get_ReSubStrings, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* ReSubStrings_Type */
	{ "ReSubStringsIterator", (DeeObject *)&librt_get_ReSubStringsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* ReSubStringsIterator_Type */
	{ "ReSubBytes", (DeeObject *)&librt_get_ReSubBytes, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* ReSubBytes_Type */
	{ "ReSubBytesIterator", (DeeObject *)&librt_get_ReSubBytesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* ReSubBytesIterator_Type */
	{ "ReBytesFindAll", (DeeObject *)&librt_get_ReBytesFindAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* ReFindAll_Type */
	{ "ReBytesFindAllIterator", (DeeObject *)&librt_get_ReBytesFindAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },     /* ReFindAllIterator_Type */
	{ "RegBytesFindAll", (DeeObject *)&librt_get_RegBytesFindAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* RegFindAll_Type */
	{ "RegBytesFindAllIterator", (DeeObject *)&librt_get_RegBytesFindAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* RegFindAllIterator_Type */
	{ "ReBytesLocateAll", (DeeObject *)&librt_get_ReBytesLocateAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ReLocateAll_Type */
	{ "ReBytesLocateAllIterator", (DeeObject *)&librt_get_ReBytesLocateAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* ReLocateAllIterator_Type */
	{ "ReBytesSplit", (DeeObject *)&librt_get_ReBytesSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* ReSplit_Type */
	{ "ReBytesSplitIterator", (DeeObject *)&librt_get_ReBytesSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* ReSplitIterator_Type */

	/* Internal types used to drive module symbol table inspection */
	{ "ModuleExports", (DeeObject *)&librt_get_ModuleExports, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ModuleExports_Type */
	{ "ModuleExportsIterator", (DeeObject *)&librt_get_ModuleExportsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* ModuleExportsIterator_Type */
	{ "ModuleGlobals", (DeeObject *)&librt_get_ModuleGlobals, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ModuleGlobals_Type */
	{ "ModuleGlobalsIterator", (DeeObject *)&librt_get_ModuleGlobalsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* ModuleGlobalsIterator_Type */

	/* Internal types used to drive user-defined classes */
	{ "ClassOperatorTable", (DeeObject *)&librt_get_ClassOperatorTable, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* ClassOperatorTable_Type */
	{ "ClassOperatorTableIterator", (DeeObject *)&librt_get_ClassOperatorTableIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* ClassOperatorTableIterator_Type */
	{ "ClassAttribute", (DeeObject *)&librt_get_ClassAttribute, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* ClassAttribute_Type */
	{ "ClassAttributeTable", (DeeObject *)&librt_get_ClassAttributeTable, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ClassAttributeTable_Type */
	{ "ClassAttributeTableIterator", (DeeObject *)&librt_get_ClassAttributeTableIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* ClassAttributeTableIterator_Type */
	{ "ObjectTable", (DeeObject *)&librt_get_ObjectTable, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* ObjectTable_Type */
	{ "TypeMRO", (DeeObject *)&librt_get_TypeMRO, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* TypeMRO_Type */
	{ "TypeMROIterator", (DeeObject *)&librt_get_TypeMROIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* TypeMROIterator_Type */
	{ "TypeBases", (DeeObject *)&librt_get_TypeBases, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* TypeBases_Type */
	{ "TypeBasesIterator", (DeeObject *)&librt_get_TypeBasesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* TypeBasesIterator_Type */

	/* Internal types used to drive natively defined types */
	{ "TypeOperators", (DeeObject *)&librt_get_TypeOperators, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("Sequence type used to enumerate operators that have been overwritten by a given type\n"
	      "A sequence of this type is returned by ?A__operators__?DType and ?A__operatorids__?DType") }, /* TypeOperators_Type */
	{ "TypeOperatorsIterator", (DeeObject *)&librt_get_TypeOperatorsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* TypeOperatorsIterator_Type */

	/* Internal types used to drive the garbage collector */
	{ "GCSet", (DeeObject *)&librt_get_GCSet, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR, /* DeeGCSet_Type */
	  DOC("The set-like type returned by ?Areferred?Dgc, ?Areferredgc?Dgc, "
	      /**/ "?Areachable?Dgc, ?Areachablegc?Dgc and ?Areferring?Dgc") },
	{ "GCSetIterator", (DeeObject *)&librt_get_GCSetIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeGCSetIterator_Type */

	/* Internal types used to drive variable keyword arguments */
	{ "BlackListKwds", (DeeObject *)&DeeBlackListKwds_Type, MODSYM_FREADONLY },
	{ "BlackListKwdsIterator", /* DeeBlackListKwdsIterator_Type */
	  (DeeObject *)&librt_get_BlackListKwdsIterator,
	  MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },
	{ "BlackListKw", (DeeObject *)&DeeBlackListKw_Type, MODSYM_FREADONLY },
	{ "BlackListKwIterator", /* DeeBlackListKwIterator_Type */
	  (DeeObject *)&librt_get_BlackListKwIterator,
	  MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },
	{ "CachedDict", (DeeObject *)&DeeCachedDict_Type, MODSYM_FREADONLY },
	{ "CachedDictIterator", /* CachedDictIterator_Type */
	  (DeeObject *)&librt_get_CachedDictIterator,
	  MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },
	{ "kw", (DeeObject *)&librt_kw, MODSYM_FNORMAL, /* varying */
	  DOC("(map:?DMapping)->?DMapping\n"
	      "Ensure that @map can be used as a keywords argument in the C API (s.a. ?A__iskw__?DType)\n"
	      "You should never have to call this function. It is mainly here to expose that detail of the "
	      /**/ "#IGATW deemon implementation.") },

	/* Internal types used to drive keyword argument support */
	{ "DocKwds", /* DocKwds_Type */
	  (DeeObject *)&librt_get_DocKwds,
	  MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("Internal type for enumerating the keywords of functions implemented in C\n"
	      "This is done via the associated doc string, with this sequence type being "
	      /**/ "used to implement the string processing. This type is then returned by "
	      /**/ "the $__kwds__ attributes of ?GKwCMethod, ?GKwObjMethod and ?GKwClassMethod "
	      /**/ "when the associated documentation string was found to be non-empty") },
	{ "DocKwdsIterator", /* DocKwdsIterator_Type */
	  (DeeObject *)&librt_get_DocKwdsIterator,
	  MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },

	/* Special types exposed by the C API, but not normally visible to user-code. */
	{ "InteractiveModule", (DeeObject *)&DeeInteractiveModule_Type, MODSYM_FREADONLY,
	  DOC("The type used to implement an interactive module, as available by #C{deemon -i}") },
#ifndef CONFIG_NO_DEX
	{ "DexModule", (DeeObject *)&DeeDex_Type, MODSYM_FREADONLY,
	  DOC("The type of a module that has been loaded from a machine-level shared library.") },
#endif /* !CONFIG_NO_DEX */
	{ "Compiler", (DeeObject *)&DeeCompiler_Type, MODSYM_FREADONLY,
	  DOC("A user-code interface for the compiler used by this implementation") },
	/* TODO: All of the different compiler wrapper types, as well as the internal types for Ast and the different Scopes:
	 *  - DeeCompilerObjItem_Type
	 *  - DeeCompilerWrapper_Type
	 *  - DeeCompilerLexer_Type
	 *  - DeeCompilerLexerExtensions_Type
	 *  - DeeCompilerLexerWarnings_Type
	 *  - DeeCompilerLexerIfdef_Type
	 *  - DeeCompilerLexerToken_Type
	 *  - DeeCompilerParser_Type
	 *  - DeeCompilerAst_Type
	 *  - DeeCompilerScope_Type
	 *  - DeeCompilerBaseScope_Type
	 *  - DeeCompilerRootScope_Type
	 *  - DeeAst_Type
	 *  - DeeScope_Type
	 *  - DeeClassScope_Type
	 *  - DeeBaseScope_Type
	 *  - DeeRootScope_Type
	 */
	{ "ClassDescriptor", (DeeObject *)&DeeClassDescriptor_Type, MODSYM_FREADONLY,
	  DOC("The descriptor type generated by the compiler as a prototype for how a class will be created at runtime (s.a. ?Gmakeclass).") },
	{ "InstanceMember", (DeeObject *)&DeeInstanceMember_Type, MODSYM_FREADONLY,
	  DOC("An unbund class-\\>instance member (e.g. ${class MyClass { member foo; } type(MyClass.foo);})") },
	{ "CMethod", (DeeObject *)&DeeCMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GFunction (e.g. ${boundattr from deemon})") },
	{ "KwCMethod", (DeeObject *)&DeeKwCMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GFunction (with keyword support)") },
	{ "ObjMethod", (DeeObject *)&DeeObjMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GInstanceMethod (e.g. ${\"FOO\".lower'})") },
	{ "KwObjMethod", (DeeObject *)&DeeKwObjMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GInstanceMethod (with keyword support)") },
	{ "ClassMethod", (DeeObject *)&DeeClsMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of an unbound class-\\>instance method (e.g. ${string.lower})") },
	{ "KwClassMethod", (DeeObject *)&DeeKwClsMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of an unbound class-\\>instance method (with keyword support)") },
	{ "ClassProperty", (DeeObject *)&DeeClsProperty_Type, MODSYM_FREADONLY,
	  DOC("C-variant of an unbound class-\\>instance getset (e.g. ${Sequence.length})") },
	{ "ClassMember", (DeeObject *)&DeeClsMember_Type, MODSYM_FREADONLY,
	  DOC("C-variant of an unbound class-\\>instance member (e.g. ${Type.__name__})") },
	{ "FileType", (DeeObject *)&DeeFileType_Type, MODSYM_FREADONLY,
	  DOC("The typetype for file types (i.e. ${type(File)})") },
	{ "YieldFunction", (DeeObject *)&DeeYieldFunction_Type, MODSYM_FREADONLY },
	{ "YieldFunctionIterator", (DeeObject *)&DeeYieldFunctionIterator_Type, MODSYM_FREADONLY },
	{ "RoDict", (DeeObject *)&DeeRoDict_Type, MODSYM_FREADONLY,
	  DOC("A read-only variant of the builtin ?GDict type, aka. ?AFrozen?DDict. "
	      /**/ "Used by the compiler to construct constant, generic mapping expression.") },
	{ "RoDictIterator", (DeeObject *)&librt_get_RoDictIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },/* RoDictIterator_Type */
	{ "RoSet", (DeeObject *)&DeeRoSet_Type, MODSYM_FREADONLY,
	  DOC("A read-only variant of the builtin ?GHashSet type, aka. ?AFrozen?DHashSet. "
	      /**/ "Used by the compiler to construct constant, generic set expression.") },
	{ "RoSetIterator", (DeeObject *)&librt_get_RoSetIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* RoSetIterator_Type */
	{ "Kwds", (DeeObject *)&DeeKwds_Type, MODSYM_FREADONLY,
	  DOC("The type used to represent keyword arguments being mapped onto positional arguments.") },
	{ "KwdsIterator", (DeeObject *)&librt_get_KwdsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeKwdsIterator_Type */
	{ "KwdsMapping", (DeeObject *)&DeeKwdsMapping_Type, MODSYM_FREADONLY,
	  DOC("A wrapper around ?GKwds and the associated argc/argv to create a proper Mapping object") },
	{ "KwdsMappingIterator", (DeeObject *)&librt_get_KwdsMappingIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeKwdsMappingIterator_Type */
	{ "DDI", (DeeObject *)&DeeDDI_Type, MODSYM_FREADONLY,
	  DOC("The type used to hold debug information for user-defined code objects (DeemonDebugInformation).") },
	{ "NoMemory_instance", (DeeObject *)&DeeError_NoMemory_instance, MODSYM_FREADONLY },
	{ "StopIteration_instance", (DeeObject *)&DeeError_StopIteration_instance, MODSYM_FREADONLY },
	{ "Interrupt_instance", (DeeObject *)&DeeError_Interrupt_instance, MODSYM_FREADONLY },

	/* Types used to drive general purpose iterator support */
	{ "GenericIterator", (DeeObject *)&librt_get_GenericIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeGenericIterator_Type */
	{ "IteratorPending", (DeeObject *)&librt_get_IteratorPending, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* IteratorPending_Type */
	{ "IteratorFuture", (DeeObject *)&librt_get_IteratorFuture, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* IteratorFuture_Type */

	/* Internal iterator types used to drive builtin sequence objects */
	{ "StringIterator", (DeeObject *)&librt_get_StringIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* StringIterator_Type */
	{ "BytesIterator", (DeeObject *)&librt_get_BytesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* BytesIterator_Type */
	{ "ListIterator", (DeeObject *)&librt_get_ListIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DeeListIterator_Type */
	{ "TupleIterator", (DeeObject *)&librt_get_TupleIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* DeeTupleIterator_Type */
	{ "HashSetIterator", (DeeObject *)&librt_get_HashSetIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },     /* HashSetIterator_Type */
	{ "TracebackIterator", (DeeObject *)&librt_get_TracebackIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeTracebackIterator_Type */

	/* Helper types used to drive the builtin `Dict' type */
	{ "DictIterator", (DeeObject *)&librt_get_DictIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DictIterator_Type */
	{ "DictProxy", (DeeObject *)&DeeDictProxy_Type, MODSYM_FREADONLY },
	{ "DictProxyIterator", (DeeObject *)&librt_get_DictProxyIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DictProxyIterator_Type */
	{ "DictKeys", (DeeObject *)&DeeDictKeys_Type, MODSYM_FREADONLY },
	{ "DictKeysIterator", (DeeObject *)&librt_get_DictKeysIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DictKeysIterator_Type */
	{ "DictItems", (DeeObject *)&DeeDictItems_Type, MODSYM_FREADONLY },
	{ "DictItemsIterator", (DeeObject *)&librt_get_DictItemsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DictItemsIterator_Type */
	{ "DictValues", (DeeObject *)&DeeDictValues_Type, MODSYM_FREADONLY },
	{ "DictValuesIterator", (DeeObject *)&librt_get_DictValuesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DictValuesIterator_Type */

	/* Special instances of non-singleton objects */
	{ "Sequence_empty", (DeeObject *)Dee_EmptySeq, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, empty sequence singleton") },
	{ "Set_empty", (DeeObject *)Dee_EmptySet, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, empty set singleton") },
	{ "Mapping_empty", (DeeObject *)Dee_EmptyMapping, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, empty mapping singleton") },
	{ "RoDict_empty", (DeeObject *)Dee_EmptyRoDict, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("An empty instance of ?GRoDict") },
	{ "Tuple_empty", (DeeObject *)Dee_EmptyTuple, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The empty tuple singleton $\"\"") },
	{ "String_empty", (DeeObject *)Dee_EmptyString, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The empty string singleton $\"\"") },
	{ "Bytes_empty", (DeeObject *)Dee_EmptyBytes, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The empty bytes singleton ${\"\".bytes()}") },
	{ "Int_0", DeeInt_Zero, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The integer constant $0") },
	{ "Int_1", DeeInt_One, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The integer constant $1") },
	{ "Int_m1", DeeInt_MinusOne, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The integer constant ${-1}") },
	{ "Code_empty", (DeeObject *)&librt_get_Code_empty, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("->?GCode\n"
	      "Special instance of ?GCode that immediately returns ?N") }, /* DeeCode_Empty_head.c_code */
	{ "GCSet_empty", (DeeObject *)&librt_get_GCSet_empty, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("->?GGCSet\n"
	      "Special instance of ?GGCSet that is used to describe an empty set of objects") }, /* DeeGCSet_Empty */
	{ "GCEnum_singleton", &DeeGCEnumTracked_Singleton, MODSYM_FREADONLY | MODSYM_FCONSTEXPR,
	  DOC("The gc-singleton which can also be found under ?Dgc") }, /* DeeGCEnumTracked_Singleton */
	{ "GCEnum", (DeeObject *)&librt_get_GCEnum, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("The result of ${type(gc from deemon)}") }, /* GCEnum_Type */
	{ "Traceback_empty", (DeeObject *)&librt_get_Traceback_empty, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("->?GTraceback\n"
	      "The fallback #Iempty traceback") }, /* DeeTraceback_Empty */
	{ "Module_deemon", (DeeObject *)&DeeModule_Deemon, MODSYM_FREADONLY | MODSYM_FCONSTEXPR,
	  DOC("->?GModule\n"
	      "The built-in ?Mdeemon module") }, /* DeeModule_Deemon */
	{ "Module_empty", (DeeObject *)&librt_get_Module_empty, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("->?GModule\n"
	      "The fallback #Iempty module") }, /* DeeModule_Empty */

	/* Re-exports of standard types also exported from `deemon' */
	{ "Int", (DeeObject *)&DeeInt_Type, MODSYM_FREADONLY },
	{ "Bool", (DeeObject *)&DeeBool_Type, MODSYM_FREADONLY },
	{ "Float", (DeeObject *)&DeeFloat_Type, MODSYM_FREADONLY },
	{ "Sequence", (DeeObject *)&DeeSeq_Type, MODSYM_FREADONLY },
	{ "Iterator", (DeeObject *)&DeeIterator_Type, MODSYM_FREADONLY },
	{ "String", (DeeObject *)&DeeString_Type, MODSYM_FREADONLY },
	{ "Bytes", (DeeObject *)&DeeBytes_Type, MODSYM_FREADONLY },
	{ "List", (DeeObject *)&DeeList_Type, MODSYM_FREADONLY },
	{ "Tuple", (DeeObject *)&DeeTuple_Type, MODSYM_FREADONLY },
	{ "HashSet", (DeeObject *)&DeeHashSet_Type, MODSYM_FREADONLY },
	{ "Dict", (DeeObject *)&DeeDict_Type, MODSYM_FREADONLY },
	{ "Traceback", (DeeObject *)&DeeTraceback_Type, MODSYM_FREADONLY },
	{ "Frame", (DeeObject *)&DeeFrame_Type, MODSYM_FREADONLY },
	{ "Module", (DeeObject *)&DeeModule_Type, MODSYM_FREADONLY },
	{ "Set", (DeeObject *)&DeeSet_Type, MODSYM_FREADONLY },
	{ "Mapping", (DeeObject *)&DeeMapping_Type, MODSYM_FREADONLY },
	{ "Code", (DeeObject *)&DeeCode_Type, MODSYM_FREADONLY },
	{ "Function", (DeeObject *)&DeeFunction_Type, MODSYM_FREADONLY },
	{ "Type", (DeeObject *)&DeeType_Type, MODSYM_FREADONLY },
	{ "Object", (DeeObject *)&DeeObject_Type, MODSYM_FREADONLY },
	{ "Callable", (DeeObject *)&DeeCallable_Type, MODSYM_FREADONLY },
	{ "Numeric", (DeeObject *)&DeeNumeric_Type, MODSYM_FREADONLY },
	{ "InstanceMethod", (DeeObject *)&DeeInstanceMethod_Type, MODSYM_FREADONLY },
	{ "Property", (DeeObject *)&DeeProperty_Type, MODSYM_FREADONLY },
	{ "Super", (DeeObject *)&DeeSuper_Type, MODSYM_FREADONLY },
	{ "Thread", (DeeObject *)&DeeThread_Type, MODSYM_FREADONLY },
	{ "WeakRef", (DeeObject *)&DeeWeakRef_Type, MODSYM_FREADONLY },
	{ "Cell", (DeeObject *)&DeeCell_Type, MODSYM_FREADONLY },
	{ "File", (DeeObject *)&DeeFile_Type, MODSYM_FREADONLY,
	  DOC("(intended) base class for all file types (is to ?GFileType what ?GObject is to ?GType).") },
	{ "FileBuffer", (DeeObject *)&DeeFileBuffer_Type, MODSYM_FREADONLY }, /* `File.Buffer' */
	{ "SystemFile", (DeeObject *)&DeeSystemFile_Type, MODSYM_FREADONLY,
	  DOC("Base class for file types that are managed by the system.") },
	{ "FSFile", (DeeObject *)&DeeFSFile_Type, MODSYM_FREADONLY,
	  DOC("Derived from ?GSystemFile: A system file that has been opened via the file system.") },
	{ "MapFile", (DeeObject *)&DeeMapFile_Type, MODSYM_FREADONLY,
	  DOC("Owner type for mmap buffers used during large file reads.") },
	{ "NoneType", (DeeObject *)&DeeNone_Type, MODSYM_FREADONLY },         /* `type(none)' */
	{ "None", Dee_None, MODSYM_FREADONLY },                               /* `none' */
	{ "MemoryFile", (DeeObject *)&DeeMemoryFile_Type, MODSYM_FREADONLY,   /* An internal file type for streaming from read-only raw memory. */
	  DOC("A special file type that may be used by the deemon runtime to temporarily "
	      /**/ "allow user-code access to raw memory regions via the file interface, rather "
	      /**/ "than the bytes interface. Note however that this type of file cannot be "
	      /**/ "constructed from user-code such that it would reference data, and that memory "
	      /**/ "files impose special access restrictions to prevent user-code from maintaining "
	      /**/ "access to wrapped memory once the file's creator destroys it.") },
	{ "FileReader", (DeeObject *)&DeeFileReader_Type, MODSYM_FREADONLY },             /* `File.Reader' */
	{ "FileWriter", (DeeObject *)&DeeFileWriter_Type, MODSYM_FREADONLY },             /* `File.Writer' */
	{ "FilePrinter", (DeeObject *)&DeeFilePrinter_Type, MODSYM_FREADONLY,
	  DOC("Internal file-type for wrapping #Cdformatprinter when invoking user-defined print/printrepr operators") },
	{ "Attribute", (DeeObject *)&DeeAttribute_Type, MODSYM_FREADONLY },               /* `Attribute' */
	{ "EnumAttr", (DeeObject *)&DeeEnumAttr_Type, MODSYM_FREADONLY },                 /* `enumattr' */
	{ "EnumAttrIterator", (DeeObject *)&DeeEnumAttrIterator_Type, MODSYM_FREADONLY }, /* `enumattr.Iterator' */

	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END

#endif /* !GUARD_DEX_RT_LIBRT_C */
