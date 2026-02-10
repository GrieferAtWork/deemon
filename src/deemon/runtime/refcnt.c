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
#ifndef GUARD_DEEMON_RUNTIME_REFCNT_C
#define GUARD_DEEMON_RUNTIME_REFCNT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Free, Dee_TryCalloc */
#include <deemon/format.h>          /* PRFXSIZ, PRFuSIZ */
#include <deemon/object.h>          /* ASSERT_OBJECT, ASSERT_OBJECT_AT, DREF, DeeObject, DeeObject_Check, DeeTypeObject, Dee_Decref*, Dee_Incref*, Dee_REFTRACKER_UNTRACKED, Dee_TYPE, Dee_refcnt_t */
#include <deemon/system-features.h> /* CONFIG_HAVE_memsetp, DeeSystem_DEFINE_memsetp, abort, strlen */
#include <deemon/type.h>            /* DeeType_*, Dee_TP_FGC, Dee_TP_FHEAP, Dee_TP_FMAYREVIVE, Dee_refchange, Dee_refchanges, Dee_reftracker, Dee_tp_destroy_t */
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/util/lock.h>       /* Dee_atomic_lock_* */

#include <hybrid/typecore.h> /* __UINTPTR_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* size_t */

#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

#ifdef CONFIG_TRACE_REFCHANGES
PRIVATE void DCALL free_reftracker(struct Dee_reftracker *__restrict self);
#endif /* CONFIG_TRACE_REFCHANGES */

#ifndef CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif // !CONFIG_HAVE_memsetp


#ifndef CONFIG_NO_BADREFCNT_CHECKS
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#define FILE_AND_LINE_FORMAT "%s(%d) : "
#elif defined(CONFIG_DEFAULT_MESSAGE_FORMAT_GCC)
#define FILE_AND_LINE_FORMAT "%s:%d: "
#endif /* ... */

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t bad_refcnt_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define bad_refcnt_lock_available()  Dee_atomic_lock_available(&bad_refcnt_lock)
#define bad_refcnt_lock_acquired()   Dee_atomic_lock_acquired(&bad_refcnt_lock)
#define bad_refcnt_lock_tryacquire() Dee_atomic_lock_tryacquire(&bad_refcnt_lock)
#define bad_refcnt_lock_acquire()    Dee_atomic_lock_acquire(&bad_refcnt_lock)
#define bad_refcnt_lock_waitfor()    Dee_atomic_lock_waitfor(&bad_refcnt_lock)
#define bad_refcnt_lock_release()    Dee_atomic_lock_release(&bad_refcnt_lock)

INTDEF void DCALL assert_print_usercode_trace(void);

PUBLIC NONNULL((1)) void DCALL
DeeFatal_BadIncref(DeeObject *ob, char const *file, int line) {
	DeeTypeObject *type;
	bad_refcnt_lock_acquire();
	Dee_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_INCREF(%p)\n",
	            file, line, ob);
	Dee_DPRINTF("refcnt : %" PRFuSIZ " (%#" PRFXSIZ ")\n",
	            ob->ob_refcnt, ob->ob_refcnt);
	type = Dee_TYPE(ob);
	if (DeeObject_Check(type) && DeeType_Check(type)) {
		Dee_DPRINTF("type : %s (%p)", DeeType_GetName(type), type);
	} else {
		Dee_DPRINTF("type : <INVALID> - %p", type);
	}
	Dee_DPRINTF("\n\n\n");
	bad_refcnt_lock_release();
	assert_print_usercode_trace();
	Dee_BREAKPOINT();
}

PUBLIC NONNULL((1)) void DCALL
DeeFatal_BadDecref(DeeObject *ob, char const *file, int line) {
	DeeTypeObject *type;
	bad_refcnt_lock_acquire();
	Dee_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_DECREF(%p)\n",
	            file, line, ob);
	Dee_DPRINTF("refcnt : %" PRFuSIZ " (%" PRFXSIZ ")\n",
	            ob->ob_refcnt, ob->ob_refcnt);
	type = Dee_TYPE(ob);
	if (DeeObject_Check(type) && DeeType_Check(type)) {
		Dee_DPRINTF("type : %s (%p)", DeeType_GetName(type), type);
	} else {
		Dee_DPRINTF("type : <INVALID> - %p", type);
	}
	Dee_DPRINTF("\n\n\n");
	bad_refcnt_lock_release();
	assert_print_usercode_trace();
	Dee_BREAKPOINT();
}
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
PUBLIC void DCALL
DeeFatal_BadIncref(DeeObject *UNUSED(ob),
                   char const *UNUSED(file),
                   int UNUSED(line)) {
	abort();
}
#ifdef __NO_DEFINE_ALIAS
PUBLIC void DCALL
DeeFatal_BadDecref(DeeObject *UNUSED(ob),
                   char const *UNUSED(file),
                   int UNUSED(line)) {
	abort();
}
#else /* __NO_DEFINE_ALIAS */
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeFatal_BadDecref, 12),
                    DCALL_ASSEMBLY_NAME(DeeFatal_BadIncref, 12));
#endif /* !__NO_DEFINE_ALIAS */
#endif /* CONFIG_NO_BADREFCNT_CHECKS */



#ifdef __INTELLISENSE__
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1(DeeObject *__restrict self);

PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1(DeeObject *__restrict self);

PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_DtorN_GC0_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_DtorN_GC1_Rev0(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_DtorN_GC0_Rev1(DeeObject *__restrict self);
PRIVATE NONNULL((1)) void DCALL DeeObject_DefaultDestroy_DtorN_GC1_Rev1(DeeObject *__restrict self);
#else /* __INTELLISENSE__ */
DECL_END

#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
#include "refcnt-destroy.c.inl"

#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1
#include "refcnt-destroy.c.inl"

#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#include "refcnt-destroy.c.inl"
#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#include "refcnt-destroy.c.inl"

DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0_Rev0 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0_Rev1 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1_Rev0 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1_Rev1 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0_Rev0 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0_Rev1 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1_Rev0 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1_Rev1 DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0_Rev0 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0_Rev1 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1_Rev0 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1_Rev1 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0_Rev0 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0_Rev1 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1_Rev0 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
#define DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1_Rev1 DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1

#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC0_Rev0 DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC0_Rev1 DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC1_Rev0 DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC1_Rev1 DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC0_Rev0 DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC0_Rev1 DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC1_Rev0 DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC1_Rev1 DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC0_Rev0 DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC0_Rev1 DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC1_Rev0 DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC1_Rev1 DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC0_Rev0 DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC0_Rev1 DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC1_Rev0 DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#define DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC1_Rev1 DeeObject_DefaultDestroy_DtorN_GC1_Rev1

/* Object destroy feature flags. */
#define DESTROY_FREV      0x01
#define DESTROY_FGC       0x02
#define DESTROY_FHEAPTYPE 0x04
#define DESTROY_FFREE     0x08
#define DESTROY_FDTOR0    0x00
#define DESTROY_FDTOR1    0x10
#define DESTROY_FDTORN    0x20
#define DESTROY_FDTORX    0x30
#define DESTROY_COUNT     0x30

/*[[[deemon
local tab = ["NULL"] * DESTROY_COUNT;
for (local rev: {false, true}) {
	for (local gc: {false, true}) {
		for (local heapType: {false, true}) {
			for (local free: {false, true}) {
				for (local dtor: {DESTROY_FDTOR0, DESTROY_FDTOR1, DESTROY_FDTORN}) {
					local flags = dtor;
					if (free)
						flags |= DESTROY_FFREE;
					if (heapType)
						flags |= DESTROY_FHEAPTYPE;
					if (gc)
						flags |= DESTROY_FGC;
					if (rev)
						flags |= DESTROY_FREV;
					tab[flags] = f"DeeObject_DefaultDestroy_Dtor{({
						DESTROY_FDTOR0 : "0",
						DESTROY_FDTOR1 : "1",
						DESTROY_FDTORN : "N"
					}[dtor])}_Free{"01"[free]}_HeapType{"01"[heapType]}_GC{"01"[gc]}_Rev{"01"[rev]}";
				}
			}
		}
	}
}

print("PRIVATE Dee_tp_destroy_t tpconst DeeObject_DefaultDestroy_table[DESTROY_COUNT] = {");
for (local x: tab)
	print("\t&", x, ",");
print("};");
]]]*/
PRIVATE Dee_tp_destroy_t tpconst DeeObject_DefaultDestroy_table[DESTROY_COUNT] = {
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0,
	&DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC0_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC0_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC1_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType0_GC1_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC0_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC0_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC1_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free0_HeapType1_GC1_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC0_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC0_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC1_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType0_GC1_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC0_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC0_Rev1,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC1_Rev0,
	&DeeObject_DefaultDestroy_DtorN_Free1_HeapType1_GC1_Rev1,
};
/*[[[end]]]*/

#undef DESTROY_TP_FLAGS_SHIFT_HINT
#if (Dee_TP_FMAYREVIVE >> 12) == DESTROY_FREV
#define DESTROY_TP_FLAGS_SHIFT_HINT 12
#endif


#ifdef DESTROY_TP_FLAGS_SHIFT_HINT
#if ((Dee_TP_FMAYREVIVE >> DESTROY_TP_FLAGS_SHIFT_HINT) != DESTROY_FREV || \
     (Dee_TP_FGC >> DESTROY_TP_FLAGS_SHIFT_HINT) != DESTROY_FGC || \
     (Dee_TP_FHEAP >> DESTROY_TP_FLAGS_SHIFT_HINT) != DESTROY_FHEAPTYPE)
#undef DESTROY_TP_FLAGS_SHIFT_HINT
#endif /* ... */
#endif /* DESTROY_TP_FLAGS_SHIFT_HINT */

LOCAL ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_destroy_t DCALL
DeeType_RequireDestroy_uncached_impl(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter;
	unsigned int flags;
#ifdef DESTROY_TP_FLAGS_SHIFT_HINT
	flags = (self->tp_flags & (Dee_TP_FMAYREVIVE |
	                           Dee_TP_FGC |
	                           Dee_TP_FHEAP)) >>
	        DESTROY_TP_FLAGS_SHIFT_HINT;
#else /* DESTROY_TP_FLAGS_SHIFT_HINT */
	flags = 0;
	if (DeeType_IsGC(self))
		flags |= DESTROY_FGC;
	if (DeeType_HasRevivingDestructor(self))
		flags |= DESTROY_FREV;
	if (DeeType_IsHeapType(self))
		flags |= DESTROY_FHEAPTYPE;
#endif /* !DESTROY_TP_FLAGS_SHIFT_HINT */
	if (self->tp_init.tp_alloc.tp_free)
		flags |= DESTROY_FFREE;
	if (self->tp_init.tp_dtor)
		flags |= DESTROY_FDTOR1;
	for (iter = self->tp_base; iter; iter = iter->tp_base) {
		if (DeeType_HasRevivingDestructor(iter))
			flags |= DESTROY_FREV;
		if (iter->tp_init.tp_dtor)
			flags = (flags & ~DESTROY_FDTORX) | DESTROY_FDTORN;
	}

	/* Select implementation based on type features. */
	return DeeObject_DefaultDestroy_table[flags];
}

PRIVATE ATTR_NOINLINE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_destroy_t DCALL
DeeType_RequireDestroy_uncached(DeeTypeObject *__restrict self) {
	Dee_tp_destroy_t result;
	result = DeeType_RequireDestroy_uncached_impl(self);
	self->tp_init.tp_destroy = result;
	COMPILER_WRITE_BARRIER();
	return result;
}


/* Return a pointer to the optimized implementation of
 * object destruction called by `DeeObject_Destroy()' */
PUBLIC ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tp_destroy_t DCALL
DeeType_RequireDestroy(DeeTypeObject *__restrict self) {
	if likely(self->tp_init.tp_destroy)
		return self->tp_init.tp_destroy;
	return DeeType_RequireDestroy_uncached(self);
}



/* Destroy a given deemon object (called when its refcnt reaches `0') */
#ifdef CONFIG_NO_BADREFCNT_CHECKS
PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy_d)(DeeObject *__restrict self,
                            char const *UNUSED(file),
                            int UNUSED(line)) {
	DeeObject_Destroy(self);
}

PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy)(DeeObject *__restrict self)
#else /* CONFIG_NO_BADREFCNT_CHECKS */
PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy)(DeeObject *__restrict self) {
	DeeObject_Destroy_d(self, NULL, 0);
}

PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy_d)(DeeObject *__restrict self,
                            char const *file, int line)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
{
	DeeTypeObject *type = Dee_TYPE(self);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(self->ob_refcnt != 0) {
		bad_refcnt_lock_acquire();
		Dee_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_DESTROY(%p)\n",
		            file, line, self);
		Dee_DPRINTF("refcnt : %" PRFuSIZ " (%" PRFXSIZ ")\n", self->ob_refcnt, self->ob_refcnt);
		if (DeeObject_Check(type) && DeeType_Check(type)) {
			Dee_DPRINTF("type : %s (%p)", DeeType_GetName(type), type);
		} else {
			Dee_DPRINTF("type : <INVALID> - %p", type);
		}
		Dee_DPRINTF("\n\n\n");
		assert_print_usercode_trace();
		bad_refcnt_lock_release();
		Dee_BREAKPOINT();
	}
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */

#if 0
#ifndef CONFIG_NO_THREADS
	/* Make sure that all threads now see this object as dead.
	 * For why this is required, see `INCREF_IF_NONZERO()' */
	atomic_thread_fence(Dee_ATOMIC_ACQ_REL);
#endif /* !CONFIG_NO_THREADS */
#endif

#if 0 /* Enable extra (expensive) memory checks during object destruction */
	Dee_CHECKMEMORY();
#endif

	/* Make use of an optimized object destructor callback. */
#ifdef __OPTIMIZE_SIZE__
	(*DeeType_RequireDestroy(type))(self);
#else /* __OPTIMIZE_SIZE__ */
	if likely(type->tp_init.tp_destroy) {
		(*type->tp_init.tp_destroy)(self);
		return;
	}
	(*DeeType_RequireDestroy_uncached(type))(self);
#endif /* !__OPTIMIZE_SIZE__ */
}



#ifdef CONFIG_TRACE_REFCHANGES
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t reftracker_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define reftracker_lock_available()  Dee_atomic_lock_available(&reftracker_lock)
#define reftracker_lock_acquired()   Dee_atomic_lock_acquired(&reftracker_lock)
#define reftracker_lock_tryacquire() Dee_atomic_lock_tryacquire(&reftracker_lock)
#define reftracker_lock_acquire()    Dee_atomic_lock_acquire(&reftracker_lock)
#define reftracker_lock_waitfor()    Dee_atomic_lock_waitfor(&reftracker_lock)
#define reftracker_lock_release()    Dee_atomic_lock_release(&reftracker_lock)

PRIVATE struct Dee_reftracker *reftracker_list = NULL;

/* #define REFLEAK_PRINTF(...) fprintf(stderr, __VA_ARGS__) */

#ifndef REFLEAK_PRINTF
#define REFLEAK_PRINT  Dee_DPRINT
#define REFLEAK_PRINTF Dee_DPRINTF
#endif /* !REFLEAK_PRINTF */
#ifndef REFLEAK_PRINT
#define REFLEAK_PRINT(str)  REFLEAK_PRINTF("%s", str)
#define REFLEAK_PRINTS(STR) REFLEAK_PRINTF("%s", STR)
#else /* !REFLEAK_PRINT */
#define REFLEAK_PRINTS Dee_DPRINT
#endif /* REFLEAK_PRINT */

PRIVATE NONNULL((1)) size_t DCALL
print_refchange_len(struct Dee_refchange *__restrict item) {
	size_t result;
	int line = item->rc_line;
	if (line < 0)
		line = -line;
	result = strlen(item->rc_file);
	if (line > 10)
		++result;
	if (line > 100)
		++result;
	if (line > 1000)
		++result;
	if (line > 10000)
		++result;
	if (line > 100000)
		++result;
	return result;
}

PRIVATE NONNULL((1)) Dee_refcnt_t DCALL
print_refchange(struct Dee_refchange *__restrict item,
                Dee_refcnt_t prev_total, size_t maxlen) {
	char mode[2] = { '+', 0 };
	Dee_refcnt_t count, next_total;
	size_t mylen;
	next_total = prev_total;
	if (item->rc_line < 0) {
		--next_total;
		mode[0] = '-';
	} else {
		++next_total;
	}
	REFLEAK_PRINTF("%s(%d) : ", item->rc_file,
	               item->rc_line < 0 ? -item->rc_line : item->rc_line);
	mylen = print_refchange_len(item);
	if (mylen < maxlen) {
		REFLEAK_PRINTF("%*s",
		               (int)(unsigned int)(maxlen - mylen),
		               " ");
	}
	REFLEAK_PRINTF("[%c][%" PRFuSIZ "->%" PRFuSIZ "]", mode[0], prev_total, next_total);
	count = next_total;
	if (count > 15)
		count = 15;
	while (count--)
		REFLEAK_PRINT(mode);
	REFLEAK_PRINTS("\n");
	return next_total;
}

PRIVATE NONNULL((1)) size_t DCALL
print_refchanges_len(struct Dee_refchanges *__restrict item) {
	unsigned int i;
	size_t result = 0;
	if (item->rc_prev)
		result = print_refchanges_len(item->rc_prev);
	for (i = 0; i < COMPILER_LENOF(item->rc_chng); ++i) {
		size_t temp;
		if (!item->rc_chng[i].rc_file)
			break;
		temp = print_refchange_len(&item->rc_chng[i]);
		if (result < temp)
			result = temp;
	}
	return result;
}

PRIVATE NONNULL((1)) Dee_refcnt_t DCALL
do_print_refchanges(struct Dee_refchanges *__restrict item,
                    Dee_refcnt_t prev_total, size_t maxlen) {
	unsigned int i;
	if (item->rc_prev)
		prev_total = do_print_refchanges(item->rc_prev, prev_total, maxlen);
	for (i = 0; i < COMPILER_LENOF(item->rc_chng); ++i) {
		if (!item->rc_chng[i].rc_file)
			break;
		prev_total = print_refchange(&item->rc_chng[i], prev_total, maxlen);
	}
	return prev_total;
}

PRIVATE NONNULL((1)) Dee_refcnt_t DCALL
print_refchanges(struct Dee_refchanges *__restrict item,
                 Dee_refcnt_t prev_total) {
	Dee_refcnt_t result;
	size_t maxlen;
	maxlen = print_refchanges_len(item);
	result = do_print_refchanges(item, prev_total, maxlen);
	return result;
}

INTERN NONNULL((1)) void DCALL
dump_reference_history(DeeObject *__restrict obj) {
	if (!obj->ob_trace)
		return;
	reftracker_lock_acquire();
	print_refchanges(obj->ob_trace->rt_last, 1);
	reftracker_lock_release();
}

PUBLIC void DCALL Dee_DumpReferenceLeaks(void) {
	struct Dee_reftracker *iter;
	reftracker_lock_acquire();
	for (iter = reftracker_list; iter; iter = iter->rt_next) {
		REFLEAK_PRINTF("Object at %p of instance %s leaked %" PRFuSIZ " references:\n",
		               iter->rt_obj, iter->rt_obj->ob_type->tp_name,
		               iter->rt_obj->ob_refcnt);
		print_refchanges(iter->rt_last, 1);
		REFLEAK_PRINTS("\n");
	}
	reftracker_lock_release();
}


PRIVATE NONNULL((1)) void DCALL
add_reftracker(struct Dee_reftracker *__restrict self) {
	reftracker_lock_acquire();
	self->rt_pself = &reftracker_list;
	if ((self->rt_next = reftracker_list) != NULL)
		reftracker_list->rt_pself = &self->rt_next;
	reftracker_list = self;
	reftracker_lock_release();
}

PRIVATE NONNULL((1)) void DCALL
del_reftracker(struct Dee_reftracker *__restrict self) {
	reftracker_lock_acquire();
	if ((*self->rt_pself = self->rt_next) != NULL)
		self->rt_next->rt_pself = self->rt_pself;
	reftracker_lock_release();
}

/* Reference count tracing. */
PRIVATE NONNULL((1)) void DCALL
free_reftracker(struct Dee_reftracker *__restrict self) {
	if (self) {
		struct Dee_refchanges *iter, *next;
		del_reftracker(self);
		iter = self->rt_last;
		while (iter) {
			next = iter->rc_prev;
			if (iter != &self->rt_first)
				Dee_Free(iter);
			iter = next;
		}
		Dee_Free(self);
	}
}

#define DID_DEFINE_DEEOBJECT_FREETRACKER
PUBLIC NONNULL((1)) void DCALL
DeeObject_FreeTracker(DeeObject *__restrict self) {
	free_reftracker(self->ob_trace);
}

PRIVATE WUNUSED NONNULL((1)) struct Dee_reftracker *DCALL
get_reftracker(DeeObject *__restrict self) {
	struct Dee_reftracker *result, *new_result;
	result = self->ob_trace;
	if likely(result)
		goto done;

	/* Allocate a new reference tracker. */
	result = (struct Dee_reftracker *)Dee_TryCalloc(sizeof(struct Dee_reftracker));
	if (!result)
		goto done;
	COMPILER_READ_BARRIER();
	result->rt_obj  = self;
	result->rt_last = &result->rt_first;
	COMPILER_WRITE_BARRIER();
	/* Setup the tracker for use by this object. */
	new_result = atomic_cmpxch_val(&self->ob_trace, NULL, result);
	if unlikely(new_result != NULL) {
		/* Race condition... */
		Dee_Free(result);
		result = new_result;
		goto done;
	}
	/* Keep track of this tracker... */
	add_reftracker(result);
done:
	return result;
}

PRIVATE NONNULL((1)) void DCALL
reftracker_addchange(DeeObject *__restrict ob,
                     char const *file, int line) {
	unsigned int i;
	struct Dee_reftracker *self;
	struct Dee_refchanges *new_changes;
	struct Dee_refchanges *last;
	self = get_reftracker(ob);
	if unlikely(!self || self == Dee_REFTRACKER_UNTRACKED)
		return;
again:
	last = self->rt_last;
	for (i = 0; i < COMPILER_LENOF(last->rc_chng); ++i) {
		if (!atomic_cmpxch(&last->rc_chng[i].rc_file, NULL, file))
			continue;
		last->rc_chng[i].rc_line = line;
		return; /* Got it! */
	}

	/* Must allocate a new set of changes. */
	new_changes = (struct Dee_refchanges *)Dee_TryCalloc(sizeof(struct Dee_refchanges));
	if unlikely(!new_changes)
		return;
	new_changes->rc_chng[0].rc_file = file;
	new_changes->rc_chng[0].rc_line = line;
	new_changes->rc_prev            = last;

	/* Save the new set of changes as the latest set active. */
	if unlikely(!atomic_cmpxch_weak(&self->rt_last, last, new_changes)) {
		Dee_Free(new_changes);
		goto again;
	}
}


PUBLIC NONNULL((1)) void DCALL
Dee_Incref_traced(DeeObject *__restrict ob,
                  char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchinc(&ob->ob_refcnt) == 0)
		DeeFatal_BadIncref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	atomic_inc(&ob->ob_refcnt);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	reftracker_addchange(ob, file, line);
}

PUBLIC NONNULL((1)) void DCALL
Dee_Incref_n_traced(DeeObject *__restrict ob, Dee_refcnt_t n,
                    char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchadd(&ob->ob_refcnt, n) == 0)
		DeeFatal_BadIncref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	atomic_add(&ob->ob_refcnt, n);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	while (n--)
		reftracker_addchange(ob, file, line);
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_IncrefIfNotZero_traced(DeeObject *__restrict ob,
                           char const *file, int line) {
	Dee_refcnt_t oldref;
	do {
		if ((oldref = atomic_read(&ob->ob_refcnt)) == 0)
			return false;
	} while (!atomic_cmpxch_weak(&ob->ob_refcnt, oldref, oldref + 1));
	reftracker_addchange(ob, file, line);
	return true;
}

PUBLIC NONNULL((1)) void DCALL
Dee_Decref_traced(DeeObject *__restrict ob,
                  char const *file, int line) {
	Dee_refcnt_t oldref;
	oldref = atomic_fetchdec(&ob->ob_refcnt);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(oldref == 0)
		DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if unlikely(oldref == 1) {
		DeeObject_Destroy_d(ob, file, line);
	} else {
		reftracker_addchange(ob, file, -line);
	}
}

PUBLIC NONNULL((1)) void DCALL
Dee_Decref_n_traced(DeeObject *__restrict ob, Dee_refcnt_t n,
                    char const *file, int line) {
	Dee_refcnt_t oldref;
	oldref = atomic_fetchsub(&ob->ob_refcnt, n);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(oldref < n)
		DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if unlikely(oldref == n) {
		DeeObject_Destroy_d(ob, file, line);
	} else {
		reftracker_addchange(ob, file, -line);
	}
}

PUBLIC NONNULL((1)) void DCALL
Dee_DecrefDokill_traced(DeeObject *__restrict ob,
                        char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchdec(&ob->ob_refcnt) != 1)
		DeeFatal_BadDecref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	/* Without `CONFIG_NO_BADREFCNT_CHECKS', DeeObject_Destroy doesn't
	 * care about the final reference count, so no need for us to change it. */
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	DeeObject_Destroy_d(ob, file, line);
}

PUBLIC NONNULL((1)) void DCALL
Dee_DecrefNokill_traced(DeeObject *__restrict ob,
                        char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchdec(&ob->ob_refcnt) <= 1)
		DeeFatal_BadDecref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	atomic_dec(&ob->ob_refcnt);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	reftracker_addchange(ob, file, -line);
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_DecrefIfOne_traced(DeeObject *__restrict ob,
                       char const *file, int line) {
	if (!atomic_cmpxch(&ob->ob_refcnt, 1, 0))
		return false;
	DeeObject_Destroy_d(ob, file, line);
	return true;
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_DecrefIfNotOne_traced(DeeObject *__restrict ob,
                          char const *file, int line) {
	Dee_refcnt_t oldref;
	do {
		if ((oldref = atomic_read(&ob->ob_refcnt)) <= 1)
			return false;
	} while (!atomic_cmpxch_weak(&ob->ob_refcnt, oldref, oldref - 1));
	reftracker_addchange(ob, file, -line);
	return true;
}

PUBLIC WUNUSED NONNULL((1)) Dee_refcnt_t DCALL
Dee_DecrefAndFetch_traced(DeeObject *__restrict ob,
                          char const *file, int line) {
	Dee_refcnt_t old_refcnt;
	old_refcnt = atomic_fetchdec(&ob->ob_refcnt);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(old_refcnt == 0)
		DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if unlikely(old_refcnt == 1) {
		DeeObject_Destroy_d(ob, file, line);
		return 0;
	}
	reftracker_addchange(ob, file, -line);
	return old_refcnt - 1;
}

PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv_traced)(DeeObject *const *__restrict object_vector,
                           size_t object_count, char const *file, int line) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Incref_traced(ob, file, line);
	}
	return (DREF DeeObject **)object_vector;
}
PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv_traced)(DREF DeeObject *const *__restrict object_vector,
                           size_t object_count, char const *file, int line) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Decref_traced(ob, file, line);
	}
	return (DREF DeeObject **)object_vector;
}
PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *const *__restrict src,
                           size_t object_count, char const *file, int line) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = src[i];
		Dee_Incref_traced(ob, file, line);
		dst[i] = ob;
	}
	return dst;
}

PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *obj, size_t object_count,
                           char const *file, int line) {
	Dee_Incref_n_traced(obj, object_count, file, line);
	return (DREF DeeObject **)memsetp(dst, obj, object_count);
}
#else /* CONFIG_TRACE_REFCHANGES */

/* Maintain ABI compatibility by always providing traced variants of functions! */


#ifndef CONFIG_NO_BADREFCNT_CHECKS
/* Can still re-use debug information if bad-refcnt checks are enabled. */
#undef DeeObject_Destroy
#undef _DeeFatal_BadIncref
#undef _DeeFatal_BadDecref
#define DeeObject_Destroy(self) DeeObject_Destroy_d(self, file, line)
#define _DeeFatal_BadIncref(ob) DeeFatal_BadIncref((DeeObject *)(ob), file, line)
#define _DeeFatal_BadDecref(ob) DeeFatal_BadDecref((DeeObject *)(ob), file, line)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */

PUBLIC NONNULL((1)) void
(DCALL Dee_Incref_traced)(DeeObject *__restrict ob,
                          char const *file, int line) {
	(void)file;
	(void)line;
	Dee_Incref(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Incref_n_traced)(DeeObject *__restrict ob, Dee_refcnt_t n,
                            char const *file, int line) {
	(void)file;
	(void)line;
	Dee_Incref_n(ob, n);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_IncrefIfNotZero_traced)(DeeObject *__restrict ob,
                                   char const *file, int line) {
	(void)file;
	(void)line;
	return Dee_IncrefIfNotZero(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Decref_traced)(DeeObject *__restrict ob,
                          char const *file, int line) {
	(void)file;
	(void)line;
	Dee_Decref(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefDokill_traced)(DeeObject *__restrict ob,
                                char const *file, int line) {
	(void)file;
	(void)line;
	Dee_DecrefDokill(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefNokill_traced)(DeeObject *__restrict ob,
                                char const *file, int line) {
	(void)file;
	(void)line;
	Dee_DecrefNokill(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne_traced)(DeeObject *__restrict ob,
                               char const *file, int line) {
	(void)file;
	(void)line;
	return Dee_DecrefIfOne(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfNotOne_traced)(DeeObject *__restrict ob,
                                  char const *file, int line) {
	(void)file;
	(void)line;
	return Dee_DecrefIfNotOne(ob);
}

PUBLIC WUNUSED NONNULL((1)) Dee_refcnt_t
(DCALL Dee_DecrefAndFetch_traced)(DeeObject *__restrict ob,
                                  char const *file, int line) {
	(void)file;
	(void)line;
	return Dee_DecrefAndFetch(ob);
}

PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv_traced)(DeeObject *const *__restrict object_vector,
                           size_t object_count,
                           char const *file, int line) {
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	(void)file;
	(void)line;
	return Dee_Increfv(object_vector, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	size_t i;
	(void)file;
	(void)line;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Incref(ob);
	}
	return (DREF DeeObject **)object_vector;
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}

PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv_traced)(DREF DeeObject *const *__restrict object_vector,
                           size_t object_count,
                           char const *file, int line) {
	(void)file;
	(void)line;
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	return Dee_Decrefv(object_vector, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Decref(ob);
	}
	return (DREF DeeObject **)object_vector;
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}

PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *const *__restrict src,
                           size_t object_count,
                           char const *file, int line) {
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	(void)file;
	(void)line;
	return Dee_Movrefv(dst, src, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	size_t i;
	(void)file;
	(void)line;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = src[i];
		Dee_Incref(ob);
		dst[i] = ob;
	}
	return dst;
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}

PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *obj, size_t object_count,
                           char const *file, int line) {
	(void)file;
	(void)line;
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	return Dee_Setrefv(dst, obj, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	Dee_Incref_n_untraced(obj, object_count);
	return (DREF DeeObject **)memsetp(dst, obj, object_count);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}


DFUNDEF void (DCALL Dee_DumpReferenceLeaks)(void);
PUBLIC void (DCALL Dee_DumpReferenceLeaks)(void) {
	COMPILER_IMPURE();
}

#ifndef CONFIG_NO_BADREFCNT_CHECKS
#undef DeeObject_Destroy
#undef _DeeFatal_BadIncref
#undef _DeeFatal_BadDecref
#define DeeObject_Destroy(self) DeeObject_Destroy_d(self, __FILE__, __LINE__)
#define _DeeFatal_BadIncref(ob) DeeFatal_BadIncref((DeeObject *)(ob), __FILE__, __LINE__)
#define _DeeFatal_BadDecref(ob) DeeFatal_BadDecref((DeeObject *)(ob), __FILE__, __LINE__)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
#endif /* !CONFIG_TRACE_REFCHANGES */

/* Also export all the reference-control macros as functions. */
PUBLIC NONNULL((1)) void
(DCALL Dee_Incref)(DeeObject *__restrict ob) {
	Dee_Incref_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Incref_n)(DeeObject *__restrict ob, Dee_refcnt_t n) {
	Dee_Incref_n_untraced(ob, n);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_IncrefIfNotZero)(DeeObject *__restrict ob) {
	return Dee_IncrefIfNotZero_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Decref)(DeeObject *__restrict ob) {
	Dee_Decref_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefDokill)(DeeObject *__restrict ob) {
	Dee_DecrefDokill_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefNokill)(DeeObject *__restrict ob) {
	Dee_DecrefNokill_untraced(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne)(DeeObject *__restrict ob) {
	return Dee_DecrefIfOne_untraced(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfNotOne)(DeeObject *__restrict ob) {
	return Dee_DecrefIfNotOne_untraced(ob);
}

PUBLIC WUNUSED NONNULL((1)) Dee_refcnt_t
(DCALL Dee_DecrefAndFetch)(DeeObject *__restrict ob) {
	return Dee_DecrefAndFetch_untraced(ob);
}


/* incref() + return `self' (may be used in type operators,
 * and receives special optimizations in some situations) */
PUBLIC ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef)(DeeObject *__restrict self) {
	ASSERT_OBJECT(self);
	Dee_Incref(self);
	return self;
}

PUBLIC ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef_traced)(DeeObject *__restrict self,
                                char const *file, int line) {
	(void)file;
	(void)line;
	ASSERT_OBJECT_AT(self, file, line);
	Dee_Incref_traced(self, file, line);
	return self;
}



/* Increment the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv)(DeeObject *const *__restrict object_vector,
                    size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Incref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

/* Decrement the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv)(DREF DeeObject *const *__restrict object_vector,
                    size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Decref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

/* Copy object pointers from `src' to `dst' and increment
 * the reference counter of every object that got copied.
 * @return: * : Always re-returns the pointer to `dst' */
PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *const *__restrict src,
                    size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = src[object_count];
		Dee_Incref_untraced(ob);
		dst[object_count] = ob;
	}
	return dst;
}

/* Fill object pointers in `dst' with `obj' and increment
 * the reference counter of `obj' accordingly.
 * @return: * : Always re-returns the pointer to `dst' */
PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *obj, size_t object_count) {
	Dee_Incref_n_untraced(obj, object_count);
	return (DREF DeeObject **)memsetp(dst, obj, object_count);
}



#ifndef DID_DEFINE_DEEOBJECT_FREETRACKER
#define DID_DEFINE_DEEOBJECT_FREETRACKER
PUBLIC NONNULL((1)) void /* Defined only for binary compatibility */
(DCALL DeeObject_FreeTracker)(DeeObject *__restrict self) {
	COMPILER_IMPURE();
	(void)self;
}
#endif /* !DID_DEFINE_DEEOBJECT_FREETRACKER */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_REFCNT_C */
