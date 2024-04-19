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
#ifndef GUARD_DEEMON_OBJECTS_TYPE_OPERATORS_C
#define GUARD_DEEMON_OBJECTS_TYPE_OPERATORS_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>
/**/

#include "gc_inspect.h"
#include "type-operators.h"

DECL_BEGIN

typedef DeeTypeObject Type;

#define OPNAME(opname) "operator " opname

#define DEFINE_OPERATOR_INVOKE(name, instance_name)                                          \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                    \
	invoke_##name(DeeTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self, \
	              size_t argc, DeeObject *const *argv, uint16_t opname);                     \
	PRIVATE struct Dee_operator_invoke tpconst name =                                        \
	Dee_OPERATOR_INVOKE_INIT(&invoke_##name, instance_name);                                 \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                    \
	invoke_##name(DeeTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self, \
	              size_t argc, DeeObject *const *argv, uint16_t opname)
PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_operator_cannot_invoke(DeeTypeObject *__restrict tp_self,
                           char const *__restrict name) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot directly invoke `%r." OPNAME("%s") "'",
	                       tp_self, name);
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_operator_requires_inplace(DeeTypeObject *__restrict tp_self,
                              char const *__restrict name) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot invoke inplace `%s." OPNAME("%s") "' without l-value",
	                       tp_self, name);
}

DEFINE_OPERATOR_INVOKE(operator_constructor, &instance_ctor) {
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
	err_operator_cannot_invoke(tp_self, "constructor");
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_copy, &instance_copy) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("copy")))
		goto err;
	return DeeObject_TCopy(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_deepcopy, &instance_deepcopy) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("deepcopy")))
		goto err;
	return DeeObject_TDeepCopy(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_destructor, &instance_destructor) {
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
	err_operator_cannot_invoke(tp_self, "destructor");
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_bool, &instance_bool) {
	int result;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("bool")))
		goto err;
	result = DeeObject_TBool(tp_self, self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_next, &instance_next) {
	DREF DeeObject *result;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("next")))
		goto err;
	result = DeeObject_TIterNext(tp_self, self);
	if unlikely(result == ITER_DONE) {
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_call, &instance_call) {
	DeeObject *args, *kw = NULL;
	DREF DeeObject *result;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o|o:" OPNAME("call"), &args, &kw))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (kw) {
		DREF DeeObject *kwds = DeeKw_Wrap(kw);
		if unlikely(!kwds)
			goto err;
		result = DeeObject_TCallTupleKw(tp_self, self, args, kwds);
		Dee_Decref(kwds);
	} else {
		result = DeeObject_TCallTuple(tp_self, self, args);
	}
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_float, &instance_double) {
	double result;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("float")))
		goto err;
	if (DeeObject_TAsDouble(tp_self, self, &result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_hash, &instance_hash) {
	dhash_t result;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("hash")))
		goto err;
	result = DeeObject_THash(tp_self, self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getattr, &instance_getattr) {
	DeeStringObject *attr;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("getattr"), &attr))
		goto err;
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	return DeeObject_TGetAttr(tp_self, self, (DeeObject *)attr);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_delattr, &instance_delattr) {
	DeeStringObject *attr;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("delattr"), &attr))
		goto err;
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_TDelAttr(tp_self, self, (DeeObject *)attr))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_setattr, &instance_setattr) {
	DeeStringObject *attr;
	DeeObject *value;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "oo:" OPNAME("setattr"), &attr, &value))
		goto err;
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_TSetAttr(tp_self, self, (DeeObject *)attr, value))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_enumattr, &instance_enumattr) {
	DeeObject *args[2];
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("enumattr")))
		goto err;
	args[0] = (DeeObject *)tp_self;
	args[1] = self;
	return DeeObject_New(&DeeEnumAttr_Type, 2, args);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_visit, &instance_visit) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("visit")))
		goto err;
	return (DREF DeeObject *)DeeGC_TNewReferred(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_clear, &instance_clear) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("clear")))
		goto err;
	DeeObject_TClear(tp_self, self);
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_pclear, &instance_pclear) {
	unsigned int gc_prio;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "u:" OPNAME("pclear"), &gc_prio))
		goto err;
	DeeObject_TPClear(tp_self, self, gc_prio);
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getbuf, NULL) {
	bool writable = false;
	size_t start = 0, end = (size_t)-1;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "|b" UNPuSIZ UNPuSIZ ":" OPNAME("getbuf"),
	                  &writable, &start, &end))
		goto err;
	return DeeObject_TBytes(tp_self, self,
	                        writable ? Dee_BUFFER_FWRITABLE
	                                 : Dee_BUFFER_FREADONLY,
	                        start, end);
err:
	return NULL;
}

/*[[[deemon
import * from deemon;
function defineUnary(name: string, nameTitle: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, ":" OPNAME("{name}")))');
	print(f'		goto err;');
	print(f'	return DeeObject_T{nameTitle}(tp_self, self);');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineUnaryInplace(name: string, nameTitle: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	(void)self;');
	print(f'	(void)opname;');
	print(f'	if unlikely(!p_self)');
	print(f'		goto err_requires_inplace;');
	print(f'	if (DeeArg_Unpack(argc, argv, ":" OPNAME("{name}")))');
	print(f'		goto err;');
	print(f'	if unlikely(DeeObject_T{nameTitle}(tp_self, p_self))');
	print(f'		goto err;');
	print(f'	return_reference_(*p_self);');
	print(f'err_requires_inplace:');
	print(f'	err_operator_requires_inplace(tp_self, "{name}");');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineUnaryInt(name: string, nameTitle: string = none, returnNone: bool = false) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, ":" OPNAME("{name}")))');
	print(f'		goto err;');
	print(f'	if (DeeObject_T{nameTitle}(tp_self, self))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineBinary(name: string, nameTitle: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	DeeObject *other;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("{name}"), &other))');
	print(f'		goto err;');
	print(f'	return DeeObject_T{nameTitle}(tp_self, self, other);');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineBinaryInt(name: string, nameTitle: string = none, returnNone: bool = false) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	DeeObject *other;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("{name}"), &other))');
	print(f'		goto err;');
	print(f'	if (DeeObject_T{nameTitle}(tp_self, self, other))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineBinaryInplace(name: string, nameTitle: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_i{name}, &instance_i{name}) \{');
	print(f'	DeeObject *other;');
	print(f'	(void)self;');
	print(f'	(void)opname;');
	print(f'	if unlikely(!p_self)');
	print(f'		goto err_requires_inplace;');
	print(f'	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("i{name}"), &other))');
	print(f'		goto err;');
	print(f'	if unlikely(DeeObject_TInplace{nameTitle}(tp_self, p_self, other))');
	print(f'		goto err;');
	print(f'	return_reference_(*p_self);');
	print(f'err_requires_inplace:');
	print(f'	err_operator_requires_inplace(tp_self, "i{name}");');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineTrinary(name: string, nameTitle: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	DeeObject *a, *b;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, "oo:" OPNAME("{name}"), &a, &b))');
	print(f'		goto err;');
	print(f'	return DeeObject_T{nameTitle}(tp_self, self, a, b);');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineTrinaryInt(name: string, nameTitle: string = none, returnNone: bool = false) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	DeeObject *a, *b;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, "oo:" OPNAME("{name}"), &a, &b))');
	print(f'		goto err;');
	print(f'	if (DeeObject_T{nameTitle}(tp_self, self, a, b))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineQuaternaryInt(name: string, nameTitle: string = none, returnNone: bool = false) {
	if (nameTitle is none)
		nameTitle = name.title();
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &instance_{name}) \{');
	print(f'	DeeObject *a, *b, *c;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	if (DeeArg_Unpack(argc, argv, "ooo:" OPNAME("{name}"), &a, &b, &c))');
	print(f'		goto err;');
	print(f'	if (DeeObject_T{nameTitle}(tp_self, self, a, b, c))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

local mathOps = ["add", "sub", "mul", "div", "mod", "shl", "shr", "and", "or", "xor", "pow"];
defineUnary("str");
defineUnary("repr");
defineBinaryInt("assign");
defineBinaryInt("moveassign", nameTitle: "MoveAssign");
defineUnary("int");
defineUnary("inv");
defineUnary("pos");
defineUnary("neg");
defineUnaryInplace("inc");
defineUnaryInplace("dec");
for (local name: mathOps)
	defineBinary(name);
for (local name: mathOps)
	defineBinaryInplace(name);
defineBinary("eq", nameTitle: "CompareEqObject");
defineBinary("ne", nameTitle: "CompareNeObject");
defineBinary("lo", nameTitle: "CompareLoObject");
defineBinary("le", nameTitle: "CompareLeObject");
defineBinary("gr", nameTitle: "CompareGrObject");
defineBinary("ge", nameTitle: "CompareGeObject");
defineUnary("iter", nameTitle: "IterSelf");
defineUnary("size", nameTitle: "SizeObject");
defineBinary("contains", nameTitle: "ContainsObject");
defineBinary("getitem", nameTitle: "GetItem");
defineBinaryInt("delitem", nameTitle: "DelItem", returnNone: true);
defineTrinaryInt("setitem", nameTitle: "SetItem", returnNone: true);
defineTrinary("getrange", nameTitle: "GetRange");
defineTrinaryInt("delrange", nameTitle: "DelRange", returnNone: true);
defineQuaternaryInt("setrange", nameTitle: "SetRange", returnNone: true);
defineUnaryInt("enter", returnNone: true);
defineUnaryInt("leave", returnNone: true);
]]]*/
DEFINE_OPERATOR_INVOKE(operator_str, &instance_str) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("str")))
		goto err;
	return DeeObject_TStr(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_repr, &instance_repr) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("repr")))
		goto err;
	return DeeObject_TRepr(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_assign, &instance_assign) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("assign"), &other))
		goto err;
	if (DeeObject_TAssign(tp_self, self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_moveassign, &instance_moveassign) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("moveassign"), &other))
		goto err;
	if (DeeObject_TMoveAssign(tp_self, self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_int, &instance_int) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("int")))
		goto err;
	return DeeObject_TInt(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_inv, &instance_inv) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("inv")))
		goto err;
	return DeeObject_TInv(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_pos, &instance_pos) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("pos")))
		goto err;
	return DeeObject_TPos(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_neg, &instance_neg) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("neg")))
		goto err;
	return DeeObject_TNeg(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_inc, &instance_inc) {
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("inc")))
		goto err;
	if unlikely(DeeObject_TInc(tp_self, p_self))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "inc");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_dec, &instance_dec) {
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("dec")))
		goto err;
	if unlikely(DeeObject_TDec(tp_self, p_self))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "dec");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_add, &instance_add) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("add"), &other))
		goto err;
	return DeeObject_TAdd(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_sub, &instance_sub) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("sub"), &other))
		goto err;
	return DeeObject_TSub(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_mul, &instance_mul) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("mul"), &other))
		goto err;
	return DeeObject_TMul(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_div, &instance_div) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("div"), &other))
		goto err;
	return DeeObject_TDiv(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_mod, &instance_mod) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("mod"), &other))
		goto err;
	return DeeObject_TMod(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_shl, &instance_shl) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("shl"), &other))
		goto err;
	return DeeObject_TShl(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_shr, &instance_shr) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("shr"), &other))
		goto err;
	return DeeObject_TShr(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_and, &instance_and) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("and"), &other))
		goto err;
	return DeeObject_TAnd(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_or, &instance_or) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("or"), &other))
		goto err;
	return DeeObject_TOr(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_xor, &instance_xor) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("xor"), &other))
		goto err;
	return DeeObject_TXor(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_pow, &instance_pow) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("pow"), &other))
		goto err;
	return DeeObject_TPow(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_iadd, &instance_iadd) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("iadd"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceAdd(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "iadd");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_isub, &instance_isub) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("isub"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceSub(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "isub");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_imul, &instance_imul) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("imul"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceMul(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "imul");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_idiv, &instance_idiv) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("idiv"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceDiv(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "idiv");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_imod, &instance_imod) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("imod"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceMod(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "imod");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ishl, &instance_ishl) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ishl"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceShl(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ishl");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ishr, &instance_ishr) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ishr"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceShr(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ishr");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_iand, &instance_iand) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("iand"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceAnd(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "iand");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ior, &instance_ior) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ior"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceOr(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ior");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ixor, &instance_ixor) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ixor"), &other))
		goto err;
	if unlikely(DeeObject_TInplaceXor(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ixor");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ipow, &instance_ipow) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ipow"), &other))
		goto err;
	if unlikely(DeeObject_TInplacePow(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ipow");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_eq, &instance_eq) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("eq"), &other))
		goto err;
	return DeeObject_TCompareEqObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ne, &instance_ne) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ne"), &other))
		goto err;
	return DeeObject_TCompareNeObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_lo, &instance_lo) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("lo"), &other))
		goto err;
	return DeeObject_TCompareLoObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_le, &instance_le) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("le"), &other))
		goto err;
	return DeeObject_TCompareLeObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_gr, &instance_gr) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("gr"), &other))
		goto err;
	return DeeObject_TCompareGrObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ge, &instance_ge) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("ge"), &other))
		goto err;
	return DeeObject_TCompareGeObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_iter, &instance_iter) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("iter")))
		goto err;
	return DeeObject_TIterSelf(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_size, &instance_size) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("size")))
		goto err;
	return DeeObject_TSizeObject(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_contains, &instance_contains) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("contains"), &other))
		goto err;
	return DeeObject_TContainsObject(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getitem, &instance_getitem) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("getitem"), &other))
		goto err;
	return DeeObject_TGetItem(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_delitem, &instance_delitem) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "o:" OPNAME("delitem"), &other))
		goto err;
	if (DeeObject_TDelItem(tp_self, self, other))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_setitem, &instance_setitem) {
	DeeObject *a, *b;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "oo:" OPNAME("setitem"), &a, &b))
		goto err;
	if (DeeObject_TSetItem(tp_self, self, a, b))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getrange, &instance_getrange) {
	DeeObject *a, *b;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "oo:" OPNAME("getrange"), &a, &b))
		goto err;
	return DeeObject_TGetRange(tp_self, self, a, b);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_delrange, &instance_delrange) {
	DeeObject *a, *b;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "oo:" OPNAME("delrange"), &a, &b))
		goto err;
	if (DeeObject_TDelRange(tp_self, self, a, b))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_setrange, &instance_setrange) {
	DeeObject *a, *b, *c;
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, "ooo:" OPNAME("setrange"), &a, &b, &c))
		goto err;
	if (DeeObject_TSetRange(tp_self, self, a, b, c))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_enter, &instance_enter) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("enter")))
		goto err;
	if (DeeObject_TEnter(tp_self, self))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_leave, &instance_leave) {
	(void)p_self;
	(void)opname;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("leave")))
		goto err;
	if (DeeObject_TLeave(tp_self, self))
		goto err;
	return_none;
err:
	return NULL;
}
/*[[[end]]]*/

#undef DEFINE_OPERATOR_INVOKE

INTERN_CONST struct type_operator tpconst type_operators[LENGTHOF_type_operators] = {
	/* IMPORTANT: This stuff needs to be kept sorted by operator ID!
	 * Also: (for this array only) Every operator until "OPERATOR_USERCOUNT" *MUST* be present
	 *       (with none skipped), and after that, OPERATOR_PRIVMIN..OPERATOR_PRIVMAX must be
	 *       enumerated (also without any gaps). */
	TYPE_OPERATOR_DECL(OPERATOR_0000_CONSTRUCTOR, /**/ OPCLASS_TYPE, /*             */ offsetof(Type, tp_init.tp_alloc.tp_any_ctor), /* */ OPCC_SPECIAL, /*         */ "this", /*    */ "constructor", /**/ "tp_any_ctor", &operator_constructor),
	TYPE_OPERATOR_DECL(OPERATOR_0001_COPY, /*       */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_init.tp_alloc.tp_copy_ctor), /**/ OPCC_SPECIAL, /*         */ "copy", /*    */ "copy", /*       */ "tp_copy_ctor", &operator_copy),
	TYPE_OPERATOR_DECL(OPERATOR_0002_DEEPCOPY, /*   */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_init.tp_alloc.tp_deep_ctor), /**/ OPCC_SPECIAL, /*         */ "deepcopy", /**/ "deepcopy", /*   */ "tp_deep_ctor", &operator_deepcopy),
	TYPE_OPERATOR_DECL(OPERATOR_0003_DESTRUCTOR, /* */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_init.tp_dtor), /*              */ OPCC_UNARY_VOID, /*      */ "~this", /*   */ "destructor", /* */ "tp_dtor", &operator_destructor),
	TYPE_OPERATOR_DECL(OPERATOR_0004_ASSIGN, /*     */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_init.tp_assign), /*            */ OPCC_BINARY_INT, /*      */ ":=", /*      */ "assign", /*     */ "tp_assign", &operator_assign),
	TYPE_OPERATOR_DECL(OPERATOR_0005_MOVEASSIGN, /* */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_init.tp_move_assign), /*       */ OPCC_SPECIAL, /*         */ "move:=", /*  */ "moveassign", /* */ "tp_move_assign", &operator_moveassign),
	TYPE_OPERATOR_DECL(OPERATOR_0006_STR, /*        */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_cast.tp_str), /*               */ OPCC_UNARY_OBJECT, /*    */ "str", /*     */ "str", /*        */ "tp_str", &operator_str),
	TYPE_OPERATOR_DECL(OPERATOR_0007_REPR, /*       */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_cast.tp_repr), /*              */ OPCC_UNARY_OBJECT, /*    */ "repr", /*    */ "repr", /*       */ "tp_repr", &operator_repr),
	TYPE_OPERATOR_DECL(OPERATOR_0008_BOOL, /*       */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_cast.tp_bool), /*              */ OPCC_UNARY_INT, /*       */ "bool", /*    */ "bool", /*       */ "tp_bool", &operator_bool),
	TYPE_OPERATOR_DECL(OPERATOR_0009_ITERNEXT, /*   */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_iter_next), /*                 */ OPCC_UNARY_OBJECT, /*    */ "next", /*    */ "next", /*       */ "tp_iter_next", &operator_next),
	TYPE_OPERATOR_DECL(OPERATOR_000A_CALL, /*       */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_call), /*                      */ OPCC_BINARY_OBJECT, /*   */ "()", /*      */ "call", /*       */ "tp_call", &operator_call),
//	TYPE_OPERATOR_DECL(OPERATOR_000A_CALL, /*       */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_call_kw) /*,                   */ OPCC_TRINARY_OBJECT, /*  */ "()", /*      */ "call", /*       */ "tp_call_kw", &operator_call),
	TYPE_OPERATOR_DECL(OPERATOR_000B_INT, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_int), /*           */ OPCC_SPECIAL, /*         */ "int", /*     */ "int", /*        */ "tp_int", &operator_int),
	TYPE_OPERATOR_DECL(OPERATOR_000C_FLOAT, /*      */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_double), /*        */ OPCC_SPECIAL, /*         */ "float", /*   */ "float", /*      */ "tp_double", &operator_float),
	TYPE_OPERATOR_DECL(OPERATOR_000D_INV, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inv), /*           */ OPCC_UNARY_OBJECT, /*    */ "~", /*       */ "inv", /*        */ "tp_inv", &operator_inv),
	TYPE_OPERATOR_DECL(OPERATOR_000E_POS, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_pos), /*           */ OPCC_UNARY_OBJECT, /*    */ "+", /*       */ "pos", /*        */ "tp_pos", &operator_pos),
	TYPE_OPERATOR_DECL(OPERATOR_000F_NEG, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_neg), /*           */ OPCC_BINARY_OBJECT, /*   */ "-", /*       */ "neg", /*        */ "tp_neg", &operator_neg),
	TYPE_OPERATOR_DECL(OPERATOR_0010_ADD, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_add), /*           */ OPCC_BINARY_OBJECT, /*   */ "+", /*       */ "add", /*        */ "tp_add", &operator_add),
	TYPE_OPERATOR_DECL(OPERATOR_0011_SUB, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_sub), /*           */ OPCC_BINARY_OBJECT, /*   */ "-", /*       */ "sub", /*        */ "tp_sub", &operator_sub),
	TYPE_OPERATOR_DECL(OPERATOR_0012_MUL, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_mul), /*           */ OPCC_BINARY_OBJECT, /*   */ "*", /*       */ "mul", /*        */ "tp_mul", &operator_mul),
	TYPE_OPERATOR_DECL(OPERATOR_0013_DIV, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_div), /*           */ OPCC_BINARY_OBJECT, /*   */ "/", /*       */ "div", /*        */ "tp_div", &operator_div),
	TYPE_OPERATOR_DECL(OPERATOR_0014_MOD, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_mod), /*           */ OPCC_BINARY_OBJECT, /*   */ "%", /*       */ "mod", /*        */ "tp_mod", &operator_mod),
	TYPE_OPERATOR_DECL(OPERATOR_0015_SHL, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_shl), /*           */ OPCC_BINARY_OBJECT, /*   */ "<<", /*      */ "shl", /*        */ "tp_shl", &operator_shl),
	TYPE_OPERATOR_DECL(OPERATOR_0016_SHR, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_shr), /*           */ OPCC_BINARY_OBJECT, /*   */ ">>", /*      */ "shr", /*        */ "tp_shr", &operator_shr),
	TYPE_OPERATOR_DECL(OPERATOR_0017_AND, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_and), /*           */ OPCC_BINARY_OBJECT, /*   */ "&", /*       */ "and", /*        */ "tp_and", &operator_and),
	TYPE_OPERATOR_DECL(OPERATOR_0018_OR, /*         */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_or), /*            */ OPCC_BINARY_OBJECT, /*   */ "|", /*       */ "or", /*         */ "tp_or", &operator_or),
	TYPE_OPERATOR_DECL(OPERATOR_0019_XOR, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_xor), /*           */ OPCC_BINARY_OBJECT, /*   */ "^", /*       */ "xor", /*        */ "tp_xor", &operator_xor),
	TYPE_OPERATOR_DECL(OPERATOR_001A_POW, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_pow), /*           */ OPCC_BINARY_OBJECT, /*   */ "**", /*      */ "pow", /*        */ "tp_pow", &operator_pow),
	TYPE_OPERATOR_DECL(OPERATOR_001B_INC, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inc), /*           */ OPCC_UNARY_INPLACE, /*   */ "++", /*      */ "inc", /*        */ "tp_inc", &operator_inc),
	TYPE_OPERATOR_DECL(OPERATOR_001C_DEC, /*        */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_dec), /*           */ OPCC_UNARY_INPLACE, /*   */ "--", /*      */ "dec", /*        */ "tp_dec", &operator_dec),
	TYPE_OPERATOR_DECL(OPERATOR_001D_INPLACE_ADD, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_add), /*   */ OPCC_BINARY_INPLACE, /*  */ "+=", /*      */ "iadd", /*       */ "tp_inplace_add", &operator_iadd),
	TYPE_OPERATOR_DECL(OPERATOR_001E_INPLACE_SUB, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_sub), /*   */ OPCC_BINARY_INPLACE, /*  */ ",=", /*      */ "isub", /*       */ "tp_inplace_sub", &operator_isub),
	TYPE_OPERATOR_DECL(OPERATOR_001F_INPLACE_MUL, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_mul), /*   */ OPCC_BINARY_INPLACE, /*  */ "*=", /*      */ "imul", /*       */ "tp_inplace_mul", &operator_imul),
	TYPE_OPERATOR_DECL(OPERATOR_0020_INPLACE_DIV, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_div), /*   */ OPCC_BINARY_INPLACE, /*  */ "/=", /*      */ "idiv", /*       */ "tp_inplace_div", &operator_idiv),
	TYPE_OPERATOR_DECL(OPERATOR_0021_INPLACE_MOD, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_mod), /*   */ OPCC_BINARY_INPLACE, /*  */ "%=", /*      */ "imod", /*       */ "tp_inplace_mod", &operator_imod),
	TYPE_OPERATOR_DECL(OPERATOR_0022_INPLACE_SHL, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_shl), /*   */ OPCC_BINARY_INPLACE, /*  */ "<<=", /*     */ "ishl", /*       */ "tp_inplace_shl", &operator_ishl),
	TYPE_OPERATOR_DECL(OPERATOR_0023_INPLACE_SHR, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_shr), /*   */ OPCC_BINARY_INPLACE, /*  */ ">>=", /*     */ "ishr", /*       */ "tp_inplace_shr", &operator_ishr),
	TYPE_OPERATOR_DECL(OPERATOR_0024_INPLACE_AND, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_and), /*   */ OPCC_BINARY_INPLACE, /*  */ "&=", /*      */ "iand", /*       */ "tp_inplace_and", &operator_iand),
	TYPE_OPERATOR_DECL(OPERATOR_0025_INPLACE_OR, /* */ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_or), /*    */ OPCC_BINARY_INPLACE, /*  */ "|=", /*      */ "ior", /*        */ "tp_inplace_or", &operator_ior),
	TYPE_OPERATOR_DECL(OPERATOR_0026_INPLACE_XOR, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_xor), /*   */ OPCC_BINARY_INPLACE, /*  */ "^=", /*      */ "ixor", /*       */ "tp_inplace_xor", &operator_ixor),
	TYPE_OPERATOR_DECL(OPERATOR_0027_INPLACE_POW, /**/ offsetof(Type, tp_math), /*  */ offsetof(struct type_math, tp_inplace_pow), /*   */ OPCC_BINARY_INPLACE, /*  */ "**=", /*     */ "ipow", /*       */ "tp_inplace_pow", &operator_ipow),
	TYPE_OPERATOR_DECL(OPERATOR_0028_HASH, /*       */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_hash), /*           */ OPCC_UNARY_UINTPTR_NX, /**/ "hash", /*    */ "hash", /*       */ "tp_hash", &operator_hash),
	TYPE_OPERATOR_DECL(OPERATOR_0029_EQ, /*         */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_eq), /*             */ OPCC_BINARY_OBJECT, /*   */ "==", /*      */ "eq", /*         */ "tp_eq", &operator_eq),
	TYPE_OPERATOR_DECL(OPERATOR_002A_NE, /*         */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_ne), /*             */ OPCC_BINARY_OBJECT, /*   */ "!=", /*      */ "ne", /*         */ "tp_ne", &operator_ne),
	TYPE_OPERATOR_DECL(OPERATOR_002B_LO, /*         */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_lo), /*             */ OPCC_BINARY_OBJECT, /*   */ "<", /*       */ "lo", /*         */ "tp_lo", &operator_lo),
	TYPE_OPERATOR_DECL(OPERATOR_002C_LE, /*         */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_le), /*             */ OPCC_BINARY_OBJECT, /*   */ "<=", /*      */ "le", /*         */ "tp_le", &operator_le),
	TYPE_OPERATOR_DECL(OPERATOR_002D_GR, /*         */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_gr), /*             */ OPCC_BINARY_OBJECT, /*   */ ">", /*       */ "gr", /*         */ "tp_gr", &operator_gr),
	TYPE_OPERATOR_DECL(OPERATOR_002E_GE, /*         */ offsetof(Type, tp_cmp), /*   */ offsetof(struct type_cmp, tp_ge), /*             */ OPCC_BINARY_OBJECT, /*   */ ">=", /*      */ "ge", /*         */ "tp_ge", &operator_ge),
	TYPE_OPERATOR_DECL(OPERATOR_002F_ITERSELF, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_iter_self), /*      */ OPCC_UNARY_OBJECT, /*    */ "iter", /*    */ "iter", /*       */ "tp_iter_self", &operator_iter),
	TYPE_OPERATOR_DECL(OPERATOR_0030_SIZE, /*       */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_size), /*           */ OPCC_UNARY_OBJECT, /*    */ "#", /*       */ "size", /*       */ "tp_size", &operator_size),
	TYPE_OPERATOR_DECL(OPERATOR_0031_CONTAINS, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_contains), /*       */ OPCC_BINARY_OBJECT, /*   */ "contains", /**/ "contains", /*   */ "tp_contains", &operator_contains),
	TYPE_OPERATOR_DECL(OPERATOR_0032_GETITEM, /*    */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_get), /*            */ OPCC_BINARY_OBJECT, /*   */ "[]", /*      */ "getitem", /*    */ "tp_get", &operator_getitem),
	TYPE_OPERATOR_DECL(OPERATOR_0033_DELITEM, /*    */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_del), /*            */ OPCC_BINARY_INT, /*      */ "del[]", /*   */ "delitem", /*    */ "tp_del", &operator_delitem),
	TYPE_OPERATOR_DECL(OPERATOR_0034_SETITEM, /*    */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_set), /*            */ OPCC_TRINARY_INT, /*     */ "[]=", /*     */ "setitem", /*    */ "tp_set", &operator_setitem),
	TYPE_OPERATOR_DECL(OPERATOR_0035_GETRANGE, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_range_get), /*      */ OPCC_TRINARY_OBJECT, /*  */ "[:]", /*     */ "getrange", /*   */ "tp_range_get", &operator_getrange),
	TYPE_OPERATOR_DECL(OPERATOR_0036_DELRANGE, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_range_del), /*      */ OPCC_TRINARY_INT, /*     */ "del[:]", /*  */ "delrange", /*   */ "tp_range_del", &operator_delrange),
	TYPE_OPERATOR_DECL(OPERATOR_0037_SETRANGE, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_range_set), /*      */ OPCC_QUATERNARY_INT, /*  */ "[:]=", /*    */ "setrange", /*   */ "tp_range_set", &operator_setrange),
	TYPE_OPERATOR_DECL(OPERATOR_0038_GETATTR, /*    */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_getattr), /*       */ OPCC_SPECIAL, /*         */ ".", /*       */ "getattr", /*    */ "tp_getattr", &operator_getattr),
	TYPE_OPERATOR_DECL(OPERATOR_0039_DELATTR, /*    */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_delattr), /*       */ OPCC_SPECIAL, /*         */ "del.", /*    */ "delattr", /*    */ "tp_delattr", &operator_delattr),
	TYPE_OPERATOR_DECL(OPERATOR_003A_SETATTR, /*    */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_setattr), /*       */ OPCC_SPECIAL, /*         */ ".=", /*      */ "setattr", /*    */ "tp_setattr", &operator_setattr),
	TYPE_OPERATOR_DECL(OPERATOR_003B_ENUMATTR, /*   */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_enumattr), /*      */ OPCC_SPECIAL, /*         */ "...", /*     */ "enumattr", /*   */ "tp_enumattr", &operator_enumattr),
	TYPE_OPERATOR_DECL(OPERATOR_003C_ENTER, /*      */ offsetof(Type, tp_with), /*  */ offsetof(struct type_with, tp_enter), /*         */ OPCC_UNARY_INT, /*       */ "enter", /*   */ "enter", /*      */ "tp_enter", &operator_enter),
	TYPE_OPERATOR_DECL(OPERATOR_003D_LEAVE, /*      */ offsetof(Type, tp_with), /*  */ offsetof(struct type_with, tp_leave), /*         */ OPCC_UNARY_INT, /*       */ "leave", /*   */ "leave", /*      */ "tp_leave", &operator_leave),
	TYPE_OPERATOR_DECL(OPERATOR_8000_VISIT, /*      */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_visit), /*                     */ OPCC_SPECIAL, /*         */ "", /*        */ "", /*           */ "tp_visit", &operator_visit),
	TYPE_OPERATOR_DECL(OPERATOR_8001_CLEAR, /*      */ offsetof(Type, tp_gc), /*    */ offsetof(struct type_gc, tp_clear), /*           */ OPCC_UNARY_VOID, /*      */ "", /*        */ "", /*           */ "tp_clear", &operator_clear),
	TYPE_OPERATOR_DECL(OPERATOR_8002_PCLEAR, /*     */ offsetof(Type, tp_gc), /*    */ offsetof(struct type_gc, tp_pclear), /*          */ OPCC_SPECIAL, /*         */ "", /*        */ "", /*           */ "tp_pclear", &operator_pclear),
	TYPE_OPERATOR_DECL(OPERATOR_8003_GETBUF, /*     */ offsetof(Type, tp_buffer), /**/ offsetof(struct type_buffer, tp_getbuf), /*      */ OPCC_SPECIAL, /*         */ "", /*        */ "", /*           */ "tp_getbuf", &operator_getbuf)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TYPE_OPERATORS_C */
