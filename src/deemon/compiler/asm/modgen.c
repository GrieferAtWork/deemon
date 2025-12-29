/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_MODGEN_C
#define GUARD_DEEMON_COMPILER_ASM_MODGEN_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/symbol.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/serial.h>
#include <deemon/system.h> /* DeeSystem_GetWalltime() */
#include <deemon/util/atomic.h>

#ifndef CONFIG_NO_DEC
#include <deemon/compiler/dec.h>
#include <deemon/error.h>
#endif /* !CONFIG_NO_DEC */
/**/

#include <stdint.h> /* uint16_t */

DECL_BEGIN

#undef container_of
#define container_of COMPILER_CONTAINER_OF

INTDEF struct module_symbol empty_module_buckets[];

/* Compile a new module, using `current_rootscope' for module information,
 * and the given code object as root code executed when the module is loaded.
 * WARNING: During this process a lot of data is directly inherited from
 *         `current_rootscope' by the returned module object, meaning that the
 *          root scope will have been reset to an empty (or near empty) state.
 * #ifndef CONFIG_EXPERIMENTAL_MMAP_DEC
 * @param: flags: Set of `ASM_F*' (Assembly flags; see above)
 * #endif // !CONFIG_EXPERIMENTAL_MMAP_DEC */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
INTERN DeeObject current_module_marker = {
	OBJECT_HEAD_INIT(&DeeModuleDee_Type)
};

INTERN WUNUSED NONNULL((1, 2)) int DCALL
module_compile(struct Dee_serial *__restrict writer,
               /*inherit(always)*/ DREF DeeCodeObject *__restrict root_code,
               uint64_t ctime) {
	DeeModuleObject *out_module;
	Dee_seraddr_t addrof_module;
	size_t sizeof_module;
	ASSERT(DeeCompiler_LockWriting());
	ASSERT_OBJECT_TYPE_EXACT(root_code, &DeeCode_Type);
	ASSERT_OBJECT_TYPE(&current_rootscope->rs_scope.bs_scope, &DeeRootScope_Type);
	ASSERT(current_rootscope->rs_code == root_code);
	ASSERT(root_code->ob_refcnt == 2);

	/* Resolve the "co_module" fields of code objects to "current_module_marker"
	 * This is a hacky work-around so we have something that can be linked against-,
	 * and be resolve to the module we're about to write. */
	{
		DREF DeeCodeObject *iter, *next;
		iter = current_rootscope->rs_code;
		current_rootscope->rs_code = NULL;
		while (iter) {
			next = iter->co_next;
			if (Dee_DecrefIfOne(iter)) {
				/* Unused code object? */
			} else {
				iter->co_module = (DeeModuleObject *)&current_module_marker;
				Dee_Incref(&current_module_marker);
				Dee_Decref_unlikely(iter);
			}
			iter = next;
		}
	}

	/* Allocate the primary buffer for the module object */
	sizeof_module = offsetof(DeeModuleObject, mo_globalv) +
	                current_rootscope->rs_globalc * sizeof(DREF DeeObject *);
	addrof_module = DeeSerial_GCObjectCalloc(writer, sizeof_module, &current_module_marker);
	if (!Dee_SERADDR_ISOK(addrof_module))
		goto err;
#define ADDROF(field) (addrof_module + offsetof(DeeModuleObject, field))

	/* Initialize basic fields */
	out_module = DeeSerial_Addr2Mem(writer, addrof_module, DeeModuleObject);
	out_module->mo_globalc = current_rootscope->rs_globalc;
	out_module->mo_importc = current_rootscope->rs_importc;
	out_module->mo_bucketm = current_rootscope->rs_bucketm;
	out_module->mo_flags   = current_rootscope->rs_flags | Dee_MODULE_FHASCTIME;
	out_module->mo_ctime   = ctime;

	/* Output "current_rootscope->rs_bucketv" into "mo_bucketv" */
	if (current_rootscope->rs_bucketv == empty_module_buckets) {
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(mo_bucketv), empty_module_buckets))
			goto err;
	} else {
		size_t i, count = current_rootscope->rs_bucketm + 1;
		Dee_seraddr_t out__mo_bucketv;
		struct Dee_module_symbol *ou__mo_bucketv;
		struct Dee_module_symbol *in__mo_bucketv = current_rootscope->rs_bucketv;
		out__mo_bucketv = DeeSerial_Malloc(writer, count * sizeof(struct Dee_module_symbol), NULL);
		if (!Dee_SERADDR_ISOK(out__mo_bucketv))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(mo_bucketv), out__mo_bucketv))
			goto err;
		ou__mo_bucketv = DeeSerial_Addr2Mem(writer, out__mo_bucketv, struct Dee_module_symbol);
		memcpyc(ou__mo_bucketv, in__mo_bucketv, count, sizeof(struct Dee_module_symbol));
		for (i = 0; i < count; ++i, ++in__mo_bucketv) {
			Dee_seraddr_t out__mo_bucketv_i = out__mo_bucketv + i * sizeof(struct Dee_module_symbol);
			if (in__mo_bucketv->ss_name) {
				int status;
				if (in__mo_bucketv->ss_flags & Dee_MODSYM_FNAMEOBJ) {
					DeeStringObject *ob = container_of(in__mo_bucketv->ss_name, DeeStringObject, s_str);
					status = DeeSerial_PutObjectEx(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_name),
					                               ob, offsetof(DeeStringObject, s_str));
				} else {
					status = DeeSerial_PutPointer(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_name),
					                             in__mo_bucketv->ss_name);
				}
				if unlikely(status)
					goto err;
			}
			if (in__mo_bucketv->ss_doc) {
				int status;
				if (in__mo_bucketv->ss_flags & Dee_MODSYM_FDOCOBJ) {
					DeeStringObject *ob = container_of(in__mo_bucketv->ss_doc, DeeStringObject, s_str);
					status = DeeSerial_PutObjectEx(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_doc),
					                               ob, offsetof(DeeStringObject, s_str));
				} else {
					status = DeeSerial_PutPointer(writer, out__mo_bucketv_i + offsetof(struct Dee_module_symbol, ss_doc),
					                             in__mo_bucketv->ss_doc);
				}
				if unlikely(status)
					goto err;
			}
		}
	}

	/* Output "current_rootscope->rs_importv" into "mo_importv" */
	if (current_rootscope->rs_importv) {
		uint16_t count = current_rootscope->rs_importc;
		Dee_seraddr_t out__mo_importv;
		DREF DeeModuleObject **ou__mo_importv;
		out__mo_importv = DeeSerial_Malloc(writer, count * sizeof(DREF DeeModuleObject *), NULL);
		if (!Dee_SERADDR_ISOK(out__mo_importv))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(mo_importv), out__mo_importv))
			goto err;
		ou__mo_importv = DeeSerial_Addr2Mem(writer, out__mo_importv, DREF DeeModuleObject *);
		Dee_Movrefv(ou__mo_importv, current_rootscope->rs_importv, count);
		if (DeeSerial_InplacePutObjectv(writer, out__mo_importv, count))
			goto err;
	}

	/* XXX: At this point, we'd be able to output static initializers for module
	 *      globals by emitting objects into 'ADDROF(mo_globalv[GID])'. */

	/* Serialize the module's root-code object */
	return DeeSerial_PutObjectInherited(writer, ADDROF(mo_moddata.mo_rootcode), root_code);
#undef ADDROF
err:
	Dee_Decref(root_code);
	return -1;
}
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
INTERN WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
module_compile(/*inherit(always)*/ DREF DeeCodeObject *__restrict root_code, uint64_t ctime)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
module_compile(DeeModuleObject *__restrict mod,
               DeeCodeObject *__restrict root_code,
               uint16_t flags)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
{
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DREF DeeModuleObject *mod;
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	ASSERT(DeeCompiler_LockWriting());
	ASSERT_OBJECT_TYPE_EXACT(root_code, &DeeCode_Type);
	ASSERT_OBJECT_TYPE(&current_rootscope->rs_scope.bs_scope, &DeeRootScope_Type);
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	ASSERT(current_rootscope->rs_code == root_code);
	ASSERT(root_code->ob_refcnt == 2);
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Truncate some vectors before we'll be inheriting them. */
	ASSERT(current_rootscope->rs_importc <= current_rootscope->rs_importa);
	if (current_rootscope->rs_importa > current_rootscope->rs_importc) {
		DREF DeeModuleObject **new_vector;
		new_vector = (DREF DeeModuleObject **)Dee_TryReallocc(current_rootscope->rs_importv,
		                                                      current_rootscope->rs_importc,
		                                                      sizeof(DREF DeeModuleObject *));
		if likely(new_vector)
			current_rootscope->rs_importv = new_vector;
	}

	/* Start filling in members of the module. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	mod = (DREF DeeModuleObject *)DeeGCObject_Callocc(offsetof(DeeModuleObject, mo_globalv),
	                                                  current_rootscope->rs_globalc,
	                                                  sizeof(DREF DeeObject *));
	if unlikely(!mod)
		goto err;
	DeeObject_Init(mod, &DeeModuleDee_Type);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	mod->mo_globalv = (DREF DeeObject **)Dee_Callocc(current_rootscope->rs_globalc,
	                                                 sizeof(DREF DeeObject *));
	if unlikely(!mod->mo_globalv)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	mod->mo_globalc = current_rootscope->rs_globalc;
	mod->mo_importc = current_rootscope->rs_importc;
	mod->mo_bucketm = current_rootscope->rs_bucketm;
	mod->mo_bucketv = current_rootscope->rs_bucketv;
	mod->mo_importv = current_rootscope->rs_importv;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	mod->mo_moddata.mo_rootcode = root_code; /* Inherit reference */
	mod->mo_flags = current_rootscope->rs_flags | Dee_MODULE_FHASCTIME;
	mod->mo_ctime = ctime;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	atomic_or(&mod->mo_flags, current_rootscope->rs_flags);
	mod->mo_root = root_code;
	Dee_Incref(root_code);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Yes, we're just stealing all of these. */
	current_rootscope->rs_importv = NULL;
	current_rootscope->rs_importc = 0;
	current_rootscope->rs_importa = 0;
	current_rootscope->rs_bucketv = empty_module_buckets;
	current_rootscope->rs_bucketm = 0;

	{
		DREF DeeCodeObject *iter, *next;
		iter = current_rootscope->rs_code;
		current_rootscope->rs_code = NULL;
		while (iter) {
			next = iter->co_next;
			iter->co_module = mod;
			Dee_Incref(mod);  /* Create the new module-reference now stored in `iter->co_module'. */
			Dee_Decref(iter); /* This reference was owned by the chain before. */
			iter = next;
		}
	}

	/* Since we're now updated all code objects ever created with the
	 * current module, the given root-code had to have been one of them. */
	ASSERTF(root_code->co_module == mod,
	        "The given root-code was not generated for this module");

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifndef CONFIG_NO_DEC
	/* Save the time when compilation of the module started. */
	mod->mo_ctime = DeeSystem_GetWalltime();
	atomic_or(&mod->mo_flags, Dee_MODULE_FHASCTIME);
#endif /* !CONFIG_NO_DEC */

#ifndef CONFIG_NO_DEC
	/* With the module fully initialized, package it into a compiled DEC file.
	 * NOTE: if the `ASM_FNODEC' flag has been set, don't create a DEC file. */
	if (!(flags & ASM_FNODEC) && dec_create(mod)) {
#if 1
		/* If creation of a DEC file failed because of an illegal
		 * constant, don't fail the entire compilation process. */
		if (!DeeError_Catch(&DeeError_NotImplemented))
			goto err_module;
#else
		goto err_module;
#endif
	}
#endif /* !CONFIG_NO_DEC */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	return mod;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return 0;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifndef CONFIG_NO_DEC
err_module:
	/* Transfer ownership back to the compiler. */
	current_rootscope->rs_importv = (DREF DeeModuleObject **)mod->mo_importv;
	current_rootscope->rs_importc = mod->mo_importc;
	current_rootscope->rs_importa = mod->mo_importc;
	current_rootscope->rs_bucketv = mod->mo_bucketv;
	current_rootscope->rs_bucketm = mod->mo_bucketm;

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DeeGCObject_Free(mod);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	mod->mo_importv = NULL;
	mod->mo_importc = 0;
	mod->mo_bucketv = empty_module_buckets;
	mod->mo_bucketm = 0;

	/* Free the previously allocated global object vector. */
	mod->mo_globalc = 0;
	Dee_Free((void *)mod->mo_globalv);
	mod->mo_globalv = NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#endif /* !CONFIG_NO_DEC */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
err:
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	Dee_DecrefNokill(root_code); /* *Nokill because "current_rootscope->rs_code == root_code" */
	return NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_MODGEN_C */
