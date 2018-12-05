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
#ifndef GUARD_DEX_RT_LIBRT_C
#define GUARD_DEX_RT_LIBRT_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/float.h>
#include <deemon/map.h>
#include <deemon/set.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/traceback.h>
#include <deemon/bytes.h>
#include <deemon/seq.h>
#include <deemon/list.h>
#include <deemon/tuple.h>
#include <deemon/hashset.h>
#include <deemon/dict.h>
#include <deemon/compiler/compiler.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/thread.h>

#include "librt.h"

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
 * Since the `rt' module as a whole is non-portable, exports symbols are not
 * required to include the `_np' suffix normally required for such symbols. */




PRIVATE DREF DeeObject *DCALL
librt_getstacklimit_f(size_t argc, DeeObject **__restrict argv) {
 uint16_t result;
 if (DeeArg_Unpack(argc,argv,":getstacklimit"))
     return NULL;
 result = ATOMIC_READ(DeeExec_StackLimit);
 return DeeInt_NewU16(result);
}
PRIVATE DREF DeeObject *DCALL
librt_setstacklimit_f(size_t argc, DeeObject **__restrict argv) {
 uint16_t result,newval = DEE_CONFIG_DEFAULT_STACK_LIMIT;
 if (DeeArg_Unpack(argc,argv,"|I16u:setstacklimit",&newval))
     return NULL;
 result = ATOMIC_XCH(DeeExec_StackLimit,newval);
 return DeeInt_NewU16(result);
}

PRIVATE DEFINE_CMETHOD(librt_getstacklimit,librt_getstacklimit_f);
PRIVATE DEFINE_CMETHOD(librt_setstacklimit,librt_setstacklimit_f);

#if 1
#define STR_ITERATOR  COMPILER_CONTAINER_OF(DeeIterator_Type.tp_name,DeeStringObject,s_str)
#else
#define STR_ITERATOR &str_iterator
PRIVATE DEFINE_STRING(str_iterator,"iterator");
#endif

LOCAL DREF DeeObject *DCALL
get_type_of(DREF DeeObject *ob) {
 DREF DeeObject *result = NULL;
 if likely(ob) {
  result = (DeeObject *)Dee_TYPE(ob);
  Dee_Incref(result);
  Dee_Decref_unlikely(ob);
 }
 return result;
}

LOCAL DREF DeeObject *DCALL
get_iterator_of(DREF DeeObject *ob) {
 DREF DeeObject *result = NULL;
 if likely(ob) {
  result = DeeObject_GetAttr((DeeObject *)ob,
                             (DeeObject *)STR_ITERATOR);
  Dee_Decref_unlikely(ob);
 }
 return result;
}


LOCAL DREF DeeObject *DCALL
librt_get_ClassOperatorTable_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type,"OperatorTable");
}
LOCAL DREF DeeObject *DCALL
librt_get_ClassAttribute_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type,"Attribute");
}
LOCAL DREF DeeObject *DCALL
librt_get_ClassAttributeTable_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type,"AttributeTable");
}
LOCAL DREF DeeObject *DCALL
librt_get_ObjectTable_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeClassDescriptor_Type,"ObjectTable");
}

PRIVATE DREF DeeObject *DCALL
librt_get_ClassOperatorTable_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ClassOperatorTable_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ClassOperatorTableIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ClassOperatorTable_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_ClassAttribute_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ClassAttribute_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ClassAttributeTable_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ClassAttributeTable_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ClassAttributeTableIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ClassAttributeTable_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_ObjectTable_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ObjectTable_impl_f();
}


PRIVATE DREF DeeObject *DCALL
librt_get_RoDictIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeRoDict_Type,
                          (DeeObject *)STR_ITERATOR);
}

PRIVATE DREF DeeObject *DCALL
librt_get_RoSetIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeRoSet_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_KwdsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeKwds_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_KwdsMappingIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeKwdsMapping_Type,
                          (DeeObject *)STR_ITERATOR);
}


PRIVATE DREF DeeObject *DCALL
librt_get_GenericIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 /* The internal `_ObjectTable' type doesn't implement its own iterator type,
  * but instead uses the generic iterator. - By requesting access to the iterator
  * that's being used, we can thereby gain backdoor access to the internal, generic
  * iterator implementation type. */
 return get_iterator_of(librt_get_ObjectTable_impl_f());
}


LOCAL DREF DeeObject *DCALL
get_generic_iterator_member_type(char const *__restrict name) {
 /* return type(iterator().operator . (name)); */
 DREF DeeObject *result,*stub_iterator,*member;
 stub_iterator = DeeObject_NewDefault(&DeeIterator_Type);
 if unlikely(!stub_iterator) goto err;
 member = DeeObject_GetAttrString(stub_iterator,name);
 Dee_Decref_likely(stub_iterator);
 if unlikely(!member) goto err;
 result = (DREF DeeObject *)Dee_TYPE(member);
 Dee_Incref(result);
 Dee_Decref_likely(member);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
librt_get_IteratorPending_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_generic_iterator_member_type("pending");
}
PRIVATE DREF DeeObject *DCALL
librt_get_IteratorFuture_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_generic_iterator_member_type("future");
}




PRIVATE DREF DeeObject *DCALL
librt_get_StringIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeString_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeBytes_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_ListIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeList_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_TupleIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeTuple_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_HashSetIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeHashSet_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_DictIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeDict_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_DictProxyIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeDictProxy_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_DictKeysIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeDictKeys_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_DictItemsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeDictItems_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_DictValuesIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeDictValues_Type,
                          (DeeObject *)STR_ITERATOR);
}
PRIVATE DREF DeeObject *DCALL
librt_get_TracebackIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return DeeObject_GetAttr((DeeObject *)&DeeTraceback_Type,
                          (DeeObject *)STR_ITERATOR);
}
LOCAL DREF DeeObject *DCALL
librt_get_GCEnum_singleton_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(),"gc");
}
PRIVATE DREF DeeObject *DCALL
librt_get_GCEnum_singleton_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_GCEnum_singleton_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_GCEnum_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_type_of(librt_get_GCEnum_singleton_impl_f());
}
LOCAL DREF DeeObject *DCALL
librt_get_GCSet_empty_impl_f(void) {
 DREF DeeObject *gc,*result = NULL;
 gc = librt_get_GCEnum_singleton_impl_f();
 if likely(gc) {
  DeeObject *argv[] = { Dee_None };
  result = DeeObject_CallAttrString(gc,"reachable",1,argv);
  Dee_Decref(gc);
 }
 return result;
}
PRIVATE DREF DeeObject *DCALL
librt_get_GCSet_empty_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_GCSet_empty_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_GCSet_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_type_of(librt_get_GCSet_empty_impl_f());
}




LOCAL DREF DeeObject *DCALL
librt_get_DocKwds_impl_f(void) {
 /* To implement this, we need to get access to an instance of it,
  * which we are doing via `type((import_ from deemon).__kwds__)'.
  * Because the `import()' function is known to implement keyword
  * support, we can use it as a reference point for a C-level function
  * with a non-empty keyword list, without having to create such an
  * object ourself. */
 DREF DeeObject *result,*kwds,*import_func;
 import_func = DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(),"import");
 if unlikely(!import_func) goto err;
 kwds = DeeObject_GetAttrString(import_func,"__kwds__");
 Dee_Decref_unlikely(import_func);
 if unlikely(!kwds) goto err;
 result = (DREF DeeObject *)Dee_TYPE(kwds);
 Dee_Incref(result);
 Dee_Decref_likely(kwds);
 return result;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
librt_get_DocKwds_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_DocKwds_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_DocKwdsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_DocKwds_impl_f());
}


PRIVATE DREF DeeObject *DCALL
librt_get_MappingProxy_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type,"proxy");
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingKeys_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type,"proxy");
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingValues_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type,"values");
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingItems_impl_f(void) {
 return DeeObject_GetAttrString((DeeObject *)&DeeMapping_Type,"items");
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingProxy_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_MappingProxy_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingKeys_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_MappingKeys_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingValues_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_MappingValues_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingItems_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_MappingItems_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingProxyIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_MappingProxy_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingKeysIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_MappingKeys_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingValuesIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_MappingValues_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_MappingItemsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_MappingItems_impl_f());
}

PRIVATE DREF DeeObject *DCALL
librt_get_TypeOperators_impl_f(void) {
 return get_type_of(DeeObject_GetAttrString((DeeObject *)&DeeString_Type,"__operators__"));
}
PRIVATE DREF DeeObject *DCALL
librt_get_TypeOperators_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_TypeOperators_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_TypeOperatorsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_TypeOperators_impl_f());
}


LOCAL DREF DeeObject *DCALL
librt_get_sequence_mutation_type(char const *__restrict name) {
 DREF DeeObject *ob,*result = NULL;
 ob = DeeTuple_Pack(2,Dee_None,Dee_None);
 if likely(ob) {
  DeeObject *argv[] = { &DeeInt_One };
  result = get_type_of(DeeObject_CallAttrString(ob,name,1,argv));
  Dee_Decref(ob);
 }
 return result;
}

LOCAL DREF DeeObject *DCALL
librt_get_string_mutation_type(char const *__restrict name) {
 DeeObject *argv[] = { &DeeInt_One };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,name,1,argv));
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqCombinations_impl_f(void) {
 return librt_get_string_mutation_type("combinations");
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqCombinations_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqCombinations_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqCombinationsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqCombinations_impl_f());
}


LOCAL DREF DeeObject *DCALL
librt_get_SeqRepeatCombinations_impl_f(void) {
 return librt_get_string_mutation_type("repeatcombinations");
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqRepeatCombinations_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqRepeatCombinations_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqRepeatCombinationsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqRepeatCombinations_impl_f());
}


LOCAL DREF DeeObject *DCALL
librt_get_SeqPermutations_impl_f(void) {
 return librt_get_string_mutation_type("permutations");
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqPermutations_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqPermutations_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqPermutationsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqPermutations_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqSegments_impl_f(void) {
 /* Since string overrides `segments', we must use a true sequence here! */
 return librt_get_sequence_mutation_type("segments");
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqSegments_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqSegments_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqSegmentsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqSegments_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqConcat_impl_f(void) {
 return get_type_of(DeeObject_Add(Dee_EmptySeq,Dee_EmptySeq));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqConcat_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqConcat_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqConcatIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqConcat_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqFilter_impl_f(void) {
 DeeObject *argv[] = { Dee_None };
 return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq,"filter",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqFilter_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqFilter_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqFilterIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqFilter_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqLocator_impl_f(void) {
 DeeObject *argv[] = { Dee_None };
 return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq,"locateall",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqLocator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqLocator_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqLocatorIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqLocator_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqSubRange_impl_f(void) {
 return get_type_of(DeeObject_GetRange(Dee_EmptySeq,&DeeInt_Zero,&DeeInt_One));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqSubRange_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqSubRange_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqSubRangeIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqSubRange_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqSubRangeN_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_type_of(DeeObject_GetRange(Dee_EmptySeq,&DeeInt_One,Dee_None));
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqTransformation_impl_f(void) {
 DeeObject *argv[] = { Dee_None };
 return get_type_of(DeeObject_CallAttrString(Dee_EmptySeq,"transform",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqTransformation_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqTransformation_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqTransformationIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqTransformation_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqRange_impl_f(void) {
 return get_type_of(DeeRange_New(Dee_None,Dee_None,NULL));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqRange_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqRange_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqRangeIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqRange_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqIntRange_impl_f(void) {
 return get_type_of(DeeRange_NewInt(0,20,1));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqIntRange_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqIntRange_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqIntRangeIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqIntRange_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqRepeat_impl_f(void) {
 DeeObject *argv[] = { Dee_EmptySeq, &DeeInt_One };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type,"repeatseq",2,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqRepeat_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqRepeat_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqRepeatIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqRepeat_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_SeqItemRepeat_impl_f(void) {
 DeeObject *argv[] = { Dee_None, &DeeInt_One };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&DeeSeq_Type,"repeat",2,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqItemRepeat_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SeqItemRepeat_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SeqItemRepeatIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SeqItemRepeat_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_nonempty_stub_set(void) {
 DREF DeeObject *result;
 result = DeeRoSet_NewWithHint(1);
 if (result && DeeRoSet_Insert(&result,Dee_None))
     Dee_Clear(result);
 return result;
}


LOCAL DREF DeeObject *DCALL
librt_get_SetUnion_impl_f(void) {
 DREF DeeObject *a,*b,*c;
 a = librt_get_nonempty_stub_set();
 if unlikely(!a) return NULL;
 b = librt_get_nonempty_stub_set();
 if unlikely(!b) { Dee_Decref(a); return NULL; }
 c = DeeObject_Or(a,b);
 Dee_Decref(b);
 Dee_Decref(a);
 return get_type_of(c);
}
LOCAL DREF DeeObject *DCALL
librt_get_SetSymmetricDifference_impl_f(void) {
 DREF DeeObject *a,*b,*c;
 a = librt_get_nonempty_stub_set();
 if unlikely(!a) return NULL;
 b = librt_get_nonempty_stub_set();
 if unlikely(!b) { Dee_Decref(a); return NULL; }
 c = DeeObject_Xor(a,b);
 Dee_Decref(b);
 Dee_Decref(a);
 return get_type_of(c);
}
LOCAL DREF DeeObject *DCALL
librt_get_SetIntersection_impl_f(void) {
 DREF DeeObject *a,*b,*c;
 a = librt_get_nonempty_stub_set();
 if unlikely(!a) return NULL;
 b = librt_get_nonempty_stub_set();
 if unlikely(!b) { Dee_Decref(a); return NULL; }
 c = DeeObject_And(a,b);
 Dee_Decref(b);
 Dee_Decref(a);
 return get_type_of(c);
}
LOCAL DREF DeeObject *DCALL
librt_get_SetDifference_impl_f(void) {
 DREF DeeObject *a,*b,*c;
 a = librt_get_nonempty_stub_set();
 if unlikely(!a) return NULL;
 b = librt_get_nonempty_stub_set();
 if unlikely(!b) { Dee_Decref(a); return NULL; }
 c = DeeObject_Sub(a,b);
 Dee_Decref(b);
 Dee_Decref(a);
 return get_type_of(c);
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetUnion_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SetUnion_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetUnionIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SetUnion_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetSymmetricDifference_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SetSymmetricDifference_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetSymmetricDifferenceIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SetSymmetricDifference_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetIntersection_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SetIntersection_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetIntersectionIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SetIntersection_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetDifference_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SetDifference_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SetDifferenceIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SetDifference_impl_f());
}




PRIVATE DREF DeeObject *DCALL
librt_get_SharedVector_impl_f(void) {
 DREF DeeObject *ob,*result = NULL;
 ob = DeeSharedVector_NewShared(0,NULL);
 if likely(ob) {
  result = (DREF DeeObject *)Dee_TYPE(ob);
  Dee_Incref(result);
  DeeSharedVector_Decref(ob);
 }
 return result;
}
PRIVATE DREF DeeObject *DCALL
librt_get_SharedVector_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SharedVector_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SharedVectorIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SharedVector_impl_f());
}


PRIVATE DREF DeeObject *DCALL
librt_get_SharedMap_impl_f(void) {
 DREF DeeObject *ob,*result = NULL;
 ob = DeeSharedMap_NewShared(0,NULL);
 if likely(ob) {
  result = (DREF DeeObject *)Dee_TYPE(ob);
  Dee_Incref(result);
  DeeSharedMap_Decref(ob);
 }
 return result;
}
PRIVATE DREF DeeObject *DCALL
librt_get_SharedMap_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_SharedMap_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_SharedMapIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_SharedMap_impl_f());
}


PRIVATE DREF DeeObject *DCALL
librt_get_RefVector_impl_f(void) {
 return get_type_of(DeeRefVector_NewReadonly(Dee_None,0,NULL));
}
PRIVATE DREF DeeObject *DCALL
librt_get_RefVector_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_RefVector_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_RefVectorIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_RefVector_impl_f());
}




PRIVATE DREF DeeObject *DCALL
librt_get_ModuleExports_impl_f(void) {
 return get_type_of(DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(),"__exports__"));
}
PRIVATE DREF DeeObject *DCALL
librt_get_ModuleExports_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ModuleExports_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ModuleExportsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ModuleExports_impl_f());
}

PRIVATE DREF DeeObject *DCALL
librt_get_ModuleGlobals_impl_f(void) {
 return get_type_of(DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(),"__globals__"));
}
PRIVATE DREF DeeObject *DCALL
librt_get_ModuleGlobals_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ModuleGlobals_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ModuleGlobalsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ModuleGlobals_impl_f());
}



PRIVATE DEFINE_BYTES(small_bytes,DEE_BUFFER_FREADONLY,1,{ 0 });
LOCAL DREF DeeObject *DCALL
librt_get_BytesFind_impl_f(void) {
 DeeObject *argv[] = { &DeeInt_Zero };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes,"findall",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesFind_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_BytesFind_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesFindIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_BytesFind_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_BytesCaseFind_impl_f(void) {
 DeeObject *argv[] = { &DeeInt_Zero };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes,"casefindall",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesCaseFind_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_BytesCaseFind_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesCaseFindIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_BytesCaseFind_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_BytesSegments_impl_f(void) {
 DeeObject *argv[] = { &DeeInt_One };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes,"segments",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesSegments_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_BytesSegments_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesSegmentsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_BytesSegments_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_BytesSplit_impl_f(void) {
 DeeObject *argv[] = { &DeeInt_Zero };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes,"split",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_BytesSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_BytesSplit_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_BytesCaseSplit_impl_f(void) {
 DeeObject *argv[] = { &DeeInt_Zero };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes,"casesplit",1,argv));
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesCaseSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_BytesCaseSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesCaseSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_BytesCaseSplit_impl_f());
}

LOCAL DREF DeeObject *DCALL
librt_get_BytesLineSplit_impl_f(void) {
 return get_type_of(DeeObject_CallAttrString((DeeObject *)&small_bytes,"splitlines",0,NULL));
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesLineSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_BytesLineSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_BytesLineSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_BytesLineSplit_impl_f());
}






LOCAL DREF DeeObject *DCALL
librt_get_StringSegments_impl_f(void) {
 return librt_get_string_mutation_type("segments");
}
LOCAL DREF DeeObject *DCALL
librt_get_StringFind_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"findall",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_StringCaseFind_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"casefindall",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_StringSplit_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"split",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_StringCaseSplit_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"casesplit",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_StringLineSplit_impl_f(void) {
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"splitlines",0,NULL));
}
LOCAL DREF DeeObject *DCALL
librt_get_StringScan_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"scanf",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_StringOrdinals_impl_f(void) {
 return get_type_of(DeeObject_GetAttrString((DeeObject *)STR_ITERATOR,"ordinals"));
}


PRIVATE DREF DeeObject *DCALL
librt_get_StringSegments_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringSegments_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringSegmentsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringSegments_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringFind_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringFind_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringFindIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringFind_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringCaseFind_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringCaseFind_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringCaseFindIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringCaseFind_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringSplit_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringCaseSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringCaseSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringCaseSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringCaseSplit_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringLineSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringLineSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringLineSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringLineSplit_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringScan_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringScan_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringScanIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringScan_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringOrdinals_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_StringOrdinals_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_StringOrdinalsIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_StringOrdinals_impl_f());
}



LOCAL DREF DeeObject *DCALL
librt_get_ReFindAll_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"refindall",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_ReLocateAll_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"relocateall",1,argv));
}
LOCAL DREF DeeObject *DCALL
librt_get_ReSplit_impl_f(void) {
 DeeObject *argv[] = { (DeeObject *)STR_ITERATOR };
 return get_type_of(DeeObject_CallAttrString((DeeObject *)STR_ITERATOR,"resplit",1,argv));
}

PRIVATE DREF DeeObject *DCALL
librt_get_ReFindAll_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ReFindAll_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ReFindAllIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ReFindAll_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_ReLocateAll_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ReLocateAll_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ReLocateAllIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ReLocateAll_impl_f());
}
PRIVATE DREF DeeObject *DCALL
librt_get_ReSplit_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return librt_get_ReSplit_impl_f();
}
PRIVATE DREF DeeObject *DCALL
librt_get_ReSplitIterator_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
 return get_iterator_of(librt_get_ReSplit_impl_f());
}




PRIVATE DEFINE_CMETHOD(librt_get_SeqCombinations,librt_get_SeqCombinations_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqCombinationsIterator,librt_get_SeqCombinationsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeatCombinations,librt_get_SeqRepeatCombinations_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeatCombinationsIterator,librt_get_SeqRepeatCombinationsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqPermutations,librt_get_SeqPermutations_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqPermutationsIterator,librt_get_SeqPermutationsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSegments,librt_get_SeqSegments_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSegmentsIterator,librt_get_SeqSegmentsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqConcat,librt_get_SeqConcat_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqConcatIterator,librt_get_SeqConcatIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqFilter,librt_get_SeqFilter_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqFilterIterator,librt_get_SeqFilterIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqLocator,librt_get_SeqLocator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqLocatorIterator,librt_get_SeqLocatorIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSubRange,librt_get_SeqSubRange_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSubRangeIterator,librt_get_SeqSubRangeIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqSubRangeN,librt_get_SeqSubRangeN_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqTransformation,librt_get_SeqTransformation_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqTransformationIterator,librt_get_SeqTransformationIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRange,librt_get_SeqRange_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRangeIterator,librt_get_SeqRangeIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqIntRange,librt_get_SeqIntRange_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqIntRangeIterator,librt_get_SeqIntRangeIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeat,librt_get_SeqRepeat_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqRepeatIterator,librt_get_SeqRepeatIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqItemRepeat,librt_get_SeqItemRepeat_f);
PRIVATE DEFINE_CMETHOD(librt_get_SeqItemRepeatIterator,librt_get_SeqItemRepeatIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetUnion,librt_get_SetUnion_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetUnionIterator,librt_get_SetUnionIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetSymmetricDifference,librt_get_SetSymmetricDifference_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetSymmetricDifferenceIterator,librt_get_SetSymmetricDifferenceIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetIntersection,librt_get_SetIntersection_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetIntersectionIterator,librt_get_SetIntersectionIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetDifference,librt_get_SetDifference_f);
PRIVATE DEFINE_CMETHOD(librt_get_SetDifferenceIterator,librt_get_SetDifferenceIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ClassOperatorTable,librt_get_ClassOperatorTable_f);
PRIVATE DEFINE_CMETHOD(librt_get_ClassOperatorTableIterator,librt_get_ClassOperatorTableIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ClassAttribute,librt_get_ClassAttribute_f);
PRIVATE DEFINE_CMETHOD(librt_get_ClassAttributeTable,librt_get_ClassAttributeTable_f);
PRIVATE DEFINE_CMETHOD(librt_get_ClassAttributeTableIterator,librt_get_ClassAttributeTableIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ObjectTable,librt_get_ObjectTable_f);
PRIVATE DEFINE_CMETHOD(librt_get_RoDictIterator,librt_get_RoDictIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_RoSetIterator,librt_get_RoSetIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_KwdsIterator,librt_get_KwdsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_KwdsMappingIterator,librt_get_KwdsMappingIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_GenericIterator,librt_get_GenericIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_IteratorPending,librt_get_IteratorPending_f);
PRIVATE DEFINE_CMETHOD(librt_get_IteratorFuture,librt_get_IteratorFuture_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringIterator,librt_get_StringIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesIterator,librt_get_BytesIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ListIterator,librt_get_ListIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_TupleIterator,librt_get_TupleIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_HashSetIterator,librt_get_HashSetIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_DictIterator,librt_get_DictIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_DictProxyIterator,librt_get_DictProxyIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_DictKeysIterator,librt_get_DictKeysIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_DictItemsIterator,librt_get_DictItemsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_DictValuesIterator,librt_get_DictValuesIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_TracebackIterator,librt_get_TracebackIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_GCSet,librt_get_GCSet_f);
PRIVATE DEFINE_CMETHOD(librt_get_GCSet_empty,librt_get_GCSet_empty_f);
PRIVATE DEFINE_CMETHOD(librt_get_GCEnum_singleton,librt_get_GCEnum_singleton_f);
PRIVATE DEFINE_CMETHOD(librt_get_GCEnum,librt_get_GCEnum_f);
PRIVATE DEFINE_CMETHOD(librt_get_DocKwds,librt_get_DocKwds_f);
PRIVATE DEFINE_CMETHOD(librt_get_DocKwdsIterator,librt_get_DocKwdsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingProxy,librt_get_MappingProxy_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingProxyIterator,librt_get_MappingProxyIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingKeys,librt_get_MappingKeys_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingKeysIterator,librt_get_MappingKeysIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingValues,librt_get_MappingValues_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingValuesIterator,librt_get_MappingValuesIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingItems,librt_get_MappingItems_f);
PRIVATE DEFINE_CMETHOD(librt_get_MappingItemsIterator,librt_get_MappingItemsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SharedVector,librt_get_SharedVector_f);
PRIVATE DEFINE_CMETHOD(librt_get_SharedVectorIterator,librt_get_SharedVectorIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_SharedMap,librt_get_SharedMap_f);
PRIVATE DEFINE_CMETHOD(librt_get_SharedMapIterator,librt_get_SharedMapIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_RefVector,librt_get_RefVector_f);
PRIVATE DEFINE_CMETHOD(librt_get_RefVectorIterator,librt_get_RefVectorIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_TypeOperators,librt_get_TypeOperators_f);
PRIVATE DEFINE_CMETHOD(librt_get_TypeOperatorsIterator,librt_get_TypeOperatorsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleExports,librt_get_ModuleExports_f);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleExportsIterator,librt_get_ModuleExportsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleGlobals,librt_get_ModuleGlobals_f);
PRIVATE DEFINE_CMETHOD(librt_get_ModuleGlobalsIterator,librt_get_ModuleGlobalsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesFind,librt_get_BytesFind_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesFindIterator,librt_get_BytesFindIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseFind,librt_get_BytesCaseFind_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseFindIterator,librt_get_BytesCaseFindIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSegments,librt_get_BytesSegments_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSegmentsIterator,librt_get_BytesSegmentsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSplit,librt_get_BytesSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesSplitIterator,librt_get_BytesSplitIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseSplit,librt_get_BytesCaseSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesCaseSplitIterator,librt_get_BytesCaseSplitIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesLineSplit,librt_get_BytesLineSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_BytesLineSplitIterator,librt_get_BytesLineSplitIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringFind,librt_get_StringFind_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringFindIterator,librt_get_StringFindIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseFind,librt_get_StringCaseFind_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseFindIterator,librt_get_StringCaseFindIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringSegments,librt_get_StringSegments_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringSegmentsIterator,librt_get_StringSegmentsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringSplit,librt_get_StringSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringSplitIterator,librt_get_StringSplitIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseSplit,librt_get_StringCaseSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringCaseSplitIterator,librt_get_StringCaseSplitIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringLineSplit,librt_get_StringLineSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringLineSplitIterator,librt_get_StringLineSplitIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringScan,librt_get_StringScan_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringScanIterator,librt_get_StringScanIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringOrdinals,librt_get_StringOrdinals_f);
PRIVATE DEFINE_CMETHOD(librt_get_StringOrdinalsIterator,librt_get_StringOrdinalsIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ReFindAll,librt_get_ReFindAll_f);
PRIVATE DEFINE_CMETHOD(librt_get_ReFindAllIterator,librt_get_ReFindAllIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ReLocateAll,librt_get_ReLocateAll_f);
PRIVATE DEFINE_CMETHOD(librt_get_ReLocateAllIterator,librt_get_ReLocateAllIterator_f);
PRIVATE DEFINE_CMETHOD(librt_get_ReSplit,librt_get_ReSplit_f);
PRIVATE DEFINE_CMETHOD(librt_get_ReSplitIterator,librt_get_ReSplitIterator_f);


/* NOTE: At first glance, the combination of `MODSYM_FPROPERTY|MODSYM_FCONSTEXPR' may
 *       not look like it would make sense, but by using this combination, we prevent
 *       the symbols to be considered properties during enumeration (`ATTR_PROPERTY'
 *       doesn't get set), thus allowing the doc server to browse them unrestrictedly. */
PRIVATE struct dex_symbol symbols[] = {
    { "getstacklimit", (DeeObject *)&librt_getstacklimit, MODSYM_FNORMAL,
      DOC("->?Dint\n"
          "Returns the current stack limit, that is the max number of "
          "user-code functions that may be executed consecutively before "
          "a :StackOverflow error is thrown\n"
          "The default stack limit is $" PP_STR(DEE_CONFIG_DEFAULT_STACK_LIMIT)) },
    { "setstacklimit", (DeeObject *)&librt_setstacklimit, MODSYM_FNORMAL,
      DOC("(new_limit=!" PP_STR(DEE_CONFIG_DEFAULT_STACK_LIMIT) ")->?Dint\n"
          "@throw IntegerOverflow @new_limit is negative, or greater than $0xffff\n"
          "Set the new stack limit to @new_limit and return the old limit\n"
          "The stack limit is checked every time a user-code function is "
          "entered, at which point a :StackOverflow error is thrown if "
          "the currently set limit is exceeded") },
    { "stacklimitunlimited",
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
      Dee_True
#else
      Dee_False
#endif
      ,
      MODSYM_FNORMAL,
      DOC("->?Dbool\n"
          "A boolean that is :true if the deemon interpreter supports "
          "an unlimited stack limit, meaning that #setstacklimit can "
          "be used to set a stack limit of to up $0xffff ($65535), which "
          "is the hard limit\n"
          "When :false, setting the stack limit higher than the default "
          "may lead to hard crashes of the deemon interpreter, causing "
          "the user-script and hosting application to crash in an undefined "
          "manner.\n"
          "Unlimited stack limit support requires a special arch-specific "
          "sub-routine within the deemon core, which may not be implemented "
          "for an arbitrary architecture") },
    { "SlabStat", (DeeObject *)&SlabStat_Type, MODSYM_FREADONLY }, /* Access to slab allocator statistics. */

    /* Internal types used to drive sequence proxies */
    { "SeqCombinations", (DeeObject *)&librt_get_SeqCombinations, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqCombinations_Type */
    { "SeqCombinationsIterator", (DeeObject *)&librt_get_SeqCombinationsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqCombinationsIterator_Type */
    { "SeqRepeatCombinations", (DeeObject *)&librt_get_SeqRepeatCombinations, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqRepeatCombinations_Type */
    { "SeqRepeatCombinationsIterator", (DeeObject *)&librt_get_SeqRepeatCombinationsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqRepeatCombinationsIterator_Type */
    { "SeqPermutations", (DeeObject *)&librt_get_SeqPermutations, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqPermutations_Type */
    { "SeqPermutationsIterator", (DeeObject *)&librt_get_SeqPermutationsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqPermutationsIterator_Type */
    { "SeqSegments", (DeeObject *)&librt_get_SeqSegments, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqSegments_Type */
    { "SeqSegmentsIterator", (DeeObject *)&librt_get_SeqSegmentsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqSegmentsIterator_Type */
    { "SeqConcat", (DeeObject *)&librt_get_SeqConcat, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqConcat_Type */
    { "SeqConcatIterator", (DeeObject *)&librt_get_SeqConcatIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqConcatIterator_Type */
    { "SeqFilter", (DeeObject *)&librt_get_SeqFilter, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqFilter_Type */
    { "SeqFilterIterator", (DeeObject *)&librt_get_SeqFilterIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqFilterIterator_Type */
    { "SeqLocator", (DeeObject *)&librt_get_SeqLocator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqLocator_Type */
    { "SeqLocatorIterator", (DeeObject *)&librt_get_SeqLocatorIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqLocatorIterator_Type */
    { "SeqSubRange", (DeeObject *)&librt_get_SeqSubRange, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqSubRange_Type */
    { "SeqSubRangeIterator", (DeeObject *)&librt_get_SeqSubRangeIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqSubRangeIterator_Type */
    { "SeqSubRangeN", (DeeObject *)&librt_get_SeqSubRangeN, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqSubRangeN_Type */
    { "SeqTransformation", (DeeObject *)&librt_get_SeqTransformation, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqTransformation_Type */
    { "SeqTransformationIterator", (DeeObject *)&librt_get_SeqTransformationIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqTransformationIterator_Type */
    { "SeqRange", (DeeObject *)&librt_get_SeqRange, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqRange_Type */
    { "SeqRangeIterator", (DeeObject *)&librt_get_SeqRangeIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqRangeIterator_Type */
    { "SeqIntRange", (DeeObject *)&librt_get_SeqIntRange, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqIntRange_Type */
    { "SeqIntRangeIterator", (DeeObject *)&librt_get_SeqIntRangeIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqIntRangeIterator_Type */
    { "SeqRepeat", (DeeObject *)&librt_get_SeqRepeat, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqRepeat_Type */
    { "SeqRepeatIterator", (DeeObject *)&librt_get_SeqRepeatIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqRepeatIterator_Type */
    { "SeqItemRepeat", (DeeObject *)&librt_get_SeqItemRepeat, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqItemRepeat_Type */
    { "SeqItemRepeatIterator", (DeeObject *)&librt_get_SeqItemRepeatIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SeqItemRepeatIterator_Type */
    /* TODO: SeqRemoveIfAllWrapper_Type */

    /* Internal types used to drive set proxies */
    { "SetUnion", (DeeObject *)&librt_get_SetUnion, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetUnion_Type */
    { "SetUnionIterator", (DeeObject *)&librt_get_SetUnionIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetUnionIterator_Type */
    { "SetSymmetricDifference", (DeeObject *)&librt_get_SetSymmetricDifference, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetSymmetricDifference_Type */
    { "SetSymmetricDifferenceIterator", (DeeObject *)&librt_get_SetSymmetricDifferenceIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetSymmetricDifferenceIterator_Type */
    { "SetIntersection", (DeeObject *)&librt_get_SetIntersection, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetIntersection_Type */
    { "SetIntersectionIterator", (DeeObject *)&librt_get_SetIntersectionIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetIntersectionIterator_Type */
    { "SetDifference", (DeeObject *)&librt_get_SetDifference, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetDifference_Type */
    { "SetDifferenceIterator", (DeeObject *)&librt_get_SetDifferenceIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SetDifferenceIterator_Type */

    /* Internal types used to drive mapping proxies */
    { "MappingProxy", (DeeObject *)&librt_get_MappingProxy, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Base class for #MappingKeys, #MappingValues and #MappingItems") }, /* DeeMappingProxy_Type */
    { "MappingProxyIterator", (DeeObject *)&librt_get_MappingProxyIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator class for #MappingProxy, and Base class for #MappingKeysIterator, #MappingValuesIterator and #MappingItemsIterator") }, /* DeeMappingProxyIterator_Type */
    { "MappingKeys", (DeeObject *)&librt_get_MappingKeys, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("General purpose, sequence proxy type for viewing the keys of an abstract mapping object\n"
          "When not overwritten by the mapping type itself, this is the type of sequence that's returned "
          "by :deemon:mapping.i:keys") }, /* DeeMappingKeys_Type */
    { "MappingKeysIterator", (DeeObject *)&librt_get_MappingKeysIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator type for #MappingKeys") }, /* DeeMappingProxyIterator_Type */
    { "MappingValues", (DeeObject *)&librt_get_MappingValues, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("General purpose, sequence proxy type for viewing the values of an abstract mapping object\n"
          "When not overwritten by the mapping type itself, this is the type of sequence that's returned "
          "by :deemon:mapping.i:values") }, /* DeeMappingValues_Type */
    { "MappingValuesIterator", (DeeObject *)&librt_get_MappingValuesIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator type for #MappingValues") }, /* DeeMappingProxyIterator_Type */
    { "MappingItems", (DeeObject *)&librt_get_MappingItems, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("General purpose, sequence proxy type for viewing the items (key-value pairs) of an abstract mapping object\n"
          "When not overwritten by the mapping type itself, this is the type of sequence that's returned "
          "by :deemon:mapping.i:items") }, /* DeeMappingItems_Type */
    { "MappingItemsIterator", (DeeObject *)&librt_get_MappingItemsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator type for #MappingItems") }, /* DeeMappingProxyIterator_Type */

    /* Internal types used for safe & fast passing of temporary sequences */
    { "SharedVector", (DeeObject *)&librt_get_SharedVector, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SharedVector_Type */
    { "SharedVectorIterator", (DeeObject *)&librt_get_SharedVectorIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SharedVectorIterator_Type */
    { "SharedMap", (DeeObject *)&librt_get_SharedMap, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SharedMap_Type */
    { "SharedMapIterator", (DeeObject *)&librt_get_SharedMapIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* SharedMapIterator_Type */
    { "RefVector", (DeeObject *)&librt_get_RefVector, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* RefVector_Type */
    { "RefVectorIterator", (DeeObject *)&librt_get_RefVectorIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* RefVectorIterator_Type */

    /* Internal types used to drive sequence operations on `bytes' */
    { "BytesFind", (DeeObject *)&librt_get_BytesFind, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesFind_Type */
    { "BytesFindIterator", (DeeObject *)&librt_get_BytesFindIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesFindIterator_Type */
    { "BytesCaseFind", (DeeObject *)&librt_get_BytesCaseFind, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesCaseFind_Type */
    { "BytesCaseFindIterator", (DeeObject *)&librt_get_BytesCaseFindIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesCaseFindIterator_Type */
    { "BytesSegments", (DeeObject *)&librt_get_BytesSegments, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesSegments_Type */
    { "BytesSegmentsIterator", (DeeObject *)&librt_get_BytesSegmentsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesSegmentsIterator_Type */
    { "BytesSplit", (DeeObject *)&librt_get_BytesSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesSplit_Type */
    { "BytesSplitIterator", (DeeObject *)&librt_get_BytesSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesSplitIterator_Type */
    { "BytesCaseSplit", (DeeObject *)&librt_get_BytesCaseSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesCaseSplit_Type */
    { "BytesCaseSplitIterator", (DeeObject *)&librt_get_BytesCaseSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesCaseSplitIterator_Type */
    { "BytesLineSplit", (DeeObject *)&librt_get_BytesLineSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesLineSplit_Type */
    { "BytesLineSplitIterator", (DeeObject *)&librt_get_BytesLineSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesLineSplitIterator_Type */

    /* Internal types used to drive sequence operations on `string' */
    { "StringScan", (DeeObject *)&librt_get_StringScan, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringScan_Type */
    { "StringScanIterator", (DeeObject *)&librt_get_StringScanIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringScanIterator_Type */
    { "StringFind", (DeeObject *)&librt_get_StringFind, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringFind_Type */
    { "StringFindIterator", (DeeObject *)&librt_get_StringFindIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringFindIterator_Type */
    { "StringCaseFind", (DeeObject *)&librt_get_StringCaseFind, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringCaseFind_Type */
    { "StringCaseFindIterator", (DeeObject *)&librt_get_StringCaseFindIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringCaseFindIterator_Type */
    { "StringOrdinals", (DeeObject *)&librt_get_StringOrdinals, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringOrdinals_Type */
    { "StringOrdinalsIterator", (DeeObject *)&librt_get_StringOrdinalsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringOrdinalsIterator_Type */
    { "StringSegments", (DeeObject *)&librt_get_StringSegments, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringSegments_Type */
    { "StringSegmentsIterator", (DeeObject *)&librt_get_StringSegmentsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringSegmentsIterator_Type */
    { "StringSplit", (DeeObject *)&librt_get_StringSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringSplit_Type */
    { "StringSplitIterator", (DeeObject *)&librt_get_StringSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringSplitIterator_Type */
    { "StringCaseSplit", (DeeObject *)&librt_get_StringCaseSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringCaseSplit_Type */
    { "StringCaseSplitIterator", (DeeObject *)&librt_get_StringCaseSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringCaseSplitIterator_Type */
    { "StringLineSplit", (DeeObject *)&librt_get_StringLineSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringLineSplit_Type */
    { "StringLineSplitIterator", (DeeObject *)&librt_get_StringLineSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringLineSplitIterator_Type */

    /* Internal types used to drive sequence operations with regular expressions */
    { "ReFindAll", (DeeObject *)&librt_get_ReFindAll, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ReFindAll_Type */
    { "ReFindAllIterator", (DeeObject *)&librt_get_ReFindAllIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ReFindAllIterator_Type */
    { "ReLocateAll", (DeeObject *)&librt_get_ReLocateAll, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ReLocateAll_Type */
    { "ReLocateAllIterator", (DeeObject *)&librt_get_ReLocateAllIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ReLocateAllIterator_Type */
    { "ReSplit", (DeeObject *)&librt_get_ReSplit, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ReSplit_Type */
    { "ReSplitIterator", (DeeObject *)&librt_get_ReSplitIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ReSplitIterator_Type */

    /* Internal types used to drive module symbol table inspection */
    { "ModuleExports", (DeeObject *)&librt_get_ModuleExports, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ModuleExports_Type */
    { "ModuleExportsIterator", (DeeObject *)&librt_get_ModuleExportsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ModuleExportsIterator_Type */
    { "ModuleGlobals", (DeeObject *)&librt_get_ModuleGlobals, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ModuleGlobals_Type */
    { "ModuleGlobalsIterator", (DeeObject *)&librt_get_ModuleGlobalsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ModuleGlobalsIterator_Type */

    /* Internal types used to drive user-defined classes */
    { "ClassOperatorTable", (DeeObject *)&librt_get_ClassOperatorTable, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ClassOperatorTable_Type */
    { "ClassOperatorTableIterator", (DeeObject *)&librt_get_ClassOperatorTableIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ClassOperatorTableIterator_Type */
    { "ClassAttribute", (DeeObject *)&librt_get_ClassAttribute, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ClassAttribute_Type */
    { "ClassAttributeTable", (DeeObject *)&librt_get_ClassAttributeTable, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ClassAttributeTable_Type */
    { "ClassAttributeTableIterator", (DeeObject *)&librt_get_ClassAttributeTableIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ClassAttributeTableIterator_Type */
    { "ObjectTable", (DeeObject *)&librt_get_ObjectTable, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* ObjectTable_Type */

    /* Internal types used to drive natively defined types */
    { "TypeOperators", (DeeObject *)&librt_get_TypeOperators, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Sequence type used to enumerate operators that have been overwritten by a given type\n"
          "A sequence of this type is returned by :deemon.type.i:__operators__ and :deemon.type.i:__operatorids__") }, /* TypeOperators_Type */
    { "TypeOperatorsIterator", (DeeObject *)&librt_get_TypeOperatorsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator type for #TypeOperators") }, /* TypeOperatorsIterator_Type */

    /* Internal types used to drive the garbage collector */
    { "GCSet", (DeeObject *)&librt_get_GCSet, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR, /* DeeGCSet_Type */
      DOC("The set-like type returned by :deemon:gc.referred, :deemon:gc.referredgc, "
          ":deemon:gc.reachable, :deemon:gc.reachablegc and :deemon:gc.referring") },
    { "GCSetIterator", (DeeObject *)&librt_get_GCSet, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator for #GCSet") }, /* DeeGCSetIterator_Type */

    /* Internal types used to drive keyword argument support */
    { "DocKwds", (DeeObject *)&librt_get_DocKwds, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Internal type for enumerating the keywords of functions implemented in C\n"
          "This is done via the associated doc string, with this sequence type being "
          "used to implement the string processing. This type is then returned by "
          "the $__kwds__ attributes of #KwCMethod, #KwObjMethod and #KwClassMethod "
          "when the associated documentation string was found to be non-empty") }, /* DocKwds_Type */
    { "DocKwdsIterator", (DeeObject *)&librt_get_DocKwdsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Iterator type for #DocKwds") }, /* DocKwdsIterator_Type */

    /* Special types exposed by the C API, but not normally visible to user-code. */
    { "InteractiveModule", (DeeObject *)&DeeInteractiveModule_Type, MODSYM_FREADONLY }, /* The type used to implement an interactive module, as available by `deemon -i' */
#ifndef CONFIG_NO_DEX
    { "DexModule", (DeeObject *)&DeeDex_Type, MODSYM_FREADONLY },                   /* The type of a module that has been loaded from a machine-level shared library. */
#endif
    { "Compiler", (DeeObject *)&DeeCompiler_Type, MODSYM_FREADONLY },               /* A user-code interface for the compiler used by this implementation */
    { "ClassDescriptor", (DeeObject *)&DeeClassDescriptor_Type, MODSYM_FREADONLY }, /* The descriptor type generated by the compiler as a prototype for how a class will be created at runtime. */
    { "InstanceMember", (DeeObject *)&DeeInstanceMember_Type, MODSYM_FREADONLY },   /* An unbund class->instance member (e.g. `class MyClass { member foo; } type(MyClass.foo);') */
    { "CMethod", (DeeObject *)&DeeCMethod_Type, MODSYM_FREADONLY },                 /* C-variant of `function' (e.g. `boundattr from deemon') */
    { "KwCMethod", (DeeObject *)&DeeKwCMethod_Type, MODSYM_FREADONLY },             /* C-variant of `function' (with keyword support) */
    { "ObjMethod", (DeeObject *)&DeeObjMethod_Type, MODSYM_FREADONLY },             /* C-variant of `instancemethod' (e.g. `"FOO".lower') */
    { "KwObjMethod", (DeeObject *)&DeeKwObjMethod_Type, MODSYM_FREADONLY },         /* C-variant of `instancemethod' (with keyword support) */
    { "ClassMethod", (DeeObject *)&DeeClsMethod_Type, MODSYM_FREADONLY },           /* C-variant of an unbound class->instance method (e.g. `string.lower') */
    { "KwClassMethod", (DeeObject *)&DeeKwClsMethod_Type, MODSYM_FREADONLY },       /* C-variant of an unbound class->instance method (with keyword support) */
    { "ClassProperty", (DeeObject *)&DeeClsProperty_Type, MODSYM_FREADONLY },       /* C-variant of an unbound class->instance getset (e.g. `sequence.length') */
    { "ClassMember", (DeeObject *)&DeeClsMember_Type, MODSYM_FREADONLY },           /* C-variant of an unbound class->instance member (e.g. `type.__name__') */
    { "FileType", (DeeObject *)&DeeFileType_Type, MODSYM_FREADONLY },               /* `type(file)' -- The typetype for file types. */
    { "YieldFunction", (DeeObject *)&DeeYieldFunction_Type, MODSYM_FREADONLY },
    { "YieldFunctionIterator", (DeeObject *)&DeeYieldFunctionIterator_Type, MODSYM_FREADONLY },
    { "RoDict", (DeeObject *)&DeeRoDict_Type, MODSYM_FREADONLY },                   /* A read-only variant of the builtin `dict' type (used by the compiler to construct constant, generic mapping expression) */
    { "RoDictIterator", (DeeObject *)&librt_get_RoDictIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* RoDictIterator_Type */
    { "RoSet", (DeeObject *)&DeeRoSet_Type, MODSYM_FREADONLY },                     /* A read-only variant of the builtin `hashset' type (used by the compiler to construct constant, generic set expression) */
    { "RoSetIterator", (DeeObject *)&librt_get_RoSetIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* RoSetIterator_Type */
    { "Kwds", (DeeObject *)&DeeKwds_Type, MODSYM_FREADONLY },                       /* The type used to represent keyword arguments being mapped onto positional arguments. */
    { "KwdsIterator", (DeeObject *)&librt_get_KwdsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DeeKwdsIterator_Type */
    { "KwdsMapping", (DeeObject *)&DeeKwdsMapping_Type, MODSYM_FREADONLY },         /* A wrapper around `kwds' and the associated argc/argv to create a proper mapping object */
    { "KwdsMappingIterator", (DeeObject *)&librt_get_KwdsMappingIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DeeKwdsMappingIterator_Type */
    { "Ddi", (DeeObject *)&DeeDDI_Type, MODSYM_FREADONLY },                         /* The type used to hold debug information for user-defined code objects. */
    { "NoMemory_instance", (DeeObject *)&DeeError_NoMemory_instance, MODSYM_FREADONLY },
    { "StopIteration_instance", (DeeObject *)&DeeError_StopIteration_instance, MODSYM_FREADONLY },
    { "Interrupt_instance", (DeeObject *)&DeeError_Interrupt_instance, MODSYM_FREADONLY },

    /* Types used to drive general purpose iteartor support */
    { "GenericIterator", (DeeObject *)&librt_get_GenericIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DeeGenericIterator_Type */
    { "IteratorPending", (DeeObject *)&librt_get_IteratorPending, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* IteratorPending_Type */
    { "IteratorFuture", (DeeObject *)&librt_get_IteratorFuture, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* IteratorFuture_Type */

    /* Internal iterator types used to drive builtin sequence objects */
    { "StringIterator", (DeeObject *)&librt_get_StringIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* StringIterator_Type */
    { "BytesIterator", (DeeObject *)&librt_get_BytesIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* BytesIterator_Type */
    { "ListIterator", (DeeObject *)&librt_get_ListIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DeeListIterator_Type */
    { "TupleIterator", (DeeObject *)&librt_get_TupleIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DeeTupleIterator_Type */
    { "HashSetIterator", (DeeObject *)&librt_get_HashSetIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* HashSetIterator_Type */
    { "TracebackIterator", (DeeObject *)&librt_get_TracebackIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DeeTracebackIterator_Type */

    /* Helper types used to drive the builtin `dict' type */
    { "DictIterator", (DeeObject *)&librt_get_DictIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DictIterator_Type */
    { "DictProxy", (DeeObject *)&DeeDictProxy_Type, MODSYM_FREADONLY },
    { "DictProxyIterator", (DeeObject *)&librt_get_DictProxyIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DictProxyIterator_Type */
    { "DictKeys", (DeeObject *)&DeeDictKeys_Type, MODSYM_FREADONLY },
    { "DictKeysIterator", (DeeObject *)&librt_get_DictKeysIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DictKeysIterator_Type */
    { "DictItems", (DeeObject *)&DeeDictItems_Type, MODSYM_FREADONLY },
    { "DictItemsIterator", (DeeObject *)&librt_get_DictItemsIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DictItemsIterator_Type */
    { "DictValues", (DeeObject *)&DeeDictValues_Type, MODSYM_FREADONLY },
    { "DictValuesIterator", (DeeObject *)&librt_get_DictValuesIterator, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR }, /* DictValuesIterator_Type */

    /* Special instances of non-singleton objects */
    { "Sequence_empty", (DeeObject *)Dee_EmptySeq, MODSYM_FREADONLY, DOC("A general-purpose, empty sequence singleton") },
    { "Set_empty", (DeeObject *)Dee_EmptySet, MODSYM_FREADONLY, DOC("A general-purpose, empty set singleton") },
    { "Mapping_empty", (DeeObject *)Dee_EmptyMapping, MODSYM_FREADONLY, DOC("A general-purpose, empty mapping singleton") },
    { "Tuple_empty", (DeeObject *)Dee_EmptyTuple, MODSYM_FREADONLY, DOC("The empty tuple singleton $\"\"") },
    { "String_empty", (DeeObject *)Dee_EmptyString, MODSYM_FREADONLY, DOC("The empty string singleton $\"\"") },
    { "Bytes_empty", (DeeObject *)Dee_EmptyBytes, MODSYM_FREADONLY, DOC("The empty bytes singleton ${\"\".bytes()}") },
    { "Int_0", (DeeObject *)&DeeInt_Zero, MODSYM_FREADONLY, DOC("The integer constant $0") },
    { "Int_1", (DeeObject *)&DeeInt_One, MODSYM_FREADONLY, DOC("The integer constant $1") },
    { "Int_m1", (DeeObject *)&DeeInt_MinusOne, MODSYM_FREADONLY, DOC("The integer constant ${-1}") },
    { "GCSet_empty", (DeeObject *)&librt_get_GCSet_empty, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("Special instance of #GCSet that is used to describe an empty set of objects") }, /* DeeGCSet_Empty */
    { "GCEnum_singleton", (DeeObject *)&librt_get_GCEnum_singleton, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("The gc-singleton which can also be found under :deemon:gc") }, /* DeeGCEnumTracked_Singleton */
    { "GCEnum", (DeeObject *)&librt_get_GCEnum, MODSYM_FREADONLY|MODSYM_FPROPERTY|MODSYM_FCONSTEXPR,
      DOC("The result of ${type(gc from deemon)}") }, /* GCEnum_Type */

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
    { "Function", (DeeObject *)&DeeFunction_Type, MODSYM_FREADONLY },
    { "Type", (DeeObject *)&DeeType_Type, MODSYM_FREADONLY },
    { "Object", (DeeObject *)&DeeObject_Type, MODSYM_FREADONLY },
    { "File", (DeeObject *)&DeeFile_Type, MODSYM_FREADONLY },                       /* (intended) base class for all file types (is to `FileType' what `Object' is to `Type'). */
    { "FileBuffer", (DeeObject *)&DeeFileBuffer_Type, MODSYM_FREADONLY },           /* `file.buffer' */
    { "SystemFile", (DeeObject *)&DeeSystemFile_Type, MODSYM_FREADONLY },           /* Base class for file types that are managed by the system. */
    { "FSFile", (DeeObject *)&DeeFSFile_Type, MODSYM_FREADONLY },                   /* Derived from `SystemFile': A system file that has been opened via the file system. */
    { "NoneType", (DeeObject *)&DeeNone_Type, MODSYM_FREADONLY },                   /* `type(none)' */
    { "MemoryFile", (DeeObject *)&DeeMemoryFile_Type, MODSYM_FREADONLY,             /* An internal file type for streaming from read-only raw memory. */
      DOC("A special file type that may be used by the deemon runtime to temporarily "
          "allow user-code access to raw memory regions via the file interface, rather "
          "than the bytes interface. Note however that this type of file cannot be "
          "constructed from user-code such that it would reference data, and that memory "
          "files impose special access restrictions to prevent user-code from maintaining "
          "access to wrapped memory once the file's creator destroys it") },
    { "FileReader", (DeeObject *)&DeeFileReader_Type, MODSYM_FREADONLY },           /* `file.reader' */
    { "FileWriter", (DeeObject *)&DeeFileWriter_Type, MODSYM_FREADONLY },           /* `file.writer' */

    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols
};

DECL_END

#endif /* !GUARD_DEX_RT_LIBRT_C */
