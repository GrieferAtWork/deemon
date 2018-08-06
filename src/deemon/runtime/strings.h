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
#ifndef GUARD_DEEMON_RUNTIME_STRINGS_H
#define GUARD_DEEMON_RUNTIME_STRINGS_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/file.h>

#ifndef STRING2
#define STRING2(name,str) INTDEF DeeObject name;
#endif
#ifndef STRING
#define STRING(str) STRING2(str_##str,#str)
#endif

DECL_BEGIN

STRING(Signal)
STRING(Error)
STRING(traceback)
STRING(frame)
STRING(object)
STRING(type)
STRING(none)
STRING(true)
STRING(false)
STRING(nonempty)
STRING(deemon)
STRING(seq)
STRING(function)
STRING(property)
STRING(operators)
STRING(instancemethod)
STRING(code)
STRING(module)
STRING(thread)
STRING(super)
STRING(file)
STRING(files)
STRING(codecs)
STRING(encode)
STRING(decode)
STRING(strict)
STRING(replace)
STRING(ignore)
STRING(joined)
STRING(sequence)
STRING(mapping)
STRING(get)
STRING(set)
STRING(iterator)
STRING(numeric)
STRING(callable)
STRING(attribute)
STRING(enumattr)
STRING(gc)
STRING(import)
STRING(weakref)
STRING(weakrefable)
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
STRING(__format__)
STRING(first)
STRING(last)
#ifndef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
STRING(fileno)
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
STRING2(str_cell_empty,"cell empty")
#endif
STRING2(str_tab,"\t")

STRING(__pooad)
STRING(__neosb)
STRING(__giosi)
STRING(__grosr)
STRING(__gaosa)

STRING(bool)
STRING(string)
STRING(bytes)
STRING(tuple)
STRING(list)
STRING(dict)
STRING(hashset)
STRING(int)
STRING(float)
STRING(cell)

STRING(d200)

STRING2(str_nomemory,"allocation failed")
STRING2(str_dots,"...")

DECL_END

#undef STRING2
#undef STRING

#endif /* !GUARD_DEEMON_RUNTIME_STRINGS_H */
