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
#ifndef GUARD_DEEMON_OBJECTS_TYPE_OPERATORS_C
#define GUARD_DEEMON_OBJECTS_TYPE_OPERATORS_C 1

#include <deemon/api.h>

#include <deemon/arg.h>            /* DeeArg_Unpack*, UNPuSIZ, UNPxSIZ */
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include "gc_inspect.h"
#include "type-operators.h"

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

typedef DeeTypeObject Type;

#define OPNAME(opname) "operator " opname

#define DEFINE_OPERATOR_INVOKE(name, instance_name, inherit_name)                            \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                    \
	invoke_##name(DeeTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self, \
	              size_t argc, DeeObject *const *argv, Dee_operator_t opname);               \
	PRIVATE struct Dee_operator_invoke tpconst name =                                        \
	Dee_OPERATOR_INVOKE_INIT(&invoke_##name, instance_name, inherit_name);                   \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                    \
	invoke_##name(DeeTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self, \
	              size_t argc, DeeObject *const *argv, Dee_operator_t opname)
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

PRIVATE NONNULL((1, 2, 3)) void DCALL
do_inherit_noop(DeeTypeObject *self,
                DeeTypeObject *type_type,
                struct Dee_opinfo const *info) {
	(void)self;
	(void)type_type;
	(void)info;
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
do_inherit_constructor(DeeTypeObject *self,
                       DeeTypeObject *type_type,
                       struct Dee_opinfo const *info) {
	(void)type_type;
	(void)info;
	(void)DeeType_InheritConstructors(self);
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
do_inherit_buffer(DeeTypeObject *self,
                  DeeTypeObject *type_type,
                  struct Dee_opinfo const *info) {
	(void)type_type;
	(void)info;
	(void)DeeType_InheritBuffer(self);
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
do_inherit_builtin_operator(DeeTypeObject *self,
                            DeeTypeObject *type_type,
                            struct Dee_opinfo const *info) {
	enum Dee_tno_id id = DeeType_GetTnoOfOperator(info->oi_id);
	(void)type_type;
	COMPILER_UNUSED(DeeType_GetNativeOperatorWithoutUnsupported(self, id));
}

DEFINE_OPERATOR_INVOKE(operator_constructor, &instance_ctor, &do_inherit_constructor) {
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
	err_operator_cannot_invoke(tp_self, "constructor");
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_copy, &instance_copy, &do_inherit_constructor) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("copy"));
	return DeeObject_TCopy(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_deepcopy, &instance_deepcopy, &do_inherit_constructor) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("deepcopy"));
#ifdef CONFIG_EXPERIMENTAL_SERIALIZED_DEEPCOPY
	(void)tp_self;
	return DeeObject_DeepCopy(self);
#else /* CONFIG_EXPERIMENTAL_SERIALIZED_DEEPCOPY */
	return DeeObject_TDeepCopy(tp_self, self);
#endif /* !CONFIG_EXPERIMENTAL_SERIALIZED_DEEPCOPY */
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_destructor, &instance_destructor, &do_inherit_noop) {
	(void)self;
	(void)p_self;
	(void)argc;
	(void)argv;
	(void)opname;
	err_operator_cannot_invoke(tp_self, "destructor");
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_bool, &usrtype__bool__with__BOOL, &do_inherit_builtin_operator) {
	int result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("bool"));
	result = DeeObject_TBool(tp_self, self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_next, &usrtype__iter_next__with__ITERNEXT, &do_inherit_builtin_operator) {
	DREF DeeObject *result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("next"));
	result = DeeObject_TIterNext(tp_self, self);
	if unlikely(result == ITER_DONE) {
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_call, &usrtype__call__with__CALL, &do_inherit_builtin_operator) {
	DeeObject *args, *kw = NULL;
	DREF DeeObject *result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1Or2(err, argc, argv, OPNAME("call"), &args, &kw);
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

DEFINE_OPERATOR_INVOKE(operator_float, &usrtype__double__with__FLOAT, &do_inherit_builtin_operator) {
	double result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("float"));
	if (DeeObject_TAsDouble(tp_self, self, &result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_hash, &usrtype__hash__with__HASH, &do_inherit_builtin_operator) {
	Dee_hash_t result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("hash"));
	result = DeeObject_THash(tp_self, self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getattr, &usrtype__getattr__with__GETATTR, &do_inherit_noop) {
	DeeStringObject *attr;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("getattr"), &attr);
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	return DeeObject_TGetAttr(tp_self, self, (DeeObject *)attr);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_delattr, &usrtype__delattr__with__DELATTR, &do_inherit_noop) {
	DeeStringObject *attr;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("delattr"), &attr);
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_TDelAttr(tp_self, self, (DeeObject *)attr))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_setattr, &usrtype__setattr__with__SETATTR, &do_inherit_noop) {
	DeeStringObject *attr;
	DeeObject *value;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("setattr"), &attr, &value);
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_TSetAttr(tp_self, self, (DeeObject *)attr, value))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_enumattr, &instance_iterattr, &do_inherit_noop) {
	DeeObject *args[2];
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("enumattr"));
	args[0] = (DeeObject *)tp_self;
	args[1] = self;
	return DeeObject_New(&DeeEnumAttr_Type, 2, args);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_visit, &instance_visit, &do_inherit_noop) {
	DREF GCSet *result;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("visit"));
	result = DeeGC_TNewReferred(tp_self, self);
	return Dee_AsObject(result);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_clear, &instance_clear, &do_inherit_noop) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("clear"));
	DeeObject_TClear(tp_self, self);
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_pclear, &instance_pclear, &do_inherit_noop) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("pclear") "', params: """
	unsigned int gc_prio;
""");]]]*/
	struct {
		unsigned int gc_prio;
	} args;
	DeeArg_Unpack1X(err, argc, argv, OPNAME("pclear"), &args.gc_prio, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	DeeObject_TPClear(tp_self, self, args.gc_prio);
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getbuf, NULL, &do_inherit_buffer) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("getbuf") "', params: "
	bool writable = false;
	size_t start = 0;
	size_t end = (size_t)-1;
");]]]*/
	struct {
		bool writable;
		size_t start;
		size_t end;
	} args;
	args.writable = false;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "|b" UNPuSIZ UNPxSIZ ":" OPNAME("getbuf"), &args))
		goto err;
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	return DeeObject_TBytes(tp_self, self,
	                        args.writable ? Dee_BUFFER_FWRITABLE
	                                      : Dee_BUFFER_FREADONLY,
	                        args.start, args.end);
err:
	return NULL;
}

/* >> operator str(): string;
 * >> operator str(fp: File); */
DEFINE_OPERATOR_INVOKE(operator_str, &usrtype__str__with__STR, &do_inherit_builtin_operator) {
	DeeObject *fp = NULL;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0Or1(err, argc, argv, OPNAME("str"), &fp);
	if (fp) {
		if unlikely(DeeObject_TPrint(tp_self, self, (Dee_formatprinter_t)&DeeFile_WriteAll, fp) < 0)
			goto err;
		return_none;
	}
	return DeeObject_TStr(tp_self, self);
err:
	return NULL;
}

/* >> operator repr(): string;
 * >> operator repr(fp: File); */
DEFINE_OPERATOR_INVOKE(operator_repr, &usrtype__repr__with__REPR, &do_inherit_builtin_operator) {
	DeeObject *fp = NULL;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0Or1(err, argc, argv, OPNAME("repr"), &fp);
	if (fp) {
		if unlikely(DeeObject_TPrintRepr(tp_self, self, (Dee_formatprinter_t)&DeeFile_WriteAll, fp) < 0)
			goto err;
		return_none;
	}
	return DeeObject_TRepr(tp_self, self);
err:
	return NULL;
}

/*[[[deemon
import * from deemon;
global operatorApiName: {string: string} = {
	"Eq" : "CmpEq",
	"Ne" : "CmpNe",
	"Lo" : "CmpLo",
	"Le" : "CmpLe",
	"Gr" : "CmpGr",
	"Ge" : "CmpGe",
	"Size" : "SizeOb",
};
function getGroupName(name: string) {
	return operatorApiName.get(name, name);
}

function defineUnary(name: string, nameTitle: string = none, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack0(err, argc, argv, OPNAME("{name}"));');
	print(f'	return DeeObject_T{getGroupName(nameTitle)}(tp_self, self);');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineUnaryInplace(name: string, nameTitle: string = none, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	(void)self;');
	print(f'	(void)opname;');
	print(f'	if unlikely(!p_self)');
	print(f'		goto err_requires_inplace;');
	print(f'	DeeArg_Unpack0(err, argc, argv, OPNAME("{name}"));');
	print(f'	if unlikely(DeeObject_T{getGroupName(nameTitle)}(tp_self, p_self))');
	print(f'		goto err;');
	print(f'	return_reference_(*p_self);');
	print(f'err_requires_inplace:');
	print(f'	err_operator_requires_inplace(tp_self, "{name}");');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineUnaryInt(name: string, nameTitle: string = none, returnNone: bool = false, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack0(err, argc, argv, OPNAME("{name}"));');
	print(f'	if (DeeObject_T{getGroupName(nameTitle)}(tp_self, self))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineBinary(name: string, nameTitle: string = none, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	DeeObject *other;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack1(err, argc, argv, OPNAME("{name}"), &other);');
	print(f'	return DeeObject_T{getGroupName(nameTitle)}(tp_self, self, other);');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineBinaryInt(name: string, nameTitle: string = none, returnNone: bool = false, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	DeeObject *other;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack1(err, argc, argv, OPNAME("{name}"), &other);');
	print(f'	if (DeeObject_T{getGroupName(nameTitle)}(tp_self, self, other))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineBinaryInplace(name: string, nameTitle: string = none, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__inplace_{name}__with__INPLACE_{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_i{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	DeeObject *other;');
	print(f'	(void)self;');
	print(f'	(void)opname;');
	print(f'	if unlikely(!p_self)');
	print(f'		goto err_requires_inplace;');
	print(f'	DeeArg_Unpack1(err, argc, argv, OPNAME("i{name}"), &other);');
	print(f'	if unlikely(DeeObject_T{getGroupName(nameTitle)[:-#nameTitle]}Inplace{nameTitle}(tp_self, p_self, other))');
	print(f'		goto err;');
	print(f'	return_reference_(*p_self);');
	print(f'err_requires_inplace:');
	print(f'	err_operator_requires_inplace(tp_self, "i{name}");');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineTrinary(name: string, nameTitle: string = none, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	DeeObject *a, *b;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack2(err, argc, argv, OPNAME("{name}"), &a, &b);');
	print(f'	return DeeObject_T{getGroupName(nameTitle)}(tp_self, self, a, b);');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineTrinaryInt(name: string, nameTitle: string = none, returnNone: bool = false, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	DeeObject *a, *b;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack2(err, argc, argv, OPNAME("{name}"), &a, &b);');
	print(f'	if (DeeObject_T{getGroupName(nameTitle)}(tp_self, self, a, b))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

function defineQuaternaryInt(name: string, nameTitle: string = none, returnNone: bool = false, usrtype: string = none) {
	if (nameTitle is none)
		nameTitle = name.title();
	if (usrtype is none)
		usrtype = f"usrtype__{name}__with__{name.upper()}";
	print(f'DEFINE_OPERATOR_INVOKE(operator_{name}, &{usrtype}, &do_inherit_builtin_operator) \{');
	print(f'	DeeObject *a, *b, *c;');
	print(f'	(void)p_self;');
	print(f'	(void)opname;');
	print(f'	DeeArg_Unpack3(err, argc, argv, OPNAME("{name}"), &a, &b, &c);');
	print(f'	if (DeeObject_T{getGroupName(nameTitle)}(tp_self, self, a, b, c))');
	print(f'		goto err;');
	print(f'	{returnNone ? "return_none" : "return_reference_(self)"};');
	print(f'err:');
	print(f'	return NULL;');
	print(f'\}');
	print;
}

local mathOps = ["add", "sub", "mul", "div", "mod", "shl", "shr", "and", "or", "xor", "pow"];
defineBinaryInt("assign");
defineBinaryInt("moveassign", nameTitle: "MoveAssign", usrtype: "usrtype__move_assign__with__MOVEASSIGN");
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
defineBinary("eq");
defineBinary("ne");
defineBinary("lo");
defineBinary("le");
defineBinary("gr");
defineBinary("ge");
defineUnary("iter", nameTitle: "Iter");
defineUnary("size", nameTitle: "Size", usrtype: "usrtype__sizeob__with__SIZE");
defineBinary("contains", nameTitle: "Contains");
defineBinary("getitem", nameTitle: "GetItem");
defineBinaryInt("delitem", nameTitle: "DelItem", returnNone: true);
defineTrinaryInt("setitem", nameTitle: "SetItem", returnNone: true);
defineTrinary("getrange", nameTitle: "GetRange");
defineTrinaryInt("delrange", nameTitle: "DelRange", returnNone: true);
defineQuaternaryInt("setrange", nameTitle: "SetRange", returnNone: true);
defineUnaryInt("enter", returnNone: true);
defineUnaryInt("leave", returnNone: true);
]]]*/
DEFINE_OPERATOR_INVOKE(operator_assign, &usrtype__assign__with__ASSIGN, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("assign"), &other);
	if (DeeObject_TAssign(tp_self, self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_moveassign, &usrtype__move_assign__with__MOVEASSIGN, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("moveassign"), &other);
	if (DeeObject_TMoveAssign(tp_self, self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_int, &usrtype__int__with__INT, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("int"));
	return DeeObject_TInt(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_inv, &usrtype__inv__with__INV, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("inv"));
	return DeeObject_TInv(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_pos, &usrtype__pos__with__POS, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("pos"));
	return DeeObject_TPos(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_neg, &usrtype__neg__with__NEG, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("neg"));
	return DeeObject_TNeg(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_inc, &usrtype__inc__with__INC, &do_inherit_builtin_operator) {
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack0(err, argc, argv, OPNAME("inc"));
	if unlikely(DeeObject_TInc(tp_self, p_self))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "inc");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_dec, &usrtype__dec__with__DEC, &do_inherit_builtin_operator) {
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack0(err, argc, argv, OPNAME("dec"));
	if unlikely(DeeObject_TDec(tp_self, p_self))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "dec");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_add, &usrtype__add__with__ADD, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("add"), &other);
	return DeeObject_TAdd(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_sub, &usrtype__sub__with__SUB, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("sub"), &other);
	return DeeObject_TSub(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_mul, &usrtype__mul__with__MUL, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("mul"), &other);
	return DeeObject_TMul(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_div, &usrtype__div__with__DIV, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("div"), &other);
	return DeeObject_TDiv(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_mod, &usrtype__mod__with__MOD, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("mod"), &other);
	return DeeObject_TMod(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_shl, &usrtype__shl__with__SHL, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("shl"), &other);
	return DeeObject_TShl(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_shr, &usrtype__shr__with__SHR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("shr"), &other);
	return DeeObject_TShr(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_and, &usrtype__and__with__AND, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("and"), &other);
	return DeeObject_TAnd(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_or, &usrtype__or__with__OR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("or"), &other);
	return DeeObject_TOr(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_xor, &usrtype__xor__with__XOR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("xor"), &other);
	return DeeObject_TXor(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_pow, &usrtype__pow__with__POW, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("pow"), &other);
	return DeeObject_TPow(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_iadd, &usrtype__inplace_add__with__INPLACE_ADD, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("iadd"), &other);
	if unlikely(DeeObject_TInplaceAdd(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "iadd");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_isub, &usrtype__inplace_sub__with__INPLACE_SUB, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("isub"), &other);
	if unlikely(DeeObject_TInplaceSub(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "isub");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_imul, &usrtype__inplace_mul__with__INPLACE_MUL, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("imul"), &other);
	if unlikely(DeeObject_TInplaceMul(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "imul");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_idiv, &usrtype__inplace_div__with__INPLACE_DIV, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("idiv"), &other);
	if unlikely(DeeObject_TInplaceDiv(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "idiv");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_imod, &usrtype__inplace_mod__with__INPLACE_MOD, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("imod"), &other);
	if unlikely(DeeObject_TInplaceMod(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "imod");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ishl, &usrtype__inplace_shl__with__INPLACE_SHL, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ishl"), &other);
	if unlikely(DeeObject_TInplaceShl(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ishl");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ishr, &usrtype__inplace_shr__with__INPLACE_SHR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ishr"), &other);
	if unlikely(DeeObject_TInplaceShr(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ishr");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_iand, &usrtype__inplace_and__with__INPLACE_AND, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("iand"), &other);
	if unlikely(DeeObject_TInplaceAnd(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "iand");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ior, &usrtype__inplace_or__with__INPLACE_OR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ior"), &other);
	if unlikely(DeeObject_TInplaceOr(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ior");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ixor, &usrtype__inplace_xor__with__INPLACE_XOR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ixor"), &other);
	if unlikely(DeeObject_TInplaceXor(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ixor");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ipow, &usrtype__inplace_pow__with__INPLACE_POW, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)self;
	(void)opname;
	if unlikely(!p_self)
		goto err_requires_inplace;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ipow"), &other);
	if unlikely(DeeObject_TInplacePow(tp_self, p_self, other))
		goto err;
	return_reference_(*p_self);
err_requires_inplace:
	err_operator_requires_inplace(tp_self, "ipow");
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_eq, &usrtype__eq__with__EQ, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("eq"), &other);
	return DeeObject_TCmpEq(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ne, &usrtype__ne__with__NE, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ne"), &other);
	return DeeObject_TCmpNe(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_lo, &usrtype__lo__with__LO, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("lo"), &other);
	return DeeObject_TCmpLo(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_le, &usrtype__le__with__LE, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("le"), &other);
	return DeeObject_TCmpLe(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_gr, &usrtype__gr__with__GR, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("gr"), &other);
	return DeeObject_TCmpGr(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_ge, &usrtype__ge__with__GE, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("ge"), &other);
	return DeeObject_TCmpGe(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_iter, &usrtype__iter__with__ITER, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("iter"));
	return DeeObject_TIter(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_size, &usrtype__sizeob__with__SIZE, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("size"));
	return DeeObject_TSizeOb(tp_self, self);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_contains, &usrtype__contains__with__CONTAINS, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("contains"), &other);
	return DeeObject_TContains(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getitem, &usrtype__getitem__with__GETITEM, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("getitem"), &other);
	return DeeObject_TGetItem(tp_self, self, other);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_delitem, &usrtype__delitem__with__DELITEM, &do_inherit_builtin_operator) {
	DeeObject *other;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack1(err, argc, argv, OPNAME("delitem"), &other);
	if (DeeObject_TDelItem(tp_self, self, other))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_setitem, &usrtype__setitem__with__SETITEM, &do_inherit_builtin_operator) {
	DeeObject *a, *b;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("setitem"), &a, &b);
	if (DeeObject_TSetItem(tp_self, self, a, b))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_getrange, &usrtype__getrange__with__GETRANGE, &do_inherit_builtin_operator) {
	DeeObject *a, *b;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("getrange"), &a, &b);
	return DeeObject_TGetRange(tp_self, self, a, b);
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_delrange, &usrtype__delrange__with__DELRANGE, &do_inherit_builtin_operator) {
	DeeObject *a, *b;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack2(err, argc, argv, OPNAME("delrange"), &a, &b);
	if (DeeObject_TDelRange(tp_self, self, a, b))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_setrange, &usrtype__setrange__with__SETRANGE, &do_inherit_builtin_operator) {
	DeeObject *a, *b, *c;
	(void)p_self;
	(void)opname;
	DeeArg_Unpack3(err, argc, argv, OPNAME("setrange"), &a, &b, &c);
	if (DeeObject_TSetRange(tp_self, self, a, b, c))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_enter, &usrtype__enter__with__ENTER, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("enter"));
	if (DeeObject_TEnter(tp_self, self))
		goto err;
	return_none;
err:
	return NULL;
}

DEFINE_OPERATOR_INVOKE(operator_leave, &usrtype__leave__with__LEAVE, &do_inherit_builtin_operator) {
	(void)p_self;
	(void)opname;
	DeeArg_Unpack0(err, argc, argv, OPNAME("leave"));
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
	TYPE_OPERATOR_DECL(OPERATOR_002F_ITER, /*       */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_iter), /*           */ OPCC_UNARY_OBJECT, /*    */ "iter", /*    */ "iter", /*       */ "tp_iter", &operator_iter),
	TYPE_OPERATOR_DECL(OPERATOR_0030_SIZE, /*       */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_sizeob), /*         */ OPCC_UNARY_OBJECT, /*    */ "#", /*       */ "size", /*       */ "tp_sizeob", &operator_size),
	TYPE_OPERATOR_DECL(OPERATOR_0031_CONTAINS, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_contains), /*       */ OPCC_BINARY_OBJECT, /*   */ "contains", /**/ "contains", /*   */ "tp_contains", &operator_contains),
	TYPE_OPERATOR_DECL(OPERATOR_0032_GETITEM, /*    */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_getitem), /*        */ OPCC_BINARY_OBJECT, /*   */ "[]", /*      */ "getitem", /*    */ "tp_getitem", &operator_getitem),
	TYPE_OPERATOR_DECL(OPERATOR_0033_DELITEM, /*    */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_delitem), /*        */ OPCC_BINARY_INT, /*      */ "del[]", /*   */ "delitem", /*    */ "tp_delitem", &operator_delitem),
	TYPE_OPERATOR_DECL(OPERATOR_0034_SETITEM, /*    */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_setitem), /*        */ OPCC_TRINARY_INT, /*     */ "[]=", /*     */ "setitem", /*    */ "tp_setitem", &operator_setitem),
	TYPE_OPERATOR_DECL(OPERATOR_0035_GETRANGE, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_getrange), /*       */ OPCC_TRINARY_OBJECT, /*  */ "[:]", /*     */ "getrange", /*   */ "tp_getrange", &operator_getrange),
	TYPE_OPERATOR_DECL(OPERATOR_0036_DELRANGE, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_delrange), /*       */ OPCC_TRINARY_INT, /*     */ "del[:]", /*  */ "delrange", /*   */ "tp_delrange", &operator_delrange),
	TYPE_OPERATOR_DECL(OPERATOR_0037_SETRANGE, /*   */ offsetof(Type, tp_seq), /*   */ offsetof(struct type_seq, tp_setrange), /*       */ OPCC_QUATERNARY_INT, /*  */ "[:]=", /*    */ "setrange", /*   */ "tp_setrange", &operator_setrange),
	TYPE_OPERATOR_DECL(OPERATOR_0038_GETATTR, /*    */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_getattr), /*       */ OPCC_SPECIAL, /*         */ ".", /*       */ "getattr", /*    */ "tp_getattr", &operator_getattr),
	TYPE_OPERATOR_DECL(OPERATOR_0039_DELATTR, /*    */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_delattr), /*       */ OPCC_SPECIAL, /*         */ "del.", /*    */ "delattr", /*    */ "tp_delattr", &operator_delattr),
	TYPE_OPERATOR_DECL(OPERATOR_003A_SETATTR, /*    */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_setattr), /*       */ OPCC_SPECIAL, /*         */ ".=", /*      */ "setattr", /*    */ "tp_setattr", &operator_setattr),
	TYPE_OPERATOR_DECL(OPERATOR_003B_ENUMATTR, /*   */ offsetof(Type, tp_attr), /*  */ offsetof(struct type_attr, tp_iterattr), /*      */ OPCC_SPECIAL, /*         */ "...", /*     */ "enumattr", /*   */ "tp_iterattr", &operator_enumattr),
	TYPE_OPERATOR_DECL(OPERATOR_003C_ENTER, /*      */ offsetof(Type, tp_with), /*  */ offsetof(struct type_with, tp_enter), /*         */ OPCC_UNARY_INT, /*       */ "enter", /*   */ "enter", /*      */ "tp_enter", &operator_enter),
	TYPE_OPERATOR_DECL(OPERATOR_003D_LEAVE, /*      */ offsetof(Type, tp_with), /*  */ offsetof(struct type_with, tp_leave), /*         */ OPCC_UNARY_INT, /*       */ "leave", /*   */ "leave", /*      */ "tp_leave", &operator_leave),
	TYPE_OPERATOR_DECL(OPERATOR_8000_VISIT, /*      */ OPCLASS_TYPE, /*             */ offsetof(Type, tp_visit), /*                     */ OPCC_SPECIAL, /*         */ "", /*        */ "", /*           */ "tp_visit", &operator_visit),
	TYPE_OPERATOR_DECL(OPERATOR_8001_CLEAR, /*      */ offsetof(Type, tp_gc), /*    */ offsetof(struct type_gc, tp_clear), /*           */ OPCC_UNARY_VOID, /*      */ "", /*        */ "", /*           */ "tp_clear", &operator_clear),
	TYPE_OPERATOR_DECL(OPERATOR_8002_PCLEAR, /*     */ offsetof(Type, tp_gc), /*    */ offsetof(struct type_gc, tp_pclear), /*          */ OPCC_SPECIAL, /*         */ "", /*        */ "", /*           */ "tp_pclear", &operator_pclear),
	TYPE_OPERATOR_DECL(OPERATOR_8003_GETBUF, /*     */ offsetof(Type, tp_buffer), /**/ offsetof(struct type_buffer, tp_getbuf), /*      */ OPCC_SPECIAL, /*         */ "", /*        */ "", /*           */ "tp_getbuf", &operator_getbuf)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TYPE_OPERATORS_C */
