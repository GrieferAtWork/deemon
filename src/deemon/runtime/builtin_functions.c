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
#include <deemon/callable.h>
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

// DeeBuiltin_HasAttr   TODO: CONSTCALL_IF_HASATTR
// DeeBuiltin_HasItem   TODO: CONSTCALL_IF_HASITEM
// DeeBuiltin_BoundAttr TODO: CONSTCALL_IF_HASATTR
// DeeBuiltin_BoundItem TODO: CONSTCALL_IF_HASITEM
// DeeBuiltin_Compare   TODO: CONSTCALL_IF_COMPARE
// DeeBuiltin_Equals    TODO: CONSTCALL_IF_COMPARE_EQ
// DeeBuiltin_Import    TODO: CONSTCALL_IF_MODULE_ALREADY_LOADED

/*[[[deemon (print_CMethod from rt.gen.unpack)("hasattr", """
	DeeObject *ob;
	DeeStringObject *attr;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_HasAttr");]]]*/
#define builtin_functions_hasattr_params "ob,attr:?Dstring"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_hasattr_f_impl(DeeObject *ob, DeeStringObject *attr);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions_hasattr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ob;
		DeeStringObject *attr;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "hasattr", &args, &args.ob, &args.attr);
	return builtin_functions_hasattr_f_impl(args.ob, args.attr);
err:
	return NULL;
}
PUBLIC DEFINE_CMETHOD(DeeBuiltin_HasAttr, &builtin_functions_hasattr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_hasattr_f_impl(DeeObject *ob, DeeStringObject *attr)
/*[[[end]]]*/
{
	int result;
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	result = DeeObject_HasAttr(ob, (DeeObject *)attr);
	if unlikely(Dee_HAS_ISERR(result))
		goto err;
	return_bool(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("hasitem", """
	DeeObject *ob;
	DeeObject *key;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_HasItem");]]]*/
#define builtin_functions_hasitem_params "ob,key"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_hasitem_f_impl(DeeObject *ob, DeeObject *key);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions_hasitem_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ob;
		DeeObject *key;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "hasitem", &args, &args.ob, &args.key);
	return builtin_functions_hasitem_f_impl(args.ob, args.key);
err:
	return NULL;
}
PUBLIC DEFINE_CMETHOD(DeeBuiltin_HasItem, &builtin_functions_hasitem_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_hasitem_f_impl(DeeObject *ob, DeeObject *key)
/*[[[end]]]*/
{
	int result = DeeObject_HasItem(ob, key);
	if unlikely(Dee_HAS_ISERR(result))
		goto err;
	return_bool(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("boundattr", """
	DeeObject *ob;
	DeeStringObject *attr;
	bool allow_missing = true;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_BoundAttr");]]]*/
#define builtin_functions_boundattr_params "ob,attr:?Dstring,allow_missing=!t"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_boundattr_f_impl(DeeObject *ob, DeeStringObject *attr, bool allow_missing);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions_boundattr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ob;
		DeeStringObject *attr;
		bool allow_missing;
	} args;
	args.allow_missing = true;
	if (DeeArg_UnpackStruct(argc, argv, "oo|b:boundattr", &args))
		goto err;
	return builtin_functions_boundattr_f_impl(args.ob, args.attr, args.allow_missing);
err:
	return NULL;
}
PUBLIC DEFINE_CMETHOD(DeeBuiltin_BoundAttr, &builtin_functions_boundattr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_boundattr_f_impl(DeeObject *ob, DeeStringObject *attr, bool allow_missing)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	switch (DeeObject_BoundAttr(ob, (DeeObject *)attr)) {
	default:
		if unlikely(!allow_missing) {
			DeeRT_ErrUnknownAttr(ob, attr, DeeRT_ATTRIBUTE_ACCESS_BOUND);
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


/*[[[deemon (print_CMethod from rt.gen.unpack)("bounditem", """
	DeeObject *ob;
	DeeObject *key;
	bool allow_missing = true;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_BoundItem");]]]*/
#define builtin_functions_bounditem_params "ob,key,allow_missing=!t"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_bounditem_f_impl(DeeObject *ob, DeeObject *key, bool allow_missing);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions_bounditem_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *ob;
		DeeObject *key;
		bool allow_missing;
	} args;
	args.allow_missing = true;
	if (DeeArg_UnpackStruct(argc, argv, "oo|b:bounditem", &args))
		goto err;
	return builtin_functions_bounditem_f_impl(args.ob, args.key, args.allow_missing);
err:
	return NULL;
}
PUBLIC DEFINE_CMETHOD(DeeBuiltin_BoundItem, &builtin_functions_bounditem_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_bounditem_f_impl(DeeObject *ob, DeeObject *key, bool allow_missing)
/*[[[end]]]*/
{
	switch (DeeObject_BoundItem(ob, key)) {
	default:
		if unlikely(!allow_missing) {
			DeeRT_ErrUnknownKey(ob, key);
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

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("compare", """
	DeeObject *lhs;
	DeeObject *rhs;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_Compare", defineKwList: false);]]]*/
#define builtin_functions_compare_params "lhs,rhs"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_compare_f_impl(DeeObject *lhs, DeeObject *rhs);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions_compare_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__lhs_rhs, "oo:compare", &args))
		goto err;
	return builtin_functions_compare_f_impl(args.lhs, args.rhs);
err:
	return NULL;
}
PUBLIC DEFINE_KWCMETHOD(DeeBuiltin_Compare, &builtin_functions_compare_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_compare_f_impl(DeeObject *lhs, DeeObject *rhs)
/*[[[end]]]*/
{
	DeeObject *result;
	int diff = DeeObject_Compare(lhs, rhs);
	if unlikely(diff == Dee_COMPARE_ERR)
		goto err;
	ASSERT(diff == -1 || diff == 0 || diff == 1);
	result = DeeInt_FromSign(diff);
	return_reference_(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("equals", """
	DeeObject *a;
	DeeObject *b;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_Equals");]]]*/
#define builtin_functions_equals_params "a,b"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_equals_f_impl(DeeObject *a, DeeObject *b);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions_equals_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *a;
		DeeObject *b;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "equals", &args, &args.a, &args.b);
	return builtin_functions_equals_f_impl(args.a, args.b);
err:
	return NULL;
}
PUBLIC DEFINE_CMETHOD(DeeBuiltin_Equals, &builtin_functions_equals_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions_equals_f_impl(DeeObject *a, DeeObject *b)
/*[[[end]]]*/
{
	int diff = DeeObject_TryCompareEq(a, b);
	if unlikely(diff == Dee_COMPARE_ERR)
		goto err;
	return_bool(diff == 0);
err:
	return NULL;
}

/*[[[deemon (print_KwCMethod from rt.gen.unpack)("__import__", """
	DeeModuleObject *base;
	DeeStringObject *name;
""", visi: "PUBLIC", cMethodSymbolName: "DeeBuiltin_Import", defineKwList: false);]]]*/
#define builtin_functions___import___params "base:?DModule,name:?Dstring"
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions___import___f_impl(DeeModuleObject *base, DeeStringObject *name);
PRIVATE WUNUSED DREF DeeObject *DCALL builtin_functions___import___f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeModuleObject *base;
		DeeStringObject *name;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__base_name, "oo:__import__", &args))
		goto err;
	return builtin_functions___import___f_impl(args.base, args.name);
err:
	return NULL;
}
PUBLIC DEFINE_KWCMETHOD(DeeBuiltin_Import, &builtin_functions___import___f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL builtin_functions___import___f_impl(DeeModuleObject *base, DeeStringObject *name)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	if (DeeObject_AssertType(base, &DeeModule_Type))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	result = DeeModule_ImportRel((DeeObject *)base,
	                             (DeeObject *)name);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
builtin_functions_f_builtin_hash(size_t argc, DeeObject *const *argv);
PUBLIC DEFINE_CMETHOD(DeeBuiltin_Hash, &builtin_functions_f_builtin_hash, METHOD_FNORMAL); /* TODO: CONSTCALL_IF_ARGS_CONSTHASH */
PRIVATE WUNUSED DREF DeeObject *DCALL
builtin_functions_f_builtin_hash(size_t argc, DeeObject *const *argv) {
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
f_builtin_exec(size_t argc, DeeObject *const *argv, DeeObject *kw);
PUBLIC DEFINE_KWCMETHOD(DeeBuiltin_Exec, &f_builtin_exec, METHOD_FNORMAL);
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





















/* These CMETHOD objects are exported from `deemon' with the `rt_' prefix replaced with `__'
 * HINT: These are exported using the `MODSYM_FHIDDEN' flag, so you won't see them in listings. */

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
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_assert(size_t argc, DeeObject *const *argv);
INTERN DEFINE_CMETHOD(rt_assert, &f_rt_assert, METHOD_FNORETURN);
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_assert(size_t argc, DeeObject *const *argv) {
	DeeObject *message = Dee_EmptyString;
	DeeObject *assertion_error;
	int operator_name = -1;
	if (argc) {
		message = argv[0];
		if (DeeCallable_Check(message)) {
			message = DeeObject_Call(message, 0, NULL);
			if unlikely(!message)
				goto err;
		} else {
			Dee_Incref(message);
		}
		if (DeeNone_Check(message)) {
			Dee_DecrefNokill(message);
			message = Dee_EmptyString;
			Dee_Incref(message);
		} else {
			if (DeeObject_AssertTypeExact(message, &DeeString_Type))
				goto err_message;
		}
		++argv;
		--argc;
	} else {
		Dee_Incref(message);
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
err_message:
	Dee_Decref(message);
	goto err;
}




/* The compiler will generate calls to these functions during explicit
 * invocation of operators when the argument count is not known at
 * compile-time:
 * >> args = pack(10, 20);
 * >> print operator + (args...);
 */

/* POsOrADd */
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_pooad(size_t argc, DeeObject *const *argv);
INTERN DEFINE_CMETHOD(rt_pooad, &f_rt_pooad, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_pooad(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 1: return DeeObject_Pos(argv[0]);
	case 2: return DeeObject_Add(argv[0], argv[1]);
	default: break;
	}
	err_invalid_argc("operator +", argc, 1, 2);
	return NULL;
}


/* NEgOrSuB */
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_neosb(size_t argc, DeeObject *const *argv);
INTERN DEFINE_CMETHOD(rt_neosb, &f_rt_neosb, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_neosb(size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 1: return DeeObject_Neg(argv[0]);
	case 2: return DeeObject_Sub(argv[0], argv[1]);
	default: break;
	}
	err_invalid_argc("operator -", argc, 1, 2);
	return NULL;
}


/* GetItemOrSetItem */
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_giosi(size_t argc, DeeObject *const *argv);
INTERN DEFINE_CMETHOD(rt_giosi, &f_rt_giosi, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_giosi(size_t argc, DeeObject *const *argv) {
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


/* GetRangeOrSetRange */
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_grosr(size_t argc, DeeObject *const *argv);
INTERN DEFINE_CMETHOD(rt_grosr, &f_rt_grosr, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_grosr(size_t argc, DeeObject *const *argv) {
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


/* GetAttrOrSetAttr */
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_gaosa(size_t argc, DeeObject *const *argv);
INTERN DEFINE_CMETHOD(rt_gaosa, &f_rt_gaosa, METHOD_FNORMAL);
PRIVATE WUNUSED DREF DeeObject *DCALL f_rt_gaosa(size_t argc, DeeObject *const *argv) {
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

/*[[[deemon (print_CMethod from rt.gen.unpack)("__badcall", """
	size_t argc_max
""", visi: "INTERN", cMethodSymbolName: "rt_badcall", noreturn: true);]]]*/
#define builtin_functions___badcall_params "argc_max:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL builtin_functions___badcall_f_impl(size_t argc_max);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL builtin_functions___badcall_f(DeeObject *__restrict arg0) {
	size_t argc_max;
	if (DeeObject_AsSize(arg0, &argc_max))
		goto err;
	return builtin_functions___badcall_f_impl(argc_max);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(rt_badcall, &builtin_functions___badcall_f, METHOD_FNORETURN);
FORCELOCAL WUNUSED DREF DeeObject *DCALL builtin_functions___badcall_f_impl(size_t argc_max)
/*[[[end]]]*/
{
	char const *function_name = NULL;
	size_t argc_cur = argc_max, argc_min = 0;
	DeeThreadObject *ts = DeeThread_Self();
	if likely(ts->t_execsz) {
		struct code_frame *frame = ts->t_exec;
		DeeCodeObject *code      = frame->cf_func->fo_code;
		argc_cur      = frame->cf_argc;
		argc_min      = code->co_argc_min;
		function_name = DeeCode_NAME(code);
	}
	/* Throw the invalid-argument-count error. */
	err_invalid_argc(function_name, argc_cur, argc_min, argc_max);
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("__roloc", """
	uint16_t lid;
""", visi: "INTERN", cMethodSymbolName: "rt_roloc", noreturn: true);]]]*/
#define builtin_functions___roloc_params "lid:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL builtin_functions___roloc_f_impl(uint16_t lid);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL builtin_functions___roloc_f(DeeObject *__restrict arg0) {
	uint16_t lid;
	if (DeeObject_AsUInt16(arg0, &lid))
		goto err;
	return builtin_functions___roloc_f_impl(lid);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(rt_roloc, &builtin_functions___roloc_f, METHOD_FNORETURN);
FORCELOCAL WUNUSED DREF DeeObject *DCALL builtin_functions___roloc_f_impl(uint16_t lid)
/*[[[end]]]*/
{
	DeeThreadObject *ts = DeeThread_Self();
	if likely(ts->t_execsz) {
		struct code_frame *frame = ts->t_exec;
		DeeCodeObject *code      = frame->cf_func->fo_code;
		err_readonly_local(code, frame->cf_ip, lid);
	} else {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Cannot modify read-only local variable %" PRFu16,
		                lid);
	}
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C */
