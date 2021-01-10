/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_MODULE_H
#define GUARD_DEEMON_CXX_MODULE_H 1

#include "api.h"
/**/

#include "../module.h"
#include "../system-features.h" /* strlen() */
#include "object.h"

DEE_CXX_BEGIN

class module: public Object {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeModule_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeModule_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeModule_CheckExact(ob);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(module, Object)
	module(obj_string name)
	    : Object(inherit(DeeModule_New(name))) { }
	module(char const *__restrict name)
	    : Object(inherit(DeeModule_NewString(name, strlen(name)))) { }
	module(char const *__restrict name, size_t namelen)
	    : Object(inherit(DeeModule_NewString(name, namelen))) { }
	struct Dee_module_symbol *symbol(char const *__restrict attr_name) const DEE_CXX_NOTHROW {
		return DeeModule_GetSymbolString((DeeModuleObject *)this->ptr(), attr_name, Dee_HashStr(attr_name));
	}
	struct Dee_module_symbol *symbol(char const *__restrict attr_name, Dee_hash_t hash) const DEE_CXX_NOTHROW {
		return DeeModule_GetSymbolString((DeeModuleObject *)this->ptr(), attr_name, hash);
	}
	struct Dee_module_symbol *symbol(uint16_t gid) const DEE_CXX_NOTHROW {
		return DeeModule_GetSymbolID((DeeModuleObject *)this->ptr(), gid);
	}
	DREF DeeObject *symbol_getref(struct Dee_module_symbol *__restrict sym) const DEE_CXX_NOTHROW {
		return DeeModule_GetAttrSymbol((DeeModuleObject *)this->ptr(), sym);
	}
	Object symbol_get(struct Dee_module_symbol *__restrict sym) const {
		return inherit(symbol_getref(sym));
	}
	int symbol_bound(struct Dee_module_symbol *__restrict sym) const {
		return throw_if_negative(DeeModule_BoundAttrSymbol((DeeModuleObject *)this->ptr(), sym)) != 0;
	}
	void symbol_del(struct Dee_module_symbol *__restrict sym) const {
		throw_if_nonzero(DeeModule_DelAttrSymbol((DeeModuleObject *)this->ptr(), sym));
	}
	void symbol_set(struct Dee_module_symbol *__restrict sym, DeeObject *__restrict value) const {
		throw_if_nonzero(DeeModule_SetAttrSymbol((DeeModuleObject *)this->ptr(), sym, value));
	}
	char const *globalname(uint16_t gid) const DEE_CXX_NOTHROW {
		return DeeModule_GlobalName(*this, gid);
	}
	bool runinit() const DEE_CXX_NOTHROW {
		return throw_if_negative(DeeModule_RunInit(*this)) == 0;
	}
	void initimports() const DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeModule_InitImports(*this));
	}
	Object root(bool set_initialized = true) const DEE_CXX_NOTHROW {
		return inherit(DeeModule_GetRoot(*this, set_initialized));
	}
	void *nativesymbol(char const *__restrict name) const DEE_CXX_NOTHROW {
		return DeeModule_GetNativeSymbol(*this, name);
	}

public:
	static module deemon() DEE_CXX_NOTHROW {
		return nonnull((DeeObject *)DeeModule_GetDeemon());
	}
	static module opensourcefile(obj_string source_pathname, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFile(source_pathname, NULL, options, true));
	}
	static module opensourcefile(obj_string source_pathname, /*opt*/ obj_string module_global_name, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFile(source_pathname, module_global_name, options, true));
	}
	static module opensourcefile(char const *__restrict source_pathname, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFileString(source_pathname, strlen(source_pathname), NULL, 0, options, true));
	}
	static module opensourcefile(char const *__restrict source_pathname, char const *module_global_name, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFileString(source_pathname, strlen(source_pathname), module_global_name, module_global_name ? strlen(module_global_name) : 0, options, true));
	}
	static module opensourcefile(char const *__restrict source_pathname, char const *module_global_name, size_t module_global_namesize, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFileString(source_pathname, strlen(source_pathname), module_global_name, module_global_namesize, options, true));
	}
	static module opensourcefile(char const *__restrict source_pathname, size_t source_pathsize, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFileString(source_pathname, source_pathsize, NULL, 0, options, true));
	}
	static module opensourcefile(char const *__restrict source_pathname, size_t source_pathsize, char const *module_global_name, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFileString(source_pathname, source_pathsize, module_global_name, module_global_name ? strlen(module_global_name) : 0, options, true));
	}
	static module opensourcefile(char const *__restrict source_pathname, size_t source_pathsize, char const *module_global_name, size_t module_global_namesize, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceFileString(source_pathname, source_pathsize, module_global_name, module_global_namesize, options, true));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, data_size, 0, 0, options, NULL, NULL));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, data_size, 0, 0, options, source_pathname, NULL));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname, /*opt*/ obj_string module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, data_size, 0, 0, options, source_pathname, module_name));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, 0, 0, options, source_pathname, source_pathsize, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, 0, 0, options, source_pathname, source_pathsize, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, 0, 0, options, source_pathname, source_pathsize, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line = 0, int start_col = 0, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, data_size, start_line, start_col, options, NULL, NULL));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, data_size, start_line, start_col, options, source_pathname, NULL));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname, /*opt*/ obj_string module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, data_size, start_line, start_col, options, source_pathname, module_name));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, start_line, start_col, options, source_pathname, source_pathsize, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, start_line, start_col, options, source_pathname, source_pathsize, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, size_t data_size, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, data_size, start_line, start_col, options, source_pathname, source_pathsize, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, strlen(data), 0, 0, options, NULL, NULL));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, strlen(data), 0, 0, options, source_pathname, NULL));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname, /*opt*/ obj_string module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, strlen(data), 0, 0, options, source_pathname, module_name));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), 0, 0, options, source_pathname, source_pathsize, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), 0, 0, options, source_pathname, source_pathsize, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), 0, 0, options, source_pathname, source_pathsize, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, int start_line = 0, int start_col = 0, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, strlen(data), start_line, start_col, options, NULL, NULL));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, strlen(data), start_line, start_col, options, source_pathname, NULL));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname, /*opt*/ obj_string module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemory(data, strlen(data), start_line, start_col, options, source_pathname, module_name));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_namesize));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), start_line, start_col, options, source_pathname, source_pathsize, NULL, 0));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), start_line, start_col, options, source_pathname, source_pathsize, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcememory(char const *__restrict data, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceMemoryString(data, strlen(data), start_line, start_col, options, source_pathname, source_pathsize, module_name, module_namesize));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStream(source_stream, 0, 0, options, NULL, NULL));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStream(source_stream, 0, 0, options, source_pathname, NULL));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname, /*opt*/ obj_string module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStream(source_stream, 0, 0, options, source_pathname, module_name));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, NULL, 0));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, 0, 0, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_namesize));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, 0, 0, options, source_pathname, source_pathsize, NULL, 0));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, 0, 0, options, source_pathname, source_pathsize, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcestream(obj_file source_stream, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, 0, 0, options, source_pathname, source_pathsize, module_name, module_namesize));
	}
	static module opensourcestream(obj_file source_stream, int start_line = 0, int start_col = 0, struct Dee_compiler_options *options = NULL) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStream(source_stream, start_line, start_col, options, NULL, NULL));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStream(source_stream, start_line, start_col, options, source_pathname, NULL));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*opt*/ obj_string source_pathname, /*opt*/ obj_string module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStream(source_stream, start_line, start_col, options, source_pathname, module_name));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, NULL, 0));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, start_line, start_col, options, source_pathname, source_pathname ? strlen(source_pathname) : 0, module_name, module_namesize));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, start_line, start_col, options, source_pathname, source_pathsize, NULL, 0));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, start_line, start_col, options, source_pathname, source_pathsize, module_name, module_name ? strlen(module_name) : 0));
	}
	static module opensourcestream(obj_file source_stream, int start_line, int start_col, struct Dee_compiler_options *options, /*utf-8*/ char const *source_pathname, size_t source_pathsize, /*utf-8*/ char const *module_name, size_t module_namesize) DEE_CXX_NOTHROW {
		return inherit(DeeModule_OpenSourceStreamString(source_stream, start_line, start_col, options, source_pathname, source_pathsize, module_name, module_namesize));
	}
	bool isglobal() const DEE_CXX_NOTHROW {
		return DeeModule_IsGlobal((DeeObject *)*this);
	}
	unsigned int loadsourcestream(obj_file input_file, struct Dee_compiler_options *options) {
		return throw_if_negative(DeeModule_LoadSourceStream(*this, input_file, 0, 0, options));
	}
	unsigned int loadsourcestream(obj_file input_file, int start_line = 0, int start_col = 0, struct Dee_compiler_options *options = NULL) {
		return throw_if_negative(DeeModule_LoadSourceStream(*this, input_file, start_line, start_col, options));
	}
};


inline module import(obj_string module_name, struct Dee_compiler_options *options = NULL) {
	return inherit(DeeModule_OpenGlobal(module_name, options, true));
}
inline module import(/*utf-8*/ char const *__restrict module_name, struct Dee_compiler_options *options = NULL) {
	return inherit(DeeModule_OpenGlobalString(module_name, strlen(module_name), options, true));
}
inline module import(/*utf-8*/ char const *__restrict module_name, size_t module_namesize, struct Dee_compiler_options *options = NULL) {
	return inherit(DeeModule_OpenGlobalString(module_name, module_namesize, options, true));
}


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_MODULE_H */
