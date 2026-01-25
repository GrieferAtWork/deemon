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
#ifndef GUARD_DEX_STREXEC_LIBJIT_C
#define GUARD_DEX_STREXEC_LIBJIT_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DEFINE_KWLIST, DeeArg_UnpackStructKw */
#include <deemon/bytes.h>           /* DeeBytes* */
#include <deemon/dex.h>             /* DEX_*, Dee_DEXSYM_READONLY */
#include <deemon/error.h>           /* DeeError_*, ERROR_PRINT_DOHANDLE */
#include <deemon/file.h>            /* DeeFile_ReadBytes */
#include <deemon/module.h>          /* DeeModuleObject */
#include <deemon/object.h>
#include <deemon/objmethod.h>       /* DEFINE_KWCMETHOD */
#include <deemon/string.h>          /* DeeString_AsUtf8, DeeString_Check, WSTR_LENGTH */
#include <deemon/system-features.h> /* EOF */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_Self */

#include <stdbool.h> /* true */
#include <stddef.h>  /* NULL, size_t */

DECL_BEGIN

/* !!! THIS MODULE IS NON-STANDARD AND DRIVES THE BUILTIN `exec' FUNCTION FOR !!!
 * !!! THE GATW IMPLEMENTATION OF DEEMON                                      !!!
 * --------------------------------------------------------------------------------
 * Because this module is non-portable between deemon implementations, it's name
 * starts with a leading underscore, indicative of this fact. */


/*[[[deemon (print_KwCMethod from rt.gen.unpack)("exec", """
	DeeObject *expr:?X3?Dstring?DBytes?DFile;
	DeeObject *globals?:?M?Dstring?O;
	DeeModuleObject *base?:?DModule;
	DeeObject *import?:?DCallable;
""");]]]*/
#define libjit_exec_params "expr:?X3?Dstring?DBytes?DFile,globals?:?M?Dstring?O,base?:?DModule,import?:?DCallable"
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libjit_exec_f_impl(DeeObject *expr, DeeObject *globals, DeeModuleObject *base, DeeObject *import_);
#ifndef DEFINED_kwlist__expr_globals_base_import
#define DEFINED_kwlist__expr_globals_base_import
PRIVATE DEFINE_KWLIST(kwlist__expr_globals_base_import, { KEX("expr", 0x391ad037, 0x9928df37efec67d5), KEX("globals", 0x98e98592, 0x188be45e73afd7e), KEX("base", 0xc3cb0590, 0x56fd8eccbdfdd7a7), KEX("import", 0x1a3f5a1f, 0x5a525def3865fbed), KEND });
#endif /* !DEFINED_kwlist__expr_globals_base_import */
PRIVATE WUNUSED DREF DeeObject *DCALL libjit_exec_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *expr;
		DeeObject *globals;
		DeeModuleObject *base;
		DeeObject *import_;
	} args;
	args.globals = NULL;
	args.base = NULL;
	args.import_ = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__expr_globals_base_import, "o|ooo:exec", &args))
		goto err;
	return libjit_exec_f_impl(args.expr, args.globals, args.base, args.import_);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(libjit_exec, &libjit_exec_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL libjit_exec_f_impl(DeeObject *expr, DeeObject *globals, DeeModuleObject *base, DeeObject *import_)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	JITContext context;
	JITLexer lexer;
	char const *usertext;
	size_t usersize;
	DeeThreadObject *ts;
	lexer.jl_text      = expr;
	context.jc_import  = import_;
	context.jc_impbase = base;
	if (DeeString_Check(lexer.jl_text)) {
		usertext = DeeString_AsUtf8(lexer.jl_text);
		if unlikely(!usertext)
			goto err;
		usersize = WSTR_LENGTH(usertext);
		Dee_Incref(lexer.jl_text);
	} else if (DeeBytes_Check(lexer.jl_text)) {
		usertext = (char const *)DeeBytes_DATA(lexer.jl_text);
		usersize = DeeBytes_SIZE(lexer.jl_text);
		Dee_Incref(lexer.jl_text);
	} else {
		lexer.jl_text = DeeFile_ReadBytes(lexer.jl_text, (size_t)-1, true);
		if unlikely(!lexer.jl_text)
			goto err;
		if (DeeString_Check(lexer.jl_text)) {
			usertext = DeeString_AsUtf8(lexer.jl_text);
			if unlikely(!usertext)
				goto err_expr;
			usersize = WSTR_LENGTH(usertext);
		} else {
			usertext = (char const *)DeeBytes_DATA(lexer.jl_text);
			usersize = DeeBytes_SIZE(lexer.jl_text);
		}
	}
	ts                        = DeeThread_Self();
	context.jc_locals.otp_tab = NULL;
	context.jc_locals.otp_ind = 0;
	context.jc_globals        = globals;
	context.jc_retval         = NULL;
	context.jc_except         = ts->t_exceptsz;
	context.jc_flags          = 0;
	lexer.jl_context          = &context;
	lexer.jl_errpos           = NULL;
	JITLValue_Init(&lexer.jl_lvalue);
	JITLexer_Start(&lexer,
	               (unsigned char const *)usertext,
	               (unsigned char const *)usertext + usersize);
	{
		unsigned int was;
		result = JITLexer_EvalHybrid(&lexer, &was);
		if (lexer.jl_tok && likely(result)) {
			if (was == AST_PARSE_WASEXPR_YES) {
				/* Dangling code after expression. */
				if (result == JIT_LVALUE) {
					JITLValue_Fini(&lexer.jl_lvalue);
					JITLValue_Init(&lexer.jl_lvalue);
				} else {
					Dee_Decref(result);
				}
				result = NULL;
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Unexpected token `%$s' after expression",
				                (size_t)(lexer.jl_tokend - lexer.jl_tokstart),
				                lexer.jl_tokstart);
			} else {
				do {
					/* Multi-statement. */
					if (result == JIT_LVALUE) {
						JITLValue_Fini(&lexer.jl_lvalue);
						JITLValue_Init(&lexer.jl_lvalue);
					} else {
						Dee_Decref(result);
					}
#if 1
					result = JITLexer_EvalStatement(&lexer);
#else
					result = JITLexer_EvalHybridSecondary(&lexer, &was);
#endif
					if unlikely(!result)
						break;
				} while (lexer.jl_tok);
			}
		}
	}
	ASSERT(!result || (result == JIT_LVALUE) ==
	                  (lexer.jl_lvalue.lv_kind != JIT_LVALUE_NONE));
	/* Check if the resulting expression evaluates to an L-Value
	 * If so, unpack that l-value to access the pointed-to object. */
	if (result == JIT_LVALUE) {
		result = JITLValue_GetValue(&lexer.jl_lvalue,
		                            &context);
		JITLValue_Fini(&lexer.jl_lvalue);
		lexer.jl_lvalue.lv_kind = JIT_LVALUE_NONE;
	}
	if (context.jc_locals.otp_tab) {
		JITObjectTable_Fini(context.jc_locals.otp_tab);
		JITObjectTable_Free(context.jc_locals.otp_tab);
	}
	ASSERT(ts->t_exceptsz >= context.jc_except);
	/* Check for non-propagated exceptions. */
	if (ts->t_exceptsz > context.jc_except) {
		if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (JITCONTEXT_RETVAL_ISSET(context.jc_retval))
				Dee_Decref(context.jc_retval);
			context.jc_retval = JITCONTEXT_RETVAL_UNSET;
		}
		Dee_XClear(result);
		while (ts->t_exceptsz > context.jc_except + 1) {
			DeeError_Print("Discarding secondary error",
			               ERROR_PRINT_DOHANDLE);
		}
	}
	if likely(result) {
		ASSERT(context.jc_retval == JITCONTEXT_RETVAL_UNSET);
		if unlikely(lexer.jl_tok != TOK_EOF) {
			DeeError_Throwf(&DeeError_SyntaxError,
			                "Expected EOF but got `%$s'",
			                (size_t)(lexer.jl_end - lexer.jl_tokstart),
			                lexer.jl_tokstart);
			lexer.jl_errpos = lexer.jl_tokstart;
			Dee_Clear(result);
			goto handle_error;
		}
	} else if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
		if (JITCONTEXT_RETVAL_ISSET(context.jc_retval)) {
			result = context.jc_retval;
		} else {
			/* Exited code via unconventional means, such as `break' or `continue' */
			DeeError_Throwf(&DeeError_SyntaxError,
			                "Attempted to use `break' or `continue' outside of a loop");
			lexer.jl_errpos = lexer.jl_tokstart;
			goto handle_error;
		}
	} else {
		if (!lexer.jl_errpos)
			lexer.jl_errpos = lexer.jl_tokstart;
handle_error:
		JITLValue_Fini(&lexer.jl_lvalue);
		/* TODO: Somehow remember that the error happened at `lexer.jl_errpos' */
		;
	}
	ASSERT(!globals || context.jc_globals == globals);
	if (context.jc_globals != globals)
		Dee_Decref(context.jc_globals);
	Dee_Decref_unlikely(lexer.jl_text);
	return result;
err_expr:
	Dee_Decref(lexer.jl_text);
err:
	return NULL;
}



DEX_BEGIN
DEX_MEMBER_F("exec", &libjit_exec, Dee_DEXSYM_READONLY,
             "(expr:?X3?Dstring?DBytes?DFile,globals?:?M?Dstring?O,base?:?DModule,import?:?DCallable)->\n"
             "Execute a given expression @expr and return the result\n"
             "This function is used to implement the builtin ?Dexec function"),

/* TODO: `mode:?Dstring=!Prestricted'
 * >> One of `full', `restricted' or `pure', controlling which language
 *    features are available to the code being executed.
 *  - full:
 *     - `print' statements without a file target are compiled as follows
 *       >> print "foo"; // This...
 *       >> print globals["__stdout__"]: "foo"; // ... becomes this
 *       If no `__stdout__' global is provided, `File.stdout' is used.
 *  - restricted:
 *     - `catch' statements/expressions with interrupt capabilities are not allowed.
 *     - Recursive calls to `exec()' result in a `Error.RuntimeError.StackOverflow'
 *       being thrown immediately, thus preventing code from breaking out of the
 *       sandbox by creating another, less restrictive one.
 *     - `print' statements without a file target are compiled as follows
 *       >> print "foo"; // This...
 *       >> print globals["__stdout__"]: "foo"; // ... becomes this
 *       If no `__stdout__' global is provided, a NotImplemented error is thrown
 *     - `import' (both in expressions, as well as statements) is restricted
 *       for any arbitrary module, but is subject to the following restriction:
 *       >> // NOTE: import statements are compiled to all use the standard import
 *       >> //       expression, while access to imported modules is compiled to
 *       >> //       generate the appropriate attribute operators.
 *       >> import("foo"); // This...
 *       >> globals.get("__import__", restricted_import)("foo"); // ... becomes this
 *       >>
 *       >> // Where `restricted_import' is implemented as follows:
 *       >> //   - A function that emulates `deemon.import()', but only allows
 *       >> //     modules to be taken from a globally available mapping-like
 *       >> //     table of allowed modules
 *       >> //     >> import deemon;
 *       >> //     >> exec(get_untrusted_code(), mode: "restricted", globals: {
 *       >> //     >>     "__modules__": {
 *       >> //     >>         "deemon" : deemon
 *       >> //     >>     }
 *       >> //     >> });
 *       >> function restricted_import(name: string, base?: module): module | none {
 *       >>     local allowed;
 *       >>     try {
 *       >>        // NOTE: `globals' here is the argument passed to `exec()'
 *       >>        allowed = globals["__modules__"];
 *       >>     } catch (Error.KeyError) {
 *       >>        allowed = {
 *       >>            "deemon",
 *       >>            "codecs",
 *       >>            "threading",
 *       >>            "util",
 *       >>            "operators",
 *       >>            "rt.d200",
 *       >>        };
 *       >>        // Using the regular import
 *       >>        if (name in allowed)
 *       >>            return import(name);
 *       >>        goto restricted_module;
 *       >>     }
 *       >>     try {
 *       >>         return allowed[name];
 *       >>     } catch (Error.KeyError) {
 *       >> restricted_module:
 *       >>         throw Error.SystemError.FSError.FileNotFound("Restricted module");
 *       >>     }
 *       >> }
 *       Separately, accessing an attribute of a module has special restrictions
 *       applied when done for the exports of the builtin `deemon' module:
 *         - The following types/objects are off-limits, and access
 *           will cause an AttributeError to be thrown:
 *             - `deemon.gc'
 *             - `deemon.Thread'
 *             - `deemon.Error.SystemError'
 *             - `deemon.Error.AppExit'
 *             - `deemon.Signal.Interrupt'
 *         - `deemon.import' will cause the value of the expression
 *            `globals.get("__import__", restricted_import)' to be
 *            returned instead.
 *       Note that other ways of loading modules, such as `string.decode()'
 *       are not restricted, as them becoming unsafe would already require
 *       either a bug in their implementation, or pre-existing write-access
 *       to the deemon library path, meaning that they don't pose a security
 *       risk on their own.
 *     - Seperately, the runtime restricts access to `File.open()',
 *       `File.System' (and `File.io'), as well as `File.Buffer.sync()'
 *       Attempting to perform any of these operations will cause a
 *       `NotImplemented' error to be thrown, emulating a target system
 *       that doesn't implement user-code I/O support.
 *     - Access to attributes beginning with a leading underscore is disallowed
 *       This is done to prevent access to implementation-specific attributes that
 *       could be used to break out of the restricted-code sandbox
 *       >> import fs;
 *       >> function safe_unlink(path: string) {
 *       >>     if (!is_allowed_path(path))
 *       >>         throw "Illegal path";
 *       >>     fs.unlink(path);
 *       >> }
 *       >> exec('
 *       >>     local real_unlink = safe_unlink.__refs__[0]; // Ups...
 *       >>     local fs_module = real_unlink.__module__;    // This is bad...
 *       >>     function deltree(x) {
 *       >>         for (local y: fs_module.dir(x)) {
 *       >>             try {
 *       >>                 y = fs_module.joinpath(x, y);
 *       >>                 if (fs_module.stat.isdir(y)) {
 *       >>                     deltree(y);
 *       >>                     fs_module.rmdir(y);
 *       >>                 } else {
 *       >>                     fs_module.unlink(y);
 *       >>                 }
 *       >>             } catch (...) {
 *       >>             }
 *       >>         }
 *       >>     }
 *       >>     // Let's just delete _EVERYTHING_ _EVERYWHERE_
 *       >>     deltree("/");
 *       >>     for (local x: [:26])
 *       >>         deltree(type("").chr("A".ord() + x) + ":");
 *       >> ', mode: "pure", globals: {
 *       >>     "safe_unlink" : safe_unlink // Wasn't so safe afterall...
 *       >> });
 *       However, this can easily be prevented by disallowing access to attributes
 *       starting with a leading underscore.
 *       Without implementation-specific, underscore-accessible features, there shouldn't
 *       be any way of reversing module access from a single member, or gaining full type
 *       access without a bound instance of the associated type.
 *
 *  - pure:
 *     - `catch' statements/expressions with interrupt capabilities are not allowed.
 *     - Recursive calls to `exec()' result in a `Error.RuntimeError.StackOverflow'
 *       being thrown immediately, thus preventing code from breaking out of the
 *       sandbox by creating another.
 *     - `type' and `.class' expressions can only be used if the result is one of the following:
 *        - `deemon.Error'
 *        - `deemon.Signal'
 *        - `deemon.bool'
 *        - `deemon.string'
 *        - `deemon.Bytes'
 *        - `deemon.Tuple'
 *        - `deemon.List'
 *        - `deemon.Dict'
 *        - `deemon.HashSet'
 *        - `deemon.int'
 *        - `deemon.float'
 *        - `deemon.Object'
 *        - `deemon.Type'
 *        - `deemon.Cell'
 *        - `deemon.WeakRef'
 *        - `type(none)'
 *        - `deemon.Super'
 *        - `deemon.InstanceMethod'
 *        - `deemon.Property'
 *        - `deemon.Attribute'
 *        - `deemon.Frame'
 *        - `deemon.enumattr'
 *     - `print' statements without a file target are compiled as follows
 *       >> print "foo"; // This...
 *       >> print globals["__stdout__"]: "foo"; // ... becomes this
 *       If no `__stdout__' global is provided, a NotImplemented error is thrown
 *     - `import' (both in expressions, as well as statements) is not allowed
 *     - Attempting to use an instance of one of the following types as
 *       operands for any kind of expression will cause a NotImplemented error
 *       to be thrown, thus preventing them from ever appearing in `pure' code
 *       expressions.
 *       NOTE: An exception to this are `x is y', as well as `x === y' and `x !== y' expressions.
 *       The types are:
 *         - `deemon.Thread'     (blocking access to multi-threading)
 *         - `deemon.File'       (blocking access to `deemon.File.open()')
 *       Additionally, the following objects are disallowed:
 *         - `deemon.gc'
 *         - `deemon.enumattr'   (There is no reason for code to do this)
 *                                NOTE: This also includes `operator enumattr()'!
 *         - `deemon.Traceback'  (Tracebacks should be restricted to the invoker of the code)
 *         - `deemon.import'
 *         - `deemon.Error.SystemError'
 *         - `deemon.Error.AppExit'
 *         - `deemon.Signal.Interrupt'
 *     - Access to attributes beginning with a leading underscore is disallowed
 *       This is done to prevent access to implementation-specific attributes that
 *       could be used to break out of the pure-code sandbox
 *
 * WARNING:
 *   - None of the above mentioned restriction models can prevent executed code
 *     from interacting with arguments passed, or anything somehow reachable from
 *     them from being worked with.
 *     If you are careless and pass globals to objects that code should not be
 *     allowed to operate on, then you've only got yourself to blame.
 *     >> exec("fs.unlink('/path/to/important/file')",
 *     >>      mode: "pure",  // Pure, so nothing bad can happen (not...)
 *     >>      globals: {
 *     >>          "fs" : import("fs") // Sure. - Let's give it filesystem access
 *     >>      });
 *   - On its own, executed code can (even accidentally) cause an infinite loop
 *     Note however that every time execution jumps to a point it was already at
 *     before, interrupts are checked for the calling thread, meaning that another
 *     thread can easily set up a timeout for monitoring the exec() thread, and
 *     interrupt it if it ends up taking too long, or using too much CPU.
 *   - On its own, exec() code can create an arbitrary amount of objects, potentially
 *     allowing for memory starvation attacks by having code allocate ridiculous amounts
 *     of memory though simple interfaces such as `Bytes from deemon', or simply by
 *     doing something like `"foo" * 12345678'
 *     XXX: Add a runtime feature to allow for pre-thread redirection of heap functions,
 *          thus allowing for a custom implementation which could then set a ceiling on
 *          memory allocation
 */
DEX_MEMBER_F_NODOC("Function", &JITFunction_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("YieldFunction", &JITYieldFunction_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("YieldFunctionIterator", &JITYieldFunctionIterator_Type, Dee_DEXSYM_READONLY),
DEX_END(NULL, NULL, NULL);

DECL_END

#endif /* !GUARD_DEX_STREXEC_LIBJIT_C */
