/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_MODGEN_C
#define GUARD_DEEMON_COMPILER_ASM_MODGEN_C 1
#define Dee_WANT_CODE_OBJECT__co_next

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TryReallocc */
#include <deemon/code.h>               /* DeeCodeObject, DeeCode_Type */
#include <deemon/compiler/assembler.h> /*  */
#include <deemon/compiler/compiler.h>  /* DeeCompiler_LockWriting */
#include <deemon/compiler/symbol.h>    /* DeeRootScope_Type, current_rootscope */
#include <deemon/gc.h>                 /* DeeGCObject_Callocc, DeeGCObject_Malloc, DeeGC_TRACK */
#include <deemon/module.h>             /* DeeModuleDee_Type, DeeModuleObject, Dee_MODSYM_FDOCOBJ, Dee_MODSYM_FNAMEOBJ, Dee_MODULE_FHASBUILDID, Dee_module_object, Dee_module_symbol */
#include <deemon/object.h>             /* ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_Type, DeeTypeObject, Dee_Decref*, Dee_Incref, Dee_Movrefv, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t, Dee_serial */
#include <deemon/string.h>             /* DeeStringObject */
#include <deemon/system-features.h>    /* memcpyc */
#include <deemon/system.h>             /* DeeSystem_GetWalltime */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_VAR, TF_NONE, TP_F* */

#include <hybrid/int128.h> /* __hybrid_uint128_set64 */

#ifndef CONFIG_NO_DEC
#endif /* !CONFIG_NO_DEC */
/**/

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uint16_t, uint64_t */

DECL_BEGIN

#undef container_of
#define container_of COMPILER_CONTAINER_OF

INTDEF struct Dee_module_symbol empty_module_buckets[];

/* Compile a new module, using `current_rootscope' for module information,
 * and the given code object as root code executed when the module is loaded.
 * WARNING: During this process a lot of data is directly inherited from
 *         `current_rootscope' by the returned module object, meaning that the
 *          root scope will have been reset to an empty (or near empty) state.
 * #ifndef CONFIG_EXPERIMENTAL_MMAP_DEC
 * @param: flags: Set of `ASM_F*' (Assembly flags; see above)
 * #endif // !CONFIG_EXPERIMENTAL_MMAP_DEC */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
typedef struct {
	OBJECT_HEAD
} ModuleCurrent;

INTERN DeeTypeObject DeeModuleCurrent_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ NULL,
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FVARIABLE | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_destroy     = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
};

/* Create a dummy object of "size" bytes that can be used as the "current-module-marker"
 * We used to use a statically allocated object for this, and that (seemed to) work fine,
 * until someone built deemon with a long-untested build environment.
 *
 * If this was a statically allocated object, it would have a fixed, maximum object size,
 * so when `DeeSerial_GCObject_Calloc()' allocates the dec version of the module, it might
 * end up mapping some other (unrelated) statically allocated objects from the deemon core
 * to "current_module_marker" also. And well... that ended up happening (in that case: it
 * ended up overlapping with `DeeCode_Type', which then broke the module root initializer
 * function of any user-code module with a sufficiently great number of global variables,
 * since the size of the root module, und thus "current_module_marker" depends on the #
 * of global variables)
 *
 * The solution for all of this is to just dynamically allocated "current_module_marker",
 * so it always reserves (at least) as much memory as it needs to reserve.
 *
 * However: things are still a little more complicated than that, since modules need to
 *          be GC objects, so the "current_module_marker", also needs to be a GC object. */
PRIVATE WUNUSED DREF ModuleCurrent *DCALL
create_current_module_marker(size_t size) {
	DREF ModuleCurrent *result;
	result = (DREF ModuleCurrent *)DeeGCObject_Malloc(size);
	if unlikely(!result)
		goto err;
	DeeObject_InitStatic(result, &DeeModuleCurrent_Type);
	return DeeGC_TRACK(ModuleCurrent, result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
module_compile(struct Dee_serial *__restrict writer,
               /*inherit(always)*/ DREF DeeCodeObject *__restrict root_code) {
	DeeModuleObject *out_module;
	Dee_seraddr_t addrof_module;
	size_t sizeof_module;
	DREF ModuleCurrent *current_module_marker;
	ASSERT(DeeCompiler_LockWriting());
	ASSERT_OBJECT_TYPE_EXACT(root_code, &DeeCode_Type);
	ASSERT_OBJECT_TYPE(&current_rootscope->rs_scope.bs_scope, &DeeRootScope_Type);
	ASSERT(current_rootscope->rs_code == root_code);
	ASSERT(root_code->ob_refcnt == 2);

	/* Allocate the placeholder buffer for the module object */
	sizeof_module = offsetof(DeeModuleObject, mo_globalv) +
	                current_rootscope->rs_globalc * sizeof(DREF DeeObject *);
	current_module_marker = create_current_module_marker(sizeof_module);
	if unlikely(!current_module_marker)
		goto err;

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
				iter->co_module = (DeeModuleObject *)current_module_marker;
				Dee_Incref(current_module_marker);
				Dee_Decref_unlikely(iter);
			}
			iter = next;
		}
	}

	/* Allocate the primary buffer for the module object.
	 * Really hacky: we need to allocate it as a deemon-module object */
	current_module_marker->ob_type = &DeeModuleDee_Type;
	addrof_module = DeeSerial_GCObject_Calloc(writer, sizeof_module, current_module_marker);
	current_module_marker->ob_type = &DeeModuleCurrent_Type;
	Dee_Decref_unlikely(current_module_marker);
	if (!Dee_SERADDR_ISOK(addrof_module))
		goto err;
#define ADDROF(field) (addrof_module + offsetof(DeeModuleObject, field))

	/* Initialize basic fields */
	out_module = DeeSerial_Addr2Mem(writer, addrof_module, DeeModuleObject);
	out_module->mo_globalc = current_rootscope->rs_globalc;
	out_module->mo_importc = current_rootscope->rs_importc;
	out_module->mo_bucketm = current_rootscope->rs_bucketm;
	out_module->mo_flags   = current_rootscope->rs_flags | Dee_MODULE_FHASBUILDID;

	/* These get overwritten later (as the MD5 hash of the dec file) */
	out_module->mo_buildid.mbi_word64[0] = 0;
	out_module->mo_buildid.mbi_word64[1] = 0;

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
				if (*in__mo_bucketv->ss_doc == '\0') {
					/* No point in saving an empty string; that just bloats the .dec file! */
					struct Dee_module_symbol *ou_sym;
					ou_sym = DeeSerial_Addr2Mem(writer, out__mo_bucketv_i, struct Dee_module_symbol);
					ou_sym->ss_doc = NULL;
					ou_sym->ss_flags &= ~Dee_MODSYM_FDOCOBJ;
					status = 0;
				} else if (in__mo_bucketv->ss_flags & Dee_MODSYM_FDOCOBJ) {
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
INTERN WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
module_compile(/*inherit(always)*/ DREF DeeCodeObject *__restrict root_code) {
	DREF DeeModuleObject *mod;
	ASSERT(DeeCompiler_LockWriting());
	ASSERT_OBJECT_TYPE_EXACT(root_code, &DeeCode_Type);
	ASSERT_OBJECT_TYPE(&current_rootscope->rs_scope.bs_scope, &DeeRootScope_Type);
	ASSERT(current_rootscope->rs_code == root_code);
	ASSERT(root_code->ob_refcnt == 2);

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
	mod = (DREF DeeModuleObject *)DeeGCObject_Callocc(offsetof(DeeModuleObject, mo_globalv),
	                                                  current_rootscope->rs_globalc,
	                                                  sizeof(DREF DeeObject *));
	if unlikely(!mod)
		goto err;
	DeeObject_InitStatic(mod, &DeeModuleDee_Type);
	mod->mo_globalc = current_rootscope->rs_globalc;
	mod->mo_importc = current_rootscope->rs_importc;
	mod->mo_bucketm = current_rootscope->rs_bucketm;
	mod->mo_bucketv = current_rootscope->rs_bucketv;
	mod->mo_importv = current_rootscope->rs_importv;
	mod->mo_moddata.mo_rootcode = root_code; /* Inherit reference */
	mod->mo_flags = current_rootscope->rs_flags | Dee_MODULE_FHASBUILDID;
	{
		uint64_t ts = DeeSystem_GetWalltime();
		__hybrid_uint128_set64(mod->mo_buildid, ts);
	}

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

	return mod;
err:
	Dee_DecrefNokill(root_code); /* *Nokill because "current_rootscope->rs_code == root_code" */
	return NULL;
}
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_MODGEN_C */
