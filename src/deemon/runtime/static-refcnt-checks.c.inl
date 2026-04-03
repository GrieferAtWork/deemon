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
#ifndef GUARD_DEEMON_RUNTIME_STATIC_REFCNT_CHECKS_C_INL
#define GUARD_DEEMON_RUNTIME_STATIC_REFCNT_CHECKS_C_INL 1

#include <deemon/api.h>

#include "static-refcnt-checks.h"

#ifdef HAVE_DEBUG_STATIC_REFS
#include <deemon/attribute.h>          /* DeeAttribute_Type, DeeEnumAttrIterator_Type, DeeEnumAttr_Type */
#include <deemon/bool.h>               /* DeeBool_Type */
#include <deemon/bytes.h>              /* DeeBytes_Empty, DeeBytes_Type */
#include <deemon/cached-dict.h>        /* DeeCachedDict_Type */
#include <deemon/callable.h>           /* DeeCallable_Type */
#include <deemon/cell.h>               /* DeeCell_Type */
#include <deemon/class.h>              /* DeeClassDescriptor_Type, DeeInstanceMember_Type */
#include <deemon/code.h>               /* DeeCode_Empty, DeeCode_Type, DeeDDI_Type, DeeFunction_Type, DeeYieldFunctionIterator_Type, DeeYieldFunction_Type */
#include <deemon/compiler/ast.h>       /* DeeAst_Type */
#include <deemon/compiler/compiler.h>  /* DeeCompiler* */
#include <deemon/compiler/interface.h> /* DeeCompiler* */
#include <deemon/compiler/symbol.h>    /* DeeBaseScope_Type, DeeClassScope_Type, DeeRootScope_Type, DeeScope_Type */
#include <deemon/dex.h>                /* DEXSYM_READONLY, DEX_MEMBER_F, DeeDex_Type */
#include <deemon/dict.h>               /* DeeDict_Type */
#include <deemon/error_types.h>        /* DeeError_*_instance */
#include <deemon/file.h>               /* DeeFSFile_Type, DeeFileBuffer_Type, DeeFileType_Type, DeeFile_Type, DeeSystemFile_Type */
#include <deemon/filetypes.h>          /* DeeFilePrinter_Type, DeeFileReader_Type, DeeFileWriter_Type, DeeMemoryFile_Type */
#include <deemon/float.h>              /* DeeFloat_Type */
#include <deemon/format.h>             /* PRFdPTR, PRFxPTR */
#include <deemon/gc.h>                 /* DeeGCEnumTracked_Singleton */
#include <deemon/hashset.h>            /* DeeHashSet_Type */
#include <deemon/instancemethod.h>     /* DeeInstanceMethod_Type */
#include <deemon/int.h>                /* DeeInt_MinusOne_Zero_One, DeeInt_Type */
#include <deemon/kwds.h>               /* DeeBlackListKw_Type, DeeBlackListKwds_Type, DeeKwdsMapping_Type, DeeKwds_Type */
#include <deemon/list.h>               /* DeeList_Type */
#include <deemon/map.h>                /* DeeMap_EmptyInstance, DeeMap_Type, DeeSharedMap_Type */
#include <deemon/mapfile.h>            /* DeeMapFile_Type */
#include <deemon/module.h>             /* DeeInteractiveModule_Type, DeeModule* */
#include <deemon/none.h>               /* DeeNone_Singleton, DeeNone_Type */
#include <deemon/numeric.h>            /* DeeNumeric_Type */
#include <deemon/object.h>             /* DeeObject_Type */
#include <deemon/objmethod.h>          /* DeeCMethod*_*, DeeClsMember_Type, DeeClsMethod_Type, DeeClsProperty_Type, DeeKwCMethod_Type, DeeKwClsMethod_Type, DeeKwObjMethod_Type, DeeObjMethod_Type */
#include <deemon/pair.h>               /* CONFIG_ENABLE_SEQ_ONE_TYPE, CONFIG_ENABLE_SEQ_PAIR_TYPE, DeeSeqOne_Type, DeeSeqPair_Type */
#include <deemon/property.h>           /* DeeProperty_Type */
#include <deemon/rodict.h>             /* DeeRoDict_EmptyInstance, DeeRoDict_Type */
#include <deemon/roset.h>              /* DeeRoSet_Type */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeqSome_Type, DeeSeq_EmptyInstance, DeeSeq_Type, DeeSharedVector_Type */
#include <deemon/set.h>                /* DeeSet_* */
#include <deemon/string.h>             /* DeeString_Empty, DeeString_Type */
#include <deemon/super.h>              /* DeeSuper_Type */
#include <deemon/thread.h>             /* DeeThread_Type */
#include <deemon/traceback.h>          /* DeeFrame_Type, DeeTraceback_Empty, DeeTraceback_Type */
#include <deemon/tuple.h>              /* DeeNullableTuple_Empty, DeeNullableTuple_Type, DeeTuple_Empty, DeeTuple_Type */
#include <deemon/type.h>               /* DeeType_* */
#include <deemon/types.h>              /* DeeObject, DeeTypeObject, Dee_AsObject, Dee_STATIC_REFCOUNT_INIT, Dee_TYPE */
#include <deemon/weakref.h>            /* DeeWeakRef_Type */

#include "../execute/function-wrappers.h"
#include "../execute/module-globals.h"
#include "../objects/bytes.h"
#include "../objects/callable.h"
#include "../objects/class_desc.h"
#include "../objects/dict.h"
#include "../objects/hashset.h"
#include "../objects/iterator.h"
#include "../objects/list.h"
#include "../objects/objmethod.h"
#include "../objects/rodict.h"
#include "../objects/roset.h"
#include "../objects/seq/byattr.h"
#include "../objects/seq/cached-seq.h"
#include "../objects/seq/combinations.h"
#include "../objects/seq/concat.h"
#include "../objects/seq/default-enumerate.h"
#include "../objects/seq/default-iterators.h"
#include "../objects/seq/default-map-proxy.h"
#include "../objects/seq/default-maps.h"
#include "../objects/seq/default-reversed.h"
#include "../objects/seq/default-sequences.h"
#include "../objects/seq/default-sets.h"
#include "../objects/seq/each.h"
#include "../objects/seq/enumerate-cb.h"
#include "../objects/seq/filter.h"
#include "../objects/seq/flat.h"
#include "../objects/seq/hashfilter.h"
#include "../objects/seq/map-fromattr.h"
#include "../objects/seq/map-fromkeys.h"
#include "../objects/seq/mapped.h"
#include "../objects/seq/one.h"
#include "../objects/seq/pair.h"
#include "../objects/seq/range.h"
#include "../objects/seq/removeif-cb.h"
#include "../objects/seq/repeat.h"
#include "../objects/seq/segments.h"
#include "../objects/seq/simpleproxy.h"
#include "../objects/seq/smap.h"
#include "../objects/seq/svec.h"
#include "../objects/seq/typemro.h"
#include "../objects/seq/unique-iterator.h"
#include "../objects/string.h"
#include "../objects/traceback.h"
#include "../objects/tuple.h"
#include "../objects/unicode/bytes_finder.h"
#include "../objects/unicode/bytes_segments.h"
#include "../objects/unicode/bytes_split.h"
#include "../objects/unicode/cscanf.h"
#include "../objects/unicode/finder.h"
#include "../objects/unicode/ordinals.h"
#include "../objects/unicode/regroups.h"
#include "../objects/unicode/reproxy.h"
#include "../objects/unicode/segments.h"
#include "../objects/unicode/split.h"
#include "gc-inspect.h"
#include "kwds-wrappers.h"
#include "kwds.h"
#include "operator_info.h"

#include <stdint.h> /* intptr_t */

#undef lengthof
#define lengthof COMPILER_LENOF

DECL_BEGIN

PRIVATE NONNULL((1)) void DCALL
sr_check_obj(DeeObject *__restrict self) {
	if (self->ob_refcnt != Dee_STATIC_REFCOUNT_INIT) {
		Dee_DPRINTF("WARNING: Bad static refcnt: %p: ", self);
		if (DeeType_Check(self)) {
			Dee_DPRINTF("<type %s>", DeeType_GetName((DeeTypeObject *)self));
		} else {
			Dee_DPRINTF("<instance of %s>", DeeType_GetName(Dee_TYPE(self)));
		}
		Dee_DPRINTF(" [ob_refcnt=%" PRFxPTR ",delta:%" PRFdPTR "]\n",
		            self->ob_refcnt, (intptr_t)(self->ob_refcnt - Dee_STATIC_REFCOUNT_INIT));
	}
}

PRIVATE DeeObject *tpconst static_objects[] = {
#define O(x) Dee_AsObject(&x)

	/* Internal types used to drive sequence proxies */
	O(SeqCombinations_Type),
	O(SeqCombinationsIterator_Type),
	O(SeqRepeatCombinations_Type),
	O(SeqRepeatCombinationsIterator_Type),
	O(SeqPermutations_Type),
	O(SeqPermutationsIterator_Type),
	O(SeqCombinationsView_Type),

	O(SeqSegments_Type),
	O(SeqSegmentsIterator_Type),
	O(SeqConcat_Type),
	O(SeqConcatIterator_Type),
	O(SeqFilter_Type),
	O(SeqFilterAsUnbound_Type),
	O(SeqFilterIterator_Type),
	O(SeqHashFilter_Type),
	O(SeqHashFilterIterator_Type),
	O(SeqMapped_Type),
	O(SeqMappedIterator_Type),
	O(SeqRange_Type),
	O(SeqRangeIterator_Type),
	O(SeqIntRange_Type),
	O(SeqIntRangeIterator_Type),
	O(SeqRepeat_Type),
	O(SeqRepeatIterator_Type),
	O(SeqRepeatItem_Type),
	O(SeqRepeatItemIterator_Type),
	O(SeqIds_Type),
	O(SeqIdsIterator_Type),
	O(SeqTypes_Type),
	O(SeqTypesIterator_Type),
	O(SeqClasses_Type),
	O(SeqClassesIterator_Type),
	O(SeqFlat_Type),
	O(SeqFlatIterator_Type),

	/* Seq-each wrapper types. */
	O(SeqEach_Type),
	O(SeqEachOperator_Type),
	O(SeqEachOperatorIterator_Type),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	O(SeqEachGetAttr_Type),
	O(SeqEachGetAttrIterator_Type),
	O(SeqEachCallAttr_Type),
	O(SeqEachCallAttrIterator_Type),
	O(SeqEachCallAttrKw_Type),
	O(SeqEachCallAttrKwIterator_Type),
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */

	/* Seq-some wrapper types. */
	O(DeeSeqSome_Type),
	O(SeqSomeOperator_Type),
#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
	O(SeqSomeGetAttr_Type),
	O(SeqSomeCallAttr_Type),
	O(SeqSomeCallAttrKw_Type),
#endif /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */

	/* Default enumeration types */
	O(DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast),
	O(DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast),
	O(DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index),
	O(DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index),
	O(DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index),
	O(DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index),
	O(DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem),
	O(DefaultEnumeration__with__seq_operator_getitem_index),
	O(DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index),
	O(DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem),
	O(DefaultEnumeration__with__seq_operator_getitem),
	O(DefaultEnumerationWithFilter__with__seq_operator_getitem),
	O(DefaultEnumeration__with__seq_operator_iter__and__counter),
	O(DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter),
	O(DefaultEnumeration__with__seq_enumerate),
	O(DefaultEnumerationWithIntFilter__with__seq_enumerate_index),
	O(DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack),
	O(DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem),
	O(DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem),
	O(DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem),
	O(DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem),
	O(DefaultEnumeration__with__map_enumerate),
	O(DefaultEnumerationWithFilter__with__map_enumerate_range),

	/* Default sequence types */
	O(DefaultSequence_WithSizeAndGetItemIndex_Type),
	O(DefaultSequence_WithSizeAndGetItemIndexFast_Type),
	O(DefaultSequence_WithSizeAndTryGetItemIndex_Type),
	O(DefaultSequence_WithSizeObAndGetItem_Type),
	O(DefaultSequence_WithIter_Type),
	O(DefaultSequence_WithIterAndLimit_Type),

	/* Special sequence types */
#ifdef CONFIG_ENABLE_SEQ_ONE_TYPE
	O(DeeSeqOne_Type),
	O(SeqOneIterator_Type),
#endif /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	O(DeeSeqPair_Type),
	O(SeqPairIterator_Type),
#endif /* CONFIG_ENABLE_SEQ_PAIR_TYPE */

	/* Misc. helper types for sequences */
	O(SeqEnumerateWrapper_Type),
	O(SeqEnumerateIndexWrapper_Type),
	O(SeqRemoveWithRemoveIfPredicate_Type),
	O(SeqRemoveWithRemoveIfPredicateWithKey_Type),
	O(SeqRemoveIfWithRemoveAllItem_Type),
	O(SeqRemoveIfWithRemoveAllKey_Type),
	O(SeqRemoveIfWithRemoveAllItem_DummyInstance),

	/* Default iterator types */
	O(DefaultIterator_WithGetItemIndex_Type),
	O(DefaultIterator_WithGetItemIndexPair_Type),
	O(DefaultIterator_WithSizeAndGetItemIndex_Type),
	O(DefaultIterator_WithSizeAndGetItemIndexPair_Type),
	O(DefaultIterator_WithSizeAndGetItemIndexFast_Type),
	O(DefaultIterator_WithSizeAndGetItemIndexFastPair_Type),
	O(DefaultIterator_WithSizeAndTryGetItemIndex_Type),
	O(DefaultIterator_WithSizeAndTryGetItemIndexPair_Type),
	O(DefaultIterator_WithGetItem_Type),
	O(DefaultIterator_WithGetItemPair_Type),
	O(DefaultIterator_WithSizeObAndGetItem_Type),
	O(DefaultIterator_WithSizeObAndGetItemPair_Type),
	O(DefaultIterator_WithNextAndLimit_Type),
	O(DefaultIterator_WithIterKeysAndGetItemMap_Type),
	O(DefaultIterator_WithIterKeysAndTryGetItemMap_Type),
	O(DefaultIterator_WithNextAndCounterPair_Type),
	O(DefaultIterator_WithNextAndCounterAndLimitPair_Type),
	O(DefaultIterator_WithNextAndUnpackFilter_Type),
	O(DefaultIterator_WithNextKey),
	O(DefaultIterator_WithNextValue),

	/* Default types for `Sequence.reversed()' */
	O(DefaultReversed_WithGetItemIndex_Type),
	O(DefaultReversed_WithGetItemIndexFast_Type),
	O(DefaultReversed_WithTryGetItemIndex_Type),

	/* Default types for `Sequence.distinct()' */
	O(DistinctIterator_Type),
	O(DistinctIteratorWithKey_Type),
	O(DistinctSetWithKey_Type),
	O(DistinctMappingIterator_Type),

	/* Default sequence cache types */
	O(CachedSeq_WithIter_Type),
	O(CachedSeq_WithIter_Iterator_Type),

	/* Internal types used to drive set proxies */
	O(SetInversion_Type),
	O(SetUnion_Type),
	O(SetUnionIterator_Type),
	O(SetSymmetricDifference_Type),
	O(SetSymmetricDifferenceIterator_Type),
	O(SetIntersection_Type),
	O(SetIntersectionIterator_Type),
	O(SetDifference_Type),
	O(SetDifferenceIterator_Type),

	/* Internal types used to drive map proxies */
	O(MapUnion_Type),
	O(MapUnionIterator_Type),
	O(MapSymmetricDifference_Type),
	O(MapSymmetricDifferenceIterator_Type),
	O(MapIntersection_Type),
	O(MapIntersectionIterator_Type),
	O(MapDifference_Type),
	O(MapDifferenceIterator_Type),

	/* Internal types used to drive mapping proxies */
	O(MapHashFilter_Type),
	O(MapHashFilterIterator_Type),
	O(MapByAttr_Type),
	O(DefaultSequence_MapKeys_Type),
	O(DefaultSequence_MapValues_Type),

	/* Internal types used to implement "Mapping.fromkeys" */
	O(MapFromKeysAndValue_Type),
	O(MapFromKeysAndCallback_Type),
	O(MapFromKeysAndValueIterator_Type),
	O(MapFromKeysAndCallbackIterator_Type),

	/* Internal types used to implement "Mapping.fromattr" */
	O(MapFromAttr_Type),
	O(MapFromAttrKeysIterator_Type),

	/* The special "nullable" tuple sequence type. */
	O(DeeNullableTuple_Type),

	/* Internal types used for safe & fast passing of temporary sequences */
	O(DeeSharedVector_Type),
	O(DeeSharedMap_Type),
	O(SharedMapIterator_Type),
	O(RefVector_Type),

	/* Internal types used to drive sequence operations on `Bytes' */
	O(BytesFind_Type),
	O(BytesFindIterator_Type),
	O(BytesCaseFind_Type),
	O(BytesCaseFindIterator_Type),
	O(BytesSegments_Type),
	O(BytesSegmentsIterator_Type),
	O(BytesSplit_Type),
	O(BytesSplitIterator_Type),
	O(BytesCaseSplit_Type),
	O(BytesCaseSplitIterator_Type),
	O(BytesLineSplit_Type),
	O(BytesLineSplitIterator_Type),

	/* Internal types used to drive sequence operations on `string' */
	O(StringScan_Type),
	O(StringScanIterator_Type),
	O(StringFind_Type),
	O(StringFindIterator_Type),
	O(StringCaseFind_Type),
	O(StringCaseFindIterator_Type),
	O(StringOrdinals_Type),
	O(StringSegments_Type),
	O(StringSegmentsIterator_Type),
	O(StringSplit_Type),
	O(StringSplitIterator_Type),
	O(StringCaseSplit_Type),
	O(StringCaseSplitIterator_Type),
	O(StringLineSplit_Type),
	O(StringLineSplitIterator_Type),

	/* Internal types used to drive sequence operations with regular expressions */
	O(ReFindAll_Type),
	O(ReFindAllIterator_Type),
	O(RegFindAll_Type),
	O(RegFindAllIterator_Type),
	O(ReLocateAll_Type),
	O(ReLocateAllIterator_Type),
	O(RegLocateAll_Type),
	O(RegLocateAllIterator_Type),
	O(ReSplit_Type),
	O(ReSplitIterator_Type),
	O(ReGroups_Type),
	O(ReSubStrings_Type),
	O(ReSubBytes_Type),
	O(ReBytesFindAll_Type),
	O(ReBytesFindAllIterator_Type),
	O(RegBytesFindAll_Type),
	O(RegBytesFindAllIterator_Type),
	O(ReBytesLocateAll_Type),
	O(ReBytesLocateAllIterator_Type),
	O(RegBytesLocateAll_Type),
	O(RegBytesLocateAllIterator_Type),
	O(ReBytesSplit_Type),
	O(ReBytesSplitIterator_Type),

	/* Internal types used to drive module symbol table inspection */
	O(ModuleExports_Type),
	O(ModuleExportsIterator_Type),
	O(ModuleExportsKeysIterator_Type),
	O(ModuleGlobals_Type),
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	O(ModuleLibNames_Type),
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Internal types used to drive user-defined classes */
	O(ClassOperatorTable_Type),
	O(ClassOperatorTableIterator_Type),
	O(ClassAttribute_Type),
	O(ClassAttributeTable_Type),
	O(ClassAttributeTableIterator_Type),
	O(ObjectTable_Type),
	O(TypeMRO_Type),
	O(TypeMROIterator_Type),
	O(TypeBases_Type),
	O(TypeBasesIterator_Type),

	/* Internal types used to drive natively defined types */
	O(TypeOperators_Type),
	O(TypeOperatorsIterator_Type),

	/* Internal types used to drive the garbage collector */
	O(GCCollection_Type),
	O(GCCollectionIterator_Type),

	/* Internal types used to drive variable keyword arguments */
	O(DeeBlackListKwds_Type),
	O(DeeBlackListKwdsIterator_Type),
	O(DeeBlackListKw_Type),
	O(DeeBlackListKwIterator_Type),
	O(DeeCachedDict_Type),

	/* Internal types used to drive keyword argument support */
	O(DocKwds_Type),
	O(DocKwdsIterator_Type),

	/* Special types exposed by the C API, but not normally visible to user-code. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	O(DeeModuleDee_Type),
	O(DeeModuleDir_Type),
#ifndef CONFIG_NO_DEX
	O(DeeModuleDex_Type),
#endif /* !CONFIG_NO_DEX */
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	O(DeeInteractiveModule_Type),
#ifndef CONFIG_NO_DEX
	O(DeeDex_Type),
#endif /* !CONFIG_NO_DEX */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	O(DeeCompiler_Type),

	/* All of the different compiler wrapper types, as well as the internal types for Ast and the different Scopes: */
	O(DeeCompilerObjItem_Type),
	O(DeeCompilerWrapper_Type),
	O(DeeCompilerLexer_Type),
	O(DeeCompilerLexerExtensions_Type),
	O(DeeCompilerLexerWarnings_Type),
	O(DeeCompilerLexerIfdef_Type),
	O(DeeCompilerLexerToken_Type),
	O(DeeCompilerParser_Type),
	O(DeeCompilerAst_Type),
	O(DeeCompilerScope_Type),
	O(DeeCompilerBaseScope_Type),
	O(DeeCompilerRootScope_Type),
	O(DeeAst_Type),
	O(DeeScope_Type),
	O(DeeClassScope_Type),
	O(DeeBaseScope_Type),
	O(DeeRootScope_Type),

	O(DeeClassDescriptor_Type),
	O(DeeInstanceMember_Type),
	O(DeeCMethod_Type),
	O(DeeCMethod0_Type),
	O(DeeCMethod1_Type),
	O(DeeKwCMethod_Type),
	O(DeeObjMethod_Type),
	O(DeeKwObjMethod_Type),
	O(DeeClsMethod_Type),
	O(DeeKwClsMethod_Type),
	O(DeeClsProperty_Type),
	O(DeeClsMember_Type),
	O(DeeFileType_Type),
	O(DeeYieldFunction_Type),
	O(DeeYieldFunctionIterator_Type),
	O(DeeRoDict_Type),
	O(RoDictIterator_Type),
	O(DeeRoSet_Type),
	O(RoSetIterator_Type),
	O(DeeKwds_Type),
	O(DeeKwdsIterator_Type),
	O(DeeKwdsMapping_Type),
	O(DeeKwdsMappingIterator_Type),
	O(DeeDDI_Type),
	O(DeeError_NoMemory_instance),
	O(DeeError_StopIteration_instance),
	O(DeeError_Interrupt_instance),
	
	/* Types used to drive general purpose iterator support */
	O(IteratorPending_Type),
	O(IteratorFuture_Type),

	/* Internal iterator types used to drive builtin sequence objects */
	O(StringIterator_Type),
	O(BytesIterator_Type),
	O(DeeListIterator_Type),
	O(DeeTupleIterator_Type),
	O(HashSetIterator_Type),
	O(DeeTracebackIterator_Type),
	O(DictIterator_Type),

	/* Stuff related to composition of callable objects */
	O(FunctionComposition_Type),
	O(FunctionComposition_Identity),
	
	/* Special instances of non-singleton objects */
	O(DeeSeq_EmptyInstance),
	O(DeeSet_EmptyInstance),
	O(DeeSet_UniversalInstance),
	O(DeeMap_EmptyInstance),
	O(DeeRoDict_EmptyInstance),
	O(DeeTuple_Empty),
	O(DeeString_Empty),
	O(DeeBytes_Empty),
	O(DeeInt_MinusOne_Zero_One[0]),
	O(DeeInt_MinusOne_Zero_One[1]),
	O(DeeInt_MinusOne_Zero_One[2]),
	O(DeeNullableTuple_Empty),
	O(DeeCode_Empty),
	O(DeeGCEnumTracked_Singleton),
	O(GCEnum_Type),
	O(GCIter_Type),
	O(DeeTraceback_Empty),
	O(DeeModule_Deemon),
	O(DeeModule_Empty),
	
	/* Re-exports of standard types also exported from `deemon' */
	O(DeeInt_Type),
	O(DeeBool_Type),
	O(DeeFloat_Type),
	O(DeeSeq_Type),
	O(DeeIterator_Type),
	O(DeeString_Type),
	O(DeeBytes_Type),
	O(DeeList_Type),
	O(DeeTuple_Type),
	O(DeeHashSet_Type),
	O(DeeDict_Type),
	O(DeeTraceback_Type),
	O(DeeFrame_Type),
	O(DeeModule_Type),
	O(DeeSet_Type),
	O(DeeMap_Type),
	O(DeeCode_Type),
	O(DeeFunction_Type),
	O(DeeType_Type),
	O(DeeObject_Type),
	O(DeeCallable_Type),
	O(DeeNumeric_Type),
	O(DeeInstanceMethod_Type),
	O(DeeProperty_Type),
	O(DeeSuper_Type),
	O(DeeThread_Type),
	O(DeeWeakRef_Type),
	O(DeeCell_Type),
	O(DeeFile_Type.ft_base),
	O(DeeFileBuffer_Type.ft_base),
	O(DeeSystemFile_Type.ft_base),
	O(DeeFSFile_Type.ft_base),
	O(DeeMapFile_Type),
	O(DeeNone_Type),
	O(DeeNone_Singleton),
	O(DeeMemoryFile_Type.ft_base),
	O(DeeFileReader_Type.ft_base),
	O(DeeFileWriter_Type.ft_base),
	O(DeeFilePrinter_Type.ft_base),
	O(DeeAttribute_Type),
	O(DeeEnumAttr_Type),
	O(DeeEnumAttrIterator_Type),

	/* Function wrapper types */
	O(FunctionStatics_Type),
	O(FunctionStatics_Type),
	O(FunctionSymbolsByName_Type),
	O(FunctionSymbolsByName_Type),
	O(FunctionSymbolsByNameKeysIterator_Type),
	O(YieldFunctionSymbolsByName_Type),
	O(YieldFunctionSymbolsByName_Type),
	O(YieldFunctionSymbolsByNameKeysIterator_Type),
	O(FrameArgs_Type),
	O(FrameLocals_Type),
	O(FrameStack_Type),
	O(FrameSymbolsByName_Type),
	O(FrameSymbolsByName_Type),
	O(FrameSymbolsByNameKeysIterator_Type),

	/* Builtin functions and special types */
	O(DeeBuiltin_HasAttr),
	O(DeeBuiltin_HasItem),
	O(DeeBuiltin_BoundAttr),
	O(DeeBuiltin_BoundItem),
	O(DeeBuiltin_Compare),
	O(DeeBuiltin_Equals),
	O(DeeBuiltin_Hash),
	O(DeeBuiltin_Exec),
	O(DeeBuiltin_Import),
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	O(DeeBuiltin_ImportType),
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
};

/* Dump unexpected changes to reference counters of static objects. */
INTERN void DCALL DeeDbg_DumpStaticRefChanges(void) {
	size_t i;
	for (i = 0; i < lengthof(static_objects); ++i)
		sr_check_obj(static_objects[i]);
}

DECL_END
#endif /* HAVE_DEBUG_STATIC_REFS */

#endif /* !GUARD_DEEMON_RUNTIME_STATIC_REFCNT_CHECKS_C_INL */
