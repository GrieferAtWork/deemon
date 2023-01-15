/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_STRINGS_H
#define GUARD_DEEMON_RUNTIME_STRINGS_H 1

#include <deemon/api.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/string.h>

DECL_BEGIN

#ifndef STRING2
#define STRING2(name, str) INTDEF DeeStringObject name;
#endif /* !STRING2 */
#ifndef STRING
#define STRING(str) STRING2(str_##str, #str)
#endif /* !STRING */

/*[[[begin]]]*/
STRING(Signal)
STRING(Error)

STRING(Attribute)
STRING(Bytes)
STRING(Callable)
STRING(Cell)
STRING(Dict)
STRING(File)
STRING(Frame)
STRING(Function)
STRING(HashSet)
STRING(InstanceMethod)
STRING(Iterator)
STRING(Joined)
STRING(List)
STRING(Mapping)
STRING(Module)
STRING(Numeric)
STRING(Object)
STRING(Property)
STRING(Sequence)
STRING(Set)
STRING(Super)
STRING(Thread)
STRING(Traceback)
STRING(Tuple)
STRING(Type)
STRING(WeakRef)
STRING(WeakRefAble)

STRING(bool)
STRING(string)
STRING(int)
STRING(float)

STRING(none)
STRING(true)
STRING(false)
STRING(deemon)
STRING(seq)
STRING(operators)
STRING(files)
STRING(_jit)
STRING(codecs)
STRING(__encode)
STRING(__decode)
STRING(strict)
STRING(replace)
STRING(ignore)
STRING(fs)
STRING2(str_environ, "environ")
STRING(get)
STRING(set)
STRING(enumattr)
STRING(gc)
STRING(import)
STRING(exec)
STRING(isatty)
STRING(pop)
STRING(remove)
STRING(rremove)
STRING(removeall)
STRING(removeif)
STRING(erase)
STRING(insert)
STRING(clear)
STRING(append)
STRING(extend)
STRING(insertall)
STRING(xch)
STRING(resize)
STRING(pushfront)
STRING(pushback)
STRING(popfront)
STRING(popback)

STRING(revert)
STRING(advance)
STRING(index)
STRING(prev)
STRING(hasprev)
STRING(hasnext)
STRING(rewind)
STRING(peek)


STRING(format)
STRING(__format__)
STRING(__name__)
STRING(__doc__)
STRING(__type__)
STRING(__kwds__)
STRING(__module__)
STRING(first)
STRING(last)

#ifdef DeeSysFD_GETSET
STRING2(str_getsysfd, DeeSysFD_GETSET)
#endif /* DeeSysFD_GETSET */

STRING(size)
STRING(filename)
#ifndef CONFIG_NO_THREADS
STRING(run)
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_LANGUAGE_NO_ASM
STRING(except)
STRING(this)
STRING(this_module)
STRING(this_function)
STRING2(str_cell_empty, "cell empty")
#endif /* !CONFIG_LANGUAGE_NO_ASM */
STRING2(str_tab, "\t")

STRING(__pooad)
STRING(__neosb)
STRING(__giosi)
STRING(__grosr)
STRING(__gaosa)

STRING(d200)

STRING2(str_nomemory, "allocation failed")
STRING2(str_dots, "...")


/* Strings used for internal AST branches */
STRING(constexpr)
STRING(sym)
STRING(unbind)
STRING(bound)
STRING(multiple)
STRING(return)
STRING(yield)
STRING(throw)
STRING(try)
STRING(loop)
STRING(loopctl)
STRING(conditional)
/*STRING(bool)*/
STRING(expand)
STRING(function)
STRING(operatorfunc)
STRING(operator)
STRING(action)
STRING(class)
STRING(label)
STRING(goto)
STRING(switch)
STRING(assembly)

/* Strings used for internal symbol classes */
/*STRING(none)*/
STRING(global)
STRING(extern)
STRING(module)
STRING(mymod)
STRING(getset)
STRING(ifield)
STRING(cfield)
STRING(alias)
STRING(arg)
STRING(local)
STRING(stack)
STRING(static)
/*STRING(except)*/
STRING(myfunc)
/*STRING(this)*/
STRING(ambig)
STRING(fwd)
STRING(const)
/*[[[end]]]*/

#undef STRING2
#undef STRING

/*[[[deemon
import * from deemon;
local lines = File.open(__FILE__).read()
	.partition("/" "*[[[begin]]]*" "/").last
	.partition("/" "*[[[end]]]*" "/").first
	.strip()
	.splitlines();
for (local line: lines) {
	line = line.strip();
	if (line.startswith("#")) {
		print(line);
	} else {
		local name;
		try {
			name = line.rescanf(r"STRING\((\w+)\)")...;
		} catch (...) {
			continue;
		}
		print("#ifndef STR_", name);
		print("#define STR_", name, " DeeString_STR(&str_", name, ")");
		print("#endif /" "* !STR_", name, " *" "/");
	}
}
]]]*/
#ifndef STR_Signal
#define STR_Signal DeeString_STR(&str_Signal)
#endif /* !STR_Signal */
#ifndef STR_Error
#define STR_Error DeeString_STR(&str_Error)
#endif /* !STR_Error */
#ifndef STR_Attribute
#define STR_Attribute DeeString_STR(&str_Attribute)
#endif /* !STR_Attribute */
#ifndef STR_Bytes
#define STR_Bytes DeeString_STR(&str_Bytes)
#endif /* !STR_Bytes */
#ifndef STR_Callable
#define STR_Callable DeeString_STR(&str_Callable)
#endif /* !STR_Callable */
#ifndef STR_Cell
#define STR_Cell DeeString_STR(&str_Cell)
#endif /* !STR_Cell */
#ifndef STR_Dict
#define STR_Dict DeeString_STR(&str_Dict)
#endif /* !STR_Dict */
#ifndef STR_File
#define STR_File DeeString_STR(&str_File)
#endif /* !STR_File */
#ifndef STR_Frame
#define STR_Frame DeeString_STR(&str_Frame)
#endif /* !STR_Frame */
#ifndef STR_Function
#define STR_Function DeeString_STR(&str_Function)
#endif /* !STR_Function */
#ifndef STR_HashSet
#define STR_HashSet DeeString_STR(&str_HashSet)
#endif /* !STR_HashSet */
#ifndef STR_InstanceMethod
#define STR_InstanceMethod DeeString_STR(&str_InstanceMethod)
#endif /* !STR_InstanceMethod */
#ifndef STR_Iterator
#define STR_Iterator DeeString_STR(&str_Iterator)
#endif /* !STR_Iterator */
#ifndef STR_Joined
#define STR_Joined DeeString_STR(&str_Joined)
#endif /* !STR_Joined */
#ifndef STR_List
#define STR_List DeeString_STR(&str_List)
#endif /* !STR_List */
#ifndef STR_Mapping
#define STR_Mapping DeeString_STR(&str_Mapping)
#endif /* !STR_Mapping */
#ifndef STR_Module
#define STR_Module DeeString_STR(&str_Module)
#endif /* !STR_Module */
#ifndef STR_Numeric
#define STR_Numeric DeeString_STR(&str_Numeric)
#endif /* !STR_Numeric */
#ifndef STR_Object
#define STR_Object DeeString_STR(&str_Object)
#endif /* !STR_Object */
#ifndef STR_Property
#define STR_Property DeeString_STR(&str_Property)
#endif /* !STR_Property */
#ifndef STR_Sequence
#define STR_Sequence DeeString_STR(&str_Sequence)
#endif /* !STR_Sequence */
#ifndef STR_Set
#define STR_Set DeeString_STR(&str_Set)
#endif /* !STR_Set */
#ifndef STR_Super
#define STR_Super DeeString_STR(&str_Super)
#endif /* !STR_Super */
#ifndef STR_Thread
#define STR_Thread DeeString_STR(&str_Thread)
#endif /* !STR_Thread */
#ifndef STR_Traceback
#define STR_Traceback DeeString_STR(&str_Traceback)
#endif /* !STR_Traceback */
#ifndef STR_Tuple
#define STR_Tuple DeeString_STR(&str_Tuple)
#endif /* !STR_Tuple */
#ifndef STR_Type
#define STR_Type DeeString_STR(&str_Type)
#endif /* !STR_Type */
#ifndef STR_WeakRef
#define STR_WeakRef DeeString_STR(&str_WeakRef)
#endif /* !STR_WeakRef */
#ifndef STR_WeakRefAble
#define STR_WeakRefAble DeeString_STR(&str_WeakRefAble)
#endif /* !STR_WeakRefAble */
#ifndef STR_bool
#define STR_bool DeeString_STR(&str_bool)
#endif /* !STR_bool */
#ifndef STR_string
#define STR_string DeeString_STR(&str_string)
#endif /* !STR_string */
#ifndef STR_int
#define STR_int DeeString_STR(&str_int)
#endif /* !STR_int */
#ifndef STR_float
#define STR_float DeeString_STR(&str_float)
#endif /* !STR_float */
#ifndef STR_none
#define STR_none DeeString_STR(&str_none)
#endif /* !STR_none */
#ifndef STR_true
#define STR_true DeeString_STR(&str_true)
#endif /* !STR_true */
#ifndef STR_false
#define STR_false DeeString_STR(&str_false)
#endif /* !STR_false */
#ifndef STR_deemon
#define STR_deemon DeeString_STR(&str_deemon)
#endif /* !STR_deemon */
#ifndef STR_seq
#define STR_seq DeeString_STR(&str_seq)
#endif /* !STR_seq */
#ifndef STR_operators
#define STR_operators DeeString_STR(&str_operators)
#endif /* !STR_operators */
#ifndef STR_files
#define STR_files DeeString_STR(&str_files)
#endif /* !STR_files */
#ifndef STR__jit
#define STR__jit DeeString_STR(&str__jit)
#endif /* !STR__jit */
#ifndef STR_codecs
#define STR_codecs DeeString_STR(&str_codecs)
#endif /* !STR_codecs */
#ifndef STR___encode
#define STR___encode DeeString_STR(&str___encode)
#endif /* !STR___encode */
#ifndef STR___decode
#define STR___decode DeeString_STR(&str___decode)
#endif /* !STR___decode */
#ifndef STR_strict
#define STR_strict DeeString_STR(&str_strict)
#endif /* !STR_strict */
#ifndef STR_replace
#define STR_replace DeeString_STR(&str_replace)
#endif /* !STR_replace */
#ifndef STR_ignore
#define STR_ignore DeeString_STR(&str_ignore)
#endif /* !STR_ignore */
#ifndef STR_fs
#define STR_fs DeeString_STR(&str_fs)
#endif /* !STR_fs */
#ifndef STR_get
#define STR_get DeeString_STR(&str_get)
#endif /* !STR_get */
#ifndef STR_set
#define STR_set DeeString_STR(&str_set)
#endif /* !STR_set */
#ifndef STR_enumattr
#define STR_enumattr DeeString_STR(&str_enumattr)
#endif /* !STR_enumattr */
#ifndef STR_gc
#define STR_gc DeeString_STR(&str_gc)
#endif /* !STR_gc */
#ifndef STR_import
#define STR_import DeeString_STR(&str_import)
#endif /* !STR_import */
#ifndef STR_exec
#define STR_exec DeeString_STR(&str_exec)
#endif /* !STR_exec */
#ifndef STR_isatty
#define STR_isatty DeeString_STR(&str_isatty)
#endif /* !STR_isatty */
#ifndef STR_pop
#define STR_pop DeeString_STR(&str_pop)
#endif /* !STR_pop */
#ifndef STR_remove
#define STR_remove DeeString_STR(&str_remove)
#endif /* !STR_remove */
#ifndef STR_rremove
#define STR_rremove DeeString_STR(&str_rremove)
#endif /* !STR_rremove */
#ifndef STR_removeall
#define STR_removeall DeeString_STR(&str_removeall)
#endif /* !STR_removeall */
#ifndef STR_removeif
#define STR_removeif DeeString_STR(&str_removeif)
#endif /* !STR_removeif */
#ifndef STR_erase
#define STR_erase DeeString_STR(&str_erase)
#endif /* !STR_erase */
#ifndef STR_insert
#define STR_insert DeeString_STR(&str_insert)
#endif /* !STR_insert */
#ifndef STR_clear
#define STR_clear DeeString_STR(&str_clear)
#endif /* !STR_clear */
#ifndef STR_append
#define STR_append DeeString_STR(&str_append)
#endif /* !STR_append */
#ifndef STR_extend
#define STR_extend DeeString_STR(&str_extend)
#endif /* !STR_extend */
#ifndef STR_insertall
#define STR_insertall DeeString_STR(&str_insertall)
#endif /* !STR_insertall */
#ifndef STR_xch
#define STR_xch DeeString_STR(&str_xch)
#endif /* !STR_xch */
#ifndef STR_resize
#define STR_resize DeeString_STR(&str_resize)
#endif /* !STR_resize */
#ifndef STR_pushfront
#define STR_pushfront DeeString_STR(&str_pushfront)
#endif /* !STR_pushfront */
#ifndef STR_pushback
#define STR_pushback DeeString_STR(&str_pushback)
#endif /* !STR_pushback */
#ifndef STR_popfront
#define STR_popfront DeeString_STR(&str_popfront)
#endif /* !STR_popfront */
#ifndef STR_popback
#define STR_popback DeeString_STR(&str_popback)
#endif /* !STR_popback */
#ifndef STR_revert
#define STR_revert DeeString_STR(&str_revert)
#endif /* !STR_revert */
#ifndef STR_advance
#define STR_advance DeeString_STR(&str_advance)
#endif /* !STR_advance */
#ifndef STR_index
#define STR_index DeeString_STR(&str_index)
#endif /* !STR_index */
#ifndef STR_prev
#define STR_prev DeeString_STR(&str_prev)
#endif /* !STR_prev */
#ifndef STR_hasprev
#define STR_hasprev DeeString_STR(&str_hasprev)
#endif /* !STR_hasprev */
#ifndef STR_hasnext
#define STR_hasnext DeeString_STR(&str_hasnext)
#endif /* !STR_hasnext */
#ifndef STR_rewind
#define STR_rewind DeeString_STR(&str_rewind)
#endif /* !STR_rewind */
#ifndef STR_peek
#define STR_peek DeeString_STR(&str_peek)
#endif /* !STR_peek */
#ifndef STR_format
#define STR_format DeeString_STR(&str_format)
#endif /* !STR_format */
#ifndef STR___format__
#define STR___format__ DeeString_STR(&str___format__)
#endif /* !STR___format__ */
#ifndef STR___name__
#define STR___name__ DeeString_STR(&str___name__)
#endif /* !STR___name__ */
#ifndef STR___doc__
#define STR___doc__ DeeString_STR(&str___doc__)
#endif /* !STR___doc__ */
#ifndef STR___type__
#define STR___type__ DeeString_STR(&str___type__)
#endif /* !STR___type__ */
#ifndef STR___kwds__
#define STR___kwds__ DeeString_STR(&str___kwds__)
#endif /* !STR___kwds__ */
#ifndef STR___module__
#define STR___module__ DeeString_STR(&str___module__)
#endif /* !STR___module__ */
#ifndef STR_first
#define STR_first DeeString_STR(&str_first)
#endif /* !STR_first */
#ifndef STR_last
#define STR_last DeeString_STR(&str_last)
#endif /* !STR_last */
#ifdef DeeSysFD_GETSET
#endif /* DeeSysFD_GETSET */
#ifndef STR_size
#define STR_size DeeString_STR(&str_size)
#endif /* !STR_size */
#ifndef STR_filename
#define STR_filename DeeString_STR(&str_filename)
#endif /* !STR_filename */
#ifndef CONFIG_NO_THREADS
#ifndef STR_run
#define STR_run DeeString_STR(&str_run)
#endif /* !STR_run */
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_LANGUAGE_NO_ASM
#ifndef STR_except
#define STR_except DeeString_STR(&str_except)
#endif /* !STR_except */
#ifndef STR_this
#define STR_this DeeString_STR(&str_this)
#endif /* !STR_this */
#ifndef STR_this_module
#define STR_this_module DeeString_STR(&str_this_module)
#endif /* !STR_this_module */
#ifndef STR_this_function
#define STR_this_function DeeString_STR(&str_this_function)
#endif /* !STR_this_function */
#endif /* !CONFIG_LANGUAGE_NO_ASM */
#ifndef STR___pooad
#define STR___pooad DeeString_STR(&str___pooad)
#endif /* !STR___pooad */
#ifndef STR___neosb
#define STR___neosb DeeString_STR(&str___neosb)
#endif /* !STR___neosb */
#ifndef STR___giosi
#define STR___giosi DeeString_STR(&str___giosi)
#endif /* !STR___giosi */
#ifndef STR___grosr
#define STR___grosr DeeString_STR(&str___grosr)
#endif /* !STR___grosr */
#ifndef STR___gaosa
#define STR___gaosa DeeString_STR(&str___gaosa)
#endif /* !STR___gaosa */
#ifndef STR_d200
#define STR_d200 DeeString_STR(&str_d200)
#endif /* !STR_d200 */
#ifndef STR_constexpr
#define STR_constexpr DeeString_STR(&str_constexpr)
#endif /* !STR_constexpr */
#ifndef STR_sym
#define STR_sym DeeString_STR(&str_sym)
#endif /* !STR_sym */
#ifndef STR_unbind
#define STR_unbind DeeString_STR(&str_unbind)
#endif /* !STR_unbind */
#ifndef STR_bound
#define STR_bound DeeString_STR(&str_bound)
#endif /* !STR_bound */
#ifndef STR_multiple
#define STR_multiple DeeString_STR(&str_multiple)
#endif /* !STR_multiple */
#ifndef STR_return
#define STR_return DeeString_STR(&str_return)
#endif /* !STR_return */
#ifndef STR_yield
#define STR_yield DeeString_STR(&str_yield)
#endif /* !STR_yield */
#ifndef STR_throw
#define STR_throw DeeString_STR(&str_throw)
#endif /* !STR_throw */
#ifndef STR_try
#define STR_try DeeString_STR(&str_try)
#endif /* !STR_try */
#ifndef STR_loop
#define STR_loop DeeString_STR(&str_loop)
#endif /* !STR_loop */
#ifndef STR_loopctl
#define STR_loopctl DeeString_STR(&str_loopctl)
#endif /* !STR_loopctl */
#ifndef STR_conditional
#define STR_conditional DeeString_STR(&str_conditional)
#endif /* !STR_conditional */
#ifndef STR_expand
#define STR_expand DeeString_STR(&str_expand)
#endif /* !STR_expand */
#ifndef STR_function
#define STR_function DeeString_STR(&str_function)
#endif /* !STR_function */
#ifndef STR_operatorfunc
#define STR_operatorfunc DeeString_STR(&str_operatorfunc)
#endif /* !STR_operatorfunc */
#ifndef STR_operator
#define STR_operator DeeString_STR(&str_operator)
#endif /* !STR_operator */
#ifndef STR_action
#define STR_action DeeString_STR(&str_action)
#endif /* !STR_action */
#ifndef STR_class
#define STR_class DeeString_STR(&str_class)
#endif /* !STR_class */
#ifndef STR_label
#define STR_label DeeString_STR(&str_label)
#endif /* !STR_label */
#ifndef STR_goto
#define STR_goto DeeString_STR(&str_goto)
#endif /* !STR_goto */
#ifndef STR_switch
#define STR_switch DeeString_STR(&str_switch)
#endif /* !STR_switch */
#ifndef STR_assembly
#define STR_assembly DeeString_STR(&str_assembly)
#endif /* !STR_assembly */
#ifndef STR_global
#define STR_global DeeString_STR(&str_global)
#endif /* !STR_global */
#ifndef STR_extern
#define STR_extern DeeString_STR(&str_extern)
#endif /* !STR_extern */
#ifndef STR_module
#define STR_module DeeString_STR(&str_module)
#endif /* !STR_module */
#ifndef STR_mymod
#define STR_mymod DeeString_STR(&str_mymod)
#endif /* !STR_mymod */
#ifndef STR_getset
#define STR_getset DeeString_STR(&str_getset)
#endif /* !STR_getset */
#ifndef STR_ifield
#define STR_ifield DeeString_STR(&str_ifield)
#endif /* !STR_ifield */
#ifndef STR_cfield
#define STR_cfield DeeString_STR(&str_cfield)
#endif /* !STR_cfield */
#ifndef STR_alias
#define STR_alias DeeString_STR(&str_alias)
#endif /* !STR_alias */
#ifndef STR_arg
#define STR_arg DeeString_STR(&str_arg)
#endif /* !STR_arg */
#ifndef STR_local
#define STR_local DeeString_STR(&str_local)
#endif /* !STR_local */
#ifndef STR_stack
#define STR_stack DeeString_STR(&str_stack)
#endif /* !STR_stack */
#ifndef STR_static
#define STR_static DeeString_STR(&str_static)
#endif /* !STR_static */
#ifndef STR_myfunc
#define STR_myfunc DeeString_STR(&str_myfunc)
#endif /* !STR_myfunc */
#ifndef STR_ambig
#define STR_ambig DeeString_STR(&str_ambig)
#endif /* !STR_ambig */
#ifndef STR_fwd
#define STR_fwd DeeString_STR(&str_fwd)
#endif /* !STR_fwd */
#ifndef STR_const
#define STR_const DeeString_STR(&str_const)
#endif /* !STR_const */
/*[[[end]]]*/
#ifdef DeeSysFD_GETSET
#ifndef STR_getsysfd
#define STR_getsysfd DeeString_STR(&str_getsysfd)
#endif /* !STR_getsysfd */
#endif /* DeeSysFD_GETSET */


#ifndef STR_except
#define STR_except "except"
#endif /* !STR_except */
#ifndef STR_this
#define STR_this "this"
#endif /* !STR_this */
#ifndef STR_this_module
#define STR_this_module "this_module"
#endif /* !STR_this_module */
#ifndef STR_this_function
#define STR_this_function "this_function"
#endif /* !STR_this_function */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_STRINGS_H */
