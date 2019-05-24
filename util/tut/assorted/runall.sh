# Copyright (c) 2019 Griefer@Work
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgement in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

# Executes all tutorial files as a light-weight
# way of making sure that deemon doesn't crash.

deemon classes.dee || exit $?
deemon conversion_operator.dee || exit $?
deemon deemon_format.dee || exit $?
deemon destructor_hax.dee || exit $?
deemon enum_attributes.dee || exit $?
deemon expand_expressions.dee || exit $?
#call deemon fancy_loading.dee || exit $?
deemon generators.dee || exit $?
deemon linked_list.dee || exit $?
deemon list_env.dee || exit $?
deemon more_class_fun.dee || exit $?
deemon nametuple.dee || exit $?
deemon pipe_thread.dee || exit $?
deemon python_style_main.dee || exit $?
deemon simple_echo.dee < simple_echo.dee || exit $?
deemon simple_io.dee || exit $?
unlink simple_io_out.txt
deemon simple_scanf.dee || exit $?
deemon super_classes.dee || exit $?
deemon thread_punishment.dee || exit $?
deemon walk_path.dee . || exit $?
deemon yield_awesomeness.dee || exit $?
deemon yield_iter_copy.dee || exit $?




