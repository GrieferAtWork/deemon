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
#ifdef __INTELLISENSE__
#include "refcnt.c"
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1
//#define DEFINE_DeeGCObject_FinishDestroyAfterUntrack
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0) +                 \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0) +                 \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1) +                 \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1) + \
     defined(DEFINE_DeeGCObject_FinishDestroyAfterUntrack)) != 1
#error "Must #define exactly 1 of these macros"
#endif /* ... */

#include <deemon/api.h>

#include <deemon/alloc.h>       /* DeeObject_Free */
#include <deemon/gc.h>          /* DeeGCObject_Free, DeeGC_Head, DeeGC_UntrackAsync, Dee_GC_FLAG_FINALIZED */
#include <deemon/object.h>      /* DeeObject, DeeTypeObject, Dee_DecrefNokill, Dee_Decref_unlikely, Dee_TYPE, Dee_funptr_t */
#include <deemon/type.h>        /* DeeType_IsFinal, Dee_TF_TPVISIT */
#include <deemon/util/atomic.h> /* atomic_* */

#include <stddef.h> /* NULL */

DECL_BEGIN


/* Configure feature options:
 * LOCAL_HAS_Dtor == 0: No type-specific destructors
 * LOCAL_HAS_Dtor == 1: Exactly 1 type-specific destructor, which is
 *                      stored in "Dee_TYPE(self)->tp_init.tp_dtor"
 * LOCAL_HAS_Dtor == 2: Exactly 1 type-specific destructor, which is
 *                      stored in "Dee_TYPE(self)->tp_init.tp_dtor", but
 *                      that dtor takes 2 arguments, ala `Dee_TF_TPVISIT'
 * LOCAL_HAS_Dtor == 3: Destructors may exist in base-classes
 *
 * LOCAL_HAS_Free == 0:      "Dee_TYPE(self)" does not have a custom "tp_free"
 * LOCAL_HAS_Free == 1:      "Dee_TYPE(self)" defines a custom "tp_free"
 * !defined(LOCAL_HAS_Free): "Dee_TYPE(self)" may have a custom "tp_free"
 *
 * LOCAL_HAS_HeapType == 0:  "Dee_TYPE(self)" isn't a heap-type (!TP_FHEAP)
 * LOCAL_HAS_HeapType == 1:  "Dee_TYPE(self)" is a heap-type (TP_FHEAP)
 * LOCAL_HAS_HeapType == 2:  "Dee_TYPE(self)" may be a heap-type
 *
 * LOCAL_HAS_GC:        "Dee_TYPE(self)" is a GC-type (TP_FGC)
 * LOCAL_HAS_Rev:       type-specific destructors may revive the object (TP_FMAYREVIVE) */
#ifdef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC0_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC1_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC0_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC1_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC0_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC1_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC0_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC1_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#define LOCAL_HAS_Dtor                 3
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#define LOCAL_HAS_Dtor                 3
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define LOCAL_HAS_Dtor                 3
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#define LOCAL_HAS_Dtor                 3
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeGCObject_FinishDestroyAfterUntrack)
#define LOCAL_DeeObject_DefaultDestroy DeeGCObject_FinishDestroyAfterUntrack
#define LOCAL_HAS_Dtor 3     /* Unknown... */
#undef LOCAL_HAS_Free        /* Unknown... */
#define LOCAL_HAS_HeapType 2 /* Unknown... */
#define LOCAL_HAS_GC       1 /* Only used for GC-objects */
#define LOCAL_HAS_Rev      0 /* "tp_finalize" was already called at this point */
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifndef LOCAL_HAS_Dtor
#define LOCAL_HAS_Dtor 3
#endif /* !LOCAL_HAS_Dtor */
#ifndef LOCAL_HAS_HeapType
#define LOCAL_HAS_HeapType 1
#endif /* !LOCAL_HAS_HeapType */
#ifndef LOCAL_HAS_Rev
#if LOCAL_HAS_Dtor == 0
#define LOCAL_HAS_Rev 0
#else /* LOCAL_HAS_Dtor == 0 */
#define LOCAL_HAS_Rev 1
#endif /* LOCAL_HAS_Dtor != 0 */
#endif /* !LOCAL_HAS_Rev */
#ifndef LOCAL_HAS_GC
#error "LOCAL_HAS_GC must be specified statically"
#endif /* !LOCAL_HAS_GC */

PRIVATE NONNULL((1)) void DCALL
LOCAL_DeeObject_DefaultDestroy(DeeObject *__restrict self) {
	DeeTypeObject *orig_type;

#if LOCAL_HAS_Rev
	/* Invoke "tp_finalize" operators */
#if LOCAL_HAS_GC
	if (!(atomic_fetchor(&DeeGC_Head(self)->gc_info.gi_flag,
	                     Dee_GC_FLAG_FINALIZED) &
	      Dee_GC_FLAG_FINALIZED))
#endif /* LOCAL_HAS_GC */
	{
		/* FIXME: This way of doing destructors is cool and all, however:
		 *        there are ways to create infinite loops that make it
		 *        impossible for deemon to exit properly:
		 *
		 * >> global instance;
		 * >> class MyClass {
		 * >>     ~this() {
		 * >>         print "In destructor:", this.id;
		 * >>         instance = MyClass();
		 * >>     }
		 * >> }
		 * >> MyClass();
		 *
		 * ^^ This code doesn't even need to make use of self-reviving objects.
		 *    All it does is simply create a new instance of the class that is
		 *    being finalized. This then repeats indefinitely as the GC notices
		 *    over and over again that "instance" is unreachable.
		 *
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> In destructor: 10439416
		 * >> In destructor: 10420432
		 * >> [...]
		 *
		 * Doing the same thing in python:
		 * >> instance = None
		 * >> class MyClass:
		 * >>     def __del__(self):
		 * >>         global instance
		 * >>         print("In fini: " + str(MyClass))
		 * >>         instance = MyClass()
		 * >> MyClass()
		 *
		 * This code seems to resolve itself after only a couple of GC iterations:
		 * >> In fini: __main__.MyClass
		 * >> In fini: __main__.MyClass
		 * >> In fini: None
		 * >> Exception TypeError: "'NoneType' object is not callable" in <bound method MyClass.__del__ of <__main__.MyClass instance at 0x03391E90>> ignored
		 *
		 * So python is (somehow) solving this problem.
		 * However, I have no idea what's allowing it to clear the "MyClass" variable
		 * in this situation, since obviously that variable is still being used by the
		 * __del__ method that's yet to be invoked.
		 *
		 * Adjusting the code above to work around MyClass suddenly being "None":
		 * >> instance = None
		 * >> class MyClass:
		 * >>     def __init__(self):
		 * >>         self.c = MyClass
		 * >>     def __del__(self):
		 * >>         global instance
		 * >>         print("In fini: " + str(MyClass) + ", " + str(self.c))
		 * >>         instance = self.c()
		 * >> MyClass()
		 *
		 * And now the output is this:
		 * >> In fini: __main__.MyClass, __main__.MyClass
		 * >> In fini: __main__.MyClass, __main__.MyClass
		 * >> In fini: None, __main__.MyClass
		 *
		 * So obviously, at some point python just stops bothering to invoke user-
		 * defined __del__ method entirely. I wonder what's causing this...
		 *
		 * ---
		 *
		 * Reading python's docs, I think this line explains it:
		 * >> It is not guaranteed that __del__() methods are called for objects that
		 * >> still exist when the interpreter exits. 
		 */
		DeeTypeObject *type = Dee_TYPE(self);
		ASSERT(self->ob_refcnt == 0);
		atomic_write(&self->ob_refcnt, 1);
		do {
			if (type->tp_init.tp_finalize) {
				COMPILER_WRITE_BARRIER();
				(*type->tp_init.tp_finalize)(type, self);
				COMPILER_READ_BARRIER();
#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
				Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */
			}
		} while ((type = type->tp_base) != NULL);

		/* After finalizers were invoked: check if object has been revived */
		if unlikely(atomic_decfetch(&self->ob_refcnt) != 0)
			return;
	}
#endif /* LOCAL_HAS_Rev */

#ifndef DEFINE_DeeGCObject_FinishDestroyAfterUntrack
	/* Start by untracking the object in question. */
#if LOCAL_HAS_GC
	self = DeeGC_UntrackAsync(self);
	if unlikely(self == NULL)
		return; /* Remainder of object destruction happens asynchronously */
#endif /* LOCAL_HAS_GC */
#endif /* !DEFINE_DeeGCObject_FinishDestroyAfterUntrack */

	/* Load the object's type. */
	orig_type = Dee_TYPE(self);

#if LOCAL_HAS_Dtor > 2
	{
		DeeTypeObject *type = orig_type;
		do {
			ASSERT(self->ob_refcnt == 0);
			ASSERTF(type == orig_type || !DeeType_IsFinal(type),
			        "Final type `%k' with sub-class `%k'",
			        type, orig_type);
			if (type->tp_init.tp_dtor) {
				if (type->tp_features & Dee_TF_TPVISIT) {
					(*(void (DCALL *)(DeeTypeObject *, DeeObject *__restrict))(Dee_funptr_t)type->tp_init.tp_dtor)(type, self);
				} else {
					(*type->tp_init.tp_dtor)(self);
				}
#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
				Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */
			}
		} while ((type = type->tp_base) != NULL);
	}
#elif LOCAL_HAS_Dtor == 2 || LOCAL_HAS_Dtor == 1

	/* Invoke a custom destructor callback. */
#if LOCAL_HAS_Dtor == 2
	(*(void (DCALL *)(DeeTypeObject *, DeeObject *__restrict))(Dee_funptr_t)orig_type->tp_init.tp_dtor)(orig_type, self);
#else /* LOCAL_HAS_Dtor == 2 */
	(*orig_type->tp_init.tp_dtor)(self);
#endif /* LOCAL_HAS_Dtor != 2 */

#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */
#endif /* LOCAL_HAS_Dtor == ... */

	/* Free the tracker for changes to reference counts. */
#ifdef CONFIG_TRACE_REFCHANGES
	free_reftracker(self->ob_trace);
#endif /* CONFIG_TRACE_REFCHANGES */

#if LOCAL_HAS_HeapType
#define LOCAL_decref_orig_type() Dee_Decref_unlikely(orig_type)
#else /* LOCAL_HAS_HeapType */
	/* Non-heap types can never be destroyed, so we can use the *Nokill version here! */
#define LOCAL_decref_orig_type() Dee_DecrefNokill(orig_type)
#endif /* !LOCAL_HAS_HeapType */

	/* Invoke `tp_free' of `orig_type', and decref `orig_type' */
#ifndef LOCAL_HAS_Free
	if (orig_type->tp_init.tp_alloc.tp_free) {
#if LOCAL_HAS_HeapType
		(*orig_type->tp_init.tp_alloc.tp_free)(self);
		LOCAL_decref_orig_type();
#else /* LOCAL_HAS_HeapType */
		LOCAL_decref_orig_type();
		(*orig_type->tp_init.tp_alloc.tp_free)(self);
#endif /* !LOCAL_HAS_HeapType */
	} else {
		LOCAL_decref_orig_type();
#if LOCAL_HAS_GC
		DeeGCObject_Free(self);
#else /* LOCAL_HAS_GC */
		DeeObject_Free(self);
#endif /* !LOCAL_HAS_GC */
	}
#elif LOCAL_HAS_Free && LOCAL_HAS_HeapType
	(*orig_type->tp_init.tp_alloc.tp_free)(self);
	/* It's a heap-type, so "tp_free" may become invalid if this ends up
	 * destroying the type (as such: this has to happen *after* "tp_free") */
	LOCAL_decref_orig_type();
#elif LOCAL_HAS_Free
	/* Static type, so allowed to decref it before calling "tp_free"
	 * (doing it this way allows compiler to optimize tail recursion) */
	LOCAL_decref_orig_type();
	(*orig_type->tp_init.tp_alloc.tp_free)(self);
#elif LOCAL_HAS_GC
	LOCAL_decref_orig_type();
	DeeGCObject_Free(self);
#else /* ... */
	LOCAL_decref_orig_type();
	DeeObject_Free(self);
#endif /* !... */
#undef LOCAL_decref_orig_type
}

#undef LOCAL_DeeObject_DefaultDestroy
#undef LOCAL_HAS_Dtor
#undef LOCAL_HAS_Free
#undef LOCAL_HAS_HeapType
#undef LOCAL_HAS_GC
#undef LOCAL_HAS_Rev

DECL_END

#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType0_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free0_HeapType1_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType0_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor2_Free1_HeapType1_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#undef DEFINE_DeeGCObject_FinishDestroyAfterUntrack
