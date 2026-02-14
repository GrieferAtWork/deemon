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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>          /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/error.h>          /* DeeError_* */
#include <deemon/gc.h>             /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK */
#include <deemon/int.h>            /* DeeInt_NewZero, _DeeInt_Zero */
#include <deemon/method-hints.h>   /* DeeObject_InvokeMethodHint, DeeType_RequireMethodHint */
#include <deemon/object.h>         /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_Decref, Dee_Incref, Dee_Incref_n, Dee_TYPE, Dee_visit_t, OBJECT_HEAD_INIT */
#include <deemon/operator-hints.h> /* DeeType_RequireNativeOperator */
#include <deemon/seq.h>            /* DeeSeq_NewEmpty, DeeSeq_Type */
#include <deemon/serial.h>         /* DeeSerial, DeeSerial_Addr2Mem, Dee_seraddr_t */
#include <deemon/type.h>           /* DeeObject_Init, DeeType_GetName, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_MEMBER*, type_member, type_seq */
#include <deemon/util/lock.h>      /* Dee_atomic_lock_init */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-enumerate.h"
#include "default-iterators.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

/* After adding/remocing stuff to this [[[impls]]]-block, run:
 * >> deemon -F src/deemon/objects/seq/default-enumerate.c */
/*[[[begin:impls]]]*/

/* common: DefaultEnumeration */
STATIC_ASSERT(offsetof(DefaultEnumeration, de_seq) == offsetof(ProxyObject, po_obj));
#define DefaultEnumeration__init      generic_proxy__init
#define DefaultEnumeration__serialize generic_proxy__serialize
#define DefaultEnumeration__fini      generic_proxy__fini
#define DefaultEnumeration__visit     generic_proxy__visit
#define DefaultEnumeration__copy      generic_proxy__copy_alias

PRIVATE WUNUSED NONNULL((1)) int DCALL
DefaultEnumeration__ctor(DefaultEnumeration *__restrict self) {
	self->de_seq = DeeSeq_NewEmpty();
	return 0;
}


/* common: DefaultEnumerationWithFilter */
STATIC_ASSERT(offsetof(DefaultEnumerationWithFilter, dewf_seq) == offsetof(ProxyObject3, po_obj1));
STATIC_ASSERT(offsetof(DefaultEnumerationWithFilter, dewf_start) == offsetof(ProxyObject3, po_obj2));
STATIC_ASSERT(offsetof(DefaultEnumerationWithFilter, dewf_end) == offsetof(ProxyObject3, po_obj3));
#define DefaultEnumerationWithFilter__init      generic_proxy3__init
#define DefaultEnumerationWithFilter__serialize generic_proxy3__serialize
#define DefaultEnumerationWithFilter__fini      generic_proxy3__fini
#define DefaultEnumerationWithFilter__visit     generic_proxy3__visit
#define DefaultEnumerationWithFilter__copy      generic_proxy3__copy_alias123
STATIC_ASSERT(offsetof(DefaultEnumeration, de_seq) == offsetof(DefaultEnumerationWithFilter, dewf_seq));
PRIVATE struct type_member tpconst DefaultEnumerationWithFilter__members[] = {
	TYPE_MEMBER_FIELD("__start__", STRUCT_OBJECT_AB, offsetof(DefaultEnumerationWithFilter, dewf_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_OBJECT_AB, offsetof(DefaultEnumerationWithFilter, dewf_end)),
#define DefaultEnumeration__members (DefaultEnumerationWithFilter__members + 2)
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT_AB, offsetof(DefaultEnumerationWithFilter, dewf_seq)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
DefaultEnumerationWithFilter__ctor(DefaultEnumerationWithFilter *__restrict self) {
	self->dewf_seq = DeeSeq_NewEmpty();
	Dee_Incref_n(_DeeInt_Zero, 2);
	self->dewf_start = (DeeObject *)_DeeInt_Zero;
	self->dewf_end   = (DeeObject *)_DeeInt_Zero;
	return 0;
}


/* common: DefaultEnumerationWithIntFilter */
STATIC_ASSERT(offsetof(DefaultEnumerationWithIntFilter, dewif_seq) == offsetof(ProxyObject, po_obj));
#define DefaultEnumerationWithIntFilter__fini  generic_proxy__fini
#define DefaultEnumerationWithIntFilter__visit generic_proxy__visit
PRIVATE struct type_member tpconst DefaultEnumerationWithIntFilter__members[] = {
	TYPE_MEMBER_FIELD("__start__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultEnumerationWithIntFilter, dewif_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultEnumerationWithIntFilter, dewif_end)),
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT_AB, offsetof(DefaultEnumerationWithIntFilter, dewif_seq)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
DefaultEnumerationWithIntFilter__init(DefaultEnumerationWithIntFilter *__restrict self,
                                      size_t argc, DeeObject *const *argv) {
	char const *tp_name;
	if likely(argc == 3) {
		if (DeeObject_AsSize(argv[1], &self->dewif_start))
			goto err;
		if (DeeObject_AsSize(argv[2], &self->dewif_end))
			goto err;
		self->dewif_seq = argv[0];
		Dee_Incref(self->dewif_seq);
		return 0;
	}
	tp_name = DeeType_GetName(Dee_TYPE(self));
	err_invalid_argc(tp_name, argc, 3, 3);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DefaultEnumerationWithIntFilter__copy(DefaultEnumerationWithIntFilter *__restrict self,
                                      DefaultEnumerationWithIntFilter *__restrict other) {
	self->dewif_start = other->dewif_start;
	self->dewif_end   = other->dewif_end;
	return generic_proxy__copy_alias((ProxyObject *)self, (ProxyObject *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DefaultEnumerationWithIntFilter__serialize(DefaultEnumerationWithIntFilter *__restrict self,
                                           DeeSerial *__restrict writer,
                                           Dee_seraddr_t addr) {
	DefaultEnumerationWithIntFilter *out;
	out = DeeSerial_Addr2Mem(writer, addr, DefaultEnumerationWithIntFilter);
	out->dewif_start = self->dewif_start;
	out->dewif_end   = self->dewif_end;
	return generic_proxy__serialize((ProxyObject *)self, writer, addr);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DefaultEnumerationWithIntFilter__ctor(DefaultEnumerationWithIntFilter *__restrict self) {
	self->dewif_seq   = DeeSeq_NewEmpty();
	self->dewif_start = 0;
	self->dewif_end   = 0;
	return 0;
}


/* Constructors/destructors */
/*[[[deemon
import * from deemon;
global final NAMESPACES: {string: string} = {
	"de_sos_gif__":      "DefaultEnumeration__",
	"dewif_sos_gif__":   "DefaultEnumerationWithIntFilter__",
	"de_sos_sotgi__":    "DefaultEnumeration__",
	"dewif_sos_sotgi__": "DefaultEnumerationWithIntFilter__",
	"de_sos_sogi__":     "DefaultEnumeration__",
	"dewif_sos_soii__":  "DefaultEnumerationWithIntFilter__",
	"de_soso_sog__":     "DefaultEnumeration__",
	"de_sogi__":         "DefaultEnumeration__",
	"dewif_soii__":      "DefaultEnumerationWithIntFilter__",
	"dewf_soso_sog__":   "DefaultEnumerationWithFilter__",
	"de_sog__":          "DefaultEnumeration__",
	"dewf_sog__":        "DefaultEnumerationWithFilter__",
	"de_soi__":          "DefaultEnumeration__",
	"dewif_soi__":       "DefaultEnumerationWithIntFilter__",
	"de_se__":           "DefaultEnumeration__",
	"dewif_sei__":       "DefaultEnumerationWithIntFilter__",

	"dewf_moi__":        "DefaultEnumerationWithFilter__",
	"de_mik_mog__":      "DefaultEnumeration__",
	"dewf_mik_mog__":    "DefaultEnumerationWithFilter__",
	"de_mik_motg__":     "DefaultEnumeration__",
	"dewf_mik_motg__":   "DefaultEnumerationWithFilter__",
	"de_me__":           "DefaultEnumeration__",
	"dewf_mer__":        "DefaultEnumerationWithFilter__",
};
function printPrefixAliases(name: string) {
	print("/" "* Common: ", name, " *" "/");
	local longestNamespaceName = (NAMESPACES.keys.each.length > ...) + #name;
	for (local prefix, typePrefix: NAMESPACES)
		print("#define ", (prefix + name).ljust(longestNamespaceName), " ", typePrefix, name);
	print;
}
printPrefixAliases("ctor");
printPrefixAliases("init");
printPrefixAliases("copy");
printPrefixAliases("serialize");
printPrefixAliases("fini");
printPrefixAliases("visit");
printPrefixAliases("members");
]]]*/
/* Common: ctor */
#define de_sos_gif__ctor      DefaultEnumeration__ctor
#define dewif_sos_gif__ctor   DefaultEnumerationWithIntFilter__ctor
#define de_sos_sotgi__ctor    DefaultEnumeration__ctor
#define dewif_sos_sotgi__ctor DefaultEnumerationWithIntFilter__ctor
#define de_sos_sogi__ctor     DefaultEnumeration__ctor
#define dewif_sos_soii__ctor  DefaultEnumerationWithIntFilter__ctor
#define de_soso_sog__ctor     DefaultEnumeration__ctor
#define de_sogi__ctor         DefaultEnumeration__ctor
#define dewif_soii__ctor      DefaultEnumerationWithIntFilter__ctor
#define dewf_soso_sog__ctor   DefaultEnumerationWithFilter__ctor
#define de_sog__ctor          DefaultEnumeration__ctor
#define dewf_sog__ctor        DefaultEnumerationWithFilter__ctor
#define de_soi__ctor          DefaultEnumeration__ctor
#define dewif_soi__ctor       DefaultEnumerationWithIntFilter__ctor
#define de_se__ctor           DefaultEnumeration__ctor
#define dewif_sei__ctor       DefaultEnumerationWithIntFilter__ctor
#define dewf_moi__ctor        DefaultEnumerationWithFilter__ctor
#define de_mik_mog__ctor      DefaultEnumeration__ctor
#define dewf_mik_mog__ctor    DefaultEnumerationWithFilter__ctor
#define de_mik_motg__ctor     DefaultEnumeration__ctor
#define dewf_mik_motg__ctor   DefaultEnumerationWithFilter__ctor
#define de_me__ctor           DefaultEnumeration__ctor
#define dewf_mer__ctor        DefaultEnumerationWithFilter__ctor

/* Common: init */
#define de_sos_gif__init      DefaultEnumeration__init
#define dewif_sos_gif__init   DefaultEnumerationWithIntFilter__init
#define de_sos_sotgi__init    DefaultEnumeration__init
#define dewif_sos_sotgi__init DefaultEnumerationWithIntFilter__init
#define de_sos_sogi__init     DefaultEnumeration__init
#define dewif_sos_soii__init  DefaultEnumerationWithIntFilter__init
#define de_soso_sog__init     DefaultEnumeration__init
#define de_sogi__init         DefaultEnumeration__init
#define dewif_soii__init      DefaultEnumerationWithIntFilter__init
#define dewf_soso_sog__init   DefaultEnumerationWithFilter__init
#define de_sog__init          DefaultEnumeration__init
#define dewf_sog__init        DefaultEnumerationWithFilter__init
#define de_soi__init          DefaultEnumeration__init
#define dewif_soi__init       DefaultEnumerationWithIntFilter__init
#define de_se__init           DefaultEnumeration__init
#define dewif_sei__init       DefaultEnumerationWithIntFilter__init
#define dewf_moi__init        DefaultEnumerationWithFilter__init
#define de_mik_mog__init      DefaultEnumeration__init
#define dewf_mik_mog__init    DefaultEnumerationWithFilter__init
#define de_mik_motg__init     DefaultEnumeration__init
#define dewf_mik_motg__init   DefaultEnumerationWithFilter__init
#define de_me__init           DefaultEnumeration__init
#define dewf_mer__init        DefaultEnumerationWithFilter__init

/* Common: copy */
#define de_sos_gif__copy      DefaultEnumeration__copy
#define dewif_sos_gif__copy   DefaultEnumerationWithIntFilter__copy
#define de_sos_sotgi__copy    DefaultEnumeration__copy
#define dewif_sos_sotgi__copy DefaultEnumerationWithIntFilter__copy
#define de_sos_sogi__copy     DefaultEnumeration__copy
#define dewif_sos_soii__copy  DefaultEnumerationWithIntFilter__copy
#define de_soso_sog__copy     DefaultEnumeration__copy
#define de_sogi__copy         DefaultEnumeration__copy
#define dewif_soii__copy      DefaultEnumerationWithIntFilter__copy
#define dewf_soso_sog__copy   DefaultEnumerationWithFilter__copy
#define de_sog__copy          DefaultEnumeration__copy
#define dewf_sog__copy        DefaultEnumerationWithFilter__copy
#define de_soi__copy          DefaultEnumeration__copy
#define dewif_soi__copy       DefaultEnumerationWithIntFilter__copy
#define de_se__copy           DefaultEnumeration__copy
#define dewif_sei__copy       DefaultEnumerationWithIntFilter__copy
#define dewf_moi__copy        DefaultEnumerationWithFilter__copy
#define de_mik_mog__copy      DefaultEnumeration__copy
#define dewf_mik_mog__copy    DefaultEnumerationWithFilter__copy
#define de_mik_motg__copy     DefaultEnumeration__copy
#define dewf_mik_motg__copy   DefaultEnumerationWithFilter__copy
#define de_me__copy           DefaultEnumeration__copy
#define dewf_mer__copy        DefaultEnumerationWithFilter__copy

/* Common: serialize */
#define de_sos_gif__serialize      DefaultEnumeration__serialize
#define dewif_sos_gif__serialize   DefaultEnumerationWithIntFilter__serialize
#define de_sos_sotgi__serialize    DefaultEnumeration__serialize
#define dewif_sos_sotgi__serialize DefaultEnumerationWithIntFilter__serialize
#define de_sos_sogi__serialize     DefaultEnumeration__serialize
#define dewif_sos_soii__serialize  DefaultEnumerationWithIntFilter__serialize
#define de_soso_sog__serialize     DefaultEnumeration__serialize
#define de_sogi__serialize         DefaultEnumeration__serialize
#define dewif_soii__serialize      DefaultEnumerationWithIntFilter__serialize
#define dewf_soso_sog__serialize   DefaultEnumerationWithFilter__serialize
#define de_sog__serialize          DefaultEnumeration__serialize
#define dewf_sog__serialize        DefaultEnumerationWithFilter__serialize
#define de_soi__serialize          DefaultEnumeration__serialize
#define dewif_soi__serialize       DefaultEnumerationWithIntFilter__serialize
#define de_se__serialize           DefaultEnumeration__serialize
#define dewif_sei__serialize       DefaultEnumerationWithIntFilter__serialize
#define dewf_moi__serialize        DefaultEnumerationWithFilter__serialize
#define de_mik_mog__serialize      DefaultEnumeration__serialize
#define dewf_mik_mog__serialize    DefaultEnumerationWithFilter__serialize
#define de_mik_motg__serialize     DefaultEnumeration__serialize
#define dewf_mik_motg__serialize   DefaultEnumerationWithFilter__serialize
#define de_me__serialize           DefaultEnumeration__serialize
#define dewf_mer__serialize        DefaultEnumerationWithFilter__serialize

/* Common: fini */
#define de_sos_gif__fini      DefaultEnumeration__fini
#define dewif_sos_gif__fini   DefaultEnumerationWithIntFilter__fini
#define de_sos_sotgi__fini    DefaultEnumeration__fini
#define dewif_sos_sotgi__fini DefaultEnumerationWithIntFilter__fini
#define de_sos_sogi__fini     DefaultEnumeration__fini
#define dewif_sos_soii__fini  DefaultEnumerationWithIntFilter__fini
#define de_soso_sog__fini     DefaultEnumeration__fini
#define de_sogi__fini         DefaultEnumeration__fini
#define dewif_soii__fini      DefaultEnumerationWithIntFilter__fini
#define dewf_soso_sog__fini   DefaultEnumerationWithFilter__fini
#define de_sog__fini          DefaultEnumeration__fini
#define dewf_sog__fini        DefaultEnumerationWithFilter__fini
#define de_soi__fini          DefaultEnumeration__fini
#define dewif_soi__fini       DefaultEnumerationWithIntFilter__fini
#define de_se__fini           DefaultEnumeration__fini
#define dewif_sei__fini       DefaultEnumerationWithIntFilter__fini
#define dewf_moi__fini        DefaultEnumerationWithFilter__fini
#define de_mik_mog__fini      DefaultEnumeration__fini
#define dewf_mik_mog__fini    DefaultEnumerationWithFilter__fini
#define de_mik_motg__fini     DefaultEnumeration__fini
#define dewf_mik_motg__fini   DefaultEnumerationWithFilter__fini
#define de_me__fini           DefaultEnumeration__fini
#define dewf_mer__fini        DefaultEnumerationWithFilter__fini

/* Common: visit */
#define de_sos_gif__visit      DefaultEnumeration__visit
#define dewif_sos_gif__visit   DefaultEnumerationWithIntFilter__visit
#define de_sos_sotgi__visit    DefaultEnumeration__visit
#define dewif_sos_sotgi__visit DefaultEnumerationWithIntFilter__visit
#define de_sos_sogi__visit     DefaultEnumeration__visit
#define dewif_sos_soii__visit  DefaultEnumerationWithIntFilter__visit
#define de_soso_sog__visit     DefaultEnumeration__visit
#define de_sogi__visit         DefaultEnumeration__visit
#define dewif_soii__visit      DefaultEnumerationWithIntFilter__visit
#define dewf_soso_sog__visit   DefaultEnumerationWithFilter__visit
#define de_sog__visit          DefaultEnumeration__visit
#define dewf_sog__visit        DefaultEnumerationWithFilter__visit
#define de_soi__visit          DefaultEnumeration__visit
#define dewif_soi__visit       DefaultEnumerationWithIntFilter__visit
#define de_se__visit           DefaultEnumeration__visit
#define dewif_sei__visit       DefaultEnumerationWithIntFilter__visit
#define dewf_moi__visit        DefaultEnumerationWithFilter__visit
#define de_mik_mog__visit      DefaultEnumeration__visit
#define dewf_mik_mog__visit    DefaultEnumerationWithFilter__visit
#define de_mik_motg__visit     DefaultEnumeration__visit
#define dewf_mik_motg__visit   DefaultEnumerationWithFilter__visit
#define de_me__visit           DefaultEnumeration__visit
#define dewf_mer__visit        DefaultEnumerationWithFilter__visit

/* Common: members */
#define de_sos_gif__members      DefaultEnumeration__members
#define dewif_sos_gif__members   DefaultEnumerationWithIntFilter__members
#define de_sos_sotgi__members    DefaultEnumeration__members
#define dewif_sos_sotgi__members DefaultEnumerationWithIntFilter__members
#define de_sos_sogi__members     DefaultEnumeration__members
#define dewif_sos_soii__members  DefaultEnumerationWithIntFilter__members
#define de_soso_sog__members     DefaultEnumeration__members
#define de_sogi__members         DefaultEnumeration__members
#define dewif_soii__members      DefaultEnumerationWithIntFilter__members
#define dewf_soso_sog__members   DefaultEnumerationWithFilter__members
#define de_sog__members          DefaultEnumeration__members
#define dewf_sog__members        DefaultEnumerationWithFilter__members
#define de_soi__members          DefaultEnumeration__members
#define dewif_soi__members       DefaultEnumerationWithIntFilter__members
#define de_se__members           DefaultEnumeration__members
#define dewif_sei__members       DefaultEnumerationWithIntFilter__members
#define dewf_moi__members        DefaultEnumerationWithFilter__members
#define de_mik_mog__members      DefaultEnumeration__members
#define dewf_mik_mog__members    DefaultEnumerationWithFilter__members
#define de_mik_motg__members     DefaultEnumeration__members
#define dewf_mik_motg__members   DefaultEnumerationWithFilter__members
#define de_me__members           DefaultEnumeration__members
#define dewf_mer__members        DefaultEnumerationWithFilter__members
/*[[[end]]]*/


/* Type names */
#define de_sos_gif__name      "_SeqEnumWithSeqOperatorSizeAndGetItemIndexFast"
#define dewif_sos_gif__name   "_SeqEnumWithIntFilterAndSeqOperatorSizeAndGetItemIndexFast"
#define de_sos_sotgi__name    "_SeqEnumWithSeqOperatorSizeAndSeqOperatorTryGetItemIndex"
#define dewif_sos_sotgi__name "_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorTryGetItemIndex"
#define de_sos_sogi__name     "_SeqEnumWithSeqOperatorSizeAndSeqOperatorGetItemIndex"
#define dewif_sos_soii__name  "_SeqEnumWithIntFilterAndSeqOperatorSizeAndSeqOperatorGetItemIndex"
#define de_soso_sog__name     "_SeqEnumWithSeqOperatorSizeobAndSeqOperatorGetItem"
#define de_sogi__name         "_SeqEnumWithSeqOperatorGetItemIndex"
#define dewif_soii__name      "_SeqEnumWithIntFilterAndSeqOperatorGetItemIndex"
#define dewf_soso_sog__name   "_SeqEnumWithFilterAndSeqOperatorSizeobAndSeqOperatorGetItem"
#define de_sog__name          "_SeqEnumWithSeqOperatorGetItem"
#define dewf_sog__name        "_SeqEnumWithFilterAndSeqOperatorGetItem"
#define de_soi__name          "_SeqEnumWithSeqOperatorIterAndCounter"
#define dewif_soi__name       "_SeqEnumWithIntFilterAndSeqOperatorIterAndCounter"
#define de_se__name           "_SeqEnumWithSeqEnumerate"
#define dewif_sei__name       "_SeqEnumWithIntFilterAndSeqEnumerateIndex"

#define dewf_moi__name        "_SeqEnumWithFilterAndMapOperatorIterAndUnpack"
#define de_mik_mog__name      "_SeqEnumWithMapIterkeysAndMapOperatorGetItem"
#define dewf_mik_mog__name    "_SeqEnumWithFilterAndMapIterkeysAndMapOperatorGetItem"
#define de_mik_motg__name     "_SeqEnumWithMapIterkeysAndMapOperatorTryGetItem"
#define dewf_mik_motg__name   "_SeqEnumWithFilterAndMapIterkeysAndMapOperatorTryGetItem"
#define de_me__name           "_SeqEnumWithMapEnumerate"
#define dewf_mer__name        "_SeqEnumWithFilterAndMapEnumerateRange"

/* Doc strings */
#define de_sos_gif__doc      "(obj_with____seq_size____and__tp_getitem_index_fast)"
#define dewif_sos_gif__doc   "(obj_with____seq_size____and__tp_getitem_index_fast,start:Dint,end:Dint)"
#define de_sos_sotgi__doc    "(obj_with____seq_size____and____seq_getitem__)"
#define dewif_sos_sotgi__doc "(obj_with____seq_size____and____seq_getitem__,start:Dint,end:Dint)"
#define de_sos_sogi__doc     "(obj_with____seq_size____and____seq_getitem__)"
#define dewif_sos_soii__doc  "(obj_with____seq_size____and____seq_getitem__,start:Dint,end:Dint)"
#define de_soso_sog__doc     "(obj_with____seq_size____and____seq_getitem__)"
#define de_sogi__doc         "(obj_with____seq_getitem__)"
#define dewif_soii__doc      "(obj_with____seq_getitem__,start:Dint,end:Dint)"
#define dewf_soso_sog__doc   "(obj_with____seq_size____and____seq_getitem__,start:?X2?Dint?O,end:?X2?Dint?O)"
#define de_sog__doc          "(obj_with____seq_getitem__)"
#define dewf_sog__doc        "(obj_with____seq_getitem__,start:?X2?Dint?O,end:?X2?Dint?O)"
#define de_soi__doc          "(obj_with____seq_iter__)"
#define dewif_soi__doc       "(obj_with____seq_iter__,start:Dint,end:Dint)"
#define de_se__doc           "(obj_with____seq_enumerate__)"
#define dewif_sei__doc       "(obj_with____seq_enumerate__,start:Dint,end:Dint)"

#define dewf_moi__doc        "(obj_with____set_iter__,start:?X2?Dint?O,end:?X2?Dint?O)"
#define de_mik_mog__doc      "(obj_with____map_iterkeys____and____map_getitem__)"
#define dewf_mik_mog__doc    "(obj_with____map_iterkeys____and____map_getitem__,start:?X2?Dint?O,end:?X2?Dint?O)"
#define de_mik_motg__doc     "(obj_with____map_iterkeys____and____map_getitem__)"
#define dewf_mik_motg__doc   "(obj_with____map_iterkeys____and____map_getitem__,start:?X2?Dint?O,end:?X2?Dint?O)"
#define de_me__doc           "(obj_with____map_enumerate__)"
#define dewf_mer__doc        "(obj_with____map_enumerate__,start:?X2?Dint?O,end:?X2?Dint?O)"



/* Special case for the constructors of "*_getitem_index_fast" variants:
 * Assert presensence of the tp_getitem_index_fast operator. */
#undef de_sos_gif__init
#undef dewif_sos_gif__init
PRIVATE ATTR_CONST NONNULL((1)) int DCALL
err_no_getitem_index_fast(DeeTypeObject *__restrict type) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Type '%r' does not support the tp_getitem_index_fast interface",
	                       type);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
de_sos_gif__init(DefaultEnumeration *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	int result = DefaultEnumeration__init((ProxyObject *)self, argc, argv);
	if likely(result == 0) {
		DeeTypeObject *tp_seq = Dee_TYPE(self->de_seq);
		if unlikely(!tp_seq->tp_seq)
			goto err_self_no_getitem_index_fast;
		if unlikely(!tp_seq->tp_seq->tp_getitem_index_fast)
			goto err_self_no_getitem_index_fast;
	}
	return result;
err_self_no_getitem_index_fast:
	result = err_no_getitem_index_fast(Dee_TYPE(self->de_seq));
	DefaultEnumeration__fini((ProxyObject *)self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dewif_sos_gif__init(DefaultEnumerationWithIntFilter *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	int result = DefaultEnumerationWithIntFilter__init(self, argc, argv);
	if likely(result == 0) {
		DeeTypeObject *tp_seq = Dee_TYPE(self->dewif_seq);
		if unlikely(!tp_seq->tp_seq)
			goto err_self_no_getitem_index_fast;
		if unlikely(!tp_seq->tp_seq->tp_getitem_index_fast)
			goto err_self_no_getitem_index_fast;
	}
	return result;
err_self_no_getitem_index_fast:
	result = err_no_getitem_index_fast(Dee_TYPE(self->dewif_seq));
	DefaultEnumerationWithIntFilter__fini((ProxyObject *)self);
	return result;
}








/************************************************************************/
/* For Sequence                                                         */
/************************************************************************/

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithSizeAndGetItemIndexFastPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithSizeAndTryGetItemIndexPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithSizeAndGetItemIndexPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndexPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithSizeObAndGetItemPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeObAndGetItemPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithGetItemIndexPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithGetItemIndexPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithGetItemPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithGetItemPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithNextAndCounterPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndCounterPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithNextAndCounterAndLimitPair_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndCounterAndLimitPair_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithNextAndUnpackFilter_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndUnpackFilter_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithIterKeysAndGetItemMap_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithIterKeysAndGetItemMap_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst Iterator__is__DefaultIterator_WithIterKeysAndTryGetItemMap_Type[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type),
	TYPE_MEMBER_END
};



/* $with__seq_operator_size__and__operator_getitem_index_fast */
#define de_sos_gif__class_members Iterator__is__DefaultIterator_WithSizeAndGetItemIndexFastPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_sos_gif__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->de_seq;
	ASSERT(Dee_TYPE(self->de_seq)->tp_seq);
	result->disgi_tp_getitem_index = Dee_TYPE(self->de_seq)->tp_seq->tp_getitem_index_fast;
	result->disgi_index = 0;
	result->disgi_end   = DeeObject_InvokeMethodHint(seq_operator_size, self->de_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	ASSERT(result->disgi_tp_getitem_index || !result->disgi_end);
	Dee_Incref(self->de_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* WithIntFilter: $with__seq_operator_size__and__operator_getitem_index_fast */
#define dewif_sos_gif__class_members Iterator__is__DefaultIterator_WithSizeAndGetItemIndexFastPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
dewif_sos_gif__iter(DefaultEnumerationWithIntFilter *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->dewif_seq;
	ASSERT(Dee_TYPE(self->dewif_seq)->tp_seq);
	result->disgi_tp_getitem_index = Dee_TYPE(self->dewif_seq)->tp_seq->tp_getitem_index_fast;
	result->disgi_index = self->dewif_start;
	result->disgi_end   = DeeObject_InvokeMethodHint(seq_operator_size, self->dewif_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	if (result->disgi_end > self->dewif_end)
		result->disgi_end = self->dewif_end;
	ASSERT(result->disgi_tp_getitem_index || !result->disgi_end);
	Dee_Incref(self->dewif_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFastPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* $with__seq_operator_size__and__seq_operator_trygetitem_index */
#define de_sos_sotgi__class_members Iterator__is__DefaultIterator_WithSizeAndTryGetItemIndexPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_sos_sotgi__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->de_seq;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_operator_trygetitem_index);
	result->disgi_index = 0;
	result->disgi_end   = DeeObject_InvokeMethodHint(seq_operator_size, self->de_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	Dee_Incref(self->de_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* WithIntFilter: $with__seq_operator_size__and__seq_operator_trygetitem_index */
#define dewif_sos_sotgi__class_members Iterator__is__DefaultIterator_WithSizeAndTryGetItemIndexPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
dewif_sos_sotgi__iter(DefaultEnumerationWithIntFilter *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->dewif_seq;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self->dewif_seq), seq_operator_trygetitem_index);
	result->disgi_index = self->dewif_start;
	result->disgi_end   = DeeObject_InvokeMethodHint(seq_operator_size, self->dewif_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	if (result->disgi_end > self->dewif_end)
		result->disgi_end = self->dewif_end;
	Dee_Incref(self->dewif_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* $with__seq_operator_size__and__seq_operator_getitem_index */
#define de_sos_sogi__class_members Iterator__is__DefaultIterator_WithSizeAndGetItemIndexPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
de_sos_sogi__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->de_seq;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_operator_getitem_index);
	result->disgi_index = 0;
	result->disgi_end   = DeeObject_InvokeMethodHint(seq_operator_size, self->de_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	Dee_Incref(self->de_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* WithIntFilter: $with__seq_operator_size__and__seq_operator_getitem_index */
#define dewif_sos_soii__class_members Iterator__is__DefaultIterator_WithSizeAndGetItemIndexPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
dewif_sos_soii__iter(DefaultEnumerationWithIntFilter *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->dewif_seq;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self->dewif_seq), seq_operator_getitem_index);
	result->disgi_index = self->dewif_start;
	result->disgi_end   = DeeObject_InvokeMethodHint(seq_operator_size, self->dewif_seq);
	if unlikely(result->disgi_end == (size_t)-1)
		goto err_r;
	if (result->disgi_end > self->dewif_end)
		result->disgi_end = self->dewif_end;
	Dee_Incref(self->dewif_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* $with__seq_operator_sizeob__and__seq_operator_getitem */
#define de_soso_sog__class_members Iterator__is__DefaultIterator_WithSizeObAndGetItemPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
de_soso_sog__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	result->disg_end = DeeObject_InvokeMethodHint(seq_operator_sizeob, self->de_seq);
	if unlikely(!result->disg_end)
		goto err_r;
	result->disg_index = DeeObject_NewDefault(Dee_TYPE(result->disg_end));
	if unlikely(!result->disg_index)
		goto err_r_end;
	result->disg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_operator_getitem);
	Dee_Incref(self->de_seq);
	result->disg_seq = self->de_seq;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItemPair_Type);
	return DeeGC_TRACK(DefaultIterator_WithSizeObAndGetItem, result);
err_r_end:
	Dee_Decref(result->disg_end);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}




/* $with__seq_operator_getitem_index */
#define de_sogi__class_members Iterator__is__DefaultIterator_WithGetItemIndexPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithGetItemIndex *DCALL
de_sogi__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->digi_index = 0;
	result->digi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_operator_getitem_index);
	Dee_Incref(self->de_seq);
	result->digi_seq = self->de_seq;
	DeeObject_Init(result, &DefaultIterator_WithGetItemIndexPair_Type);
	return result;
err:
	return NULL;
}




/* WithIntFilter: $with__seq_operator_getitem_index */
#define dewif_soii__class_members Iterator__is__DefaultIterator_WithSizeAndGetItemIndexPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
dewif_soii__iter(DefaultEnumerationWithIntFilter *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	result->disgi_seq = self->dewif_seq;
	result->disgi_tp_getitem_index = DeeType_RequireMethodHint(Dee_TYPE(self->dewif_seq), seq_operator_getitem_index);
	result->disgi_index = self->dewif_start;
	result->disgi_end   = self->dewif_end;
	Dee_Incref(self->dewif_seq);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexPair_Type);
	return result;
err:
	return NULL;
}




/* WithFilter: $with__seq_operator_sizeob__and__seq_operator_getitem */
#define dewf_soso_sog__class_members Iterator__is__DefaultIterator_WithSizeObAndGetItemPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
dewf_soso_sog__iter(DefaultEnumerationWithFilter *__restrict self) {
	int temp;
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	result->disg_end = DeeObject_InvokeMethodHint(seq_operator_sizeob, self->dewf_seq);
	if unlikely(!result->disg_end)
		goto err_r;
	temp = DeeObject_CmpLoAsBool(self->dewf_end, result->disg_end);
	if (temp != 0) {
		if unlikely(temp < 0)
			goto err_r_end;
		Dee_Incref(self->dewf_end);
		Dee_Decref(result->disg_end);
		result->disg_end = self->dewf_end;
	}
	Dee_Incref(self->dewf_start);
	result->disg_index = self->dewf_start;
	result->disg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self->dewf_seq), seq_operator_getitem);
	Dee_Incref(self->dewf_seq);
	result->disg_seq = self->dewf_seq;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItemPair_Type);
	return DeeGC_TRACK(DefaultIterator_WithSizeObAndGetItem, result);
err_r_end:
	Dee_Decref(result->disg_end);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}




/* $with__seq_operator_getitem */
#define de_sog__class_members Iterator__is__DefaultIterator_WithGetItemPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithGetItem *DCALL
de_sog__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithGetItem);
	if unlikely(!result)
		goto err;
	result->dig_index      = DeeInt_NewZero();
	result->dig_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_operator_getitem);
	Dee_Incref(self->de_seq);
	result->dig_seq = self->de_seq;
	Dee_atomic_lock_init(&result->dig_lock);
	DeeObject_Init(result, &DefaultIterator_WithGetItemPair_Type);
	return DeeGC_TRACK(DefaultIterator_WithGetItem, result);
err:
	return NULL;
}




/* WithFilter: $with__seq_operator_getitem */
#define dewf_sog__class_members Iterator__is__DefaultIterator_WithSizeObAndGetItemPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
dewf_sog__iter(DefaultEnumerationWithFilter *__restrict self) {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dewf_seq);
	result->disg_seq = self->dewf_seq;
	result->disg_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self->dewf_seq), seq_operator_getitem);
	Dee_Incref(self->dewf_start);
	result->disg_index = self->dewf_start;
	Dee_Incref(self->dewf_end);
	result->disg_end = self->dewf_end;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItemPair_Type);
	return DeeGC_TRACK(DefaultIterator_WithSizeObAndGetItem, result);
err:
	return NULL;
}




/* $with__seq_operator_iter__and__counter */
#define de_soi__class_members Iterator__is__DefaultIterator_WithNextAndCounterPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithNextAndCounter *DCALL
de_soi__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithNextAndCounter *result;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndCounter);
	if unlikely(!result)
		goto err;
	result->dinc_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->de_seq);
	if unlikely(!result->dinc_iter)
		goto err_r;
	result->dinc_tp_next = DeeType_RequireNativeOperator(Dee_TYPE(result->dinc_iter), iter_next);
	result->dinc_counter = 0;
	DeeObject_Init(result, &DefaultIterator_WithNextAndCounterPair_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* WithIntFilter: $with__seq_operator_iter__and__counter */
#define dewif_soi__class_members Iterator__is__DefaultIterator_WithNextAndCounterAndLimitPair_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithNextAndCounterAndLimit *DCALL
dewif_soi__iter(DefaultEnumerationWithIntFilter *__restrict self) {
	DREF DefaultIterator_WithNextAndCounterAndLimit *result;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndCounterAndLimit);
	if unlikely(!result)
		goto err;
	result->dincl_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->dewif_seq);
	if unlikely(!result->dincl_iter)
		goto err_r;
	result->dincl_tp_next = DeeType_RequireNativeOperator(Dee_TYPE(result->dincl_iter), iter_next);
	if (DeeObject_IterAdvance(result->dincl_iter, self->dewif_start) == (size_t)-1)
		goto err_r_iter;
	result->dincl_counter = self->dewif_start;
	result->dincl_limit   = self->dewif_end;
	DeeObject_Init(result, &DefaultIterator_WithNextAndCounterAndLimitPair_Type);
	return result;
err_r_iter:
	Dee_Decref(result->dincl_iter);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* $with__seq_enumerate */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_se__iter(DefaultEnumeration *__restrict self) {
	/* TODO: Custom iterator type that uses:
	 * DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_enumerate) */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}




/* WithIntFilter: $with__seq_enumerate_index */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dewif_sei__iter(DefaultEnumerationWithIntFilter *__restrict self) {
	/* TODO: Custom iterator type that uses:
	 * DeeType_RequireMethodHint(Dee_TYPE(self->de_seq), seq_enumerate) */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}








/************************************************************************/
/* For Mapping                                                          */
/************************************************************************/

/* WithFilter: $with__map_operator_iter__and__unpack */
#define dewf_moi__class_members Iterator__is__DefaultIterator_WithNextAndUnpackFilter_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithNextAndUnpackFilter *DCALL
dewf_moi__iter(DefaultEnumerationWithFilter *__restrict self) {
	DREF DefaultIterator_WithNextAndUnpackFilter *result;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndUnpackFilter);
	if unlikely(!result)
		goto err;
	result->dinuf_iter = DeeObject_InvokeMethodHint(map_operator_iter, self->dewf_seq);
	if unlikely(!result->dinuf_iter)
		goto err_r;
	result->dinuf_tp_next = DeeType_RequireNativeOperator(Dee_TYPE(result->dinuf_iter), iter_next);
	Dee_Incref(self->dewf_start);
	result->dinuf_start = self->dewf_start;
	Dee_Incref(self->dewf_end);
	result->dinuf_end = self->dewf_end;
	DeeObject_Init(result, &DefaultIterator_WithNextAndUnpackFilter_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* $with__map_iterkeys__and__map_operator_getitem */
#define de_mik_mog__class_members Iterator__is__DefaultIterator_WithIterKeysAndGetItemMap_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithIterKeysAndGetItem *DCALL
de_mik_mog__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = DeeObject_InvokeMethodHint(map_iterkeys, self->de_seq);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	result->diikgi_tp_next    = DeeType_RequireNativeOperator(Dee_TYPE(result->diikgi_iter), iter_next);
	result->diikgi_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem);
	Dee_Incref(self->de_seq);
	result->diikgi_seq = self->de_seq;
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndGetItemMap_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* WithFilter: $with__map_iterkeys__and__map_operator_getitem */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dewf_mik_mog__iter(DefaultEnumerationWithFilter *__restrict self) {
	/* TODO: Custom iterator type: `DefaultIterator_WithIterKeysAndGetItemAndUnpackFilter' */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}




/* $with__map_iterkeys__and__map_operator_trygetitem */
#define de_mik_motg__class_members Iterator__is__DefaultIterator_WithIterKeysAndTryGetItemMap_Type
PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithIterKeysAndGetItem *DCALL
de_mik_motg__iter(DefaultEnumeration *__restrict self) {
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = DeeObject_InvokeMethodHint(map_iterkeys, self->de_seq);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	result->diikgi_tp_next    = DeeType_RequireNativeOperator(Dee_TYPE(result->diikgi_iter), iter_next);
	result->diikgi_tp_getitem = DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_trygetitem);
	Dee_Incref(self->de_seq);
	result->diikgi_seq = self->de_seq;
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}




/* WithFilter: $with__map_iterkeys__and__map_operator_trygetitem */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dewf_mik_motg__iter(DefaultEnumerationWithFilter *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}




/* $with__map_enumerate */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
de_me__iter(DefaultEnumeration *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}




/* WithFilter: $with__map_enumerate */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dewf_mer__iter(DefaultEnumerationWithFilter *__restrict self) {
	/* TODO: Custom iterator type */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}


/*[[[end:impls]]]*/



/*[[[deemon
import * from deemon;
final class EnumType {
	@@DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast
	public member typename: string;
	@@de_sos_gif__
	public member symbolPrefix: string;
	@@\bde_sos_gif__[[:symcont:]]+\b
	public member symbolPrefixRegex: string;
	public member symbols: {string...} = HashSet();

	this(typename: string, symbolPrefix: string) {
		this.typename = typename;
		this.symbolPrefix = symbolPrefix;
		this.symbolPrefixRegex = f"\\b{symbolPrefix}[[:symcont:]]+\\b";
	}

	public getField(name: string, def: string = "NULL"): string {
		if (name in symbols)
			return symbolPrefix + name;
		return def;
	}

	public getFieldPtr(name: string, def: string = "NULL"): string {
		if (name in symbols)
			return "&" + symbolPrefix + name;
		return def;
	}

	public getFieldTypedPtr(name: string, typ: string, def: string = "NULL"): string {
		if (name in symbols)
			return typ + "&" + symbolPrefix + name;
		return def;
	}

	public doPrintSubStruct(name: string, fieldsAndTypes: {string: string}): bool {
		for (local field: fieldsAndTypes.keys) {
			if (field in symbols)
				goto do_print_substruct;
		}
		return false;
do_print_substruct:
		print("PRIVATE struct type_", name, " ", symbolPrefix, name, " = {");
		local longestFieldName: int = fieldsAndTypes.keys.each.length > ...;
		for (local field, fieldType: fieldsAndTypes) {
			print("	/" "* .tp_", field.ljust(longestFieldName), " = *" "/ ",
				getFieldTypedPtr(field, fieldType), ",");
		}
		print("};");
		print;
		return true;
	}

	public printSubStruct(name: string, fieldsAndTypes: {string: string}): bool {
		if (name in symbols)
			return true;
		local present = doPrintSubStruct(name, fieldsAndTypes);
		if (present)
			symbols.insert(name);
		return present;
	}

	public printType() {
		print("/" "* ", typename, " *" "/");
		printSubStruct("seq", {
			'iter': '(DREF DeeObject *(DCALL *)(DeeObject *__restrict))',
			'sizeob': 'DREF DeeObject *(DCALL *)(DeeObject *__restrict))',
			'contains': 'DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))',
			'getitem': 'DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))',
			'delitem': 'int (DCALL *)(DeeObject *, DeeObject *))',
			'setitem': 'int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))',
			'getrange': 'DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))',
			'delrange': 'int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))',
			'setrange': 'int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))',
			'foreach': 'Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))',
			'foreach_pair': 'Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))',
			'bounditem': 'int (DCALL *)(DeeObject *, DeeObject *))',
			'hasitem': 'int (DCALL *)(DeeObject *, DeeObject *))',
			'size': 'size_t (DCALL *)(DeeObject *__restrict))',
			'size_fast': 'size_t (DCALL *)(DeeObject *__restrict))',
			'getitem_index': 'DREF DeeObject *(DCALL *)(DeeObject *, size_t))',
			'getitem_index_fast': 'DREF DeeObject *(DCALL *)(DeeObject *, size_t))',
			'delitem_index': 'int (DCALL *)(DeeObject *, size_t))',
			'setitem_index': 'int (DCALL *)(DeeObject *, size_t, DeeObject *))',
			'bounditem_index': 'int (DCALL *)(DeeObject *, size_t))',
			'hasitem_index': 'int (DCALL *)(DeeObject *, size_t))',
			'getrange_index': 'DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))',
			'delrange_index': 'int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))',
			'setrange_index': 'int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))',
			'getrange_index_n': 'DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))',
			'delrange_index_n': 'int (DCALL *)(DeeObject *, Dee_ssize_t))',
			'setrange_index_n': 'int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))',
			'trygetitem': 'DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))',
			'trygetitem_index': 'DREF DeeObject *(DCALL *)(DeeObject *, size_t))',
			'trygetitem_string_hash': 'DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))',
			'getitem_string_hash': 'DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))',
			'delitem_string_hash': 'int (DCALL *)(DeeObject *, char const *, Dee_hash_t))',
			'setitem_string_hash': 'int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))',
			'bounditem_string_hash': 'int (DCALL *)(DeeObject *, char const *, Dee_hash_t))',
			'hasitem_string_hash': 'int (DCALL *)(DeeObject *, char const *, Dee_hash_t))',
			'trygetitem_string_len_hash': 'DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))',
			'getitem_string_len_hash': 'DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))',
			'delitem_string_len_hash': 'int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))',
			'setitem_string_len_hash': 'int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))',
			'bounditem_string_len_hash': 'int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))',
			'hasitem_string_len_hash': 'int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))',
			'asvector': 'size_t (DCALL *)(DeeObject *, size_t dst_length, /' '*out*' '/ DREF DeeObject **))',
		});

		print('INTERN DeeTypeObject ', typename, ' = {');
		print('	OBJECT_HEAD_INIT(&DeeType_Type),');
		print('	/' '* .tp_name     = *' '/ ', getField('name'), ',');
		print('	/' '* .tp_doc      = *' '/ DOC(', getField('doc'), '),');
		print('	/' '* .tp_flags    = *' '/ ', getField('flags', 'TP_FNORMAL | TP_FFINAL'), ',');
		print('	/' '* .tp_weakrefs = *' '/ 0,');
		print('	/' '* .tp_features = *' '/ TF_NONE,');
		print('	/' '* .tp_base     = *' '/ &DeeSeq_Type,');
		print('	/' '* .tp_init = *' '/ {');
		print('		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(');
		print('			/' '* T:              *' '/ ', typename.partition("__").first, ',');
		print('			/' '* tp_ctor:        *' '/ ', getFieldPtr('ctor'), ',');
		print('			/' '* tp_copy_ctor:   *' '/ ', getFieldPtr('copy'), ',');
		print('			/' '* tp_any_ctor:    *' '/ ', getFieldPtr('init'), ',');
		print('			/' '* tp_any_ctor_kw: *' '/ NULL,');
		print('			/' '* tp_serialize:   *' '/ ', getFieldPtr('serialize'));
		print('		),');
		print('		/' '* .tp_dtor        = *' '/ ', getFieldTypedPtr('fini', '(void (DCALL *)(DeeObject *__restrict))'), ',');
		print('		/' '* .tp_assign      = *' '/ NULL,');
		print('		/' '* .tp_move_assign = *' '/ NULL');
		print('	},');
		print('	/' '* .tp_cast = *' '/ {');
		print('		/' '* .tp_str  = *' '/ NULL,');
		print('		/' '* .tp_repr = *' '/ NULL,');
		print('		/' '* .tp_bool = *' '/ NULL');
		print('	},');
		print('	/' '* .tp_visit         = *' '/ ', getFieldTypedPtr('visit', '(void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))'), ',');
		print('	/' '* .tp_gc            = *' '/ NULL,');
		print('	/' '* .tp_math          = *' '/ NULL,');
		print('	/' '* .tp_cmp           = *' '/ NULL,');
		print('	/' '* .tp_seq           = *' '/ ', getFieldPtr('seq'), ',');
		print('	/' '* .tp_iter_next     = *' '/ NULL,');
		print('	/' '* .tp_iterator      = *' '/ NULL,');
		print('	/' '* .tp_attr          = *' '/ NULL,');
		print('	/' '* .tp_with          = *' '/ NULL,');
		print('	/' '* .tp_buffer        = *' '/ NULL,');
		print('	/' '* .tp_methods       = *' '/ ', getField('methods'), ',');
		print('	/' '* .tp_getsets       = *' '/ ', getField('getsets'), ',');
		print('	/' '* .tp_members       = *' '/ ', getField('members'), ',');
		print('	/' '* .tp_class_methods = *' '/ ', getField('class_methods'), ',');
		print('	/' '* .tp_class_getsets = *' '/ ', getField('class_getsets'), ',');
		print('	/' '* .tp_class_members = *' '/ ', getField('class_members'), ',');
		print('	/' '* .tp_method_hints  = *' '/ ', getField('method_hints'), ',');
		print('};');
		print;
		print;
	}
}

global final TYPES: {EnumType...} = {
	EnumType("DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast",                         "de_sos_gif__"),
	EnumType("DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast",            "dewif_sos_gif__"),
	EnumType("DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index",              "de_sos_sotgi__"),
	EnumType("DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index", "dewif_sos_sotgi__"),
	EnumType("DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index",                 "de_sos_sogi__"),
	EnumType("DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index",    "dewif_sos_soii__"),
	EnumType("DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem",                     "de_soso_sog__"),
	EnumType("DefaultEnumeration__with__seq_operator_getitem_index",                                         "de_sogi__"),
	EnumType("DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index",                            "dewif_soii__"),
	EnumType("DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem",           "dewf_soso_sog__"),
	EnumType("DefaultEnumeration__with__seq_operator_getitem",                                               "de_sog__"),
	EnumType("DefaultEnumerationWithFilter__with__seq_operator_getitem",                                     "dewf_sog__"),
	EnumType("DefaultEnumeration__with__seq_operator_iter__and__counter",                                    "de_soi__"),
	EnumType("DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter",                       "dewif_soi__"),
	EnumType("DefaultEnumeration__with__seq_enumerate",                                                      "de_se__"),
	EnumType("DefaultEnumerationWithIntFilter__with__seq_enumerate_index",                                   "dewif_sei__"),

	EnumType("DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack",                           "dewf_moi__"),
	EnumType("DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem",                            "de_mik_mog__"),
	EnumType("DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem",                  "dewf_mik_mog__"),
	EnumType("DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem",                         "de_mik_motg__"),
	EnumType("DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem",               "dewf_mik_motg__"),
	EnumType("DefaultEnumeration__with__map_enumerate",                                                      "de_me__"),
	EnumType("DefaultEnumerationWithFilter__with__map_enumerate_range",                                      "dewf_mer__"),
};

local inImplBlock = false;
for (local line: File.open("default-enumerate.c", "rb")) {
	line = line.partition("//").first.strip();
	if (line == '/' '*[[[begin:impls]]]*' '/') {
		inImplBlock = true;
		continue;
	}
	if (line == '/' '*[[[end:impls]]]*' '/')
		break;
	if (inImplBlock) {
		for (local typ: TYPES) {
			for (local sym: line.relocateall(typ.symbolPrefixRegex)) {
				typ.symbols.insert(str sym[#typ.symbolPrefix:]);
			}
		}
	}
}

for (local typ: TYPES)
	typ.printType();
]]]*/
/* DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast */
PRIVATE struct type_seq de_sos_gif__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_sos_gif__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_size__and__getitem_index_fast = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_sos_gif__name,
	/* .tp_doc      = */ DOC(de_sos_gif__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_sos_gif__ctor,
			/* tp_copy_ctor:   */ &de_sos_gif__copy,
			/* tp_any_ctor:    */ &de_sos_gif__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_sos_gif__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_sos_gif__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_sos_gif__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_sos_gif__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_sos_gif__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_sos_gif__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast */
PRIVATE struct type_seq dewif_sos_gif__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewif_sos_gif__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_size__and__getitem_index_fast = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewif_sos_gif__name,
	/* .tp_doc      = */ DOC(dewif_sos_gif__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithIntFilter,
			/* tp_ctor:        */ &dewif_sos_gif__ctor,
			/* tp_copy_ctor:   */ &dewif_sos_gif__copy,
			/* tp_any_ctor:    */ &dewif_sos_gif__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewif_sos_gif__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewif_sos_gif__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewif_sos_gif__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewif_sos_gif__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewif_sos_gif__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewif_sos_gif__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index */
PRIVATE struct type_seq de_sos_sotgi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_sos_sotgi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_size__and__seq_operator_trygetitem_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_sos_sotgi__name,
	/* .tp_doc      = */ DOC(de_sos_sotgi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_sos_sotgi__ctor,
			/* tp_copy_ctor:   */ &de_sos_sotgi__copy,
			/* tp_any_ctor:    */ &de_sos_sotgi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_sos_sotgi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_sos_sotgi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_sos_sotgi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_sos_sotgi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_sos_sotgi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_sos_sotgi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index */
PRIVATE struct type_seq dewif_sos_sotgi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewif_sos_sotgi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_trygetitem_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewif_sos_sotgi__name,
	/* .tp_doc      = */ DOC(dewif_sos_sotgi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithIntFilter,
			/* tp_ctor:        */ &dewif_sos_sotgi__ctor,
			/* tp_copy_ctor:   */ &dewif_sos_sotgi__copy,
			/* tp_any_ctor:    */ &dewif_sos_sotgi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewif_sos_sotgi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewif_sos_sotgi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewif_sos_sotgi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewif_sos_sotgi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewif_sos_sotgi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewif_sos_sotgi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index */
PRIVATE struct type_seq de_sos_sogi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_sos_sogi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_size__and__seq_operator_getitem_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_sos_sogi__name,
	/* .tp_doc      = */ DOC(de_sos_sogi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_sos_sogi__ctor,
			/* tp_copy_ctor:   */ &de_sos_sogi__copy,
			/* tp_any_ctor:    */ &de_sos_sogi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_sos_sogi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_sos_sogi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_sos_sogi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_sos_sogi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_sos_sogi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_sos_sogi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index */
PRIVATE struct type_seq dewif_sos_soii__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewif_sos_soii__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_size__and__seq_operator_getitem_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewif_sos_soii__name,
	/* .tp_doc      = */ DOC(dewif_sos_soii__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithIntFilter,
			/* tp_ctor:        */ &dewif_sos_soii__ctor,
			/* tp_copy_ctor:   */ &dewif_sos_soii__copy,
			/* tp_any_ctor:    */ &dewif_sos_soii__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewif_sos_soii__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewif_sos_soii__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewif_sos_soii__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewif_sos_soii__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewif_sos_soii__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewif_sos_soii__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem */
PRIVATE struct type_seq de_soso_sog__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_soso_sog__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_sizeob__and__seq_operator_getitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_soso_sog__name,
	/* .tp_doc      = */ DOC(de_soso_sog__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_soso_sog__ctor,
			/* tp_copy_ctor:   */ &de_soso_sog__copy,
			/* tp_any_ctor:    */ &de_soso_sog__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_soso_sog__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_soso_sog__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_soso_sog__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_soso_sog__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_soso_sog__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_soso_sog__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_operator_getitem_index */
PRIVATE struct type_seq de_sogi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_sogi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_getitem_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_sogi__name,
	/* .tp_doc      = */ DOC(de_sogi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_sogi__ctor,
			/* tp_copy_ctor:   */ &de_sogi__copy,
			/* tp_any_ctor:    */ &de_sogi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_sogi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_sogi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_sogi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_sogi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_sogi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_sogi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index */
PRIVATE struct type_seq dewif_soii__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewif_soii__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_getitem_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewif_soii__name,
	/* .tp_doc      = */ DOC(dewif_soii__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithIntFilter,
			/* tp_ctor:        */ &dewif_soii__ctor,
			/* tp_copy_ctor:   */ &dewif_soii__copy,
			/* tp_any_ctor:    */ &dewif_soii__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewif_soii__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewif_soii__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewif_soii__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewif_soii__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewif_soii__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewif_soii__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem */
PRIVATE struct type_seq dewf_soso_sog__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewf_soso_sog__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithFilter__with__seq_operator_sizeob__and__seq_operator_getitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewf_soso_sog__name,
	/* .tp_doc      = */ DOC(dewf_soso_sog__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithFilter,
			/* tp_ctor:        */ &dewf_soso_sog__ctor,
			/* tp_copy_ctor:   */ &dewf_soso_sog__copy,
			/* tp_any_ctor:    */ &dewf_soso_sog__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewf_soso_sog__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewf_soso_sog__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewf_soso_sog__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewf_soso_sog__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewf_soso_sog__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewf_soso_sog__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_operator_getitem */
PRIVATE struct type_seq de_sog__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_sog__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_getitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_sog__name,
	/* .tp_doc      = */ DOC(de_sog__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_sog__ctor,
			/* tp_copy_ctor:   */ &de_sog__copy,
			/* tp_any_ctor:    */ &de_sog__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_sog__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_sog__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_sog__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_sog__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_sog__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_sog__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithFilter__with__seq_operator_getitem */
PRIVATE struct type_seq dewf_sog__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewf_sog__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithFilter__with__seq_operator_getitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewf_sog__name,
	/* .tp_doc      = */ DOC(dewf_sog__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithFilter,
			/* tp_ctor:        */ &dewf_sog__ctor,
			/* tp_copy_ctor:   */ &dewf_sog__copy,
			/* tp_any_ctor:    */ &dewf_sog__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewf_sog__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewf_sog__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewf_sog__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewf_sog__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewf_sog__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewf_sog__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_operator_iter__and__counter */
PRIVATE struct type_seq de_soi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_soi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_operator_iter__and__counter = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_soi__name,
	/* .tp_doc      = */ DOC(de_soi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_soi__ctor,
			/* tp_copy_ctor:   */ &de_soi__copy,
			/* tp_any_ctor:    */ &de_soi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_soi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_soi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_soi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_soi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_soi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_soi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter */
PRIVATE struct type_seq dewif_soi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewif_soi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_operator_iter__and__counter = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewif_soi__name,
	/* .tp_doc      = */ DOC(dewif_soi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithIntFilter,
			/* tp_ctor:        */ &dewif_soi__ctor,
			/* tp_copy_ctor:   */ &dewif_soi__copy,
			/* tp_any_ctor:    */ &dewif_soi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewif_soi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewif_soi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewif_soi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewif_soi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewif_soi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewif_soi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__seq_enumerate */
PRIVATE struct type_seq de_se__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_se__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__seq_enumerate = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_se__name,
	/* .tp_doc      = */ DOC(de_se__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_se__ctor,
			/* tp_copy_ctor:   */ &de_se__copy,
			/* tp_any_ctor:    */ &de_se__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_se__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_se__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_se__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_se__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_se__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithIntFilter__with__seq_enumerate_index */
PRIVATE struct type_seq dewif_sei__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewif_sei__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithIntFilter__with__seq_enumerate_index = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewif_sei__name,
	/* .tp_doc      = */ DOC(dewif_sei__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithIntFilter,
			/* tp_ctor:        */ &dewif_sei__ctor,
			/* tp_copy_ctor:   */ &dewif_sei__copy,
			/* tp_any_ctor:    */ &dewif_sei__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewif_sei__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewif_sei__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewif_sei__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewif_sei__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewif_sei__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack */
PRIVATE struct type_seq dewf_moi__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewf_moi__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithFilter__with__map_operator_iter__and__unpack = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewf_moi__name,
	/* .tp_doc      = */ DOC(dewf_moi__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithFilter,
			/* tp_ctor:        */ &dewf_moi__ctor,
			/* tp_copy_ctor:   */ &dewf_moi__copy,
			/* tp_any_ctor:    */ &dewf_moi__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewf_moi__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewf_moi__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewf_moi__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewf_moi__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewf_moi__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dewf_moi__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem */
PRIVATE struct type_seq de_mik_mog__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_mik_mog__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__map_iterkeys__and__map_operator_getitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_mik_mog__name,
	/* .tp_doc      = */ DOC(de_mik_mog__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_mik_mog__ctor,
			/* tp_copy_ctor:   */ &de_mik_mog__copy,
			/* tp_any_ctor:    */ &de_mik_mog__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_mik_mog__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_mik_mog__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_mik_mog__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_mik_mog__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_mik_mog__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_mik_mog__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem */
PRIVATE struct type_seq dewf_mik_mog__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewf_mik_mog__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_getitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewf_mik_mog__name,
	/* .tp_doc      = */ DOC(dewf_mik_mog__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithFilter,
			/* tp_ctor:        */ &dewf_mik_mog__ctor,
			/* tp_copy_ctor:   */ &dewf_mik_mog__copy,
			/* tp_any_ctor:    */ &dewf_mik_mog__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewf_mik_mog__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewf_mik_mog__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewf_mik_mog__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewf_mik_mog__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewf_mik_mog__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem */
PRIVATE struct type_seq de_mik_motg__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_mik_motg__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__map_iterkeys__and__map_operator_trygetitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_mik_motg__name,
	/* .tp_doc      = */ DOC(de_mik_motg__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_mik_motg__ctor,
			/* tp_copy_ctor:   */ &de_mik_motg__copy,
			/* tp_any_ctor:    */ &de_mik_motg__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_mik_motg__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_mik_motg__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_mik_motg__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_mik_motg__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_mik_motg__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ de_mik_motg__class_members,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem */
PRIVATE struct type_seq dewf_mik_motg__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewf_mik_motg__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithFilter__with__map_iterkeys__and__map_operator_trygetitem = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewf_mik_motg__name,
	/* .tp_doc      = */ DOC(dewf_mik_motg__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithFilter,
			/* tp_ctor:        */ &dewf_mik_motg__ctor,
			/* tp_copy_ctor:   */ &dewf_mik_motg__copy,
			/* tp_any_ctor:    */ &dewf_mik_motg__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewf_mik_motg__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewf_mik_motg__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewf_mik_motg__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewf_mik_motg__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewf_mik_motg__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumeration__with__map_enumerate */
PRIVATE struct type_seq de_me__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&de_me__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumeration__with__map_enumerate = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ de_me__name,
	/* .tp_doc      = */ DOC(de_me__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumeration,
			/* tp_ctor:        */ &de_me__ctor,
			/* tp_copy_ctor:   */ &de_me__copy,
			/* tp_any_ctor:    */ &de_me__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &de_me__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&de_me__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&de_me__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &de_me__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ de_me__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};


/* DefaultEnumerationWithFilter__with__map_enumerate_range */
PRIVATE struct type_seq dewf_mer__seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dewf_mer__iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
	/* .tp_asvector                   = */ NULL,
};

INTERN DeeTypeObject DefaultEnumerationWithFilter__with__map_enumerate_range = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ dewf_mer__name,
	/* .tp_doc      = */ DOC(dewf_mer__doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultEnumerationWithFilter,
			/* tp_ctor:        */ &dewf_mer__ctor,
			/* tp_copy_ctor:   */ &dewf_mer__copy,
			/* tp_any_ctor:    */ &dewf_mer__init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dewf_mer__serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dewf_mer__fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dewf_mer__visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dewf_mer__seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dewf_mer__members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};
/*[[[end]]]*/

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ENUMERATE_C */
