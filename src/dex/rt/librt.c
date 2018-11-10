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
#ifndef GUARD_DEX_FS_LIBRT_C
#define GUARD_DEX_FS_LIBRT_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/compiler.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/thread.h>

#include "librt.h"

DECL_BEGIN

/* !!! THIS MODULE IS NON-STANDARD AND MEANT TO EXPOSE IMPLEMENTATION-INTERNALS !!!
 * --------------------------------------------------------------------------------
 * Any compiler/interpreter that wishes to comply with the deemon standard is
 * not required to implement this module, or if it chooses to implement it, it
 * is not required to provide exports identical, or compatible with those seen
 * here.
 * --------------------------------------------------------------------------------
 * If an implementation should choose to provide this module, the purpose of it
 * is to expose implementation-specific internals, such as controls for max
 * stack recursion, objects and types used to implement the compiler, as well
 * as other internal types.
 * --------------------------------------------------------------------------------
 * Since the `rt' module as a whole is non-portable, exports symbols are not
 * required to include the `_np' suffix normally required for such symbols. */




PRIVATE DREF DeeObject *DCALL
librt_getstacklimit_f(size_t argc, DeeObject **__restrict argv) {
 uint16_t result;
 if (DeeArg_Unpack(argc,argv,":getstacklimit"))
     return NULL;
 result = ATOMIC_READ(DeeExec_StackLimit);
 return DeeInt_NewU16(result);
}
PRIVATE DREF DeeObject *DCALL
librt_setstacklimit_f(size_t argc, DeeObject **__restrict argv) {
 uint16_t result,newval = DEE_CONFIG_DEFAULT_STACK_LIMIT;
 if (DeeArg_Unpack(argc,argv,"|I16u:setstacklimit",&newval))
     return NULL;
 result = ATOMIC_XCH(DeeExec_StackLimit,newval);
 return DeeInt_NewU16(result);
}

PRIVATE DEFINE_CMETHOD(librt_getstacklimit,librt_getstacklimit_f);
PRIVATE DEFINE_CMETHOD(librt_setstacklimit,librt_setstacklimit_f);



PRIVATE struct dex_symbol symbols[] = {
    { "getstacklimit", (DeeObject *)&librt_getstacklimit, MODSYM_FNORMAL,
      DOC("->?Dint\n"
          "Returns the current stack limit, that is the max number of "
          "user-code functions that may be executed consecutively before "
          "a :StackOverflow error is thrown\n"
          "The default stack limit is $" PP_STR(DEE_CONFIG_DEFAULT_STACK_LIMIT)) },
    { "setstacklimit", (DeeObject *)&librt_setstacklimit, MODSYM_FNORMAL,
      DOC("(new_limit=!" PP_STR(DEE_CONFIG_DEFAULT_STACK_LIMIT) ")->?Dint\n"
          "@throw IntegerOverflow @new_limit is negative, or greater than $0xffff\n"
          "Set the new stack limit to @new_limit and return the old limit\n"
          "The stack limit is checked every time a user-code function is "
          "entered, at which point a :StackOverflow error is thrown if "
          "the currently set limit is exceeded") },
    { "stacklimitunlimited",
#ifdef CONFIG_HAVE_EXEC_ALTSTACK
      Dee_True
#else
      Dee_False
#endif
      ,
      MODSYM_FNORMAL,
      DOC("->?Dbool\n"
          "A boolean that is :true if the deemon interpreter supports "
          "an unlimited stack limit, meaning that #setstacklimit can "
          "be used to set a stack limit to up $0xffff ($65535), which "
          "is the hard limit\n"
          "When :false, setting the stack limit higher than the default "
          "may lead to hard crashes of the deemon interpreter, causing "
          "the user-script and hosting application to crash in an undefined "
          "manner.") },
    { "interactivemodule", (DeeObject *)&DeeInteractiveModule_Type }, /* The type used to implement an interactive module, as available by `deemon -i' */
#ifndef CONFIG_NO_DEX
    { "dexmodule", (DeeObject *)&DeeDex_Type },                   /* The type of a module that has been loaded from a machine-level shared library. */
#endif
    { "compiler", (DeeObject *)&DeeCompiler_Type },               /* A user-code interface for the compiler used by this implementation */
    { "classdescriptor", (DeeObject *)&DeeClassDescriptor_Type }, /* The descriptor type generated by the compiler as a prototype for how a class will be created at runtime. */
    { "instancemember", (DeeObject *)&DeeInstanceMember_Type },   /* An unbund class->instance member (e.g. `class MyClass { member foo; } type(MyClass.foo);') */
    { "cmethod", (DeeObject *)&DeeCMethod_Type },                 /* C-variant of `function' (e.g. `boundattr from deemon') */
    { "kwcmethod", (DeeObject *)&DeeKwCMethod_Type },             /* C-variant of `function' (with keyword support) */
    { "objmethod", (DeeObject *)&DeeObjMethod_Type },             /* C-variant of `instancemethod' (e.g. `"FOO".lower') */
    { "kwobjmethod", (DeeObject *)&DeeKwObjMethod_Type },         /* C-variant of `instancemethod' (with keyword support) */
    { "classmethod", (DeeObject *)&DeeClsMethod_Type },           /* C-variant of an unbound class->instance method (e.g. `string.lower') */
    { "kwclassmethod", (DeeObject *)&DeeKwClsMethod_Type },       /* C-variant of an unbound class->instance method (with keyword support) */
    { "classproperty", (DeeObject *)&DeeClsProperty_Type },       /* C-variant of an unbound class->instance getset (e.g. `sequence.length') */
    { "classmember", (DeeObject *)&DeeClsMember_Type },           /* C-variant of an unbound class->instance member (e.g. `type.__name__') */
    { "filetype", (DeeObject *)&DeeFileType_Type },               /* type(file) -- The typetype for file types. */
    { "nonetype", (DeeObject *)&DeeNone_Type },                   /* type(none) */
    { "memoryfile", (DeeObject *)&DeeMemoryFile_Type },           /* An internal file type for streaming from read-only raw memory. */
    { "rodict", (DeeObject *)&DeeRoDict_Type },                   /* A read-only variant of the builtin `dict' type (used by the compiler to construct constant, generic mapping expression) */
    { "roset", (DeeObject *)&DeeRoSet_Type },                     /* A read-only variant of the builtin `hashset' type (used by the compiler to construct constant, generic set expression) */
    { "kwds", (DeeObject *)&DeeKwds_Type },                       /* The type used to represent keyword arguments being mapped onto positional arguments. */
    { "kwdsmapping", (DeeObject *)&DeeKwdsMapping_Type },         /* A wrapper around `kwds' and the associated argc/argv to create a proper mapping object */
    { "ddi", (DeeObject *)&DeeDDI_Type },                         /* The type used to hold debug information for user-defined code objects. */
    { "NoMemory_instance", (DeeObject *)&DeeError_NoMemory_instance },
    { "StopIteration_instance", (DeeObject *)&DeeError_StopIteration_instance },
    { "Interrupt_instance", (DeeObject *)&DeeError_Interrupt_instance },
    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols
};

DECL_END

#endif /* !GUARD_DEX_FS_LIBRT_C */
