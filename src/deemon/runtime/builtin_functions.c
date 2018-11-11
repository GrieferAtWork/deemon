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
#ifndef GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C
#define GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/super.h>
#include <deemon/string.h>
#include <deemon/format.h>
#include <deemon/objmethod.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/arg.h>

#include "runtime_error.h"

DECL_BEGIN

PRIVATE DREF DeeObject *DCALL
f_builtin_get_documentation(size_t argc, DeeObject **__restrict argv) {
 DeeObject *self,*attr = NULL;
 if (DeeArg_Unpack(argc,argv,"o|o:docfor",&self,&attr))
     goto err;
 if (!attr)
     return DeeObject_Doc(self);
 if (DeeObject_AssertTypeExact(attr,&DeeString_Type))
     goto err;
 return DeeObject_DocAttr(self,attr);
err:
 return NULL;
}
INTERN DEFINE_CMETHOD(builtin_get_documentation,&f_builtin_get_documentation);

PRIVATE DREF DeeObject *DCALL
f_builtin_hasattr(size_t argc, DeeObject **__restrict argv) {
 DeeObject *self,*attr; int result;
 if (DeeArg_Unpack(argc,argv,"oo:hasattr",&self,&attr))
     goto err;
 if (DeeObject_AssertTypeExact(attr,&DeeString_Type))
     goto err;
 result = DeeObject_HasAttr(self,attr);
 if unlikely(result < 0)
    goto err;
 return_bool_(result);
err:
 return NULL;
}
INTERN DEFINE_CMETHOD(builtin_hasattr,&f_builtin_hasattr);

PRIVATE DREF DeeObject *DCALL
f_builtin_boundattr(size_t argc, DeeObject **__restrict argv) {
 DeeObject *self,*attr; bool allow_missing = true;
 if (DeeArg_Unpack(argc,argv,"oo|b:boundattr",&self,&attr,&allow_missing))
     goto err;
 if (DeeObject_AssertTypeExact(attr,&DeeString_Type))
     goto err;
 switch (DeeObject_BoundAttr(self,attr)) {
 case 0: return_false;
 case 1: return_true;
 case -1: break; /* Error */
 default:
  if (allow_missing)
      return_false; /* Unknown attributes are unbound. */
  err_unknown_attribute(DeeObject_Class(self),
                        DeeString_STR(attr),
                        ATTR_ACCESS_GET);
  break;
 }
err:
 return NULL;
}
INTERN DEFINE_CMETHOD(builtin_boundattr,&f_builtin_boundattr);

PRIVATE DREF DeeObject *DCALL
f_builtin_bounditem(size_t argc, DeeObject **__restrict argv) {
 DeeObject *self,*key; bool allow_missing = true;
 if (DeeArg_Unpack(argc,argv,"oo|b:bounditem",&self,&key,&allow_missing))
     goto err;
 switch (DeeObject_BoundItem(self,key,allow_missing)) {
 default: return_false;
 case 1: return_true;
 case -1: break; /* Error */
 }
err:
 return NULL;
}
INTERN DEFINE_CMETHOD(builtin_bounditem,&f_builtin_bounditem);

PRIVATE DREF DeeObject *DCALL
f_builtin_import(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DREF DeeObject *result;
 DeeObject *module_name,*base = NULL;
 PRIVATE struct keyword kwlist[] = { K(name), K(base), KEND };
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"o|o:import",&module_name,&base))
     goto err;
 if (DeeObject_AssertTypeExact(module_name,&DeeString_Type))
     goto err;
 if (base) {
  if (DeeObject_AssertType(module_name,&DeeModule_Type))
      goto err;
  result = DeeModule_ImportRel(base,module_name,NULL,true);
 } else {
  result = DeeModule_Import(module_name,NULL,true);
 }
 if unlikely(!result)
    goto err;
 if unlikely(DeeModule_RunInit(result) < 0)
    goto err_r;
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}
INTERN DEFINE_KWCMETHOD(builtin_import,&f_builtin_import);


PRIVATE DREF DeeObject *DCALL
get_expression_repr(uint16_t operator_name,
                    size_t argc, DeeObject **__restrict argv) {
 struct opinfo *info;
 struct unicode_printer printer; size_t i;
 info = Dee_OperatorInfo(argc ? Dee_TYPE(argv[0]) : NULL,
                         operator_name);
 if (!info) goto fallback;
 if (argc == 1) {
  /* Generic unary operator. */
  return DeeString_Newf("%s%r",info->oi_uname,argv[0]);
 }
 if (argc == 2) {
  if (operator_name == OPERATOR_GETATTR) {
   if (DeeString_Check(argv[1]))
       return DeeString_Newf("%r.%k",argv[0],argv[1]);
  } else if (operator_name == OPERATOR_DELATTR) {
   if (DeeString_Check(argv[1]))
       return DeeString_Newf("del %r.%k",argv[0],argv[1]);
  } else if (operator_name == OPERATOR_GETITEM) {
   return DeeString_Newf("%r[%r]",argv[0],argv[1]);
  } else if (operator_name == OPERATOR_DELITEM) {
   return DeeString_Newf("del %r[%r]",argv[0],argv[1]);
  } else if (operator_name == OPERATOR_CONTAINS) {
   return DeeString_Newf("%r in %r",argv[1],argv[0]);
  } else {
   /* Generic binary operator. */
   return DeeString_Newf("%r %s %r",argv[0],info->oi_uname,argv[1]);
  }
 }
 if (argc == 3) {
  if (operator_name == OPERATOR_SETATTR) {
   if (DeeString_Check(argv[1]))
       return DeeString_Newf("%r.%k = %r",argv[0],argv[1],argv[2]);
  } else if (operator_name == OPERATOR_SETITEM) {
   return DeeString_Newf("%r[%r] = %r",argv[0],argv[1],argv[2]);
  } else if (operator_name == OPERATOR_GETRANGE) {
   return DeeString_Newf("%r[%r:%r]",argv[0],argv[1],argv[2]);
  } else if (operator_name == OPERATOR_DELRANGE) {
   return DeeString_Newf("del %r[%r:%r]",argv[0],argv[1],argv[2]);
  }
 }
fallback:
 /* Fallback: Print a generic operator representation. */
 unicode_printer_init(&printer);
 if (argc) {
  if (unicode_printer_printf(&printer,"%r.",argv[0]) < 0)
      goto err_printer;
 }
 if (UNICODE_PRINTER_PRINT(&printer,"operator ") < 0)
     goto err_printer;
 if (info) {
  if (unicode_printer_printf(&printer,"__%s__ (",info->oi_sname) < 0)
      goto err_printer;
 } else {
  if (unicode_printer_printf(&printer,"%I16u (",operator_name) < 0)
      goto err_printer;
 }
 for (i = 1; i < argc; ++i) {
  if (unicode_printer_printf(&printer,"%s%r",i > 1 ? ", " : "",argv[i]) < 0)
      goto err_printer;
 }
 if (UNICODE_PRINTER_PRINT(&printer,")") < 0)
     goto err_printer;
 return unicode_printer_pack(&printer);
err_printer:
 unicode_printer_fini(&printer);
 return NULL;
}

/* ASSERT(string message = "", int operator_id = -1, operator_args...) -> none;
 * NOTE: When `operator_id' is -1, ignore all remaining arguments. */
PRIVATE DREF DeeObject *DCALL
f_rt_assert(size_t argc, DeeObject **__restrict argv) {
 DeeObject *message = Dee_EmptyString;
 DeeObject *assertion_error;
 int operator_name = -1;
 if (argc) {
  message = *argv;
  if (DeeNone_Check(message))
      message = Dee_EmptyString;
  if (DeeObject_AssertTypeExact(message,&DeeString_Type))
      goto err;
  ++argv,--argc;
 }
 if (argc) {
  if (DeeObject_AsInt(*argv,&operator_name))
      goto err;
  ++argv,--argc;
 }
 if (operator_name >= 0) {
  DREF DeeObject *repr;
  repr = get_expression_repr((uint16_t)operator_name,argc,argv);
  if unlikely(!repr) goto err;
  if (DeeString_IsEmpty(message)) {
   message = DeeString_Newf("Assertion failed: %k",repr);
  } else {
   message = DeeString_Newf("Assertion failed: %k - %k",repr,message);
  }
  Dee_Decref(repr);
 } else {
  if (DeeString_IsEmpty(message)) {
   message = DeeString_New("Assertion failed");
  } else {
   message = DeeString_Newf("Assertion failed - %k",message);
  }
 }
 if unlikely(!message) goto err;
 /* Construct the assertion error object. */
 assertion_error = DeeObject_New(&DeeError_AssertionError,1,&message);
 Dee_Decref(message);
 if unlikely(!assertion_error) goto err;
 /* Throw the assertion error. */
 DeeError_Throw(assertion_error);
 Dee_Decref(assertion_error);
err:
 return NULL;
}
INTERN DEFINE_CMETHOD(rt_assert,&f_rt_assert);


/* The compiler will generate calls to these functions during explicit
 * invocation of operators when the argument count is not known at
 * compile-time:
 * >> args = pack(10,20);
 * >> print operator + (args...);
 */
PRIVATE DREF DeeObject *DCALL /* POsOrADd */
f_rt_pooad(size_t argc, DeeObject **__restrict argv) {
 switch (argc) {
 case 1: return DeeObject_Pos(argv[0]);
 case 2: return DeeObject_Add(argv[0],argv[1]);
 default: break;
 }
 err_invalid_argc("operator +",argc,1,2);
 return NULL;
}
PRIVATE DREF DeeObject *DCALL /* NEgOrSuB */
f_rt_neosb(size_t argc, DeeObject **__restrict argv) {
 switch (argc) {
 case 1: return DeeObject_Neg(argv[0]);
 case 2: return DeeObject_Sub(argv[0],argv[1]);
 default: break;
 }
 err_invalid_argc("operator -",argc,1,2);
 return NULL;
}
PRIVATE DREF DeeObject *DCALL /* GetItemOrSetItem */
f_rt_giosi(size_t argc, DeeObject **__restrict argv) {
 switch (argc) {
 case 2: return DeeObject_GetItem(argv[0],argv[1]);
 case 3:
  if (DeeObject_SetItem(argv[0],argv[1],argv[2]))
      goto err;
  return_none;
 default: break;
 }
 err_invalid_argc("operator []",argc,2,3);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL /* GetRangeOrSetRange */
f_rt_grosr(size_t argc, DeeObject **__restrict argv) {
 switch (argc) {
 case 3: return DeeObject_GetRange(argv[0],argv[1],argv[2]);
 case 4:
  if (DeeObject_SetRange(argv[0],argv[1],argv[2],argv[3]))
      goto err;
  return_none;
 default: break;
 }
 err_invalid_argc("operator [:]",argc,3,4);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL /* GetAttrOrSetAttr */
f_rt_gaosa(size_t argc, DeeObject **__restrict argv) {
 switch (argc) {
 case 2:
  if (DeeObject_AssertTypeExact(argv[1],&DeeString_Type))
      goto err;
  return DeeObject_GetAttr(argv[0],argv[1]);
 case 3:
  if (DeeObject_AssertTypeExact(argv[1],&DeeString_Type))
      goto err;
  if (DeeObject_SetAttr(argv[0],argv[1],argv[2]))
      goto err;
  return_none;
 default: break;
 }
 err_invalid_argc("operator .",argc,2,3);
err:
 return NULL;
}

/* These CMETHOD objects are exported from `deemon' with the `rt_' prefix replaced with `__'
 * HINT: These are exported using the `MODSYM_FHIDDEN' flag, so you won't see them in listings. */
INTERN DEFINE_CMETHOD(rt_pooad,&f_rt_pooad);
INTERN DEFINE_CMETHOD(rt_neosb,&f_rt_neosb);
INTERN DEFINE_CMETHOD(rt_giosi,&f_rt_giosi);
INTERN DEFINE_CMETHOD(rt_grosr,&f_rt_grosr);
INTERN DEFINE_CMETHOD(rt_gaosa,&f_rt_gaosa);



PRIVATE DREF DeeObject *DCALL
f_rt_badcall(size_t argc, DeeObject **__restrict argv) {
 DeeThreadObject *ts;
 size_t argc_cur,argc_min = 0,argc_max;
 char const *function_name = NULL;
 if (DeeArg_Unpack(argc,argv,"Iu:__badcall",&argc_max))
     goto done;
 ts = DeeThread_Self();
 argc_cur = argc_max;
 if likely(ts->t_execsz) {
  struct code_frame *frame = ts->t_exec;
  DeeCodeObject *code = frame->cf_func->fo_code;
  argc_cur = frame->cf_argc;
  argc_min = code->co_argc_min;
  function_name = DeeDDI_NAME(code->co_ddi);
  if (!*function_name) function_name = NULL;
 }
 /* Throw the invalid-argument-count error. */
 err_invalid_argc(function_name,
                  argc_cur,
                  argc_min,
                  argc_max);
done:
 return NULL;
}
INTERN DEFINE_CMETHOD(rt_badcall,&f_rt_badcall);



DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_FUNCTIONS_C */
