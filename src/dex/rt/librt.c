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
#ifndef GUARD_DEX_RT_LIBRT_C
#define GUARD_DEX_RT_LIBRT_C 1
#define DEE_SOURCE

#include "librt.h"

#include <deemon/compiler/compiler.h>

#include <deemon/abi/ctypes.h>
#include <deemon/alloc.h>
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
#include <deemon/method-hints.h>
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
#include <deemon/util/lock.h>
#include <deemon/util/rlock.h>
#include <deemon/weakref.h>

#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

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


/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("enumerate");
print define_Dee_HashStr("itervalues");
print define_Dee_HashStr("reversed");
print define_Dee_HashStr("Attribute");
print define_Dee_HashStr("AttributeTable");
print define_Dee_HashStr("ObjectTable");
print define_Dee_HashStr("OperatorTable");
print define_Dee_HashStr("KeysIterator");
print define_Dee_HashStr("__args__");
print define_Dee_HashStr("__bases__");
print define_Dee_HashStr("__exports__");
print define_Dee_HashStr("__globals__");
print define_Dee_HashStr("__kwds__");
print define_Dee_HashStr("__locals__");
print define_Dee_HashStr("__mro__");
print define_Dee_HashStr("__operators__");
print define_Dee_HashStr("__stack__");
print define_Dee_HashStr("__statics__");
print define_Dee_HashStr("__symbols__");
print define_Dee_HashStr("byattr");
print define_Dee_HashStr("byhash");
print define_Dee_HashStr("casefindall");
print define_Dee_HashStr("casesplit");
print define_Dee_HashStr("classes");
print define_Dee_HashStr("flatten");
print define_Dee_HashStr("combinations");
print define_Dee_HashStr("each");
print define_Dee_HashStr("filter");
print define_Dee_HashStr("ubfilter");
print define_Dee_HashStr("findall");
print define_Dee_HashStr("future");
print define_Dee_HashStr("ids");
print define_Dee_HashStr("ordinals");
print define_Dee_HashStr("pending");
print define_Dee_HashStr("permutations");
print define_Dee_HashStr("reachable");
print define_Dee_HashStr("refindall");
print define_Dee_HashStr("regfindall");
print define_Dee_HashStr("reglocateall");
print define_Dee_HashStr("regmatch");
print define_Dee_HashStr("relocateall");
print define_Dee_HashStr("repeat");
print define_Dee_HashStr("repeatcombinations");
print define_Dee_HashStr("repeatseq");
print define_Dee_HashStr("rescanf");
print define_Dee_HashStr("resplit");
print define_Dee_HashStr("scanf");
print define_Dee_HashStr("segments");
print define_Dee_HashStr("seq");
print define_Dee_HashStr("split");
print define_Dee_HashStr("splitlines");
print define_Dee_HashStr("map");
print define_Dee_HashStr("types");
print define_Dee_HashStr("distinct");
print define_Dee_HashStr("fromkeys");
print define_Dee_HashStr("fromattr");
print define_Dee_HashStr("__SeqWithIter__");
print define_Dee_HashStr("__IterWithForeach__");
print define_Dee_HashStr("__IterWithForeachPair__");
print define_Dee_HashStr("__IterWithEnumerateMap__");
print define_Dee_HashStr("__IterWithEnumerateIndexSeq__");
print define_Dee_HashStr("__IterWithEnumerateSeq__");
print define_Dee_HashStr("__IterWithEnumerateIndexMap__");
]]]*/
#define Dee_HashStr__enumerate _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3)
#define Dee_HashStr__itervalues _Dee_HashSelectC(0xcb00bab3, 0xe9a89082a994930a)
#define Dee_HashStr__reversed _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f)
#define Dee_HashStr__Attribute _Dee_HashSelectC(0xa08b731, 0x2e763a5308721ff3)
#define Dee_HashStr__AttributeTable _Dee_HashSelectC(0xb3fdb6eb, 0x2abf3f78af71dee0)
#define Dee_HashStr__ObjectTable _Dee_HashSelectC(0xc5d943d2, 0x8cda148232a1cdb2)
#define Dee_HashStr__OperatorTable _Dee_HashSelectC(0xee6c4bef, 0x7987fd5ae34b3d62)
#define Dee_HashStr__KeysIterator _Dee_HashSelectC(0x4414d7ed, 0xb21fd2b052003297)
#define Dee_HashStr____args__ _Dee_HashSelectC(0x938e1f4c, 0x78969e2a67f8471d)
#define Dee_HashStr____bases__ _Dee_HashSelectC(0xff4ac0d2, 0x56bdc053fa64e4c9)
#define Dee_HashStr____exports__ _Dee_HashSelectC(0x1d7df2db, 0x304ed10433cd0d26)
#define Dee_HashStr____globals__ _Dee_HashSelectC(0x6bd07fac, 0x3114153efd2ae18d)
#define Dee_HashStr____kwds__ _Dee_HashSelectC(0xd3926a14, 0xa90825b224a7262b)
#define Dee_HashStr____locals__ _Dee_HashSelectC(0x9afc687c, 0x5b2c71a072cbe841)
#define Dee_HashStr____mro__ _Dee_HashSelectC(0x4b5e22f6, 0x8dbbdb5c2f99ff7a)
#define Dee_HashStr____operators__ _Dee_HashSelectC(0xed78c2be, 0xfc4c23ee6412f79c)
#define Dee_HashStr____stack__ _Dee_HashSelectC(0xe739f041, 0x170e744df540dd5c)
#define Dee_HashStr____statics__ _Dee_HashSelectC(0x4147b465, 0x61d855a2a9645021)
#define Dee_HashStr____symbols__ _Dee_HashSelectC(0x81659df, 0xf4073184aaae4c4c)
#define Dee_HashStr__byattr _Dee_HashSelectC(0x7f16cf28, 0x58b9e1994d29c7ca)
#define Dee_HashStr__byhash _Dee_HashSelectC(0x7b5277ce, 0x773c8074445a28d9)
#define Dee_HashStr__casefindall _Dee_HashSelectC(0x68f0403d, 0x2aa76ae718f34e43)
#define Dee_HashStr__casesplit _Dee_HashSelectC(0x8d4e9c87, 0x69205bfad60e0e61)
#define Dee_HashStr__classes _Dee_HashSelectC(0x75e5899b, 0xc75d2d970415e4a0)
#define Dee_HashStr__flatten _Dee_HashSelectC(0x6790c1a3, 0x3eb59c1a2ed05257)
#define Dee_HashStr__combinations _Dee_HashSelectC(0x184d9b51, 0x3e5802b7656c4900)
#define Dee_HashStr__each _Dee_HashSelectC(0x9de8b13d, 0x374e052f37a5e158)
#define Dee_HashStr__filter _Dee_HashSelectC(0x3110088a, 0x32e04884df75b1c1)
#define Dee_HashStr__ubfilter _Dee_HashSelectC(0x9f55cd0c, 0xa457507f0faa4d80)
#define Dee_HashStr__findall _Dee_HashSelectC(0xa7064666, 0x73bffde4f31b16e5)
#define Dee_HashStr__future _Dee_HashSelectC(0x5ca3159c, 0x8ab2926ab5959525)
#define Dee_HashStr__ids _Dee_HashSelectC(0x3173a48f, 0x7cd9fae6cf17bb9f)
#define Dee_HashStr__ordinals _Dee_HashSelectC(0x4237d4c5, 0x5459ba3a055d9b9a)
#define Dee_HashStr__pending _Dee_HashSelectC(0xa318502a, 0x9f3f699bf5a1e785)
#define Dee_HashStr__permutations _Dee_HashSelectC(0x923fbd44, 0x498779abec4910f6)
#define Dee_HashStr__reachable _Dee_HashSelectC(0x54a10efd, 0x6f461510341a0f20)
#define Dee_HashStr__refindall _Dee_HashSelectC(0x821c12cd, 0x6e1b190da9b3fef9)
#define Dee_HashStr__regfindall _Dee_HashSelectC(0x48a7b09d, 0x34acc7f51335ea55)
#define Dee_HashStr__reglocateall _Dee_HashSelectC(0x6848908c, 0x45c3cc62f38d8d0d)
#define Dee_HashStr__regmatch _Dee_HashSelectC(0x29e07576, 0xf95c79aef53a2c4f)
#define Dee_HashStr__relocateall _Dee_HashSelectC(0xe2b79628, 0x188da1195d4b6ae9)
#define Dee_HashStr__repeat _Dee_HashSelectC(0x26374320, 0x5a5a8c53402eacfe)
#define Dee_HashStr__repeatcombinations _Dee_HashSelectC(0xa3bc4ae1, 0x7ef1d21507ad27f5)
#define Dee_HashStr__repeatseq _Dee_HashSelectC(0xb271fb0, 0x2045cc35458ec0da)
#define Dee_HashStr__rescanf _Dee_HashSelectC(0x18d32308, 0xbf03693953b5fdbb)
#define Dee_HashStr__resplit _Dee_HashSelectC(0x8147c0a0, 0xb0ae6734658c0c4f)
#define Dee_HashStr__scanf _Dee_HashSelectC(0x1369b612, 0x86d0b7e2f8b4b546)
#define Dee_HashStr__segments _Dee_HashSelectC(0x20acb0bd, 0x8554d160c212a46a)
#define Dee_HashStr__seq _Dee_HashSelectC(0x232af2b7, 0x80a0b0950a5a5251)
#define Dee_HashStr__split _Dee_HashSelectC(0x916f9388, 0xca3aa391a92a6314)
#define Dee_HashStr__splitlines _Dee_HashSelectC(0xed695afd, 0xbac074bd124b8342)
#define Dee_HashStr__map _Dee_HashSelectC(0xeb1d32c8, 0x6ed228005fef6a3)
#define Dee_HashStr__types _Dee_HashSelectC(0x871b2836, 0xde8693a2d24930)
#define Dee_HashStr__distinct _Dee_HashSelectC(0xe1eb56d, 0x9c50bb058e287b02)
#define Dee_HashStr__fromkeys _Dee_HashSelectC(0xa8bdff1e, 0x34221e7828fcf94f)
#define Dee_HashStr__fromattr _Dee_HashSelectC(0x3db0dca7, 0x665ab1c0e444f870)
#define Dee_HashStr____SeqWithIter__ _Dee_HashSelectC(0x337ea2df, 0xb25329aebe2c9945)
#define Dee_HashStr____IterWithForeach__ _Dee_HashSelectC(0xb9e197d8, 0xa7821cd4b81f3978)
#define Dee_HashStr____IterWithForeachPair__ _Dee_HashSelectC(0xb64dbee5, 0xc91aa0d30329b6f3)
#define Dee_HashStr____IterWithEnumerateMap__ _Dee_HashSelectC(0x1cd8bec4, 0x15c6710443d657fc)
#define Dee_HashStr____IterWithEnumerateIndexSeq__ _Dee_HashSelectC(0x46c315bf, 0xfbcafdb8adece080)
#define Dee_HashStr____IterWithEnumerateSeq__ _Dee_HashSelectC(0x9f86b78c, 0x4fe8bf8aafe855be)
#define Dee_HashStr____IterWithEnumerateIndexMap__ _Dee_HashSelectC(0xf2e455a4, 0x4693cf704005698)
/*[[[end]]]*/

#define STR_AND_HASH(s) #s, Dee_HashStr__##s

/*[[[deemon (print_CMethod from rt.gen.unpack)("getstacklimit", "", ispure: true);]]]*/
#define librt_getstacklimit_params ""
PRIVATE WUNUSED DREF DeeObject *DCALL librt_getstacklimit_f_impl(void);
PRIVATE DEFINE_CMETHOD0(librt_getstacklimit, &librt_getstacklimit_f_impl, METHOD_FPURECALL);
PRIVATE WUNUSED DREF DeeObject *DCALL librt_getstacklimit_f_impl(void)
/*[[[end]]]*/
{
	uint16_t result = atomic_read(&DeeExec_StackLimit);
	return DeeInt_NewUInt16(result);
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("setstacklimit", """
	uint16_t newval=!0 = DEE_CONFIG_DEFAULT_STACK_LIMIT
""");]]]*/
#define librt_setstacklimit_params "newval=!0"
FORCELOCAL WUNUSED DREF DeeObject *DCALL librt_setstacklimit_f_impl(uint16_t newval);
PRIVATE WUNUSED DREF DeeObject *DCALL librt_setstacklimit_f(size_t argc, DeeObject *const *argv) {
	struct {
		uint16_t newval;
	} args;
	args.newval = DEE_CONFIG_DEFAULT_STACK_LIMIT;
	DeeArg_Unpack0Or1X(err, argc, argv, "setstacklimit", &args.newval, UNPu16, DeeObject_AsUInt16);
	return librt_setstacklimit_f_impl(args.newval);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(librt_setstacklimit, &librt_setstacklimit_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL librt_setstacklimit_f_impl(uint16_t newval)
/*[[[end]]]*/
{
	uint16_t result = atomic_xch(&DeeExec_StackLimit, newval);
	return DeeInt_NewUInt16(result);
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("getcalloptimizethreshold", "");]]]*/
#define librt_getcalloptimizethreshold_params ""
PRIVATE WUNUSED DREF DeeObject *DCALL librt_getcalloptimizethreshold_f_impl(void);
PRIVATE DEFINE_CMETHOD0(librt_getcalloptimizethreshold, &librt_getcalloptimizethreshold_f_impl, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL librt_getcalloptimizethreshold_f_impl(void)
/*[[[end]]]*/
{
	size_t result = DeeCode_GetOptimizeCallThreshold();
	return DeeInt_NewSize(result);
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("setcalloptimizethreshold", "size_t newThreshold");]]]*/
#define librt_setcalloptimizethreshold_params "newThreshold:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL librt_setcalloptimizethreshold_f_impl(size_t newThreshold);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL librt_setcalloptimizethreshold_f(DeeObject *__restrict arg0) {
	size_t newThreshold;
	if (DeeObject_AsSize(arg0, &newThreshold))
		goto err;
	return librt_setcalloptimizethreshold_f_impl(newThreshold);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(librt_setcalloptimizethreshold, &librt_setcalloptimizethreshold_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL librt_setcalloptimizethreshold_f_impl(size_t newThreshold)
/*[[[end]]]*/
{
	size_t result = DeeCode_SetOptimizeCallThreshold(newThreshold);
	return DeeInt_NewSize(result);
}


/*[[[deemon (print_KwCMethod from rt.gen.unpack)("makeclass", """
	DeeObject *base:?X3?N?DType?S?DType;
	DeeObject *descriptor:?GClassDescriptor;
	DeeModuleObject *module:?X2?DModule?N = (DeeModuleObject *)Dee_None;
""");]]]*/
#define librt_makeclass_params "base:?X3?N?DType?S?DType,descriptor:?GClassDescriptor,module:?X2?DModule?N=!N"
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL librt_makeclass_f_impl(DeeObject *base, DeeObject *descriptor, DeeModuleObject *module_);
#ifndef DEFINED_kwlist__base_descriptor_module
#define DEFINED_kwlist__base_descriptor_module
PRIVATE DEFINE_KWLIST(kwlist__base_descriptor_module, { KEX("base", 0xc3cb0590, 0x56fd8eccbdfdd7a7), KEX("descriptor", 0xea150d1, 0xfc445bb1317b9d67), KEX("module", 0xae3684a4, 0xbb78a82535e5801e), KEND });
#endif /* !DEFINED_kwlist__base_descriptor_module */
PRIVATE WUNUSED DREF DeeObject *DCALL librt_makeclass_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *base;
		DeeObject *descriptor;
		DeeModuleObject *module_;
	} args;
	args.module_ = (DeeModuleObject *)Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__base_descriptor_module, "oo|o:makeclass", &args))
		goto err;
	return librt_makeclass_f_impl(args.base, args.descriptor, args.module_);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(librt_makeclass, &librt_makeclass_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL librt_makeclass_f_impl(DeeObject *base, DeeObject *descriptor, DeeModuleObject *module_)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(descriptor, &DeeClassDescriptor_Type))
		goto err;
	if (DeeNone_Check(module_)) {
		/* Special case: no declaring module given. */
		module_ = NULL;
	} else {
		if (DeeObject_AssertTypeExact(module_, &DeeModule_Type))
			goto err;
	}
	return (DREF DeeObject *)DeeClass_New(base, descriptor, module_);
err:
	return NULL;
}

#if 1
#define str_Iterator (*COMPILER_CONTAINER_OF(DeeIterator_Type.tp_name, DeeStringObject, s_str))
#else
/*[[[deemon (PRIVATE_DEFINE_STRING from rt.gen.string)("str_Iterator", "Iterator");]]]*/
PRIVATE DEFINE_STRING_EX(str_Iterator, "Iterator", 0xfce46883, 0x3c33c9d5c64ebfff);
/*[[[end]]]*/
#endif

/*[[[deemon (PRIVATE_DEFINE_STRING from rt.gen.string)("str_ItemType", "ItemType");]]]*/
PRIVATE DEFINE_STRING_EX(str_ItemType, "ItemType", 0x6e2bcfbc, 0x930e682d9edfa03b);
/*[[[end]]]*/


#ifdef __OPTIMIZ_SIZE__
#define return_cached(expr) return expr
#else /* __OPTIMIZ_SIZE__ */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DREF DeeObject *DFCALL
_store_cache(DeeObject **p_cache, DREF DeeObject *result) {
	if likely(result) {
		DREF DeeObject *result_module;
		result_module = DeeModule_FromStaticPointer(result);
		if likely(result_module == (DREF DeeObject *)DeeModule_GetDeemon()) {
			/* Objects statically allocated within the deemon core can never
			 * be destroyed. - As such, we can store a non-referencing pointer
			 * to them that will allow us to skip object calculation the next
			 * time the relevant function is called. */
			atomic_cmpxch(p_cache, NULL, result);
			Dee_DecrefNokill(result_module);
		} else if (result_module) {
			Dee_Decref_unlikely(result_module);
		}
	}
	return result;
}

#define return_cached(expr)                              \
	do {                                                 \
		static DeeObject *_cache = NULL;                 \
		DREF DeeObject *_result  = atomic_read(&_cache); \
		if (_result)                                     \
			return_reference_(_result);                  \
		_result = (expr);                                \
		return _store_cache(&_cache, _result);           \
	}	__WHILE0
#endif /* !__OPTIMIZ_SIZE__ */



PRIVATE WUNUSED DREF DeeObject *DCALL
get_type_of(DREF DeeObject *ob) {
	DREF DeeObject *result = NULL;
	if likely(ob) {
		result = (DeeObject *)Dee_TYPE(ob);
		Dee_Incref(result);
		Dee_Decref_unlikely(ob);
	}
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
get_Iterator_of(DREF DeeObject *ob) {
	DREF DeeObject *result = NULL;
	if likely(ob) {
		result = DeeObject_GetAttr((DeeObject *)ob,
		                           (DeeObject *)&str_Iterator);
		Dee_Decref_unlikely(ob);
	}
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
get_KeysIterator_of(DREF DeeObject *ob) {
	DREF DeeObject *result = NULL;
	if likely(ob) {
		result = DeeObject_GetAttrStringHash((DeeObject *)ob, STR_AND_HASH(KeysIterator));
		Dee_Decref_unlikely(ob);
	}
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
get_ItemType_of(DREF DeeObject *ob) {
	DREF DeeObject *result = NULL;
	if likely(ob) {
		result = DeeObject_GetAttr((DeeObject *)ob,
		                           (DeeObject *)&str_ItemType);
		Dee_Decref_unlikely(ob);
	}
	return result;
}

PRIVATE struct nonempty_roset_instance_struct {
	Dee_OBJECT_HEAD
	size_t                rs_mask;    /* [>= rs_size] Allocated set size. */
	size_t                rs_size;    /* [<= rs_mask] Amount of non-NULL keys. */
	struct Dee_roset_item rs_elem[2]; /* [1..rs_mask+1] Set key hash-vector. */
} nonempty_roset_instance = {
	OBJECT_HEAD_INIT(&DeeRoSet_Type),
	/* .rs_mask = */ 1,
	/* .rs_size = */ 1,
	/* .rs_elem = */ { { DeeInt_Zero, 0 } } /* hash(int(0)) == 0 */
};

STATIC_ASSERT(offsetof(struct nonempty_roset_instance_struct, rs_mask) == offsetof(DeeRoSetObject, rs_mask));
STATIC_ASSERT(offsetof(struct nonempty_roset_instance_struct, rs_size) == offsetof(DeeRoSetObject, rs_size));
STATIC_ASSERT(offsetof(struct nonempty_roset_instance_struct, rs_elem) == offsetof(DeeRoSetObject, rs_elem));

#define nonempty_stub_set ((DeeObject *)&nonempty_roset_instance)



PRIVATE struct nonempty_rodict_instance_struct {
	Dee_OBJECT_HEAD /* All of the below fields are [const] */
	/*real*/Dee_dict_vidx_t rd_vsize;        /* # of key-value pairs in the dict. */
	Dee_hash_t              rd_hmask;        /* [>= rd_vsize] Hash-mask */
	Dee_dict_gethidx_t      rd_hidxget;      /* [1..1] Getter for "rd_htab" */
	void                   *rd_htab;         /* [== (byte_t *)(_DeeRoDict_GetRealVTab(this) + rd_vsize)] Hash-table (contains indices into "rd_vtab", index==Dee_DICT_HTAB_EOF means END-OF-CHAIN) */
	struct Dee_dict_item    rd_vtab[1];      /* [rd_vsize] Dict key-item pairs (never contains deleted keys). */
	byte_t                  rd_htab_data[2]; /* Dict hash-table. */
} nonempty_rodict_instance = {
	OBJECT_HEAD_INIT(&DeeRoDict_Type),
	/* .rd_vsize     = */ 1,
	/* .rd_hmask     = */ 1,
	/* .rd_hidxget   = */ &Dee_dict_gethidx8,
	/* .rd_htab      = */ nonempty_rodict_instance.rd_htab_data,
	/* .rd_vtab      = */ { Dee_DICT_ITEM_INIT(0, DeeInt_Zero, DeeInt_Zero) }, /* hash(int(0)) == 0 */
	/* .rd_htab_data = */ { Dee_dict_vidx_tovirt(0), Dee_DICT_HTAB_EOF },
};

STATIC_ASSERT(offsetof(struct nonempty_rodict_instance_struct, rd_vsize) == offsetof(DeeRoDictObject, rd_vsize));
STATIC_ASSERT(offsetof(struct nonempty_rodict_instance_struct, rd_hmask) == offsetof(DeeRoDictObject, rd_hmask));
STATIC_ASSERT(offsetof(struct nonempty_rodict_instance_struct, rd_hidxget) == offsetof(DeeRoDictObject, rd_hidxget));
STATIC_ASSERT(offsetof(struct nonempty_rodict_instance_struct, rd_htab) == offsetof(DeeRoDictObject, rd_htab));
STATIC_ASSERT(offsetof(struct nonempty_rodict_instance_struct, rd_vtab) == offsetof(DeeRoDictObject, rd_vtab));

#define nonempty_stub_map ((DeeObject *)&nonempty_rodict_instance)




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassOperatorTable_f(void) {
	return_cached(DeeObject_GetAttrStringHash((DeeObject *)&DeeClassDescriptor_Type, STR_AND_HASH(OperatorTable)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttribute_f(void) {
	return_cached(DeeObject_GetAttrStringHash((DeeObject *)&DeeClassDescriptor_Type, STR_AND_HASH(Attribute)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttributeTable_f(void) {
	return_cached(DeeObject_GetAttrStringHash((DeeObject *)&DeeClassDescriptor_Type, STR_AND_HASH(AttributeTable)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ObjectTable_f(void) {
	return_cached(DeeObject_GetAttrStringHash((DeeObject *)&DeeClassDescriptor_Type, STR_AND_HASH(ObjectTable)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeMRO_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeInt_Type, STR_AND_HASH(__mro__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeBases_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeInt_Type, STR_AND_HASH(__bases__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeMROIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_TypeMRO_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeBasesIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_TypeBases_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassOperatorTableIterator_f() {
	return_cached(get_Iterator_of(librt_get_ClassOperatorTable_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ClassAttributeTableIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ClassAttributeTable_f()));
}



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RoDictIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeRoDict_Type, (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RoSetIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeRoSet_Type, (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_KwdsIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeKwds_Type, (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_KwdsMappingIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeKwdsMapping_Type, (DeeObject *)&str_Iterator));
}

PRIVATE DeeObject DeeIterator_StubInstance = {
	OBJECT_HEAD_INIT(&DeeIterator_Type)
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
get_generic_iterator_member_type(char const *__restrict name, Dee_hash_t hash) {
	/* return type(iterator().operator . (name)); */
	return get_type_of(DeeObject_GetAttrStringHash(&DeeIterator_StubInstance, name, hash));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IteratorPending_f(void) {
	return_cached(get_generic_iterator_member_type(STR_AND_HASH(pending)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IteratorFuture_f(void) {
	return_cached(get_generic_iterator_member_type(STR_AND_HASH(future)));
}




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeString_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeBytes_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ListIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeList_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TupleIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeTuple_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_HashSetIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeHashSet_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DictIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeDict_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TracebackIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeTraceback_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCEnum_f(void) {
	return_reference((DREF DeeObject *)Dee_TYPE(&DeeGCEnumTracked_Singleton));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCIter_f(void) {
	return_cached(get_Iterator_of(librt_get_GCEnum_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Traceback_empty_uncached_f(void) {
	DREF DeeObject *iter, *result = NULL;
	iter = librt_get_TracebackIterator_f();
	if likely(iter) {
		result = DeeObject_GetAttrStringHash(iter, STR_AND_HASH(seq));
		Dee_Decref_likely(iter);
	}
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Traceback_empty_f(void) {
	return_cached(librt_get_Traceback_empty_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSet_empty_uncached_f(void) {
	DeeObject *argv[] = { Dee_None };
	return DeeObject_CallAttrStringHash(&DeeGCEnumTracked_Singleton, STR_AND_HASH(reachable), 1, argv);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSet_empty_f(void) {
	return_cached(librt_get_GCSet_empty_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSet_f(void) {
	return_cached(get_type_of(librt_get_GCSet_empty_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_GCSetIterator_f(void) {
	return_cached(get_Iterator_of(get_type_of(librt_get_GCSet_empty_f())));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_NullableTuple_empty_f(void) {
	return_cached(DeeObject_NewDefault(&DeeNullableTuple_Type));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Code_empty_f(void) {
	/* The empty-code object is set when `Code()' is called without any arguments. */
	return_cached(DeeObject_NewDefault(&DeeCode_Type));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_Module_empty_uncached_f(void) {
	DREF DeeObject *empty_code;
	DREF DeeModuleObject *result;
	empty_code = librt_get_Code_empty_f();
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
librt_get_Module_empty_f(void) {
	return_cached(librt_get_Module_empty_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BlackListKwdsIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeBlackListKwds_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BlackListKwIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeBlackListKw_Type,
	                                (DeeObject *)&str_Iterator));
}



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DocKwds_uncached_f(void) {
	/* To implement this, we need to get access to an instance of it,
	 * which we are doing via `type(("import" from deemon).__kwds__)'.
	 * Because the `import()' function is known to implement keyword
	 * support, we can use it as a reference point for a C-level function
	 * with a non-empty keyword list, without having to create such an
	 * object ourself. */
	DREF DeeObject *result, *kwds;
	kwds = DeeObject_GetAttrStringHash((DeeObject *)&DeeBuiltin_Import, STR_AND_HASH(__kwds__));
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
librt_get_DocKwds_f(void) {
	return_cached(librt_get_DocKwds_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DocKwdsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_DocKwds_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapHashFilter_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrStringHash(nonempty_stub_map, STR_AND_HASH(byhash), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapHashFilter_f(void) {
	return_cached(librt_get_MapHashFilter_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapByAttr_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash(nonempty_stub_map, STR_AND_HASH(byattr))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapKeys_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(map_keys, nonempty_stub_map)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapValues_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(map_values, nonempty_stub_map)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapHashFilterIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapHashFilter_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeOperators_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeString_Type, STR_AND_HASH(__operators__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_TypeOperatorsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_TypeOperators_f()));
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
librt_get_sequence_mutation_type(char const *__restrict name, Dee_hash_t hash) {
	DREF DeeObject *ob, *result = NULL;
	ob = DeeTuple_Pack(2, Dee_None, Dee_None);
	if likely(ob) {
		DeeObject *argv[] = { DeeInt_One };
		result = get_type_of(DeeObject_CallAttrStringHash(ob, name, hash, 1, argv));
		Dee_Decref(ob);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
librt_get_string_mutation_type(char const *__restrict name, Dee_hash_t hash) {
	DeeObject *argv[] = { DeeInt_One };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, name, hash, 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqCombinations_f(void) {
	return_cached(librt_get_string_mutation_type(STR_AND_HASH(combinations)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqCombinationsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqCombinations_f()));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatCombinations_f(void) {
	return_cached(librt_get_string_mutation_type(STR_AND_HASH(repeatcombinations)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatCombinationsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqRepeatCombinations_f()));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqPermutations_f(void) {
	return_cached(librt_get_string_mutation_type(STR_AND_HASH(permutations)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqPermutationsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqPermutations_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqCombinationsView_f(void) {
	return_cached(get_ItemType_of(librt_get_SeqCombinations_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSegments_f(void) {
	/* Since string overrides `segments', we must use a true sequence here! */
	return_cached(librt_get_sequence_mutation_type(STR_AND_HASH(segments)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSegmentsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqSegments_f()));
}




PRIVATE WUNUSED NONNULL((1)) size_t DCALL
my_custom_size(DeeObject *__restrict self) {
	(void)self;
	return 42;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
my_custom_getitem_index(DeeObject *self, size_t index) {
	(void)self;
	(void)index;
	return_none;
}

#define my_custom_iter         my_custom_sizeob
#define my_custom_map_iterkeys my_custom_sizeob
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
my_custom_sizeob(DeeObject *__restrict self) {
	(void)self;
	return_none;
}

PRIVATE struct type_getset tpconst type_getset_with_iterkeys[] = {
	TYPE_GETTER_NODOC("iterkeys", &my_custom_map_iterkeys),
	TYPE_GETSET_END,
};

#if __SIZEOF_SIZE_T__ == __SIZEOF_POINTER__
#define my_custom_getitem_PTR ((DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&my_custom_getitem_index)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_POINTER__ */
#define my_custom_getitem_PTR &my_custom_getitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
my_custom_getitem(DeeObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	return_none;
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_POINTER__ */

#define INIT_CUSTOM_SEQ_TYPE(tp_seq) \
	INIT_CUSTOM_SEQ_TYPE_EX(&DeeSeq_Type, tp_seq, NULL, NULL, NULL)
#define INIT_CUSTOM_SEQ_TYPE_EX(tp_base, tp_seq, tp_methods,  \
                                tp_getsets, tp_method_hints)  \
	{                                                         \
		OBJECT_HEAD_INIT(&DeeType_Type),                      \
		/* .tp_name     = */ NULL,                            \
		/* .tp_doc      = */ NULL,                            \
		/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,    \
		/* .tp_weakrefs = */ 0,                               \
		/* .tp_features = */ TF_NONE,                         \
		/* .tp_base     = */ tp_base,                         \
		/* .tp_init = */ {                                    \
			{                                                 \
				/* .tp_alloc = */ {                           \
					/* .tp_ctor      = */ (Dee_funptr_t)NULL, \
					/* .tp_copy_ctor = */ (Dee_funptr_t)NULL, \
					/* .tp_deep_ctor = */ (Dee_funptr_t)NULL, \
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL, \
					TYPE_AUTO_ALLOCATOR(DeeObject)            \
				}                                             \
			},                                                \
			/* .tp_dtor        = */ NULL,                     \
			/* .tp_assign      = */ NULL,                     \
			/* .tp_move_assign = */ NULL                      \
		},                                                    \
		/* .tp_cast = */ { NULL },                            \
		/* .tp_visit         = */ NULL,                       \
		/* .tp_gc            = */ NULL,                       \
		/* .tp_math          = */ NULL,                       \
		/* .tp_cmp           = */ NULL,                       \
		/* .tp_seq           = */ tp_seq,                     \
		/* .tp_iter_next     = */ NULL,                       \
		/* .tp_iterator      = */ NULL,                       \
		/* .tp_attr          = */ NULL,                       \
		/* .tp_with          = */ NULL,                       \
		/* .tp_buffer        = */ NULL,                       \
		/* .tp_methods       = */ tp_methods,                 \
		/* .tp_getsets       = */ tp_getsets,                 \
		/* .tp_members       = */ NULL,                       \
		/* .tp_class_methods = */ NULL,                       \
		/* .tp_class_getsets = */ NULL,                       \
		/* .tp_class_members = */ NULL,                       \
		/* .tp_method_hints  = */ tp_method_hints             \
	}

PRIVATE struct type_seq type_seq_with_size_and_getitem_index = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ &my_custom_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ &my_custom_getitem_index,
};
PRIVATE DeeTypeObject type_with_size_and_getitem_index = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_size_and_getitem_index);
PRIVATE DeeObject object_with_size_and_getitem_index = { OBJECT_HEAD_INIT(&type_with_size_and_getitem_index) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqConcat_f(void) {
	return_cached(get_type_of(DeeObject_Add(&object_with_size_and_getitem_index,
	                                        &object_with_size_and_getitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqConcatIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqConcat_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilter_uncached_f(void) {
	DeeObject *argv[] = { Dee_None };
	return get_type_of(DeeObject_CallAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(filter), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilterAsUnbound_uncached_f(void) {
	DeeObject *argv[] = { Dee_None };
	return get_type_of(DeeObject_CallAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(ubfilter), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqHashFilter_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(byhash), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilter_f(void) {
	return_cached(librt_get_SeqFilter_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilterAsUnbound_f(void) {
	return_cached(librt_get_SeqFilterAsUnbound_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqHashFilter_f(void) {
	return_cached(librt_get_SeqHashFilter_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFilterIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqFilter_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqHashFilterIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqHashFilter_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqMapped_uncached_f(void) {
	DeeObject *argv[] = { Dee_None };
	return get_type_of(DeeObject_CallAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(map), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqMapped_f(void) {
	return_cached(librt_get_SeqMapped_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqMappedIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqMapped_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRange_f(void) {
	return_cached(get_type_of(DeeRange_New(Dee_None, Dee_None, NULL)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRangeIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqRange_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIntRange_f(void) {
	return_cached(get_type_of(DeeRange_NewInt(0, 20, 1)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIntRangeIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqIntRange_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeat_uncached_f(void) {
	DeeObject *argv[] = { &object_with_size_and_getitem_index, DeeInt_One };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&DeeSeq_Type, STR_AND_HASH(repeatseq), 2, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeat_f(void) {
	return_cached(librt_get_SeqRepeat_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqRepeatIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqRepeat_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqItemRepeat_uncached_f(void) {
	DeeObject *argv[] = { Dee_None, DeeInt_One };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&DeeSeq_Type, STR_AND_HASH(repeat), 2, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqItemRepeat_f(void) {
	return_cached(librt_get_SeqItemRepeat_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqItemRepeatIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqItemRepeat_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIds_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(ids))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqIdsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqIds_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqTypes_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(types))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqTypesIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqTypes_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqClasses_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(classes))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqClassesIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqClasses_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFlat_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(flatten))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqFlatIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqFlat_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEach_stub_instance(void) {
	return_cached(DeeObject_GetAttrStringHash(&object_with_size_and_getitem_index, STR_AND_HASH(each)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
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

PRIVATE WUNUSED DREF DeeObject *DCALL
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

PRIVATE WUNUSED DREF DeeObject *DCALL
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

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrKw_stub_instance(void) {
	DREF DeeObject *result;
	result = librt_get_SeqEach_stub_instance();
	if likely(result) {
		DREF DeeObject *temp;
		temp = DeeObject_CallAttrKw(result, (DeeObject *)&str_Iterator,
		                            0, NULL, Dee_EmptyMapping);
		Dee_Decref(result);
		result = temp;
	}
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEach_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqEach_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachOperator_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqEachOperator_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachOperatorIterator_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(librt_get_SeqEachOperator_stub_instance())));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachGetAttr_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqEachGetAttr_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachGetAttrIterator_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(librt_get_SeqEachGetAttr_stub_instance())));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttr_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqEachCallAttr_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrIterator_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(librt_get_SeqEachCallAttr_stub_instance())));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrKw_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqEachCallAttrKw_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEachCallAttrKwIterator_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(librt_get_SeqEachCallAttrKw_stub_instance())));
}




PRIVATE DeeSeqSomeObject SeqSome_stub_instance = {
	OBJECT_HEAD_INIT(&DeeSeqSome_Type),
	/* .se_seq = */ Dee_EmptyTuple
};

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeOperator_stub_instance(void) {
	return DeeObject_Pos((DeeObject *)&SeqSome_stub_instance);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeGetAttr_stub_instance(void) {
	return DeeObject_GetAttr((DeeObject *)&SeqSome_stub_instance, (DeeObject *)&str_Iterator);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeCallAttr_stub_instance(void) {
	return DeeObject_CallAttr((DeeObject *)&SeqSome_stub_instance,
	                          (DeeObject *)&str_Iterator, 0, NULL);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeCallAttrKw_stub_instance(void) {
	return DeeObject_CallAttrKw((DeeObject *)&SeqSome_stub_instance,
	                            (DeeObject *)&str_Iterator, 0, NULL,
	                            Dee_EmptyMapping);
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeOperator_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqSomeOperator_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeGetAttr_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqSomeGetAttr_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeCallAttr_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqSomeCallAttr_stub_instance()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqSomeCallAttrKw_Type_f(void) {
	return_cached(get_type_of(librt_get_SeqSomeCallAttrKw_stub_instance()));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqWithSizeAndGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_GetRange(&object_with_size_and_getitem_index, DeeInt_Zero, DeeInt_One)));
}


PRIVATE struct type_seq type_seq_with_size_and_getitem_index_fast = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ &my_custom_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_getitem_index_fast = */ &my_custom_getitem_index,
};
PRIVATE DeeTypeObject type_with_size_and_getitem_index_fast = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_size_and_getitem_index_fast);
PRIVATE DeeObject object_with_size_and_getitem_index_fast = { OBJECT_HEAD_INIT(&type_with_size_and_getitem_index_fast) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqWithSizeAndGetItemIndexFast_Type_f(void) {
	return_cached(get_type_of(DeeObject_GetRange(&object_with_size_and_getitem_index_fast, DeeInt_Zero, DeeInt_One)));
}

PRIVATE struct type_seq type_seq_with_size_and_trygetitem_index = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ &my_custom_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_trygetitem         = */ NULL,
	/* .tp_trygetitem_index   = */ &my_custom_getitem_index,
};
PRIVATE DeeTypeObject type_with_size_and_trygetitem_index = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_size_and_trygetitem_index);
PRIVATE DeeObject object_with_size_and_trygetitem_index = { OBJECT_HEAD_INIT(&type_with_size_and_trygetitem_index) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqWithSizeAndTryGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_GetRange(&object_with_size_and_trygetitem_index, DeeInt_Zero, DeeInt_One)));
}

PRIVATE struct type_seq type_seq_with_sizeob_and_getitem = {
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ &my_custom_sizeob,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ my_custom_getitem_PTR,
};
PRIVATE DeeTypeObject type_with_sizeob_and_getitem = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_sizeob_and_getitem);
PRIVATE DeeObject object_with_sizeob_and_getitem = { OBJECT_HEAD_INIT(&type_with_sizeob_and_getitem) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqWithSizeAndGetItem_Type_f(void) {
	return_cached(get_type_of(DeeObject_GetRange(&object_with_sizeob_and_getitem, DeeInt_Zero, DeeInt_One)));
}

PRIVATE struct type_seq type_seq_with_iter_and_size = {
	/* .tp_iter            = */ &my_custom_iter,
	/* .tp_sizeob          = */ NULL,
	/* .tp_contains        = */ NULL,
	/* .tp_getitem         = */ NULL,
	/* .tp_delitem         = */ NULL,
	/* .tp_setitem         = */ NULL,
	/* .tp_getrange        = */ NULL,
	/* .tp_delrange        = */ NULL,
	/* .tp_setrange        = */ NULL,
	/* .tp_foreach         = */ NULL,
	/* .tp_foreach_pair    = */ NULL,
	/* .tp_bounditem       = */ NULL,
	/* .tp_hasitem         = */ NULL,
	/* .tp_size            = */ &my_custom_size,
};
PRIVATE DeeTypeObject type_with_iter_and_size = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_iter_and_size);
PRIVATE DeeObject object_with_iter_and_size = { OBJECT_HEAD_INIT(&type_with_iter_and_size) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqWithIterAndLimit_Type_f(void) {
	return_cached(get_type_of(DeeObject_GetRange(&object_with_iter_and_size, DeeInt_Zero, DeeInt_One)));
}

PRIVATE struct type_seq type_seq_with_getitem_index = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ NULL,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ &my_custom_getitem_index,
};
PRIVATE DeeTypeObject type_with_getitem_index = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_getitem_index);
PRIVATE DeeObject object_with_getitem_index = { OBJECT_HEAD_INIT(&type_with_getitem_index) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_getitem_index)));
}

#define get_seq_enumerate_of_noinherit(self) \
	DeeObject_InvokeMethodHint(seq_makeenumeration, self)
#define get_seq_enumerate_of_noinherit_with_int_range(self) \
	DeeObject_InvokeMethodHint(seq_makeenumeration_with_intrange, self, 0, 1)
#define get_seq_enumerate_of_noinherit_with_range(self) \
	DeeObject_InvokeMethodHint(seq_makeenumeration_with_range, self, Dee_EmptyString, Dee_EmptyString)

#define get_map_enumerate_of_noinherit(self) \
	DeeObject_InvokeMethodHint(map_makeenumeration, self)
#define get_map_enumerate_of_noinherit_with_range(self) \
	DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, Dee_EmptyString, Dee_EmptyString)

/*
PRIVATE WUNUSED DREF DeeObject *DCALL
get_seq_enumerate_of(DREF DeeObject *seq) {
	DREF DeeObject *result;
	if unlikely(!seq)
		goto err;
	result = get_seq_enumerate_of_noinherit(seq);
	Dee_Decref(seq);
	return result;
err:
	return NULL;
}
*/


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithGetItemIndexPair_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(get_seq_enumerate_of_noinherit(&object_with_getitem_index))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeAndGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_size_and_getitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeAndGetItemIndexPair_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(get_seq_enumerate_of_noinherit(&object_with_size_and_getitem_index))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeAndGetItemIndexFast_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_size_and_getitem_index_fast)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeAndGetItemIndexFastPair_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(get_seq_enumerate_of_noinherit(&object_with_size_and_getitem_index_fast))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeAndTryGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_size_and_trygetitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeAndTryGetItemIndexPair_Type_f(void) {
	return_cached(get_Iterator_of(get_type_of(get_seq_enumerate_of_noinherit(&object_with_size_and_trygetitem_index))));
}

PRIVATE struct type_seq type_seq_with_getitem = {
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ my_custom_getitem_PTR,
};
PRIVATE DeeTypeObject type_with_getitem = INIT_CUSTOM_SEQ_TYPE(&type_seq_with_getitem);
PRIVATE DeeObject object_with_getitem = { OBJECT_HEAD_INIT(&type_with_getitem) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithGetItem_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_getitem)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeObAndGetItem_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_sizeob_and_getitem)));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
my_custom_enumerate(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	(void)self;
	(void)proc;
	(void)arg;
	return 0;
}


PRIVATE struct type_method_hint type_with_seq_enumerate_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate, &my_custom_enumerate),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method type_with_seq_enumerate_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE DeeTypeObject type_with_seq_enumerate = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ NULL,
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_AUTO_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ { NULL },
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ type_with_seq_enumerate_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ type_with_seq_enumerate_method_hints,
};
PRIVATE DeeObject object_with_seq_enumerate = { OBJECT_HEAD_INIT(&type_with_seq_enumerate) };

PRIVATE struct type_seq type_seq_with_getitem_formap = {
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ my_custom_getitem_PTR,
};
PRIVATE DeeTypeObject type_with_iterkeys_and_getitem_formap = INIT_CUSTOM_SEQ_TYPE_EX(&DeeMapping_Type, &type_seq_with_getitem_formap, NULL, type_getset_with_iterkeys, NULL);
PRIVATE DeeObject object_with_iterkeys_and_getitem_formap = { OBJECT_HEAD_INIT(&type_with_iterkeys_and_getitem_formap) };



PRIVATE struct type_seq type_seq_with_trygetitem = {
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ NULL,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_trygetitem         = */ my_custom_getitem_PTR,
};
PRIVATE DeeTypeObject type_with_iterkeys_and_trygetitem_formap = INIT_CUSTOM_SEQ_TYPE_EX(&DeeMapping_Type, &type_seq_with_trygetitem, NULL, type_getset_with_iterkeys, NULL);
PRIVATE DeeObject object_with_iterkeys_and_trygetitem_formap = { OBJECT_HEAD_INIT(&type_with_iterkeys_and_trygetitem_formap) };




PRIVATE DeeTypeObject type_with_iter_and_size_formap = INIT_CUSTOM_SEQ_TYPE_EX(&DeeMapping_Type, &type_seq_with_iter_and_size, NULL, NULL, NULL);
PRIVATE DeeObject object_with_iter_and_size_formap = { OBJECT_HEAD_INIT(&type_with_iter_and_size_formap) };

PRIVATE struct type_method_hint type_with_map_enumerate_method_hints[] = {
	TYPE_METHOD_HINT(map_enumerate, &my_custom_enumerate),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method type_with_map_enumerate_methods[] = {
	TYPE_METHOD_HINTREF(__map_enumerate__),
	TYPE_METHOD_END
};

PRIVATE DeeTypeObject type_with_map_enumerate = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ NULL,
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_AUTO_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ { NULL },
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ type_with_map_enumerate_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ type_with_map_enumerate_method_hints,
};
PRIVATE DeeObject object_with_map_enumerate = { OBJECT_HEAD_INIT(&type_with_map_enumerate) };

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorSizeAndGetItemIndexFast_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_size_and_getitem_index_fast)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndGetItemIndexFast_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_int_range(&object_with_size_and_getitem_index_fast)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorTryGetItemIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_size_and_trygetitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorTryGetItemIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_int_range(&object_with_size_and_trygetitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorGetItemIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_size_and_getitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorGetItemIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_int_range(&object_with_size_and_getitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_sizeob_and_getitem)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorGetItemIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_getitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithIntFilterAndSeqOperatorGetItemIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_int_range(&object_with_getitem_index)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithFilterAndSeqOperatorSizeobAndSeqOperatorGetItem_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_range(&object_with_sizeob_and_getitem)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorGetItem_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_getitem)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithFilterAndSeqOperatorGetItem_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_range(&object_with_getitem)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqOperatorIterAndCounter_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_iter_and_size)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithIntFilterAndSeqOperatorIterAndCounter_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_int_range(&object_with_iter_and_size)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithSeqEnumerate_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit(&object_with_seq_enumerate)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithIntFilterAndSeqEnumerateIndex_Type_f(void) {
	return_cached(get_type_of(get_seq_enumerate_of_noinherit_with_int_range(&object_with_seq_enumerate)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithFilterAndMapOperatorIterAndUnpack_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit_with_range(&object_with_iter_and_size_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithMapIterkeysAndMapOperatorGetItem_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit(&object_with_iterkeys_and_getitem_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorGetItem_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit_with_range(&object_with_iterkeys_and_getitem_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithMapIterkeysAndMapOperatorTryGetItem_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit(&object_with_iterkeys_and_trygetitem_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorTryGetItem_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit_with_range(&object_with_iterkeys_and_trygetitem_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithMapEnumerate_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit(&object_with_map_enumerate)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqEnumWithFilterAndMapEnumerateRange_Type_f(void) {
	return_cached(get_type_of(get_map_enumerate_of_noinherit_with_range(&object_with_map_enumerate)));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithSizeObAndGetItemPair_Type_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem_Type_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithGetItemPair_Type_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqEnumWithSeqOperatorGetItem_Type_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithNextAndLimit_Type_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqWithIterAndLimit_Type_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithIterKeysAndGetItemForMap_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_iterkeys_and_getitem_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithIterKeysAndTryGetItemForMap_Type_f(void) {
	return_cached(get_type_of(DeeObject_Iter(&object_with_iterkeys_and_trygetitem_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithNextAndCounterPair_Type_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqEnumWithSeqOperatorIterAndCounter_Type_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithNextAndCounterAndLimitPair_Type_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqEnumWithIntFilterAndSeqOperatorIterAndCounter_Type_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithNextAndUnpackFilter_Type_f(void) {
	return_cached(get_Iterator_of(librt_get_SeqEnumWithFilterAndMapOperatorIterAndUnpack_Type_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithNextKey_Type_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(map_iterkeys, &object_with_iter_and_size_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithNextValue_Type_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(map_itervalues, &object_with_iter_and_size_formap)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqReversedWithGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(seq_reversed, &object_with_size_and_getitem_index, 0, (size_t)-1)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqReversedWithGetItemIndexFast_Type_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(seq_reversed, &object_with_size_and_getitem_index_fast, 0, (size_t)-1)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqReversedWithTryGetItemIndex_Type_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(seq_reversed, &object_with_size_and_trygetitem_index, 0, (size_t)-1)));
}

#define librt_get_default_sequence_type(name) \
	DeeObject_GetAttrStringHash((DeeObject *)&DeeSeq_Type, STR_AND_HASH(name))

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqWithIter_Type_f(void) {
	return_cached(librt_get_default_sequence_type(__SeqWithIter__));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithForeach_Type_f(void) {
	return_cached(librt_get_default_sequence_type(__IterWithForeach__));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithForeachPair_Type_f(void) {
	return_cached(librt_get_default_sequence_type(__IterWithForeachPair__));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithEnumerateMap_Type_f(void) {
	return_cached(librt_get_default_sequence_type(__IterWithEnumerateMap__));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithEnumerateIndexSeq_Type_f(void) {
	return_cached(librt_get_default_sequence_type(__IterWithEnumerateIndexSeq__));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_IterWithEnumerateSeq_Type_f(void) {
	return_cached(librt_get_default_sequence_type(__IterWithEnumerateSeq__));
}



PRIVATE DEFINE_TUPLE(non_empty_tuple, 1, { Dee_None });

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DistinctIterator_Type_f(void) {
	return_cached(get_type_of((*DeeSet_Type.tp_seq->tp_iter)((DeeObject *)&non_empty_tuple)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DistinctSetWithKey_uncached_f(void) {
	DeeObject *arg0 = (DeeObject *)&non_empty_tuple;
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&non_empty_tuple,
	                                                STR_AND_HASH(distinct), 1, &arg0));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DistinctSetWithKey_f(void) {
	return_cached(librt_get_DistinctSetWithKey_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DistinctIteratorWithKey_f(void) {
	return_cached(get_Iterator_of(librt_get_DistinctSetWithKey_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_DistinctMappingIterator_Type_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(map_operator_iter, &object_with_iter_and_size)));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_CachedSeqWithIter_f(void) {
	return_cached(get_type_of(DeeObject_InvokeMethodHint(seq_cached, &object_with_iter_and_size)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_CachedSeqWithIterIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_CachedSeqWithIter_f()));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetInversion_uncached_f(void) {
	return get_type_of(DeeObject_Inv(nonempty_stub_set));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetInversion_f(void) {
	return_cached(librt_get_SetInversion_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetUnion_uncached_f(void) {
	return get_type_of(DeeObject_Or(nonempty_stub_set, nonempty_stub_set));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetSymmetricDifference_uncached_f(void) {
	return get_type_of(DeeObject_Xor(nonempty_stub_set, nonempty_stub_set));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetIntersection_uncached_f(void) {
	return get_type_of(DeeObject_And(nonempty_stub_set, nonempty_stub_set));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetDifference_uncached_f(void) {
	return get_type_of(DeeObject_Sub(nonempty_stub_set, nonempty_stub_set));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapUnion_uncached_f(void) {
	return get_type_of(DeeObject_Or(nonempty_stub_map, nonempty_stub_map));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapSymmetricDifference_uncached_f(void) {
	return get_type_of(DeeObject_Xor(nonempty_stub_map, nonempty_stub_map));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapIntersection_uncached_f(void) {
	return get_type_of(DeeObject_And(nonempty_stub_map, nonempty_stub_map));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapDifference_uncached_f(void) {
	return get_type_of(DeeObject_Sub(nonempty_stub_map, nonempty_stub_map));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetUnion_f(void) {
	return_cached(librt_get_SetUnion_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetSymmetricDifference_f(void) {
	return_cached(librt_get_SetSymmetricDifference_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetIntersection_f(void) {
	return_cached(librt_get_SetIntersection_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetDifference_f(void) {
	return_cached(librt_get_SetDifference_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapUnion_f(void) {
	return_cached(librt_get_MapUnion_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapSymmetricDifference_f(void) {
	return_cached(librt_get_MapSymmetricDifference_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapIntersection_f(void) {
	return_cached(librt_get_MapIntersection_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapDifference_f(void) {
	return_cached(librt_get_MapDifference_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetUnionIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SetUnion_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetSymmetricDifferenceIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SetSymmetricDifference_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetIntersectionIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SetIntersection_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SetDifferenceIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_SetDifference_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapUnionIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapUnion_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapSymmetricDifferenceIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapSymmetricDifference_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapIntersectionIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapIntersection_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapDifferenceIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapDifference_f()));
}



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SeqOneIterator_Type_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeSeqOne_Type,
	                                (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SharedVectorIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeSharedVector_Type, (DeeObject *)&str_Iterator));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_SharedMapIterator_f(void) {
	return_cached(DeeObject_GetAttr((DeeObject *)&DeeSharedMap_Type, (DeeObject *)&str_Iterator));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RefVector_f(void) {
	return_cached(get_type_of(DeeRefVector_NewReadonly(Dee_None, 0, NULL)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RefVectorIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_RefVector_f()));
}




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleExports_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)DeeModule_GetDeemon(), STR_AND_HASH(__exports__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleExportsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ModuleExports_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ModuleGlobals_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)DeeModule_GetDeemon(), STR_AND_HASH(__globals__))));
}

PRIVATE DEFINE_BYTES(small_bytes, Dee_BUFFER_FREADONLY, 1, { 0 });

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesFind_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&small_bytes, STR_AND_HASH(findall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesFind_f(void) {
	return_cached(librt_get_BytesFind_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesFindIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_BytesFind_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseFind_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&small_bytes, STR_AND_HASH(casefindall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseFind_f(void) {
	return_cached(librt_get_BytesCaseFind_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseFindIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_BytesCaseFind_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSegments_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_One };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&small_bytes, STR_AND_HASH(segments), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSegments_f(void) {
	return_cached(librt_get_BytesSegments_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSegmentsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_BytesSegments_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSplit_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&small_bytes, STR_AND_HASH(split), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSplit_f(void) {
	return_cached(librt_get_BytesSplit_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_BytesSplit_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseSplit_uncached_f(void) {
	DeeObject *argv[] = { DeeInt_Zero };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&small_bytes, STR_AND_HASH(casesplit), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseSplit_f(void) {
	return_cached(librt_get_BytesCaseSplit_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesCaseSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_BytesCaseSplit_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesLineSplit_f(void) {
	return_cached(get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&small_bytes, STR_AND_HASH(splitlines), 0, NULL)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_BytesLineSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_BytesLineSplit_f()));
}






PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSegments_f(void) {
	return_cached(librt_get_string_mutation_type(STR_AND_HASH(segments)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringFind_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(findall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseFind_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(casefindall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSplit_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(split), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseSplit_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(casesplit), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringScan_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)&str_Iterator };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(scanf), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringFind_f(void) {
	return_cached(librt_get_StringFind_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseFind_f(void) {
	return_cached(librt_get_StringCaseFind_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSplit_f(void) {
	return_cached(librt_get_StringSplit_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseSplit_f(void) {
	return_cached(librt_get_StringCaseSplit_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringLineSplit_f(void) {
	return_cached(get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(splitlines), 0, NULL)));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringScan_f(void) {
	return_cached(librt_get_StringScan_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringOrdinals_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&str_Iterator, STR_AND_HASH(ordinals))));
}


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSegmentsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringSegments_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringFindIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringFind_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseFindIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringCaseFind_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringSplit_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringCaseSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringCaseSplit_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringLineSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringLineSplit_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_StringScanIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_StringScan_f()));
}



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReFindAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(refindall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegFindAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(regfindall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReLocateAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(relocateall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegLocateAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(reglocateall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSplit_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(resplit), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReGroups_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(regmatch), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubStrings_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyString, STR_AND_HASH(rescanf), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesFindAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyBytes, STR_AND_HASH(refindall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesFindAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyBytes, STR_AND_HASH(regfindall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesLocateAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyBytes, STR_AND_HASH(relocateall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesLocateAll_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyBytes, STR_AND_HASH(reglocateall), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesSplit_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyBytes, STR_AND_HASH(resplit), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubBytes_uncached_f(void) {
	DeeObject *argv[] = { (DeeObject *)Dee_EmptyString };
	return get_type_of(DeeObject_CallAttrStringHash(Dee_EmptyBytes, STR_AND_HASH(rescanf), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReFindAll_f(void) {
	return_cached(librt_get_ReFindAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegFindAll_f(void) {
	return_cached(librt_get_RegFindAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReLocateAll_f(void) {
	return_cached(librt_get_ReLocateAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegLocateAll_f(void) {
	return_cached(librt_get_RegLocateAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSplit_f(void) {
	return_cached(librt_get_ReSplit_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReGroups_f(void) {
	return_cached(librt_get_ReGroups_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubStrings_f(void) {
	return_cached(librt_get_ReSubStrings_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesFindAll_f(void) {
	return_cached(librt_get_ReBytesFindAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesFindAll_f(void) {
	return_cached(librt_get_RegBytesFindAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesLocateAll_f(void) {
	return_cached(librt_get_ReBytesLocateAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesLocateAll_f(void) {
	return_cached(librt_get_RegBytesLocateAll_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesSplit_f(void) {
	return_cached(librt_get_ReBytesSplit_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSubBytes_f(void) {
	return_cached(librt_get_ReSubBytes_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReFindAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ReFindAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegFindAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_RegFindAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReLocateAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ReLocateAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegLocateAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_RegLocateAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ReSplit_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesFindAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ReBytesFindAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesFindAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_RegBytesFindAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesLocateAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ReBytesLocateAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_RegBytesLocateAllIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_RegBytesLocateAll_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_ReBytesSplitIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_ReBytesSplit_f()));
}

PRIVATE DEFINE_CODE(DeeCode_EmptyYielding,
                    /* co_flags:      */ CODE_FCOPYABLE | CODE_FYIELDING,
                    /* co_localc:     */ 0,
                    /* co_constc:     */ 0,
                    /* co_refc:       */ 0,
                    /* co_refstaticc: */ 0,
                    /* co_exceptc:    */ 0,
                    /* co_argc_min:   */ 0,
                    /* co_argc_max:   */ 0,
                    /* co_framesize:  */ 0,
                    /* co_codebytes:  */ 1,
                    /* co_module:     */ &DeeModule_Deemon,
                    /* co_keywords:   */ NULL,
                    /* co_defaultv:   */ NULL,
                    /* co_constv:     */ NULL,
                    /* co_exceptv:    */ NULL,
                    /* co_ddi:        */ &DeeDDI_Empty,
                    /* co_code:       */ { ASM_RET_NONE });
PRIVATE DEFINE_FUNCTION_NOREFS(DeeFunction_EmptyYielding,
                               /* fo_code: */ (DeeCodeObject *)&DeeCode_EmptyYielding);
PRIVATE DEFINE_YIELD_FUNCTION_NOARGS(DeeYieldFunction_Empty,
                                     /* yf_func: */ (DeeFunctionObject *)&DeeFunction_EmptyYielding.ob,
                                     /* yf_kw:   */ NULL,
                                     /* yf_this: */ NULL);
PRIVATE struct {
	struct gc_head_link _gc_head_data;
	DeeYieldFunctionIteratorObject ob;
} DeeYieldFunctionIterator_Empty = {
	{ NULL, NULL },
	{
		OBJECT_HEAD_INIT(&DeeYieldFunctionIterator_Type),
		/* .yi_func = */ (DeeYieldFunctionObject *)&DeeYieldFunction_Empty,
		/* .yi_frame = */ {
			/* .cf_prev    = */ NULL,
			/* .cf_func    = */ (DeeFunctionObject *)&DeeFunction_EmptyYielding.ob,
			/* .cf_argc    = */ 0,
			/* .cf_argv    = */ NULL,
			/* .cf_kw      = */ NULL,
			/* .cf_frame   = */ NULL,
			/* .cf_stack   = */ NULL,
			/* .cf_sp      = */ NULL,
			/* .cf_ip      = */ DeeCode_EmptyYielding.co_code,
			/* .cf_vargs   = */ NULL,
			/* .cf_this    = */ NULL,
			/* .cf_result  = */ NULL,
			/* .cf_stacksz = */ 0,
			/* .cf_flags   = */ CODE_FCOPYABLE | CODE_FYIELDING,
		},
#ifndef CONFIG_NO_THREADS
		/* .yi_lock = */ DEE_RSHARED_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	}
};

PRIVATE DeeFrameObject DeeFrame_Empty = {
	OBJECT_HEAD_INIT(&DeeFrame_Type),
	/* .f_owner = */ (DeeObject *)&DeeYieldFunctionIterator_Empty.ob,
	/* .f_frame = */ &DeeYieldFunctionIterator_Empty.ob.yi_frame,
#ifndef CONFIG_NO_THREADS
	{ (Dee_atomic_rwlock_t *)&DeeYieldFunctionIterator_Empty.ob.yi_lock },
	/* .f_lock  = */ DEE_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .f_flags = */ DEEFRAME_FREADONLY | DEEFRAME_FSHRLOCK | DEEFRAME_FRECLOCK,
	/* .f_revsp = */ 0,
};



PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FunctionStatics_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeFunction_EmptyYielding.ob, STR_AND_HASH(__statics__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FunctionSymbolsByName_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeFunction_EmptyYielding.ob, STR_AND_HASH(__symbols__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_YieldFunctionSymbolsByName_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeYieldFunction_Empty, STR_AND_HASH(__symbols__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FrameArgs_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeFrame_Empty, STR_AND_HASH(__args__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FrameLocals_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeFrame_Empty, STR_AND_HASH(__locals__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FrameStack_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeFrame_Empty, STR_AND_HASH(__stack__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FrameSymbolsByName_f(void) {
	return_cached(get_type_of(DeeObject_GetAttrStringHash((DeeObject *)&DeeFrame_Empty, STR_AND_HASH(__symbols__))));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FunctionStaticsIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_FunctionStatics_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FunctionSymbolsByNameIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_FunctionSymbolsByName_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FunctionSymbolsByNameKeysIterator_f(void) {
	return_cached(get_KeysIterator_of(librt_get_FunctionSymbolsByName_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_YieldFunctionSymbolsByNameIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_YieldFunctionSymbolsByName_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_YieldFunctionSymbolsByNameKeysIterator_f(void) {
	return_cached(get_KeysIterator_of(librt_get_YieldFunctionSymbolsByName_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FrameSymbolsByNameIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_FrameSymbolsByName_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_FrameSymbolsByNameKeysIterator_f(void) {
	return_cached(get_KeysIterator_of(librt_get_FrameSymbolsByName_f()));
}




PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromKeysAndValue_uncached_f(void) {
	DeeObject *argv[] = { Dee_EmptySet, Dee_None };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&DeeMapping_Type, STR_AND_HASH(fromkeys), 2, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromKeysAndCallback_uncached_f(void) {
	DeeObject *argv[] = { Dee_EmptySet, Dee_None, (DeeObject *)&DeeFunction_EmptyYielding.ob };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&DeeMapping_Type, STR_AND_HASH(fromkeys), 3, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromKeysAndValue_f(void) {
	return_cached(librt_get_MapFromKeysAndValue_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromKeysAndCallback_f(void) {
	return_cached(librt_get_MapFromKeysAndCallback_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromKeysAndValueIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapFromKeysAndValue_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromKeysAndCallbackIterator_f(void) {
	return_cached(get_Iterator_of(librt_get_MapFromKeysAndCallback_f()));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromAttr_f(void) {
	DeeObject *argv[] = { (DeeObject *)&DeeFunction_EmptyYielding.ob };
	return get_type_of(DeeObject_CallAttrStringHash((DeeObject *)&DeeMapping_Type, STR_AND_HASH(fromattr), 1, argv));
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_MapFromAttrKeysIterator_f(void) {
	return_cached(get_KeysIterator_of(librt_get_MapFromAttr_f()));
}



struct custom_seq_enumerate_ob {
	OBJECT_HEAD
	DREF DeeObject *cse_wrapper; /* [0..1] The wrapper that was generated (out). */
};

PRIVATE WUNUSED DREF DeeObject *DCALL
custom_seq_enumerate(struct custom_seq_enumerate_ob *self,
                     size_t argc, DeeObject *const *argv) {
	if likely(argc >= 1) {
		DeeObject *wrapper = argv[0];
		Dee_Incref(wrapper);
		if unlikely(!atomic_cmpxch(&self->cse_wrapper, NULL, wrapper))
			Dee_DecrefNokill(wrapper);
	}
	return_none;
}

PRIVATE struct type_method tpconst type_with_custom_seq_enumerate_methods[] = {
	TYPE_METHOD_NODOC(DeeMA___seq_enumerate___name, &custom_seq_enumerate),
	TYPE_METHOD_END
};
PRIVATE DeeTypeObject type_with_custom_seq_enumerate = INIT_CUSTOM_SEQ_TYPE_EX(&DeeSet_Type, NULL, type_with_custom_seq_enumerate_methods, NULL, NULL);

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
noop_seq_enumerate_cb(void *UNUSED(arg),
                      DeeObject *UNUSED(key),
                      DeeObject *UNUSED(value)) {
	return 0;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_EnumerateWrapper_Type_uncached_f(void) {
	Dee_ssize_t error;
	struct custom_seq_enumerate_ob ob = { OBJECT_HEAD_INIT(&type_with_custom_seq_enumerate), NULL };
	ASSERT(ob.ob_refcnt == Dee_STATIC_REFCOUNT_INIT);
	error = DeeObject_InvokeMethodHint(seq_enumerate, (DeeObject *)&ob, &noop_seq_enumerate_cb, NULL);
	ASSERT(ob.ob_refcnt == Dee_STATIC_REFCOUNT_INIT);
	if unlikely(error < 0)
		goto err;
	ASSERT(ob.cse_wrapper);
	return get_type_of(ob.cse_wrapper);
err:
	return NULL;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
noop_seq_enumerate_index_cb(void *UNUSED(arg),
                            size_t UNUSED(key),
                            DeeObject *UNUSED(value)) {
	return 0;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_EnumerateIndexWrapper_Type_uncached_f(void) {
	Dee_ssize_t error;
	struct custom_seq_enumerate_ob ob = { OBJECT_HEAD_INIT(&type_with_custom_seq_enumerate), NULL };
	ASSERT(ob.ob_refcnt == Dee_STATIC_REFCOUNT_INIT);
	error = DeeObject_InvokeMethodHint(seq_enumerate_index, (DeeObject *)&ob, &noop_seq_enumerate_index_cb, NULL, 0, (size_t)-1);
	ASSERT(ob.ob_refcnt == Dee_STATIC_REFCOUNT_INIT);
	if unlikely(error < 0)
		goto err;
	ASSERT(ob.cse_wrapper);
	return get_type_of(ob.cse_wrapper);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_EnumerateWrapper_Type_f(void) {
	return_cached(librt_get_EnumerateWrapper_Type_uncached_f());
}

PRIVATE WUNUSED DREF DeeObject *DCALL
librt_get_EnumerateIndexWrapper_Type_f(void) {
	return_cached(librt_get_EnumerateIndexWrapper_Type_uncached_f());
}


PRIVATE DEFINE_CMETHOD0(librt_get_SeqCombinations, &librt_get_SeqCombinations_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqCombinationsIterator, &librt_get_SeqCombinationsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqRepeatCombinations, &librt_get_SeqRepeatCombinations_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqRepeatCombinationsIterator, &librt_get_SeqRepeatCombinationsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqPermutations, &librt_get_SeqPermutations_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqPermutationsIterator, &librt_get_SeqPermutationsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqCombinationsView, &librt_get_SeqCombinationsView_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqSegments, &librt_get_SeqSegments_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqSegmentsIterator, &librt_get_SeqSegmentsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqConcat, &librt_get_SeqConcat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqConcatIterator, &librt_get_SeqConcatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqFilter, &librt_get_SeqFilter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqFilterAsUnbound, &librt_get_SeqFilterAsUnbound_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqFilterIterator, &librt_get_SeqFilterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqHashFilter, &librt_get_SeqHashFilter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqHashFilterIterator, &librt_get_SeqHashFilterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqMapped, &librt_get_SeqMapped_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqMappedIterator, &librt_get_SeqMappedIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqRange, &librt_get_SeqRange_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqRangeIterator, &librt_get_SeqRangeIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqIntRange, &librt_get_SeqIntRange_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqIntRangeIterator, &librt_get_SeqIntRangeIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqRepeat, &librt_get_SeqRepeat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqRepeatIterator, &librt_get_SeqRepeatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqItemRepeat, &librt_get_SeqItemRepeat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqItemRepeatIterator, &librt_get_SeqItemRepeatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqIds, &librt_get_SeqIds_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqIdsIterator, &librt_get_SeqIdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqTypes, &librt_get_SeqTypes_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqTypesIterator, &librt_get_SeqTypesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqClasses, &librt_get_SeqClasses_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqClassesIterator, &librt_get_SeqClassesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqFlat, &librt_get_SeqFlat_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqFlatIterator, &librt_get_SeqFlatIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEach, &librt_get_SeqEach_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachOperator, &librt_get_SeqEachOperator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachOperatorIterator, &librt_get_SeqEachOperatorIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachGetAttr_np, &librt_get_SeqEachGetAttr_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachGetAttrIterator_np, &librt_get_SeqEachGetAttrIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachCallAttr_np, &librt_get_SeqEachCallAttr_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachCallAttrIterator_np, &librt_get_SeqEachCallAttrIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachCallAttrKw_np, &librt_get_SeqEachCallAttrKw_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEachCallAttrKwIterator_np, &librt_get_SeqEachCallAttrKwIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqSomeOperator, &librt_get_SeqSomeOperator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqSomeGetAttr_np, &librt_get_SeqSomeGetAttr_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqSomeCallAttr_np, &librt_get_SeqSomeCallAttr_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqSomeCallAttrKw_np, &librt_get_SeqSomeCallAttrKw_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorSizeAndGetItemIndexFast, &librt_get_SeqEnumWithSeqOperatorSizeAndGetItemIndexFast_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndGetItemIndexFast, &librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndGetItemIndexFast_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorTryGetItemIndex, &librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorTryGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorTryGetItemIndex, &librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorTryGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorGetItemIndex, &librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorGetItemIndex, &librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem, &librt_get_SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorGetItemIndex, &librt_get_SeqEnumWithSeqOperatorGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithIntFilterAndSeqOperatorGetItemIndex, &librt_get_SeqEnumWithIntFilterAndSeqOperatorGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithFilterAndSeqOperatorSizeobAndSeqOperatorGetItem, &librt_get_SeqEnumWithFilterAndSeqOperatorSizeobAndSeqOperatorGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorGetItem, &librt_get_SeqEnumWithSeqOperatorGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithFilterAndSeqOperatorGetItem, &librt_get_SeqEnumWithFilterAndSeqOperatorGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqOperatorIterAndCounter, &librt_get_SeqEnumWithSeqOperatorIterAndCounter_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithIntFilterAndSeqOperatorIterAndCounter, &librt_get_SeqEnumWithIntFilterAndSeqOperatorIterAndCounter_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithSeqEnumerate, &librt_get_SeqEnumWithSeqEnumerate_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithIntFilterAndSeqEnumerateIndex, &librt_get_SeqEnumWithIntFilterAndSeqEnumerateIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithFilterAndMapOperatorIterAndUnpack, &librt_get_SeqEnumWithFilterAndMapOperatorIterAndUnpack_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithMapIterkeysAndMapOperatorGetItem, &librt_get_SeqEnumWithMapIterkeysAndMapOperatorGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorGetItem, &librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithMapIterkeysAndMapOperatorTryGetItem, &librt_get_SeqEnumWithMapIterkeysAndMapOperatorTryGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorTryGetItem, &librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorTryGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithMapEnumerate, &librt_get_SeqEnumWithMapEnumerate_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqEnumWithFilterAndMapEnumerateRange, &librt_get_SeqEnumWithFilterAndMapEnumerateRange_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqWithSizeAndGetItemIndex, &librt_get_SeqWithSizeAndGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqWithSizeAndGetItemIndexFast, &librt_get_SeqWithSizeAndGetItemIndexFast_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqWithSizeAndTryGetItemIndex, &librt_get_SeqWithSizeAndTryGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqWithSizeAndGetItem, &librt_get_SeqWithSizeAndGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqWithIter, &librt_get_SeqWithIter_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqWithIterAndLimit, &librt_get_SeqWithIterAndLimit_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_EnumerateWrapper, &librt_get_EnumerateWrapper_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_EnumerateIndexWrapper, &librt_get_EnumerateIndexWrapper_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithGetItemIndex, &librt_get_IterWithGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithGetItemIndexPair, &librt_get_IterWithGetItemIndexPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeAndGetItemIndex, &librt_get_IterWithSizeAndGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeAndGetItemIndexPair, &librt_get_IterWithSizeAndGetItemIndexPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeAndGetItemIndexFast, &librt_get_IterWithSizeAndGetItemIndexFast_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeAndGetItemIndexFastPair, &librt_get_IterWithSizeAndGetItemIndexFastPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeAndTryGetItemIndex, &librt_get_IterWithSizeAndTryGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeAndTryGetItemIndexPair, &librt_get_IterWithSizeAndTryGetItemIndexPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithGetItem, &librt_get_IterWithGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithGetItemPair, &librt_get_IterWithGetItemPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeObAndGetItem, &librt_get_IterWithSizeObAndGetItem_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithSizeObAndGetItemPair, &librt_get_IterWithSizeObAndGetItemPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithNextAndLimit, &librt_get_IterWithNextAndLimit_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithIterKeysAndGetItemForMap, &librt_get_IterWithIterKeysAndGetItemForMap_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithIterKeysAndTryGetItemForMap, &librt_get_IterWithIterKeysAndTryGetItemForMap_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithForeach, &librt_get_IterWithForeach_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithForeachPair, &librt_get_IterWithForeachPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithEnumerateMap, &librt_get_IterWithEnumerateMap_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithEnumerateIndexSeq, &librt_get_IterWithEnumerateIndexSeq_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithEnumerateSeq, &librt_get_IterWithEnumerateSeq_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithNextAndCounterPair, &librt_get_IterWithNextAndCounterPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithNextAndCounterAndLimitPair, &librt_get_IterWithNextAndCounterAndLimitPair_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithNextAndUnpackFilter, &librt_get_IterWithNextAndUnpackFilter_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithNextKey, &librt_get_IterWithNextKey_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IterWithNextValue, &librt_get_IterWithNextValue_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqReversedWithGetItemIndex, &librt_get_SeqReversedWithGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqReversedWithGetItemIndexFast, &librt_get_SeqReversedWithGetItemIndexFast_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqReversedWithTryGetItemIndex, &librt_get_SeqReversedWithTryGetItemIndex_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DistinctIterator, &librt_get_DistinctIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DistinctIteratorWithKey, &librt_get_DistinctIteratorWithKey_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DistinctSetWithKey, &librt_get_DistinctSetWithKey_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DistinctMappingIterator, &librt_get_DistinctMappingIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_CachedSeqWithIter, &librt_get_CachedSeqWithIter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_CachedSeqWithIterIterator, &librt_get_CachedSeqWithIterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetInversion, &librt_get_SetInversion_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetUnion, &librt_get_SetUnion_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetUnionIterator, &librt_get_SetUnionIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetSymmetricDifference, &librt_get_SetSymmetricDifference_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetSymmetricDifferenceIterator, &librt_get_SetSymmetricDifferenceIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetIntersection, &librt_get_SetIntersection_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetIntersectionIterator, &librt_get_SetIntersectionIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetDifference, &librt_get_SetDifference_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SetDifferenceIterator, &librt_get_SetDifferenceIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapUnion, &librt_get_MapUnion_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapUnionIterator, &librt_get_MapUnionIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapSymmetricDifference, &librt_get_MapSymmetricDifference_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapSymmetricDifferenceIterator, &librt_get_MapSymmetricDifferenceIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapIntersection, &librt_get_MapIntersection_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapIntersectionIterator, &librt_get_MapIntersectionIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapDifference, &librt_get_MapDifference_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapDifferenceIterator, &librt_get_MapDifferenceIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SeqOneIterator, &librt_get_SeqOneIterator_Type_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ClassOperatorTable, &librt_get_ClassOperatorTable_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ClassOperatorTableIterator, &librt_get_ClassOperatorTableIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ClassAttribute, &librt_get_ClassAttribute_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ClassAttributeTable, &librt_get_ClassAttributeTable_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ClassAttributeTableIterator, &librt_get_ClassAttributeTableIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ObjectTable, &librt_get_ObjectTable_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TypeMRO, &librt_get_TypeMRO_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TypeMROIterator, &librt_get_TypeMROIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TypeBases, &librt_get_TypeBases_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TypeBasesIterator, &librt_get_TypeBasesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RoDictIterator, &librt_get_RoDictIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RoSetIterator, &librt_get_RoSetIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_KwdsIterator, &librt_get_KwdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_KwdsMappingIterator, &librt_get_KwdsMappingIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IteratorPending, &librt_get_IteratorPending_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_IteratorFuture, &librt_get_IteratorFuture_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringIterator, &librt_get_StringIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesIterator, &librt_get_BytesIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ListIterator, &librt_get_ListIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TupleIterator, &librt_get_TupleIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_HashSetIterator, &librt_get_HashSetIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DictIterator, &librt_get_DictIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TracebackIterator, &librt_get_TracebackIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_GCSet, &librt_get_GCSet_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_GCSetIterator, &librt_get_GCSetIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_GCSet_empty, &librt_get_GCSet_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_NullableTuple_empty, &librt_get_NullableTuple_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_Code_empty, &librt_get_Code_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BlackListKwdsIterator, &librt_get_BlackListKwdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BlackListKwIterator, &librt_get_BlackListKwIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_GCEnum, &librt_get_GCEnum_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_GCIter, &librt_get_GCIter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_Traceback_empty, &librt_get_Traceback_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_Module_empty, &librt_get_Module_empty_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DocKwds, &librt_get_DocKwds_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_DocKwdsIterator, &librt_get_DocKwdsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapHashFilter, &librt_get_MapHashFilter_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapHashFilterIterator, &librt_get_MapHashFilterIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapByAttr, &librt_get_MapByAttr_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapKeys, &librt_get_MapKeys_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapValues, &librt_get_MapValues_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapFromKeysAndValue, &librt_get_MapFromKeysAndValue_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapFromKeysAndCallback, &librt_get_MapFromKeysAndCallback_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapFromKeysAndValueIterator, &librt_get_MapFromKeysAndValueIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapFromKeysAndCallbackIterator, &librt_get_MapFromKeysAndCallbackIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapFromAttr, &librt_get_MapFromAttr_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_MapFromAttrKeysIterator, &librt_get_MapFromAttrKeysIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SharedVectorIterator, &librt_get_SharedVectorIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_SharedMapIterator, &librt_get_SharedMapIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RefVector, &librt_get_RefVector_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RefVectorIterator, &librt_get_RefVectorIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TypeOperators, &librt_get_TypeOperators_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_TypeOperatorsIterator, &librt_get_TypeOperatorsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ModuleExports, &librt_get_ModuleExports_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ModuleExportsIterator, &librt_get_ModuleExportsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ModuleGlobals, &librt_get_ModuleGlobals_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesFind, &librt_get_BytesFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesFindIterator, &librt_get_BytesFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesCaseFind, &librt_get_BytesCaseFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesCaseFindIterator, &librt_get_BytesCaseFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesSegments, &librt_get_BytesSegments_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesSegmentsIterator, &librt_get_BytesSegmentsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesSplit, &librt_get_BytesSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesSplitIterator, &librt_get_BytesSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesCaseSplit, &librt_get_BytesCaseSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesCaseSplitIterator, &librt_get_BytesCaseSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesLineSplit, &librt_get_BytesLineSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_BytesLineSplitIterator, &librt_get_BytesLineSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringFind, &librt_get_StringFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringFindIterator, &librt_get_StringFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringCaseFind, &librt_get_StringCaseFind_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringCaseFindIterator, &librt_get_StringCaseFindIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringSegments, &librt_get_StringSegments_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringSegmentsIterator, &librt_get_StringSegmentsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringSplit, &librt_get_StringSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringSplitIterator, &librt_get_StringSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringCaseSplit, &librt_get_StringCaseSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringCaseSplitIterator, &librt_get_StringCaseSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringLineSplit, &librt_get_StringLineSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringLineSplitIterator, &librt_get_StringLineSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringScan, &librt_get_StringScan_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringScanIterator, &librt_get_StringScanIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_StringOrdinals, &librt_get_StringOrdinals_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReFindAll, &librt_get_ReFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReFindAllIterator, &librt_get_ReFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegFindAll, &librt_get_RegFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegFindAllIterator, &librt_get_RegFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReLocateAll, &librt_get_ReLocateAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReLocateAllIterator, &librt_get_ReLocateAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegLocateAll, &librt_get_RegLocateAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegLocateAllIterator, &librt_get_RegLocateAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReSplit, &librt_get_ReSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReSplitIterator, &librt_get_ReSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReGroups, &librt_get_ReGroups_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReSubStrings, &librt_get_ReSubStrings_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReSubBytes, &librt_get_ReSubBytes_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReBytesFindAll, &librt_get_ReBytesFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReBytesFindAllIterator, &librt_get_ReBytesFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegBytesFindAll, &librt_get_RegBytesFindAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegBytesFindAllIterator, &librt_get_RegBytesFindAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReBytesLocateAll, &librt_get_ReBytesLocateAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReBytesLocateAllIterator, &librt_get_ReBytesLocateAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegBytesLocateAll, &librt_get_RegBytesLocateAll_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_RegBytesLocateAllIterator, &librt_get_RegBytesLocateAllIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReBytesSplit, &librt_get_ReBytesSplit_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_ReBytesSplitIterator, &librt_get_ReBytesSplitIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FunctionStatics, &librt_get_FunctionStatics_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FunctionStaticsIterator, &librt_get_FunctionStaticsIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FunctionSymbolsByName, &librt_get_FunctionSymbolsByName_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FunctionSymbolsByNameIterator, &librt_get_FunctionSymbolsByNameIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FunctionSymbolsByNameKeysIterator, &librt_get_FunctionSymbolsByNameKeysIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_YieldFunctionSymbolsByName, &librt_get_YieldFunctionSymbolsByName_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_YieldFunctionSymbolsByNameIterator, &librt_get_YieldFunctionSymbolsByNameIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_YieldFunctionSymbolsByNameKeysIterator, &librt_get_YieldFunctionSymbolsByNameKeysIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FrameArgs, &librt_get_FrameArgs_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FrameLocals, &librt_get_FrameLocals_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FrameStack, &librt_get_FrameStack_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FrameSymbolsByName, &librt_get_FrameSymbolsByName_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FrameSymbolsByNameIterator, &librt_get_FrameSymbolsByNameIterator_f, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD0(librt_get_FrameSymbolsByNameKeysIterator, &librt_get_FrameSymbolsByNameKeysIterator_f, METHOD_FCONSTCALL);


PRIVATE WUNUSED DREF DeeObject *DCALL
librt_argv_set_f(DeeObject *new_tuple) {
	size_t i;
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

PRIVATE DEFINE_CMETHOD0(librt_argv_get, &Dee_GetArgv, METHOD_FPURECALL);
PRIVATE DEFINE_CMETHOD1(librt_argv_set, &librt_argv_set_f, METHOD_FNORMAL);

PRIVATE DEFINE_CMETHOD1(librt_kw, &DeeKw_Wrap, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD1(librt_ctypes_addrof, &DeeCTypes_CreateVoidPointer, METHOD_FCONSTCALL);


/* Define some magic constants that may be of interest to user-code. */
/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "rt";

include("constants.def");
gi("HASHOF_EMPTY_SEQUENCE", "DEE_HASHOF_EMPTY_SEQUENCE");
gi("HASHOF_UNBOUND_ITEM", "DEE_HASHOF_UNBOUND_ITEM");
gi("HASHOF_RECURSIVE_ITEM", "DEE_HASHOF_RECURSIVE_ITEM");
]]]*/
#include "constants.def"
/*[[[end]]]*/






/* NOTE: At first glance, the combination of `MODSYM_FPROPERTY|MODSYM_FCONSTEXPR' may
 *       not look like it would make sense, but by using this combination, we prevent
 *       the symbols to be considered properties during enumeration (`Dee_ATTRPERM_F_PROPERTY'
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
	      /**/ "sub-routine within the deemon core, which may not be available "
	      /**/ "on any arbitrary architecture.") },

	{ "getcalloptimizethreshold", (DeeObject *)&librt_getcalloptimizethreshold, MODSYM_FREADONLY,
	  DOC("->?Dint\n"
	      "Get the threshold specifying how often a ?DFunction or ?DCode object "
	      /**/ "needs to be called before deemon will automatically try to optimize it.") },
	{ "setcalloptimizethreshold", (DeeObject *)&librt_setcalloptimizethreshold, MODSYM_FREADONLY,
	  DOC("(" librt_setcalloptimizethreshold_params ")->?Dint\n"
	      "#r{The old threshold}"
	      "Set the threshold specifying how often a ?DFunction or ?DCode object "
	      /**/ "needs to be called before deemon will automatically try to optimize it.") },

	{ "SlabStat", (DeeObject *)&SlabStat_Type, MODSYM_FREADONLY },             /* Access to slab allocator statistics. */
	{ "StringFiniHook", (DeeObject *)&StringFiniHook_Type, MODSYM_FREADONLY }, /* Definition of user-defined string finalization hooks. */
	{ "makeclass", (DeeObject *)&librt_makeclass, MODSYM_FREADONLY,
	  DOC("(" librt_makeclass_params ")->?DType\n"
	      "#pmodule{The module that is declaring the class (and returned by ${return.__module__}). "
	      /*    */ "When not given (or given as ?N), the type is not linked to a module.}"
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
	{ "SeqCombinationsView", (DeeObject *)&librt_get_SeqCombinationsView, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* SeqCombinationsView_Type */

	{ "SeqSegments", (DeeObject *)&librt_get_SeqSegments, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* SeqSegments_Type */
	{ "SeqSegmentsIterator", (DeeObject *)&librt_get_SeqSegmentsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* SeqSegmentsIterator_Type */
	{ "SeqConcat", (DeeObject *)&librt_get_SeqConcat, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* SeqConcat_Type */
	{ "SeqConcatIterator", (DeeObject *)&librt_get_SeqConcatIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqConcatIterator_Type */
	{ "SeqFilter", (DeeObject *)&librt_get_SeqFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* SeqFilter_Type */
	{ "SeqFilterAsUnbound", (DeeObject *)&librt_get_SeqFilterAsUnbound, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* SeqFilterAsUnbound_Type */
	{ "SeqFilterIterator", (DeeObject *)&librt_get_SeqFilterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqFilterIterator_Type */
	{ "SeqHashFilter", (DeeObject *)&librt_get_SeqHashFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* SeqHashFilter_Type */
	{ "SeqHashFilterIterator", (DeeObject *)&librt_get_SeqHashFilterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* SeqHashFilterIterator_Type */
	{ "SeqMapped", (DeeObject *)&librt_get_SeqMapped, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* SeqMapped_Type */
	{ "SeqMappedIterator", (DeeObject *)&librt_get_SeqMappedIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqMappedIterator_Type */
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
	{ "SeqFlat", (DeeObject *)&librt_get_SeqFlat, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* SeqFlat_Type */
	{ "SeqFlatIterator", (DeeObject *)&librt_get_SeqFlatIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqFlatIterator_Type */

	/* Seq-each wrapper types. */
	{ "SeqEach", (DeeObject *)&librt_get_SeqEach, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* SeqEach_Type */
	{ "SeqEachOperator", (DeeObject *)&librt_get_SeqEachOperator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqEachOperator_Type */
	{ "SeqEachOperatorIterator", (DeeObject *)&librt_get_SeqEachOperatorIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* SeqEachOperatorIterator_Type */
	{ "SeqEachGetAttr_np", (DeeObject *)&librt_get_SeqEachGetAttr_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqEachGetAttr_Type */
	{ "SeqEachGetAttrIterator_np", (DeeObject *)&librt_get_SeqEachGetAttrIterator_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* SeqEachGetAttrIterator_Type */
	{ "SeqEachCallAttr_np", (DeeObject *)&librt_get_SeqEachCallAttr_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* SeqEachCallAttr_Type */
	{ "SeqEachCallAttrIterator_np", (DeeObject *)&librt_get_SeqEachCallAttrIterator_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* SeqEachCallAttrIterator_Type */
	{ "SeqEachCallAttrKw_np", (DeeObject *)&librt_get_SeqEachCallAttrKw_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* SeqEachCallAttrKw_Type */
	{ "SeqEachCallAttrKwIterator_np", (DeeObject *)&librt_get_SeqEachCallAttrKwIterator_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* SeqEachCallAttrKwIterator_Type */

	/* Seq-some wrapper types. */
	{ "SeqSome", (DeeObject *)&DeeSeqSome_Type, MODSYM_FREADONLY },
	{ "SeqSomeOperator", (DeeObject *)&librt_get_SeqSomeOperator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SeqSomeOperator_Type */
	{ "SeqSomeGetAttr_np", (DeeObject *)&librt_get_SeqSomeGetAttr_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* SeqSomeGetAttr_Type */
	{ "SeqSomeCallAttr_np", (DeeObject *)&librt_get_SeqSomeCallAttr_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* SeqSomeCallAttr_Type */
	{ "SeqSomeCallAttrKw_np", (DeeObject *)&librt_get_SeqSomeCallAttrKw_np, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* SeqSomeCallAttrKw_Type */

	/* Default enumeration types */
	{ "SeqEnumWithSeqOperatorSizeAndGetItemIndexFast", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorSizeAndGetItemIndexFast, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast */
	{ "SeqEnumWithIntFilterAndSeqOperatorSizeAndGetItemIndexFast", (DeeObject *)&librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndGetItemIndexFast, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast */
	{ "SeqEnumWithSeqOperatorSizeAndSeqOperatorTryGetItemIndex", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorTryGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index */
	{ "SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorTryGetItemIndex", (DeeObject *)&librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorTryGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index */
	{ "SeqEnumWithSeqOperatorSizeAndSeqOperatorGetItemIndex", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorSizeAndSeqOperatorGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index */
	{ "SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorGetItemIndex", (DeeObject *)&librt_get_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index */
	{ "SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem */
	{ "SeqEnumWithSeqOperatorGetItemIndex", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                                   /* DefaultEnumeration__with__seq_operator_getitem_index */
	{ "SeqEnumWithIntFilterAndSeqOperatorGetItemIndex", (DeeObject *)&librt_get_SeqEnumWithIntFilterAndSeqOperatorGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index */
	{ "SeqEnumWithFilterAndSeqOperatorSizeobAndSeqOperatorGetItem", (DeeObject *)&librt_get_SeqEnumWithFilterAndSeqOperatorSizeobAndSeqOperatorGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem */
	{ "SeqEnumWithSeqOperatorGetItem", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                                             /* DefaultEnumeration__with__seq_operator_getitem */
	{ "SeqEnumWithFilterAndSeqOperatorGetItem", (DeeObject *)&librt_get_SeqEnumWithFilterAndSeqOperatorGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                           /* DefaultEnumerationWithFilter__with__seq_operator_getitem */
	{ "SeqEnumWithSeqOperatorIterAndCounter", (DeeObject *)&librt_get_SeqEnumWithSeqOperatorIterAndCounter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                               /* DefaultEnumeration__with__seq_operator_iter__and__counter */
	{ "SeqEnumWithIntFilterAndSeqOperatorIterAndCounter", (DeeObject *)&librt_get_SeqEnumWithIntFilterAndSeqOperatorIterAndCounter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                       /* DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter */
	{ "SeqEnumWithSeqEnumerate", (DeeObject *)&librt_get_SeqEnumWithSeqEnumerate, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                                                         /* DefaultEnumeration__with__seq_enumerate */
	{ "SeqEnumWithIntFilterAndSeqEnumerateIndex", (DeeObject *)&librt_get_SeqEnumWithIntFilterAndSeqEnumerateIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                       /* DefaultEnumerationWithIntFilter__with__seq_enumerate_index */
	{ "SeqEnumWithFilterAndMapOperatorIterAndUnpack", (DeeObject *)&librt_get_SeqEnumWithFilterAndMapOperatorIterAndUnpack, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                               /* DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack */
	{ "SeqEnumWithMapIterkeysAndMapOperatorGetItem", (DeeObject *)&librt_get_SeqEnumWithMapIterkeysAndMapOperatorGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                 /* DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem */
	{ "SeqEnumWithFilterAndMapIterkeysAndMapOperatorGetItem", (DeeObject *)&librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem */
	{ "SeqEnumWithMapIterkeysAndMapOperatorTryGetItem", (DeeObject *)&librt_get_SeqEnumWithMapIterkeysAndMapOperatorTryGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem */
	{ "SeqEnumWithFilterAndMapIterkeysAndMapOperatorTryGetItem", (DeeObject *)&librt_get_SeqEnumWithFilterAndMapIterkeysAndMapOperatorTryGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem */
	{ "SeqEnumWithMapEnumerate", (DeeObject *)&librt_get_SeqEnumWithMapEnumerate, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                                                         /* DefaultEnumeration__with__map_enumerate */
	{ "SeqEnumWithFilterAndMapEnumerateRange", (DeeObject *)&librt_get_SeqEnumWithFilterAndMapEnumerateRange, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                             /* DefaultEnumerationWithFilter__with__map_enumerate_range */

	/* Default sequence types */
	{ "SeqWithSizeAndGetItemIndex", (DeeObject *)&librt_get_SeqWithSizeAndGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* DefaultSequence_WithSizeAndGetItemIndex_Type */
	{ "SeqWithSizeAndGetItemIndexFast", (DeeObject *)&librt_get_SeqWithSizeAndGetItemIndexFast, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* DefaultSequence_WithSizeAndGetItemIndexFast_Type */
	{ "SeqWithSizeAndTryGetItemIndex", (DeeObject *)&librt_get_SeqWithSizeAndTryGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* DefaultSequence_WithSizeAndTryGetItemIndex_Type */
	{ "SeqWithSizeObAndGetItem", (DeeObject *)&librt_get_SeqWithSizeAndGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* DefaultSequence_WithSizeObAndGetItem_Type */
	{ "SeqWithIter", (DeeObject *)&librt_get_SeqWithIter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                   /* DefaultSequence_WithIter_Type */
	{ "SeqWithIterAndLimit", (DeeObject *)&librt_get_SeqWithIterAndLimit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* DefaultSequence_WithIterAndLimit_Type */

	/* Special sequence types */
	{ "SeqOne", (DeeObject *)&DeeSeqOne_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },
	{ "SeqOneIterator", (DeeObject *)&librt_get_SeqOneIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* SeqOneIterator_Type */

	/* Misc. helper types for sequences */
	{ "SeqEnumerateWrapper", (DeeObject *)&librt_get_EnumerateWrapper, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* SeqEnumerateWrapper_Type */
	{ "SeqEnumerateIndexWrapper", (DeeObject *)&librt_get_EnumerateIndexWrapper, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* SeqEnumerateIndexWrapper_Type */
//TODO:	{ "SeqRemoveWithRemoveIfPredicate", (DeeObject *)&librt_get_SeqRemoveWithRemoveIfPredicate, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* SeqRemoveWithRemoveIfPredicate_Type */
//TODO:	{ "SeqRemoveWithRemoveIfPredicateWithKey", (DeeObject *)&librt_get_SeqRemoveWithRemoveIfPredicateWithKey, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* SeqRemoveWithRemoveIfPredicateWithKey_Type */
//TODO:	{ "SeqRemoveIfWithRemoveAllItem", (DeeObject *)&librt_get_SeqRemoveIfWithRemoveAllItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* SeqRemoveIfWithRemoveAllItem_Type */
//TODO:	{ "SeqRemoveIfWithRemoveAllKey", (DeeObject *)&librt_get_SeqRemoveIfWithRemoveAllKey, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* SeqRemoveIfWithRemoveAllKey_Type */
//TODO:	{ "SeqRemoveIfWithRemoveAllItem_DummyInstance", (DeeObject *)&librt_get_SeqRemoveIfWithRemoveAllItem_DummyInstance, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* SeqRemoveIfWithRemoveAllItem_DummyInstance */

	/* Default iterator types */
	{ "IterWithGetItemIndex", (DeeObject *)&librt_get_IterWithGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* DefaultIterator_WithGetItemIndex_Type */
	{ "IterWithGetItemIndexPair", (DeeObject *)&librt_get_IterWithGetItemIndexPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* DefaultIterator_WithGetItemIndexPair_Type */
	{ "IterWithSizeAndGetItemIndex", (DeeObject *)&librt_get_IterWithSizeAndGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* DefaultIterator_WithSizeAndGetItemIndex_Type */
	{ "IterWithSizeAndGetItemIndexPair", (DeeObject *)&librt_get_IterWithSizeAndGetItemIndexPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DefaultIterator_WithSizeAndGetItemIndexPair_Type */
	{ "IterWithSizeAndGetItemIndexFast", (DeeObject *)&librt_get_IterWithSizeAndGetItemIndexFast, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DefaultIterator_WithSizeAndGetItemIndexFast_Type */
	{ "IterWithSizeAndGetItemIndexFastPair", (DeeObject *)&librt_get_IterWithSizeAndGetItemIndexFastPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* DefaultIterator_WithSizeAndGetItemIndexFastPair_Type */
	{ "IterWithSizeAndTryGetItemIndex", (DeeObject *)&librt_get_IterWithSizeAndTryGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* DefaultIterator_WithSizeAndTryGetItemIndex_Type */
	{ "IterWithSizeAndTryGetItemIndexPair", (DeeObject *)&librt_get_IterWithSizeAndTryGetItemIndexPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },     /* DefaultIterator_WithSizeAndTryGetItemIndexPair_Type */
	{ "IterWithGetItem", (DeeObject *)&librt_get_IterWithGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* DefaultIterator_WithGetItem_Type */
	{ "IterWithGetItemPair", (DeeObject *)&librt_get_IterWithGetItemPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* DefaultIterator_WithGetItemPair_Type */
	{ "IterWithSizeObAndGetItem", (DeeObject *)&librt_get_IterWithSizeObAndGetItem, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* DefaultIterator_WithSizeObAndGetItem_Type */
	{ "IterWithSizeAndGetItemPair", (DeeObject *)&librt_get_IterWithSizeObAndGetItemPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* DefaultIterator_WithSizeObAndGetItemPair_Type */
	{ "IterWithNextAndLimit", (DeeObject *)&librt_get_IterWithNextAndLimit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* DefaultIterator_WithNextAndLimit_Type */
	{ "IterWithIterKeysAndGetItemForMap", (DeeObject *)&librt_get_IterWithIterKeysAndGetItemForMap, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* DefaultIterator_WithIterKeysAndGetItemMap_Type */
	{ "IterWithIterKeysAndTryGetItemForMap", (DeeObject *)&librt_get_IterWithIterKeysAndTryGetItemForMap, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* DefaultIterator_WithIterKeysAndTryGetItemMap_Type */
	{ "IterWithForeach", (DeeObject *)&librt_get_IterWithForeach, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* DefaultIterator_WithForeach_Type */
	{ "IterWithForeachPair", (DeeObject *)&librt_get_IterWithForeachPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* DefaultIterator_WithForeachPair_Type */
	{ "IterWithEnumerateMap", (DeeObject *)&librt_get_IterWithEnumerateMap, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* DefaultIterator_WithEnumerateMap_Type */
	{ "IterWithEnumerateIndexSeq", (DeeObject *)&librt_get_IterWithEnumerateIndexSeq, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* DefaultIterator_WithEnumerateIndexSeq_Type */
	{ "IterWithEnumerateSeq", (DeeObject *)&librt_get_IterWithEnumerateSeq, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* DefaultIterator_WithEnumerateSeq_Type */
	{ "IterWithNextAndCounterPair", (DeeObject *)&librt_get_IterWithNextAndCounterPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* DefaultIterator_WithNextAndCounterPair_Type */
	{ "IterWithNextAndCounterAndLimitPair", (DeeObject *)&librt_get_IterWithNextAndCounterAndLimitPair, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },     /* DefaultIterator_WithNextAndCounterAndLimitPair_Type */
	{ "IterWithNextAndUnpackFilter", (DeeObject *)&librt_get_IterWithNextAndUnpackFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* DefaultIterator_WithNextAndUnpackFilter_Type */
	{ "IterWithNextKey", (DeeObject *)&librt_get_IterWithNextKey, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                           /* DefaultIterator_WithNextKey */
	{ "IterWithNextValue", (DeeObject *)&librt_get_IterWithNextValue, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                       /* DefaultIterator_WithNextValue */

	/* Default types for `Sequence.reversed()' */
	{ "SeqReversedWithGetItemIndex", (DeeObject *)&librt_get_SeqReversedWithGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* DefaultReversed_WithGetItemIndex_Type */
	{ "SeqReversedWithGetItemIndexFast", (DeeObject *)&librt_get_SeqReversedWithGetItemIndexFast, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DefaultReversed_WithGetItemIndexFast_Type */
	{ "SeqReversedWithTryGetItemIndex", (DeeObject *)&librt_get_SeqReversedWithTryGetItemIndex, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },             /* DefaultReversed_WithTryGetItemIndex_Type */

	/* Default types for `Sequence.distinct()' */
	{ "DistinctIterator", (DeeObject *)&librt_get_DistinctIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                         /* DistinctIterator_Type */
	{ "DistinctIteratorWithKey", (DeeObject *)&librt_get_DistinctIteratorWithKey, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* DistinctIteratorWithKey_Type */
	{ "DistinctSetWithKey", (DeeObject *)&librt_get_DistinctSetWithKey, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* DistinctSetWithKey_Type */
	{ "DistinctMappingIterator", (DeeObject *)&librt_get_DistinctMappingIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* DistinctMappingIterator_Type */

	/* Default sequence cache types */
	{ "CachedSeqWithIter", (DeeObject *)&librt_get_CachedSeqWithIter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                       /* CachedSeq_WithIter_Type */
	{ "CachedSeqWithIterIterator", (DeeObject *)&librt_get_CachedSeqWithIterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* CachedSeq_WithIter_Iterator_Type */

	/* Internal types used to drive set proxies */
	{ "SetInversion", (DeeObject *)&librt_get_SetInversion, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                     /* SetInversion_Type */
	{ "SetUnion", (DeeObject *)&librt_get_SetUnion, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* SetUnion_Type */
	{ "SetUnionIterator", (DeeObject *)&librt_get_SetUnionIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* SetUnionIterator_Type */
	{ "SetSymmetricDifference", (DeeObject *)&librt_get_SetSymmetricDifference, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* SetSymmetricDifference_Type */
	{ "SetSymmetricDifferenceIterator", (DeeObject *)&librt_get_SetSymmetricDifferenceIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* SetSymmetricDifferenceIterator_Type */
	{ "SetIntersection", (DeeObject *)&librt_get_SetIntersection, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* SetIntersection_Type */
	{ "SetIntersectionIterator", (DeeObject *)&librt_get_SetIntersectionIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* SetIntersectionIterator_Type */
	{ "SetDifference", (DeeObject *)&librt_get_SetDifference, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* SetDifference_Type */
	{ "SetDifferenceIterator", (DeeObject *)&librt_get_SetDifferenceIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* SetDifferenceIterator_Type */

	/* Internal types used to drive map proxies */
	{ "MapUnion", (DeeObject *)&librt_get_MapUnion, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                             /* MapUnion_Type */
	{ "MapUnionIterator", (DeeObject *)&librt_get_MapUnionIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* MapUnionIterator_Type */
	{ "MapSymmetricDifference", (DeeObject *)&librt_get_MapSymmetricDifference, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* MapSymmetricDifference_Type */
	{ "MapSymmetricDifferenceIterator", (DeeObject *)&librt_get_MapSymmetricDifferenceIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* MapSymmetricDifferenceIterator_Type */
	{ "MapIntersection", (DeeObject *)&librt_get_MapIntersection, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                               /* MapIntersection_Type */
	{ "MapIntersectionIterator", (DeeObject *)&librt_get_MapIntersectionIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* MapIntersectionIterator_Type */
	{ "MapDifference", (DeeObject *)&librt_get_MapDifference, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* MapDifference_Type */
	{ "MapDifferenceIterator", (DeeObject *)&librt_get_MapDifferenceIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* MapDifferenceIterator_Type */

	/* Internal types used to drive mapping proxies */
	{ "MapHashFilter", (DeeObject *)&librt_get_MapHashFilter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* MapHashFilter_Type */
	{ "MapHashFilterIterator", (DeeObject *)&librt_get_MapHashFilterIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* MapHashFilterIterator_Type */
	{ "MapByAttr", (DeeObject *)&librt_get_MapByAttr, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* MapByAttr_Type */
	{ "MapKeys", (DeeObject *)&librt_get_MapKeys, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* DefaultSequence_MapKeys_Type */
	{ "MapValues", (DeeObject *)&librt_get_MapValues, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* DefaultSequence_MapValues_Type */

	/* Internal types used to implement "Mapping.fromkeys" */
	{ "MapFromKeysAndValue", (DeeObject *)&librt_get_MapFromKeysAndValue, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* MapFromKeysAndValue_Type */
	{ "MapFromKeysAndCallback", (DeeObject *)&librt_get_MapFromKeysAndCallback, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* MapFromKeysAndCallback_Type */
	{ "MapFromKeysAndValueIterator", (DeeObject *)&librt_get_MapFromKeysAndValueIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* MapFromKeysAndValueIterator_Type */
	{ "MapFromKeysAndCallbackIterator", (DeeObject *)&librt_get_MapFromKeysAndCallbackIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* MapFromKeysAndCallbackIterator_Type */

	/* Internal types used to implement "Mapping.fromattr" */
	{ "MapFromAttr", (DeeObject *)&librt_get_MapFromAttr, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* MapFromAttr_Type */
	{ "MapFromAttrKeysIterator", (DeeObject *)&librt_get_MapFromAttrKeysIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* MapFromAttrKeysIterator_Type */

	/* The special "nullable" tuple sequence type. */
	{ "NullableTuple", (DeeObject *)&DeeNullableTuple_Type, MODSYM_FREADONLY },

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
	{ "RegLocateAll", (DeeObject *)&librt_get_RegLocateAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* RegLocateAll_Type */
	{ "RegLocateAllIterator", (DeeObject *)&librt_get_RegLocateAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* RegLocateAllIterator_Type */
	{ "ReSplit", (DeeObject *)&librt_get_ReSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                   /* ReSplit_Type */
	{ "ReSplitIterator", (DeeObject *)&librt_get_ReSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* ReSplitIterator_Type */
	{ "ReGroups", (DeeObject *)&librt_get_ReGroups, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* ReGroups_Type */
	{ "ReSubStrings", (DeeObject *)&librt_get_ReSubStrings, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* ReSubStrings_Type */
	{ "ReSubBytes", (DeeObject *)&librt_get_ReSubBytes, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                             /* ReSubBytes_Type */
	{ "ReBytesFindAll", (DeeObject *)&librt_get_ReBytesFindAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                     /* ReBytesFindAll_Type */
	{ "ReBytesFindAllIterator", (DeeObject *)&librt_get_ReBytesFindAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },     /* ReBytesFindAllIterator_Type */
	{ "RegBytesFindAll", (DeeObject *)&librt_get_RegBytesFindAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                   /* RegBytesFindAll_Type */
	{ "RegBytesFindAllIterator", (DeeObject *)&librt_get_RegBytesFindAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* RegBytesFindAllIterator_Type */
	{ "ReBytesLocateAll", (DeeObject *)&librt_get_ReBytesLocateAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ReBytesLocateAll_Type */
	{ "ReBytesLocateAllIterator", (DeeObject *)&librt_get_ReBytesLocateAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* ReBytesLocateAllIterator_Type */
	{ "RegBytesLocateAll", (DeeObject *)&librt_get_RegBytesLocateAll, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },               /* RegBytesLocateAll_Type */
	{ "RegBytesLocateAllIterator", (DeeObject *)&librt_get_RegBytesLocateAllIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },/* RegBytesLocateAllIterator_Type */
	{ "ReBytesSplit", (DeeObject *)&librt_get_ReBytesSplit, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                         /* ReBytesSplit_Type */
	{ "ReBytesSplitIterator", (DeeObject *)&librt_get_ReBytesSplitIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* ReBytesSplitIterator_Type */

	/* Internal types used to drive module symbol table inspection */
	{ "ModuleExports", (DeeObject *)&librt_get_ModuleExports, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ModuleExports_Type */
	{ "ModuleExportsIterator", (DeeObject *)&librt_get_ModuleExportsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* ModuleExportsIterator_Type */
	{ "ModuleGlobals", (DeeObject *)&librt_get_ModuleGlobals, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* ModuleGlobals_Type */

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
	  DOC("C-variant of ?GFunction taking a variable number of arguments (e.g. ${boundattr from deemon})") },
	{ "CMethod0", (DeeObject *)&DeeCMethod0_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GFunction that does not take any arguments") },
	{ "CMethod1", (DeeObject *)&DeeCMethod1_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GFunction that takes exactly 1 argument") },
	{ "KwCMethod", (DeeObject *)&DeeKwCMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GFunction (with keyword support)") },
	{ "ObjMethod", (DeeObject *)&DeeObjMethod_Type, MODSYM_FREADONLY,
	  DOC("C-variant of ?GInstanceMethod (e.g. ${\"FOO\".lower})") },
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
	{ "IteratorPending", (DeeObject *)&librt_get_IteratorPending, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* IteratorPending_Type */
	{ "IteratorFuture", (DeeObject *)&librt_get_IteratorFuture, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* IteratorFuture_Type */

	/* Internal iterator types used to drive builtin sequence objects */
	{ "StringIterator", (DeeObject *)&librt_get_StringIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },       /* StringIterator_Type */
	{ "BytesIterator", (DeeObject *)&librt_get_BytesIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* BytesIterator_Type */
	{ "ListIterator", (DeeObject *)&librt_get_ListIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DeeListIterator_Type */
	{ "TupleIterator", (DeeObject *)&librt_get_TupleIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* DeeTupleIterator_Type */
	{ "HashSetIterator", (DeeObject *)&librt_get_HashSetIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },     /* HashSetIterator_Type */
	{ "TracebackIterator", (DeeObject *)&librt_get_TracebackIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* DeeTracebackIterator_Type */
	{ "DictIterator", (DeeObject *)&librt_get_DictIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* DictIterator_Type */

	/* Special instances of non-singleton objects */
	{ "Sequence_empty", Dee_EmptySeq, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, empty sequence singleton") },
	{ "Set_empty", (DeeObject *)Dee_EmptySet, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, empty set singleton") },
	{ "Set_universal", (DeeObject *)Dee_UniversalSet, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, universal set singleton") },
	{ "Mapping_empty", (DeeObject *)Dee_EmptyMapping, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("A general-purpose, empty mapping singleton") },
	{ "RoDict_empty", (DeeObject *)Dee_EmptyRoDict, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("An empty instance of ?GRoDict") },
	{ "Tuple_empty", (DeeObject *)Dee_EmptyTuple, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The empty tuple singleton ${()}") },
	{ "String_empty", (DeeObject *)Dee_EmptyString, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The empty string singleton $\"\"") },
	{ "Bytes_empty", (DeeObject *)Dee_EmptyBytes, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The empty bytes singleton ${\"\".bytes()}") },
	{ "Int_0", DeeInt_Zero, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The integer constant $0") },
	{ "Int_1", DeeInt_One, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The integer constant $1") },
	{ "Int_m1", DeeInt_MinusOne, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, DOC("The integer constant ${-1}") },
	{ "NullableTuple_empty", (DeeObject *)&librt_get_NullableTuple_empty, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("The empty nullable-tuple singleton") },
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
	{ "GCIter", (DeeObject *)&librt_get_GCIter, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR,
	  DOC("The result of ${type((gc from deemon)operator iter())}") }, /* GCIter_Type */
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

	/* Function wrapper types */
	{ "FunctionStatics", (DeeObject *)&librt_get_FunctionStatics, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                       /* FunctionStatics_Type */
	{ "FunctionStaticsIterator", (DeeObject *)&librt_get_FunctionStaticsIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                       /* FunctionStatics_Type */
	{ "FunctionSymbolsByName", (DeeObject *)&librt_get_FunctionSymbolsByName, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                           /* FunctionSymbolsByName_Type */
	{ "FunctionSymbolsByNameIterator", (DeeObject *)&librt_get_FunctionSymbolsByNameIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },           /* FunctionSymbolsByName_Type */
	{ "FunctionSymbolsByNameKeysIterator", (DeeObject *)&librt_get_FunctionSymbolsByNameKeysIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },   /* FunctionSymbolsByNameKeysIterator_Type */
	{ "YieldFunctionSymbolsByName", (DeeObject *)&librt_get_YieldFunctionSymbolsByName, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* YieldFunctionSymbolsByName_Type */
	{ "YieldFunctionSymbolsByNameIterator", (DeeObject *)&librt_get_YieldFunctionSymbolsByNameIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* YieldFunctionSymbolsByName_Type */
	{ "YieldFunctionSymbolsByNameKeysIterator", (DeeObject *)&librt_get_YieldFunctionSymbolsByNameKeysIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR }, /* YieldFunctionSymbolsByNameKeysIterator_Type */
	{ "FrameArgs", (DeeObject *)&librt_get_FrameArgs, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                   /* FrameArgs_Type */
	{ "FrameLocals", (DeeObject *)&librt_get_FrameLocals, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                               /* FrameLocals_Type */
	{ "FrameStack", (DeeObject *)&librt_get_FrameStack, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                                 /* FrameStack_Type */
	{ "FrameSymbolsByName", (DeeObject *)&librt_get_FrameSymbolsByName, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                                 /* FrameSymbolsByName_Type */
	{ "FrameSymbolsByNameIterator", (DeeObject *)&librt_get_FrameSymbolsByNameIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },                 /* FrameSymbolsByName_Type */
	{ "FrameSymbolsByNameKeysIterator", (DeeObject *)&librt_get_FrameSymbolsByNameKeysIterator, MODSYM_FREADONLY | MODSYM_FPROPERTY | MODSYM_FCONSTEXPR },         /* FrameSymbolsByNameKeysIterator_Type */

	/* Some more aliases... */
	{ "gc", (DeeObject *)&DeeGCEnumTracked_Singleton, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },
	{ "enumattr", (DeeObject *)&DeeEnumAttr_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },
	{ "bool", (DeeObject *)&DeeBool_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },
	{ "string", (DeeObject *)&DeeString_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },
	{ "int", (DeeObject *)&DeeInt_Type, MODSYM_FREADONLY | MODSYM_FCONSTEXPR },

	/* Special constants */
	RT_HASHOF_EMPTY_SEQUENCE_DEF
	RT_HASHOF_UNBOUND_ITEM_DEF
	RT_HASHOF_RECURSIVE_ITEM_DEF

	{ "ctypes_addrof", (DeeObject *)&librt_ctypes_addrof, MODSYM_FREADONLY,
	  DOC("(ob)->?Aptr?Ectypes:void\n"
	      "Returns the object address of @ob") },
	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END

#endif /* !GUARD_DEX_RT_LIBRT_C */
