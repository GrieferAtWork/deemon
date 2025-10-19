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
#ifndef GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C
#define GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/code.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
/**/

#include "kwlist.h"
#include "runtime_error.h"
#include "strings.h"
/**/

#include <stdint.h> /* uint16_t */

DECL_BEGIN

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_hasattr(size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("hasattr", params: "
	DeeObject *ob;
	DeeStringObject *attr;
");]]]*/
	struct {
		DeeObject *ob;
		DeeStringObject *attr;
	} args;
	DeeArg_Unpack2(err, argc, argv, "hasattr", &args.ob, &args.attr);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;
	result = DeeObject_HasAttr(args.ob, (DeeObject *)args.attr);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_hasitem(size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("hasitem", params: "
	DeeObject *ob;
	DeeObject *key;
");]]]*/
	struct {
		DeeObject *ob;
		DeeObject *key;
	} args;
	DeeArg_Unpack2(err, argc, argv, "hasitem", &args.ob, &args.key);
/*[[[end]]]*/
	result = DeeObject_HasItem(args.ob, args.key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_boundattr(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("boundattr", params: "
	DeeObject *ob;
	DeeStringObject *attr;
	bool allow_missing = true;
");]]]*/
	struct {
		DeeObject *ob;
		DeeStringObject *attr;
		bool allow_missing;
	} args;
	args.allow_missing = true;
	if (DeeArg_UnpackStruct(argc, argv, "oo|b:boundattr", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.attr, &DeeString_Type))
		goto err;
	switch (DeeObject_BoundAttr(args.ob, (DeeObject *)args.attr)) {
	default:
		if unlikely(!args.allow_missing) {
			DeeRT_ErrUnknownAttr(args.ob, args.attr, DeeRT_ATTRIBUTE_ACCESS_BOUND);
			goto err;
		}
		ATTR_FALLTHROUGH
	case Dee_BOUND_NO:
		return_false;
	case Dee_BOUND_YES:
		return_true;
	case Dee_BOUND_ERR:
		break;
	}
err:
	return NULL;
}


PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_bounditem(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("bounditem", params: "
	DeeObject *ob;
	DeeObject *key;
	bool allow_missing = true;
");]]]*/
	struct {
		DeeObject *ob;
		DeeObject *key;
		bool allow_missing;
	} args;
	args.allow_missing = true;
	if (DeeArg_UnpackStruct(argc, argv, "oo|b:bounditem", &args))
		goto err;
/*[[[end]]]*/
	switch (DeeObject_BoundItem(args.ob, args.key)) {
	default:
		if unlikely(!args.allow_missing) {
			DeeRT_ErrUnknownKey(args.ob, args.key);
			goto err;
		}
		ATTR_FALLTHROUGH
	case Dee_BOUND_NO:
		return_false;
	case Dee_BOUND_YES:
		return_true;
	case Dee_BOUND_ERR:
		break;
	}
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_compare(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *result;
	int diff;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("compare", params: "lhs,rhs");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lhs_rhs, "oo:compare", &args))
		goto err;
/*[[[end]]]*/
	diff = DeeObject_Compare(args.lhs, args.rhs);
	if unlikely(diff == Dee_COMPARE_ERR)
		goto err;
	ASSERT(diff == -1 || diff == 0 || diff == 1);
	result = DeeInt_FromSign(diff);
	return_reference_(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_equals(size_t argc, DeeObject *const *argv) {
	int diff;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("equals", params: "a,b");]]]*/
	struct {
		DeeObject *a;
		DeeObject *b;
	} args;
	DeeArg_Unpack2(err, argc, argv, "equals", &args.a, &args.b);
/*[[[end]]]*/
	diff = DeeObject_TryCompareEq(args.a, args.b);
	if unlikely(diff == Dee_COMPARE_ERR)
		goto err;
	return_bool(diff == 0);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_import(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__import__", params: "
	DeeModuleObject *base;
	DeeStringObject *name;
");]]]*/
	struct {
		DeeModuleObject *base;
		DeeStringObject *name;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__base_name, "oo:__import__", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertType(args.base, &DeeModule_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	result = DeeModule_ImportRel((DeeObject *)args.base,
	                             (DeeObject *)args.name);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_hash(size_t argc, DeeObject *const *argv) {
	Dee_ssize_t result = DeeObject_Hashv(argv, argc);
	return DeeInt_NewHash(result);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
builtin_exec_fallback(size_t argc,
                      DeeObject *const *argv,
                      DeeObject *kw) {
	DREF DeeObject *result;
	char const *usertext;
	size_t usersize;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("exec", params: "
	DeeObject *expr;
	DeeObject *globals = NULL;
");]]]*/
	struct {
		DeeObject *expr;
		DeeObject *globals;
	} args;
	args.globals = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_globals, "o|o:exec", &args))
		goto err;
/*[[[end]]]*/
	if (DeeString_Check(args.expr)) {
		usertext = DeeString_AsUtf8(args.expr);
		if unlikely(!usertext)
			goto err;
		usersize = WSTR_LENGTH(usertext);
		Dee_Incref(args.expr);
	} else if (DeeBytes_Check(args.expr)) {
		usertext = (char const *)DeeBytes_DATA(args.expr);
		usersize = DeeBytes_SIZE(args.expr);
		Dee_Incref(args.expr);
	} else {
		args.expr = DeeFile_ReadBytes(args.expr, (size_t)-1, true);
		if unlikely(!args.expr)
			goto err;
		usertext = (char const *)DeeBytes_DATA(args.expr);
		usersize = DeeBytes_SIZE(args.expr);
	}
	result = DeeExec_RunMemory(usertext,
	                           usersize,
	                           DEE_EXEC_RUNMODE_EXPR,
	                           0,
	                           NULL,
	                           0,
	                           0,
	                           NULL,
	                           args.globals,
	                           NULL,
	                           NULL);
	Dee_Decref_unlikely(args.expr);
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *strexec_module = NULL; /* import("_strexec") */
PRIVATE DREF DeeObject *strexec_exec = NULL;   /* _strexec.exec */

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t strexec_access_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#define strexec_access_lock_reading()    Dee_atomic_rwlock_reading(&strexec_access_lock)
#define strexec_access_lock_writing()    Dee_atomic_rwlock_writing(&strexec_access_lock)
#define strexec_access_lock_tryread()    Dee_atomic_rwlock_tryread(&strexec_access_lock)
#define strexec_access_lock_trywrite()   Dee_atomic_rwlock_trywrite(&strexec_access_lock)
#define strexec_access_lock_canread()    Dee_atomic_rwlock_canread(&strexec_access_lock)
#define strexec_access_lock_canwrite()   Dee_atomic_rwlock_canwrite(&strexec_access_lock)
#define strexec_access_lock_waitread()   Dee_atomic_rwlock_waitread(&strexec_access_lock)
#define strexec_access_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&strexec_access_lock)
#define strexec_access_lock_read()       Dee_atomic_rwlock_read(&strexec_access_lock)
#define strexec_access_lock_write()      Dee_atomic_rwlock_write(&strexec_access_lock)
#define strexec_access_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&strexec_access_lock)
#define strexec_access_lock_upgrade()    Dee_atomic_rwlock_upgrade(&strexec_access_lock)
#define strexec_access_lock_downgrade()  Dee_atomic_rwlock_downgrade(&strexec_access_lock)
#define strexec_access_lock_endwrite()   Dee_atomic_rwlock_endwrite(&strexec_access_lock)
#define strexec_access_lock_endread()    Dee_atomic_rwlock_endread(&strexec_access_lock)
#define strexec_access_lock_end()        Dee_atomic_rwlock_end(&strexec_access_lock)

INTERN bool DCALL clear_strexec_cache(void) {
	DREF DeeObject *mod, *exec;
	strexec_access_lock_write();
	mod  = strexec_module;
	exec = strexec_exec;
	if (!ITER_ISOK(mod)) {
		ASSERT(!ITER_ISOK(exec));
		strexec_access_lock_endwrite();
		return false;
	}
	strexec_module = NULL;
	strexec_exec   = NULL;
	strexec_access_lock_endwrite();
	Dee_Decref(mod);
	if (ITER_ISOK(exec))
		Dee_Decref(exec);
	return true;
}

PRIVATE WUNUSED DREF DeeObject *DCALL get_strexec_module(void) {
	DREF DeeObject *result;
again:
	strexec_access_lock_read();
	result = strexec_module;
	if unlikely(!result) {
		strexec_access_lock_endread();
		result = DeeModule_OpenGlobal((DeeObject *)&str__strexec, NULL, false);
		if unlikely(!ITER_ISOK(result)) {
			if (!result)
				return NULL;
		} else {
			if unlikely(DeeModule_RunInit(result) < 0) {
				Dee_Decref(result);
				return NULL;
			}
			Dee_Incref(result); /* The reference stored in `strexec_module' */
		}
		strexec_access_lock_write();
		if unlikely(atomic_read(&strexec_module)) {
			strexec_access_lock_endwrite();
			if (ITER_ISOK(result))
				Dee_Decref(result);
			goto again;
		}
		strexec_module = result; /* Inherit reference. */
		strexec_access_lock_endwrite();
	} else {
		if (result != ITER_DONE)
			Dee_Incref(result);
		strexec_access_lock_endread();
	}
	return result;
}


PRIVATE WUNUSED DREF DeeObject *DCALL
f_builtin_exec(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *exec;
	strexec_access_lock_read();
	exec = strexec_exec;
	if likely(ITER_ISOK(exec)) {
		Dee_Incref(exec);
		strexec_access_lock_endread();
do_exec:
		return DeeObject_CallKwInherited(exec, argc, argv, kw);
	}
	strexec_access_lock_endread();
	if unlikely(exec == ITER_DONE)
		goto fallback;
	/* Load the exec function. */
	{
		DREF DeeObject *module;
		module = get_strexec_module();
		if unlikely(!ITER_ISOK(module)) {
			if unlikely(!module)
				goto err;
			strexec_access_lock_write();
			exec = atomic_read(&strexec_exec);
			if (exec == NULL) {
				strexec_exec = ITER_DONE;
				strexec_access_lock_endwrite();
				goto fallback;
			}
			if unlikely(ITER_ISOK(exec)) {
				/* Shouldn't happen... */
				Dee_Incref(exec);
				strexec_access_lock_endwrite();
				goto do_exec;
			}
			strexec_access_lock_endwrite();
			goto fallback;
		}
		/* Load the exec function. */
		exec = DeeObject_GetAttr(module, (DeeObject *)&str_exec);
		Dee_Decref(module);
		if unlikely(!exec)
			goto err;
		strexec_access_lock_write();
		module = atomic_read(&strexec_exec);
		if likely(!module) {
set_exec_and_run:
			strexec_exec = exec;
			Dee_Incref(exec);
			strexec_access_lock_endwrite();
			goto do_exec;
		}
		if unlikely(module == ITER_DONE)
			goto set_exec_and_run;
		Dee_Incref(module);
		strexec_access_lock_endwrite();
		Dee_Decref(exec);
		exec = module;
		goto do_exec;
	}
fallback:
	return builtin_exec_fallback(argc, argv, kw);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
get_expression_repr(Dee_operator_t operator_name,
                    size_t argc, DeeObject *const *argv) {
	Dee_ssize_t error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeFormat_PrintOperatorRepr(&unicode_printer_print, &printer,
	                                    argv[0], operator_name,
	                                    argc - 1, argv + 1,
	                                    NULL, 0, NULL, 0);
	if unlikely(error < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

/* ASSERT(string message = "", int operator_id = -1, operator_args...) -> none;
 * NOTE: When `operator_id' is -1, ignore all remaining arguments. */
PRIVATE WUNUSED DREF DeeObject *DCALL
f_rt_assert(size_t argc, DeeObject *const *argv) {
	DeeObject *message = Dee_EmptyString;
	DeeObject *assertion_error;
	int operator_name = -1;
	if (argc) {
		message = argv[0];
		if (DeeNone_Check(message))
			message = Dee_EmptyString;
		if (DeeObject_AssertTypeExact(message, &DeeString_Type))
			goto err;
		++argv;
		--argc;
	}
	if (argc) {
		if (DeeObject_AsInt(argv[0], &operator_name))
			goto err;
		++argv;
		--argc;
	}
	if (operator_name >= 0) {
		DREF DeeObject *repr;
		repr = get_expression_repr((Dee_operator_t)operator_name, argc, argv);
		if unlikely(!repr)
			goto err;
		if (DeeString_IsEmpty(message)) {
			message = DeeString_Newf("Assertion failed: %k", repr);
		} else {
			message = DeeString_Newf("Assertion failed: %k - %k", repr, message);
		}
		Dee_Decref(repr);
	} else {
		if (DeeString_IsEmpty(message)) {
			message = DeeString_New("Assertion failed");
		} else {
			message = DeeString_Newf("Assertion failed - %k", message);
		}
	}
	if unlikely(!message)
		goto err;

	/* Construct the assertion error object. */
	assertion_error = DeeObject_New(&DeeError_AssertionError, 1, &message);
	Dee_Decref(message);
	if unlikely(!assertion_error)
		goto err;

	/* Throw the assertion error. */
	DeeError_ThrowInherited(assertion_error);
err:
	return NULL;
}


/* The compiler will generate calls to these functions during explicit
 * invocation of operators when the argument count is not known at
 * compile-time:
 * >> args = pack(10, 20);
 * >> print operator + (args...);
 */
PRIVATE WUNUSED DREF DeeObject *DCALL /* POsOrADd */
f_rt_pooad(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 1: return DeeObject_Pos(argv[0]);
	case 2: return DeeObject_Add(argv[0], argv[1]);
	default: break;
	}
	err_invalid_argc("operator +", argc, 1, 2);
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL /* NEgOrSuB */
f_rt_neosb(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 1: return DeeObject_Neg(argv[0]);
	case 2: return DeeObject_Sub(argv[0], argv[1]);
	default: break;
	}
	err_invalid_argc("operator -", argc, 1, 2);
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL /* GetItemOrSetItem */
f_rt_giosi(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 2: return DeeObject_GetItem(argv[0], argv[1]);
	case 3:
		if (DeeObject_SetItem(argv[0], argv[1], argv[2]))
			goto err;
		return_reference_(argv[2]);
	default: break;
	}
	err_invalid_argc("operator []", argc, 2, 3);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL /* GetRangeOrSetRange */
f_rt_grosr(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 3: return DeeObject_GetRange(argv[0], argv[1], argv[2]);
	case 4:
		if (DeeObject_SetRange(argv[0], argv[1], argv[2], argv[3]))
			goto err;
		return_reference_(argv[3]);
	default: break;
	}
	err_invalid_argc("operator [:]", argc, 3, 4);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL /* GetAttrOrSetAttr */
f_rt_gaosa(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 2:
		if (DeeObject_AssertTypeExact(argv[1], &DeeString_Type))
			goto err;
		return DeeObject_GetAttr(argv[0], argv[1]);
	case 3:
		if (DeeObject_AssertTypeExact(argv[1], &DeeString_Type))
			goto err;
		if (DeeObject_SetAttr(argv[0], argv[1], argv[2]))
			goto err;
		return_none;
	default: break;
	}
	err_invalid_argc("operator .", argc, 2, 3);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_rt_badcall(size_t argc, DeeObject *const *argv) {
	DeeThreadObject *ts;
	char const *function_name = NULL;
	size_t argc_cur, argc_min = 0;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("__badcall", params: "size_t argc_max");]]]*/
	struct {
		size_t argc_max;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, UNPuSIZ ":__badcall", &args))
		goto err;
/*[[[end]]]*/
	ts       = DeeThread_Self();
	argc_cur = args.argc_max;
	if likely(ts->t_execsz) {
		struct code_frame *frame = ts->t_exec;
		DeeCodeObject *code      = frame->cf_func->fo_code;
		argc_cur      = frame->cf_argc;
		argc_min      = code->co_argc_min;
		function_name = DeeCode_NAME(code);
	}
	/* Throw the invalid-argument-count error. */
	err_invalid_argc(function_name,
	                 argc_cur,
	                 argc_min,
	                 args.argc_max);
err:
	return NULL;
}


PRIVATE WUNUSED DREF DeeObject *DCALL
f_rt_roloc(size_t argc, DeeObject *const *argv) {
	DeeThreadObject *ts;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("__roloc", params: "uint16_t lid");]]]*/
	struct {
		uint16_t lid;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, UNPu16 ":__roloc", &args))
		goto err;
/*[[[end]]]*/
	ts = DeeThread_Self();
	if likely(ts->t_execsz) {
		struct code_frame *frame = ts->t_exec;
		DeeCodeObject *code      = frame->cf_func->fo_code;
		err_readonly_local(code, frame->cf_ip, args.lid);
	} else {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Cannot modify read-only local variable %" PRFu16,
		                args.lid);
	}
err:
	return NULL;
}


/* These functions are exported by the C api. */
PUBLIC DEFINE_CMETHOD(DeeBuiltin_HasAttr, &f_builtin_hasattr, METHOD_FNORMAL);     /* TODO: CONSTCALL_IF_HASATTR */
PUBLIC DEFINE_CMETHOD(DeeBuiltin_HasItem, &f_builtin_hasitem, METHOD_FNORMAL);     /* TODO: CONSTCALL_IF_HASITEM */
PUBLIC DEFINE_CMETHOD(DeeBuiltin_BoundAttr, &f_builtin_boundattr, METHOD_FNORMAL); /* TODO: CONSTCALL_IF_HASATTR */
PUBLIC DEFINE_CMETHOD(DeeBuiltin_BoundItem, &f_builtin_bounditem, METHOD_FNORMAL); /* TODO: CONSTCALL_IF_HASITEM */
PUBLIC DEFINE_KWCMETHOD(DeeBuiltin_Compare, &f_builtin_compare, METHOD_FNORMAL);   /* TODO: CONSTCALL_IF_COMPARE */
PUBLIC DEFINE_CMETHOD(DeeBuiltin_Equals, &f_builtin_equals, METHOD_FNORMAL);       /* TODO: CONSTCALL_IF_COMPARE_EQ */
PUBLIC DEFINE_KWCMETHOD(DeeBuiltin_Import, &f_builtin_import, METHOD_FNORMAL);     /* TODO: CONSTCALL_IF_MODULE_ALREADY_LOADED */
PUBLIC DEFINE_CMETHOD(DeeBuiltin_Hash, &f_builtin_hash, METHOD_FNORMAL);           /* TODO: CONSTCALL_IF_ARGS_CONSTHASH */
PUBLIC DEFINE_KWCMETHOD(DeeBuiltin_Exec, &f_builtin_exec, METHOD_FNORMAL);

/* These CMETHOD objects are exported from `deemon' with the `rt_' prefix replaced with `__'
 * HINT: These are exported using the `MODSYM_FHIDDEN' flag, so you won't see them in listings. */
INTERN DEFINE_CMETHOD(rt_assert, &f_rt_assert, METHOD_FNORETURN);
INTERN DEFINE_CMETHOD(rt_pooad, &f_rt_pooad, METHOD_FNORMAL);
INTERN DEFINE_CMETHOD(rt_neosb, &f_rt_neosb, METHOD_FNORMAL);
INTERN DEFINE_CMETHOD(rt_giosi, &f_rt_giosi, METHOD_FNORMAL);
INTERN DEFINE_CMETHOD(rt_grosr, &f_rt_grosr, METHOD_FNORMAL);
INTERN DEFINE_CMETHOD(rt_gaosa, &f_rt_gaosa, METHOD_FNORMAL);
INTERN DEFINE_CMETHOD(rt_badcall, &f_rt_badcall, METHOD_FNORETURN);
INTERN DEFINE_CMETHOD(rt_roloc, &f_rt_roloc, METHOD_FNORETURN);

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C */
