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
#ifndef GUARD_DEEMON_EXECUTE_DEX_C
#define GUARD_DEEMON_EXECUTE_DEX_C 1

#include <deemon/api.h>

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#include <deemon/dex.h>       /* DeeDexObject, DeeDex_Check, Dee_dex, Dee_dex_symbol */
#include <deemon/module.h>    /* DeeModule*, Dee_MODSYM_F*, Dee_MODULE_FDIDINIT, Dee_MODULE_FDIDLOAD, Dee_MODULE_HASHNX, Dee_module_symbol */
#include <deemon/object.h>    /* ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_Check, DeeObject_Type, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_DecrefIfNotOne, Dee_Incref, Dee_XClear, Dee_XIncref, Dee_hash_t, Dee_weakref_support_init, OBJECT_HEAD_INIT */
#include <deemon/type.h>      /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, TF_NONE, TP_FGC, TP_FNORMAL */
#include <deemon/util/hash.h> /* Dee_HashStr */

#ifndef CONFIG_NO_DEX
#include <deemon/alloc.h>              /* Dee_*alloc*, Dee_Free, Dee_Freea, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/computed-operators.h>
#include <deemon/error.h>              /* DeeError_*, Dee_ERROR_HANDLED_RESTORE */
#include <deemon/format.h>             /* Dee_sprintf, PRFuSIZ */
#include <deemon/gc.h>                 /* DeeGCObject_CALLOC, DeeGC_Track */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/system-features.h>    /* CONFIG_HAVE_*, DeeSystem_DlOpen_USE_LoadLibrary, DeeSystem_DlOpen_USE_dlopen, dl_iterate_phdr, dladdr, dlgethandle, dlopen, memcpyc, strcmp, strlen */
#include <deemon/system.h>             /* DeeSystem_DlClose, DeeSystem_DlSym */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_* */

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/host.h>            /* __ARCH_PAGESIZE, __i386__, __pic__, __x86_64__ */
#include <hybrid/typecore.h>        /* __BYTE_TYPE__ */

#ifndef CONFIG_NO_DEC
#endif /* !CONFIG_NO_DEC */

#ifdef DeeSystem_DlOpen_USE_LoadLibrary
#include <Windows.h>
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* UINT16_MAX, uint8_t, uint16_t, uintptr_t */
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */

#ifdef CONFIG_HAVE_LINK_H
#include <link.h>
#endif /* CONFIG_HAVE_LINK_H */

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */


DECL_BEGIN

INTDEF struct Dee_module_symbol empty_module_buckets[];

/* Try to load an extension file.
 * NOTE: This isn't where the dex gets initialized!
 * @return:  0: The extension was successfully loaded.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1)) int DCALL
dex_load_handle(DeeDexObject *__restrict self,
                void *handle,
                DeeObject *__restrict input_file) {
	struct Dee_module_symbol *modsym;
	struct Dee_dex_symbol const *symbols;
	struct Dee_dex *descriptor;
	DREF DeeObject **globals;
	size_t symcount, glbcount;
	uint16_t symi, bucket_mask;
	descriptor = (struct Dee_dex *)DeeSystem_DlSym(handle, "DEX");
	if (!descriptor)
		descriptor = (struct Dee_dex *)DeeSystem_DlSym(handle, "_DEX");
	DBG_ALIGNMENT_ENABLE();
	if (!descriptor) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Dex extension %r does not export a descriptor table",
		                input_file);
		goto err;
	}
	self->d_handle = handle;
	self->d_dex    = descriptor;

	/* Load the extension's symbol table. */
	symbols  = descriptor->d_symbols;
	symcount = 0;
	glbcount = 0;
	if (symbols) {
		while (symbols->ds_name) {
			if ((symbols->ds_flags & (Dee_MODSYM_FPROPERTY | Dee_MODSYM_FREADONLY)) == Dee_MODSYM_FPROPERTY) {
				symbols += 2;
				glbcount += 2;
			}
			++symbols;
			++symcount;
		}
	}
	glbcount += symcount;
	if unlikely(glbcount > UINT16_MAX) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Dex extension %r is too large",
		                input_file);
		goto err;
	}
	ASSERT(glbcount >= symcount);
	/* Generate the global variable table. */
	symbols = descriptor->d_symbols;
	globals = (DREF DeeObject **)Dee_Mallocc(glbcount, sizeof(DREF DeeObject *));
	if unlikely(!globals)
		goto err;
	/* Figure out how large the hash-mask should be. */
	bucket_mask = 1;
	while (bucket_mask < symcount)
		bucket_mask <<= 1;
	if ((bucket_mask - symcount) < 16)
		bucket_mask <<= 1;
	--bucket_mask;
	modsym = (struct Dee_module_symbol *)Dee_Callocc(bucket_mask + 1,
	                                             sizeof(struct Dee_module_symbol));
	if unlikely(!modsym)
		goto err_glob;
	/* Set the symbol table and global variable vector. */
	for (symi = 0; symi < (uint16_t)glbcount; ++symi) {
		struct Dee_dex_symbol const *sym = &symbols[symi];
		Dee_hash_t i, perturb, hash;
		ASSERT(sym->ds_name);
		ASSERTF(!sym->ds_obj || DeeObject_Check(sym->ds_obj),
		        "Invalid object %p exported: `%s' by `%s'",
		        sym->ds_obj, sym->ds_name, DeeString_STR(input_file));
		hash    = Dee_HashStr(sym->ds_name);
		perturb = i = hash & bucket_mask;
		for (;; Dee_MODULE_HASHNX(i, perturb)) {
			struct Dee_module_symbol *target = &modsym[i & bucket_mask];
			if (target->ss_name)
				continue;
			target->ss_name  = sym->ds_name;
			target->ss_doc   = sym->ds_doc;
			target->ss_index = symi;
			target->ss_hash  = hash;
			target->ss_flags = (uint8_t)sym->ds_flags;
			ASSERT(!(sym->ds_flags & (Dee_MODSYM_FNAMEOBJ | Dee_MODSYM_FDOCOBJ)));
			break;
		}
		/* Safe the proper initialization object in the global table. */
		globals[symi] = sym->ds_obj;
		Dee_XIncref(sym->ds_obj);
		if ((sym->ds_flags & (Dee_MODSYM_FPROPERTY | Dee_MODSYM_FREADONLY)) == Dee_MODSYM_FPROPERTY) {
			/* Initialize a property */
			globals[symi + 1] = sym[1].ds_obj;
			globals[symi + 2] = sym[2].ds_obj;
			Dee_XIncref(sym[1].ds_obj);
			Dee_XIncref(sym[2].ds_obj);
			symi += 2;
		}
	}
	/* Write the tables into the module descriptor. */
	self->d_module.mo_importc = 0;
	self->d_module.mo_importv = NULL;
	self->d_module.mo_globalc = (uint16_t)glbcount;
	self->d_module.mo_globalv = globals;
	self->d_module.mo_bucketm = bucket_mask;
	self->d_module.mo_bucketv = modsym;
	return 0;
err_glob:
	Dee_Free(globals);
err:
	DeeSystem_DlClose(handle);
	return -1;
}

/* Special handling needed to deal with the "@NN"
 * suffix caused by STDCALL functions on i386-pe */
#undef NEED_DeeModule_GetNativeSymbol_AT_SUFFIX
#if defined(__i386__) && !defined(__x86_64__) && defined(__PE__)
#define NEED_DeeModule_GetNativeSymbol_AT_SUFFIX
#endif /* __i386__ && !__x86_64__ && __PE__ */

/* Return the export address of a native symbol exported from a dex `self'.
 * When `self' isn't a dex, but a regular module, or if the symbol wasn't found, return `NULL'.
 * NOTE: Because native symbols cannot appear in user-defined modules,
 *       in the interest of keeping native functionality to its bare
 *       minimum, any code making using of this function should contain
 *       a fallback that calls a global symbol of the module, rather
 *       than a native symbol:
 * >> static int (*p_add)(int x, int y) = NULL;
 * >> if (!p_add)
 * >>     *(void **)&p_add = DeeModule_GetNativeSymbol(IMPORTED_MODULE, "add");
 * >> // Fallback: Invoke a member attribute `add' if the native symbol doesn't exist.
 * >> if (!p_add)
 * >>     return DeeObject_CallAttrStringf(IMPORTED_MODULE, "add", "dd", x, y);
 * >> // Invoke the native symbol.
 * >> return DeeInt_NewInt((*p_add)(x, y)); */
PUBLIC WUNUSED NONNULL((1, 2)) void *DCALL
DeeModule_GetNativeSymbol(DeeModuleObject *__restrict self,
                          char const *__restrict name) {
	void *result;
	DeeDexObject *me = (DeeDexObject *)self;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (!DeeDex_Check(self) || !(me->d_module.mo_flags & Dee_MODULE_FDIDLOAD))
		return NULL;
	result = DeeSystem_DlSym(me->d_handle, name);
	if (!result) {
#ifdef NEED_DeeModule_GetNativeSymbol_AT_SUFFIX
		/* Try again after inserting an underscore. */
		char *temp_name;
		size_t namelen = strlen(name);
#ifdef Dee_MallocaNoFailc
		Dee_MallocaNoFailc(temp_name, namelen + 6, sizeof(char));
#else /* Dee_MallocaNoFailc */
		temp_name = (char *)Dee_Mallocac(namelen + 6, sizeof(char));
		if unlikely(!temp_name) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
			return NULL; /* ... Technically not correct, but if memory has gotten
			              *     this low, that's the last or the user's problems. */
		}
#endif /* !Dee_MallocaNoFailc */
		memcpyc(temp_name + 1, name, namelen + 1, sizeof(char));
		temp_name[0] = '_';
		result = DeeSystem_DlSym(me->d_handle, temp_name);
		if (!result) {
			/* Try to append (in order): "@0", "@4", "@8", "@12", "@16", "@20", "@24", "@28", "@32" */
			size_t n;
			for (n = 0; n <= 32; n += 4) {
				Dee_sprintf(temp_name + 1 + namelen, "@%" PRFuSIZ, n);

				/* Try without leading '_' */
				result = DeeSystem_DlSym(me->d_handle, temp_name + 1);
				if (result)
					break;

				/* Try with leading '_' */
				result = DeeSystem_DlSym(me->d_handle, temp_name);
				if (result)
					break;
			}
		}
		Dee_Freea(temp_name);

#else /* NEED_DeeModule_GetNativeSymbol_AT_SUFFIX */
		/* Try again after inserting an underscore. */
#ifdef __ARCH_PAGESIZE_MIN
		if (((uintptr_t)(name) & ~(__ARCH_PAGESIZE_MIN - 1)) ==
		    ((uintptr_t)(name - 1) & ~(__ARCH_PAGESIZE_MIN - 1)) &&
		    name[-1] == '_') {
			result = DeeSystem_DlSym(me->d_handle, name - 1);
		} else
#endif /* __ARCH_PAGESIZE_MIN */
		{
			char *temp_name;
			size_t namelen = strlen(name);
#ifdef Dee_MallocaNoFailc
			Dee_MallocaNoFailc(temp_name, namelen + 2, sizeof(char));
#else /* Dee_MallocaNoFailc */
			temp_name = (char *)Dee_Mallocac(namelen + 2, sizeof(char));
			if unlikely(!temp_name) {
				DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
				return NULL; /* ... Technically not correct, but if memory has gotten
				              *     this low, that's the last or the user's problems. */
			}
#endif /* !Dee_MallocaNoFailc */
			memcpyc(temp_name + 1, name,
			        namelen + 1,
			        sizeof(char));
			temp_name[0] = '_';
			result = DeeSystem_DlSym(me->d_handle, temp_name);
			Dee_Freea(temp_name);
		}
#endif /* !NEED_DeeModule_GetNativeSymbol_AT_SUFFIX */
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDex_New(DeeObject *__restrict name) {
	DREF DeeDexObject *result;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	result = DeeGCObject_CALLOC(DeeDexObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(&result->d_module, &DeeDex_Type);
	result->d_module.mo_name    = (DeeStringObject *)name;
	result->d_module.mo_bucketv = empty_module_buckets;
	Dee_Incref(name);
	Dee_weakref_support_init(&result->d_module);
	return DeeGC_Track(Dee_AsObject(&result->d_module));
err:
	return NULL;
}

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t dex_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define dex_lock_reading()    Dee_atomic_rwlock_reading(&dex_lock)
#define dex_lock_writing()    Dee_atomic_rwlock_writing(&dex_lock)
#define dex_lock_tryread()    Dee_atomic_rwlock_tryread(&dex_lock)
#define dex_lock_trywrite()   Dee_atomic_rwlock_trywrite(&dex_lock)
#define dex_lock_canread()    Dee_atomic_rwlock_canread(&dex_lock)
#define dex_lock_canwrite()   Dee_atomic_rwlock_canwrite(&dex_lock)
#define dex_lock_waitread()   Dee_atomic_rwlock_waitread(&dex_lock)
#define dex_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&dex_lock)
#define dex_lock_read()       Dee_atomic_rwlock_read(&dex_lock)
#define dex_lock_write()      Dee_atomic_rwlock_write(&dex_lock)
#define dex_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&dex_lock)
#define dex_lock_upgrade()    Dee_atomic_rwlock_upgrade(&dex_lock)
#define dex_lock_downgrade()  Dee_atomic_rwlock_downgrade(&dex_lock)
#define dex_lock_endwrite()   Dee_atomic_rwlock_endwrite(&dex_lock)
#define dex_lock_endread()    Dee_atomic_rwlock_endread(&dex_lock)
#define dex_lock_end()        Dee_atomic_rwlock_end(&dex_lock)

/* [0..1][lock(dex_lock)] Global chain of loaded dex extensions. */
PRIVATE DREF DeeDexObject *dex_chain;

/* Clear global data caches of all loaded dex modules. */
INTERN bool DCALL DeeDex_Cleanup(void) {
	bool result = false;
	DREF DeeDexObject *dex;
	dex_lock_read();
	/* NOTE: Since DEX modules are only actually removed
	 *       at a later phase, we can still be sure that
	 *       we can safely traverse the list and simply
	 *       hold the DEX-lock while walking entries,
	 *       knowing that our current entry won't just
	 *       randomly disappear. */
	for (dex = dex_chain; dex;
	     dex = dex->d_next) {
		dex_lock_endread();
		/* Invoke the clear-operator on the dex (If it implements it). */
		if (dex->d_dex->d_clear && (*dex->d_dex->d_clear)())
			result = true;
		dex_lock_read();
	}
	dex_lock_endread();
	return result;
}

/* Unload all loaded dex modules. */
INTERN void DCALL DeeDex_Finalize(void) {
	DREF DeeDexObject *dex;
again:
	dex_lock_write();
	while ((dex = dex_chain) != NULL) {
		dex_chain    = dex->d_next;
		dex->d_pself = NULL;
		dex->d_next  = NULL;
		if (!Dee_DecrefIfNotOne(&dex->d_module)) {
			dex_lock_endwrite();
			Dee_Decref(&dex->d_module);
			goto again;
		}
	}
	dex_lock_endwrite();
}


/* Initialize the given dex module. */
INTERN WUNUSED NONNULL((1)) int DCALL
dex_initialize(DeeDexObject *__restrict self) {
	int (DCALL *func)(void);
	ASSERT(self->d_dex);
	func = self->d_dex->d_init;
	if (func && (*func)())
		goto err;

	/* Register the dex in the global chain. */
	dex_lock_write();
	if ((self->d_next = dex_chain) != NULL)
		dex_chain->d_pself = &self->d_next;
	self->d_pself = &dex_chain;
	dex_chain     = self;
	Dee_Incref(&self->d_module); /* The reference stored in the dex chain. */
	dex_lock_endwrite();
	return 0;
err:
	return -1;
}

INTDEF size_t DCALL Dee_membercache_clearall(size_t max_clear);

PRIVATE NONNULL((1)) void DCALL
dex_fini(DeeDexObject *__restrict self) {
	ASSERT(!self->d_pself);
	if (self->d_module.mo_flags & Dee_MODULE_FDIDLOAD) {
		uint16_t i;
		/* Clear global variables before we unload the module,
		 * because most likely they're all still pointing inside. */
		for (i = 0; i < self->d_module.mo_globalc; ++i)
			Dee_XClear(self->d_module.mo_globalv[i]);
		ASSERT(self->d_dex);
		if ((self->d_module.mo_flags & Dee_MODULE_FDIDINIT) &&
		    (self->d_dex->d_fini != NULL))
			(*self->d_dex->d_fini)();

		/* Must clear membercaches that may have been loaded by
		 * this extension before unloading the associated library.
		 * If we don't do this before, dangling points may be left
		 * in the global chain of active membercaches.
		 * XXX: Only do this for caches apart of this module's static binary image? */
		Dee_membercache_clearall((size_t)-1);
#if 0
		/* FIXME: Work-around for preventing DEX modules being unloaded
		 *        while objects referring to statically defined types inside
		 *        still exist.
		 *        The proper way to fix this would be to use a GC-like mechanism
		 *        for this that delays the actual DEX unloading until no objects
		 *        exist that are apart of, or use types from the DEX.
		 *        Note that for this, we must also change the ruling that objects
		 *        pointing to other objects don't have to implement GC-visit if it
		 *        is known that none of those objects are ever GC objects. With this
		 *        change, _all_ objects have to implement GC-visit! (though we could
		 *        add a type-flag to indicate that pointed-to objects can never form
		 *        a ~conventional~ loop) */
		DeeSystem_DlClose(self->d_handle);
#endif
	}
}


#if 0
PRIVATE NONNULL((1, 2)) void DCALL
dex_visit(DeeDexObject *__restrict self,
          Dee_visit_t proc, void *arg) {
	if (self->d_module.mo_flags & Dee_MODULE_FDIDLOAD) {
		struct Dee_dex_symbol *iter;
		ASSERT(self->d_dex);
		iter = self->d_dex->d_symbols;
		if (iter) {
			for (; iter->ds_name; ++iter)
				Dee_XVisit(iter->ds_obj);
		}
	}
}
#endif


PUBLIC DeeTypeObject DeeDex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DexModule",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeModule_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DeeDexObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dex_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&module_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&module_printrepr),
	},
	/* .tp_visit         = */ NULL, // (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dex_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__8C153DCE147F6A78),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

/* Using `RTLD_DI_LINKMAP' isn't necessary on KOS */
#ifdef __KOS__
#undef CONFIG_HAVE_dlinfo__RTLD_DI_LINKMAP
#endif /* __KOS__ */

#ifdef CONFIG_HAVE_dlinfo__RTLD_DI_LINKMAP
#define dlinfo_RTLD_DI_LINKMAP(handle, p_result) \
	(void)(dlinfo(handle, RTLD_DI_LINKMAP, (void *)(p_result)) || (*(p_result) = (struct link_map *)(handle), 1))
#else /* CONFIG_HAVE_dlinfo__RTLD_DI_LINKMAP */
#define dlinfo_RTLD_DI_LINKMAP(handle, p_result) \
	(void)(*(p_result) = (struct link_map *)(handle))
#endif /* !CONFIG_HAVE_dlinfo__RTLD_DI_LINKMAP */



#undef DeeModule_OfPointer_USE_GetModuleHandleExW
#undef DeeModule_OfPointer_USE_dlgethandle
#undef DeeModule_OfPointer_USE_dl_iterate_phdr
#undef DeeModule_OfPointer_USE_xdlmodule_info
#undef DeeModule_OfPointer_USE_dladdr1__RTLD_DL_LINKMAP
#undef DeeModule_OfPointer_USE_dladdr__dli_fname
#undef DeeModule_OfPointer_USE_STUB
#ifdef DeeSystem_DlOpen_USE_LoadLibrary
#define DeeModule_OfPointer_USE_GetModuleHandleExW /* Windows */
#elif defined(DeeSystem_DlOpen_USE_dlopen) && defined(CONFIG_HAVE_dlgethandle)
#define DeeModule_OfPointer_USE_dlgethandle /* KOSmk4 */
#elif defined(DeeSystem_DlOpen_USE_dlopen) && defined(__KOS_VERSION__) && (__KOS_VERSION__ >= 300 && __KOS_VERSION__ < 400)
#define DeeModule_OfPointer_USE_xdlmodule_info /* KOSmk3 */
#else /* ... */
/* Under linux, there are many different ways to do this, some of which may not work at runtime. */
#if defined(DeeSystem_DlOpen_USE_dlopen) && defined(CONFIG_HAVE_dladdr1__RTLD_DL_LINKMAP)
#define DeeModule_OfPointer_USE_dladdr1__RTLD_DL_LINKMAP /* Linux */
#endif /* DeeSystem_DlOpen_USE_dlopen && CONFIG_HAVE_dladdr1__RTLD_DL_LINKMAP */
#if defined(DeeSystem_DlOpen_USE_dlopen) && defined(CONFIG_HAVE_dladdr)
#define DeeModule_OfPointer_USE_dladdr__dli_fname /* Linux */
#endif /* DeeSystem_DlOpen_USE_dlopen && CONFIG_HAVE_dladdr */
#if defined(DeeSystem_DlOpen_USE_dlopen) && defined(CONFIG_HAVE_dl_iterate_phdr)
#define DeeModule_OfPointer_USE_dl_iterate_phdr /* Linux */
#endif /* DeeSystem_DlOpen_USE_dlopen && CONFIG_HAVE_dl_iterate_phdr */
#define DeeModule_OfPointer_USE_STUB
#endif /* !... */


DECL_BEGIN

#ifdef DeeSystem_DlOpen_USE_LoadLibrary
#ifdef _MSC_VER
extern /*IMAGE_DOS_HEADER*/ __BYTE_TYPE__ const __ImageBase[];
#define HINST_THISCOMPONENT ((HINSTANCE)__ImageBase)
#else /* _MSC_VER */
/* XXX: This only works when deemon is the primary
 *      binary, but not if it was loaded as a DLL! */
#define HINST_THISCOMPONENT GetModuleHandleW(NULL)
#endif /* !_MSC_VER */
#endif /* DeeSystem_DlOpen_USE_LoadLibrary */



#ifdef DeeModule_OfPointer_USE_dl_iterate_phdr

PRIVATE WUNUSED DREF DeeModuleObject *DCALL
DeeModule_FromElfLoadAddr(ElfW(Addr) addr) {
	DeeDexObject *iter;
	dex_lock_read();
	for (iter = dex_chain; iter; iter = iter->d_next) {
		struct link_map *lm;
		dlinfo_RTLD_DI_LINKMAP(iter->d_handle, &lm);
		if (lm->l_addr != addr)
			continue;
		Dee_Incref(&iter->d_module);
		dex_lock_endread();
		return &iter->d_module;
	}
	dex_lock_endread();
	return NULL;
}

struct iter_modules_data {
	void const           *search_ptr; /* [const] The pointer who's associated module should be located. */
	DREF DeeModuleObject *search_res; /* [0..1][lock(WRITE_ONCE)] The module found to be associated with `search_ptr' */
};

PRIVATE int
iter_modules_callback(struct dl_phdr_info *info,
                      size_t size, void *cookie) {
	ElfW(Half) i;
	struct iter_modules_data *data;
	if unlikely(size < COMPILER_OFFSETAFTER(struct dl_phdr_info, dlpi_phnum))
		return 0;
	data = (struct iter_modules_data *)cookie;
	if unlikely(data->search_res)
		return 1;
	for (i = 0; i < info->dlpi_phnum; ++i) {
		uintptr_t start = (uintptr_t)info->dlpi_addr + (uintptr_t)info->dlpi_phdr[i].p_vaddr;
		uintptr_t end   = (uintptr_t)start + (uintptr_t)info->dlpi_phdr[i].p_memsz;
		if ((uintptr_t)data->search_ptr < start)
			continue;
		if ((uintptr_t)data->search_ptr >= end)
			continue;

		/* Check for the special case of this being the deemon core module. */
		if ((uintptr_t)&DeeObject_Type >= start &&
		    (uintptr_t)&DeeObject_Type < end) {
			data->search_res = DeeModule_GetDeemon();
			Dee_Incref(data->search_res);
		} else {
			/* Given a loaded module, search for its base
			 * address as one of the loaded modules. */
			data->search_res = DeeModule_FromElfLoadAddr(info->dlpi_addr);
		}
		return 1;
	}
	return 0;
}
#endif /* DeeModule_OfPointer_USE_dl_iterate_phdr */



/* Given a static pointer `ptr' (as in: a pointer to some statically allocated structure),
 * try to determine which DEX module (if not the deemon core itself) was used to declare
 * a structure located at that pointer, and return a reference to that module.
 * If this proves to be impossible, or if `ptr' is an invalid pointer, return `NULL'
 * instead, but don't throw an error.
 * When deemon has been built with `CONFIG_NO_DEX', this function will always return
 * a reference to the builtin `deemon' module.
 * @return: * :   A pointer to the dex module (or to `DeeModule_GetDeemon()') that
 *                contains a static memory segment of which `ptr' is apart of.
 * @return: NULL: Either `ptr' is an invalid pointer, part of a library not loaded
 *                as a module, or points to a heap/stack segment.
 *                No matter the case, no error is thrown for this, meaning that
 *                the caller must decide on how to handle this. */
PUBLIC WUNUSED DREF /*Module*/ DeeModuleObject *DCALL
DeeModule_OfPointer(void const *ptr) {
	/* TODO: This function should cache the address ranges of
	 *       native modules in order to speed up operations.
	 *
	 * CONFIG_EXPERIMENTAL_MMAP_DEC needs to call this function once
	 * for **EVERY** object written to a .dec file (to figure out if
	 * the object/function-pointer/etc. is statically allocated by the
	 * deemon core or some external dex module, as opposed to being a
	 * heap pointer)
	 *
	 * This cache should take the form of a R/B-tree where nodes use
	 * the min/max addresses of module segments, whilst pointing to
	 * the relevant DeeModuleObject associated with the address. */

#ifdef DeeModule_OfPointer_USE_GetModuleHandleExW
	HMODULE hTypeModule;
	DBG_ALIGNMENT_DISABLE();
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
	                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
	                       (LPCWSTR)ptr, &hTypeModule)) {
		DeeDexObject *iter;
		DBG_ALIGNMENT_ENABLE();
		dex_lock_read();
		for (iter = dex_chain; iter; iter = iter->d_next) {
			if ((HMODULE)iter->d_handle != hTypeModule)
				continue;
			Dee_Incref(&iter->d_module);
			dex_lock_endread();
			return &iter->d_module;
		}
		dex_lock_endread();
		DBG_ALIGNMENT_DISABLE();
		if (hTypeModule == HINST_THISCOMPONENT) {
			DREF DeeModuleObject *result;
			/* Type is declared as part of the builtin `deemon' module. */
			DBG_ALIGNMENT_ENABLE();
			result = DeeModule_GetDeemon();
			Dee_Incref(result);
			return result;
		}
		DBG_ALIGNMENT_ENABLE();
	}
	return NULL;
#endif /* DeeModule_OfPointer_USE_GetModuleHandleExW */

#ifdef DeeModule_OfPointer_USE_dlgethandle
	/* KOS Mk4 changed up the dynlib API somewhat, such that we
	 * need a different way of translating module pointers.
	 * In Mk4, this is done by:
	 * >> #define _KOS_SOURCE 1
	 * >> #include <dlfcn.h>
	 * >> 
	 * >> static int obj = 0;
	 * >> char const *getNameOfMyModule() {
	 * >> 	void *h;
	 * >> 	char const *name;
	 * >> 	h    = dlgethandle(&obj, DLGETHANDLE_FNORMAL);
	 * >> 	name = dlmodulename(h);
	 * >> 	return name;
	 * >> }
	 * WARNING: When the given `ptr' is invalid, this function
	 *          will clobber `dlerror()'!
	 */
	DeeDexObject *iter;
	void *module_handle;
	module_handle = dlgethandle(ptr, DLGETHANDLE_FNORMAL);
	dex_lock_read();
	for (iter = dex_chain; iter; iter = iter->d_next) {
		if (iter->d_handle != module_handle)
			continue;
		Dee_Incref(Dee_AsObject(&iter->d_module));
		dex_lock_endread();
		return &iter->d_module;
	}
	dex_lock_endread();

	/* Check if we're dealing with the deemon core itself. */
#ifndef __pic__
	/* Without PIC, our binary has to be the main executable, so we
	 * can use `dlopen(NULL)' to get the handle for our own binary. */
	if (module_handle == dlopen(NULL, 0))
#else /* __pic__ */
	if (module_handle == dlgethandle((void *)&DeeModule_OfPointer, DLGETHANDLE_FNORMAL))
#endif /* !__pic__ */
	{
		/* It is the deemon core. */
		DREF DeeModuleObject *result;
		result = DeeModule_GetDeemon();
		Dee_Incref(result);
		return result;
	}
	return NULL;
#endif /* DeeModule_OfPointer_USE_dlgethandle */

#ifdef DeeModule_OfPointer_USE_xdlmodule_info
	struct module_basic_info info;
	DeeDexObject *iter;
	dex_lock_read();
	for (iter = dex_chain; iter; iter = iter->d_next) {
		if (xdlmodule_info(iter->d_handle, MODULE_INFO_CLASS_BASIC, &info, sizeof(info)) < sizeof(info))
			continue;
		if ((uintptr_t)ptr < info.mi_segstart)
			continue;
		if ((uintptr_t)ptr >= info.mi_segend)
			continue;
		Dee_Incref(&iter->d_module);
		dex_lock_endread();
		return &iter->d_module;
	}
	dex_lock_endread();
	/* Check if we're dealing with the deemon core itself.
	 * NOTE: This assumes KOS's special linker behavior where any
	 *       pointer apart of a module can also be used as an
	 *       alternative value for that module's handle. */
	if (xdlmodule_info(ptr, MODULE_INFO_CLASS_BASIC, &info, sizeof(info)) >= sizeof(info)) {
		if ((uintptr_t)ptr >= info.mi_segstart && (uintptr_t)ptr < info.mi_segend) {
			/* Now just check if the info relates to the deemon core by checking
			 * if something that is known to be located within the core is also
			 * located with the segment we've just found. */
			if ((uintptr_t)&DeeObject_Type >= info.mi_segstart &&
			    (uintptr_t)&DeeObject_Type < info.mi_segend) {
				/* It is the deemon core. */
				DREF DeeModuleObject *result;
				result = DeeModule_GetDeemon();
				Dee_Incref(result);
				return result;
			}
		}
	}
	return NULL;
#endif /* DeeModule_OfPointer_USE_xdlmodule_info */

#ifdef DeeModule_OfPointer_USE_dladdr1__RTLD_DL_LINKMAP
	/* Compare link map pointers. */
	{
		Dl_info dli;
		struct link_map *ptr_lm = NULL;
		if (dladdr1(ptr, &dli, (void **)&ptr_lm, RTLD_DL_LINKMAP) && ptr_lm) {
			struct link_map *dex_lm;
			DeeDexObject *iter;
			dex_lock_read();
			for (iter = dex_chain; iter; iter = iter->d_next) {
				dlinfo_RTLD_DI_LINKMAP(iter->d_handle, &dex_lm);
				if (dex_lm != ptr_lm)
					continue;
				Dee_Incref(&iter->d_module);
				dex_lock_endread();
				return &iter->d_module;
			}
			dex_lock_endread();

			/* Check if it's the main module. */
			if (dladdr1((void *)&DeeModule_OfPointer, &dli, (void **)&dex_lm, RTLD_DL_LINKMAP) && dex_lm) {
				if (ptr_lm == dex_lm) {
					/* It is the deemon core. */
					DREF DeeModuleObject *result;
					result = DeeModule_GetDeemon();
					Dee_Incref(result);
					return result;
				}
				return NULL;
			}
		}
	}
#endif /* DeeModule_OfPointer_USE_dladdr1__RTLD_DL_LINKMAP */

#ifdef DeeModule_OfPointer_USE_dladdr__dli_fname
#define Dl_info__dli_fname__equal(a, b) \
	((sizeof(((Dl_info *)0)->dli_fname) == sizeof(void *) && (a) == (b)) || strcmp(a, b) == 0)
	/* Compare object filenames. */
	{
		Dl_info dli;
		if (dladdr(ptr, &dli) && dli.dli_fname) {
			Dl_info dex_dli;
			DeeDexObject *iter;
			dex_lock_read();
			for (iter = dex_chain; iter; iter = iter->d_next) {
#ifdef CONFIG_HAVE_struct__link_map__l_name
				/* If possible, try not to make yet another call to `dladdr()' */
				struct link_map *dex_lm;
				dlinfo_RTLD_DI_LINKMAP(iter->d_handle, &dex_lm);
				if (Dl_info__dli_fname__equal(dli.dli_fname, dex_lm->l_name)) {
					Dee_Incref(&iter->d_module);
					dex_lock_endread();
					return &iter->d_module;
				}
#else /* CONFIG_HAVE_struct__link_map__l_name */
				if (dladdr(iter->d_dex, &dex_dli) && dex_dli.dli_fname &&
				    Dl_info__dli_fname__equal(dli.dli_fname, dex_dli.dli_fname)) {
					Dee_Incref(&iter->d_module);
					dex_lock_endread();
					return &iter->d_module;
				}
#endif /* !CONFIG_HAVE_struct__link_map__l_name */
			}
			dex_lock_endread();

			/* Check if it's the main module. */
			if (dladdr((void *)&DeeModule_OfPointer, &dex_dli) && dex_dli.dli_fname) {
				if (Dl_info__dli_fname__equal(dli.dli_fname, dex_dli.dli_fname)) {
					/* It is the deemon core. */
					DREF DeeModuleObject *result;
					result = DeeModule_GetDeemon();
					Dee_Incref(result);
					return result;
				}
				return NULL;
			}
		}
	}
#undef Dl_info__dli_fname__equal
#endif /* DeeModule_OfPointer_USE_dladdr__dli_fname */

#ifdef DeeModule_OfPointer_USE_dl_iterate_phdr
	{
		struct iter_modules_data data;
		data.search_ptr = ptr;
		data.search_res = NULL;
		/* Enumerate all loaded modules. */
		dl_iterate_phdr(&iter_modules_callback, (void *)&data);
		if (data.search_res != NULL)
			return data.search_res;
	}
#endif /* DeeModule_OfPointer_USE_dl_iterate_phdr */

#ifdef DeeModule_OfPointer_USE_STUB
	(void)ptr;
	return NULL;
#endif /* DeeModule_OfPointer_USE_STUB */
}

DECL_END

#else /* !CONFIG_NO_DEX */

DECL_BEGIN

/* Return the export address of a native symbol exported from a dex `self'.
 * When `self' isn't a dex, but a regular module, or if the symbol wasn't found, return `NULL'.
 * NOTE: Because native symbols cannot appear in user-defined modules,
 *       in the interest of keeping native functionality to its bare
 *       minimum, any code making using of this function should contain
 *       a fallback that calls a global symbol of the module, rather
 *       than a native symbol:
 * >> static int (*p_add)(int x, int y) = NULL;
 * >> if (!p_add) *(void **)&p_add = DeeModule_GetNativeSymbol(IMPORTED_MODULE, "add");
 * >> // Fallback: Invoke a member attribute `add' if the native symbol doesn't exist.
 * >> if (!p_add) return DeeObject_CallAttrStringf(IMPORTED_MODULE, "add", "dd", x, y);
 * >> // Invoke the native symbol.
 * >> return DeeInt_NewInt((*p_add)(x, y)); */
PUBLIC WUNUSED NONNULL((1, 2)) void *DCALL
DeeModule_GetNativeSymbol(DeeModuleObject *__restrict self,
                          char const *__restrict name) {
	(void)self;
	(void)name;
	COMPILER_IMPURE();
	return NULL;
}

/* Given a static pointer `ptr' (as in: a pointer to some statically allocated structure),
 * try to determine which DEX module (if not the deemon core itself) was used to declare
 * a structure located at that pointer, and return a reference to that module.
 * If this proves to be impossible, or if `ptr' is an invalid pointer, return `NULL'
 * instead, but don't throw an error.
 * When deemon has been built with `CONFIG_NO_DEX', this function will always return
 * a reference to the builtin `deemon' module.
 * @return: * :   A pointer to the dex module (or to `DeeModule_GetDeemon()') that
 *                contains a static memory segment of which `ptr' is apart of.
 * @return: NULL: Either `ptr' is an invalid pointer, part of a library not loaded
 *                as a module, or points to a heap/stack segment.
 *                No matter the case, no error is thrown for this, meaning that
 *                the caller must decide on how to handle this. */
PUBLIC WUNUSED DREF DeeModuleObject *DCALL
DeeModule_OfPointer(void const *ptr) {
	DREF DeeModuleObject *result;
	(void)ptr;
	COMPILER_IMPURE();
	result = DeeModule_GetDeemon();
	Dee_Incref(result);
	return result;
}

DECL_END

#endif /* CONFIG_NO_DEX */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#endif /* !GUARD_DEEMON_EXECUTE_DEX_C */
