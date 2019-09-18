/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_OPERATOR_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_OPERATOR_C 1
#define _KOS_SOURCE 1

#include <deemon/HashSet.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>

DECL_BEGIN

INTDEF DREF DeeObject *DCALL string_decode(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL string_encode(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

INTDEF DREF DeeObject *DCALL DeeCodec_NormalizeName(DeeObject *__restrict name);
INTDEF unsigned int DCALL DeeCodec_GetErrorMode(char const *__restrict errors);
INTDEF DREF DeeObject *DCALL DeeCodec_DecodeIntern(DeeObject *__restrict self, DeeObject *__restrict name, unsigned int error_mode);
INTDEF DREF DeeObject *DCALL DeeCodec_EncodeIntern(DeeObject *__restrict self, DeeObject *__restrict name, unsigned int error_mode);

PRIVATE DREF DeeObject *DCALL
emulate_object_decode(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	/* Something like `"foo".encode("UTF-8")' can still be
	 * optimized at compile-time, however `"foo".encode("hex")'
	 * mustn't, because the codec is implemented externally */
	DeeObject *name;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_Unpack(argc, argv, "o|s:decode", &name, &errors) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (errors) {
		error_mode = DeeCodec_GetErrorMode(errors);
		if
			unlikely(error_mode == (unsigned int)-1)
		goto err;
	}
	return DeeCodec_DecodeIntern(self, name, error_mode);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
emulate_object_encode(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	DeeObject *name;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_Unpack(argc, argv, "o|s:encode", &name, &errors) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (errors) {
		error_mode = DeeCodec_GetErrorMode(errors);
		if
			unlikely(error_mode == (unsigned int)-1)
		goto err;
	}
	return DeeCodec_EncodeIntern(self, name, error_mode);
err:
	return NULL;
}


INTDEF DREF DeeObject *DCALL
object_id_get(DeeObject *__restrict self);


/* Don't allow member calls with these types at compile-time. */
#define IS_BLACKLISTED_BASE(self)                 \
	((self) == (DeeObject *)&DeeThread_Type ||    \
	 (self) == (DeeObject *)&DeeTraceback_Type || \
	 (self) == (DeeObject *)&DeeModule_Type)

/* Returns `ITER_DONE' if the call isn't allowed. */
PRIVATE DREF DeeObject *DCALL
emulate_method_call(DeeObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	if (DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self)) {
		/* Must emulate encode() and decode() functions, so they don't
		 * call into libcodecs, which should only be loaded at runtime!
		 * However, builtin codecs are still allowed!
		 * NOTE: Both `string' and `bytes' use the same underlying
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
	}
	return DeeObject_Call(self, argc, argv);
}

/* Returns `ITER_DONE' if the call isn't allowed. */
INTERN DREF DeeObject *DCALL
emulate_member_call(DeeObject *__restrict base,
                    DeeObject *__restrict name,
                    size_t argc, DeeObject **__restrict argv) {
#define NAME_EQ(x)                                 \
	(DeeString_SIZE(name) == COMPILER_STRLEN(x) && \
	 memcmp(DeeString_STR(name), x, sizeof(x) - sizeof(char)) == 0)
	if (DeeString_Check(base) || DeeBytes_Check(base)) {
		/* Same as the other call emulator: special
		 * handling for (string|bytes).(encode|decode) */
		if (NAME_EQ("encode"))
			return emulate_object_encode(base, argc, argv);
		if (NAME_EQ("decode"))
			return emulate_object_decode(base, argc, argv);
	}
	/* `Object.id()' should not be evaluated at compile-time! */
	if (NAME_EQ("id"))
		return ITER_DONE;
	if (IS_BLACKLISTED_BASE(base))
		return ITER_DONE;
	return DeeObject_CallAttr(base, name, argc, argv);
}


INTERN int(DCALL ast_optimize_operator)(struct ast_optimize_stack *__restrict stack,
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
	/* Since `objmethod' isn't allowed in constant expressions, but
	* since it is the gateway to all kinds of compiler optimizations,
	* such as `"foo".upper()' --> `"FOO"', as a special case we try
	* to bridge across the GETATTR operator invocation and try to
	* directly invoke the function when possible. */
	if (self->a_flag == OPERATOR_CALL &&
	    self->a_operator.o_op1 &&
	    self->a_operator.o_op0->a_type == AST_OPERATOR &&
	    self->a_operator.o_op0->a_flag == OPERATOR_GETATTR &&
	    self->a_operator.o_op0->a_operator.o_op1 &&
	    !(self->a_operator.o_op0->a_operator.o_exflag & (AST_OPERATOR_FVARARGS | AST_OPERATOR_FPOSTOP))) {
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
				for (++i; i < opcount; ++i)
					Dee_Decref(argv[i]);
				goto generic_operator_optimizations;
			}
			if (temp == CONSTEXPR_USECOPY) {
				operand = DeeObject_DeepCopy(operand);
				if
					unlikely(!operand)
				{
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
			if
				unlikely(self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP)
			{
				operator_result = DeeObject_Copy(argv[0]);
				if
					likely(operator_result)
				{
					DREF DeeObject *real_result;
					real_result = emulate_method_call(argv[0],
					                                  DeeTuple_SIZE(argv[1]),
					                                  DeeTuple_ELEM(argv[1]));
					if
						likely(real_result)
					Dee_Decref(real_result);
					else {
						Dee_Clear(operator_result);
					}
				}
			}
			else {
				operator_result = emulate_method_call(argv[0],
				                                      DeeTuple_SIZE(argv[1]),
				                                      DeeTuple_ELEM(argv[1]));
			}
			if (operator_result == ITER_DONE) {
			not_allowed:
				for (i = 0; i < opcount; ++i)
					Dee_Decref(argv[i]);
				goto done;
			}
		} else if (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) {
			/* Return a copy of the original operand. */
			operator_result = DeeObject_Copy(argv[0]);
			if
				likely(operator_result)
			{
				DREF DeeObject *real_result;
				real_result = DeeObject_InvokeOperator(argv[0], self->a_flag, opcount - 1, argv + 1);
				if
					likely(real_result)
				Dee_Decref(real_result);
				else {
					Dee_Clear(operator_result);
				}
			}
		} else {
			operator_result = DeeObject_InvokeOperator(argv[0], self->a_flag, opcount - 1, argv + 1);
		}
#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
		if (operator_result &&
		    allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
			struct opinfo *info;
			info = Dee_OperatorInfo(Dee_TYPE(argv[0]), self->a_flag);
			OPTIMIZE_VERBOSE("Reduce constant expression `%r.operator %s %R -> %r'\n",
			                 argv[0], info ? info->oi_uname : "?",
			                 self->a_flag == OPERATOR_CALL && opcount == 2
			                 ? DeeObject_NewRef(argv[1])
			                 : DeeTuple_NewVector(opcount - 1, argv + 1),
			                 operator_result);
		}
#endif /* CONFIG_HAVE_OPTIMIZE_VERBOSE */
		for (i = 0; i < opcount; ++i)
			Dee_Decref(argv[i]);
	}
	/* If the operator failed, don't do any propagation. */
set_operator_result:
	if
		unlikely(!operator_result)
	{
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
			 * >> local x = list([10,20,30]); // Optimize to `x = [10,20,30]' */
			OPTIMIZE_VERBOSE("Discard no-op cast-style function call to %k\n", function);
			/* We can simply get rid of this function call! */
			if (ast_assign(self, cast_expr))
				goto err;
			goto did_optimize;
		}
		if (cast_expr->a_type == AST_MULTIPLE &&
		    cast_expr->a_flag != AST_FMULTIPLE_KEEPLAST) {
			/* Propagate explicit cast calls to underlying sequence types:
			 * >> tuple([10,20,30]); // Optimize to `pack(10,20,30)' */
			uint16_t new_kind;
			if (function == (DeeObject *)&DeeTuple_Type)
				new_kind = AST_FMULTIPLE_TUPLE;
			else if (function == (DeeObject *)&DeeList_Type)
				new_kind = AST_FMULTIPLE_LIST;
			else if (function == (DeeObject *)&DeeHashSet_Type)
				new_kind = AST_FMULTIPLE_HASHSET;
			else if (function == (DeeObject *)&DeeDict_Type)
				new_kind = AST_FMULTIPLE_DICT;
			else {
				goto after_sequence_cast_propagation;
			}
			if (AST_FMULTIPLE_ISDICT(new_kind)) {
				if (!AST_FMULTIPLE_ISDICT(cast_expr->a_flag)) {
					/* TODO: unpack each element of `cast_expr' into a key/value
					 *       pair, and inline all of them into a new multi-branch
					 *       If this isn't possible for all branches, don't perform
					 *       the optimization.
					 * >> local x = [("foo",a),("bar",b)];
					 * >> // Optimize into this:
					 * >> local y = Dict { "foo" : a, "bar" : b }; */
					goto after_sequence_cast_propagation;
				}
			} else {
				if (AST_FMULTIPLE_ISDICT(cast_expr->a_flag)) {
					if
						unlikely((cast_expr->a_multiple.m_astc & 1) != 0)
					goto after_sequence_cast_propagation;
					/* TODO: Take every first and second element and pack them together
					 *       as tuple expression-like multi-branches.
					 * >> local x = List { "foo" : a, "bar" : b };
					 * >> // Optimize into this:
					 * >> local y = [("foo",a),("bar",b)];
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
