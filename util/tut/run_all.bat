@echo off
rem Copyright (c) 2018 Griefer@Work
rem
rem This software is provided 'as-is', without any express or implied
rem warranty. In no event will the authors be held liable for any damages
rem arising from the use of this software.
rem
rem Permission is granted to anyone to use this software for any purpose,
rem including commercial applications, and to alter it and redistribute it
rem freely, subject to the following restrictions:
rem
rem 1. The origin of this software must not be misrepresented; you must not
rem    claim that you wrote the original software. If you use this software
rem    in a product, an acknowledgement in the product documentation would be
rem    appreciated but is not required.
rem 2. Altered source versions must be plainly marked as such, and must not be
rem    misrepresented as being the original software.
rem 3. This notice may not be removed or altered from any source distribution.

rem Executes all tutorial files as a light-weight
rem way of making sure that deemon doesn't crash.

rem Sorry, but no sh script for this one...

call deemon destructor_hax.dee
call deemon expand_expressions.dee
rem call deemon extern_function.dee
rem call deemon fancy_loading.dee
call deemon list_env.dee
call deemon marshal_magic.dee
call deemon marshal_magic.dee
call del marshal_magic_compiled.dc
call deemon modules.dee
call deemon pipe_thread.dee
call deemon scopes.dee
call deemon simple_echo.dee < simple_echo.dee
call deemon simple_io.dee
call del simple_io_out.txt
call deemon switch_statement.dee
call deemon thread_punishment.dee
call deemon throw_in_finally.dee
call deemon walk_path.dee .
call deemon yield_and_exceptions.dee
call deemon yield_awesomeness.dee




