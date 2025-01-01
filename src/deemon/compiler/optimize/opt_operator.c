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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_OPERATOR_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_OPERATOR_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>

DECL_BEGIN

INTDEF WUNUSED NONNULL((1)) int
(DCALL ast_flatten_tostr)(struct ast *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_decode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_encode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCodec_NormalizeName(DeeObject *__restrict name);
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL
DeeCodec_GetErrorMode(char const *__restrict errors);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_DecodeIntern(DeeObject *self, DeeObject *name, unsigned int error_mode);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_EncodeIntern(DeeObject *self, DeeObject *name, unsigned int error_mode);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
emulate_object_decode(DeeObject *self, size_t argc, DeeObject *const *argv) {
	/* Something like `"foo".encode("UTF-8")' can still be
	 * optimized at compile-time, however `"foo".encode("hex")'
	 * mustn't, because the codec is implemented externally */
	DeeObject *name;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_Unpack(argc, argv, "o|s:decode", &name, &errors))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (errors) {
		error_mode = DeeCodec_GetErrorMode(errors);
		if unlikely(error_mode == (unsigned int)-1)
			goto err;
	}
	return DeeCodec_DecodeIntern(self, name, error_mode);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
emulate_object_encode(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_Unpack(argc, argv, "o|s:encode", &name, &errors))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (errors) {
		error_mode = DeeCodec_GetErrorMode(errors);
		if unlikely(error_mode == (unsigned int)-1)
			goto err;
	}
	return DeeCodec_EncodeIntern(self, name, error_mode);
err:
	return NULL;
}


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL object_id_get(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL string_hashed(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL string_hasutf(DeeObject *__restrict self);


/* Don't allow member calls with these types at compile-time. */
#define IS_BLACKLISTED_BASE(self)                 \
	((self) == (DeeObject *)&DeeThread_Type ||    \
	 (self) == (DeeObject *)&DeeTraceback_Type || \
	 (self) == (DeeObject *)&DeeModule_Type)

/* Returns `ITER_DONE' if the call isn't allowed. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
emulate_method_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self)) {
		/* Must emulate encode() and decode() functions, so they don't
		 * call into libcodecs, which should only be loaded at runtime!
		 * However, builtin codecs are still allowed!
		 * NOTE: Both `string' and `Bytes' use the same underlying
		 *       function in order to implement `encode' and `decode'! */
		dobjmethod_t method;
		DeeObject *meth_self = DeeObjMethod_SELF(self);
		method               = DeeObjMethod_FUNC(self);
		if (method == (dobjmethod_t)&string_encode)
			return emulate_object_encode(meth_self, argc, argv);
		if (method == (dobjmethod_t)&string_decode)
			return emulate_object_decode(meth_self, argc, argv);
		if (IS_BLACKLISTED_BASE(meth_self))
			return ITER_DONE;
	}
	if (DeeClsProperty_Check(self)) {
		dgetmethod_t get;
		get = DeeClsProperty_GET(self);
		/* `Object.id()' should not be evaluated at compile-time! */
		if (get == &object_id_get)
			return ITER_DONE;
		/* `string.__hasutf__' and `string.__hashed__' are runtime-volatile. */
		if (get == &string_hasutf || get == &string_hashed)
			return ITER_DONE;
	}
	return DeeObject_Call(self, argc, argv);
}


/* Returns `ITER_DONE' if the call isn't allowed. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
emulate_member_call(DeeObject *base, DeeObject *name,
                    size_t argc, DeeObject *const *argv) {
	if (DeeString_Check(base) || DeeBytes_Check(base)) {
		/* Same as the other call emulator: special
		 * handling for (string|bytes).(encode|decode) */
		if (DeeString_EQUALS_ASCII(name, "encode"))
			return emulate_object_encode(base, argc, argv);
		if (DeeString_EQUALS_ASCII(name, "decode"))
			return emulate_object_decode(base, argc, argv);
	}
	/* `Object.id()' should not be evaluated at compile-time! */
	if (DeeString_EQUALS_ASCII(name, "id"))
		return ITER_DONE;
	if (IS_BLACKLISTED_BASE(base))
		return ITER_DONE;
	return DeeObject_CallAttr(base, name, argc, argv);
}

/* Returns `ITER_DONE' if the call isn't allowed. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
emulate_getattr(DeeObject *base, DeeObject *name) {
	if (DeeString_Check(base)) {
		/* `string.__hasutf__' and `string.__hashed__' are runtime-volatile. */
		if (DeeString_EQUALS_ASCII(name, "__hasutf__") ||
		    DeeString_EQUALS_ASCII(name, "__hashed__"))
			return ITER_DONE;
	}
	return DeeObject_GetAttr(base, name);
}




INTDEF WUNUSED NONNULL((1)) DREF DeeStringObject *
(DCALL string_getsubstr)(DeeStringObject *__restrict self, size_t start, size_t end);
#define string_getsubstr(self, start, end) (DeeObject *)string_getsubstr((DeeStringObject *)(self), start, end)

PRIVATE NONNULL((1, 3, 4)) bool DCALL
find_string_template_insert_area(/*utf-8*/ char const *__restrict template_str,
                                 size_t argument_number,
                                 char const **p_insert_start,
                                 char const **p_insert_end) {
	for (;;) {
		char ch = *template_str++;
		if (ch == '\0')
			break;
		if (ch != '{')
			continue;
		if (*template_str == '{') {
			/* Escaped '{' */
			++template_str;
			continue;
		}
		if (argument_number) {
			--argument_number;
			continue;
		}
		*p_insert_start = template_str - 1;
		while (*template_str && *template_str != '}')
			++template_str;
		*p_insert_end = template_str + 1;
		return true;
	}
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
unicode_printer_print_brace_escaped(struct unicode_printer *__restrict self,
                                    /*utf-8*/ char const *__restrict text,
                                    size_t textlen) {
	char const *flush_start = text;
	char const *text_end    = text + textlen;
	while (text < text_end) {
		char ch = *text++;
		if (ch == '{' || ch == '}') {
			if unlikely(unicode_printer_print(self, text,
			                                  (size_t)(text_end - flush_start)) < 0)
				goto err;
			flush_start = text - 1; /* Repeat the character to escape it. */
		}
	}
	if unlikely(unicode_printer_print(self, flush_start,
	                                  (size_t)(text_end - flush_start)) < 0)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
unicode_printer_print_brace_escaped_strob(struct unicode_printer *__restrict self,
                                          DeeObject *__restrict strob) {
	char const *utf8 = DeeString_AsUtf8(strob);
	if unlikely(!utf8)
		return -1;
	return unicode_printer_print_brace_escaped(self, utf8, WSTR_LENGTH(utf8));
}


INTDEF WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
string_format(DeeStringObject *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL ast_optimize_string_format)(struct ast *ast_callattr_format,
                                   struct ast *ast_template_for_ddi,
                                   DREF DeeObject **p_template_str,
                                   struct ast *ast_args) {
	DeeObject *template_str = *p_template_str;
	struct ast **argv       = ast_args->a_multiple.m_astv;
	size_t i, argc = ast_args->a_multiple.m_astc;
	ASSERT(ast_callattr_format->a_type == AST_OPERATOR);
	ASSERT(ast_callattr_format->a_flag == OPERATOR_CALL);
	ASSERT_OBJECT_TYPE_EXACT(template_str, &DeeString_Type);
	ASSERT(ast_args->a_type == AST_MULTIPLE);
	ASSERT(AST_FMULTIPLE_ISSEQUENCE(ast_args->a_flag));

	i = 0;
again_optimize_arg_at_index_i:
	for (; i < argc; ++i) {
		struct ast *arg = argv[i];
		while (ast_isoperator1(arg, OPERATOR_STR)) {
			/* from: >> local foo = "value = {}".format({ str bar });
			 * to:   >> local foo = "value = {}".format({ bar }); */
			OPTIMIZE_VERBOSEAT(ast_callattr_format, "Optimize `\"{}\".format({ str x })' to `\"{}\".format({ x })'\n");
			if (ast_assign(arg, arg->a_operator.o_op0))
				goto err;
			++optimizer_count;
		}
		if (ast_isoperator1(arg, OPERATOR_REPR)) {
			/* from: >> "foo = {}".format({ repr bar }); // aka. f"foo = {repr bar}"
			 * to:   >> "foo = {!r}".format({ bar }); */
			char const *format_utf8;
			char const *insert_start, *insert_end;
			format_utf8 = DeeString_AsUtf8(template_str);
			if unlikely(!format_utf8)
				goto err;
			if (find_string_template_insert_area(format_utf8, i,
			                                     &insert_start,
			                                     &insert_end) &&
			    insert_start + 2 == insert_end) {
				OPTIMIZE_VERBOSEAT(arg, "Optimize `\"{}\".format({ repr x })' to `\"{!r}\".format({ x })'\n");
				DREF DeeObject *new_template_str;
				size_t len_before = (size_t)(insert_start - format_utf8);
				size_t len_after  = (size_t)((format_utf8 + WSTR_LENGTH(format_utf8)) - insert_end);
				new_template_str = DeeString_Newf("%$s{!r}%$s",
				                                  len_before, format_utf8,
				                                  len_after, insert_end);
				if unlikely(!new_template_str)
					goto err;
				Dee_Decref(*p_template_str);
				*p_template_str = new_template_str; /* Inherit reference */
				template_str    = new_template_str;
				if (ast_assign(arg, arg->a_operator.o_op0))
					goto err;
				++optimizer_count;
			}
		}
		if (arg->a_type == AST_CONSTEXPR) {
			/* from: >> local foo = "value = {}, {}".format({ 42, bar });
			 * to:   >> local foo = "value = 42, {}".format({ bar }); */
			char const *format_utf8;
			char const *insert_start, *insert_end;
			format_utf8 = DeeString_AsUtf8(template_str);
			if unlikely(!format_utf8)
				goto err;
			if (find_string_template_insert_area(format_utf8, i,
			                                     &insert_start,
			                                     &insert_end)) {
				struct unicode_printer printer;
				DREF DeeObject *new_template_str;
				DREF DeeObject *insert_obj_str;
				size_t len_before;
				size_t len_after;
				bool do_repr;
				if (insert_start + 2 == insert_end) {
					do_repr = false;
				} else if (insert_start + 4 == insert_end &&
				           insert_start[1] == '!' && insert_start[2] == 'r') {
					do_repr = true;
				} else {
					goto dont_do_inline_constant_arg_optimization;
				}
				OPTIMIZE_VERBOSEAT(arg, "Optimize `\"{}{}\".format({ \"foo\", x })' to `\"foo{}\".format({ x })'\n");
				len_before = (size_t)(insert_start - format_utf8);
				len_after  = (size_t)((format_utf8 + WSTR_LENGTH(format_utf8)) - insert_end);
				insert_obj_str = do_repr ? DeeObject_Repr(arg->a_constexpr)
				                         : DeeObject_Str(arg->a_constexpr);
				if unlikely(!insert_obj_str)
					goto err;
				unicode_printer_init(&printer);
				if unlikely(unicode_printer_print(&printer, format_utf8, len_before) < 0) {
err_inline_constexpr_printer_insert_obj_str:
					Dee_Decref(insert_obj_str);
err_inline_constexpr_printer:
					unicode_printer_fini(&printer);
					goto err;
				}
				if unlikely(unicode_printer_print_brace_escaped_strob(&printer, insert_obj_str))
					goto err_inline_constexpr_printer_insert_obj_str;
				Dee_Decref(insert_obj_str);
				if unlikely(unicode_printer_print(&printer, insert_end, len_after) < 0)
					goto err_inline_constexpr_printer;
				new_template_str = unicode_printer_pack(&printer);
				if unlikely(!new_template_str)
					goto err;
				Dee_Decref(*p_template_str);
				*p_template_str = new_template_str; /* Inherit reference */
				template_str    = new_template_str;

				/* Remove the argument at index `i' */
				ast_decref(arg);
				--argc;
				memmovedownp(&argv[i], &argv[i + 1], argc - i);
				ast_args->a_multiple.m_astc = argc;

				++optimizer_count;
				goto again_optimize_arg_at_index_i;
			}
dont_do_inline_constant_arg_optimization:
			;
		}
	}

	if (argc == 1) {
		if (DeeString_ENDSWITH_ASCII(template_str, "{}")) {
			if (DeeString_SIZE(template_str) == 2) {
				/* from: >> local foo = "{}".format({ bar });
				 * to:   >> local foo = str bar; */
				int error;
				DREF struct ast *str_arg;
				OPTIMIZE_VERBOSEAT(argv[0], "Optimize `\"{}\".format({ x })' to `str x'\n");
				str_arg = ast_operator1(OPERATOR_STR, AST_OPERATOR_FNORMAL, argv[0]);
				if unlikely(!str_arg)
					goto err;
				str_arg = ast_setddi(str_arg, &ast_template_for_ddi->a_ddi);
				error   = ast_assign(ast_callattr_format, str_arg);
				ast_decref(str_arg);
				if unlikely(error)
					goto err;
				++optimizer_count;
			} else {
				/* from: >> local foo = "value = {}".format({ bar });
				 * to:   >> local foo = "value = " + bar; */
				struct ast *ast_format_function_name;
				DREF struct ast *ast_template;
				DREF struct ast *add_ast;
				int error;
				OPTIMIZE_VERBOSEAT(argv[0], "Optimize `\"foo{}\".format({ x })' to `\"foo\" + x'\n");
				template_str = string_getsubstr(template_str, 0, DeeString_SIZE(template_str) - 2);
				if unlikely(!template_str)
					goto err;
				ast_template = ast_setddi(ast_constexpr(template_str), &ast_template_for_ddi->a_ddi);
				Dee_Decref(template_str);
				if unlikely(!ast_template)
					goto err;
				add_ast = ast_operator2(OPERATOR_ADD, AST_OPERATOR_FNORMAL, ast_template, argv[0]);
				ast_decref(ast_template);
				if unlikely(!add_ast)
					goto err;
				ast_format_function_name = ast_callattr_format->a_operator.o_op0->a_operator.o_op1;
				add_ast                  = ast_setddi(add_ast, &ast_format_function_name->a_ddi);
				error = ast_assign(ast_callattr_format, add_ast);
				ast_decref(add_ast);
				if unlikely(error)
					goto err;
				++optimizer_count;
			}
		} else if (DeeString_EQUALS_ASCII(template_str, "{!r}")) {
			/* from: >> local foo = "{!r}".format({ bar });
			 * to:   >> local foo = repr bar; */
			int error;
			DREF struct ast *str_arg;
			OPTIMIZE_VERBOSEAT(argv[0], "Optimize `\"{!r}\".format({ x })' to `repr x'\n");
			str_arg = ast_operator1(OPERATOR_REPR, AST_OPERATOR_FNORMAL, argv[0]);
			if unlikely(!str_arg)
				goto err;
			str_arg = ast_setddi(str_arg, &ast_template_for_ddi->a_ddi);
			error   = ast_assign(ast_callattr_format, str_arg);
			ast_decref(str_arg);
			if unlikely(error)
				goto err;
			++optimizer_count;
		}
	} else if (argc == 2) {
		if (DeeString_EQUALS_ASCII(template_str, "{}{}") &&
		    ast_predict_type(argv[0]) == &DeeString_Type) {
			/* from: >> local foo = "{}{}".format({ a, b });
			 * to:   >> local foo = a + b; // Only when type(a) == string */
			int error;
			DREF struct ast *add_ast;
			OPTIMIZE_VERBOSEAT(argv[0], "Optimize `\"{}{}\".format({ a, b })' (where `type a === string') to `a + b'\n");
			add_ast = ast_operator2(OPERATOR_ADD, AST_OPERATOR_FNORMAL, argv[0], argv[1]);
			if unlikely(!add_ast)
				goto err;
			add_ast = ast_setddi(add_ast, &ast_template_for_ddi->a_ddi);
			error   = ast_assign(ast_callattr_format, add_ast);
			ast_decref(add_ast);
			if unlikely(error)
				goto err;
			++optimizer_count;
		}
	}

	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize_operator)(struct ast_optimize_stack *__restrict stack,
                              struct ast *__restrict self, bool result_used) {
	unsigned int opcount;
	int temp;
	DREF DeeObject *operator_result;
	ASSERT(self->a_type == AST_OPERATOR);
	/* Only optimize sub-branches, but don't propagate constants
	 * if the branch has already been optimized before. */
	if (self->a_operator.o_exflag & AST_OPERATOR_FDONTOPT) {
		if (ast_optimize(stack, self->a_operator.o_op0, true))
			goto err;
		if (self->a_operator.o_op1) {
			if (ast_optimize(stack, self->a_operator.o_op1, true))
				goto err;
			if (self->a_operator.o_op2) {
				if (ast_optimize(stack, self->a_operator.o_op2, true))
					goto err;
				if (self->a_operator.o_op3 &&
				    ast_optimize(stack, self->a_operator.o_op3, true))
					goto err;
			}
		}
		return 0;
	}
	self->a_operator.o_exflag |= AST_OPERATOR_FDONTOPT;
	/* If the result isn't used, then we can delete the postop flag. */
	if (!result_used)
		self->a_operator.o_exflag &= ~(AST_OPERATOR_FPOSTOP);
	if (self->a_operator.o_exflag & AST_OPERATOR_FVARARGS) {
		/* TODO: Unknown varargs when their number can now be predicted. */
		return 0;
	}
	if (self->a_flag == OPERATOR_GETATTR) {
		struct ast *base = self->a_operator.o_op0;
		struct ast *attr = self->a_operator.o_op1;
		if (ast_optimize(stack, base, true))
			goto err;
		if (ast_optimize(stack, attr, true))
			goto err;
		if (attr->a_type == AST_CONSTEXPR &&
		    DeeString_Check(attr->a_constexpr)) {
			if (base->a_type == AST_CONSTEXPR) {
				/* Black-list certain attributes */
				operator_result = emulate_getattr(base->a_constexpr,
				                                  attr->a_constexpr);
				if (operator_result == ITER_DONE)
					goto done; /* lookup wasn't allowed. */
#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
				if (operator_result &&
				    allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
					OPTIMIZE_VERBOSE("Reduce constant expression `%r.%k -> %r'\n",
					                 base->a_constexpr,
					                 attr->a_constexpr,
					                 operator_result);
				}
#endif /* CONFIG_HAVE_OPTIMIZE_VERBOSE */
				opcount = 2;
				goto set_operator_result;
			}
			/* Special handling to allow propagation of `static final' member of user-defined classes:
			 * >> class MyClass1 {
			 * >>     public static final FOO = 42;
			 * >> }
			 * >> class MyClass2 {
			 * >>     public static final BAR = MyClass1;
			 * >> }
			 * >> print MyClass1.FOO;     // Can be optimized to `42'
			 * >> print MyClass2.BAR.FOO; // Can be optimized to `42'
			 * >> print MyClass2.BAR;     // Can be optimized to `MyClass1' */

			/* TODO: Must differentiate a couple of different cases here:
			 * - AST_SYM -> SYMBOL_TYPE_EXTERN             (for `MyClass.FOO')
			 * - ast_isoperator2(base, OPERATOR_GETATTR)   (for `MyClass2.BAR.FOO')
			 *
			 * Note that this doesn't yet handle the case where the class gets
			 * defined by the current module! For that, `ast_assumes' probably
			 * needs to be re-written to not track constant expressions, but
			 * instead track the ASTs that get assigned to variables (because
			 * that way, we can know if a symbol will *always* hold some class
			 * type, even if that type hasn't been created yet (because the
			 * attached AST would be of type AST_CLASS))
			 */
		}
		goto do_generic;
	}

	/* Since `ObjMethod' isn't allowed in constant expressions, but
	 * since it is the gateway to all kinds of compiler optimizations,
	 * such as `"foo".upper()' --> `"FOO"', as a special case we try
	 * to bridge across the GETATTR operator invocation and try to
	 * directly invoke the function when possible. */
	if (self->a_flag == OPERATOR_CALL && self->a_operator.o_op1 &&
	    ast_isoperator2(self->a_operator.o_op0, OPERATOR_GETATTR)) {
		struct ast *base = self->a_operator.o_op0->a_operator.o_op0;
		struct ast *name = self->a_operator.o_op0->a_operator.o_op1;
		struct ast *args = self->a_operator.o_op1;
		struct ast_optimize_stack function_base;
		function_base.os_ast  = self->a_operator.o_op0;
		function_base.os_prev = stack;
		function_base.os_used = true;
#ifdef OPTIMIZE_FASSUME
		function_base.os_assume = stack->os_assume;
#endif /* OPTIMIZE_FASSUME */
		/* Optimize the attribute name and make sure it's a constant string. */
		if (ast_optimize(&function_base, name, true))
			goto err;
		if (name->a_type == AST_CONSTEXPR && DeeString_Check(name->a_constexpr)) {
			/* Optimize the base-expression and make sure it's constant. */
			if (ast_optimize(&function_base, base, true))
				goto err;
			if (base->a_type == AST_CONSTEXPR) {
				/* Optimize the argument list and make sure it's a constant tuple. */
				if (ast_optimize(stack, args, true))
					goto err;
				if (args->a_type == AST_CONSTEXPR && DeeTuple_Check(args->a_constexpr)) {
					/* All right! everything has fallen into place, and this is
					 * a valid candidate for <getattr> -> <call> optimization. */
					operator_result = emulate_member_call(base->a_constexpr,
					                                      name->a_constexpr,
					                                      DeeTuple_SIZE(args->a_constexpr),
					                                      DeeTuple_ELEM(args->a_constexpr));
					if (operator_result == ITER_DONE)
						goto done; /* Call wasn't allowed. */
#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
					if (operator_result &&
					    allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
						OPTIMIZE_VERBOSE("Reduce constant expression `%r.%k%r -> %r'\n",
						                 base->a_constexpr, name->a_constexpr,
						                 args->a_constexpr, operator_result);
					}
#endif /* CONFIG_HAVE_OPTIMIZE_VERBOSE */
					opcount = 2;
					goto set_operator_result;
				}
			}
		}
	}

do_generic:
	opcount = 1;
	if (ast_optimize(stack, self->a_operator.o_op0, true))
		goto err;
	if (self->a_operator.o_op1) {
		++opcount;
		if (ast_optimize(stack, self->a_operator.o_op1, true))
			goto err;
		if (self->a_operator.o_op2) {
			++opcount;
			if (ast_optimize(stack, self->a_operator.o_op2, true))
				goto err;
			if (self->a_operator.o_op3) {
				++opcount;
				if (ast_optimize(stack, self->a_operator.o_op3, true))
					goto err;
			}
		}
	}

	/* Optimize stuff like `str(a, "foo", "bar", b)' into `str(a, "foobar", b)' */
	if (self->a_flag == OPERATOR_STR) {
		ASSERT(opcount == 1);
		if (ast_flatten_tostr(self->a_operator.o_op0))
			goto err;
		if (ast_predict_type(self->a_operator.o_op0) == &DeeString_Type) {
			OPTIMIZE_VERBOSE("Optimize `str x' (where `type x === string') into `x'\n");
			if (ast_assign(self, self->a_operator.o_op0))
				goto err;
			++optimizer_count;
		}
	}

	if (self->a_flag == OPERATOR_ADD) {
		ASSERT(opcount == 2);
		if (ast_isoperator1(self->a_operator.o_op1, OPERATOR_STR) &&
		    ast_predict_type(self->a_operator.o_op0) == &DeeString_Type) {
			OPTIMIZE_VERBOSE("Optimize `\"foo\" + str x' into `\"foo\" + x'\n");
			if (ast_assign(self->a_operator.o_op1,
			               self->a_operator.o_op1->a_operator.o_op0))
				goto err;
			++optimizer_count;
		}

		/* TODO: from: >> ((a) + b) + c        // where `type a === string' at comile-time
		 *       to:   >> "{}{}{}".format({ a, b, c })
		 * NOTE: Only do this when there are 3 or more operands */

		/* TODO: from: >> "a = {}, ".format({ a }) + "b = {}".format({ b })
		 *       to:   >> "a = {}, b = {}".format({ a, b }) */

		/* TODO: from: >> "a = {}, ".format({ a }) + "b = 42"
		 *       to:   >> "a = {}, b = 42".format({ a }) */

		/* TODO: from: >> "a = 42, " + "b = {}".format({ b })
		 *       to:   >> "a = 42, b = {}".format({ b }) */
	}

	if (self->a_flag == OPERATOR_CALL && self->a_operator.o_op1) {
		if (ast_isoperator2(self->a_operator.o_op0, OPERATOR_GETATTR)) {
			struct ast *base = self->a_operator.o_op0->a_operator.o_op0;
			struct ast *name = self->a_operator.o_op0->a_operator.o_op1;
			struct ast *args = self->a_operator.o_op1;
			if (name->a_type == AST_CONSTEXPR && DeeString_Check(name->a_constexpr)) {
				if (base->a_type == AST_CONSTEXPR && DeeString_Check(base->a_constexpr) &&
				    args->a_type == AST_MULTIPLE && args->a_flag == AST_FMULTIPLE_TUPLE &&
				    args->a_multiple.m_astc == 1 &&
				    args->a_multiple.m_astv[0]->a_type == AST_MULTIPLE &&
				    AST_FMULTIPLE_ISSEQUENCE(args->a_multiple.m_astv[0]->a_flag) &&
				    DeeString_EQUALS_ASCII(name->a_constexpr, "format")) {
					unsigned int old_optimizer_count = optimizer_count;
	
					/* Special optimizations for `string.format' (since
					 * that one's used to implement template-strings) */
					if (ast_optimize_string_format(self, base, &base->a_constexpr, args->a_multiple.m_astv[0]))
						goto err;
					if (old_optimizer_count != optimizer_count)
						goto done;
				}
			}
		} else if (self->a_operator.o_op0->a_type == AST_CONSTEXPR &&
		           (DeeObjMethod_Check(self->a_operator.o_op0->a_constexpr) ||
		            DeeKwObjMethod_Check(self->a_operator.o_op0->a_constexpr))) {
			struct ast *args = self->a_operator.o_op1;
			DeeObjMethodObject *objmethod = (DeeObjMethodObject *)self->a_operator.o_op0->a_constexpr;
			if (DeeString_Check(objmethod->om_this) &&
			    args->a_type == AST_MULTIPLE && args->a_flag == AST_FMULTIPLE_TUPLE &&
			    args->a_multiple.m_astc == 1 &&
			    args->a_multiple.m_astv[0]->a_type == AST_MULTIPLE &&
			    AST_FMULTIPLE_ISSEQUENCE(args->a_multiple.m_astv[0]->a_flag) &&
			    objmethod->om_func == (Dee_objmethod_t)&string_format) {
				unsigned int old_optimizer_count = optimizer_count;
				if (DeeObject_IsShared(objmethod)) {
					objmethod = (DeeObjMethodObject *)DeeObject_Copy((DeeObject *)objmethod);
					if unlikely(!objmethod)
						goto err;
					Dee_Decref(self->a_operator.o_op0->a_constexpr);
					self->a_operator.o_op0->a_constexpr = (DREF DeeObject *)objmethod; /* Inherit reference */
				}
	
				/* Special optimizations for `string.format' (since
				 * that one's used to implement template-strings) */
				if (ast_optimize_string_format(self, self, &objmethod->om_this,
				                               args->a_multiple.m_astv[0]))
					goto err;
	
				if (old_optimizer_count != optimizer_count)
					goto done;
			}
		}
	}


	/* Invoke the specified operator. */
	/* XXX: `AST_FOPERATOR_POSTOP'? */
	{
		DREF DeeObject *argv[4];
		unsigned int i = opcount;
		/* Check if we can do some constant propagation. */
		while (i--) {
			DeeObject *operand;
			if (self->a_operator_ops[i]->a_type != AST_CONSTEXPR)
				goto cleanup_operands;
			operand = self->a_operator_ops[i]->a_constexpr;
			/* Check if the operand can appear in constant expression. */
			temp = allow_constexpr(operand);
			if (temp == CONSTEXPR_ILLEGAL) {
cleanup_operands:
				++i;
				Dee_Decrefv(argv + i, opcount - i);
				goto generic_operator_optimizations;
			}
			if (temp == CONSTEXPR_USECOPY) {
				operand = DeeObject_DeepCopy(operand);
				if unlikely(!operand) {
					DeeError_Handled(ERROR_HANDLED_RESTORE);
					goto cleanup_operands;
				}
			} else {
				Dee_Incref(operand);
			}
			argv[i] = operand;
		}
		/* Special handling when performing a call operation. */
		if (self->a_flag == OPERATOR_CALL) {
			if (opcount != 2)
				goto not_allowed;
			if (!DeeTuple_Check(argv[1]))
				goto not_allowed;
			if unlikely(self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
				operator_result = DeeObject_Copy(argv[0]);
				if likely(operator_result) {
					DREF DeeObject *real_result;
					real_result = emulate_method_call(argv[0],
					                                  DeeTuple_SIZE(argv[1]),
					                                  DeeTuple_ELEM(argv[1]));
					if likely(real_result) {
						Dee_Decref(real_result);
					} else {
						Dee_Clear(operator_result);
					}
				}
			} else {
				operator_result = emulate_method_call(argv[0],
				                                      DeeTuple_SIZE(argv[1]),
				                                      DeeTuple_ELEM(argv[1]));
			}
			if (operator_result == ITER_DONE) {
not_allowed:
				Dee_Decrefv(argv, opcount);
				goto done;
			}
		} else if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
			/* Return a copy of the original operand. */
			operator_result = DeeObject_Copy(argv[0]);
			if likely(operator_result) {
				DREF DeeObject *real_result;
				real_result = DeeObject_InvokeOperator(argv[0], self->a_flag, opcount - 1, argv + 1);
				if likely(real_result) {
					Dee_Decref(real_result);
				} else {
					Dee_Clear(operator_result);
				}
			}
		} else {
			operator_result = DeeObject_InvokeOperator(argv[0], self->a_flag, opcount - 1, argv + 1);
		}
#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
		if (operator_result &&
		    allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
			struct opinfo const *info;
			info = DeeTypeType_GetOperatorById(Dee_TYPE(Dee_TYPE(argv[0])), self->a_flag);
			OPTIMIZE_VERBOSE("Reduce constant expression `%r.operator %s %R -> %r'\n",
			                 argv[0], info ? info->oi_uname : "?",
			                 self->a_flag == OPERATOR_CALL && opcount == 2
			                 ? DeeObject_NewRef(argv[1])
			                 : DeeTuple_NewVector(opcount - 1, argv + 1),
			                 operator_result);
		}
#endif /* CONFIG_HAVE_OPTIMIZE_VERBOSE */
		Dee_Decrefv(argv, opcount);
	}
	/* If the operator failed, don't do any propagation. */
set_operator_result:
	if unlikely(!operator_result) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		goto generic_operator_optimizations;
	}
	/* Check result is allowed in constant expressions. */
	temp = allow_constexpr(operator_result);
	if (temp != CONSTEXPR_ALLOWED) {
		if (temp == CONSTEXPR_ILLEGAL) {
dont_optimize_operator:
			Dee_Decref(operator_result);
			goto generic_operator_optimizations;
		}
		/* Replace with a deep copy (if shared) */
		if (DeeObject_InplaceDeepCopy(&operator_result)) {
			DeeError_Handled(ERROR_HANDLED_RESTORE);
			goto dont_optimize_operator;
		}
	}

	/* Override this branch with a constant expression `operator_result' */
	while (opcount--)
		ast_decref(self->a_operator_ops[opcount]);
	self->a_type      = AST_CONSTEXPR;
	self->a_flag      = AST_FNORMAL;
	self->a_constexpr = operator_result;
	goto did_optimize;
generic_operator_optimizations:
	if (self->a_flag == OPERATOR_CALL &&
	    self->a_operator.o_op1 &&
	    self->a_operator.o_op1->a_type == AST_MULTIPLE &&
	    self->a_operator.o_op1->a_multiple.m_astc == 1 &&
	    self->a_operator.o_op0->a_type == AST_CONSTEXPR) {
		DeeObject *function   = self->a_operator.o_op0->a_constexpr;
		struct ast *cast_expr = self->a_operator.o_op1->a_multiple.m_astv[0];
		if (has_cast_constructor(function) &&
		    ast_predict_type(cast_expr) == (DeeTypeObject *)function) {
			/* Certain types of calls can be optimized away:
			 * >> local x = List([10, 20, 30]); // Optimize to `x = [10, 20, 30]' */
			OPTIMIZE_VERBOSE("Discard no-op cast-style function call to %k\n", function);
			/* We can simply get rid of this function call! */
			if (ast_assign(self, cast_expr))
				goto err;
			goto did_optimize;
		}
		if (cast_expr->a_type == AST_MULTIPLE &&
		    cast_expr->a_flag != AST_FMULTIPLE_KEEPLAST) {
			/* Propagate explicit cast calls to underlying sequence types:
			 * >> tuple([10, 20, 30]); // Optimize to `pack(10, 20, 30)' */
			uint16_t new_kind;
			if (function == (DeeObject *)&DeeTuple_Type) {
				new_kind = AST_FMULTIPLE_TUPLE;
			} else if (function == (DeeObject *)&DeeList_Type) {
				new_kind = AST_FMULTIPLE_LIST;
			} else if (function == (DeeObject *)&DeeHashSet_Type) {
				new_kind = AST_FMULTIPLE_HASHSET;
			} else if (function == (DeeObject *)&DeeDict_Type) {
				new_kind = AST_FMULTIPLE_DICT;
			} else {
				goto after_sequence_cast_propagation;
			}
			if (AST_FMULTIPLE_ISDICT(new_kind)) {
				if (!AST_FMULTIPLE_ISDICT(cast_expr->a_flag)) {
					/* TODO: unpack each element of `cast_expr' into a key/value
					 *       pair, and inline all of them into a new multi-branch
					 *       If this isn't possible for all branches, don't perform
					 *       the optimization.
					 * >> local x = [("foo", a), ("bar", b)];
					 * >> // Optimize into this:
					 * >> local y = Dict { "foo" : a, "bar" : b }; */
					goto after_sequence_cast_propagation;
				}
			} else {
				if (AST_FMULTIPLE_ISDICT(cast_expr->a_flag)) {
					if unlikely((cast_expr->a_multiple.m_astc & 1) != 0)
						goto after_sequence_cast_propagation;
					/* TODO: Take every first and second element and pack them together
					 *       as tuple expression-like multi-branches.
					 * >> local x = List { "foo" : a, "bar" : b };
					 * >> // Optimize into this:
					 * >> local y = [("foo", a), ("bar", b)];
					 */
					goto after_sequence_cast_propagation;
				}
			}
			OPTIMIZE_VERBOSE("Propagate cast-style function call to %k "
			                 "onto expression getting casted\n",
			                 function);
			cast_expr->a_flag = new_kind;
			if (ast_assign(self, cast_expr))
				goto err;
			goto did_optimize;
		}
after_sequence_cast_propagation:
		;
	}
done:
	return 0;
did_optimize:
	++optimizer_count;
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_OPERATOR_C */
