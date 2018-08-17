/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPERATOR_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPERATOR_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/error.h>
#include <deemon/objmethod.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN

INTDEF DREF DeeObject *DCALL string_decode(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL string_encode(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);

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
 DeeObject *name; char *errors = NULL;
 unsigned int error_mode = STRING_ERROR_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"o|s:decode",&name,&errors) ||
     DeeObject_AssertTypeExact(name,&DeeString_Type))
     goto err;
 if (errors) {
  error_mode = DeeCodec_GetErrorMode(errors);
  if unlikely(error_mode == (unsigned int)-1) goto err;
 }
 return DeeCodec_DecodeIntern(self,name,error_mode);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
emulate_object_encode(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 DeeObject *name; char *errors = NULL;
 unsigned int error_mode = STRING_ERROR_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"o|s:encode",&name,&errors) ||
     DeeObject_AssertTypeExact(name,&DeeString_Type))
     goto err;
 if (errors) {
  error_mode = DeeCodec_GetErrorMode(errors);
  if unlikely(error_mode == (unsigned int)-1) goto err;
 }
 return DeeCodec_EncodeIntern(self,name,error_mode);
err:
 return NULL;
}


/* Returns `ITER_DONE' if the call isn't allowed. */
PRIVATE DREF DeeObject *DCALL
emulate_method_call(DeeObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 if (DeeObjMethod_Check(self)) {
  /* Must emulate encode() and decode() functions, so they don't
   * call into libcodecs, which should only be loaded at runtime!
   * However, builtin codecs are still allowed!
   * NOTE: Both `string' and `bytes' use the same underlying
   *       function in order to implement `encode' and `decode'! */
  dobjmethod_t method;
  method = ((DeeObjMethodObject *)self)->om_func;
  if (method == &string_encode)
      return emulate_object_encode(((DeeObjMethodObject *)self)->om_self,argc,argv);
  if (method == &string_decode)
      return emulate_object_decode(((DeeObjMethodObject *)self)->om_self,argc,argv);
 }
 return DeeObject_Call(self,argc,argv);
}

/* Returns `ITER_DONE' if the call isn't allowed. */
INTERN DREF DeeObject *DCALL
emulate_member_call(DeeObject *__restrict base,
                    DeeObject *__restrict name,
                    size_t argc, DeeObject **__restrict argv) {
#define NAME_EQ(x) \
       (DeeString_SIZE(name) == COMPILER_STRLEN(x) && \
        memcmp(DeeString_STR(name),x,sizeof(x)-sizeof(char)) == 0)
 if (DeeString_Check(base) || DeeBytes_Check(base)) {
  /* Same as the other call emulator: special
   * handling for (string|bytes).(encode|decode) */
  if (NAME_EQ("encode"))
      return emulate_object_encode(base,argc,argv);
  if (NAME_EQ("decode"))
      return emulate_object_decode(base,argc,argv);
 }
 return DeeObject_CallAttr(base,name,argc,argv);
}


INTERN int (DCALL ast_optimize_operator)(DeeAstObject *__restrict self, bool result_used) {
 unsigned int opcount; int temp;
 DREF DeeObject *operator_result;
 ASSERT(self->ast_type == AST_OPERATOR);
 /* Only optimize sub-branches, but don't propagate constants
  * if the branch has already been optimized before. */
 if (self->ast_operator.ast_exflag & AST_OPERATOR_FDONTOPT) {
  if (ast_optimize(self->ast_operator.ast_opa,true)) goto err;
  if (self->ast_operator.ast_opb) {
   if (ast_optimize(self->ast_operator.ast_opb,true)) goto err;
   if (self->ast_operator.ast_opc) {
    if (ast_optimize(self->ast_operator.ast_opc,true)) goto err;
    if (self->ast_operator.ast_opd &&
        ast_optimize(self->ast_operator.ast_opd,true)) goto err;
   }
  }
  return 0;
 }
 self->ast_operator.ast_exflag |= AST_OPERATOR_FDONTOPT;
 /* If the result isn't used, then we can delete the postop flag. */
 if (!result_used)
      self->ast_operator.ast_exflag &= ~(AST_OPERATOR_FPOSTOP);
 if (self->ast_operator.ast_exflag & AST_OPERATOR_FVARARGS) {
  /* TODO: Unknown varargs when their number can now be predicted. */
  return 0;
 }
 /* Since `objmethod' isn't allowed in constant expressions, but
  * since it is the gateway to all kinds of compiler optimizations,
  * such as `"foo".upper()' --> `"FOO"', as a special case we try
  * to bridge across the GETATTR operator invocation and try to
  * directly invoke the function when possible. */
 if (self->ast_flag == OPERATOR_CALL &&
     self->ast_operator.ast_opa &&  self->ast_operator.ast_opb &&
    !self->ast_operator.ast_opc && !self->ast_operator.ast_opd &&
     self->ast_operator.ast_opa->ast_type == AST_OPERATOR &&
     self->ast_operator.ast_opa->ast_flag == OPERATOR_GETATTR &&
     self->ast_operator.ast_opa->ast_operator.ast_opa &&
     self->ast_operator.ast_opa->ast_operator.ast_opb &&
    !self->ast_operator.ast_opa->ast_operator.ast_opc &&
    !self->ast_operator.ast_opa->ast_operator.ast_opd) {
  DeeAstObject *base = self->ast_operator.ast_opa->ast_operator.ast_opa;
  DeeAstObject *name = self->ast_operator.ast_opa->ast_operator.ast_opb;
  DeeAstObject *args = self->ast_operator.ast_opb;
  /* Optimize the attribute name and make sure it's a constant string. */
  if (ast_optimize(name,true)) goto err;
  if (name->ast_type == AST_CONSTEXPR && DeeString_Check(name->ast_constexpr)) {
   /* Optimize the base-expression and make sure it's constant. */
   if (ast_optimize(base,true)) goto err;
   if (base->ast_type == AST_CONSTEXPR) {
    /* Optimize the argument list and make sure it's a constant tuple. */
    if (ast_optimize(args,true)) goto err;
    if (args->ast_type == AST_CONSTEXPR && DeeTuple_Check(args->ast_constexpr)) {
     /* All right! everything has fallen into place, and this is
      * a valid candidate for <getattr> -> <call> optimization. */
     operator_result = emulate_member_call(base->ast_constexpr,
                                           name->ast_constexpr,
                                           DeeTuple_SIZE(args->ast_constexpr),
                                           DeeTuple_ELEM(args->ast_constexpr));
     if (operator_result == ITER_DONE)
         goto done; /* Call wasn't allowed. */
#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
     if (operator_result &&
         allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
      OPTIMIZE_VERBOSE("Reduce constant expression `%r.%k%r -> %r'\n",
                       base->ast_constexpr,name->ast_constexpr,
                       args->ast_constexpr,operator_result);
     }
#endif
     opcount = 2;
     goto set_operator_result;
    }
   }
  }
 }

 opcount = 1;
 if (ast_optimize(self->ast_operator.ast_opa,true)) goto err;
 if (self->ast_operator.ast_opb) {
  ++opcount;
  if (ast_optimize(self->ast_operator.ast_opb,true)) goto err;
  if (self->ast_operator.ast_opc) {
   ++opcount;
   if (ast_optimize(self->ast_operator.ast_opc,true)) goto err;
   if (self->ast_operator.ast_opd) {
    ++opcount;
    if (ast_optimize(self->ast_operator.ast_opd,true)) goto err;
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
   if (self->ast_operator_ops[i]->ast_type != AST_CONSTEXPR)
       goto cleanup_operands;
   operand = self->ast_operator_ops[i]->ast_constexpr;
   /* Check if the operand can appear in constant expression. */
   temp = allow_constexpr(operand);
   if (temp == CONSTEXPR_ILLEGAL) {
cleanup_operands:
    for (++i; i < opcount; ++i) Dee_Decref(argv[i]);
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
  if (self->ast_flag == OPERATOR_CALL) {
   if (opcount != 2) goto not_allowed;
   if (!DeeTuple_Check(argv[1])) goto not_allowed;
   if unlikely(self->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) {
    operator_result = DeeObject_Copy(argv[0]);
    if likely(operator_result) {
     DREF DeeObject *real_result;
     real_result = emulate_method_call(argv[0],
                                       DeeTuple_SIZE(argv[1]),
                                       DeeTuple_ELEM(argv[1]));
     if likely(real_result)
        Dee_Decref(real_result);
     else {
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
    for (i = 0; i < opcount; ++i)
         Dee_Decref(argv[i]);
    goto done;
   }
  } else if (self->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) {
   /* Return a copy of the original operand. */
   operator_result = DeeObject_Copy(argv[0]);
   if likely(operator_result) {
    DREF DeeObject *real_result;
    real_result = DeeObject_InvokeOperator(argv[0],self->ast_flag,opcount-1,argv+1);
    if likely(real_result)
       Dee_Decref(real_result);
    else {
     Dee_Clear(operator_result);
    }
   }
  } else {
   operator_result = DeeObject_InvokeOperator(argv[0],self->ast_flag,opcount-1,argv+1);
  }
#ifdef HAVE_VERBOSE
  if (operator_result &&
      allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
   struct opinfo *info;
   info = Dee_OperatorInfo(Dee_TYPE(argv[0]),self->ast_flag);
   OPTIMIZE_VERBOSE("Reduce constant expression `%r.operator %s %R -> %r'\n",
                     argv[0],info ? info->oi_uname : "?",
                     self->ast_flag == OPERATOR_CALL && opcount == 2
                   ? DeeObject_NewRef(argv[1])
                   : DeeTuple_NewVector(opcount-1,argv+1),
                     operator_result);
  }
#endif
  for (i = 0; i < opcount; ++i)
       Dee_Decref(argv[i]);
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
 while (opcount--) Dee_Decref(self->ast_operator_ops[opcount]);
 self->ast_type      = AST_CONSTEXPR;
 self->ast_flag      = AST_FNORMAL;
 self->ast_constexpr = operator_result;
 goto did_optimize;
generic_operator_optimizations:
 if (self->ast_flag == OPERATOR_CALL &&
     self->ast_operator.ast_opb &&
     self->ast_operator.ast_opb->ast_type == AST_MULTIPLE &&
     self->ast_operator.ast_opb->ast_multiple.ast_exprc == 1 &&
     self->ast_operator.ast_opa->ast_type == AST_CONSTEXPR) {
  /* Certain types of calls can be optimized away:
   * >> local x = list([10,20,30]); // Optimize to `x = [10,20,30]' */
  DeeObject *function = self->ast_operator.ast_opa->ast_constexpr;
  DeeAstObject *cast_expr = self->ast_operator.ast_opb->ast_multiple.ast_exprv[0];
  if (has_cast_constructor(function) &&
      ast_predict_type(cast_expr) == (DeeTypeObject *)function) {
   OPTIMIZE_VERBOSE("Discard no-op cast-style function call to %k\n",function);
   /* We can simply get rid of this function call! */
   if (ast_assign(self,cast_expr)) goto err;
   return 0;
  }
  /* TODO: Propagate explicit cast calls to underlying sequence types:
   * >> tuple([10,20,30]); // Optimize to `pack(10,20,30)' */
 }
did_optimize:
 ++optimizer_count;
 return 0;
done:
 return 0;
err:
 return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPERATOR_C */
