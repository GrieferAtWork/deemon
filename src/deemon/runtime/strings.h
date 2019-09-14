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
#ifndef GUARD_DEEMON_RUNTIME_STRINGS_H
#define GUARD_DEEMON_RUNTIME_STRINGS_H 1

#ifndef ONLY_LISTING
#include <deemon/api.h>
#include <deemon/file.h>
#include <deemon/object.h>
DECL_BEGIN
#endif /* !ONLY_LISTING */

#ifndef STRING2
#define STRING2(name, str) INTDEF DeeObject name;
#endif
#ifndef STRING
#define STRING(str) STRING2(str_##str, #str)
#endif

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
STRING(nonempty)
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


STRING(__format__)
STRING(__name__)
STRING(__doc__)
STRING(__type__)
STRING(__kwds__)
STRING(__module__)
STRING(first)
STRING(last)
#ifndef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
STRING(fileno)
#define STR_FILENO DeeString_STR(&str_fileno)
#else
#define STR_FILENO "fileno"
#endif

STRING(size)
STRING(filename)
#ifndef CONFIG_NO_THREADS
STRING(run)
#endif
#ifndef CONFIG_LANGUAGE_NO_ASM
STRING(except)
STRING(this)
STRING(this_module)
STRING(this_function)
STRING2(str_cell_empty, "cell empty")
#endif
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

#undef STRING2
#undef STRING

#ifndef ONLY_LISTING
DECL_END
#endif /* !ONLY_LISTING */

#endif /* !GUARD_DEEMON_RUNTIME_STRINGS_H */
